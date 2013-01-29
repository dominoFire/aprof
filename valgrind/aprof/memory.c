/*
 * Load, store and modify handlers
 * 
 * Last changed: $Date$
 * Revision:     $Rev$
 */
 
/*
   This file is part of aprof, an input sensitive profiler.

   Copyright (C) 2011-2013, Emilio Coppa (ercoppa@gmail.com),
                            Camil Demetrescu,
                            Irene Finocchi,
                            Romolo Marotta

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

/* Memory resolution: we can aggregate addresses in order
 * to decrese the shadow memory. 
 * - 1 => finest resolution, each byte has its timestamp
 * - 2 => every 2 bytes we have a single timestamp 
 * - ...
 */
UInt APROF_(addr_multiple) = 4;

#if INPUT_METRIC == RVMS
/* 
 * Global shadow memory used for checking latest "version" of an input.
 * If a routine writes a memory cell, we update the associated timestamp
 * in the global shadow memory with the current value of the global counter 
 */
LookupTable * APROF_(global_shadow_memory) = NULL;

/*
 * This global counter is increased by one when there is:
 * - a new routine activation
 * - a thread switch
 */
UInt APROF_(global_counter) = 0;
#endif

memory_event * memory_buffer = NULL;
ULong memory_buffer_size      = 0;
UInt count                   = 0;

VG_REGPARM(3) void APROF_(trace_access)(UWord type, 
                                        Addr addr, 
                                        SizeT size,
                                        UWord kernel_access) {

    /*
    VG_(printf)("memory_buffer: %p\n", memory_buffer);
    VG_(printf)("memory_buffer_size: %llu\n", memory_buffer_size);
    VG_(printf)("count: %u\n", count);
    */
    
    AP_ASSERT(0, "babab");
    
    memory_event * ev = &(memory_buffer[count]);
    AP_ASSERT(ev->addr == addr, "Bad address");
    AP_ASSERT(ev->size == size, "Bad address");
    AP_ASSERT(ev->type == type, "Bad address");
    
    count++;
    AP_ASSERT(count * sizeof(memory_event) == memory_buffer_size, "Bad count");
    
    AP_ASSERT(count < BUF_MEM_SIZE, "Memory buffer overflow");
    /*
    memory_buffer[memory_buffer_size].addr = addr;
    memory_buffer[memory_buffer_size].size = size;
    memory_buffer[memory_buffer_size].type = type;
    
    AP_ASSERT(memory_buffer[memory_buffer_size].addr == addr, "error");
    AP_ASSERT(memory_buffer[memory_buffer_size].size == size, "error");
    AP_ASSERT(memory_buffer[memory_buffer_size].type == type, "error");
    
    memory_buffer_size++;
    
    if (memory_buffer_size == BUF_MEM_SIZE)
        commit_memory_events();
*/
}

void init_memory_buffer(void) {
    
    //VG_(printf)("memory_buffer: %p\n", memory_buffer);
    memory_buffer = VG_(calloc)("memory events", 
        BUF_MEM_SIZE, sizeof(memory_event));
    //VG_(printf)("memory_buffer: %p\n\n", memory_buffer);

}

inline void memory_commit(UWord type, 
                            Addr addr, 
                            SizeT size);

void commit_memory_events(void) {

    UInt size = memory_buffer_size / sizeof(memory_event);
    /*
    if (size > 500)
        VG_(printf)("Commit: %u\n", size);
    */
    UInt i;
    for (i = 0; i < size; i++) {

        memory_commit(  memory_buffer[i].type, 
                                memory_buffer[i].addr, 
                                memory_buffer[i].size);
        
    }

    memory_buffer_size = 0;
    count = 0;
}

inline void memory_commit(UWord type, 
                            Addr addr, 
                            SizeT size) {

    ThreadData * tdata = APROF_(current_tdata);
    
    #if DEBUG
    AP_ASSERT(tdata != NULL, "Invalid tdata");
    #endif
    
    #if VERBOSE > 1
    if (type == LOAD) VG_(printf)("Load: %lu:%lu\n", addr, size);
    else if (type == STORE) VG_(printf)("Store: %lu:%lu\n", addr, size);
    else if (type == MODIFY) VG_(printf)("Modify: %lu:%lu\n", addr, size);
    #endif
    
    #if EVENTCOUNT == 1 || EVENTCOUNT == 3
    if (type == LOAD) tdata->num_read++;
    else if (type == STORE) tdata->num_write++;
    else if (type == MODIFY) tdata->num_modify++;
    else AP_ASSERT(0, "Invalid type")
    #endif
    
    #if EMPTY_ANALYSIS
    return;
    #endif
    
    if (tdata->stack_depth == 0) return;
    
    #if TRACE_FUNCTION && IGNORE_DL_RUNTIME
    if (tdata->skip) return;
    #endif
    
    #if COSTANT_MEM_ACCESS
    addr = addr & ~(APROF_(addr_multiple)-1);
    #else
    Int size_fix = size;
    APROF_(fix_access_size)(addr, size_fix);
    #endif
    
    #if !COSTANT_MEM_ACCESS
    while (size_fix > 0) {
    #endif
        
        //VG_(printf)("addr: %lu ~ size: %d\n", addr, size_fix);
        
        Activation * act = APROF_(get_activation_noresize)(tdata, tdata->stack_depth);
        
        #if INPUT_METRIC == RVMS
        
        UInt ts = APROF_(global_counter);
        UInt old_ts;
        UInt wts;
        
        /*
         * We are writing a new input value. So, this is a new "version"
         * of the memory cell. Update its timestamp in the global shadow memory.
         */
         if (type == STORE || type == MODIFY) {
            
            #if THREAD_INPUT == 1
            wts = LK_insert(APROF_(global_shadow_memory), addr, ts);
            #endif

            if (kernel_access) {
                
                #if THREAD_INPUT == 0
                wts = LK_insert(APROF_(global_shadow_memory), addr, ts);
                #endif
                
                #if !COSTANT_MEM_ACCESS
                size_fix -= APROF_(addr_multiple);
                addr += APROF_(addr_multiple);
                continue;
                #else
                return;
                #endif
            }

        } else { 
            
            /*
             * This is a load. What is the latest "version" of this
             * memory cell? Get timestamp of the memory cell in
             * the global shadow memory.
             */
            wts = LK_lookup(APROF_(global_shadow_memory), addr);

        }

        /*
         * Update the timestamp in the private shadow memory.
         */
        old_ts = LK_insert(tdata->accesses, addr, ts);

        if(type == STORE) {
            #if COSTANT_MEM_ACCESS
            return;
            #else
            size_fix -= APROF_(addr_multiple);
            addr += APROF_(addr_multiple);
            continue;
            #endif
        }

        if(old_ts < wts){
            act->rvms++;
        }
 
        else if (old_ts < act->aid) {
            
            act->rms++;
            act->rvms++;
            if (old_ts > 0 && old_ts >= APROF_(get_activation_noresize)(tdata, 1)->aid) {
                
                act = APROF_(get_activation_by_aid)(tdata, old_ts);
                act->rvms--;
                act->rms--;
            
            }

        }
        
        #else
        
        UInt old_aid = LK_insert( tdata->accesses, addr, act->aid);
        
        if (old_aid < act->aid && (type == LOAD || type == MODIFY)) {
            
            act->rms++;
            if (old_aid > 0 && old_aid >= APROF_(get_activation_noresize)(tdata, 1)->aid) {
                
                APROF_(get_activation_by_aid)(tdata, old_aid)->rms--;

            }

        }
        
        #endif
    
    #if !COSTANT_MEM_ACCESS
    
        size_fix -= APROF_(addr_multiple);
        addr += APROF_(addr_multiple);
        
    }
    #endif
    
}

