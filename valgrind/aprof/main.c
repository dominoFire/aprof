/*
 * Setup of valgrind and instrumentation needed by aprof;
 * command line parser
 * 
 * Last changed: $Date$
 * Revision:     $Rev$
 */
 
 /*
   This file is part of aprof, an input sensitive profiler.

   Copyright (C) 2011-2012, Emilio Coppa (ercoppa@gmail.com),
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

const HChar * APROF_(log_dir) = NULL;

#if MEM_TRACE && IGNORE_REPEAT_ACC

#if !COSTANT_MEM_ACCESS
#error "IGNORE_REPEAT_ACC only works with COSTANT_MEM_ACCESS enabled"
#endif

#define INIT_SIZE_ADDR_ACC 32
static UInt  APROF_(addr_accessed_size) = INIT_SIZE_ADDR_ACC;
static IRExpr **  APROF_(addr_accessed) = NULL;

/*
 * Return true if the accessed memory address was not accessed previously
 * by the current BB. Linear search.
 */
static Bool APROF_(do_access)(IRExpr * e) {
    
    #if DEBUG
    AP_ASSERT(e != NULL, "Invalid IRExpr");
    AP_ASSERT(isIRAtom(e), "Invalid IRExpr");
    #endif
    
    int i = 0;
    for(; i < APROF_(addr_accessed_size); i++) {
        
        if (APROF_(addr_accessed)[i] == NULL) break;
        
        if (eqIRAtom(APROF_(addr_accessed)[i], e)) {
            return False;
        }
    
    }
    
    // resize the array
    if (i >= APROF_(addr_accessed_size)) {
        
        //VG_(printf)("Relocate addr_accessed\n");
        IRExpr ** addr_accessed2 = VG_(calloc)("addr acc", 
                                sizeof(IRExpr *), APROF_(addr_accessed_size)*2);
        int j = 0;
        for (; j < APROF_(addr_accessed_size); j++) 
            addr_accessed2[j] = APROF_(addr_accessed)[j];
        APROF_(addr_accessed_size) = APROF_(addr_accessed_size) * 2;
        VG_(free)(APROF_(addr_accessed));
        APROF_(addr_accessed) = addr_accessed2;
    }
    
    APROF_(addr_accessed)[i] = e;
    
    return True;
}

#endif

static
IRSB* APROF_(instrument) (  VgCallbackClosure* closure, 
                    IRSB* sbIn,
                    VexGuestLayout* layout, 
                    VexGuestExtents* vge,
                    VexArchInfo* archinfo_host,
                    IRType gWordTy, IRType hWordTy ) {

    Int        i;
    IRSB*      sbOut;
    
    #if MEM_TRACE
    IRTypeEnv* tyenv = sbIn->tyenv;
    #endif
    APROF_(events_used) = 0;

    if (gWordTy != hWordTy) /* We don't currently support this case. */
        VG_(tool_panic)("host/guest word size mismatch");

    #if MEM_TRACE && IGNORE_REPEAT_ACC
    APROF_(addr_accessed_size) = INIT_SIZE_ADDR_ACC;
    APROF_(addr_accessed) = VG_(calloc)("addr acc", 
                sizeof(IRExpr *), APROF_(addr_accessed_size));
    #endif
    
    #if TRACE_FUNCTION
    UInt instr_offset = 0;
    #endif

    /* Set up SB */
    sbOut = deepCopyIRSBExceptStmts(sbIn);

    // Copy verbatim any IR preamble preceding the first IMark
    i = 0;
    while (i < sbIn->stmts_used && sbIn->stmts[i]->tag != Ist_IMark) {
        addStmtToIRSB( sbOut, sbIn->stmts[i] );
        i++;
    }
    
    #if TRACE_FUNCTION
    
    BB * bb = APROF_(get_BB)(sbIn->stmts[i]->Ist.IMark.addr +
                                + (Addr)sbIn->stmts[i]->Ist.IMark.delta);
    #if DEBUG
    AP_ASSERT(bb != NULL, "Invalid BB")
    #endif

    IRExpr  * e3 = mkIRExpr_HWord ( (HWord) bb );
    IRExpr  * e2 = mkIRExpr_HWord ( (HWord) (Addr)sbIn->stmts[i]->Ist.IMark.addr
                                    + (Addr)sbIn->stmts[i]->Ist.IMark.delta);
    IRDirty * di3 = unsafeIRDirty_0_N( 2, "BB start",
                                VG_(fnptr_to_fnentry)( &APROF_(BB_start) ),
                                mkIRExprVec_2( e2, e3 ) );

    addStmtToIRSB( sbOut, IRStmt_Dirty(di3) );
    
    #endif
    
    #if TIME == BB_COUNT && !TRACE_FUNCTION
    IRDirty * di2 = unsafeIRDirty_0_N( 0, "add_one_guest_BB", 
    VG_(fnptr_to_fnentry)( &APROF_(add_one_guest_BB) ), 
                                mkIRExprVec_0() );
    addStmtToIRSB( sbOut, IRStmt_Dirty(di2) );
    #endif
   
    #if MEM_TRACE && IGNORE_REPEAT_ACC
    HChar * helperNameA;
    void * helperAddrA;
    IRExpr * * argvA;
    IRDirty * diA;
    #endif
   
    for (/*use current i*/; i < sbIn->stmts_used; i++) {
        
        IRStmt* st = sbIn->stmts[i];
        if (!st || st->tag == Ist_NoOp) continue;

        switch (st->tag) {

            case Ist_NoOp:
            case Ist_AbiHint:
            case Ist_Put:
            case Ist_PutI:
            case Ist_MBE: {
                addStmtToIRSB( sbOut, st );
                break;
            }

            case Ist_IMark: {
                
                #if TIME == INSTR
                di = unsafeIRDirty_0_N( 0, "add_one_guest_instr",
                                        VG_(fnptr_to_fnentry)( &APROF_(add_one_guest_instr) ), 
                                        mkIRExprVec_0() );
                addStmtToIRSB( sbOut, IRStmt_Dirty(di) );
                #endif

                #if MEM_TRACE
                
                #if IGNORE_REPEAT_ACC
                /*
                APROF_(do_access)(mkIRExpr_HWord( (HWord)st->Ist.IMark.addr + 
                                            (Addr)st->Ist.IMark.delta));
                */
                #else
                APROF_(addEvent_Ir)( sbOut, mkIRExpr_HWord( (HWord)st->Ist.IMark.addr +
                                                        (Addr)st->Ist.IMark.delta),
                                st->Ist.IMark.len );
                #endif
                
                #endif
                
                #if TRACE_FUNCTION
                if (st->Ist.IMark.len == 0)
                    instr_offset += VG_MIN_INSTR_SZB;
                else
                    instr_offset += st->Ist.IMark.len;
                #endif

                addStmtToIRSB( sbOut, st );
                break;
            }

            case Ist_WrTmp:{

                #if MEM_TRACE
                IRExpr* data = st->Ist.WrTmp.data;
                if (data->tag == Iex_Load) {
                    
                    #if IGNORE_REPEAT_ACC
                    IRExpr * a = data->Iex.Load.addr;
                    if (APROF_(do_access)(a)) {
                        helperNameA = "trace_load";
                        argvA = mkIRExprVec_4(  mkIRExpr_HWord(LOAD),
                                                a, 
                                                mkIRExpr_HWord( sizeofIRType(data->Iex.Load.ty) ),
                                                mkIRExpr_HWord(False)
                                            );
                        helperAddrA = APROF_(trace_access);
                        diA = unsafeIRDirty_0_N( 3, helperNameA, 
                                    VG_(fnptr_to_fnentry)( helperAddrA ),
                                    argvA );
                        addStmtToIRSB( sbOut, IRStmt_Dirty(diA) );
                    }
                    #else
                    APROF_(addEvent_Dr)( sbOut, data->Iex.Load.addr,
                                    sizeofIRType(data->Iex.Load.ty) );
                    #endif
                }
                #endif

                addStmtToIRSB( sbOut, st );
                break;
            }

            case Ist_Store: {
                
                #if MEM_TRACE
                IRExpr * data  = st->Ist.Store.data;
                
                #if IGNORE_REPEAT_ACC
                IRExpr * a = st->Ist.Store.addr;
                if (APROF_(do_access)(a)) {
                    helperNameA = "trace_store";
                    argvA = mkIRExprVec_4(  mkIRExpr_HWord(STORE),
                                            a, 
                                            mkIRExpr_HWord( sizeofIRType(typeOfIRExpr(tyenv, data)) ),
                                            mkIRExpr_HWord(False)
                                        );
                    helperAddrA = APROF_(trace_access);
                    diA = unsafeIRDirty_0_N( 3, helperNameA, 
                                VG_(fnptr_to_fnentry)( helperAddrA ),
                                argvA );
                    addStmtToIRSB( sbOut, IRStmt_Dirty(diA) );
                }
                #else
                APROF_(addEvent_Dw)(sbOut, st->Ist.Store.addr,
                            sizeofIRType(typeOfIRExpr(tyenv, data)) );
                #endif
                #endif

                addStmtToIRSB( sbOut, st );
                break;
            }

            case Ist_Dirty: {

                #if MEM_TRACE
                Int dsize;
                IRDirty* d = st->Ist.Dirty.details;
                if (d->mFx != Ifx_None) {
                    
                    // This dirty helper accesses memory.  Collect the details.
                    tl_assert(d->mAddr != NULL);
                    tl_assert(d->mSize != 0);
                    dsize = d->mSize;
                    
                    #if IGNORE_REPEAT_ACC
                    IRExpr * a = d->mAddr;
                    if (d->mFx == Ifx_Read || d->mFx == Ifx_Modify) {
                        if (APROF_(do_access)(a)) {
                            helperNameA = "trace_load";
                            argvA = mkIRExprVec_4(    mkIRExpr_HWord(LOAD),
                                                        a, 
                                                        mkIRExpr_HWord( dsize ),
                                                        mkIRExpr_HWord(False)
                                                    );
                            helperAddrA = APROF_(trace_access);
                            diA = unsafeIRDirty_0_N( 3, helperNameA, 
                                        VG_(fnptr_to_fnentry)( helperAddrA ),
                                        argvA );
                            addStmtToIRSB( sbOut, IRStmt_Dirty(diA) );
                        }
                    }
                    else if (d->mFx == Ifx_Write) {
                        if (APROF_(do_access)(a)) {
                            helperNameA = "trace_store";
                            argvA = mkIRExprVec_4(    mkIRExpr_HWord(STORE),
                                                        a, 
                                                        mkIRExpr_HWord( dsize ),
                                                        mkIRExpr_HWord(False)
                                                    );
                            helperAddrA = APROF_(trace_access);
                            diA = unsafeIRDirty_0_N( 3, helperNameA, 
                                        VG_(fnptr_to_fnentry)( helperAddrA ),
                                        argvA );
                            addStmtToIRSB( sbOut, IRStmt_Dirty(diA) );
                        }
                    }
                    #else
                    if (d->mFx == Ifx_Read || d->mFx == Ifx_Modify) {
                        APROF_(addEvent_Dr)( sbOut, d->mAddr, dsize );
                    }
                    if (d->mFx == Ifx_Write || d->mFx == Ifx_Modify) {
                        APROF_(addEvent_Dw)( sbOut, d->mAddr, dsize );
                    }
                    #endif
                } else {
                    tl_assert(d->mAddr == NULL);
                    tl_assert(d->mSize == 0);
                }
                #endif

                addStmtToIRSB( sbOut, st );
                break;
            }

            case Ist_CAS: {
                
                #if MEM_TRACE
                /* We treat it as a read and a write of the location.  I
                think that is the same behaviour as it was before IRCAS
                was introduced, since prior to that point, the Vex
                front ends would translate a lock-prefixed instruction
                into a (normal) read followed by a (normal) write. */
                Int    dataSize;
                IRType dataTy;
                IRCAS* cas = st->Ist.CAS.details;
                tl_assert(cas->addr != NULL);
                tl_assert(cas->dataLo != NULL);
                dataTy   = typeOfIRExpr(tyenv, cas->dataLo);
                dataSize = sizeofIRType(dataTy);
                if (cas->dataHi != NULL)
                    dataSize *= 2; /* since it's a doubleword-CAS */
                
                #if IGNORE_REPEAT_ACC
                IRExpr * a = cas->addr;
                if (APROF_(do_access)(a)) {
                    helperNameA = "trace_load";
                    argvA = mkIRExprVec_4(  mkIRExpr_HWord(LOAD),
                                            a, 
                                            mkIRExpr_HWord( dataSize ),
                                            mkIRExpr_HWord(False)
                                        );
                    helperAddrA = APROF_(trace_access);
                    diA = unsafeIRDirty_0_N( 3, helperNameA, 
                                VG_(fnptr_to_fnentry)( helperAddrA ),
                                argvA );
                    addStmtToIRSB( sbOut, IRStmt_Dirty(diA) );
                }
                #else
                APROF_(addEvent_Dr)( sbOut, cas->addr, dataSize );
                APROF_(addEvent_Dw)( sbOut, cas->addr, dataSize );
                #endif
                
                #endif

                addStmtToIRSB( sbOut, st );
                break;
            }

            case Ist_LLSC: {
                
                #if MEM_TRACE
                IRType dataTy;
                if (st->Ist.LLSC.storedata == NULL) {
                    /* LL */
                    dataTy = typeOfIRTemp(tyenv, st->Ist.LLSC.result);
                    
                    #if IGNORE_REPEAT_ACC
                    IRExpr * a = st->Ist.LLSC.addr;
                    if (APROF_(do_access)(a)) {
                        helperNameA = "trace_load";
                        argvA = mkIRExprVec_4(  mkIRExpr_HWord(LOAD),
                                                a, 
                                                mkIRExpr_HWord( sizeofIRType(dataTy) ),
                                                mkIRExpr_HWord(False)
                                            );
                        helperAddrA = APROF_(trace_access);
                        diA = unsafeIRDirty_0_N( 3, helperNameA, 
                                    VG_(fnptr_to_fnentry)( helperAddrA ),
                                    argvA );
                        addStmtToIRSB( sbOut, IRStmt_Dirty(diA) );
                    }
                    
                    #else
                    APROF_(addEvent_Dr)( sbOut, st->Ist.LLSC.addr, sizeofIRType(dataTy) );
                    #endif
                    
                } else {
                    /* SC */
                    dataTy = typeOfIRExpr(tyenv, st->Ist.LLSC.storedata);
                    #if IGNORE_REPEAT_ACC
                    IRExpr * a = st->Ist.LLSC.addr;
                    if (APROF_(do_access)(a)) {
                        helperNameA = "trace_store";
                        argvA = mkIRExprVec_4(  mkIRExpr_HWord(STORE),
                                                a, 
                                                mkIRExpr_HWord( sizeofIRType(dataTy) ),
                                                mkIRExpr_HWord(False)
                                            );
                        helperAddrA = APROF_(trace_access);
                        diA = unsafeIRDirty_0_N( 3, helperNameA, 
                                    VG_(fnptr_to_fnentry)( helperAddrA ),
                                    argvA );
                        addStmtToIRSB( sbOut, IRStmt_Dirty(diA) );
                    }
                    #else
                    APROF_(addEvent_Dw)( sbOut, st->Ist.LLSC.addr, sizeofIRType(dataTy) );
                    #endif
                }
                #endif
                
                addStmtToIRSB( sbOut, st );
                break;
            }

            case Ist_Exit: {
                #if MEM_TRACE && !IGNORE_REPEAT_ACC
                APROF_(flushEvents)(sbOut);
                #endif
                addStmtToIRSB( sbOut, st );      // Original statement
                break;
            }

            default:
                tl_assert(0);
        }
    }

    #if TRACE_FUNCTION
    
    if (sbIn->jumpkind == Ijk_Call) bb->exit = BBCALL;
    else if (sbIn->jumpkind == Ijk_Ret) bb->exit = BBRET;
    else bb->exit = BBOTHER;
    
    /* 
     * Set a global var last_exit (saved/restored when there is
     * a switch btw thread) to the type of final jump for this BB
     */
    addStmtToIRSB( sbOut, IRStmt_Store(Endness, 
                    IRExpr_Const(
                                    hWordTy == Ity_I32 ?
                                    IRConst_U32( (UWord) &APROF_(last_exit) ) :
                                    IRConst_U64( (UWord) &APROF_(last_exit) )
                                ),
                    IRExpr_Const(IRConst_U32(bb->exit))
                    )
                );
    
    bb->instr_offset = instr_offset;
    
    #endif

    #if MEM_TRACE
    
    #if !IGNORE_REPEAT_ACC
    /* At the end of the sbIn.  Flush outstandings. */
    APROF_(flushEvents)(sbOut);
    #else
    VG_(free)(APROF_(addr_accessed));
    APROF_(addr_accessed_size) = INIT_SIZE_ADDR_ACC;
    #endif
    
    #endif

    return sbOut;
}

/* aprof initialization */
static void APROF_(post_clo_init)(void) {
    
    #if TRACE_FUNCTION
    APROF_(bb_ht) = HT_construct(VG_(free));
    #endif
    
    APROF_(fn_ht) = HT_construct(NULL);    
    APROF_(obj_ht) = HT_construct(NULL);
    
    #if INPUT_METRIC == RVMS
    APROF_(global_shadow_memory) = LK_create();
    #endif
    
    VG_(clo_vex_control).iropt_unroll_thresh = 0;
    VG_(clo_vex_control).guest_chase_thresh  = 0;
}

#if MEM_USAGE_INFO 

#define PROC_SIZE 1024*5
void APROF_(print_info_mem_usage)(void) {
    
    //VG_(umsg)("Getting info about memory\n");
    
    HChar buf[PROC_SIZE];
    VG_(sprintf)(buf, "/proc/%d/status", VG_(gettid)());
    SysRes res = VG_(open)(buf, VKI_O_RDONLY, 0);
    Int file = (Int) sr_Res(res);
    if (file < 0) {
        VG_(umsg)("No info about memory usage [1]\n");
        return;
    }
    
    UInt rb = 0;
    Int ret;
    while (rb < PROC_SIZE) {
        ret = VG_(read)(file, buf + rb, PROC_SIZE - rb);
        if (ret > 0) rb = ret;
        else break;
    }
    
    VG_(close)(file);
    
    ret = 0;
    HChar * sentinel = "VmPeak:	";
    UInt sent_pos = 0;
    UInt sent_len = VG_(strlen)(sentinel);
    while (rb > 0) {

        //VG_(printf)("%c", buf[ret]);
        rb--; 
        
        if (buf[ret++] == sentinel[sent_pos]) {
            
            sent_pos++;
            if (sent_pos == sent_len) break;
                
        } else if (sent_pos > 0)
            sent_pos = 0;

    }
    
    if (sent_pos == sent_len) {
        VG_(printf)("==%d== Memory used by aprof:", VG_(getpid)());
        while (rb > 0) {
            
            VG_(printf)("%c", buf[ret]);
            if (buf[ret++] == '\n') break;
            rb--;
        
        }
    } else
        VG_(umsg)("No info about memory usage [2]\n");
}

#endif

/* aprof finalization */
static void APROF_(fini)(Int exitcode) {
    
    #if DEBUG_ALLOCATION
    //VG_(printf)("Before clean:\n");
    //APROF_(print_alloc)();
    #endif
    
    APROF_(kill_threads)();
    
    #if TRACE_FUNCTION 
        
    #if DEBUG_ALLOCATION
    UInt j = 0;
    for (j = 0; j < HT_count_nodes(APROF_(bb_ht)); j++)
        APROF_(remove_alloc)(BB_S);
    #endif
    
    HT_destruct(APROF_(bb_ht));
    #endif
    
    // destroy function objs
    HT_ResetIter(APROF_(fn_ht));
    Function * f = HT_RemoveNext(APROF_(fn_ht));
    while (f != NULL) {
        
        #if DEBUG_ALLOCATION
        APROF_(remove_alloc)(FN_NAME_S);
        if (f->mangled) APROF_(remove_alloc)(MANGLED_S);
        APROF_(remove_alloc)(FN_S);
        #endif
        
        VG_(free)(f->name);
        VG_(free)(f->mangled);
        VG_(free)(f);
        f = HT_RemoveNext(APROF_(fn_ht));
        
    }
    HT_destruct(APROF_(fn_ht));
    
    // destroy ELF objects
    HT_ResetIter(APROF_(obj_ht));
    Object * o = HT_RemoveNext(APROF_(obj_ht));
    while (o != NULL) {
        
        VG_(free)(o->name);
        VG_(free)(o);
        o = HT_RemoveNext(APROF_(obj_ht));
    
        #if DEBUG_ALLOCATION
        APROF_(remove_alloc)(OBJ_NAME_S);
        APROF_(remove_alloc)(OBJ_S);
        #endif
    }
    HT_destruct(APROF_(obj_ht));
    
    #if INPUT_METRIC == RVMS
    LK_destroy(APROF_(global_shadow_memory));
    #endif
    
    #if DEBUG_ALLOCATION
    VG_(printf)("\nAfter clean:\n");
    APROF_(print_alloc)();
    #endif

}

#if 0
/*
 * I don't have yet tested what happens when a singnal
 * is received wrt shadow stack 
 */
static void APROF_(signal)(ThreadId tid, Int sigNo, Bool alt_stack) {

    AP_ASSERT(0, "There is a signal");

}
#endif

static Bool APROF_(cmd_line)(const HChar* argv) {
    
    int value = 0;
    const HChar * val_s;
    
    if VG_INT_CLO(argv, "--memory-resolution", value) {
        APROF_(addr_multiple) = value;
    }

    else if VG_STR_CLO(argv, "--log-dir", val_s) {
        APROF_(log_dir) = val_s;
    }
    
    return True;
}

static void APROF_(print_usage)(void) {
    
    VG_(printf)(
        "    --memory-resolution=<n>        Memory resolution of the shadow memory {1, 2, 4, 8, 16} [4]\n"
        "    --log-dir=<PATH>               Report will be saved in this directory\n"
    );
}

/* Valgrind init */
static void APROF_(pre_clo_init)(void) {

    VG_(details_name)             ("aprof");
    VG_(details_version)          ("0.2_alpha");
    VG_(details_description)      ("Input-sensitive Profiler - http://code.google.com/p/aprof/");
    VG_(details_copyright_author) ("by Emilio Coppa, Camil Demetrescu, Irene Finocchi, Romolo Marotta");
    VG_(details_bug_reports_to)   ("ercoppa@gmail.com");

    VG_(basic_tool_funcs)           (    
                                        APROF_(post_clo_init), 
                                        APROF_(instrument), 
                                        APROF_(fini)
                                    );
    
    VG_(needs_command_line_options) (
                                        APROF_(cmd_line), 
                                        APROF_(print_usage),
                                        NULL
                                    );
    
    #if !TRACE_FUNCTION
    VG_(needs_client_requests)      (APROF_(trace_function));
    #endif
    
    #if INPUT_METRIC == RVMS
    VG_(needs_syscall_wrapper)      (
                                        APROF_(pre_syscall), 
                                        APROF_(post_syscall)
                                    );
    #endif
    
    VG_(track_start_client_code)    (APROF_(thread_switch));
    VG_(track_pre_thread_ll_exit)   (APROF_(thread_exit));
    //VG_(track_pre_deliver_signal)   (APROF_(signal));
    
    VG_(clo_vex_control).iropt_unroll_thresh = 0;
    VG_(clo_vex_control).guest_chase_thresh  = 0;
}

VG_DETERMINE_INTERFACE_VERSION(APROF_(pre_clo_init))