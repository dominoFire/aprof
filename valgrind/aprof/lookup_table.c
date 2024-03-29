/*
 * lookup table implementation
 * 
 * Last changed: $Date: 2013-03-13 18:08:19 +0100 (mer, 13 mar 2013) $
 * Revision:     $Rev: 822 $
 */

/*
   This file is part of aprof, an input sensitive profiler.

   Copyright (C) 2011-2014, Emilio Coppa (ercoppa@gmail.com),
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
                                                                                      
LookupTable * LK_create(void) {

    LookupTable * lk = APROF_(new)(LK_S, sizeof(struct LookupTable));
    switch(APROF_(runtime).memory_resolution) {
        
        case 4:
            lk->flt_size = 16384; 
            lk->res_shift = 2;
            break;
        
        case 1:
            lk->flt_size = 65536; 
            lk->res_shift = 0;
            break;
        
        case 2:
            lk->flt_size = 32768; 
            lk->res_shift = 1;
            break;
        
        case 8:
            lk->flt_size = 8192; 
            lk->res_shift = 3;
            break;
        
        case 16:
            lk->flt_size = 4096; 
            lk->res_shift = 4;
            break;
        
        default:
            APROF_(assert)(0, "Supported memory resolutions: 1, 2, 4, 8 or 16");
    }
    
    return lk;
}

void LK_destroy(LookupTable * lk) {

    UInt i = 0;
    while (i < LK_SIZE) {
        
        if (lk->table[i] != NULL) {
        
            #ifndef __i386__
                
                UInt j = 0;
                while (j < ILT_SIZE) {
                    
                    if (lk->table[i]->table[j] != NULL) 
                        APROF_(delete)(SEG_LK_S, lk->table[i]->table[j]);

                    j++;            
                }
                APROF_(remove_alloc)(ILT_LK_S);
            
            #else // i386
                APROF_(remove_alloc)(SEG_LK_S);
            #endif // i386
            
            VG_(free)(lk->table[i]);
        }
        i++;
    }
    APROF_(delete)(LK_S, lk); 
}



UInt LK_insert(LookupTable * lk, UWord addr, UInt ts) {

    APROF_(debug_assert)(lk != NULL, "Invalid lookup table");

    #ifdef __i386__
    UWord i = addr >> 16;
    
    #else // __i386__
    
    UWord i = addr >> 30; // 14 + 16
    
    #if CHECK_ADDR_OVERFLOW
    APROF_(assert)((i < LK_SIZE), "Address overflow");
    #endif
    
    UWord k = (addr >> 16) & 0x3fff;
    
    #endif // ! __i386__
    
    UWord j = (addr & 0xffff) >> lk->res_shift;
    
    #ifndef __i386__
    if (lk->table[i] == NULL) {
        lk->table[i] = APROF_(new)(ILT_LK_S, sizeof(ILT));
    } 
    #endif // __i386__

    #ifdef __i386__
    if (lk->table[i] == NULL) {
    #else // __i386__
    if (lk->table[i]->table[k] == NULL) {
    #endif // ! __i386__
    
        #ifdef __i386__
        lk->table[i] = APROF_(new)(SEG_LK_S, sizeof(UInt) * lk->flt_size);
        lk->table[i][j] = ts;
        #else // __i386__
        lk->table[i]->table[k] = APROF_(new)(SEG_LK_S, sizeof(UInt) * lk->flt_size);
        lk->table[i]->table[k][j] = ts;
        #endif // ! __i386__

        return 0;
    }
    
    #ifdef __i386__
    UInt * value = &(lk->table[i][j]);
    #else // __i386__
    UInt * value = &(lk->table[i]->table[k][j]);
    #endif // ! __i386__
    
    UInt old = *value;
    *value = ts;

    return old;
}

UInt LK_lookup(LookupTable * lk, UWord addr) {
    
    UWord j = (addr & 0xffff) >> lk->res_shift;
    
    #ifdef __i386__
    
    UWord i = addr >> 16;
    UInt * ssm = lk->table[i];
    if (ssm == NULL) return 0;
    return ssm[j];
    
    #else // __i386__
    
    UWord i = addr >> 30; // 14 + 16
    
    #if CHECK_ADDR_OVERFLOW
    APROF_(assert)((i < LK_SIZE), "Address overflow");
    #endif
    
    UWord k = (addr >> 16) & 0x3fff;
    if (lk->table[i] == NULL) return 0;
    UInt * ssm = lk->table[i]->table[k];
    if (ssm == NULL) return 0;
    return ssm[j];
    
    #endif // ! __i386__
}

static UInt binary_search(UInt * array, UInt init, UInt size, UInt ts){
    
    /*
     * t == 0: ignore thread
     * t > 0: check thread
     */
    
    if (size == 1) return 0;
    if (array[1] > ts) return 0;
    if (array[size - 1] <= ts) return size - 1;

    Int min = init;
    Int max = size - 1;
    do {
        
        Int index = (min + max) / 2;
        
        if (array[index] == ts) {
            return index;
        }
        
        else if (array[index] > ts) 
            max = index - 1; 
            
        else {
            
            if (array[index + 1] > ts) {
                return index;
            }
            min = index + 1;
        }
        
    } while(min <= max);

    VG_(umsg)("Binary(%u) ", ts);
    UInt i = 0;
    while (i < size) VG_(umsg)("%u ", array[i++]);
    VG_(umsg)("\n");
    APROF_(assert)(0, "Binary search fail");
    return 0;
}

/*
 * Compress global and local (thread) shadow memories.
 * 
 * array: set of valid/active ts-activations on the stacks
 * size: size of the prev array
 * shamem: pointers to active local shadow memories
 * [file]: file for debug output
 * 
 * For each memory location x:
 * - find its ts (gts) in the global shadow memory (global[x])
 * - find the index (i) in array of max activation-ts that is <= gts
 * - for active thread, check local ts (lts) of x (local[x]):
 *      - if (lts < array[i]) 
 *              then local[x] = 3 * (index max act-ts that is <= lts)
 *      - elif (lts >= array[i+1]) 
 *              then local[x] = 3 * (index max act-ts that is <= lts)
 *      - elif gts == lts  then local[x] = 3 * i + 1
 *      - elif gts > lts then local[x] = 3 * i
 *      - elif gts < lts then local[x] = 3 * i + 2
 *    finally: global[x] = 3 * i + 1.
 * 
 * Our goal is to preserve the relantionship between gts, lts and
 * all active-ts.
 */
#if COMPRESS_DEBUG
void LK_compress_drms(UInt * array, UInt size, LookupTable ** shamem, void * file) {
#else
void LK_compress_drms(UInt * array, UInt size, LookupTable ** shamem) {
#endif

    #if COMPRESS_DEBUG
    FILE * f = (FILE *) file;
    #endif
    
    UInt count_thread = APROF_(runtime).running_threads;
    UInt i = 0;
    UInt k = 0;
    UInt ts = 0;
    UInt t = 0;
    
    #ifndef __i386__ 
    UInt j = 0;
    #endif
    
    // scan global shadow memory (GSM)
    for(i = 0; i < LK_SIZE; i++){
    
        // chunk: a list of ts (e.g. ts for a 64KB segment) 
        UInt * global_chunk = (UInt *) APROF_(runtime).global_shadow_memory->table[i];
        #if COMPRESS_DEBUG
        if (global_chunk != NULL) VG_(umsg)("Global chunk %u exists\n", i);
        #endif

        #ifndef __i386__ // 64bit: 3 level lookup table...
        for(j = 0; j < ILT_SIZE; j++){

            /* 
             * check if this first level chunck of GSM is valid
             * if not we assume wts[x] = 0 
             */

            if(APROF_(runtime).global_shadow_memory->table[i] != NULL) {
                
                global_chunk = APROF_(runtime).global_shadow_memory->table[i]->table[j];
                
                #if COMPRESS_DEBUG
                if (global_chunk != NULL)
                    VG_(umsg)("Global chunk (%u,%u) exists\n", i, j);
                #endif
            
            } 
            
        #elif COMPRESS_DEBUG
        UInt j = 0;
        #endif
            
            UInt * local[count_thread];
            UInt q = 0;
            for(t = 0; t < count_thread; t++){
                
                // check if this current chunk was accessed by thread t
                
                UInt * local_chunk = (UInt *) shamem[t]->table[i];
                if (local_chunk == NULL) continue;
                
                #ifndef __i386__ // 64bit: 3 level lookup table...
                local_chunk = (UInt *) ((ILT*) local_chunk)->table[j];
                if (local_chunk == NULL) continue;
                #endif
            
                local[q++] = local_chunk;
                
            }
            
            #if COMPRESS_DEBUG
            if (q == 0 && global_chunk == NULL) {
                
                for(t = 0; t < q; t++)
                    APROF_(assert)(local[t] == NULL, "local chunk is not null")
                continue;
            }
            #endif
            
            #ifndef __i386__
            #if COMPRESS_DEBUG
            VG_(umsg)("Chunk: %u %u\n", i, j);
            #endif
            #endif
            
            UInt gts = 0; ts = 0;
            for (k = 0; k < APROF_(runtime).global_shadow_memory->flt_size; k++){
                
                if (global_chunk != NULL) {
                    
                    gts = TS(global_chunk[k]);
                    if (gts > 0) 
                        ts = binary_search(array, 0, size, gts);
                    else
                        ts = 0;
                    
                    #if COMPRESS_DEBUG
                    if (gts > 0) {
                        APROF_(fprintf)(f, "GTS[%u] => ACTIVE_TS[%u] = %u\n", gts, ts, array[ts]);
                        APROF_(fflush)(f);
                    }
                    #endif
                }
                
                #if COMPRESS_DEBUG 
                if (global_chunk == NULL)
                    APROF_(assert)(gts == 0 && ts == 0, "Invalid gts/ts");
                #endif
                
                // for each thread, access the relative chunk
                for(t = 0; t < q; t++){
                    
                    UInt * local_chunk = local[t];

                    if (local_chunk[k] < array[ts]) {
                        
                        UInt new = binary_search(array, 0, ts, local_chunk[k]);
                        
                        #if COMPRESS_DEBUG
                        APROF_(fprintf)(f, "[%d] TS: %u => %u ~ GTS: %u - [%u:%u:%u]\n", 
                            t, local_chunk[k], 3 * new, gts, i, j, k);
                        APROF_(fflush)(f);
                        #endif

                        local_chunk[k] = 3 * new;
                        
                    } else if (ts + 1 < size && local_chunk[k] >= array[ts + 1]) {
                        
                        UInt new = binary_search(array, ts, size, local_chunk[k]);
                        
                        #if COMPRESS_DEBUG
                        APROF_(fprintf)(f, "[%d] TS: %u => %u ~ GTS: %u - [%u:%u:%u]\n", 
                            t, local_chunk[k], 3 * new, gts, i, j, k);
                        APROF_(fflush)(f);
                        #endif

                        local_chunk[k] = 3 * new;
                        
                    } else if(local_chunk[k] < gts) {

                        #if COMPRESS_DEBUG
                        if (1 || gts > 0) {
                            APROF_(fprintf)(f, "[%d] TS: %u => %u ~ GTS: %u - [%u:%u:%u]\n", 
                                t, local_chunk[k], 3 * ts, gts, i, j, k);
                            APROF_(fflush)(f);
                        }
                        #endif

                        local_chunk[k] = 3 * ts;
                        
                    } else if(local_chunk[k] == gts) {
                        
                        #if COMPRESS_DEBUG
                        if (1 || gts > 0) {
                            APROF_(fprintf)(f, "[%d] TS: %u => %u ~ GTS: %u - [%u:%u:%u]\n", 
                                t, local_chunk[k], 3 * ts + 1, gts, i, j, k);
                            APROF_(fflush)(f);
                        }
                        #endif
                        
                        /* thread t wrote this value */
                        local_chunk[k] = 3 * ts + 1;

                    } else { // local_chunk[k] > gts
                        
                        #if COMPRESS_DEBUG
                        APROF_(fprintf)(f, "[%d] TS: %u => %u ~ GTS: %u (binary) - [%u:%u:%u]\n", 
                            t, local_chunk[k], 3 * ts + 2, gts, i, j, k);
                        APROF_(fflush)(f);
                        #endif
                        
                        /* 
                         * thread t reads this value so we have to reassign 
                         * the ts to the greater activation-ts that satisfies 
                         * wts[x] <= ats
                         */
                        local_chunk[k] = 3 * ts + 2;
                        
                    }
                }
  
                if (global_chunk != NULL) {
                    
                    #if COMPRESS_DEBUG
                    if (gts > 0) {
                        APROF_(fprintf)(f, "GTS: %u => %u\n", global_chunk[k], 3 * ts + 1);
                        APROF_(fflush)(f);
                    }
                    #endif
                    
                    #if INPUT_STATS
                    Bool source_io = False;
                    if (SYSCALL(global_chunk[k])) source_io = True;
                    #endif
                    
                    global_chunk[k] = 3 * ts + 1;
                    
                    #if INPUT_STATS
                    if (source_io) 
                        global_chunk[k] = SET_SYSCALL(global_chunk[k]);
                    #endif
                
                }
            }
        
        #ifndef __i386__    
        }
        #endif
        
    }
    
}

void LK_compress_rms(LookupTable * lk, UInt * arr_rid, UInt size_arr) {
    
    UInt i = 0;
    while (i < LK_SIZE) {
        
        #ifndef __i386__
        UInt q = 0;
        if (lk->table[i] != NULL) {
        
            q = 0;
            while (q < ILT_SIZE) {
                
                UInt * table = lk->table[i]->table[q]; 
        #else
                UInt * table = lk->table[i];
        #endif
                
                if (table != NULL) {

                    UInt j = 0;
                    UInt rid = 0;
                    for (j = 0; j < lk->flt_size; j++){
                        
                        rid = table[j];
                        if (rid == 0) continue;
                        int k = 0;
                        for (k = size_arr - 1; k >= 0; k--) {
                            
                            if (arr_rid[k] <= rid) {
                                /*
                                VG_(umsg)("%u=>%u [%u:%u:%u]\n", table[j], 
                                                k + 1, i, q, j);
                                */
                                table[j] = k + 1;
                                break;
                            }
                        }
                        
                        /*
                        VG_(umsg)("%u=>%u [%u:%u:%u]\n", table[j], 
                                                0, i, q, j);
                        */
                        /* 
                         * This means that this address was accessed by 
                         * an activation no more in stack, and all its
                         * ancestors are dead (for example we are dealing
                         * with an aid of a setup libc function
                         * invoked before main() )
                         */
                        if (k < 0) table[j] = 0;
                    }
                }
        #ifndef __i386__
            q++;
            }
        }
        #endif
        i++;
    }
}

