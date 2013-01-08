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

VG_REGPARM(3) void APROF_(trace_access)(UWord type, 
                                        Addr addr, 
                                        SizeT size,
                                        Bool kernel_access) {
    
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
    
    #if TRACE_FUNCTION
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
        
        Activation * act = APROF_(get_activation)(tdata, tdata->stack_depth);
        
        #if INPUT_METRIC == RVMS
        
        UInt ts;
        UInt old_ts;
        UInt wts;
        
        /*
         * We are writing a new input value. So, this is a new "version"
         * of the memory cell. Update its timestamp in the global shadow memory.
         */
         if (type == STORE || type == MODIFY) {
            
            ts = APROF_(global_counter);
            
            wts = LK_insert(APROF_(global_shadow_memory), addr, ts);

            if (kernel_access) 
                #if !COSTANT_MEM_ACCESS
                continue;
                #else
                return;
                #endif

        } else { 
            
            /*
             * This is a load. What is the latest "version" of this
             * memory cell? Get timestamp of the memory cell in
             * the global shadow memory.
             */
            
            ts = APROF_(global_counter);
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
            continue;
            #endif
        }

        if(old_ts < wts){
            act->rvms++;
        }
        
        else if (old_ts < act->aid) {
            
            act->rms++;
            act->rvms++;
            if (old_ts > 0 && old_ts >= APROF_(get_activation)(tdata, 1)->aid) {
                
                APROF_(get_activation_by_aid)(tdata, old_ts)->rvms--;
                APROF_(get_activation_by_aid)(tdata, old_ts)->rms--;
         
            }

        }
        
        #else
        
        UInt old_aid = LK_insert( tdata->accesses, addr, act->aid);
        
        if (old_aid < act->aid && (type == LOAD || type == MODIFY)) {
            
            act->rms++;
            if (old_aid > 0 && old_aid >= APROF_(get_activation)(tdata, 1)->aid) {
                
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
