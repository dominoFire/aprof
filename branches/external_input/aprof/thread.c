/*
 * Thread event handlers (start, exit, running)
 * 
 * Last changed: $Date$
 * Revision:     $Rev$
 */

/*
   This file is part of aprof, an input sensitive profiler.

   Copyright (C) 2011-2012, Emilio Coppa (ercoppa@gmail.com),
                            Camil Demetrescu,
                            Irene Finocchi

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.
*/

#include "aprof.h"


/* # running threads */
UInt APROF_(running_threads) = 0;

/* All threads running */
static ThreadData * threads[VG_N_THREADS];

/* Current running thread ID */
ThreadId APROF_(current_TID) = VG_INVALID_THREADID; /* 0 */

/* Current running thread data */
ThreadData * APROF_(current_tdata) = NULL; 


static ThreadData * APROF_(thread_start)(ThreadId tid){

	#if VERBOSE
	VG_(printf)("start thread data %d\n", tid);
	#endif
	
	ThreadData * tdata = VG_(calloc)("thread_data", sizeof(ThreadData), 1);
	#if DEBUG
	AP_ASSERT(tdata != NULL, "thread data not allocable");
	#endif
	
	#if DEBUG_ALLOCATION
	APROF_(add_alloc)(TS);
	#endif
	
	#if DEBUG
	AP_ASSERT(tid - 1 < VG_N_THREADS && tid > 0, "Tid is too big");
	#endif
	threads[tid-1] = tdata;
	
	/* we deallocate all routine info when we generate the report */
	tdata->routine_hash_table = HT_construct(NULL);
	AP_ASSERT(tdata->routine_hash_table != NULL, "rtn ht not allocable");
	
	#if DEBUG_ALLOCATION
	APROF_(add_alloc)(HT);
	#endif
	
	tdata->stack_depth = 0;
	tdata->max_stack_size = STACK_SIZE;
	tdata->stack = VG_(calloc)("stack", STACK_SIZE * sizeof(Activation), 1);
	#if DEBUG
	AP_ASSERT(tdata->stack != NULL, "stack not allocable");
	#endif
	
	#if DEBUG_ALLOCATION
	int j = 0;
	for (j = 0; j < STACK_SIZE; j++) APROF_(add_alloc)(ACT);
	#endif
	
	//tdata->next_routine_id = 0;
	
	#if TRACE_FUNCTION
	tdata->last_bb = NULL;
	#endif
	
	#if EMPTY_ANALYSIS
	return tdata;
	#endif
	
	tdata->accesses = LK_create();

	#if CCT
	
	// allocate dummy CCT root
	tdata->root = (CCTNode*) VG_(calloc)("CCT", sizeof(CCTNode), 1);
	#if DEBUG
	AP_ASSERT(tdata->root != NULL, "Can't allocate CCT root node");
	#endif

	#if DEBUG_ALLOCATION
	APROF_(add_alloc)(CCTS);
	#endif

	//tdata->root->context_id = 0;
	
	tdata->next_context_id = 1;
	#endif
	
	#if TIME == RDTSC
	tdata->entry_time = APROF_(time)();
	#endif

	APROF_(running_threads)++;

	return tdata;

}

void APROF_(thread_exit)(ThreadId tid){
	
	APROF_(current_TID) = VG_INVALID_THREADID;
	APROF_(current_tdata) = NULL;
	
	#if VERBOSE
	VG_(printf)("Exit thread %d\n", tid);
	#endif
	
	#if DEBUG
	AP_ASSERT(tid - 1 < VG_N_THREADS && tid > 0, "Invalid tid")
	#endif
	ThreadData * tdata = threads[tid - 1];
	#if DEBUG
	AP_ASSERT(tdata != NULL, "Invalid tdata")
	#endif
	
	/* Unregister thread info */
	threads[tid -1] = NULL;
	APROF_(current_TID) =  VG_INVALID_THREADID;
	APROF_(current_tdata) = NULL;
	
	#if EVENTCOUNT
	VG_(printf)("[TID=%d] Load: %llu\n", tid, tdata->num_read);
	VG_(printf)("[TID=%d] Store: %llu\n", tid, tdata->num_write);
	VG_(printf)("[TID=%d] Modify: %llu\n", tid, tdata->num_modify);
	VG_(printf)("[TID=%d] Function entry: %llu\n", tid, tdata->num_func_enter);
	VG_(printf)("[TID=%d] Function exit: %llu\n", tid, tdata->num_func_exit);
	VG_(printf)("[TID=%d] Total accesses: %llu\n", tid, 
													tdata->num_modify +
													tdata->num_write +
													tdata->num_read
													);
	#endif
	
	#if SUF2_SEARCH == STATS
	VG_(printf)("[TID=%d] Average stack depth: %llu / %llu = %llu\n", 
					tid, tdata->avg_depth, ops, tdata->avg_depth/ops);
	VG_(printf)("[TID=%d] Average # iterations: %llu / %llu = %llu\n", 
					tid, tdata->avg_iteration, ops, tdata->avg_iteration/ops);
	#endif
	
	#if EMPTY_ANALYSIS 
	HT_destruct(tdata->routine_hash_table);
	VG_(free)(tdata->stack);
	VG_(free)(tdata);
	return;
	#endif
	
	#if 0
	/* Some functions are not transformed in routines: */
	HT_ResetIter(fn_ht);
	Function * fn = HT_Next(fn_ht);
	while(fn != NULL) {
		
		RoutineInfo * rtn = HT_lookup(tdata->routine_hash_table, (UWord) fn);
		if (rtn == NULL) 
		VG_(printf)("Function %s is not a routine for this thread\n", fn->name);
		
		fn = HT_Next(fn_ht);
	}
	#endif
	
	APROF_(generate_report)(tdata, tid);
	
	APROF_(running_threads)--;
	
	/* destroy all thread data data */
	
	LK_destroy(tdata->accesses);
	
	#if CCT
	// deallocate CCT
	APROF_(free_CCT)(tdata->root);
	#endif
	
	HT_destruct(tdata->routine_hash_table);
	
	VG_(free)(tdata->stack);
	VG_(free)(tdata);

}

void APROF_(switch_thread)(ThreadId tid, ULong blocks_dispatched) {
	
	/* 
	 * Note: Callgrind says that if you have not done at least
	 * 5000 blocks you can ignore this call.
		-----
		static ULong last_blocks_done = 0;
		// throttle calls to CLG_(run_thread) by number of BBs executed
		if (blocks_done - blocks_dispatched < 5000) return;
		------
	 * Why? Investigate! 
	 */
	 
	if (tid == APROF_(current_TID)) return;
	
	#if TRACE_FUNCTION
	/* save last exit of the previous thread */
	if (APROF_(current_tdata) != NULL)
		APROF_(current_tdata)->last_exit = APROF_(last_exit);
	#endif
	
	APROF_(current_TID) = tid;
	
	if (tid == VG_INVALID_THREADID) {
		APROF_(current_tdata) = NULL;
		return;
	}
	
	if (threads[tid-1] == NULL) 
		APROF_(current_tdata) = APROF_(thread_start)(tid);
	else 
		APROF_(current_tdata) = threads[tid -1];
	
	#if DEBUG
	AP_ASSERT(APROF_(current_tdata) != NULL, "Invalid tdata");
	#endif
	
	#if TRACE_FUNCTION
	/* restore exit value of the current thread */
	APROF_(last_exit) = APROF_(current_tdata)->last_exit;
	#endif
}

void APROF_(print_stacks_acts)() {

	int i = 0;
	for (i = 0; i < APROF_(running_threads); i++) {
	
		if (threads[i] == NULL) continue;
		int depth = threads[i]->stack_depth;
		VG_(printf)("STACK THREAD %u\n", i);
		
		while (depth > 0) {
			
			VG_(printf)("[%d] %u\n", depth, 
				APROF_(get_activation)(threads[i], depth)->aid);
			
		}
		
	}
	
	
}

/*
 * global_counter is 32 bit integer so it can overflow. To overcome this
 * issue we compress our set of valid timestamps (e.g., after an overflow).
 * 
 * 1) scan all shadow thread stacks (in parallel): collect the (sorted) 
 *    set of used timestamps by all activations.
 * 
 * 2) After and before each activation ts insert a (unknown... for now) ts:
 *    all writes between two activations will be assigned to this ts.
 * 
 * 3) element i-th of the set is associated to the new timestamp i.
 *    re-assign ts to all activations.
 * 
 * 4) re-assign ts in the global shadow memory (see LK_compress_global)
 *    and compute unknown ts
 * 
 * 5) re-assign ts in all private shadow memories (see LK_compress)
 * 
 * Return the new starting value for the global counter.
 */
UInt APROF_(overflow_handler)(void){
	
	UInt sum = 0; // # valid timestamps
	UInt max = 0; 
	UInt count_thread = APROF_(running_threads);
	Activation * act_max;
	int index[count_thread]; 

	int i, j, k;
	k = i = j = 0;
	
	APROF_(print_stacks_acts)();
	
	/* compute the number of different activation-ts */

	while(i < count_thread && j < VG_N_THREADS){
		
		if(threads[j] == NULL)
			j++;
		else{
			sum += (index[i] = threads[j++]->stack_depth);
			i++;
		}
	}
	
	/* 
	 * Between two activation ts we insert a "cumulative" ts
	 * for all the write ops (performed btw these two time instants).
	 * The value of this ts is unknown (we compute the right value
	 * in LK_compress_global) so we only double the size of the
	 * set of valid ts. 
	 */
	sum = sum << 1;
	sum++;
	
	/* 
	 * an array where are two kind of timestamps 
	 * if i % 2 == 0 array[i] is a write-ts
	 * else is a activation-ts 
	 */
	UInt* array = VG_(calloc)("array overflow", sum, sizeof(UInt));
	max = 0;
	
	/*
	 * Info about activation/thread with the max activation-ts
	 *  max_ind[0]: index of the thread in threads[]
	 *  max_ind[1]: this is the index in index[] associated to this thread.
	 *              index[i] contains the lower activation (of the shadow
	 *              stack for i-th thread) already checked as candidate
	 *              for the current max      
	 */
	int max_ind[2] = {0, 0};
	
	/* 
	 * Collect valid activation-ts using a merge 
	 * and re-assign the new ts 
	 *  
	 *  i -= 2 because we alternate act-ts and write-ts
	 */
	for(i = sum-2; i > 0; i -= 2){
		
		k = 0;

		/* find the max activation ts among all shadow stacks */ 
		for(j = 0; j < count_thread; j++){

			while(threads[k] == NULL) k++;

			if(index[j] > 0){
				
				act_max = APROF_(get_activation)(threads[k], index[j]);
				if(max < act_max->aid){
				
					max_ind[1] = j;
					max_ind[0] = k;
				
				}
			
			}
	
		}
		act_max = APROF_(get_activation)(threads[max_ind[0]], index[max_ind[1]]);
		array[i] = act_max->aid;
		index[max_ind[1]]--; // next time we check for the max the caller of this act
		act_max->aid = i; // re-assign ts

	}
	
	VG_(printf)("\nArray overflow:\n");
	for (i = 0; i < sum; i++)
		VG_(printf)("%lu ", array[i]);
	
	APROF_(print_stacks_acts)();
	
	// compress global shadow memory and compute new "cumulative" write-ts 
	LK_compress_global(array, sum);
	
	VG_(printf)("\nArray overflow:\n");
	for (i = 0; i < sum; i++)
		VG_(printf)("%lu ", array[i]);
	
	// compress all private shadow memories
	i = j = 0;
	while(i < count_thread && j < VG_N_THREADS){
		if(threads[j] == NULL)
			j++;
		else{
			LK_compress(threads[j++]->accesses, array, sum);
			i++;
		}
	}

	VG_(free)(array);
	return sum + 1;
}