// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

#if !defined(TESTSUITE_BUILD)
  TO_QSPI static const char bugScreenIdMustNotBe0[] = "In function showSoftmenu: id must not be 0!";
#endif //TESTSUITE_BUILD


/* The numbers refer to the index of items in items.c
 *         item <     0  ==>  sub menu
 *     0 < item <  9999  ==>  item with top and bottom line
 * 10000 < item < 19999  ==>  item without top line
 * 20000 < item < 29999  ==>  item without bottom line
 * 30000 < item < 39999  ==>  item without top and bottom line
 */

/*      Menu name                           <----------------------------------------------------------------------------- 6 functions ---------------------------------------------------------------------------->  */
/*                                          <---------------------------------------------------------------------- 6 f shifted functions ------------------------------------------------------------------------->  */
/*                                          <---------------------------------------------------------------------- 6 g shifted functions ------------------------------------------------------------------------->  */
TO_QSPI const int16_t menu_BITS[]        = { ITM_LOGICALAND,                ITM_LOGICALOR,              ITM_LOGICALXOR,           ITM_LOGICALNOT,        ITM_MASKL,                   ITM_MASKR,
                                             ITM_LOGICALNAND,               ITM_LOGICALNOR,             ITM_LOGICALXNOR,          ITM_NUMB,              ITM_ZIP,                     ITM_UNZIP,
                                             ITM_SB,                        ITM_BS,                     ITM_FB,                   ITM_BC,                ITM_CB,                      ITM_FF,

                                             ITM_SL1,                       ITM_SR1,                    ITM_RL1,                  ITM_RR1,               ITM_RLC1,                    ITM_RRC1,
                                             ITM_SL,                        ITM_SR,                     ITM_RL,                   ITM_RR,                ITM_RLC,                     ITM_RRC,
                                             ITM_MIRROR,                    ITM_ASR,                    ITM_LJ,                   ITM_RJ,               -MNU_BITSET,                  ITM_FF                        };

TO_QSPI const int16_t menu_CLK[]         = { ITM_DATE,                      ITM_DtoJ,                   ITM_TIME,                 ITM_DTtoJ,             ITM_JtoDT,                   ITM_TIMER,
                                             ITM_DATEto,                    ITM_ymdTo,                  ITM_TIMEto,               ITM_msTo,              ITM_dotD,                    ITM_LEAPQ,
                                             ITM_toDATE,                    ITM_XtoDATE,                ITM_toTIME,               ITM_HMStoTM,           ITM_HRtoTM,                  ITM_YY_DFLT,

                                             ITM_CLK12,                     ITM_CLK24,                  ITM_TDISP,                ITM_DMY,               ITM_MDY,                     ITM_YMD,
                                             ITM_SETDAT,                    ITM_SETTIM,                 ITM_WDAY,                 ITM_DAY,               ITM_MONTH,                   ITM_YEAR,
                                             ITM_DATE,                      ITM_TIME,                   ITM_WOY,                  ITM_SECOND,            ITM_MINUTE,                  ITM_HR_DEG,

                                             ITM_GET_JUL_GREG,              ITM_JUL_GREG,               ITM_JUL_GREG_1582,        ITM_JUL_GREG_1752,     ITM_JUL_GREG_1873,           ITM_JUL_GREG_1949,
                                             ITM_GET_WOY,                   ITM_NULL,                   ITM_NULL,                 ITM_WOY_ISO,           ITM_WOY_US,                  ITM_WOY_ME                    };


TO_QSPI const int16_t menu_CLR[]         = { ITM_CF,                        ITM_CLMENU,                 ITM_CLCVAR,               ITM_CLREGS,            ITM_CLX,                     ITM_CLSTK,
                                             ITM_CLFALL,                    ITM_CLMALL,                 ITM_CLVALL,               ITM_CLSIGMA,           ITM_CLGRF,                   ITM_CLLCD,
                                             ITM_RESET,                     ITM_NULL,                   ITM_CLTVM,                ITM_NULL,              ITM_CLRMOD,                  -MNU_DELETE                    };

/*      Menu name                           <----------------------------------------------------------------------------- 6 functions ---------------------------------------------------------------------------->  */
/*                                          <---------------------------------------------------------------------- 6 f shifted functions ------------------------------------------------------------------------->  */
/*                                          <---------------------------------------------------------------------- 6 g shifted functions ------------------------------------------------------------------------->  */
TO_QSPI const int16_t menu_CPX[]         = { ITM_RE,                        ITM_IM,                     ITM_CONJ,                 ITM_REexIM,            ITM_op_j,                    ITM_op_j_pol,                       //JM re-arranged menu. CPX menu
                                             KEY_COMPLEX,                   ITM_UNITV,                  ITM_MAGNITUDE,            ITM_ARG,               ITM_DOT_PROD,                ITM_CROSS_PROD,                     //JM re-arranged menu. CPX menu
                                             ITM_toREC2,                    ITM_toPOL2,                 ITM_CXtoRE,               ITM_REtoCX,            ITM_RECT,                    ITM_POLAR                     };    //JM re-arranged menu

TO_QSPI const int16_t menu_DISP[]        = { ITM_FIX,                       ITM_SCI,                    ITM_ENG,                  ITM_UNIT,              ITM_SIGFIG,                  ITM_ALL,
                                             ITM_FRACT,                     ITM_IRFRAC,                 ITM_PROPFR,               ITM_DENMAX2,           ITM_DENANY,                  ITM_DENFIX,
                                             ITM_GAP_L,                     ITM_GAP_RX,                 ITM_GAP_R,                ITM_SETFDIGS,          ITM_FRCYC,                   ITM_TDISP,

                                             ITM_LARGELI,                   ITM_DREAL,                  ITM_DSTACK,               ITM_SHOIREP,           ITM_BASENR,                  ITM_CLKp2,
                                             ITM_CPXI,                      ITM_CPXJ,                   ITM_NULL,                 ITM_CPXMULT,           ITM_MULTCR,                  ITM_MULTDOT,
                                             ITM_SCIOVR,                    ITM_ENGOVR,                 ITM_NULL,                 ITM_NULL,              ITM_RNG,                     ITM_HIDE,

                                             ITM_SETCHN,                    ITM_SETEUR,                 ITM_SETIND,               ITM_SETJPN,            ITM_SETUK,                   ITM_SETUSA,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_SETDFLT,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL                        };

#if !defined(SAVE_SPACE_DM42_24_PROFILES)
  #if !defined(SAVE_SPACE_DM42_21_HP35)
    TO_QSPI const int16_t menu_Dev[]     = { ITM_SetHP35,                   ITM_SetC47,                 ITM_SetJM,                ITM_SetRJ,             ITM_NULL,                    ITM_NULL                        };
  #else
    TO_QSPI const int16_t menu_Dev[]     = { ITM_NULL,                      ITM_SetC47,                 ITM_SetJM,                ITM_SetRJ,             ITM_NULL,                    ITM_NULL                        };
  #endif
#else
  TO_QSPI const int16_t menu_Dev[]       = { ITM_NULL,                      ITM_SetC47,                 ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL                        };
#endif

TO_QSPI const int16_t menu_EXP[]         = { ITM_CUBE,                      ITM_YX,                     ITM_SQRT1PX2,             ITM_LOG2,              ITM_LN1X,                    ITM_LOGXY,                          //JM re-arranged menu. logxy and square to follow DM42 keyboard. Re-aligned with 42S keys.
                                             ITM_CUBEROOT,                  ITM_XTHROOT,                ITM_SQUAREROOTX,          ITM_2X,                ITM_EX1,                     ITM_EXP,                           //JM re-arranged menu. Added YˆX to follow DM42 keyboard. Swapped YˆX and Yˆ(1/X). Re-aligned with 42S keys.
                                             ITM_sinh,                      ITM_cosh,                   ITM_tanh,                 ITM_arsinh,            ITM_arcosh,                  ITM_artanh                    };

TO_QSPI const int16_t menu_TRI[]         = { ITM_DEG2,                      ITM_RAD2,                   ITM_GRAD2,                ITM_sin,               ITM_cos,                     ITM_tan,
                                             ITM_sinc,                      ITM_sincpi,                 ITM_atan2,                ITM_arcsin,            ITM_arccos,                  ITM_arctan,                         //JM re-arranged menu TRIG menu
                                             ITM_sinh,                      ITM_cosh,                   ITM_tanh,                 ITM_arsinh,            ITM_arcosh,                  ITM_artanh                    };    //JM re-arranged menu TRIG menu

TO_QSPI const int16_t menu_TRG_C47[]     = { ITM_DEG2,                      ITM_RAD2,                   ITM_GRAD2,                ITM_DMS2,              ITM_MULPI2,                  ITM_DRG,
                                             ITM_sinc,                      ITM_sincpi,                 ITM_ms,                   ITM_msTo,              ITM_toREC2,                  ITM_toPOL2,
                                             ITM_DEG,                       ITM_RAD,                    ITM_GRAD,                 ITM_DMS,               ITM_MULPI,                   ITM_atan2,                    };

TO_QSPI const int16_t menu_TRG_C47_MORE[]= { ITM_DEG2,                      ITM_RAD2,                   ITM_GRAD2,                ITM_sin,               ITM_cos,                     ITM_tan,
                                             ITM_sinc,                      ITM_sincpi,                 ITM_atan2,                ITM_arcsin,            ITM_arccos,                  ITM_arctan,                         //JM re-arranged menu TRIG menu
                                             ITM_sinh,                      ITM_cosh,                   ITM_tanh,                 ITM_arsinh,            ITM_arcosh,                  ITM_artanh                    };    //JM re-arranged menu TRIG menu
//R47 vv
TO_QSPI const int16_t menu_TRG[]         = { ITM_DEG,                       ITM_RAD,                    ITM_GRAD,                 ITM_DMS,               ITM_MULPI,                   ITM_atan2,
                                             ITM_DEG2,                      ITM_RAD2,                   ITM_GRAD2,                ITM_DMS2,              ITM_MULPI2,                  ITM_NULL,
                                             ITM_toREC2,                    ITM_toPOL2,                 ITM_ms,                   ITM_msTo,              ITM_sinc,                    ITM_sincpi,                   };
//D47 ^^

TO_QSPI const int16_t menu_FIN[]         = { ITM_SIGMAPLUS ,                ITM_PCT    ,                ITM_PC         ,          ITM_DELTAPC,            ITM_PCPMG,                   ITM_PCMRR,
                                             ITM_SIGMAMINUS,                ITM_SIGMAx ,                ITM_NSIGMA     ,          ITM_XMEAN,              ITM_NULL,                    ITM_NULL,
                                             ITM_CLSIGMA   ,                ITM_PCSIGMA,                ITM_PCSGM_DPCMN,          ITM_DPCMEAN,            ITM_NULL  ,                  -MNU_TVM                     };

/*      Menu name                           <----------------------------------------------------------------------------- 6 functions ---------------------------------------------------------------------------->  */
/*                                          <---------------------------------------------------------------------- 6 f shifted functions ------------------------------------------------------------------------->  */
/*                                          <---------------------------------------------------------------------- 6 g shifted functions ------------------------------------------------------------------------->  */

TO_QSPI const int16_t menu_TVM[]         = { VAR_NPPER,                     VAR_IPonA,                  VAR_PV,                   VAR_PMT,               VAR_FV,                      VAR_PPERonA,
                                             ITM_CLTVM,                     ITM_EFFTOI,                 ITM_ITOEFF,               ITM_NULL,              ITM_NULL,                    VAR_CPERonA,
                                             ITM_BEGINP,                    ITM_ENDP,                   ITM_SETSIG2,              ITM_NULL,              ITM_NULL,                    -MNU_CASHFL                  };

TO_QSPI const int16_t menu_FLAGS[]       = { ITM_SF,                        ITM_FS,                     ITM_FF,                   ITM_STATUS,            ITM_FC,                      ITM_CF,
                                             ITM_FSS,                       ITM_FSC,                    ITM_FSF,                  ITM_FCF,               ITM_FCS,                     ITM_FCC,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_CLFALL                    };

TO_QSPI const int16_t menu_INFO[]        = { ITM_VERS,                      ITM_LASTERR,                ITM_LASTT,                ITM_KTYP,              ITM_LocRQ,                   ITM_MEM,
                                             ITM_WHO,                       ITM_BATT,                   ITM_DISK,                 ITM_VOLQ,              ITM_MENUQ,                   ITM_LOADEDFILE,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,

                                             ITM_TYPEQ,                     ITM_M_DIMQ,                 ITM_NEIGHB,               ITM_ULP,               ITM_SSIZE,                   ITM_RMODEQ,
                                             ITM_GETRANGE,                  ITM_GETHIDE,                ITM_GETSDIGS,             ITM_GETFDIGS,          ITM_GETDMX,                  ITM_WSIZEQ,
                                             ITM_GET_JUL_GREG,              ITM_GET_WOY,                ITM_NULL,                 ITM_NULL,              ITM_BESTFQ,                  ITM_ISM                         };

TO_QSPI const int16_t menu_INTS[]        = { ITM_A,                         ITM_B,                      ITM_C,                    ITM_D,                 ITM_E,                       ITM_F,
                                             ITM_IDIV,                      ITM_RMD,                    ITM_MOD,                  ITM_XMOD,              ITM_LINT,                    ITM_LCM,
                                             ITM_DBLDIV,                    ITM_DBLR,                   ITM_DBLMULT,              ITM_PMOD,              ITM_SINT,                    ITM_GCD                       };


TO_QSPI const int16_t menu_LOOP[]        = { ITM_DSE,                       ITM_DSZ,                    ITM_DSL,                  ITM_ISE,               ITM_ISZ,                     ITM_ISG,
                                             ITM_DECR,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_INC                       };

/*      Menu name                           <----------------------------------------------------------------------------- 6 functions ---------------------------------------------------------------------------->  */
/*                                          <---------------------------------------------------------------------- 6 f shifted functions ------------------------------------------------------------------------->  */
/*                                          <---------------------------------------------------------------------- 6 g shifted functions ------------------------------------------------------------------------->  */
TO_QSPI const int16_t menu_MATX[]        = {
                                             ITM_M_NEW,                     ITM_M_INV,                  ITM_M_DET,                ITM_M_TRANSP,          ITM_SIM_EQ,                  ITM_M_EDI,
                                             ITM_ENORM,                     ITM_UNITV,                  ITM_RSUM,                 ITM_RNORM,             ITM_DOT_PROD,                ITM_CROSS_PROD,
                                             ITM_EIGVAL,                    ITM_EIGVEC,                 ITM_STOVEL,               ITM_RCLVEL,            ITM_M_LU,                    ITM_M_QR,

                                             ITM_IPLUS,                     ITM_IMINUS,                 ITM_STOIJ,                ITM_RCLIJ,             ITM_JMINUS,                  ITM_JPLUS,
                                             ITM_M_RR,                      ITM_M_DIM_GR,               ITM_M_DIM,                ITM_M_DIMQ,            ITM_INDEX,                   ITM_M_EDIN,
                                             ITM_M_PUT,                     ITM_M_GET,                  ITM_STOEL,                ITM_RCLEL,             ITM_STOELPLUS,               ITM_RCLELPLUS,

                                             ITM_toREC2,                    ITM_toPOL2,                 ITM_CPXexV,               ITM_stkexV2,          -MNU_VECCONV,                 ITM_CLSTK,
                                             ITM_ENORM,                     ITM_UNITV,                  ITM_VVDIST,               ITM_VANGLE,            ITM_DOT_PROD,                ITM_CROSS_PROD,
                                             ITM_DEG2,                      ITM_RAD2,                   ITM_MULPI2,               ITM_DRG,               ITM_V10,                     ITM_V01,

                                             ITM_toREC2,                    ITM_V3toSPH,                ITM_V3toCYL,              ITM_stkexV3,          -MNU_VECCONV,                 ITM_CLSTK,
                                             ITM_ENORM,                     ITM_UNITV,                  ITM_VVDIST,               ITM_VANGLE,            ITM_DOT_PROD,                ITM_CROSS_PROD,
                                             ITM_DEG2,                      ITM_RAD2,                   ITM_MULPI2,               ITM_V100,              ITM_V010,                    ITM_V001                      };


TO_QSPI const int16_t menu_VECCONV[]     = { ITM_STKtoV2,                   ITM_V2toSTK,                ITM_CPXtoV,               ITM_VtoCPX,            ITM_STKtoV3,                 ITM_V3toSTK,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_REGtoVEC,                ITM_VECtoREG,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL /*_VECtoSTK*/, ITM_NULL,                    ITM_NULL                      };


TO_QSPI const int16_t menu_M_SIM_Q[]     = { VAR_MATA,                      VAR_MATB,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_MATX                      }; // Should VAR_MATA and VAR_MATB be reclaced by ITM_MATA (to be created) and ITM_MATB (to be created) here?


TO_QSPI const int16_t menu_M_EDIT[]      = { ITM_M_ADDR,                    ITM_M_ADDC,                 ITM_op_j,                 ITM_M_GOTO,            ITM_LEFT_ARROW,              ITM_RIGHT_ARROW,                  //DL
                                             ITM_M_INSR,                    ITM_M_INSC,                 ITM_op_j_pol,             ITM_M_OLD,             ITM_UP_ARROW,                ITM_DOWN_ARROW,
                                             ITM_M_DELR,                    ITM_M_DELC,                 ITM_NULL,                 ITM_NULL,              ITM_M_WRAP,                  ITM_M_GROW                    };


#if defined(INLINE_TEST) && defined(DMCP_BUILD)
  #define ITM_TST -MNU_INL_TST
#else
  #define ITM_TST ITM_RESERVE2
#endif



TO_QSPI const int16_t menu_MODE[]        = { ITM_DEG,                       ITM_RAD,                    ITM_GRAD,                 ITM_SETSIG2,           ITM_RECT,                    ITM_POLAR,
                                             ITM_SYSTEM2,                   ITM_ACTUSB,                 ITM_NULL,                 ITM_ERPN,              ITM_HPRP,                    ITM_CFG,              //JM
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,                                       //JM

                                             ITM_SSIZE4,                    ITM_SSIZE8,                 ITM_CB_CPXRES,            ITM_CB_SPCRES,         ITM_RECT,                    ITM_POLAR,
                                             ITM_INP_DEF_43S,               ITM_INP_DEF_DP,             ITM_INP_DEF_CPXDP,        ITM_INP_DEF_LI,        ITM_RMODE,                   ITM_CFG,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,

                                             ITM_SAFERESET,                 ITM_G_DOUBLETAP,            ITM_SHTIM,                ITM_FGGR,              ITM_FGLNLIM,                 ITM_FGLNFUL,
                                             ITM_M14,                       ITM_M124,                   ITM_M1234,                ITM_MNUp1,             ITM_BASE_MYM,                ITM_BASE_HOME,
                                             ITM_F14,                       ITM_F124,                   ITM_F1234,                ITM_SH_LONGPRESS,      ITM_MYMx3,                   ITM_HOMEx3         };

// D47 vv
TO_QSPI const int16_t menu_PREF[]       = {  ITM_SYSTEM2,                   ITM_ACTUSB,                 ITM_SETSIG2,              ITM_ERPN,              ITM_HPRP,                    ITM_CFG,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,

                                             ITM_SSIZE4,                    ITM_SSIZE8,                 ITM_CB_CPXRES,            ITM_CB_SPCRES,         ITM_RMODE,                   ITM_CFG,
                                             ITM_INP_DEF_43S,               ITM_INP_DEF_DP,             ITM_INP_DEF_CPXDP,        ITM_INP_DEF_LI,        ITM_NULL,                    ITM_NULL,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,

//exp
////                                             ITM_SYSTEM2,                   ITM_ACTUSB,                 ITM_SETSIG2,              ITM_ERPN,              ITM_HPRP,                    ITM_CFG,
////                                             ITM_SSIZE4,                    ITM_SSIZE8,                 ITM_CB_CPXRES,            ITM_CB_SPCRES,         ITM_RMODE,                   ITM_NULL,
////                                             ITM_INP_DEF_43S,               ITM_INP_DEF_DP,             ITM_INP_DEF_CPXDP,        ITM_INP_DEF_LI,        ITM_NULL,                    ITM_NULL,



                                             ITM_SAFERESET,                 ITM_G_DOUBLETAP,            ITM_SHTIM,                ITM_FGGR,              ITM_FGLNLIM,                 ITM_FGLNFUL,
                                             ITM_M14,                       ITM_M124,                   ITM_M1234,                ITM_MNUp1,             ITM_BASE_MYM,                ITM_BASE_HOME,
                                             ITM_F14,                       ITM_F124,                   ITM_F1234,                ITM_SH_LONGPRESS,      ITM_MYMx3,                   ITM_HOMEx3         };
// D47 ^^


TO_QSPI const int16_t menu_PARTS[]       = { ITM_IP,                        ITM_FP,                     ITM_MANT,                 ITM_EXPT,              ITM_SIGN,                    ITM_DECOMP,
                                             ITM_SDL,                       ITM_SDR,                    ITM_ROUND2,               ITM_ROUNDI2,           ITM_RDP,                     ITM_RSD,                            //JM
                                             ITM_FLOOR,                     ITM_CEIL,                   ITM_MAGNITUDE,            ITM_ARG,               ITM_RE,                      ITM_IM                        };

/*      Menu name                           <----------------------------------------------------------------------------- 6 functions ---------------------------------------------------------------------------->  */
/*                                          <---------------------------------------------------------------------- 6 f shifted functions ------------------------------------------------------------------------->  */
/*                                          <---------------------------------------------------------------------- 6 g shifted functions ------------------------------------------------------------------------->  */
TO_QSPI const int16_t menu_PROB[]        = {
                                             ITM_RAN,                       ITM_RANI,                   ITM_COMB,                 ITM_PERM,              ITM_XFACT,                   -MNU_DISTR,
                                             ITM_SEED,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL       };

TO_QSPI const int16_t menu_DISTR[]       = {
#if !defined(SAVE_SPACE_DM42_15)
                                             -MNU_NORML,                    -MNU_CHI2,                  -MNU_T,                   -MNU_F,                -MNU_EXPON,                  -MNU_WEIBL,
                                             -MNU_STDNORML,                 -MNU_UNIFORM,               -MNU_CAUCH,               -MNU_PARETO,           -MNU_LOGIS,                  -MNU_GEV,
                                             ITM_NULL,                      -MNU_DISUNIFORM,            -MNU_GEOM,                -MNU_HYPER,            -MNU_POISS,                  -MNU_BINOM
#endif // !SAVE_SPACE_DM42_15
                                           };


#define DISTNMENU2(name, pdf1, lcdf1, ucdf1, qf1, pdf2, lcdf2, ucdf2, qf2, p1, p2, p3)              \
  TO_QSPI const int16_t name[] = { pdf1,     ITM_NULL, lcdf1,      ucdf1,     ITM_NULL, qf1,        \
                                   pdf2,     ITM_NULL, lcdf2,      ucdf2,     ITM_NULL, qf2,        \
                                   p1,       p2,       p3,         ITM_NULL,  ITM_NULL, ITM_NULL }

#define DISTNMENU(name, pdf, lcdf, ucdf, qf, p1, p2, p3)                                            \
  DISTNMENU2(name, pdf, lcdf, ucdf, qf, ITM_NULL, ITM_NULL, ITM_NULL, ITM_NULL, p1, p2, p3)

//        global name       PDF             LCDF            UCDF            QF                  Param 1          Param 2          Param 3
DISTNMENU(menu_t,           ITM_TPX,        ITM_TX,         ITM_TUX,        ITM_TM1P,           ITM_STO_M_nu,    ITM_NULL,        ITM_NULL);
DISTNMENU(menu_F,           ITM_FPX,        ITM_FX,         ITM_FUX,        ITM_FM1P,           ITM_STO_M_d1,    ITM_STO_N_d2,    ITM_NULL);
DISTNMENU(menu_chi2,        ITM_chi2Px,     ITM_chi2x,      ITM_chi2ux,     ITM_chi2M1,         ITM_STO_M_nu,    ITM_NULL,        ITM_NULL);
DISTNMENU(menu_StdNorml,    ITM_STDNORMLP,  ITM_STDNORML,   ITM_STDNORMLU,  ITM_STDNORMLM1,     ITM_NULL,        ITM_NULL,        ITM_NULL);
DISTNMENU(menu_Cauch,       ITM_CAUCHP,     ITM_CAUCH,      ITM_CAUCHU,     ITM_CAUCHM1,        ITM_STO_M_x0,    ITM_STO_S_gamma, ITM_NULL);
DISTNMENU(menu_Expon,       ITM_EXPONP,     ITM_EXPON,      ITM_EXPONU,     ITM_EXPONM1,        ITM_STO_R_lambda,ITM_NULL,        ITM_NULL);
DISTNMENU(menu_Logis,       ITM_LOGISP,     ITM_LOGIS,      ITM_LOGISU,     ITM_LOGISM1,        ITM_STO_M_mu,    ITM_STO_S_s,     ITM_NULL);
DISTNMENU(menu_Weibl,       ITM_WEIBLP,     ITM_WEIBL,      ITM_WEIBLU,     ITM_WEIBLM1,        ITM_STO_Q_k,     ITM_STO_S_lambda,ITM_NULL);
DISTNMENU(menu_Geom,        ITM_GEOMP,      ITM_GEOM,       ITM_GEOMU,      ITM_GEOMM1,         ITM_STO_P_p,     ITM_NULL,        ITM_NULL);
DISTNMENU(menu_Hyper,       ITM_HYPERP,     ITM_HYPER,      ITM_HYPERU,     ITM_HYPERM1,        ITM_STO_M_N,     ITM_STO_N1,      ITM_STO_Q_K);
DISTNMENU(menu_Poiss,       ITM_POISSP,     ITM_POISS,      ITM_POISSU,     ITM_POISSM1,        ITM_STO_R_lambda,ITM_NULL,        ITM_NULL);
DISTNMENU(menu_GEV,         ITM_GEVP,       ITM_GEV,        ITM_GEVU,       ITM_GEVM1,          ITM_STO_M_mu,    ITM_STO_S_sigma, ITM_STO_Q_xi);
DISTNMENU(menu_Uniform,     ITM_UNIFORMP,   ITM_UNIFORML,   ITM_UNIFORMU,   ITM_UNIFORMI,       ITM_STO_M_a,     ITM_STO_N_b,     ITM_NULL);
DISTNMENU(menu_DisUniform,  ITM_DISUNIFORMP,ITM_DISUNIFORML,ITM_DISUNIFORMU,ITM_DISUNIFORMI,    ITM_STO_M_a,     ITM_STO_N_b,     ITM_NULL);

DISTNMENU2(menu_Pareto,     ITM_PARETOP,    ITM_PARETOL,    ITM_PARETOU,    ITM_PARETOM1,
                            ITM_PARETO2P,   ITM_PARETO2L,   ITM_PARETO2U,   ITM_PARETO2M1,      ITM_STO_M_mu,    ITM_STO_S_sigma, ITM_STO_Q_alpha);
DISTNMENU2(menu_Binom,      ITM_BINOMP,     ITM_BINOM,      ITM_BINOMU,     ITM_BINOMM1,
                            ITM_NBINP,      ITM_NBIN,       ITM_NBINU,      ITM_NBINM1,         ITM_STO_P_p,     ITM_STO_N1,      ITM_NULL);
DISTNMENU2(menu_Norml,      ITM_NORMLP,     ITM_NORML,      ITM_NORMLU,     ITM_NORMLM1,
                            ITM_LGNRMP,     ITM_LGNRM,      ITM_LGNRMU,     ITM_LGNRMM1,        ITM_STO_M_mu,    ITM_STO_S_sigma, ITM_NULL);

/*      Menu name                  <----------------------------------------------------------------------------- 6 functions ---------------------------------------------------------------------------->  */
/*                                 <---------------------------------------------------------------------- 6 f shifted functions ------------------------------------------------------------------------->  */
/*                                 <---------------------------------------------------------------------- 6 g shifted functions ------------------------------------------------------------------------->  */

TO_QSPI const int16_t menu_MyPFN[]       = { ITM_LBL,                   ITM_GTO,                   ITM_XEQ,                   ITM_RTN,                   ITM_END,                   -MNU_PFN_1,
                                             ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  -MNU_LOOP,                 -MNU_TEST,
                                             ITM_EDIT,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  -MNU_PFN_3,                -MNU_PFN_2                     };

TO_QSPI const int16_t menu_PFN_1[]       = { ITM_INPUT,                 ITM_AVIEW,                 ITM_PROMPT,                ITM_PAUSE,                 ITM_TICKS,                 -MNU_PFN_2,
                                             ITM_MSG,                   ITM_ERR,                   ITM_REM,                   ITM_KEYQ,                  ITM_KTYP,                  ITM_PUTK,
                                             ITM_PIXEL,                 ITM_POINT,                 ITM_AGRAPH,                ITM_NULL,                  ITM_PLOTRST,               ITM_PLOTZOOM                   };

TO_QSPI const int16_t menu_PFN_2[]       = { ITM_KEYG,                  ITM_KEYX,                  ITM_MENU,                  ITM_MVAR,                  ITM_VARMNU,                -MNU_PFN_3,
                                             ITM_LocR,                  ITM_POPLR,                 ITM_CLMENU,                ITM_OPEN_MENU,             ITM_EXITALL,               ITM_NULL,
                                             ITM_R_COPY,                ITM_R_SORT,                ITM_R_SWAP,                ITM_R_CLR,                 ITM_NULL,                  ITM_NULL                       };

TO_QSPI const int16_t menu_PFN_3[]       = { ITM_LBL,                   ITM_GTO,                   ITM_XEQ,                   ITM_RTN,                   ITM_END,                   -MNU_PFN,
                                             ITM_SKIP,                  ITM_BACK,                  ITM_XEQP1,                 ITM_RTNP1,                 -MNU_LOOP,                 -MNU_TEST,
                                             ITM_EDIT,                  ITM_CASE,                  ITM_RCLP1,                 ITM_NULL,                  ITM_NULL,                  ITM_USER_PRESET                };


TO_QSPI const int16_t menu_STAT[]        = { ITM_SIGMAPLUS,                 ITM_XBAR,                   ITM_STDDEVWEIGHTED,       ITM_STDDEV,            ITM_SM,                      ITM_XRMS,
                                             ITM_SIGMAMINUS,                ITM_XW,                     ITM_SW,                   ITM_STDDEVPOP,         ITM_SMW,                     ITM_XH,
                                             ITM_NSIGMA,                    ITM_XG,                     ITM_SCATTFACT,            ITM_SCATTFACTp,        ITM_SCATTFACTm,             -MNU_REGR,

                                             ITM_XMIN,                      ITM_LOWER_QUARTILE,         ITM_MEDIAN,               ITM_UPPER_QUARTILE,    ITM_XMAX,                    ITM_SUM,
                                             ITM_NULL,                      ITM_PERCENTILE,             ITM_MAD,                  ITM_IQR,               ITM_NULL,                    ITM_SIGMARANGE,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_CLSIGMA                   };


TO_QSPI const int16_t menu_REGR[]        = { ITM_LR,                        ITM_CORR,                   ITM_SXY,                  ITM_COV,               ITM_XCIRC,                   ITM_YCIRC,
                                             ITM_SA,                        ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,
                                             ITM_LR_A0,                     ITM_LR_A1,                  ITM_LR_A2,                -MNU_MODEL,            ITM_PLOT_ASSESS,             ITM_PLOT_SCATR                      };

TO_QSPI const int16_t menu_MODEL[]       = { ITM_T_LINF,                    ITM_T_EXPF,                ITM_T_LOGF,                ITM_T_POWERF,          ITM_LR,                      ITM_PLOT_ASSESS,
                                             ITM_T_ROOTF,                   ITM_T_HYPF,                ITM_T_PARABF,              ITM_T_CAUCHF,          ITM_T_GAUSSF,                ITM_NULL,
                                             ITM_RSTF,                      ITM_SETALLF,               ITM_BESTF,                 ITM_BESTFQ,            ITM_NULL,                    ITM_T_ORTHOF                  };


TO_QSPI const int16_t menu_PLOTTING[]    = { ITM_SIGMAPLUS,                 ITM_SIGMAx,                ITM_SIGMAx2,               ITM_SIGMAy,            ITM_SIGMAy2,                 ITM_SIGMAxy,
                                             ITM_SIGMAMINUS,                ITM_SIGMA1onx,             ITM_SIGMA1onx2,            ITM_SIGMA1ony,         ITM_SIGMA1ony2,              ITM_SIGMAxony,
                                             ITM_NSIGMA,                    ITM_SIGMAx3,               ITM_SIGMAx4,               ITM_PLOT_STAT,        -MNU_HIST,                   -MNU_REGR,

                                             ITM_SIGMAPLUS,                 ITM_SIGMAlnx,              ITM_SIGMAln2x,             ITM_SIGMAlny,          ITM_SIGMAln2y,               ITM_SIGMAx2y,
                                             ITM_SIGMAMINUS,                ITM_SIGMAylnx,             ITM_SIGMAlnxy,             ITM_SIGMAxlny,         ITM_SIGMAx2lny,              ITM_SIGMAx2ony,
                                             ITM_NSIGMA,                    ITM_NULL,                  ITM_NULL,                  ITM_SIGMAlnyonx,       ITM_NULL,                    ITM_CLSIGMA                   };

TO_QSPI const int16_t menu_GRAPHS[]      = { VAR_LX,                        VAR_UX,                    VAR_LY,                    VAR_UY,                ITM_DRAW_LU,                 ITM_DRAW,                     };

TO_QSPI const int16_t menu_PLOT_SCATR[]  = {
                                             ITM_PLOT_CENTRL,               ITM_SMI,                    ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL                      };

TO_QSPI const int16_t menu_PLOT_LR[]   = {
                                             ITM_PLOT_NXT,                  ITM_MZOOMY,                 ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL                      };

TO_QSPI const int16_t menu_HIST[]   = {
                                             ITM_HISTOX,                    ITM_HISTOY,                 ITM_LOBIN,                ITM_nBINS,             ITM_HIBIN,                   ITM_HPLOT,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL                      };

TO_QSPI const int16_t menu_HPLOT[]   = {
                                             ITM_HNORM,                     ITM_MZOOMY,                 ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL                      };

TO_QSPI const int16_t menu_STK[]         = { ITM_DROP,                      ITM_Rdown,                  ITM_Rup,                  ITM_LASTX,             ITM_FILL,                    ITM_CLSTK,
                                             ITM_DROPY,                     ITM_Xex,                    ITM_Yex,                  ITM_Zex,               ITM_Tex,                     ITM_SHUFFLE,
                                             ITM_4SWAP,                     ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                   -MNU_MULTSTK                   };

TO_QSPI const int16_t menu_MULTSTK[]     = { ITM_3STO,                      ITM_3RCL,                   ITM_3DROP,                ITM_3DUP,              ITM_3SWAP,                   ITM_4SWAP,
                                             ITM_2STO,                      ITM_2RCL,                   ITM_NDROP,                ITM_NDUP,              ITM_NSWAP,                   ITM_NULL                      };



/*      Menu name                           <----------------------------------------------------------------------------- 6 functions ---------------------------------------------------------------------------->  */
/*                                          <---------------------------------------------------------------------- 6 f shifted functions ------------------------------------------------------------------------->  */
/*                                          <---------------------------------------------------------------------- 6 g shifted functions ------------------------------------------------------------------------->  */

TO_QSPI const int16_t menu_TEST[]        = { ITM_XLT,                       ITM_XLE,                    ITM_XEQU,                 ITM_XNE,               ITM_XGE,                     ITM_XGT,
                                             ITM_XEQUM0,                    ITM_XEQUP0,                 ITM_XAEQU,                ITM_PRIME,             ITM_NULL,                    ITM_NULL,
                                             ITM_ENTRY,                     ITM_KEYQ,                   ITM_LBLQ,                 ITM_TOP,               ITM_LEAPQ,                   ITM_CONVG,

                                             ITM_SINTQ,                     ITM_LINTQ,                  ITM_REALQ,                ITM_CPXQ,              ITM_DATEQ,                   ITM_TIMEQ,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,
                                             ITM_STRINGQ,                   ITM_NULL,                   ITM_REALMATQ,             ITM_COMPLEXMATQ,       ITM_NULL,                    ITM_CONFIGQ,

                                             ITM_FPQ,                       ITM_INTQ,                   ITM_ISREZQ,               ITM_ISIMZQ,            ITM_MATRIXQ,                 ITM_M_SQRQ,
                                             ITM_EVEN,                      ITM_ODD,                    ITM_ISREQ,                ITM_ISIMQ,             ITM_ISVECT2DQ,               ITM_ISVECT3DQ,
                                             ITM_NUMBRQ,                    ITM_ANGLEQ,                 ITM_NANQ,                 ITM_INFQ,              ITM_SPECQ,                                                  };

#if defined(OPTION_XFN_1000)
  #define XFN_M MNU_XXFCNS
#else
  #define XFN_M ITM_NULL
#endif

TO_QSPI const int16_t menu_XFN[]         = { ITM_NEXTP,                     ITM_PRIME,                  ITM_FACTORS,              ITM_FIB,               ITM_AGM,                     ITM_LINPOL,
                                             ITM_zetaX,                     ITM_PARALLEL,               ITM_PFACTORSMULT,         ITM_EE_EXP_TH,         ITM_M1X,                     ITM_XFACT,
                                             ITM_GD,                        ITM_GDM1,                  -MNU_NUMTHEORY,           -XFN_M,                 ITM_BN,                      ITM_BNS,

                                             ITM_gammaXY,                   ITM_GAMMAXY,                ITM_IGAMMAP,              ITM_IGAMMAQ,           ITM_GAMMAX,                  ITM_LNGAMMA,
                                             ITM_WM,                        ITM_WP,                     ITM_WM1,                  ITM_IXYZ,              ITM_BETAXY,                  ITM_LNBETA,
                                            -MNU_ELLIPT,                   -MNU_ORTHOG,                 ITM_JYX,                  ITM_YYX,               ITM_ERF,                     ITM_ERFC,

                                             ITM_MIN,                       ITM_MAX,                    ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL                      };

TO_QSPI const int16_t menu_NUMTHEORY[]   = { ITM_FACTORS,                   ITM_SIGMA0,                 ITM_SIGMA1,               ITM_SIGMAk,            ITM_SIGMAp1,                 ITM_SIGMApk,
                                             ITM_PFACTORSMULT,              ITM_EULPHI,                 ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL                      };

TO_QSPI const int16_t menu_Orthog[]      = { ITM_HN,                        ITM_Lm,                     ITM_LmALPHA,              ITM_Pn,                ITM_Tn,                      ITM_Un,
                                             ITM_HNP,                       ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL                      };

TO_QSPI const int16_t menu_Ellipt[]      = { ITM_sn,                        ITM_cn,                     ITM_dn,                   ITM_Kk,                ITM_Ek,                      ITM_PInk,
                                             ITM_am,                        ITM_NULL,                   ITM_NULL,                 ITM_Fphik,             ITM_Ephik,                   ITM_ZETAphik                  };


//XFCNS is different for C47hw, R47hw. Sim is not R47, therefore the C47 layout
TO_QSPI const int16_t menu_XXFCNS[]    =   { ITM_DEG2_XFN,                  ITM_RAD2_XFN,               ITM_ADD_XFN,              ITM_SUB_XFN,           ITM_MULT_XFN,                ITM_DIV_XFN,
                                             ITM_DEG,                       ITM_RAD,                    ITM_pi_XFN,               ITM_sin_XFN,           ITM_cos_XFN,                 ITM_tan_XFN,
                                             ITM_TO_XFN,                    ITM_DRG_XFN,                ITM_atan2_XFN,            ITM_arcsin_XFN,        ITM_arccos_XFN,              ITM_arctan_XFN,
#if (CALCMODEL != USER_R47)
                                             ITM_MOD_XFN,                   ITM_1ONX_XFN,               ITM_SQRT_XFN,             ITM_LOG_XFN,           ITM_LN_XFN,                  ITM_RDP_XFN,
                                             ITM_MODANG_XFN,                ITM_POWER_XFN,              ITM_SQR_XFN,              ITM_10X_XFN,           ITM_EXP_XFN,                 ITM_RSD_XFN,
                                             ITM_TO_XFN,                    ITM_XTHROOT_XFN,            ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_CHS_XFN,
#else
                                             ITM_SQR_XFN,                   ITM_SQRT_XFN,               ITM_1ONX_XFN,             ITM_POWER_XFN,         ITM_LOG_XFN,                 ITM_LN_XFN,
                                             ITM_CHS_XFN,                   ITM_MOD_XFN,                ITM_MODANG_XFN,           ITM_XTHROOT_XFN,       ITM_10X_XFN,                 ITM_EXP_XFN,
                                             ITM_TO_XFN,                    ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_RDP_XFN,                 ITM_RSD_XFN,
#endif

                                             ITM_3STO,                      ITM_3RCL,                   ITM_3DROP,                ITM_3SWAP,             ITM_3DUP,                    ITM_NULL,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,
                                             ITM_LARGELI,                   ITM_DREAL,                  ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL                   };



/*      Menu name                           <----------------------------------------------------------------------------- 6 functions ---------------------------------------------------------------------------->  */
/*                                          <---------------------------------------------------------------------- 6 f shifted functions ------------------------------------------------------------------------->  */
/*                                          <---------------------------------------------------------------------- 6 g shifted functions ------------------------------------------------------------------------->  */
TO_QSPI const int16_t menu_CATALOG[]     = { -MNU_FCNS,                    -MNU_CONST,                  -MNU_CHARS,               -MNU_PROGS,            -MNU_VARS,                   -MNU_MENUS,
                                              ITM_NULL,                    ITM_NULL,                    ITM_NULL,                 ITM_NULL,              ITM_NULL,                     ITM_KEYMAP,                   };

TO_QSPI const int16_t menu_AIMCATALOG[]  = { -MNU_MyAlpha,                 -MNU_ALPHA_OMEGA,            -MNU_ALPHAMATH,           -MNU_ALPHAMISC,        -MNU_ALPHAINTL,              ITM_KEYMAP                    };

TO_QSPI const int16_t menu_EIMCATALOG[]  = { -MNU_FCNS_EIM,                -MNU_CONST,                  -MNU_CHARS,                ITM_NULL,              ITM_NULL,                    ITM_KEYMAP                   };

TO_QSPI const int16_t menu_CHARS[]       = { -MNU_MyAlpha,                 -MNU_ALPHA_OMEGA,            -MNU_ALPHAMATH,           -MNU_ALPHAMISC,        -MNU_ALPHAINTL,               ITM_NULL                     };

TO_QSPI const int16_t menu_VARS[]        = { -MNU_NUMBRS,                  -MNU_CPXS,                  -MNU_REALS,                -MNU_ANGLES,           -MNU_LINTS,                  -MNU_ALLVARS,
                                             -MNU_CONFIGS,                 -MNU_MATRS,                 -MNU_DATES,                -MNU_TIMES,            -MNU_SINTS,                  -MNU_STRINGS                  };

TO_QSPI const int16_t menu_DELETE[]      = {  ITM_DELALL,                   ITM_DELMALL,                ITM_DELVALL,               ITM_DELPALL,           ITM_DELP,                    ITM_DELBKUP,
                                              ITM_NULL,                     ITM_NULL,                   ITM_NULL,                  ITM_NULL,              ITM_NULL,                    ITM_NULL,
                                              ITM_NULL,                     ITM_NULL,                   ITM_NULL,                  ITM_NULL,              ITM_NULL,                    ITM_DELITM,                  };

TO_QSPI const int16_t menu_DELITM[]      = {  ITM_NULL,                     ITM_NULL,                   ITM_NULL,                 -MNU_PROGS,            -MNU_VARS,                   -MNU_MENUS                    };

TO_QSPI const int16_t menu_YESNO[]       = {  ITM_NULL,                     ITM_YES,                    ITM_NULL,                  ITM_NULL,              ITM_NO,                      ITM_NULL                     };

/*      Menu name                           <----------------------------------------------------------------------------- 6 functions ---------------------------------------------------------------------------->  */
/*                                          <---------------------------------------------------------------------- 6 f shifted functions ------------------------------------------------------------------------->  */
/*                                          <---------------------------------------------------------------------- 6 g shifted functions ------------------------------------------------------------------------->  */
// Following menu is UPPER CASE for lower case: +(ITM_alpha - ITM_ALPHA)
TO_QSPI const int16_t menu_ALPHA_OMEGA[] = { ITM_ALPHA,                     ITM_BETA,                   ITM_GAMMA,                ITM_DELTA,             ITM_EPSILON,                 ITM_DIGAMMA,
                                             ITM_ZETA,                      ITM_ETA,                    ITM_THETA,                ITM_IOTA,              ITM_KAPPA,                   ITM_LAMBDA,
                                             ITM_MU,                        ITM_NU,                     ITM_XI,                   ITM_OMICRON,           ITM_PI,                      ITM_QOPPA,

                                             ITM_RHO,                       ITM_SIGMA,                  ITM_SIGMA,                ITM_TAU,               ITM_UPSILON,                 ITM_PHI,
                                             ITM_CHI,                       ITM_PSI,                    ITM_OMEGA,                ITM_SAMPI,             ITM_NULL,                    ITM_NULL,                           //JM modified greek sequence
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,

                                             ITM_IOTA_DIALYTIKA,            ITM_NULL,                   ITM_NULL,                 ITM_UPSILON_DIALYTIKA, ITM_NULL,                    ITM_NULL                      };

TO_QSPI const int16_t menu_alpha_omega[] = { ITM_alpha,                     ITM_beta,                   ITM_gamma,                ITM_delta,             ITM_epsilon,                 ITM_digamma,
                                             ITM_zeta,                      ITM_eta,                    ITM_theta,                ITM_iota,              ITM_kappa,                   ITM_lambda,
                                             ITM_mu,                        ITM_nu,                     ITM_xi,                   ITM_omicron,           ITM_pi,                      ITM_qoppa,

                                             ITM_rho,                       ITM_sigma,                  ITM_sigma_end,            ITM_tau,               ITM_upsilon,                 ITM_phi,
                                             ITM_chi,                       ITM_psi,                    ITM_omega,                ITM_sampi,             ITM_NULL,                    ITM_NULL,
                                             ITM_alpha_TONOS,               ITM_epsilon_TONOS,          ITM_eta_TONOS,            ITM_iotaTON,           ITM_iota_DIALYTIKA_TONOS,    ITM_NULL,                           //JM modified greek sequence

                                             ITM_iota_DIALYTIKA,            ITM_omicron_TONOS,          ITM_upsilon_TONOS,        ITM_upsilon_DIALYTIKA, ITM_upsilon_DIALYTIKA_TONOS, ITM_omega_TONOS               };

TO_QSPI const int16_t menu_AngleConv_43S[]= {ITM_DEG2,                      ITM_RAD2,                   ITM_GRAD2,                ITM_DMS2,              ITM_MULPI2,                 -MNU_TRI,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_dotD,                    ITM_msTo,
                                             ITM_DEG,                       ITM_RAD,                    ITM_GRAD,                 ITM_NULL,              ITM_NULL,                    ITM_NULL                      };



/*      Menu name                           <----------------------------------------------------------------------------- 6 functions ---------------------------------------------------------------------------->  */
/*                                          <---------------------------------------------------------------------- 6 f shifted functions ------------------------------------------------------------------------->  */
/*                                          <---------------------------------------------------------------------- 6 g shifted functions ------------------------------------------------------------------------->  */




//This Section is generated by UNIT and Submenu Changes documenb  0
TO_QSPI const int16_t menu_UnitConv[]        = {
                                                    -MNU_CONVE,               -MNU_CONVM,               -MNU_MISC,                -MNU_CONVX,               -MNU_CONVA,               -MNU_CONVV,
                                                    -MNU_CONVP,               -MNU_CONVYMMV,            -MNU_CONVANG,             -MNU_CONVS,               -MNU_CONVFP,              -MNU_CONVCHEF,
                                                    ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 -MNU_CONVHUM};
TO_QSPI const int16_t menu_ConvA[]        = {
                                                    ITM_ACtoHA,               ITM_HAtoAC,               ITM_HECTAREtoM2,          ITM_M2toHECTARE,          ITM_MI2toKM2,             ITM_KM2toMI2,
                                                    ITM_ACUStoHA,             ITM_HAtoACUS,             ITM_MUtoM2,               ITM_M2toMU,               ITM_NMI2toKM2,            ITM_KM2toNMI2,
                                                    ITM_FT2toHA,              ITM_HAtoFT2,              ITM_FT2toM2,              ITM_M2toFT2,              ITM_HAtoKM2,              ITM_KM2toHA};
TO_QSPI const int16_t menu_ConvE[]        = {
                                                    ITM_WHtoJ,                ITM_JtoWH,                ITM_CALtoJ,               ITM_JtoCAL,               ITM_BTUtoJ,               ITM_JtoBTU,
                                                    ITM_EVtoJ,                ITM_JtoEV,                ITM_ERGtoJ,               ITM_JtoERG,               ITM_FoetoJ,               ITM_JtoFoe,
                                                    ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL};
TO_QSPI const int16_t menu_ConvFP[]        = {
                                                    ITM_MMHGtoPA,             ITM_PAtoMMHG,             ITM_INCHHGtoPA,           ITM_PAtoINCHHG,           ITM_LBFtoN,               ITM_NtoLBF,
                                                    ITM_ATMtoPA,              ITM_PAtoATM,              ITM_PSItoPA,              ITM_PAtoPSI,              ITM_NULL,                 ITM_NULL,
                                                    ITM_BARtoPA,              ITM_PAtoBAR,              ITM_TORtoPA,              ITM_PAtoTOR,              ITM_NULL,                 ITM_NULL};
TO_QSPI const int16_t menu_ConvM[]        = {
                                                    ITM_LBStoKG,              ITM_KGtoLBS,              ITM_CWTtoKG,              ITM_KGtoCWT,              ITM_OZtoG,                ITM_GtoOZ,
                                                    ITM_STOtoKG,              ITM_KGtoSTO,              ITM_SCWtoKG,              ITM_KGtoSCW,              ITM_TRZtoG,               ITM_GtoTRZ,
                                                    ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,
                                                    ITM_TONtoKG,              ITM_KGtoTON,              ITM_STtoKG,               ITM_KGtoST,               ITM_CARATtoG,             ITM_GtoCARAT,
                                                    ITM_JINtoKG,              ITM_KGtoJIN,              ITM_LIANGtoKG,            ITM_KGtoLIANG,            ITM_NULL,                 ITM_NULL,
                                                    ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL};
TO_QSPI const int16_t menu_Misc[]        = {
                                                    ITM_HMStoHR,              ITM_HRtoHMS,              ITM_CtoF,                 ITM_FtoC,                 ITM_FRtoDB,               ITM_DBtoFR,
                                                    ITM_YEARtoS,              ITM_StoYEAR,              ITM_NMtoLBFFT,            ITM_LBFFTtoNM,            ITM_PRtoDB,               ITM_DBtoPR,
                                                    ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL};
TO_QSPI const int16_t menu_ConvP[]        = {
                                                    ITM_HPEtoW,               ITM_WtoHPE,               ITM_HPUKtoW,              ITM_WtoHPUK,              ITM_HPMtoW,               ITM_WtoHPM,
                                                    ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,
                                                    ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL};
TO_QSPI const int16_t menu_ConvX[]        = {
                                                    ITM_MItoKM,               ITM_KMtoMI,               ITM_YDtoM,                ITM_MtoYD,                ITM_PCtoM,                ITM_MtoPC,
                                                    ITM_NMItoKM,              ITM_KMtoNMI,              ITM_FTtoM,                ITM_MtoFT,                ITM_LYtoM,                ITM_MtoLY,
                                                    ITM_NMItoMI,              ITM_MItoNMI,              ITM_INCHtoMM,             ITM_MMtoINCH,             ITM_AUtoM,                ITM_MtoAU,

                                                    ITM_LItoM,                ITM_MtoLI,                ITM_YINtoM,               ITM_MtoYIN,               ITM_NULL,                 ITM_NULL,
                                                    ITM_NULL,                 ITM_NULL,                 ITM_ZHANGtoM,             ITM_MtoZHANG,             ITM_CUNtoM,               ITM_MtoCUN,
                                                    ITM_NULL,                 ITM_NULL,                 ITM_CHItoM,               ITM_MtoCHI,               ITM_FENtoM,               ITM_MtoFEN,

                                                    ITM_MILEtoM,              ITM_MtoMILE,              ITM_FTUStoM,              ITM_MtoFTUS,              ITM_POINTtoMM,            ITM_MMtoPOINT,
                                                    ITM_NMItoM,               ITM_MtoNMI,               ITM_FATHOMtoM,            ITM_MtoFATHOM,            ITM_NULL,                 ITM_NULL,
                                                    ITM_NULL,                 ITM_NULL,                 ITM_INCHtoCM,             ITM_CMtoINCH,             ITM_NULL,                 ITM_NULL};
TO_QSPI const int16_t menu_ConvV[]        = {
                                                   ITM_IN3toML,               ITM_MLtoIN3,              ITM_FZUKtoIN3,            ITM_IN3toFZUK,            ITM_FT3toL,               ITM_LtoFT3,
                                                   ITM_FZUKtoML,              ITM_MLtoFZUK,             ITM_GLUKtoFT3,            ITM_FT3toGLUK,            ITM_GLUKtoL,              ITM_LtoGLUK,
                                                   ITM_FZUKtoGLUK,            ITM_GLUKtoFZUK,           ITM_BARRELtoM3,           ITM_M3toBARREL,           ITM_QTtoL,                ITM_LtoQT,
                                                   ITM_IN3toML,               ITM_MLtoIN3,              ITM_FZUStoIN3,            ITM_IN3toFZUS,            ITM_FT3toL,               ITM_LtoFT3,
                                                   ITM_FZUStoML,              ITM_MLtoFZUS,             ITM_GalUStoFT3,           ITM_FT3toGalUS,           ITM_GLUStoL,              ITM_LtoGLUS,
                                                   ITM_FZUStoGLUS,            ITM_GLUStoFZUS,           ITM_BARRELtoM3,           ITM_M3toBARREL,           ITM_QTUStoL,              ITM_LtoQTUS               };

TO_QSPI const int16_t menu_ConvS[]        = {
                                                    ITM_KNOTtoKMH,            ITM_KMHtoKNOT,            ITM_KMHtoMPS,             ITM_MPStoKMH,             ITM_RPMtoDEGPS,           ITM_DEGPStoRPM,
                                                    ITM_MPHtoKMH,             ITM_KMHtoMPH,             ITM_MPHtoMPS,             ITM_MPStoMPH,             ITM_RPMtoRADPS,           ITM_RADPStoRPM,
                                                    ITM_FPStoKMH,             ITM_KMHtoFPS,             ITM_FPStoMPS,             ITM_MPStoFPS,             ITM_NULL,                 ITM_NULL};
TO_QSPI const int16_t menu_ConvAng[]        = {
                                                    ITM_DEGtoRAD,             ITM_RADtoDEG,             ITM_DEGtoGRAD,            ITM_GRADtoDEG,            ITM_GRADtoRAD,            ITM_RADtoGRAD,
                                                    ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,
                                                    ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL};

TO_QSPI const int16_t menu_ConvHum[]        = {
                                                    ITM_FURtoM,               ITM_MtoFUR,               ITM_FTNtoS,               ITM_StoFTN,               ITM_FPFtoMPS,             ITM_MPStoFPF,
                                                    ITM_BANANAtoMM,           ITM_MMtoBANANA,           ITM_FIRtoKG,              ITM_KGtoFIR,              ITM_FPFtoKPH,             ITM_KPHtoFPF,
                                                    ITM_BANANAtoINCH,         ITM_INCHtoBANANA,         ITM_FIRtoLB,              ITM_LBtoFIR,              ITM_FPFtoMPH,             ITM_MPHtoFPF,
                                                    ITM_BRDStoM,              ITM_MtoBRDS,              ITM_BRDStoIN,             ITM_INtoBRDS,             ITM_NULL,                 ITM_NULL,
                                                    ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,
                                                    ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL,                 ITM_NULL};

TO_QSPI const int16_t menu_ConvYmmv[]       = {
                                                    ITM_L100toKML,            ITM_KMLtoL100,            ITM_KMLEtoK100K,          ITM_K100KtoKMLE,          ITM_K100KtoKMK,           ITM_KMKtoK100K,
                                                    ITM_L100toMGUS,           ITM_MGUStoL100,           ITM_MGEUStoK100M,         ITM_K100MtoMGEUS,         ITM_K100MtoK100K,         ITM_K100KtoK100M,
                                                    ITM_L100toMGUK,           ITM_MGUKtoL100,           ITM_MGEUKtoK100M,         ITM_K100MtoMGEUK,         ITM_K100MtoMIK,           ITM_MIKtoK100M};
TO_QSPI const int16_t menu_ConvChef[]       = {

                                                    ITM_MLtoTSPUK,            ITM_TSPUKtoML,            ITM_MLtoTBSPUK,           ITM_TBSPUKtoML,           ITM_MLtoCUPUK,            ITM_CUPUKtoML,
                                                    ITM_FZUKtoTSPUK,          ITM_TSPUKtoFZUK,          ITM_FZUKtoTBSPUK,         ITM_TBSPUKtoFZUK,         ITM_FZUKtoCUPUK,          ITM_CUPUKtoFZUK,
                                                    ITM_FZUKtoML,             ITM_MLtoFZUK,             ITM_PINTUKtoML,           ITM_MLtoPINTUK,           ITM_QTtoML,               ITM_MLtoQT,
                                                    ITM_MLtoTSPC,             ITM_TSPCtoML,             ITM_MLtoTBSPC,            ITM_TBSPCtoML,            ITM_MLtoCUPC,             ITM_CUPCtoML,
                                                    ITM_FZUStoTSPC,           ITM_TSPCtoFZUS,           ITM_FZUStoTBSPC,          ITM_TBSPCtoFZUS,          ITM_FZUStoCUPC,           ITM_CUPCtoFZUS,
                                                    ITM_FZUStoML,             ITM_MLtoFZUS,             ITM_PINTLQtoML,           ITM_MLtoPINTLQ,           ITM_QTUStoML,             ITM_MLtoQTUS                    };


//---------//---------//---------//---------//---------



TO_QSPI const int16_t menu_alphaFN[]     = { ITM_FBR,                       ITM_XtoALPHA,               ITM_ALPHAtoX,             ITM_ALPHALENG,         ITM_ALPHAPOS,                ITM_XPARSE,
                                             ITM_ALPHASL,                   ITM_ALPHASR,                ITM_ALPHARL,              ITM_ALPHARR,           ITM_ALPHALOWER,              ITM_ALPHAUPPER,
                                             ITM_ALPHALTRIM,                ITM_ALPHARTRIM,             ITM_NULL,                 ITM_ALPHAMID,          ITM_ALPHALEFT,               ITM_ALPHARIGHT                };



/*      Menu name                           <----------------------------------------------------------------------------- 6 functions ---------------------------------------------------------------------------->  */
/*                                          <---------------------------------------------------------------------- 6 f shifted functions ------------------------------------------------------------------------->  */
/*                                          <---------------------------------------------------------------------- 6 g shifted functions ------------------------------------------------------------------------->  */


TO_QSPI const int16_t menu_alphaMATH[]   = { ITM_LESS_THAN,             ITM_LESS_EQUAL,            ITM_EQUAL,                 ITM_NOT_EQUAL,             ITM_GREATER_EQUAL,         ITM_GREATER_THAN,
                                             ITM_LEFT_CURLY_BRACKET,    ITM_LEFT_SQUARE_BRACKET,   ITM_LEFT_PARENTHESIS,      ITM_RIGHT_PARENTHESIS,     ITM_RIGHT_SQUARE_BRACKET,  ITM_RIGHT_CURLY_BRACKET,
                                             ITM_BACK_SLASH,            ITM_SLASH,                 ITM_OBELUS,                ITM_CROSS,                 ITM_MINUS,                 ITM_PLUS,

                                             ITM_CIRCUMFLEX,            ITM_SQUARE_ROOT,           ITM_CUBEROOT_SIGN,         ITM_xTH_ROOT,              ITM_x_UNDER_ROOT,          ITM_y_UNDER_ROOT,
                                             ITM_NOT,                   ITM_AND,                   ITM_OR,                    ITM_XOR,                   ITM_NAND,                  ITM_NOR,
                                             ITM_TILDE,                 ITM_ALMOST_EQUAL,          ITM_ASYMPOTICALLY_EQUAL,   ITM_IDENTICAL_TO,          ITM_COLON_EQUALS,          ITM_PLUS_MINUS,

                                             ITM_POLAR_char,            ITM_RIGHT_ANGLE,           ITM_ANGLE,                 ITM_MEASURED_ANGLE,        ITM_SPHERICAL_ANGLE,       ITM_AMPERSAND,
                                             ITM_PIPE,                  ITM_DEGREE,                ITM_RIGHT_SINGLE_QUOTE,    ITM_RIGHT_DOUBLE_QUOTE,    ITM_RIGHT_TACK,            ITM_PERPENDICULAR,
                                             ITM_PARALLEL,              ITM_BULLET,                ITM_RING,                  ITM_EulerE,                ITM_pi,                    ITM_op_i_char,

                                             ITM_op_j_char,             ITM_PLANCK_2PI,            ITM_EEXCHR,                ITM_SUM_char,              ITM_PRODUCT,               ITM_MICRO,
                                             ITM_INTEGRAL_SIGN,         ITM_CONTOUR_INTEGRAL,      ITM_DOUBLE_INTEGRAL,       ITM_SURFACE_INTEGRAL,      ITM_TRIPLE_INTEGRAL,       ITM_VOLUME_INTEGRAL,
                                             ITM_OMEGA,                 ITM_PARTIAL_DIFF,          ITM_INCREMENT,             ITM_NABLA,                 ITM_phi_m,                 ITM_theta_m,

                                             ITM_THERE_EXISTS,          ITM_THERE_DOES_NOT_EXIST,  ITM_ELEMENT_OF,            ITM_NOT_ELEMENT_OF,        ITM_CONTAINS,              ITM_DOES_NOT_CONTAIN,
                                             ITM_EMPTY_SET,             ITM_UNION,                 ITM_INTERSECTION,          ITM_SUBSET_OF,             ITM_NOT_SUBSET_OF,         ITM_COMPLEMENT,
                                             ITM_COMPLEX_C,             ITM_IRRATIONAL_I,          ITM_NATURAL_N,             ITM_RATIONAL_Q,            ITM_REAL_R,                ITM_INTEGER_Z,

                                             ITM_MAT_BL,                ITM_MAT_BR,                ITM_ONE_QUARTER,           ITM_ONE_HALF,              ITM_PROPORTIONAL,          ITM_INFINITY,
                                             ITM_MAT_ML,                ITM_MAT_MR,                ITM_SUP_BOLD_T,            ITM_SUB_MINUS,             ITM_SUB_PLUS,              ITM_SUB_INFINITY,
                                             ITM_MAT_TL,                ITM_MAT_TR,                ITM_FOR_ALL,               ITM_SUP_MINUS,             ITM_SUP_PLUS,              ITM_SUP_INFINITY,

                                             ITM_u_BAR,                 ITM_v_BAR,                 ITM_w_BAR,                 ITM_x_BAR,                 ITM_y_BAR,                 ITM_z_BAR,
                                             ITM_u_CIRC2,               ITM_v_CIRC,                ITM_w_CIRC,                ITM_x_CIRC,                ITM_y_CIRC,                ITM_z_CIRC,
                                             ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_i_CIRC,                ITM_j_CIRC,                ITM_k_CIRC };

/*      Menu name                           <----------------------------------------------------------------------------- 6 functions ---------------------------------------------------------------------------->  */
/*                                          <---------------------------------------------------------------------- 6 f shifted functions ------------------------------------------------------------------------->  */
/*                                          <---------------------------------------------------------------------- 6 g shifted functions ------------------------------------------------------------------------->  */

TO_QSPI const int16_t menu_alphaMisc[]    ={ ITM_CR,                       ITM_NUMBER_SIGN,              ITM_AT,                       ITM_AMPERSAND,                ITM_PERCENT,                  ITM_QUOTE,
                                             ITM_DOUBLE_QUOTE,             ITM_DOLLAR,                   ITM_CENT,                     ITM_EURO,                     ITM_POUND,                    ITM_YEN,
                                             ITM_INVERTED_EXCLAMATION_MARK,ITM_INVERTED_QUESTION_MARK,   ITM_PERIOD,                   ITM_COMMA,                    ITM_SEMICOLON,                ITM_COLON,

                                             ITM_EXCLAMATION_MARK,         ITM_QUESTION_MARK,            ITM_UP_ARROW,                 ITM_DOWN_ARROW,               ITM_LEFT_ARROW,               ITM_RIGHT_ARROW,
                                             ITM_RIGHT_SHORT_ARROW,        ITM_RIGHT_DOUBLE_ARROW,       ITM_UP_BLOCKARROW,            ITM_DOWN_BLOCKARROW,          ITM_LEFT_BLOCKARROW,          ITM_RIGHT_BLOCKARROW,
                                             ITM_LEFT_RIGHT_ARROWS,        ITM_LEFT_RIGHT_DOUBLE_ARROW,  ITM_UP_DASHARROW,             ITM_DOWN_DASHARROW,           ITM_LEFT_DASHARROW,           ITM_RIGHT_DASHARROW,

                                             ITM_ex,                       ITM_SERIAL_IO,                ITM_HOLLOW_UP_ARROW,          ITM_HOLLOW_DOWN_ARROW,        ITM_LEFT_DOUBLE_ANGLE,        ITM_RIGHT_DOUBLE_ANGLE,
                                             ITM_DATE_D,                   ITM_TIME_T,                   ITM_SECTION,                  ITM_CHECK_MARK,               ITM_BULLET,                   ITM_ASTERISK,
                                             ITM_SUP_ASTERISK,             ITM_TILDE,                    ITM_HOURGLASS,                ITM_WATCH,                    ITM_TIMER_SYMBOL,             ITM_NEG_EXCLAMATION_MARK,

                                             ITM_USER_MODE,                ITM_BATTERY,                  ITM_PRINTER,                  ITM_HAMBURGER,                ITM_BST_SIGN,                 ITM_SST_SIGN,
                                             ITM_CYCLIC,                   ITM_USB_SYMBOL,               ITM_SUB_SUN,                  ITM_SUB_EARTH,                ITM_US,                       ITM_UK,
                                             ITM_litre,                    ITM_LEFT_DOUBLE_QUOTE,        ITM_RIGHT_DOUBLE_QUOTE,       ITM_DIRECT_CURRENT,           ITM_ALTERN_CURRENT,           ITM_POWER_SYMBOL,

                                             ITM_CB_OFF,                   ITM_CB_ON,                    ITM_RB_OFF,                   ITM_RB_ON,                    ITM_DIA_OFF,                  ITM_DIA_ON };


TO_QSPI const int16_t menu_EQN[]         = { ITM_EQ_NEW,                ITM_EQ_EDI,                 -MNU_2NDDERIV,            -MNU_1STDERIV,            -MNU_Sf,                    -MNU_Solver,
                                             ITM_EQ_DEL,                ITM_NULL,                   ITM_NULL,                 ITM_NULL,                 ITM_NULL,                   -MNU_Grapher              };

TO_QSPI const int16_t menu_ADV[]         = { ITM_PIn,                  ITM_SIGMAn,                  ITM_FDQX,                 ITM_FQX,                  -MNU_Sfdx,                  ITM_SOLVE,
                                             ITM_iPIn,                 ITM_iSIGMAn,                 ITM_SLVC,                 ITM_SLVQ,                 ITM_PGMINT,                 ITM_PGMSLV,
                                             ITM_NULL,                 ITM_NULL,                    ITM_NULL,                 ITM_NULL,                 ITM_NULL,                   ITM_NULL                  };

TO_QSPI const int16_t menu_1stDeriv[]    = { ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,                 -MNU_GRAPHS,                ITM_FPHERE                };
//note: the items in here are dynamically assigned, including the static ones

TO_QSPI const int16_t menu_2ndDeriv[]    = { ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,                 -MNU_GRAPHS,                ITM_FPPHERE               };
//note: the items in here are dynamically assigned, including the static ones

TO_QSPI const int16_t menu_Sf[]          = { ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,                 ITM_NULL,                   ITM_NULL                  };
//note: the items in here are dynamically assigned, including the static ones (original population was NULL)

TO_QSPI const int16_t menu_Solver[]      = { ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,                 ITM_NULL,                   ITM_NULL                  };
//note: the items in here are dynamically assigned, including the static ones (original population was NULL)

TO_QSPI const int16_t menu_Grapher[]     = { ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,                 ITM_NULL,                   ITM_NULL                  };
//note: the items in here are dynamically assigned, including the static ones (original population was NULL)




TO_QSPI const int16_t menu_Sfdx[]        = { VAR_ACC,                       ITM_INTEGRAL_YX,            ITM_INTEGRAL,             VAR_LLIM,              VAR_ULIM,                    ITM_NULL,
 /*same*/                                    ITM_NULL,                      CST_77,                     CST_78,                   ITM_NULL,              ITM_NULL,                    ITM_NULL                   };

// Tool∫ (intgral tools)
TO_QSPI const int16_t menu_Sf_TOOL[]     = { VAR_ACC,                       ITM_INTEGRAL_YX,            ITM_INTEGRAL,             VAR_LLIM,              VAR_ULIM,                    -MNU_GRAPHS,
 /*same*/                                    ITM_NULL,                      CST_77,                     CST_78,                   ITM_NULL,              ITM_NULL,                    ITM_NULL                   };

// ToolS (solver tools)
TO_QSPI const int16_t menu_Solver_TOOL[] = { ITM_SETSIG2,                   ITM_CPXSLV,                 ITM_CPXSLV_LU,            VAR_LEST,              VAR_UEST,                    -MNU_GRAPHS,
                                             ITM_NULL,                      ITM_REALSLV,                ITM_REALSLV_LU,           ITM_NULL,              ITM_NULL,                    ITM_NULL                   };


TO_QSPI const int16_t menu_AUDIO[]       = { ITM_BEEP,                      ITM_TONE,                   ITM_BUZZ,                 ITM_PLAY,              ITM_VOLPLUS,                 ITM_VOLMINUS,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_VOL                    };

TO_QSPI const int16_t menu_IO[]          = { ITM_WRITEP,                    ITM_SAVEST,                 ITM_SAVE,                 ITM_LOADP,             ITM_LOADR,                   ITM_LOADV,
                                             ITM_READP,                     ITM_LOADST,                 ITM_LOAD,                 ITM_LOADSIGMA,         ITM_LOADSS,                  -MNU_PRINT,
                                             ITM_EXPORTP,                   ITM_WRXPALL,                ITM_SAVEAUT,              ITM_NULL,              ITM_SNAP,                    -MNU_AUDIO                    };

TO_QSPI const int16_t menu_PRINT[]       = { ITM_PRINTERX,                  ITM_PRINTERXY,              ITM_PRINTERSTK,           ITM_P_ALLREGS,         ITM_PRINTERR,                ITM_PRINTERPROG,
                                            ITM_PRINTERCHAR,                ITM_PRINTERHASH,            ITM_PRINTERLCD,           ITM_PRINTERREGS,       ITM_PRINTERSIGMA,            ITM_PRINTERUSER,
                                            ITM_PRTACT,                     ITM_PRINTERADV,             ITM_PRINTERDLAY,          ITM_PRINTERMODE,       ITM_PRINTERTAB,              ITM_PRINTERWIDTH };

TO_QSPI const int16_t menu_Tam[]         = { ITM_INDIRECTION,               -MNU_VAR,                   ITM_REG_X,                ITM_REG_Y,             ITM_REG_Z,                   ITM_REG_T,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    -MNU_REG                      };

TO_QSPI const int16_t menu_TamVarOnly[]  = { ITM_INDIRECTION,               -MNU_VAR,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL                      };


TO_QSPI const int16_t menu_TamAlpha[]    = { -MNU_ALPHA_OMEGA,             -MNU_ALPHAMATH,             -MNU_ALPHAMISC,           -MNU_ALPHAINTL,         ITM_T_LEFT_ARROW,            ITM_T_RIGHT_ARROW,            //DL
                                             -MNU_MyAlpha,                  ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,
                                             CHR_case,                      CHR_num,                    ITM_SCR,                  ITM_USERMODE,          ITM_NULL,                    ITM_NULL,                     };   //DL

TO_QSPI const int16_t menu_TamCmp[]      = { ITM_INDIRECTION,               -MNU_VAR,                   ITM_REG_X,                ITM_REG_Y,             ITM_REG_Z,                   ITM_REG_T,
                                             ITM_0P,                        ITM_1P,                     ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    -MNU_REG                      };
                                             
TO_QSPI const int16_t menu_TamFlag[]     = { ITM_INDIRECTION,               -MNU_SYSFL,                 ITM_REG_X,                ITM_REG_Y,             ITM_REG_Z,                   ITM_REG_T,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,
                                             SFL_BCD,                       SFL_TOPHEX,                 SFL_LEAD0,                SFL_OVERFLOW,          SFL_CARRY,                   -MNU_FLG                      };

TO_QSPI const int16_t menu_TamNonRegMax[]= { ITM_INDIRECTION,               ITM_TAMMAX,                 ITM_INDIRECT_X,           ITM_INDIRECT_Y,        ITM_INDIRECT_Z,              ITM_INDIRECT_T                };
TO_QSPI const int16_t menu_TamNonRegTrk[]= { ITM_INDIRECTION,               ITM_YY_TRACK,               ITM_INDIRECT_X,           ITM_INDIRECT_Y,        ITM_INDIRECT_Z,              ITM_INDIRECT_T,
                                             ITM_NULL,                      ITM_YY_OFF,                 ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL                      };

TO_QSPI const int16_t menu_TamNonReg[]   = { ITM_INDIRECTION,               ITM_NULL,                   ITM_INDIRECT_X,           ITM_INDIRECT_Y,        ITM_INDIRECT_Z,              ITM_INDIRECT_T                };
TO_QSPI const int16_t menu_TamIndirect[] = { ITM_NULL,                      -MNU_VAR,                   ITM_REG_X,                ITM_REG_Y,             ITM_REG_Z,                   ITM_REG_T,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    -MNU_REG                      };

TO_QSPI const int16_t menu_Reg[]         = { ITM_REG_M,                     ITM_REG_N,                  ITM_REG_P,                ITM_REG_Q,             ITM_REG_R,                   ITM_REG_S,
                                             ITM_REG_K,                     ITM_REG_L,                  ITM_REG_A,                ITM_REG_B,             ITM_REG_C,                   ITM_REG_D,
                                             ITM_REG_I,                     ITM_REG_J,                  ITM_REG_X,                ITM_REG_Y,             ITM_REG_Z,                   ITM_REG_T,

                                             ITM_REG_E,                     ITM_REG_F,                  ITM_REG_G,                ITM_REG_H,             ITM_REG_O,                   ITM_REG_U,
                                             ITM_REG_V,                     ITM_REG_W,                  ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL                      };

TO_QSPI const int16_t menu_Flg[]         = { ITM_REG_M,                     ITM_REG_N,                  ITM_REG_P,                ITM_REG_Q,             ITM_REG_R,                   ITM_REG_S,
                                             ITM_REG_K,                     ITM_REG_L,                  ITM_REG_A,                ITM_REG_B,             ITM_REG_C,                   ITM_REG_D,
                                             ITM_REG_I,                     ITM_REG_J,                  ITM_REG_X,                ITM_REG_Y,             ITM_REG_Z,                   ITM_REG_T,

                                             ITM_REG_E,                     ITM_REG_F,                  ITM_REG_G,                ITM_REG_H,             ITM_REG_O,                   ITM_REG_U,
                                             ITM_REG_V,                     ITM_REG_W,                  ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL                      };


TO_QSPI const int16_t menu_TamSto[]      = { ITM_INDIRECTION,               -MNU_VAR,                   ITM_REG_X,                ITM_REG_Y,             ITM_REG_Z,                   ITM_REG_T,
                                             ITM_Config,                    ITM_Stack,                  ITM_dddVEL,               ITM_dddIX,             ITM_Min,                     ITM_Max,
                                             ITM_dddEL,                     ITM_dddIJ,                  ITM_dddVEL1,              ITM_dddVEL2,           ITM_dddVEL3,                 -MNU_REG                      };

TO_QSPI const int16_t menu_TamRcl[]      = { ITM_INDIRECTION,               -MNU_VAR,                   ITM_REG_X,                ITM_REG_Y,             ITM_REG_Z,                   ITM_REG_T,
                                             ITM_Config,                    ITM_Stack,                  ITM_dddVEL,               ITM_NULL,              ITM_Min,                     ITM_Max,
                                             ITM_dddEL,                     ITM_dddIJ,                  ITM_dddVEL1,              ITM_dddVEL2,           ITM_dddVEL3,                 -MNU_REG                      };

TO_QSPI const int16_t menu_TamStoTVM[]   = { ITM_STORCL_NPPER,              ITM_STORCL_IPonA,           ITM_STORCL_PV,            ITM_STORCL_PMT,        ITM_STORCL_FV,               VAR_PPERonA,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    VAR_CPERonA,
                                             ITM_INDIRECTION,               -MNU_VAR,                   ITM_Min,                  ITM_Max,               ITM_NULL,                    -MNU_REG                     };

TO_QSPI const int16_t menu_TamRclTVM[]   = { ITM_STORCL_NPPER,              ITM_STORCL_IPonA,           ITM_STORCL_PV,            ITM_STORCL_PMT,        ITM_STORCL_FV,               VAR_PPERonA,
                                             ITM_NULL,                      ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    VAR_CPERonA,
                                             ITM_INDIRECTION,               -MNU_VAR,                   ITM_Min,                  ITM_Max,               ITM_NULL,                    -MNU_REG                     };

TO_QSPI const int16_t menu_TamShuffle[]  = { ITM_NULL,                      ITM_NULL,                   ITM_REG_X,                ITM_REG_Y,             ITM_REG_Z,                   ITM_REG_T                     };

TO_QSPI const int16_t menu_TamLabel[]    = { ITM_INDIRECTION,               -MNU_PROG,                  ITM_REG_C,                ITM_REG_D,             ITM_REG_E,                   ITM_REG_F,
                                             ITM_a,                         ITM_b,                      ITM_c,                    ITM_d,                 ITM_e,                       ITM_f,
                                             ITM_g,                         ITM_h,                      ITM_i,                    ITM_j,                 ITM_k,                       ITM_l                         };

TO_QSPI const int16_t menu_TamMenu []    = { ITM_INDIRECTION,               -MNU_MENU,                  ITM_INDIRECT_X,           ITM_INDIRECT_Y,        ITM_INDIRECT_Z,              ITM_INDIRECT_T                };

TO_QSPI const int16_t menu_TamLabelOnly[]= { ITM_INDIRECTION,               -MNU_PROG,                  ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL                      };


TO_QSPI const int16_t menu_Eim[]         = {
                                             ITM_ALOG_SIGN,                 ITM_x_SIGN,                ITM_CIRCUMFLEX,            ITM_ROOT_SIGN,         ITM_EQ_LEFT,                 ITM_EQ_RIGHT,                     //DL
                                             CHR_case,                      CHR_num,                   ITM_SCR,                   ITM_COLON,             ITM_LEFT_PARENTHESIS,        ITM_RIGHT_PARENTHESIS,
                                             ITM_NULL,                      ITM_NULL,                  ITM_NULL,                  ITM_NULL,              ITM_NULL,                    ITM_NULL,

                                             ITM_sin,                       ITM_cos,                   ITM_tan,                   ITM_pi,                ITM_EQ_LEFT,                 ITM_EQ_RIGHT,
                                             ITM_arcsin,                    ITM_arccos,                ITM_arctan,                ITM_op_j_SIGN,         ITM_VERTICAL_BAR,            ITM_ARG,
                                             ITM_NULL,                      ITM_NULL,                  ITM_NULL,                  ITM_NULL,              ITM_NULL,                    ITM_NULL,

                                             ITM_EXCLAMATION_MARK,          ITM_poly_SIGN,             ITM_XEDIT,                 ITM_XSWAP,             ITM_EQ_LEFT,                 ITM_EQ_RIGHT,
                                             ITM_NULL,                      ITM_NULL,                  ITM_NULL,                  ITM_NULL,              ITM_NULL,                    ITM_NULL

                                            };



TO_QSPI const int16_t menu_Timer[]       = { ITM_TIMER_SIGMA_T,             ITM_TIMER_SIGMA_L,          ITM_TIMER_R_T,            ITM_TIMER_R_L,         ITM_TIMER_R_S,               ITM_TIMER_RESET,
                                             ITM_PLOT_STAT,                 ITM_RBR,                    ITM_TIMER_RCL,            ITM_COUNTDN_RCL,       ITM_TIMER_0_1S,              ITM_CLSIGMA };



TO_QSPI const int16_t menu_BASE[]        = { ITM_2HEX,                      ITM_2DEC,                   ITM_2OCT,                 ITM_2BIN,              ITM_HASH_JM,                 ITM_LINT,
                                             ITM_LOGICALAND,                ITM_LOGICALOR,              ITM_LOGICALXOR,           ITM_LOGICALNOT,        ITM_BITSp2,                  ITM_NULL,
                                             ITM_WS64,                      ITM_WS32,                   ITM_WS16,                 ITM_WS8,              -MNU_BITSET,                  ITM_FF,

                                             ITM_A,                         ITM_B,                      ITM_C,                    ITM_D,                 ITM_E,                       ITM_F,
                                             ITM_S64,                       ITM_S32,                    ITM_S16,                  ITM_S08,               ITM_FBYTE,                   ITM_FWORD,
                                             ITM_U64,                       ITM_U32,                    ITM_U16,                  ITM_U08,              -MNU_BITSET,                  ITM_FF                        };


TO_QSPI const int16_t menu_BITSET[]      = { ITM_A,                         ITM_B,                      ITM_C,                    ITM_D,                 ITM_E,                       ITM_F,
                                             ITM_1COMPL,                    ITM_2COMPL,                 ITM_UNSIGN,               ITM_SIGNMT,            ITM_NULL,                    ITM_WSIZE,
                                             ITM_BCD9,                      ITM_BCD10,                  ITM_BCDU,                 ITM_BCD,               ITM_HPBASE,                  ITM_FF};


#if (CALCMODEL != USER_R47)
TO_QSPI const int16_t menu_EE[]          = { ITM_op_j,                      ITM_op_j_pol,               ITM_SQUARE,               ITM_M_INV,             ITM_PARALLEL,                ITM_CLSTK,
                                             ITM_DEG2,                      ITM_RAD2,                   ITM_op_a,                 ITM_op_a2,             ITM_MATX_A,                  ITM_MATX_A_1,
                                             ITM_DEG,                       ITM_RAD,                    ITM_STKTO3x1,             ITM_3x1TOSTK,          ITM_RECT,                    ITM_POLAR,
#else
TO_QSPI const int16_t menu_EE[]          = { ITM_op_j,                      ITM_op_j_pol,               KEY_COMPLEX,              ITM_M_INV,             ITM_PARALLEL,                ITM_CLSTK,
                                             ITM_DEG2,                      ITM_RAD2,                   ITM_op_a,                 ITM_op_a2,             ITM_MATX_A,                  ITM_MATX_A_1,
                                             ITM_DEG,                       ITM_RAD,                    ITM_STKTO3x1,             ITM_3x1TOSTK,          ITM_RECT,                    ITM_POLAR,
#endif

                                             ITM_EE_STO_IR,                ITM_EE_STO_V_Z,              ITM_EE_STO_V_I,           ITM_EE_X2BAL,          ITM_3SWAP,                   ITM_3DROP,
                                             ITM_EE_RCL_V,                 ITM_EE_RCL_I,                ITM_EE_RCL_Z,             ITM_EE_Y2D,            ITM_EE_D2Y,                  -MNU_MULTSTK,
                                             ITM_EE_STO_V,                 ITM_EE_STO_I,                ITM_EE_STO_Z,             ITM_EE_A2S,            ITM_EE_S2A,                  ITM_3R3P                      };

//#if defined(INLINE_TEST)
TO_QSPI const int16_t menu_Inl_Tst[]     = { ITM_TEST,                      ITM_NULL,                   ITM_NULL,                 ITM_SYS_FREE_RAM,      ITM_GET_TEST_BS,             ITM_SET_TEST_BS               };    //dr
//#endif // INLINE_TEST


//ASN_N menu different for C47, R47
#if (CALCMODEL != USER_R47)
TO_QSPI const int16_t menu_ASN_N[]       = { ITM_N_KEY_SIGMA,               ITM_N_KEY_USER,             ITM_N_KEY_ALPHA,          ITM_N_KEY_FGSH,        ITM_N_KEY_GSH,               ITM_FROM_USER,
                                             ITM_N_KEY_DRG,                 ITM_N_KEY_op_j_pol,         ITM_N_KEY_Rup,            ITM_N_KEY_XFACT,       ITM_NULL,                    ITM_TO_USER    };
#else
TO_QSPI const int16_t menu_ASN_N[]       = { ITM_N_KEY_SIGMA,               ITM_N_KEY_USER,             ITM_N_KEY_ALPHA,          ITM_N_KEY_SNAP,        ITM_N_KEY_STOPW,             ITM_FROM_USER,
                                             ITM_N_KEY_NIL,                 ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_TO_USER    };
#endif


//KEYS menu different for C47, R47
TO_QSPI const int16_t menu_KEYS[]      =  {  -MNU_ASN_N,                -MNU_RIBBONS,              -MNU_RESETS,               ITM_ASSIGN,                ITM_USERMODE,              ITM_KEYMAP,
#if (CALCMODEL != USER_R47)
                                             ITM_USER_C47,              ITM_USER_DM42,             ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL         };
#else
                                             ITM_USER_R47f_g,           ITM_USER_R47fg_bk,         ITM_USER_R47fg_g,          ITM_USER_R47bk_fg,         ITM_NULL,                  ITM_NULL         };
#endif


TO_QSPI const int16_t menu_RESETS[]    =  {  ITM_USER_ARESET,           ITM_USER_MRESET,           ITM_USER_HRESET,           ITM_USER_PRESET,           ITM_NULL,                  ITM_USER_KRESET,
                                             ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,
                                             ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL };

TO_QSPI const int16_t menu_RIBBONS[]   =  {  ITM_RIBBON_CPX,            ITM_RIBBON_ENG,            ITM_RIBBON_FIN,            ITM_RIBBON_SAV,            ITM_RIBBON_C47,            ITM_RIBBON_R47,
                                             ITM_NULL,                  ITM_NULL,                  ITM_RIBBON_FIN2,           ITM_RIBBON_SAV2,           ITM_RIBBON_C47PL,          ITM_RIBBON_R47PL,
                                             ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL };

TO_QSPI const int16_t menu_BLUE_C47[]    = { ITM_MAGNITUDE,       -MNU_CPX,         -MNU_STK,         -MNU_TRG_C47,     -MNU_EXP,         ITM_UNDO,
                                             ITM_ARG,             ITM_DELTAPC,      ITM_XTHROOT,      ITM_op_j,         ITM_toREC2,       ITM_toPOL2,
                                             ITM_TGLFRT,          ITM_HASH_JM,      ITM_ms,           ITM_dotD,         ITM_LBL,          ITM_RTN,

                                             ITM_RI,              ITM_VIEW,         ITM_USERMODE,     ITM_NULL,         ITM_NULL,         ITM_NULL,
                                             ITM_SNAP,            ITM_TIMER,       -MNU_INFO,        -MNU_TEST,        -MNU_CONST,        ITM_NULL,
                                             ITM_NULL,            ITM_NULL,         ITM_NULL,         ITM_NULL,         ITM_NULL,         ITM_NULL,

                                             ITM_NULL,            -MNU_KEYS,        -MNU_ALPHAFN,     -MNU_LOOP,        -MNU_IO,          ITM_NULL,
                                             ITM_FLGSV,           -MNU_BITS,        -MNU_CLK,         -MNU_PARTS,       -MNU_INTS,        ITM_NULL,
                                             ITM_RBR,             -MNU_HOME,        -MNU_FIN,         -MNU_XFN,         -MNU_PLOTTING,    ITM_NULL };

#if defined(SAVE_SPACE_DM42_6)
  #define CC_EE  ITM_NULL
#else // !SAVE_SPACE_DM42_6
  #define CC_EE  -MNU_EE
#endif // SAVE_SPACE_DM42_6

#if (CALCMODEL != USER_R47)
TO_QSPI const int16_t menu_HOME[]        = { ITM_DRG,                       ITM_YX,                     ITM_SQUARE,               ITM_10x,               ITM_EXP,                     ITM_op_j_pol,
                                             ITM_ROUND2,                    ITM_RMD,                    ITM_XFACT,                ITM_PARALLEL,          ITM_EE_EXP_TH,               ITM_LINPOL,
                                             ITM_FP,                        ITM_IP,                    -MNU_PREFIX,               CC_EE,                 ITM_RECT,                    ITM_POLAR             };
#else
TO_QSPI const int16_t menu_HOME[]        = { ITM_op_j,                      ITM_op_j_pol,               ITM_XFACT,                ITM_XTHROOT,           ITM_10x,                     ITM_EXP,
                                             ITM_ROUND2,                    ITM_RMD,                    ITM_SNAP,                 ITM_PARALLEL,          ITM_LINPOL,                  ITM_EE_EXP_TH,
                                             ITM_FP,                        ITM_IP,                     ITM_TIMER,                CC_EE,                 ITM_RECT,                    ITM_POLAR,            };
#endif //C47/R47
/*HOME MENU IS NOT USED ANYMORE*/
/*INSTEAD THIS IS USED TO POPULATE THE HOME DYNAMIC MENU (USER MENU)*/

TO_QSPI const int16_t menu_PREFIX[]      = { ITM_SI_k,                      ITM_SI_M,                   ITM_SI_G,                 ITM_SI_T,              ITM_SI_P,                    ITM_DSPCYCLE,
                                             ITM_SI_m,                      ITM_SI_u,                   ITM_SI_n,                 ITM_SI_p,              ITM_SI_f,                    ITM_DSP,
                                             ITM_SI_Ki,                     ITM_SI_Mi,                  ITM_SI_Gi,                ITM_SI_Ti,             ITM_SI_Pi,                   ITM_2TO10             };                //JM HOME



TO_QSPI const int16_t menu_PLOTFUNC[]    = {  VAR_LX,                       VAR_UX,                     ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,
                                              ITM_SCALE,                    ITM_PLOTRST,                ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,                           //JM GRAPH
                                              ITM_MZOOMY,                   ITM_PZOOMY,                 ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,

                                              ITM_SHOWY,                    ITM_SHOWX,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,                           //JM GRAPH
                                              ITM_CPXPLOT,                  ITM_LISTXY,                 ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,                           //JM GRAPH
                                             -MNU_GRAPHS,                   ITM_SNAP,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,                           //JM GRAPH

                                              ITM_PCROS,                    ITM_PPLUS,                  ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,                           //JM GRAPH
                                              ITM_PLINE,                    ITM_PBOX,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,                           //JM GRAPH
                                              ITM_PCURVE,                   ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,

                                              ITM_DIFF,                     ITM_INTG,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,
                                              ITM_RMS,                      ITM_SHADE,                  ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL,
                                              ITM_NVECT,                    ITM_VECT,                   ITM_NULL,                 ITM_NULL,              ITM_NULL,                    ITM_NULL};


TO_QSPI const int16_t menu_PLOT_STAT[]    = {
                                             ITM_SHOWX,                 ITM_SHOWY,                 ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,
                                             ITM_SCALE,                 ITM_PLOTRST,               ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,
                                             ITM_MZOOMY,                ITM_PZOOMY,                ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,

                                             ITM_PCROS,                 ITM_PPLUS,                 ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,
                                             ITM_PLINE,                 ITM_PBOX,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,
                                             ITM_PCURVE,                ITM_SNAP,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,

                                             ITM_DIFF,                  ITM_INTG,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,
                                             ITM_RMS,                   ITM_SHADE,                 ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,
                                             ITM_NVECT,                 ITM_VECT,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL,                  ITM_NULL                  };


TO_QSPI const int16_t menu_ALPHA[]       = { -MNU_ALPHA_OMEGA,             -MNU_ALPHAMATH,             -MNU_ALPHAMISC,           -MNU_ALPHAINTL,         ITM_T_LEFT_ARROW,            ITM_T_RIGHT_ARROW,
                                             -MNU_MyAlpha,                  ITM_XEDIT,                  ITM_XSWAP,                ITM_ASSIGN,            ITM_T_LLEFT_ARROW,           ITM_T_RRIGHT_ARROW,
                                             CHR_case,                      CHR_num,                    ITM_SCR,                  ITM_USERMODE,          ITM_T_UP_ARROW,              ITM_T_DOWN_ARROW                 };   //DL


TO_QSPI const int16_t menu_GAP_L[]       = { ITM_GAPPER_L,                  ITM_GAPCOM_L,               ITM_GAPDOT_L,             ITM_GAPNARAPO_L,       ITM_GAPSPC_L,                ITM_GAPNIL_L,
                                             ITM_GAPWIDPER_L,               ITM_GAPWIDCOM_L,            ITM_GAPWIDDOT_L,          ITM_GAPAPO_L,          ITM_GAPDBLSPC_L,             ITM_GAPUND_L,
                                             ITM_GRP_L,                     ITM_GRP1_L,                 ITM_GRP1_L_OF,            ITM_NULL,              ITM_GAPNARSPC_L,             ITM_NULL                         };

TO_QSPI const int16_t menu_GAP_RX[]      = { ITM_GAPPER_RX,                 ITM_GAPCOM_RX,              ITM_GAPDOT_RX,            ITM_GAPWIDPER_RX,      ITM_GAPWIDCOM_RX,            ITM_GAPWIDDOT_RX };


TO_QSPI const int16_t menu_GAP_R[]       = { ITM_GAPPER_R,                  ITM_GAPCOM_R,               ITM_GAPDOT_R,             ITM_GAPNARAPO_R,       ITM_GAPSPC_R,                ITM_GAPNIL_R,
                                             ITM_GAPWIDPER_R,               ITM_GAPWIDCOM_R,            ITM_GAPWIDDOT_R,          ITM_GAPAPO_R,          ITM_GAPDBLSPC_R,             ITM_GAPUND_R,
                                             ITM_GRP_R,                     ITM_NULL,                   ITM_NULL,                 ITM_NULL,              ITM_GAPNARSPC_R,             ITM_NULL                         };

TO_QSPI const int16_t menu_SHOW[]        = {  };
TO_QSPI const int16_t menu_CASHFL[]      = {  };
TO_QSPI const int16_t menu_AMORT[]       = {  };


#include "softmenuCatalogs.h"


TO_QSPI const softmenu_t softmenu[] = {
/* 000 */  {.menuItem = -MNU_MyMenu,      .numItems = 0,                                        .softkeyItem = NULL             }, // MyMenu must be the 1st
/* 001 */  {.menuItem = -MNU_MyAlpha,     .numItems = 0,                                        .softkeyItem = NULL             }, // Myalpha must be the 2nd
/* 002 */  {.menuItem = -MNU_PROGS,       .numItems = 0,                                        .softkeyItem = NULL             },
/* 003 */  {.menuItem = -MNU_VAR,         .numItems = 0,                                        .softkeyItem = NULL             }, // variable softmenus and
/* 004 */  {.menuItem = -MNU_PROG,        .numItems = 0,                                        .softkeyItem = NULL             }, // MUST be in the same
/* 005 */  {.menuItem = -MNU_MATRS,       .numItems = 0,                                        .softkeyItem = NULL             }, // order as the
/* 006 */  {.menuItem = -MNU_STRINGS,     .numItems = 0,                                        .softkeyItem = NULL             }, // dynamicSoftmenu area.
/* 007 */  {.menuItem = -MNU_DATES,       .numItems = 0,                                        .softkeyItem = NULL             }, //
/* 008 */  {.menuItem = -MNU_TIMES,       .numItems = 0,                                        .softkeyItem = NULL             }, // If you add or remove one:
/* 009 */  {.menuItem = -MNU_ANGLES,      .numItems = 0,                                        .softkeyItem = NULL             }, // don't forget to adjust
/* 010 */  {.menuItem = -MNU_SINTS,       .numItems = 0,                                        .softkeyItem = NULL             }, // NUMBER_OF_DYNAMIC_SOFTMENUS
/* 011 */  {.menuItem = -MNU_LINTS,       .numItems = 0,                                        .softkeyItem = NULL             }, // in defines.h
/* 012 */  {.menuItem = -MNU_REALS,       .numItems = 0,                                        .softkeyItem = NULL             },
/* 013 */  {.menuItem = -MNU_CPXS,        .numItems = 0,                                        .softkeyItem = NULL             },
/* 014 */  {.menuItem = -MNU_NUMBRS,      .numItems = 0,                                        .softkeyItem = NULL             },
/* 015 */  {.menuItem = -MNU_CONFIGS,     .numItems = 0,                                        .softkeyItem = NULL             },
/* 016 */  {.menuItem = -MNU_ALLVARS,     .numItems = 0,                                        .softkeyItem = NULL             },
/* 017 */  {.menuItem = -MNU_MVAR,        .numItems = 0,                                        .softkeyItem = NULL             },
/* 018 */  {.menuItem = -MNU_MENUS,       .numItems = 0,                                        .softkeyItem = NULL             },
/* 019 */  {.menuItem = -MNU_DYNAMIC,     .numItems = 0,                                        .softkeyItem = NULL             },
/* 020 */  {.menuItem = -ITM_MENU,        .numItems = 0,                                        .softkeyItem = NULL             },
/* 021 */  {.menuItem = -MNU_MENU,        .numItems = 0,                                        .softkeyItem = NULL             },
/* 022 */  {.menuItem = -MNU_TAMFLAG,     .numItems = sizeof(menu_TamFlag       )/sizeof(int16_t), .softkeyItem = menu_TamFlag        },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 023 */  {.menuItem = -MNU_SYSFL,       .numItems = sizeof(menu_SYSFL         )/sizeof(int16_t), .softkeyItem = menu_SYSFL          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 024 */  {.menuItem = -MNU_ALPHAINTL,   .numItems = sizeof(menu_alpha_INTL    )/sizeof(int16_t), .softkeyItem = menu_alpha_INTL     },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 025 */  {.menuItem = -MNU_ALPHAintl,   .numItems = sizeof(menu_alpha_intl    )/sizeof(int16_t), .softkeyItem = menu_alpha_intl     },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 026 */  {.menuItem = -MNU_ADV,         .numItems = sizeof(menu_ADV           )/sizeof(int16_t), .softkeyItem = menu_ADV            },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 027 */  {.menuItem = -MNU_Sfdx,        .numItems = sizeof(menu_Sfdx          )/sizeof(int16_t), .softkeyItem = menu_Sfdx           },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 028 */  {.menuItem = -MNU_BITS,        .numItems = sizeof(menu_BITS          )/sizeof(int16_t), .softkeyItem = menu_BITS           },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 029 */  {.menuItem = -MNU_CLK,         .numItems = sizeof(menu_CLK           )/sizeof(int16_t), .softkeyItem = menu_CLK            },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 030 */  {.menuItem = -MNU_CLR,         .numItems = sizeof(menu_CLR           )/sizeof(int16_t), .softkeyItem = menu_CLR            },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 031 */  {.menuItem = -MNU_CPX,         .numItems = sizeof(menu_CPX           )/sizeof(int16_t), .softkeyItem = menu_CPX            },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 032 */  {.menuItem = -MNU_DISP,        .numItems = sizeof(menu_DISP          )/sizeof(int16_t), .softkeyItem = menu_DISP           },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 033 */  {.menuItem = -MNU_EQN,         .numItems = sizeof(menu_EQN           )/sizeof(int16_t), .softkeyItem = menu_EQN            },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 034 */  {.menuItem = -MNU_1STDERIV,    .numItems = sizeof(menu_1stDeriv      )/sizeof(int16_t), .softkeyItem = menu_1stDeriv       },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 035 */  {.menuItem = -MNU_2NDDERIV,    .numItems = sizeof(menu_2ndDeriv      )/sizeof(int16_t), .softkeyItem = menu_2ndDeriv       },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 036 */  {.menuItem = -MNU_Sf,          .numItems = sizeof(menu_Sf            )/sizeof(int16_t), .softkeyItem = menu_Sf             },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 037 */  {.menuItem = -MNU_Solver,      .numItems = sizeof(menu_Solver        )/sizeof(int16_t), .softkeyItem = menu_Solver         },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 038 */  {.menuItem = -MNU_EXP,         .numItems = sizeof(menu_EXP           )/sizeof(int16_t), .softkeyItem = menu_EXP            },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 039 */  {.menuItem = -MNU_TRI,         .numItems = sizeof(menu_TRI           )/sizeof(int16_t), .softkeyItem = menu_TRI            },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 040 */  {.menuItem = -MNU_FIN,         .numItems = sizeof(menu_FIN           )/sizeof(int16_t), .softkeyItem = menu_FIN            },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 041 */  {.menuItem = -MNU_TVM,         .numItems = sizeof(menu_TVM           )/sizeof(int16_t), .softkeyItem = menu_TVM            },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 042 */  {.menuItem = -MNU_FLAGS,       .numItems = sizeof(menu_FLAGS         )/sizeof(int16_t), .softkeyItem = menu_FLAGS          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 043 */  {.menuItem = -MNU_INFO,        .numItems = sizeof(menu_INFO          )/sizeof(int16_t), .softkeyItem = menu_INFO           },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 044 */  {.menuItem = -MNU_INTS,        .numItems = sizeof(menu_INTS          )/sizeof(int16_t), .softkeyItem = menu_INTS           },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 045 */  {.menuItem = -MNU_LOOP,        .numItems = sizeof(menu_LOOP          )/sizeof(int16_t), .softkeyItem = menu_LOOP           },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 046 */  {.menuItem = -MNU_MATX,        .numItems = sizeof(menu_MATX          )/sizeof(int16_t), .softkeyItem = menu_MATX           },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 047 */  {.menuItem = -MNU_SIMQ,        .numItems = sizeof(menu_M_SIM_Q       )/sizeof(int16_t), .softkeyItem = menu_M_SIM_Q        },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 048 */  {.menuItem = -MNU_M_EDIT,      .numItems = sizeof(menu_M_EDIT        )/sizeof(int16_t), .softkeyItem = menu_M_EDIT         },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 049 */  {.menuItem = -MNU_MODE,        .numItems = sizeof(menu_MODE          )/sizeof(int16_t), .softkeyItem = menu_MODE           },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 050 */  {.menuItem = -MNU_PARTS,       .numItems = sizeof(menu_PARTS         )/sizeof(int16_t), .softkeyItem = menu_PARTS          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 051 */  {.menuItem = -MNU_PROB,        .numItems = sizeof(menu_PROB          )/sizeof(int16_t), .softkeyItem = menu_PROB           },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 052 */  {.menuItem = -MNU_T,           .numItems = sizeof(menu_t             )/sizeof(int16_t), .softkeyItem = menu_t              },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 053 */  {.menuItem = -MNU_F,           .numItems = sizeof(menu_F             )/sizeof(int16_t), .softkeyItem = menu_F              },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 054 */  {.menuItem = -MNU_CHI2,        .numItems = sizeof(menu_chi2          )/sizeof(int16_t), .softkeyItem = menu_chi2           },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 055 */  {.menuItem = -MNU_NORML,       .numItems = sizeof(menu_Norml         )/sizeof(int16_t), .softkeyItem = menu_Norml          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 056 */  {.menuItem = -ITM_MENU,        .numItems = 0,                                           .softkeyItem = NULL                },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 057 */  {.menuItem = -MNU_CAUCH,       .numItems = sizeof(menu_Cauch         )/sizeof(int16_t), .softkeyItem = menu_Cauch          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 058 */  {.menuItem = -MNU_EXPON,       .numItems = sizeof(menu_Expon         )/sizeof(int16_t), .softkeyItem = menu_Expon          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 059 */  {.menuItem = -MNU_LOGIS,       .numItems = sizeof(menu_Logis         )/sizeof(int16_t), .softkeyItem = menu_Logis          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 060 */  {.menuItem = -MNU_WEIBL,       .numItems = sizeof(menu_Weibl         )/sizeof(int16_t), .softkeyItem = menu_Weibl          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 061 */  {.menuItem = -MNU_BINOM,       .numItems = sizeof(menu_Binom         )/sizeof(int16_t), .softkeyItem = menu_Binom          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 062 */  {.menuItem = -MNU_GEOM,        .numItems = sizeof(menu_Geom          )/sizeof(int16_t), .softkeyItem = menu_Geom           },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 063 */  {.menuItem = -MNU_HYPER,       .numItems = sizeof(menu_Hyper         )/sizeof(int16_t), .softkeyItem = menu_Hyper          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 064 */  {.menuItem = -MNU_GEV,         .numItems = sizeof(menu_GEV           )/sizeof(int16_t), .softkeyItem = menu_GEV            },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 065 */  {.menuItem = -MNU_POISS,       .numItems = sizeof(menu_Poiss         )/sizeof(int16_t), .softkeyItem = menu_Poiss          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 066 */  {.menuItem = -MNU_PFN_1,       .numItems = sizeof(menu_PFN_1         )/sizeof(int16_t), .softkeyItem = menu_PFN_1          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 067 */  {.menuItem = -MNU_PFN_2,       .numItems = sizeof(menu_PFN_2         )/sizeof(int16_t), .softkeyItem = menu_PFN_2          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 068 */  {.menuItem = -MNU_STAT,        .numItems = sizeof(menu_STAT          )/sizeof(int16_t), .softkeyItem = menu_STAT           },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 069 */  {.menuItem = -MNU_PLOTTING,    .numItems = sizeof(menu_PLOTTING      )/sizeof(int16_t), .softkeyItem = menu_PLOTTING       },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 070 */  {.menuItem = -MNU_GRAPHS,      .numItems = sizeof(menu_GRAPHS        )/sizeof(int16_t), .softkeyItem = menu_GRAPHS         },       //Changed!
/* 071 */  {.menuItem = -MNU_PLOT_SCATR,  .numItems = sizeof(menu_PLOT_SCATR    )/sizeof(int16_t), .softkeyItem = menu_PLOT_SCATR     },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 072 */  {.menuItem = -MNU_PLOT_ASSESS, .numItems = sizeof(menu_PLOT_LR       )/sizeof(int16_t), .softkeyItem = menu_PLOT_LR        },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 073 */  {.menuItem = -MNU_HPLOT,       .numItems = sizeof(menu_HPLOT         )/sizeof(int16_t), .softkeyItem = menu_HPLOT          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 074 */  {.menuItem = -MNU_HIST,        .numItems = sizeof(menu_HIST          )/sizeof(int16_t), .softkeyItem = menu_HIST           },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 075 */  {.menuItem = -MNU_STK,         .numItems = sizeof(menu_STK           )/sizeof(int16_t), .softkeyItem = menu_STK            },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 076 */  {.menuItem = -MNU_TEST,        .numItems = sizeof(menu_TEST          )/sizeof(int16_t), .softkeyItem = menu_TEST           },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 077 */  {.menuItem = -MNU_XFN,         .numItems = sizeof(menu_XFN           )/sizeof(int16_t), .softkeyItem = menu_XFN            },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 078 */  {.menuItem = -MNU_ORTHOG,      .numItems = sizeof(menu_Orthog        )/sizeof(int16_t), .softkeyItem = menu_Orthog         },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 079 */  {.menuItem = -MNU_ELLIPT,      .numItems = sizeof(menu_Ellipt        )/sizeof(int16_t), .softkeyItem = menu_Ellipt         },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 080 */  {.menuItem = -MNU_CATALOG,     .numItems = sizeof(menu_CATALOG       )/sizeof(int16_t), .softkeyItem = menu_CATALOG        },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 081 */  {.menuItem = -MNU_CHARS,       .numItems = sizeof(menu_CHARS         )/sizeof(int16_t), .softkeyItem = menu_CHARS          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 082 */  {.menuItem = -MNU_VARS,        .numItems = sizeof(menu_VARS          )/sizeof(int16_t), .softkeyItem = menu_VARS           },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 083 */  {.menuItem = -MNU_ALPHA_OMEGA, .numItems = sizeof(menu_ALPHA_OMEGA   )/sizeof(int16_t), .softkeyItem = menu_ALPHA_OMEGA    },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 084 */  {.menuItem = -MNU_alpha_omega, .numItems = sizeof(menu_alpha_omega   )/sizeof(int16_t), .softkeyItem = menu_alpha_omega    },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 085 */  {.menuItem = -MNU_FCNS,        .numItems = sizeof(menu_FCNS          )/sizeof(int16_t), .softkeyItem = menu_FCNS           },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 086 */  {.menuItem = -MNU_ALPHAMATH,   .numItems = sizeof(menu_alphaMATH     )/sizeof(int16_t), .softkeyItem = menu_alphaMATH      },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 087 */  {.menuItem = -MNU_ALPHAMISC,   .numItems = sizeof(menu_alphaMisc     )/sizeof(int16_t), .softkeyItem = menu_alphaMisc      },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 088 */  {.menuItem = -MNU_ALPHAFN,     .numItems = sizeof(menu_alphaFN       )/sizeof(int16_t), .softkeyItem = menu_alphaFN        },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 089 */  {.menuItem = -MNU_ANGLECONV_43S,.numItems = sizeof(menu_AngleConv_43S)/sizeof(int16_t), .softkeyItem = menu_AngleConv_43S  },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 090 */  {.menuItem = -MNU_UNITCONV,    .numItems = sizeof(menu_UnitConv      )/sizeof(int16_t), .softkeyItem = menu_UnitConv       },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 091 */  {.menuItem = -MNU_CONVE,       .numItems = sizeof(menu_ConvE         )/sizeof(int16_t), .softkeyItem = menu_ConvE          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 092 */  {.menuItem = -MNU_CONVP,       .numItems = sizeof(menu_ConvP         )/sizeof(int16_t), .softkeyItem = menu_ConvP          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 093 */  {.menuItem = -MNU_CONVFP,      .numItems = sizeof(menu_ConvFP        )/sizeof(int16_t), .softkeyItem = menu_ConvFP         },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 094 */  {.menuItem = -MNU_CONVM,       .numItems = sizeof(menu_ConvM         )/sizeof(int16_t), .softkeyItem = menu_ConvM          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 095 */  {.menuItem = -MNU_CONVX,       .numItems = sizeof(menu_ConvX         )/sizeof(int16_t), .softkeyItem = menu_ConvX          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 096 */  {.menuItem = -MNU_CONVV,       .numItems = sizeof(menu_ConvV         )/sizeof(int16_t), .softkeyItem = menu_ConvV          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 097 */  {.menuItem = -MNU_CONVA,       .numItems = sizeof(menu_ConvA         )/sizeof(int16_t), .softkeyItem = menu_ConvA          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 098 */  {.menuItem = -MNU_CONVS,       .numItems = sizeof(menu_ConvS         )/sizeof(int16_t), .softkeyItem = menu_ConvS          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 099 */  {.menuItem = -MNU_CONVANG,     .numItems = sizeof(menu_ConvAng       )/sizeof(int16_t), .softkeyItem = menu_ConvAng        },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 100 */  {.menuItem = -MNU_CONVHUM,     .numItems = sizeof(menu_ConvHum       )/sizeof(int16_t), .softkeyItem = menu_ConvHum        },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 101 */  {.menuItem = -MNU_CONVYMMV,    .numItems = sizeof(menu_ConvYmmv      )/sizeof(int16_t), .softkeyItem = menu_ConvYmmv       },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 102 */  {.menuItem = -MNU_CONST,       .numItems = sizeof(menu_CONST         )/sizeof(int16_t), .softkeyItem = menu_CONST          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 103 */  {.menuItem = -MNU_IO,          .numItems = sizeof(menu_IO            )/sizeof(int16_t), .softkeyItem = menu_IO             },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 104 */  {.menuItem = -MNU_PRINT,       .numItems = sizeof(menu_PRINT         )/sizeof(int16_t), .softkeyItem = menu_PRINT          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 105 */  {.menuItem = -MNU_TAM,         .numItems = sizeof(menu_Tam           )/sizeof(int16_t), .softkeyItem = menu_Tam            },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 106 */  {.menuItem = -MNU_TAMCMP,      .numItems = sizeof(menu_TamCmp        )/sizeof(int16_t), .softkeyItem = menu_TamCmp         },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 107 */  {.menuItem = -MNU_TAMSTO,      .numItems = sizeof(menu_TamSto        )/sizeof(int16_t), .softkeyItem = menu_TamSto         },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 108 */  {.menuItem = -MNU_REG,         .numItems = sizeof(menu_Reg           )/sizeof(int16_t), .softkeyItem = menu_Reg            },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 109 */  {.menuItem = -MNU_TAMSHUFFLE,  .numItems = sizeof(menu_TamShuffle    )/sizeof(int16_t), .softkeyItem = menu_TamShuffle     },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 110 */  {.menuItem = -MNU_TAMLABEL,    .numItems = sizeof(menu_TamLabel      )/sizeof(int16_t), .softkeyItem = menu_TamLabel       },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 111 */  {.menuItem = -MNU_EQ_EDIT,     .numItems = sizeof(menu_Eim           )/sizeof(int16_t), .softkeyItem = menu_Eim            },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 112 */  {.menuItem = -MNU_TIMERF,      .numItems = sizeof(menu_Timer         )/sizeof(int16_t), .softkeyItem = menu_Timer          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 113 */  {.menuItem = -ITM_DELITM,      .numItems = sizeof(menu_DELITM        )/sizeof(int16_t), .softkeyItem = menu_DELITM         },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 114 */  {.menuItem = -MNU_ASN_N,       .numItems = sizeof(menu_ASN_N         )/sizeof(int16_t), .softkeyItem = menu_ASN_N          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 115 */  {.menuItem = -MNU_KEYS,        .numItems = sizeof(menu_KEYS          )/sizeof(int16_t), .softkeyItem = menu_KEYS           },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 116 */  {.menuItem = -MNU_CONVCHEF,    .numItems = sizeof(menu_ConvChef      )/sizeof(int16_t), .softkeyItem = menu_ConvChef       },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 117 */  {.menuItem = -MNU_PLOT_FUNC,   .numItems = sizeof(menu_PLOTFUNC      )/sizeof(int16_t), .softkeyItem = menu_PLOTFUNC       },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 118 */  {.menuItem = -MNU_ALPHA,       .numItems = sizeof(menu_ALPHA         )/sizeof(int16_t), .softkeyItem = menu_ALPHA          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 119 */  {.menuItem = -MNU_BASE,        .numItems = sizeof(menu_BASE          )/sizeof(int16_t), .softkeyItem = menu_BASE           },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 120 */  {.menuItem = -MNU_EE,          .numItems = sizeof(menu_EE            )/sizeof(int16_t), .softkeyItem = menu_EE             },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 121 */  {.menuItem = -MNU_TAMRCL,      .numItems = sizeof(menu_TamRcl        )/sizeof(int16_t), .softkeyItem = menu_TamRcl         },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 122 */  {.menuItem = -MNU_TRG_R47,     .numItems = sizeof(menu_TRG           )/sizeof(int16_t), .softkeyItem = menu_TRG            },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 123 */  {.menuItem = -MNU_PREF,        .numItems = sizeof(menu_PREF          )/sizeof(int16_t), .softkeyItem = menu_PREF           },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 124 */  {.menuItem = -MNU_REGR,        .numItems = sizeof(menu_REGR          )/sizeof(int16_t), .softkeyItem = menu_REGR           },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 125 */  {.menuItem = -MNU_MODEL,       .numItems = sizeof(menu_MODEL         )/sizeof(int16_t), .softkeyItem = menu_MODEL          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 126 */  {.menuItem = -MNU_MISC,        .numItems = sizeof(menu_Misc          )/sizeof(int16_t), .softkeyItem = menu_Misc           },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 127 */  {.menuItem = -MNU_STDNORML,    .numItems = sizeof(menu_StdNorml      )/sizeof(int16_t), .softkeyItem = menu_StdNorml       },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 128 */  {.menuItem = -MNU_TAMALPHA,    .numItems = sizeof(menu_TamAlpha      )/sizeof(int16_t), .softkeyItem = menu_TamAlpha       },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 129 */  {.menuItem = -MNU_TRG_C47,     .numItems = sizeof(menu_TRG_C47       )/sizeof(int16_t), .softkeyItem = menu_TRG_C47        },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 130 */  {.menuItem = -MNU_TRG_C47_MORE,.numItems = sizeof(menu_TRG_C47_MORE  )/sizeof(int16_t), .softkeyItem = menu_TRG_C47_MORE   },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 131 */  {.menuItem = -MNU_TAMNONREG,   .numItems = sizeof(menu_TamNonReg     )/sizeof(int16_t), .softkeyItem = menu_TamNonReg      },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 132 */  {.menuItem = -MNU_TAMINDIRECT, .numItems = sizeof(menu_TamIndirect   )/sizeof(int16_t), .softkeyItem = menu_TamIndirect    },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 133 */  {.menuItem = -MNU_BLUE_C47,    .numItems = sizeof(menu_BLUE_C47      )/sizeof(int16_t), .softkeyItem = menu_BLUE_C47       },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 134 */  {.menuItem = -MNU_GAP_L,       .numItems = sizeof(menu_GAP_L         )/sizeof(int16_t), .softkeyItem = menu_GAP_L          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 135 */  {.menuItem = -MNU_GAP_RX,      .numItems = sizeof(menu_GAP_RX        )/sizeof(int16_t), .softkeyItem = menu_GAP_RX         },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 136 */  {.menuItem = -MNU_GAP_R,       .numItems = sizeof(menu_GAP_R         )/sizeof(int16_t), .softkeyItem = menu_GAP_R          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 137 */  {.menuItem = -MNU_PREFIX,      .numItems = sizeof(menu_PREFIX        )/sizeof(int16_t), .softkeyItem = menu_PREFIX         },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 138 */  {.menuItem = -MNU_2233,        .numItems = 0,                                           .softkeyItem = NULL        },       // NOTE Next menu to use!
/* 139 */  {.menuItem = -MNU_RESETS,      .numItems = sizeof(menu_RESETS        )/sizeof(int16_t), .softkeyItem = menu_RESETS         },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 140 */  {.menuItem = -MNU_RIBBONS,     .numItems = sizeof(menu_RIBBONS       )/sizeof(int16_t), .softkeyItem = menu_RIBBONS        },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 141 */  {.menuItem = -MNU_INL_TST,     .numItems = sizeof(menu_Inl_Tst       )/sizeof(int16_t), .softkeyItem = menu_Inl_Tst        },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 142 */  {.menuItem = -MNU_DELETE,      .numItems = sizeof(menu_DELETE        )/sizeof(int16_t), .softkeyItem = menu_DELETE         },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 143 */  {.menuItem = -MNU_YESNO,       .numItems = sizeof(menu_YESNO         )/sizeof(int16_t), .softkeyItem = menu_YESNO          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 144 */  {.menuItem = -MNU_DISTR,       .numItems = sizeof(menu_DISTR         )/sizeof(int16_t), .softkeyItem = menu_DISTR          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 145 */  {.menuItem = -MNU_FLG,         .numItems = sizeof(menu_Flg           )/sizeof(int16_t), .softkeyItem = menu_Flg            },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 146 */  {.menuItem = -MNU_SHOW,        .numItems = sizeof(menu_SHOW          )/sizeof(int16_t), .softkeyItem = menu_SHOW           },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 147 */  {.menuItem = -MNU_Solver_TOOL, .numItems = sizeof(menu_Solver_TOOL   )/sizeof(int16_t), .softkeyItem = menu_Solver_TOOL    },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 148 */  {.menuItem = -MNU_Sf_TOOL,     .numItems = sizeof(menu_Sf_TOOL       )/sizeof(int16_t), .softkeyItem = menu_Sf_TOOL        },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 149 */  {.menuItem = -MNU_CASHFL,      .numItems = sizeof(menu_CASHFL        )/sizeof(int16_t), .softkeyItem = menu_CASHFL         },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 150 */  {.menuItem = -MNU_AMORT,       .numItems = sizeof(menu_AMORT         )/sizeof(int16_t), .softkeyItem = menu_AMORT          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 151 */  {.menuItem = -MNU_Grapher,     .numItems = sizeof(menu_Grapher       )/sizeof(int16_t), .softkeyItem = menu_Grapher        },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 152 */  {.menuItem = -MNU_AUDIO,       .numItems = sizeof(menu_AUDIO         )/sizeof(int16_t), .softkeyItem = menu_AUDIO          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-06-16 jm
/* 153 */  {.menuItem = -MNU_TAMNONREGMAX,.numItems = sizeof(menu_TamNonRegMax  )/sizeof(int16_t), .softkeyItem = menu_TamNonRegMax   },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 154 */  {.menuItem = -MNU_PFN_3,       .numItems = sizeof(menu_PFN_3         )/sizeof(int16_t), .softkeyItem = menu_PFN_3          },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 155 */  {.menuItem = -MNU_TAMMENU,     .numItems = sizeof(menu_TamMenu       )/sizeof(int16_t), .softkeyItem = menu_TamMenu        },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 156 */  {.menuItem = -MNU_NUMTHEORY,   .numItems = sizeof(menu_NUMTHEORY     )/sizeof(int16_t), .softkeyItem = menu_NUMTHEORY      },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 157 */  {.menuItem = -MNU_PLOT_STAT,   .numItems = sizeof(menu_PLOT_STAT     )/sizeof(int16_t), .softkeyItem = menu_PLOT_STAT      },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 158 */  {.menuItem = -MNU_TAMNONREGTRK,.numItems = sizeof(menu_TamNonRegTrk  )/sizeof(int16_t), .softkeyItem = menu_TamNonRegTrk   },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 159 */  {.menuItem = -MNU_PARETO,      .numItems = sizeof(menu_Pareto        )/sizeof(int16_t), .softkeyItem = menu_Pareto         },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 160 */  {.menuItem = -MNU_VECCONV,     .numItems = sizeof(menu_VECCONV       )/sizeof(int16_t), .softkeyItem = menu_VECCONV        },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 161 */  {.menuItem = -MNU_BITSET,      .numItems = sizeof(menu_BITSET        )/sizeof(int16_t), .softkeyItem = menu_BITSET         },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 162 */  {.menuItem = -MNU_AIMCATALOG,  .numItems = sizeof(menu_AIMCATALOG    )/sizeof(int16_t), .softkeyItem = menu_AIMCATALOG     },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 163 */  {.menuItem = -MNU_EIMCATALOG,  .numItems = sizeof(menu_EIMCATALOG    )/sizeof(int16_t), .softkeyItem = menu_EIMCATALOG     },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 164 */  {.menuItem = -MNU_FCNS_EIM,    .numItems = sizeof(menu_FCNS_EIM      )/sizeof(int16_t), .softkeyItem = menu_FCNS_EIM       },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 165 */  {.menuItem = -MNU_XXFCNS,      .numItems = sizeof(menu_XXFCNS        )/sizeof(int16_t), .softkeyItem = menu_XXFCNS         },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 166 */  {.menuItem = -MNU_MULTSTK,     .numItems = sizeof(menu_MULTSTK       )/sizeof(int16_t), .softkeyItem = menu_MULTSTK        },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 167 */  {.menuItem = -MNU_TAMLBLONLY,  .numItems = sizeof(menu_TamLabelOnly  )/sizeof(int16_t), .softkeyItem = menu_TamLabelOnly   },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 168 */  {.menuItem = -MNU_TAMVARONLY,  .numItems = sizeof(menu_TamVarOnly    )/sizeof(int16_t), .softkeyItem = menu_TamVarOnly     },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references. 2024-02-21 jm
/* 169 */  {.menuItem = -MNU_UNIFORM,     .numItems = sizeof(menu_Uniform       )/sizeof(int16_t), .softkeyItem = menu_Uniform        },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references.
/* 170 */  {.menuItem = -MNU_DISUNIFORM,  .numItems = sizeof(menu_DisUniform    )/sizeof(int16_t), .softkeyItem = menu_DisUniform     },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references.
/* 171 */  {.menuItem = -MNU_DEV,         .numItems = sizeof(menu_Dev           )/sizeof(int16_t), .softkeyItem = menu_Dev            },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references.
/* 171 */  {.menuItem = -MNU_TAMSTO_TVM,  .numItems = sizeof(menu_TamStoTVM     )/sizeof(int16_t), .softkeyItem = menu_TamStoTVM      },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references.
/* 171 */  {.menuItem = -MNU_TAMRCL_TVM,  .numItems = sizeof(menu_TamRclTVM     )/sizeof(int16_t), .softkeyItem = menu_TamRclTVM      },       // NOTE !! do not add menus here, add them at the end. The menu numbers are fixed for the Wiki references.

/* 172 */  {.menuItem =  0,               .numItems = 0,                                           .softkeyItem = NULL                }
};


dynamicSoftmenu_t dynamicSoftmenu[NUMBER_OF_DYNAMIC_SOFTMENUS] = {
/*   0 */  {.menuItem = -MNU_MyMenu,  .numItems = 0, .menuContent = NULL},
/*   1 */  {.menuItem = -MNU_MyAlpha, .numItems = 0, .menuContent = NULL},
/*   2 */  {.menuItem = -MNU_PROGS,   .numItems = 0, .menuContent = NULL},
/*   3 */  {.menuItem = -MNU_VAR,     .numItems = 0, .menuContent = NULL},
/*   4 */  {.menuItem = -MNU_PROG,    .numItems = 0, .menuContent = NULL},
/*   5 */  {.menuItem = -MNU_MATRS,   .numItems = 0, .menuContent = NULL},
/*   6 */  {.menuItem = -MNU_STRINGS, .numItems = 0, .menuContent = NULL},
/*   7 */  {.menuItem = -MNU_DATES,   .numItems = 0, .menuContent = NULL},
/*   8 */  {.menuItem = -MNU_TIMES,   .numItems = 0, .menuContent = NULL},
/*   9 */  {.menuItem = -MNU_ANGLES,  .numItems = 0, .menuContent = NULL},
/*  10 */  {.menuItem = -MNU_SINTS,   .numItems = 0, .menuContent = NULL},
/*  11 */  {.menuItem = -MNU_LINTS,   .numItems = 0, .menuContent = NULL},
/*  12 */  {.menuItem = -MNU_REALS,   .numItems = 0, .menuContent = NULL},
/*  13 */  {.menuItem = -MNU_CPXS,    .numItems = 0, .menuContent = NULL},
/*  14 */  {.menuItem = -MNU_NUMBRS,  .numItems = 0, .menuContent = NULL},
/*  15 */  {.menuItem = -MNU_CONFIGS, .numItems = 0, .menuContent = NULL},
/*  16 */  {.menuItem = -MNU_ALLVARS, .numItems = 0, .menuContent = NULL},
/*  17 */  {.menuItem = -MNU_MVAR,    .numItems = 0, .menuContent = NULL},
/*  18 */  {.menuItem = -MNU_MENUS,   .numItems = 0, .menuContent = NULL},
/*  19 */  {.menuItem = -MNU_DYNAMIC, .numItems = 0, .menuContent = NULL},
/*  20 */  {.menuItem = -ITM_MENU   , .numItems = 0, .menuContent = NULL},
/*  20 */  {.menuItem = -MNU_MENU   , .numItems = 0, .menuContent = NULL},
};



uint8_t *getNthString(uint8_t *ptr, int16_t n) { // Starting with string 0 (the 1st string is returned for n=0)
  while(n) {
    ptr += stringByteLength((char *)ptr) + 1;
    n--;
  }

  return ptr;
}



void fnDynamicMenu(uint16_t unusedButMandatoryParameter) {
  printf("fnDynamicMenu:\n       softmenuId = %d\n  dynamicMenuItem = %d\n", softmenuStack[0].softmenuId, dynamicMenuItem);
}



#if !defined(TESTSUITE_BUILD)
  static void initVariableSoftmenu(int16_t menu);
#endif // !TESTSUITE_BUILD



void fnOpenMenu(uint16_t menu) {
#if !defined(TESTSUITE_BUILD)
  int16_t i, numItems;
  i=0;
  while(softmenu[i].menuItem != 0) {
    if(softmenu[i].menuItem == -menu) {
      if(i < NUMBER_OF_DYNAMIC_SOFTMENUS) {
        initVariableSoftmenu(i);
        numItems = dynamicSoftmenu[i].numItems;
      }
      else {
        numItems = softmenu[i].numItems;
      }
      break;
    }
    i++;
  }

  if(softmenu[i].menuItem == 0) {                                              // Should never happen as menu is checked before the call fnOpenMenu
    displayCalcErrorMessage(ERROR_UNDEF_MENU, ERR_REGISTER_LINE, REGISTER_X);  // No check for FLAG_IGN1ER to ensure this error case is reported if it happens anyway
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "menu '%d' is not a valid menu item", menu);
      moreInfoOnError("In function fnOpenMenu:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    menuPageNumber = 1;                                            // Restore default menu page number
    return;
  }

  if((menuPageNumber > 0) && (menuPageNumber <= 9) && (((numItems > 18 * (menuPageNumber-1))) || ((numItems == 0) && (menuPageNumber == 1)))) {    // Check if menuPageNumber is within the menu
    if(menu == MNU_DYNAMIC) {
      for(i=0; i<numberOfUserMenus; i++) {
        if(compareString(tmpString, userMenus[i].menuName, CMP_NAME) == 0) {
          currentUserMenu = i;
          break;
        }
      }
    }
    enterAsmModeIfMenuIsACatalog(-menu);                             // Set catalog
    if(menu == MNU_CONVCHEF || menu == MNU_CONVV) {
      lastCatalogPosition[catalog] = getSystemFlag(FLAG_US) ? 18 : 0;
    }
    else {
      lastCatalogPosition[catalog] = 18 * (menuPageNumber-1);          // To open the menu at the right page
    }
    showSoftmenu(-menu);
    lastCatalogPosition[CATALOG_NONE] = 0;                           // Return to default page for non catalog menus
  }
  else {
    if(getSystemFlag(FLAG_IGN1ER)) {
      clearSystemFlag(FLAG_IGN1ER);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Page Number %" PRIu16 " is not a valid page for the menu %" PRIu16 "", menuPageNumber,menu);
        moreInfoOnError("In function fnOpenMenu:", errorMessage, "ignored since IGN1ER system flag was set", NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Page Number %" PRIu16 " is not a valid page for the menu %" PRIu16 "", menuPageNumber,menu);
        moreInfoOnError("In function fnOpenMenu:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
  menuPageNumber = 1;                                                // Restore default menu page number
#endif // !TESTSUITE_BUILD
}

void _stripMenuName(char *buffer, char *name) {
  int16_t i = 0;

  menuPageNumber = 1;                              // Default menu page number is 1

  while(buffer[i] !=0) {
    if((buffer[i] == (char)(STD_CR[0])) && (buffer[i+1] == (char)(STD_CR[1]))) {
      if((buffer[i+3] == 0) && (buffer[i+2] > STD_0[0]) && (buffer[i+2] <= STD_9[0])) {
        name[i] = 0;
        menuPageNumber = buffer[i+2] - STD_0[0];   // Get menu page number from the menu name string if it's there
      }
      else {
        menuPageNumber = 0;                        // If not a 1-9 single digit, menuPageNumber is invalid
      }
      break;
    }
    else {
      name[i] = buffer[i];
      i++;
    }
  }
  name[i] = 0;
}


int16_t findMenu(char *buffer) {
  int16_t menu_id = INVALID_MENU;
#if !defined(TESTSUITE_BUILD)
  char name[16];
  int16_t i, menuItem;
  bool found = false;

  _stripMenuName(buffer,name);

  i = 0;
  menuItem = MNU_MyMenu;

  while(menuItem != 0) {      // Search in predefined menus
    if((indexOfItems[menuItem].status & CAT_STATUS) == CAT_MENU) {
      if(compareString(name, indexOfItems[menuItem].itemCatalogName, CMP_CLEANED_STRING_ONLY) == 0) {
        found = true;
        menu_id = menuItem;
        break;
      }
    }
    i++;
    menuItem = -softmenu[i].menuItem;
  }

  if(!found) {                // If not found in predefined menus, search in user menus
    for(i=0; i<numberOfUserMenus; i++) {
      if(compareString(name, userMenus[i].menuName, CMP_NAME) == 0) {
        int16_t len = stringByteLength(name)+1;
        found = true;
        menu_id = MNU_DYNAMIC;
        xcopy(tmpString, name, len);
        break;
      }
    }
  }

  if(!found) {                // If still not found search in predefined hidden menus. Placed after the user menus, so that a user menu DEV will override a hidden menu.
    i = 0;
    menuItem = MNU_MyMenu;
    while(menuItem != 0) {      // Search in predefined menus for hidden menus
      if((indexOfItems[menuItem].status & CAT_STATUS) == CAT_MNUH) {
        if(compareString(name, indexOfItems[menuItem].itemCatalogName, CMP_CLEANED_STRING_ONLY) == 0) {
          found = true;
          menu_id = menuItem;
          break;
        }
      }
      i++;
      menuItem = -softmenu[i].menuItem;
    }
  }


  if(menu_id == INVALID_MENU) {
    menuPageNumber = 1;       // Restore default menu page number
  }
#endif // !TESTSUITE_BUILD
  return menu_id;
}

void _add_digitglyph(char* tmp, int16_t xx) {
  tmp[0] = 0;

  stringCopy(tmp, STD_0);
  if(xx >= 1 && xx <= 9) {
    tmp[0] += xx;
  }
}

void fnGetMenu(uint16_t funusedButMandatoryParameter) {
#if !defined(TESTSUITE_BUILD)
  int16_t lenInBytes;
  int16_t menuItem   = -softmenu[softmenuStack[0].softmenuId].menuItem;
  int16_t userMenuId = softmenuStack[0].userMenuId;
  char    menuName[12];
  int16_t firstItem;

  liftStack();
  setSystemFlag(FLAG_ASLIFT);

  if(menuItem != MNU_DYNAMIC) {
    lenInBytes = stringByteLength(indexOfItems[menuItem].itemCatalogName) + 1;
    xcopy(menuName, indexOfItems[menuItem].itemCatalogName, lenInBytes);
    firstItem = softmenuStack[0].firstItem;
    if(firstItem >= 18) {
      char tmp[16];
      lenInBytes--;
      menuName[lenInBytes++] = (uint8_t)(STD_CR[0]);
      menuName[lenInBytes++] = (uint8_t)(STD_CR[1]);
      menuName[lenInBytes] = 0;
      _add_digitglyph(tmp, (firstItem / 18)+1); stringCopy(menuName + stringByteLength(menuName), tmp);
      //_add_digitglyph(tmp, firstItem % 10); stringCopy(menuName + stringByteLength(menuName), tmp);
      lenInBytes = stringByteLength(menuName) + 1;
    }
    reallocateRegister(REGISTER_X, dtString, TO_BLOCKS(lenInBytes), amNone);
    if(lastErrorCode == ERROR_RAM_FULL) {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      fnUndo(NOPARAM);
      return;
    }
    xcopy(REGISTER_STRING_DATA(REGISTER_X), menuName, lenInBytes);
    //xcopy(REGISTER_STRING_DATA(REGISTER_X), indexOfItems[menuItem].itemCatalogName, lenInBytes);
  }
  else {
    lenInBytes = stringByteLength(userMenus[userMenuId].menuName) + 1;

    reallocateRegister(REGISTER_X, dtString, TO_BLOCKS(lenInBytes), amNone);
    if(lastErrorCode == ERROR_RAM_FULL) {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      fnUndo(NOPARAM);
      return;
    }
    xcopy(REGISTER_STRING_DATA(REGISTER_X), userMenus[userMenuId].menuName, lenInBytes);
  }
#endif // !TESTSUITE_BUILD
}


#if !defined(TESTSUITE_BUILD)
  static int sortMenu(void const *a, void const *b) {
    return compareString(a, b, CMP_EXTENSIVE);
  }



  static bool_t _filterDataType(calcRegister_t regist, dataType_t typeFilter, bool_t isAngular) {
    dataType_t dt = getRegisterDataType(regist);
    if(dt != dtReal34 && dt == typeFilter) {
      return true;
    }
    if(typeFilter == dtReal34Matrix && dt == dtComplex34Matrix) {
      return true;
    }
    if(typeFilter == dtReal34 && dt == dtReal34) {
      if(isAngular) {
        return getRegisterAngularMode(regist) != amNone;
      }
      if(!isAngular) {
        return getRegisterAngularMode(regist) == amNone;
      }
    }
    if(typeFilter == dtNumbers && (dt == dtLongInteger || dt == dtReal34 || dt == dtComplex34)) {
      return true;
    }
    return false;
  }



  static void _dynmenuConstructVars(int16_t menu, bool_t applyFilter, dataType_t typeFilter, bool_t isAngular) {
    uint16_t numberOfBytes, numberOfVars;
    uint8_t *ptr;
    numberOfBytes = 1;
    numberOfVars = 0;
    memset(tmpString, 0, TMP_STR_LENGTH);
    for(int i=0; i<numberOfNamedVariables; i++) {
      calcRegister_t regist = i+FIRST_NAMED_VARIABLE;
      if(!applyFilter || _filterDataType(regist, typeFilter, isAngular)) {
        xcopy(tmpString + 15 * numberOfVars, allNamedVariables[i].variableName + 1, allNamedVariables[i].variableName[0]);
        if((softmenu[softmenuStack[2].softmenuId].menuItem == -ITM_DELITM) &&       // Don't include "STATS", "HISTO", "Mat_A", "Mat_B" and "Mat_X" for DELITM
           ((compareString(tmpString + 15 * numberOfVars, "STATS", CMP_NAME) == 0) || (compareString(tmpString + 15 * numberOfVars, "HISTO", CMP_NAME) == 0) ||
            (compareString(tmpString + 15 * numberOfVars, "Mat_A", CMP_NAME) == 0) || (compareString(tmpString + 15 * numberOfVars, "Mat_B", CMP_NAME) == 0) ||
            (compareString(tmpString + 15 * numberOfVars, "Mat_X", CMP_NAME) == 0))) {
          memset(tmpString + 15 * numberOfVars, 0, 15);
        }
        else {
          numberOfVars++;
          numberOfBytes += 1 + allNamedVariables[i].variableName[0];
        }
      }
    }
    if(softmenu[softmenuStack[2].softmenuId].menuItem != -ITM_DELITM) {            // Don't include reserved variables for DELITM
      for(int i=FIRST_NAMED_RESERVED_VARIABLE-FIRST_RESERVED_VARIABLE; i<NUMBER_OF_RESERVED_VARIABLES; i++) {
        calcRegister_t regist = i+FIRST_RESERVED_VARIABLE;
        if((!applyFilter || _filterDataType(regist, typeFilter, isAngular))) {
          xcopy(tmpString + 15 * numberOfVars, allReservedVariables[i].reservedVariableName + 1, allReservedVariables[i].reservedVariableName[0]);
          numberOfVars++;
          numberOfBytes += 1 + allReservedVariables[i].reservedVariableName[0];
        }
      }
    }

    if(numberOfVars != 0) {
      qsort(tmpString, numberOfVars, 15, sortMenu);
    }

    ptr = malloc(numberOfBytes);
    dynamicSoftmenu[menu].menuContent = ptr;
    for(int i=0; i<numberOfVars; i++) {
      int16_t len = stringByteLength(tmpString + 15*i) + 1;
      xcopy(ptr, tmpString + 15*i, len);
      ptr += len;
    }

    dynamicSoftmenu[menu].numItems = numberOfVars;
  }



  static void _dynmenuConstructMVarsFromPgm(uint16_t label, uint16_t *numberOfBytes, uint16_t *numberOfVars) {
    uint8_t *step;
    step = labelList[label].instructionPointer;
    while((*numberOfVars < 18) && checkOpCodeOfStep(step, ITM_MVAR) && *(step + 2) == STRING_LABEL_VARIABLE) {
      xcopy(tmpString + *numberOfBytes, step + 4, *(step + 3));
      (void)findOrAllocateNamedVariable(tmpString + *numberOfBytes);
      *numberOfBytes += *(step + 3) + 1;
      (*numberOfVars)++;
      step = findNextStep(step);
    }
  }



  static void _dynmenuConstructMVars(int16_t menu) {
    uint16_t numberOfBytes = 0;
    uint16_t numberOfVars = 0;
    memset(tmpString, 0, TMP_STR_LENGTH);

    if(currentMvarLabel != INVALID_VARIABLE) {
      _dynmenuConstructMVarsFromPgm(currentMvarLabel - FIRST_LABEL, &numberOfBytes, &numberOfVars);
    }
    else if(currentSolverStatus & SOLVER_STATUS_USES_FORMULA) {
      char *bufPtr = tmpString;
      uint8_t errorCode = lastErrorCode;
      lastErrorCode = ERROR_NONE;
      parseEquation(currentFormula, EQUATION_PARSER_MVAR, tmpString + TMP_STR_LENGTH - AIM_BUFFER_LENGTH, tmpString);
      while(*bufPtr != 0 || numberOfVars < 6) {
        numberOfVars += 1;
        numberOfBytes += stringByteLength(bufPtr) + 1;
        bufPtr += stringByteLength(bufPtr) + 1;
      }
      lastErrorCode = errorCode;
    }
    else {
      _dynmenuConstructMVarsFromPgm(currentSolverProgram, &numberOfBytes, &numberOfVars);
    }

    dynamicSoftmenu[menu].menuContent = malloc(numberOfBytes);
    xcopy(dynamicSoftmenu[menu].menuContent, tmpString, numberOfBytes);
    dynamicSoftmenu[menu].numItems = numberOfVars;
  }



  static void _dynmenuConstructUser(int16_t menu) {
    userMenuItem_t *menuData = (dynamicSoftmenu[menu].menuItem == -MNU_DYNAMIC) ? userMenus[currentUserMenu].menuItem : (dynamicSoftmenu[menu].menuItem == -MNU_MyAlpha) ? userAlphaItems : userMenuItems;
    int16_t i, numberOfBytes = 1;
    uint8_t *ptr;

    for(i = 0; i < 18; i++) {
      if(menuData[i].argumentName[0] != 0) {
        numberOfBytes += stringByteLength(menuData[i].argumentName) + 1;
      }
      else if(menuData[i].item == ITM_NOP || menuData[i].item == ITM_NULL) {
        numberOfBytes += 1;
      }
      else if( indexOfItems[abs(menuData[i].item)].itemCatalogName[0] == 0 || (menuData[i].item == ITM_op_j || menuData[i].item == ITM_op_j_pol || menuData[i].item == ITM_op_a || menuData[i].item == ITM_op_a2)) {
        numberOfBytes += stringByteLength(indexOfItems[abs(menuData[i].item)].itemSoftmenuName) + 1;
      }
      else {
        numberOfBytes += stringByteLength(indexOfItems[abs(menuData[i].item)].itemCatalogName) + 1;
      }
    }
    ptr = malloc(numberOfBytes);
    dynamicSoftmenu[menu].menuContent = ptr;
    for(i = 0; i < 18; i++) {
      const char *lbl;
      if(menuData[i].argumentName[0] != 0) {
        lbl = menuData[i].argumentName;
      }
      else if(menuData[i].item == ITM_NULL) {
        lbl = "";
      }
      else if(indexOfItems[abs(menuData[i].item)].itemCatalogName[0] == 0 || (menuData[i].item == ITM_op_j || menuData[i].item == ITM_op_j_pol || menuData[i].item == ITM_op_a || menuData[i].item == ITM_op_a2)) {
        lbl = indexOfItems[abs(menuData[i].item)].itemSoftmenuName;
      }
      else {
        lbl = indexOfItems[abs(menuData[i].item)].itemCatalogName;
      }
      int16_t len = stringByteLength(lbl) + 1;
      xcopy(ptr, lbl, len);
      ptr += len;
    }
    dynamicSoftmenu[menu].numItems = (numberOfBytes <= 19) ? 0 : 18;
  }



  static void initVariableSoftmenu(int16_t menu) {
    int16_t i, numberOfBytes, numberOfGlobalLabels;
    uint8_t *ptr;

    #if defined(PC_BUILD)
      //printf("initvariableSoftMenu (cachedDynamicMenu=%i)",cachedDynamicMenu);
    #endif // PC_BUILD
    free(dynamicSoftmenu[menu].menuContent);

    switch(-dynamicSoftmenu[menu].menuItem) {
      case MNU_MyAlpha: {
        _dynmenuConstructUser(menu);
                        break;
      }

      case MNU_MyMenu: {
        _dynmenuConstructUser(menu);
        break;
      }

      case MNU_VAR: {
        _dynmenuConstructVars(menu, false, 0, false);
        break;
      }

      case MNU_PROG:
      case MNU_PROGS: {
        numberOfBytes = 1;
        numberOfGlobalLabels = 0;
        memset(tmpString, 0, TMP_STR_LENGTH);
        for(i=0; i<numberOfLabels; i++) {
          if(labelList[i].step > 0) { // Global label
            xcopy(tmpString + 15 * numberOfGlobalLabels, labelList[i].labelPointer + 1, labelList[i].labelPointer[0]);
            numberOfGlobalLabels++;
            numberOfBytes += 1 + labelList[i].labelPointer[0];
          }
        }

        if(numberOfGlobalLabels != 0) {
          qsort(tmpString, numberOfGlobalLabels, 15, sortMenu);
        }

        ptr = malloc(numberOfBytes);
        dynamicSoftmenu[menu].menuContent = ptr;
        for(i=0; i<numberOfGlobalLabels; i++) {
          int16_t len = stringByteLength(tmpString + 15*i) + 1;
          xcopy(ptr, tmpString + 15*i, len);
          ptr += len;
        }

        dynamicSoftmenu[menu].numItems = numberOfGlobalLabels;
        break;
      }

      case MNU_MATRS: {
        _dynmenuConstructVars(menu, true, dtReal34Matrix, false);
        break;
      }

      case MNU_STRINGS: {
        _dynmenuConstructVars(menu, true, dtString, false);
        break;
      }

      case MNU_DATES: {
        _dynmenuConstructVars(menu, true, dtDate, false);
        break;
      }

      case MNU_TIMES: {
        _dynmenuConstructVars(menu, true, dtTime, false);
        break;
      }

      case MNU_ANGLES: {
        _dynmenuConstructVars(menu, true, dtReal34, true);
        break;
      }

      case MNU_SINTS: {
        _dynmenuConstructVars(menu, true, dtShortInteger, false);
        break;
      }

      case MNU_LINTS: {
        _dynmenuConstructVars(menu, true, dtLongInteger, false);
        break;
      }

      case MNU_REALS: {
        _dynmenuConstructVars(menu, true, dtReal34, false);
        break;
      }

      case MNU_CPXS: {
        _dynmenuConstructVars(menu, true, dtComplex34, false);
        break;
      }

      case MNU_NUMBRS: {
        _dynmenuConstructVars(menu, true, dtNumbers, false);
        break;
      }

      case MNU_CONFIGS: {
        _dynmenuConstructVars(menu, true, dtConfig, false);
        break;
      }

      case MNU_ALLVARS: {
        _dynmenuConstructVars(menu, false, 0, false);
        break;
      }

      case MNU_MVAR: {
        _dynmenuConstructMVars(menu);
        break;
      }

      case MNU_MENU:
      case MNU_MENUS: {
        numberOfBytes = 1;
        numberOfGlobalLabels = 0;
        memset(tmpString, 0, TMP_STR_LENGTH);
        if(softmenu[softmenuStack[1].softmenuId].menuItem != -ITM_DELITM) {     // Don't include predefined menus for DELITM
          for(i=0; i<LAST_ITEM; i++) {
            if((indexOfItems[i].status & CAT_STATUS) == CAT_MENU && indexOfItems[i].itemCatalogName[0] != 0 /* && i != MNU_CATALOG && i != MNU_MENUS && i != MNU_MENU */) {
              int16_t len = stringByteLength(indexOfItems[i].itemCatalogName);
              xcopy(tmpString + 15 * numberOfGlobalLabels, indexOfItems[i].itemCatalogName, len);
              numberOfGlobalLabels++;
              numberOfBytes += 1 + len;
            }
          }
        }

        for(i=0; i<numberOfUserMenus; i++) {
          int16_t len = stringByteLength(userMenus[i].menuName);
          if((softmenu[softmenuStack[1].softmenuId].menuItem != -ITM_DELITM) ||              // Don't show HOME & P.FN in the menus to delete
             ((compareString("HOME", userMenus[i].menuName, CMP_NAME) != 0) && (compareString("P.FN", userMenus[i].menuName, CMP_NAME) != 0))) {
            xcopy(tmpString + 15 * numberOfGlobalLabels, userMenus[i].menuName, len);
            numberOfGlobalLabels++;
            numberOfBytes += 1 + len;
          }
        }

        if(numberOfGlobalLabels != 0) {
          qsort(tmpString, numberOfGlobalLabels, 15, sortMenu);
        }

        ptr = malloc(numberOfBytes);
        dynamicSoftmenu[menu].menuContent = ptr;
        for(i=0; i<numberOfGlobalLabels; i++) {
          int16_t len = stringByteLength(tmpString + 15*i) + 1;
          xcopy(ptr, tmpString + 15*i, len);
          ptr += len;
        }

        dynamicSoftmenu[menu].numItems = numberOfGlobalLabels;
        break;
      }

      case MNU_DYNAMIC: {
        _dynmenuConstructUser(menu);
        break;
      }

      case ITM_MENU: {
        numberOfBytes = 0;
        numberOfGlobalLabels = 0;
        memset(tmpString, 0, TMP_STR_LENGTH);
        for(i=0; i<18; i++) {
          xcopy(tmpString + numberOfBytes, programmableMenu.itemName[i], stringByteLength(programmableMenu.itemName[i]) + 1);
          numberOfBytes += stringByteLength(programmableMenu.itemName[i]) + 1;
        }

        ptr = malloc(numberOfBytes);
        dynamicSoftmenu[menu].menuContent = ptr;
        xcopy(ptr, tmpString, numberOfBytes);

        dynamicSoftmenu[menu].numItems = 18;
        break;
      }

      default: {
        sprintf(errorMessage, "In function initVariableSoftmenu: unexpected variable softmenu %" PRId16 "!", (int16_t)(-dynamicSoftmenu[menu].menuItem));
        displayBugScreen(errorMessage);
      }
    }
  }


static void showKey2(const char *label0, const char *label1, int16_t x1, int16_t x2, int16_t y1, int16_t y2, videoMode_t videoMode, bool_t topLine, bool_t bottomLine, int8_t showCb, int16_t showValue, const char *showText);
char label0[30];
int16_t xx1;

int8_t maxfLines = 0;
int8_t maxgLines = 0;

bool_t maxfgLines(int16_t y) {
  if(((maxfLines & 1) == 1) && ((maxgLines & 1) == 1)) {
    return (2 == y) || (1 == y);  //if any bit set in the f and g-line, allow fgline on f and g
  }
  else if(((maxfLines & 1) == 1) && ((maxgLines & 1) == 0)) {
    return (1 == y);              //if any bit set in the f-line, allow fgline on f
  }
  else {
    return false;
  }
}


  /********************************************//**
   * \brief Displays one softkey: helpers
   ***********************************************/
  #define greyout true
  static bool_t initSoftkeyCoordinates(const char *label, int16_t xSoftkey, int16_t ySoftKey, int16_t *x1, int16_t *x2, int16_t *y1, int16_t *y2) {
    if(label[0] !=0 ) {
      if(ySoftKey==1) {
        maxfLines |= 1; //set bit 0 for any non-blank softkey in f
      }
      if(ySoftKey == 2) {
        maxgLines |= 1; //set bit 0 for any non-blank softkey in g
        maxfLines |= 1; //set bit 0 for any non-blank softkey in g (add f, for a g softkey otherwise g
      }
    }
    if(GRAPHMODE && xSoftkey >= 2) {           //prevent softkeys columns 3-6 from displaying over the graph
      return false;
    }
    if(0 <= xSoftkey && xSoftkey <= 5) {
      *x1 = KEY_X[xSoftkey];
      *x2 = KEY_X[xSoftkey+1];
    }
    else {
      sprintf(errorMessage, "In function initSoftkeyCoordinates: xSoftkey=%" PRId16 " must be from 0 to 5" , xSoftkey);
      displayBugScreen(errorMessage);
      return false;
    }
    if(0 <= ySoftKey && ySoftKey <= 2) {
      *y1 = 217 - SOFTMENU_HEIGHT * ySoftKey;
      *y2 = *y1 + SOFTMENU_HEIGHT;
    }
    else {
      sprintf(errorMessage, "In function initSoftkeyCoordinates: ySoftKey=%" PRId16 " but must be from 0 to 2!" , ySoftKey);
      displayBugScreen(errorMessage);
      return false;
    }
    return true;
  }

  static void truncateAtString(char *label, const char *search) {
    int16_t i = 0;
    while(label[i+1] != 0) {
      if(search[0] == label[i] && search[1] == label[i+1]) {
        label[i] = 0;
        break;
      }
      i++;
    }
  }

  static void truncateAtArrow(char *label) {
    char sample[4];

    stringCopy(sample, STD_RIGHT_ARROW);
    truncateAtString(label, sample);

    stringCopy(sample, STD_LEFT_ARROW);
    truncateAtString(label, sample);
  }

  void greyRect(int16_t x, int16_t y, int16_t dx, int16_t dy) {
    int16_t col, row;
    for (row=y; row<dy+y; row++) {
      for (col=x+mod(x+row,2); col<dx+x; col+=2) {
        setBlackPixel(col, row);
      }
    }
  }

  /********************************************//**
   * \brief Displays one softkey
   *
   * \param[in] l const char*         Text to display
   * \param[in] xSoftkey int16_t      x location of softkey: from 0 (left) to 5 (right)
   * \param[in] ySoftKey int16_t      y location of softkey: from 0 (bottom) to 2 (top)
   * \param[in] videoMode videoMode_t Video mode normal or reverse
   * \param[in] topLineDotted bool_t  Is the item's top line dotted
   * \param[in] topLine bool_t        Draw a top line
   * \param[in] bottomLine bool_t     Draw a bottom line
   * \return void
   ***********************************************/
  static void showSoftkey(const char *label, int16_t xSoftkey, int16_t ySoftKey, videoMode_t videoMode, bool_t topLine, bool_t bottomLine, int8_t showCb, int16_t showValue, const char *showText) {
    int16_t x1, y1, x2, y2;
    if(!initSoftkeyCoordinates(label, xSoftkey, ySoftKey, &x1, &x2, &y1, &y2)) {
      return;
    }
    showKey(label, x1, x2, y1, y2, videoMode, topLine, bottomLine, showCb, showValue, showText);
  }

  static void showSoftkey2(const char *labelSM1, int16_t xSoftkey, int16_t ySoftKey, videoMode_t videoMode, bool_t topLine, bool_t bottomLine, int8_t showCb, int16_t showValue, const char *showText) {
    int16_t x1, y1, x2, y2;
    if(!initSoftkeyCoordinates(labelSM1, xSoftkey, ySoftKey, &x1, &x2, &y1, &y2)) {
      return;
    }
  char label1[30];
  if(xSoftkey == 0 || xSoftkey == 2 || xSoftkey == 4) {
    xx1 = x1;
    label0[0]=0;
    stringCopy(label0 + stringByteLength(label0), labelSM1);
    compressConversionName(label0);
  }
  truncateAtArrow(label0);

  if(xSoftkey == 1 || xSoftkey == 3 || xSoftkey == 5) {
    label1[0]=0;
    stringCopy(label1 + stringByteLength(label1), labelSM1);
    compressConversionName(label1);
    truncateAtArrow(label1);
    showKey2(label0, label1, xx1, x2, y1, y2, videoMode, topLine, bottomLine, showCb, showValue, showText);
  }
}



static inline void drawKeyFrame(int16_t x1, int16_t x2, int16_t y1, int16_t y2, videoMode_t videoMode, bool_t topLine, bool_t bottomLine) {
  // Draw the frame
  int16_t grx1 = max(0, x1), gry1 = y1 + (!bottomLine);
  greyRect(grx1, gry1, min(x2+1, SCREEN_WIDTH) - grx1, min(y2 + topLine, SCREEN_HEIGHT) - gry1);
  lcd_fill_rect(x1 + 1, y1 + 1, min(x2, SCREEN_WIDTH) - x1 - 1, min(y2, SCREEN_HEIGHT) - y1 - 1, (videoMode == vmNormal ? LCD_SET_VALUE : LCD_EMPTY_VALUE));
}



static void showKey2(const char *label0, const char *label1, int16_t x1, int16_t x2, int16_t y1, int16_t y2, videoMode_t videoMode, bool_t topLine, bool_t bottomLine, int8_t showCb, int16_t showValue, const char *showText) {
  #define YY -100
  int16_t Text0   ;
  int16_t Arr0    ;
  int16_t midpoint;
  int16_t Arr1    ;
  int16_t Text1   ;
  float   space   ;
  float   space0=0;
  float   space1=0;

  // Compute widths and strings once based on HPCONV, without changing math
  const char *t[4];
  int16_t widths[4];
  int16_t arrowSpace;
  const char *w[4];

  if(getSystemFlag(FLAG_HPCONV)) {
    t[0] = label1;         w[0] = label1;
    t[1] = STD_LEFT_ARROW; w[1] = STD_LEFT_ARROW;
    t[2] = label0;         w[2] = STD_RIGHT_ARROW;
    t[3] = STD_RIGHT_ARROW;w[3] = label0;
    arrowSpace = 2;
  } else {
    t[0] = label0;         w[0] = label0;
    t[1] = STD_RIGHT_ARROW;w[1] = STD_RIGHT_ARROW;
    t[2] = label1;         w[2] = STD_LEFT_ARROW;
    t[3] = STD_LEFT_ARROW; w[3] = label1;
    arrowSpace = 10;
  }
  for(int i=0; i<4; i++) {
    widths[i] = showStringEnhanced(w[i], &standardFont, 0, y1+YY, videoMode, false, false, DO_compress, NO_raise, NO_Show, NO_Bold, NO_LF);
  }

  midpoint = (x2 - x1) / 2;
  space0   = ((x2 - x1)/2.0f - widths[0] - widths[1] - arrowSpace) / 2.0f;
  Text0    = x1 + midpoint - arrowSpace - widths[1] - space0 - widths[0];
  Arr0     = x1 + midpoint - arrowSpace - widths[1];
  space1   = ((x2 - x1)/2.0f - widths[2] - widths[3] - arrowSpace) / 2.0f;
  Arr1     = x1 + midpoint + arrowSpace;
  Text1    = x1 + midpoint + arrowSpace + widths[2] + space1;
  // s widths[0] s widths[1] arrowSpace | arrowSpace widths[2] s widths[3] s

  //printf("@@@@ %f %f\n",space0, space1);

  if(space0 < arrowSpace || space1 < arrowSpace) {
    space    = ((x2 - x1) - widths[0] - widths[1] - widths[2] - widths[3]) / 7.0f;
    Text0    = x1 + space;
    midpoint = 3.5 * space + widths[0] + widths[1];
    if(getSystemFlag(FLAG_HPCONV)) {
      Arr0     = x1 + midpoint - arrowSpace - widths[1];
      Arr1   = x1 + midpoint + arrowSpace;
    }
    else {
      Arr0     = x1 + space + widths[0] + space;
      Arr1     = x2 - space - widths[2] - widths[3] - space;
    }
    Text1    = x2 - space - widths[3];
    // s widths[0] s widths[1] s | s widths[2] s widths[3] s

  }
  //printf(">>>> |%s|%s| Space %f, w1 %d, w2 %d, w3 %d, w4 %d\n", label0, label1, space, w1, w2, w3, w4);

  // Clear inside the frame
  drawKeyFrame(x1, x2, y1, y2, videoMode, topLine, bottomLine);

  // Display strings once using the precomputed positions and sequence
  int16_t x[4] = {Text0, Arr0, Text1, Arr1};
  for(int i=0; i<4; i++) {
    showStringEnhanced(t[i], &standardFont, x[i], y1 + 1, videoMode, false, false, DO_compress, NO_raise, DO_Show, NO_Bold, NO_LF);
  }
  // Mid vertical line, unchanged
    lcd_fill_rect(x1 + midpoint, y1 + 5, 1, min(y2, SCREEN_HEIGHT - 1) + 1 - y1 - 2*5, (videoMode == vmNormal ? LCD_EMPTY_VALUE : LCD_SET_VALUE));
}


void showKey(const char *label, int16_t x1, int16_t x2, int16_t y1, int16_t y2, videoMode_t videoMode, bool_t topLine, bool_t bottomLine, int8_t showCb, int16_t showValue, const char *showText) {
    int16_t w;
    char l[16];

    drawKeyFrame(x1, x2, y1, y2, videoMode, topLine, bottomLine);

    xcopy(l, label, stringByteLength(label) + 1);
    //    char *lw = stringAfterPixels(l, &standardFont, (rightMostSlot ? 65 : 66), false, false);
    //    *lw = 0;
    //continue with trimmed label
    w = stringWidthC47(figlabel(l, showText, showValue), stdNoEnlarge, 0, false, false);
    if((showCb >= 0) || (w >= ((min(x2, SCREEN_WIDTH) - max(0, x1))*3)/4 )) {
      w = stringWidthC47(figlabel(l, showText, showValue), stdNoEnlarge, 1, false, false);
      if(showCb >= 0) { w = w + 8; }
      //    char *lw = stringAfterPixelsC47(l, stdNoEnlarge, compressString, rightMostSlot ? 65 : 66, false, false);
      //    *lw = 0;
    compressString = 1;       //JM compressString
    showString(figlabel(l, showText, showValue), &standardFont, (x1 + x2 - w)/2, y1 + 2, videoMode, false, false);
    compressString = 0;       //JM compressString
  }
  else {
     //clearly short enough so no trimming was needed anyway
     showString(figlabel(l, showText, showValue), &standardFont, (x1 + x2 - w)/2, y1 + 2, videoMode, false, false);
  }                                                                                              //JM & dr ^^

#if defined(JM_LINE2_DRAW)
  if(showCb >= 0) {
    if(videoMode == vmNormal) {
      JM_LINE2(x2, y2);
    }
  }
#endif // JM_LINE2_DRAW

  //EXTRA DRAWINGS FOR RADIO_BUTTON AND CHECK_BOX
  if(showCb >= 0) {
    if(videoMode == vmNormal) {
      if(showCb == RB_FALSE) {
        RB_UNCHECKED(x2-11, y2-16);
      }
      else if(showCb == RB_TRUE) {
        RB_CHECKED(x2-11, y2-16);
      }
      else if(showCb == CB_TRUE) {
        CB_CHECKED(x2-11, y2-16);
      }
      else if(showCb == CB_FALSE) {
        CB_UNCHECKED(x2-11, y2-16);
      }
      else if(showCb == MB_FALSE) {
        MB_MACRO(x2-11, y2-16);
      }
      else if(showCb == MB_TRUE) {
        MB_MACRO_CHECKED(x2-11, y2-16);
      }
    }
  }


//Show a 'panelled' view of softkeys if a menu is assignable
//printf("currentMenu()=%d\n",currentMenu());
  #define _off 1 // function parameter: +1 is favoured
  if(calcMode == CM_ASSIGN && itemToBeAssigned != 0 &&
     (currentMenu() == -MNU_HOME ||
      currentMenu() == -MNU_MyMenu ||
      currentMenu() == -MNU_MyAlpha ||
      currentMenu() == -MNU_PFN ||
      currentMenu() == -MNU_DYNAMIC)) {

    int16_t xs[4], ys[4], ws[4], hs[4];
    if(_off == 2) { //inner doubling of softkey box
      xs[0] = max(0, x1)+1;         ys[0] = y1;                         ws[0] = 1; hs[0] = SOFTMENU_HEIGHT;
      xs[1] = x2-1;                 ys[1] = y1;                         ws[1] = 1; hs[1] = SOFTMENU_HEIGHT;
      xs[2] = max(0, x1)+1;         ys[2] = y1+1;                       ws[2] = min(x2, SCREEN_WIDTH)-max(0, x1)-2; hs[2] = 1;
      xs[3] = max(0, x1)+1;         ys[3] = y1+SOFTMENU_HEIGHT-1;       ws[3] = min(x2, SCREEN_WIDTH)-max(0, x1)-2; hs[3] = 1;
    }
    else { //positioning of nails or rivets
      xs[0] = max(0, x1)+2+_off;    ys[0] = y1+1+_off;                  ws[0] = 3; hs[0] = 2;
      xs[1] = max(0, x1)+2+_off;    ys[1] = y1+SOFTMENU_HEIGHT-2-_off;  ws[1] = 3; hs[1] = 2;
      xs[2] = x2-1-3-_off;          ys[2] = y1+1+_off;                  ws[2] = 3; hs[2] = 2;
      xs[3] = x2-1-3-_off;          ys[3] = y1+SOFTMENU_HEIGHT-2-_off;  ws[3] = 3; hs[3] = 2;
    }
    for(int i=0; i<4; i++) {
      lcd_fill_rect(xs[i], ys[i], ws[i], hs[i], (videoMode == vmNormal ? LCD_EMPTY_VALUE : LCD_SET_VALUE));
    }
  }
}


bool_t isFunctionItemAMenu(int16_t item) { //masquarading
  return item == ITM_PLOT_SCATR||
         item == ITM_PLOT_ASSESS||
         item == ITM_HPLOT     ||
         item == ITM_DRAW      ||
         item == ITM_DRAW_LU   ||
         item == ITM_CFG       ||
         item == ITM_GAP_L     ||
         item == ITM_GAP_RX    ||
         item == ITM_GAP_R     ||
         item == ITM_PLOT_STAT ||
         item == ITM_EQ_NEW    ||
         item == ITM_SIM_EQ    ||
         item == ITM_DELITM    ||
         item == ITM_M_EDI     ||
         item == ITM_CLKp2     ||
         item == ITM_BITSp2;
         /*item == ITM_PLOT_CENTRL ||  CENTRL does not bring up a new menu - it is the same menu therefore not inverted */
         /*|| (item == ITM_TIMER)*/       //JMvv colour PLOT in reverse font to appear to be menus
}



char FF[16]; //using static char FF
static char *changeDotAndIJ(int16_t item, const char* itemN) {
  if((item == ITM_DREAL || item == SFL_DREAL) && itemN[3] == '.') { //this replaces the 4th byte of dℤ.0 with the ascii ./,
    stringCopy(FF, itemN);
    FF[3] = RADIX34_MARK_CHAR;
    return FF;
  }
  else if(getSystemFlag(FLAG_CPXj)) {
    stringCopy(FF, itemN);
    if((item == ITM_op_j || item == ITM_op_j_pol || item == ITM_op_j_SIGN) && FF[1] == STD_op_i[1]) {
      //printf(">>>> changed: %u %u %u %u %u %s %u %u\n", (uint8_t)(FF[0]), (uint8_t)(FF[1]), (uint8_t)(FF[2]), (uint8_t)(FF[3]), (uint8_t)(FF[4]), FF , (uint8_t)(STD_SUP_i[0]), (uint8_t)(STD_SUP_i[1]));
      FF[1]++;
    }
    if(item == ITM_EE_EXP_TH && FF[3] == STD_SUP_i[1]) {
      FF[3]++;
    }
    return FF;
  }
  return (char*)itemN;
}


typedef struct {
  char     modeName[5];
} mstr;



TO_QSPI static const mstr modeNames[] = {
/*0*/  { "ALL" },
/*1*/  { "FIX" },
/*2*/  { "SCI" },
/*3*/  { "ENG" },
/*4*/  { "SIG" },
/*5*/  { "UNIT"},
};


void changeSoftKey(int16_t menuNr, int16_t itemNr, char * itemName, videoMode_t * vm, int8_t * showCb, int16_t * showValue, char * showText) {
  float tmpF = 0;
  char tmpS[30], tmpSS[20];
  real_t tmpR;
  * vm = (itemNr < 0) || (isFunctionItemAMenu(itemNr%10000)) ? vmReverse : vmNormal;
  * showCb = NOVAL;
  * showValue = NOVAL;
  showText[0] = 0;
  stringCopy(itemName, NOTEXT);
  showText[0]=0;

  if(itemNr > 0) {
    * showCb = fnCbIsSet(itemNr%10000);
    * showValue = fnItemShowValue(itemNr%10000);

    switch(itemNr%10000) {

      case VAR_ACC: {
                      real34ToReal(REGISTER_REAL34_DATA(RESERVED_VARIABLE_ACC), &tmpR);
                      if(realIsZero(&tmpR)) {
                        strcpy(tmpS,"0");
                      }
                      else {
                        realToFloat(&tmpR, &tmpF);
                        if(tmpF<0) {
                          strcpy(tmpS,"NEG");
                        }
                        else if(tmpF<1.0e-34) {
                          strcpy(tmpS,STD_GAUSS_WHITE_L "1E-34");
                        }
                        else if(tmpF>1) {
                          strcpy(tmpS,STD_GAUSS_WHITE_R "1");
                        }
                        else {
                          sprintf(tmpS,"%5.G",tmpF);
                          strcpy(tmpS, eatSpacesMid(tmpS));
                        }
                      }
                      stringCopy(showText + stringByteLength(showText), tmpS);
                      break;
                    }

      case ITM_PZOOMY  :
      case ITM_MZOOMY  :
                      if(PLOT_ZMY == zoomOverride) {
                        strcpy(tmpS, STD_SPACE_6_PER_EM STD_SUB_X);
                        *showValue = NOVAL;
                        stringCopy(showText + stringByteLength(showText), tmpS);
                      }
                      break;

      case ITM_IPLUS   :
      case ITM_IMINUS  :
                      if(isMatrixIndexed() && getRegisterAsRealQuiet(REGISTER_I, &tmpR)) {
                        sprintf(tmpS, STD_SPACE_3_PER_EM STD_SPACE_3_PER_EM "%u",(uint16_t)realToUint32C47(&tmpR));
                        stringCopy(showText + stringByteLength(showText), tmpS);
                        *showValue = NOVAL;
                      }
                      break;

      case ITM_JPLUS   :
      case ITM_JMINUS  :
                      if(isMatrixIndexed() && getRegisterAsRealQuiet(REGISTER_J, &tmpR)) {
                        sprintf(tmpS, STD_SPACE_3_PER_EM STD_SPACE_3_PER_EM "%u",(uint16_t)realToUint32C47(&tmpR));
                        stringCopy(showText + stringByteLength(showText), tmpS);
                        *showValue = NOVAL;
                      }
                      break;

      case VAR_ULIM    :
      case VAR_LLIM    :
      case VAR_UX      :
      case VAR_LX      :
      case VAR_UEST    :
      case VAR_LEST    :
      case VAR_UY      :
      case VAR_LY      :

      case ITM_STORCL_FV     :
      case ITM_STORCL_IPonA  :
      case ITM_STORCL_NPPER  :
      case ITM_STORCL_PPERonA:
      case ITM_STORCL_CPERonA:
      case ITM_STORCL_PMT    :
      case ITM_STORCL_PV     :

      case VAR_IPonA   :
      case VAR_NPPER   :
      case VAR_PPERonA :
      case VAR_CPERonA :

      case VAR_PV      : //comment these out to have no subscripted numbers on FV, PV & PMT
      case VAR_FV      : //comment these out to have no subscripted numbers on FV, PV & PMT
      case VAR_PMT     : //comment these out to have no subscripted numbers on FV, PV & PMT

                    { stringCopy(itemName, indexOfItems[itemNr%10000].itemSoftmenuName);
                      real34ToReal(REGISTER_REAL34_DATA(indexOfItems[itemNr%10000].param), &tmpR);
                      realToFloat(&tmpR, &tmpF);

                      if(false && tmpF == (int)tmpF && (
                         (tmpF >= 0 && tmpF < ((itemNr%10000 == VAR_NPPER || itemNr%10000 == VAR_PMT) ? 100000 : 1000000)) ||
                         (tmpF < 0 && -tmpF < ((itemNr%10000 == VAR_NPPER || itemNr%10000 == VAR_PMT) ? 10000 : 100000))
                         )) {
                        //integer smaller than limit
                        sprintf(tmpS,"%i",(int)tmpF);
                      }
                      else {
                        //out of range for display
                        if(tmpF>0 && tmpF<1.0e-34) {
                          strcpy(tmpS,STD_GAUSS_WHITE_R STD_SUB_0);
                        }
                        else if(tmpF<0 && tmpF>-1.0e-34) {
                          strcpy(tmpS,STD_GAUSS_WHITE_L STD_SUB_0);
                        }
                        else if(tmpF>1.0e34) {
                          strcpy(tmpS,STD_GAUSS_WHITE_R STD_GAUSS_WHITE_R );
                        }
                        else if(tmpF<-1.0e34) {
                          strcpy(tmpS,STD_GAUSS_WHITE_L STD_GAUSS_WHITE_L );
                        }
                        else {
                          bool_t convertedRealPerfectly;
                          char tmpBuf[100];
                          strcpy(tmpS, formatDoubleWidth((REGISTER_REAL34_DATA(indexOfItems[itemNr%10000].param)), 4, itemName, &convertedRealPerfectly, 400 / 6 - 2 - 4, tmpBuf, 60));
                          //printReal34ToConsole(REGISTER_REAL34_DATA(indexOfItems[itemNr%10000].param), "formatDoubleWidth1(", ", 4, \"QQ\", convertedRealPerfectly");
                          //printf(") => %s and convertedRealPerfectly = %d\n", tmpS, convertedRealPerfectly);

                          if(tmpS[0] == '?' ||  strchr(tmpS, 'E') != NULL) {    // ?? if no cenversion too place, cut string length and try again; If E on the first try, try again with wider
                            switch(itemNr%10000) {
                              case VAR_ULIM    :
                              case VAR_LLIM    :
                              case VAR_UEST    :
                              case VAR_LEST    :
                                   stringCopy(itemName + 3, STD_SPACE_4_PER_EM);
                                   break;
                              case VAR_IPonA   :
                              case VAR_PPERonA :
                              case VAR_CPERonA :
                                   stringCopy(itemName + 1, STD_SUB_a STD_SPACE_4_PER_EM);
                                   break;
                              case VAR_PV      :
                                   stringCopy(itemName + 1, STD_SUB_v);
                                   break;
                              case VAR_FV      :
                                   itemName[1] = 0;
                                   break;
                              //case VAR_NPPER   :
                              case VAR_PMT     :
                                   stringCopy(itemName + 1, STD_SUB_m);// STD_SPACE_4_PER_EM);
                                   break;
                              default:;
                            }
                            strcpy(tmpS, formatDoubleWidth((REGISTER_REAL34_DATA(indexOfItems[itemNr%10000].param)), 4, itemName, &convertedRealPerfectly, 400 / 6 - 2 - 4, tmpBuf, 60));

                            //printReal34ToConsole(REGISTER_REAL34_DATA(indexOfItems[itemNr%10000].param), "formatDoubleWidth2(", ", 4, \"Q\", convertedRealPerfectly");
                            //printf(") => %s and success = %d\n", tmpS, convertedRealPerfectly);
                          }
                        }
                      }

                      radixProcess(tmpSS,tmpS);

                      //for very short numerics, add one space
                      if(stringByteLength(tmpSS) < 4) {
                        sprintf(tmpS, STD_SPACE_3_PER_EM "%s",tmpSS);
                      }
                      else {
                        sprintf(tmpS, "%s",tmpSS);
                      }
                      stringCopy(showText + stringByteLength(showText), tmpS);
                      return;
                      break;
                    }

      case ITM_DSP:
      case ITM_UNIT: if(getSystemFlag(FLAG_2TO10) && displayFormat == DF_UN) {
                       stringCopy(showText + stringByteLength(showText), STD_SUB_i);
                      }
                      break;

      case ITM_DSPCYCLE:switch(*showValue) {
                          case 32700 :
                          case 32701 :
                          case 32702 :
                          case 32703 :
                          case 32704 :
                          case 32705 :stringCopy(showText + stringByteLength(showText), modeNames[*showValue - 32700].modeName ); *showValue = NOVAL; break;
                          default: ;
                          }
                          break;
      case ITM_SCR    :switch(*showValue) {
                          case NC_NORMAL      : *showValue = NOVAL; break;
                          case NC_SUBSCRIPT   : stringCopy(showText + stringByteLength(showText), alphaCase == AC_LOWER ? STD_SUB_s STD_SUB_u STD_SUB_b : alphaCase == AC_UPPER ? STD_SUB_S STD_SUB_U STD_SUB_B : ""); *showValue = NOVAL;
                                                stringCopy(itemName,indexOfItems[itemNr%10000].itemSoftmenuName);
                                                itemName[0]=STD_alpha[0];
                                                itemName[1]=STD_alpha[1];
                                                itemName[2]=0;
                                                return;
                                                break;
                          case NC_SUPERSCRIPT : stringCopy(showText + stringByteLength(showText), alphaCase == AC_LOWER ? STD_SUP_s STD_SUP_u STD_SUP_p : alphaCase == AC_UPPER ? STD_SUP_S STD_SUP_U STD_SUP_P : ""); *showValue = NOVAL;
                                                stringCopy(itemName,indexOfItems[itemNr%10000].itemSoftmenuName);
                                                itemName[0]=STD_alpha[0];
                                                itemName[1]=STD_alpha[1];
                                                itemName[2]=0;
                                                return;
                                                break;
                          default: ;
                        }
                        break;
      case ITM_DENMAX2: *showValue = denMax;
                        if(denMax == 0) {
                          strcpy(showText, STD_SUB_m STD_SUB_a STD_SUB_x );
                          showText[6] = 0;
                          *showValue = NOVAL;
                        }
                        break;

      case ITM_YY_DFLT: *showValue = lastCenturyHighUsed & 0x3FFF;
                        showText[0] = 0;
                        if(lastCenturyHighUsed & 0x8000) {
                          *showValue = NOVAL;
                          strcpy(showText,STD_SUB_o STD_SUB_f STD_SUB_f);
                        }
                        if(followYY()) {
                          strcat(showText,STD_SPACE_3_PER_EM STD_SUB_t);
                        }
                        break;

      case ITM_GAP_L  : if(gapItemLeft == ITM_NULL) stringCopy(showText + stringByteLength(showText), "\1\1");
                        else stringCopy(showText + stringByteLength(showText), indexOfItems[gapItemLeft].itemSoftmenuName);  //  gapCharLeft);
                        *showValue = NOVAL;
                        break;
      case ITM_GAP_RX : stringCopy(showText + stringByteLength(showText), indexOfItems[gapItemRadix].itemSoftmenuName);  //  gapCharRadix);
                        *showValue = NOVAL;
                        break;
      case ITM_GAP_R  : if(gapItemRight == ITM_NULL) stringCopy(showText + stringByteLength(showText), "\1\1");
                        else stringCopy(showText + stringByteLength(showText), indexOfItems[gapItemRight].itemSoftmenuName);  //  gapCharRight);
                        *showValue = NOVAL;
                        break;
      case ITM_GRP_L  : *showValue = grpGroupingLeft;
                        break;
      case ITM_GRP1_L : *showValue = grpGroupingGr1Left;
                        break;
      case ITM_GRP1_L_OF:*showValue = grpGroupingGr1LeftOverflow;
                        break;
      case ITM_GRP_R  : *showValue = grpGroupingRight;
                        break;
      default: ;
      }


    if(itemNr%10000 == 9999) {
      stringCopy(itemName, indexOfItems[!getSystemFlag(FLAG_MULTx) ? ITM_DOT : ITM_CROSS].itemSoftmenuName);
      //printf("WWW1: itemName=%s, 0:%i 1:%i, ItemNr=%i \n",itemName, (uint8_t) itemName[0], (uint8_t) itemName[1], itemNr);
      return;
    }
    else if((indexOfItems[itemNr%10000].status & CAT_STATUS) == CAT_CNST) {
      stringCopy(itemName, indexOfItems[itemNr%10000].itemCatalogName);
    }
    else {
      stringCopy(itemName, changeDotAndIJ(itemNr, indexOfItems[itemNr%10000].itemSoftmenuName));
      //printf("WWW2: itemName=%s, ItemNr=%i \n",itemName,itemNr);
      return;
    }
  }
  else if(itemNr < 0) { //itemNr >= 0
    stringCopy(itemName, indexOfItems[-itemNr%10000].itemSoftmenuName);
    //printf("WWW3: itemName=%s, ItemNr=%i \n",itemName,itemNr);
    return;
  }
}



bool_t savedspace(int16_t itemNr) {  //strike out all SAVED_SPACE items
  switch (itemNr) {

    #ifdef SAVE_SPACE_DM42_12ORTHO
      case -MNU_ORTHOG:
      case ITM_HN     :
      case ITM_Lm     :
      case ITM_LmALPHA:
      case ITM_Pn     :
      case ITM_Tn     :
      case ITM_Un     :
      case ITM_HNP    :
    #endif //SAVE_SPACE_DM42_12ORTHO

    #ifdef SAVE_SPACE_DM42_20_TIMER
      case ITM_TIMER  :
    #endif //SAVE_SPACE_DM42_20_TIMER

    #ifdef SAVE_SPACE_DM42_12BESSEL
      case ITM_JYX    :
      case ITM_YYX    :
    #endif //SAVE_SPACE_DM42_12BESSEL

    #ifdef SAVE_SPACE_DM42_12PRIME
      case ITM_NEXTP  :
      case ITM_PRIME  :
      case ITM_FACTORS:
      case ITM_PFACTORSMULT:
      case ITM_EULPHI :
      case ITM_SIGMA0 :
      case ITM_SIGMA1 :
      case ITM_SIGMAk :
      case ITM_SIGMAp1:
      case ITM_SIGMApk:
    #endif //SAVE_SPACE_DM42_12PRIME

    #ifdef SAVE_SPACE_DM42_16
      case -MNU_NORML :
      case ITM_NORMLP :      case ITM_NORML  :      case ITM_NORMLU :      case ITM_NORMLM1:
      case ITM_LGNRMP :      case ITM_LGNRM  :      case ITM_LGNRMU :      case ITM_LGNRMM1:
    #endif //SAVE_SPACE_DM42_16

    #ifdef SAVE_SPACE_DM42_17
      case -MNU_F: case -MNU_BINOM: case -MNU_HYPER: case -MNU_POISS: case -MNU_GEOM:
      case ITM_FPX   :     case ITM_FX   :      case ITM_FUX   :     case ITM_FM1P:
      case ITM_BINOMP:     case ITM_BINOM:      case ITM_BINOMU:     case ITM_BINOMM1:
      case ITM_NBINP :     case ITM_NBIN :      case ITM_NBINU :     case ITM_NBINM1 :
      case ITM_HYPERP:     case ITM_HYPER:      case ITM_HYPERU:     case ITM_HYPERM1:
      case ITM_POISSP:     case ITM_POISS:      case ITM_POISSU:     case ITM_POISSM1:
      case ITM_GEOMP :     case ITM_GEOM :      case ITM_GEOMU :     case ITM_GEOMM1 :
    #endif //SAVE_SPACE_DM42_17

      case 9999       : return true; break;
    default           : return false; break;
  }
}


#define typeStrikeOut 1
#define typeStrikeThrough 2
static void strokeStrike(uint8_t type_, bool_t condition, int16_t *xStroke, int16_t *yStroke, int16_t x, int16_t y) {
  for(*xStroke = x*67 + 1 +9; *xStroke < x*67 + 66 -10; (*xStroke)++) {
    if(type_ == typeStrikeOut) { //cause diagonal
      if(*xStroke%3 == 0) {
        (*yStroke)--;
      }
    }
    if(condition) {
      setBlackPixel(*xStroke, *yStroke -3);
    }
    else {
      setWhitePixel(*xStroke, *yStroke -3);
    }
  }
}


void fnStrikeOutIfNotCoded(int16_t itemNr, int16_t x, int16_t y) {
  if(itemNr == -MNU_HOME || itemNr == -MNU_PFN) {
    return;
  }
  int16_t strike = 0;
  if(itemNr > 0) {
    if(indexOfItems[itemNr%10000].func == itemToBeCoded || savedspace(itemNr)) {
      strike = 1;
    }
  }
  else if(itemNr < 0) {
    int16_t m = 0;
    while(softmenu[m].menuItem != 0) {
      if(softmenu[m].menuItem == itemNr%10000) {
       break;
      }
      m++;
    }
    if((softmenu[m].numItems == 0 || savedspace(itemNr)) && m >= NUMBER_OF_DYNAMIC_SOFTMENUS) {
      strike = -1;
    }
  }

  if(strike != 0) {
    // Strike out non coded functions
    int16_t xStroke, yStroke = SCREEN_HEIGHT - y*23 - 1;
    strokeStrike(typeStrikeOut, strike == 1, &xStroke, &yStroke, x, y);
  }
}


void fnStrikeThroughIfNA(int16_t itemNr, int16_t x, int16_t y) {
  int16_t xStroke, yStroke = SCREEN_HEIGHT - y*23 - 9;
  if(itemNotAvail(itemNr)) {
    strokeStrike(typeStrikeThrough, itemNr > 0, &xStroke, &yStroke, x, y);
  }
}


typedef enum {
  openMenu  = 0,
  closeMenu = 1
} menuOps_t;


static void setScreenUpdateFromMenu(int16_t id, menuOps_t op) {
  switch(id) {
    case -MNU_XXFCNS :
    case -MNU_EQN :
    case -MNU_DISTR :
    case -MNU_EQ_EDIT :
    case -MNU_Solver_TOOL : {
      if(op == closeMenu) {
        solverEstimatesUsed = false;
      }
      screenUpdatingMode = SCRUPD_AUTO;
      screenUpdatingMode |= SCRUPD_SKIP_STATUSBAR_ONE_TIME;
      break;
    }
    case -MNU_MVAR : {
      screenUpdatingMode = SCRUPD_AUTO;
      screenUpdatingMode |= SCRUPD_SKIP_STATUSBAR_ONE_TIME;
      break;
    }
    default:;
  }
//printf("setScreenUpdateFromMenu: solverEstimatesUsed = %d: id=%d %s\n",solverEstimatesUsed, id, indexOfItems[-id].itemCatalogName);
}


bool_t BASE_OVERRIDEONCE = false;

void showSoftmenuCurrentPart(void) {
  if(currentMenu() == -MNU_HOME) {
    changeToHOME();
  }
  else
  if(currentMenu() == -MNU_PFN) {
    changeToPFN();
  }

//JMTOCHECK: Removed exceptions for underline removal.

  maxfLines = 0;
  maxgLines = 0;
  char tmp1[16];
  int16_t x, y, yDotted=0, currentFirstItem, item, numberOfItems, m = softmenuStack[0].softmenuId;
  bool_t dottedTopLine;
  #if defined(PC_BUILD) && ((VERBOSE_LEVEL > -1) || defined(PC_BUILD_TELLTALE))
    char tmp[200]; sprintf(tmp,"^^^^showSoftmenuCurrentPart: Showing Softmenu id=%d item=%i %s\n",m, currentMenu(), indexOfItems[currentMenu() > 0 ? currentMenu() : -currentMenu()].itemSoftmenuName); jm_show_comment(tmp);
    printf("==>%s\n",tmp);
  #endif // PC_BUILD

  screenUpdatingMode &= ~(SCRUPD_MANUAL_MENU | SCRUPD_SKIP_MENU_ONE_TIME);
  setScreenUpdateFromMenu(softmenu[m].menuItem, openMenu);

  if((!IS_BASEBLANK_(m) || BASE_OVERRIDEONCE) && calcMode != CM_FLAG_BROWSER && calcMode != CM_ASN_BROWSER && calcMode != CM_FONT_BROWSER && calcMode != CM_REGISTER_BROWSER && calcMode != CM_BUG_ON_SCREEN) {           //JM: Added exclusions, as this procedure is not only called from refreshScreen, but from various places due to underline
    clearScreenOld(false, false, true); //JM, added to ensure the f/g underlines are deleted
    // clear_ul();
    BASE_OVERRIDEONCE = false;
    if(tam.mode == TM_KEY && !tam.keyInputFinished) {
      for(y=0; y<=2; y++) {
        for(x=0; x<6; x++) {
          stringCopy(tmp1, " ");
          if(1+x+y*6 > 9) {
            tmp1[0] = '1';
            stringCopy(tmp1 + 1, " ");
            tmp1[1] = (int)(48+(1+x+y*6) % 10);
          }
          else {
            tmp1[0] = (int)((48+1+x+y*6));
          }
          showSoftkey(tmp1, x, y, vmReverse, true, true, NOVAL, NOVAL, NOTEXT);
        }
      }
      return;
    }

    if(m < NUMBER_OF_DYNAMIC_SOFTMENUS) { // Dynamic softmenu
      #if defined(PC_BUILD)
        //printf("Dynamic menu: m=%i cachedDynamicMenu=%i softmenu[m].menuItem= %i \n",m, cachedDynamicMenu, softmenu[m].menuItem);
      #endif // PC_BUILD
      if(softmenu[m].menuItem != cachedDynamicMenu || softmenu[m].menuItem == -MNU_DYNAMIC) {
        initVariableSoftmenu(m);
        cachedDynamicMenu = softmenu[m].menuItem;
      }
      numberOfItems = dynamicSoftmenu[m].numItems;
    }
    else if(softmenu[m].menuItem == -MNU_EQN && numberOfFormulae == 0) {
      numberOfItems = 1;
    }
    else { // Static softmenu
      numberOfItems = softmenu[m].numItems;
    }
    currentFirstItem = softmenuStack[0].firstItem;

/*
    //JMvv Temporary method to ensure AIM is active if the 3 ALPHA menus are shown //JM TOCHECK
    if((softmenuStackPointer > 0) && (calcMode != CM_AIM && (softmenu[m].menuId == -MNU_ALPHA || softmenu[m].menuId == -MNU_T_EDIT || softmenu[m].menuId == -MNU_MyAlpha))) {
      calcMode = CM_AIM;
      cursorFont = &standardFont;
      cursorEnabled = true;
      setSystemFlag(FLAG_ALPHA);
      refreshRegisterLine(AIM_REGISTER_LINE);
    } //JM ^^
*/

    if(numberOfItems <= 18) {
      dottedTopLine = false;
      if(catalog != CATALOG_NONE) {
        currentFirstItem = softmenuStack[0].firstItem = 0;
        setCatalogLastPos();
      }
    }
    else {
      dottedTopLine = true;
      yDotted = min(3, (numberOfItems + modulo(currentFirstItem - numberOfItems, 6))/6 - currentFirstItem/6) - 1;

      if(m >= NUMBER_OF_DYNAMIC_SOFTMENUS) { // Static softmenu
        item = 6 * (currentFirstItem / 6 + yDotted);
        if(                softmenu[m].softkeyItem[item]==0 && softmenu[m].softkeyItem[item+1]==0 && softmenu[m].softkeyItem[item+2]==0 && softmenu[m].softkeyItem[item+3]==0 && softmenu[m].softkeyItem[item+4]==0 && softmenu[m].softkeyItem[item+5]==0) {
          yDotted--;
        }

        item = 6 * (currentFirstItem / 6 + yDotted);
        if(yDotted >= 0 && softmenu[m].softkeyItem[item]==0 && softmenu[m].softkeyItem[item+1]==0 && softmenu[m].softkeyItem[item+2]==0 && softmenu[m].softkeyItem[item+3]==0 && softmenu[m].softkeyItem[item+4]==0 && softmenu[m].softkeyItem[item+5]==0) {
          yDotted--;
        }

        item = 6 * (currentFirstItem / 6 + yDotted);
        if(yDotted >= 0 && softmenu[m].softkeyItem[item]==0 && softmenu[m].softkeyItem[item+1]==0 && softmenu[m].softkeyItem[item+2]==0 && softmenu[m].softkeyItem[item+3]==0 && softmenu[m].softkeyItem[item+4]==0 && softmenu[m].softkeyItem[item+5]==0) {
          yDotted--;
        }
      }
    }

    char itemName[16]; itemName[0]=0;
    char showText[16]; showText[0]=0;
    videoMode_t vm = vmNormal;
    int8_t showCb = NOVAL;
    int16_t showValue = NOVAL;
    showText[0] = 0;

    if(m < NUMBER_OF_DYNAMIC_SOFTMENUS) { // Dynamic softmenu
      #if defined(PC_BUILD)
        //printf("Dynamic menu: m=%i cachedDynamicMenu=%i softmenu[m].menuItem= %i \n",m, cachedDynamicMenu, softmenu[m].menuItem);
      #endif // PC_BUILD
      if(numberOfItems == 0) {
        for(x=0; x<6; x++) {
          showSoftkey("", x, 0, vmNormal, true, true, NOVAL, NOVAL, NOTEXT);
        }
      }
      else {
        #if defined(PC_BUILD)
          //printf("Dynamic menu: populate\n");
        #endif // PC_BUILD
        uint8_t *ptr = getNthString(dynamicSoftmenu[m].menuContent, currentFirstItem);
        for(y=0; y<3; y++) {
          for(x=0; x<6; x++) {
            if(x + 6*y + currentFirstItem < numberOfItems) {
              if(*ptr != 0) {
                vm = vmNormal;
                showText[0] = 0;
                showCb = NOVAL;
                showValue = NOVAL;
                int16_t itemNr = userMenuItems[x + 6*y].item;
                stringCopy(itemName, (char *)ptr);
                //printf(">>>> %u %u %s %s \n", x, y, itemName, userMenuItems[x + 6*y].argumentName);
                switch(-softmenu[m].menuItem) {
                  case MNU_MENU:
                  case MNU_MENUS: {
                    vm = vmReverse;
                    break;
                  }
                  case MNU_MyMenu: {
                    //printf(">>>> MyMenu: %i %s : %s\n",itemNr, itemName, userMenuItems[x + 6*y].argumentName);
                    if(itemNr < 0) {
                     vm = vmReverse;       //No item name changes available for menu names
                    }
                    else {
                      if(userMenuItems[x + 6*y].argumentName[0] == 0) {
                        changeSoftKey(softmenu[m].menuItem, itemNr, itemName, &vm, &showCb, &showValue, showText);
                      }
                    }
                    break;
                  }
                  case MNU_MyAlpha: {
                    vm = (userAlphaItems[x + 6*y].item < 0) ? vmReverse : vmNormal;
                    itemNr = 0;             // set to 0 to clear the previous itemNr to prevent unrelated strikethroughs
                    break;
                  }
                  case MNU_DYNAMIC: {
                    itemNr = userMenus[currentUserMenu].menuItem[x + 6*y].item;
                    if(itemNr < 0) {
                     vm = vmReverse;       //No item name changes available for menu names
                    }
                    else {
                      if(userMenus[currentUserMenu].menuItem[x + 6*y].argumentName[0] == 0) {
                        changeSoftKey(softmenu[m].menuItem, itemNr, itemName, &vm, &showCb, &showValue, showText);
                      }
                    }
                    break;
                  }
                  case MNU_1STDERIV:
                  case MNU_2NDDERIV:
                  case MNU_MVAR: {
                    if(!compareString((char *)getNthString(dynamicSoftmenu[m].menuContent, x+6*y), indexOfItems[ITM_DRAW].itemSoftmenuName, CMP_NAME)) {
                       vm = vmReverse;
                    } else
                    if(!compareString((char *)getNthString(dynamicSoftmenu[m].menuContent, x+6*y), indexOfItems[ITM_DRAW_LU].itemSoftmenuName, CMP_NAME)) {
                       vm = vmReverse;
                    } else
                    if(!compareString((char *)getNthString(dynamicSoftmenu[m].menuContent, x+6*y), indexOfItems[MNU_GRAPHS].itemSoftmenuName, CMP_NAME)) {
                       vm = vmReverse;
                    }
                    else if(!compareString((char *)getNthString(dynamicSoftmenu[m].menuContent, x+6*y), indexOfItems[MNU_Solver_TOOL].itemSoftmenuName, CMP_NAME)) {
                       vm = vmReverse;
                    }
                    else if(!compareString((char *)getNthString(dynamicSoftmenu[m].menuContent, x+6*y), indexOfItems[MNU_Sf_TOOL].itemSoftmenuName, CMP_NAME)) {
                       vm = vmReverse;
                    }

//CHECKNOW check for the execution keybaord??
//CHECKNOW must be added: MNU_Solver_TOOL and MNU_Sf_TOOL

                    if(!compareString((char *)getNthString(dynamicSoftmenu[m].menuContent, x+6*y), indexOfItems[ITM_SETSIG2].itemSoftmenuName, CMP_NAME)) {
                       strcpy(itemName, figlabel((char *)getNthString(dynamicSoftmenu[m].menuContent, x+6*y), "", fnItemShowValue(ITM_SETSIG2)));
                    }
//CHECKNOW not needed in this place anymore??

                    char tmpC[16];
                    tmpC[0]=0;
                    xcopy(tmpC, allNamedVariables[currentSolverVariable - FIRST_NAMED_VARIABLE].variableName + 1, allNamedVariables[currentSolverVariable - FIRST_NAMED_VARIABLE].variableName[0]);
                    tmpC[ allNamedVariables[currentSolverVariable - FIRST_NAMED_VARIABLE].variableName[0]] = 0;
                    if(!compareString((char *)getNthString(dynamicSoftmenu[m].menuContent, x+6*y), tmpC, CMP_NAME)) {
                       strcpy(itemName, tmpC);
                       strcat(itemName, "*");
                    }
                    itemNr = 0;             // set to 0 to clear the previous itemNr to prevent unrelated strikethroughs
                    break;
                  }
                  case MNU_PROG:
                  case MNU_PROGS: {
                    itemNr = 0;             // set to 0 to clear the previous itemNr to prevent unrelated strikethroughs
                    vm = vmNormal;
                    break;
                  }
                  default: {
                    vm = vmNormal;
                    break;
                  }
                }
                showSoftkey(itemName, x, y, vm, true, true, showCb, showValue, showText);
                fnStrikeOutIfNotCoded(itemNr, x, y);
                fnStrikeThroughIfNA(itemNr, x, y);
              }
              ptr += stringByteLength((char *)ptr) + 1;
            }
          }
        }
      }
      if(softmenu[m].menuItem == -MNU_MVAR && (currentSolverStatus & SOLVER_STATUS_USES_FORMULA) && (currentSolverStatus & SOLVER_STATUS_INTERACTIVE)) {
        showEquation(currentFormula, 0, EQUATION_NO_CURSOR, false, NULL, NULL);
      }
    }
    else {  //normal (not Dynamic) softmenu
      const int16_t *softkeyItem = softmenu[m].softkeyItem + currentFirstItem;
      char itemName[16];
      for(y=currentFirstItem/6; y<=min(currentFirstItem/6+2, numberOfItems/6); y++, softkeyItem+=6) {
        for(x=0; x<6; x++) {
          if(softkeyItem + x >= softmenu[m].softkeyItem + numberOfItems) {
            item = ITM_NULL;
          }
          else {
            item = softkeyItem[x];
          }
          changeSoftKey(softmenu[m].menuItem, item, itemName, &vm, &showCb, &showValue, showText);


          if(item < 0) { // item is softmenu name
            int16_t menu = 0;
            while(softmenu[menu].menuItem != 0) {
              if(softmenu[menu].menuItem == item) {
                break;
              }
              menu++;
            }

            if(item == -MNU_ASN_N && calcModel == USER_C47) {
              showSoftkey(STD_SIGMA "+ KEY", x, y-currentFirstItem/6, vmReverse, true, true, NOVAL, NOVAL, NOTEXT);
            }
            else if(item == -MNU_ASN_N && isR47FAM) {
              showSoftkey(STD_BOX " KEY", x, y-currentFirstItem/6, vmReverse, true, true, NOVAL, NOVAL, NOTEXT);
            }
            else if(item == -MNU_HOME || item == -MNU_PFN ) {  //softmenu[menu].menuItem == 0, or does not exist
              showSoftkey(indexOfItems[-item].itemSoftmenuName, x, y-currentFirstItem/6, vmReverse, true, true, NOVAL, NOVAL, NOTEXT);
            }
            else if(softmenu[menu].menuItem == 0) {
              sprintf(errorMessage, "In function showSoftmenuCurrentPart: softmenu ID %" PRId16 " not found!", item);
              displayBugScreen(errorMessage);
            }
            else if(softmenu[menu].menuItem == -MNU_ALPHA_OMEGA && alphaCase == AC_UPPER) {
              showSoftkey(indexOfItems[MNU_ALPHA_OMEGA].itemSoftmenuName, x, y-currentFirstItem/6, vmReverse, true, true, NOVAL, NOVAL, NOTEXT);
            }
            else if(softmenu[menu].menuItem == -MNU_ALPHA_OMEGA && alphaCase == AC_LOWER) {
              showSoftkey(indexOfItems[MNU_alpha_omega].itemSoftmenuName, x, y-currentFirstItem/6, vmReverse, true, true, NOVAL, NOVAL, NOTEXT);
            }
            else if(softmenu[menu].menuItem == -MNU_ALPHAINTL && alphaCase == AC_UPPER) {
              showSoftkey(indexOfItems[MNU_ALPHAINTL].itemSoftmenuName, x, y-currentFirstItem/6, vmReverse, true, true, NOVAL, NOVAL, NOTEXT);
            }
            else if(softmenu[menu].menuItem == -MNU_ALPHAINTL && alphaCase == AC_LOWER) {
              showSoftkey(indexOfItems[MNU_ALPHAintl].itemSoftmenuName, x, y-currentFirstItem/6, vmReverse, true, true, NOVAL, NOVAL, NOTEXT);
            }
            else {
              #if defined(INLINE_TEST)
                if(softmenu[menu].menuItem == -MNU_INL_TST) {
                  showSoftkey(/*STD_omicron*/STD_SPACE_3_PER_EM, x, y-currentFirstItem/6, vmNormal, false, false, NOVAL, NOVAL, NOTEXT);
                }
                else {
              #endif // INLINE_TEST
              //MAIN SOFTMENU DISPLAY
              showSoftkey(indexOfItems[-softmenu[menu].menuItem].itemSoftmenuName, x, y-currentFirstItem/6, vmReverse, true, true, NOVAL, NOVAL, NOTEXT);
              #if defined(INLINE_TEST)
                }
              #endif // INLINE_TEST

            }
          } //softmenu


          else if(softmenu[m].menuItem == -MNU_SYSFL) {                                         //JMvv add radiobuttons to standard flags
            if(indexOfItems[item%10000].itemCatalogName[0] != 0) {
              if(isSystemFlagWriteProtected(indexOfItems[item%10000].param)) {
                showSoftkey(changeDotAndIJ(item,indexOfItems[item%10000].itemCatalogName),  x, y-currentFirstItem/6, vmNormal, (item/10000)==0 || (item/10000)==2, (item/10000)==0 || (item/10000)==1, showCb, getSystemFlag(indexOfItems[item%10000].param) ?  1 : 0, NOTEXT);
              }
              else {
                showSoftkey(changeDotAndIJ(item,indexOfItems[item%10000].itemCatalogName),  x, y-currentFirstItem/6, vmNormal, (item/10000)==0 || (item/10000)==2, (item/10000)==0 || (item/10000)==1, getSystemFlag(indexOfItems[item%10000].param) ?  CB_TRUE : CB_FALSE, NOVAL, NOTEXT);
              }
            }
          }                                                                      //JM^^

          else if(softmenu[m].menuItem == -MNU_TAMFLAG && indexOfItems[item%10000].itemCatalogName[0] != 0 && indexOfItems[item%10000].func == fnGetSystemFlag) {                                         //JMvv add radiobuttons to standard flags
            showSoftkey(indexOfItems[item%10000].itemCatalogName,  x, y-currentFirstItem/6, vmNormal, (item/10000)==0 || (item/10000)==2, (item/10000)==0 || (item/10000)==1, getSystemFlag(indexOfItems[item%10000].param) ?  CB_TRUE : CB_FALSE, NOVAL, NOTEXT);
          }                                                                      //JM^^

          else if(softmenu[m].menuItem == -MNU_ALPHA && calcMode == CM_PEM && item%10000 == ITM_ASSIGN) {
            // do nothing
          }

          else if(item > 0 && indexOfItems[item%10000].itemSoftmenuName[0] != 0) { // softkey
            // item : +10000 -> no top line
            //        +20000 -> no bottom line
            //        +30000 -> neither top nor bottom line

            if( softmenu[m].menuItem  == -MNU_CONVS    || softmenu[m].menuItem  == -MNU_CONVANG  ||
                softmenu[m].menuItem  == -MNU_CONVE    || softmenu[m].menuItem  == -MNU_CONVP    ||
                softmenu[m].menuItem  == -MNU_CONVFP   || softmenu[m].menuItem  == -MNU_CONVM    ||
                softmenu[m].menuItem  == -MNU_CONVX    || softmenu[m].menuItem  == -MNU_CONVV    ||
                softmenu[m].menuItem  == -MNU_CONVA    || softmenu[m].menuItem  == -MNU_UNITCONV ||
                softmenu[m].menuItem  == -MNU_MISC     || softmenu[m].menuItem  == -MNU_CONVHUM  ||
                softmenu[m].menuItem  == -MNU_CONVYMMV || softmenu[m].menuItem  == -MNU_CONVCHEF    ) {
              showSoftkey2(indexOfItems[item%10000].itemSoftmenuName, x, y-currentFirstItem/6, vmNormal, (item/10000)==0 || (item/10000)==2, (item/10000)==0 || (item/10000)==1, showCb, showValue, showText);
            }

            else {
              if( (softmenu[m].menuItem == -MNU_FCNS || softmenu[m].menuItem == -MNU_FCNS_EIM || softmenu[m].menuItem  == -MNU_CONST) || //CONST is a normal menu not a catalog, but we expect the catalog to be treated as a catalog. //The same could be a problem with any of the generated catalogs (MNU_SYSFL, MNU_alpha_INTL, MNU_alpha_intl, )
                 ((softmenu[m].menuItem == -MNU_IO   || softmenu[m].menuItem  == -MNU_PFN  ) && (item == ITM_STOCFG || item == ITM_RCLCFG))) { //do not display "Config"
                stringCopy(itemName, changeDotAndIJ(item,indexOfItems[item%10000].itemCatalogName));
              }
              showSoftkey(itemName, x, y-currentFirstItem/6, vm, (item/10000)==0 || (item/10000)==2, (item/10000)==0 || (item/10000)==1, showCb, showValue, showText);
            }


            //softkey modifications

            if((getSystemFlag(FLAG_G_DOUBLETAP) && ( BLOCK_DOUBLEPRESS_MENU(softmenu[m].menuItem, x, y))) ||           // Indicate disabled double tap
               (softmenu[m].menuItem == -MNU_TIMERF && y == 0)) {                           // If stopwatch is open
              int16_t yStrokeA = SCREEN_HEIGHT - (y-currentFirstItem/6)*23 - 1;
              int16_t xStrokeA=x*67 + 66 -12;
              plotline1(xStrokeA +2+4, yStrokeA -16-3-1, xStrokeA +2+4+5-1, yStrokeA -16-3+5);
            }
          }

          if(softmenu[m].menuItem == -MNU_TIMERF && y == 0) {
            char tmpp[16]; tmpp[0]=0;
            char tmpq[16]; tmpq[0]=0;
            switch(item) {
              case ITM_TIMER_SIGMA_T: sprintf(tmpq,"%s","[" STD_SIGMA "+]"); break;
              case ITM_TIMER_SIGMA_L: sprintf(tmpq,"%s","[+]"); break;
              case ITM_TIMER_R_T    : sprintf(tmpq,"%s","[ENTER]"); break;
              case ITM_TIMER_R_L    : sprintf(tmpq,"%s","[.]"); break;
              case ITM_TIMER_R_S    :
              case ITM_STOP         : sprintf(tmpq,"%s","[R/S]"); break;
              case ITM_TIMER_RESET  : sprintf(tmpq,"%s","[" STD_LEFT_ARROW "]"); break;
              default:break;
            }
            int16_t x1, y1, x2, y2;
            initSoftkeyCoordinates(tmpq, x, 2, &x1, &x2, &y1, &y2);
            showKey(tmpq, x1, x2, y1, y2, vmNormal, false, true, NOVAL, NOVAL, tmpp);
            // diagonals pattern
            for(int16_t line = y1 + 3; line < y2 - 2; line += 1){
              for(int16_t col = x1 + 3 + line%6; col < x2 - 2; col += 6) {
                  setBlackPixel(col, line);
              }
            }
          }

          fnStrikeOutIfNotCoded(item%10000, x, y-currentFirstItem/6);
          fnStrikeThroughIfNA(item%10000, x, y-currentFirstItem/6);
        }
      }


      if(softmenu[m].menuItem == -MNU_EQN) {
        showEquation(currentFormula, 0, EQUATION_NO_CURSOR, false, NULL, NULL);
        dottedTopLine = (numberOfFormulae >= 2);
        yDotted = 2;
      }
      int16_t currentMenu = softmenu[m].menuItem;
      if((currentMenu == -MNU_EQ_EDIT) || ((calcMode == CM_EIM) && ((currentMenu == -MNU_EIMCATALOG) || (currentMenu == -MNU_CHARS)))){
        bool_t cursorShown;
        bool_t rightEllipsis;
        while(1) {
          showEquation(EQUATION_AIM_BUFFER, yCursor, xCursor, true, &cursorShown, &rightEllipsis);
          if(cursorShown) {
            break;
          }
          if(yCursor > xCursor) {
            --yCursor;
          }
          else {
            ++yCursor;
          }
        }
        if(!rightEllipsis && yCursor > 0) {
          do {
            --yCursor;
            showEquation(EQUATION_AIM_BUFFER, yCursor, xCursor, true, &cursorShown, &rightEllipsis);
            if((!cursorShown) || rightEllipsis) {
              ++yCursor;
              break;
            }
          } while(yCursor > 0);
        }
        showEquation(EQUATION_AIM_BUFFER, yCursor, xCursor, false, NULL, NULL);
      }
      if( (softmenu[m].menuItem == -MNU_Sfdx || softmenu[m].menuItem == -MNU_Solver_TOOL || softmenu[m].menuItem == -MNU_Sf_TOOL || softmenu[m].menuItem == -MNU_GRAPHS)
       && (currentSolverStatus & SOLVER_STATUS_USES_FORMULA) && (currentSolverStatus & SOLVER_STATUS_INTERACTIVE)) {
        showEquation(currentFormula, 0, EQUATION_NO_CURSOR, false, NULL, NULL);
      }
    }

    if(0 <= yDotted && yDotted <= 2) {
      yDotted = 217 - SOFTMENU_HEIGHT * yDotted;

      if(dottedTopLine && (!GRAPHMODE || softmenu[m].menuItem == -MNU_PLOT_FUNC)) {
        for(x=0; x < (GRAPHMODE ? SCREEN_WIDTH / 3 : SCREEN_WIDTH); x++) {
          if(x%8 < 4) {
            setBlackPixel(x, yDotted);
          }
          else {
            setWhitePixel(x, yDotted);
          }
        }


                                         //JMvv    //triangle centre point  // Triangles indicating more menus
        #define t 5
        #define t_o 1.6*t                                                             //offset
        #define tt_o 2                                                             //total offset
        lcd_fill_rect(0,(uint32_t)(yDotted-t), 20,t+1, 0);                         // (see screen.c: _selectiveClearScreen)
        uint32_t xx;
        for(xx=0; xx<=t; xx++) {
          if(!catalog) {
            lcd_fill_rect(xx,       (uint32_t)(tt_o-t + yDotted-xx+t),   2*(t-xx), 1 ,true );
            lcd_fill_rect(xx + t_o, (uint32_t)(tt_o-t + yDotted-t+xx+t), 2*(t-xx), 1 ,true );
          }
          else {
            if(xx!=t) {
              lcd_fill_rect(xx,                 (uint32_t)(tt_o-t + yDotted-xx+t),   2, 1 ,true );
              lcd_fill_rect(xx+ 2*(t-xx)-1,     (uint32_t)(tt_o-t + yDotted-xx+t),   2, 1 ,true );
              lcd_fill_rect(xx+ t_o,               (uint32_t)(tt_o-t + yDotted-t+xx+t),   2, 1 ,true );
              lcd_fill_rect(xx+ t_o+ 2*(t-xx)-1,   (uint32_t)(tt_o-t + yDotted-t+xx+t),   2, 1 ,true );
            }
          }
                                                                            //JM ^^
        }
      }
    }
    showShiftState(); //JM
  }
}





                                                              //JM ^^



  /* Pushes a new softmenu on the softmenu stack.
   *
   * \param[in] softmenuId Softmenu ID
   */
  static void pushSoftmenu(int16_t softmenuId) {
    int i;
    int16_t userMenuId;

    #if defined(PC_BUILD)
      char tmp[300]; sprintf(tmp,">>> ...... pushing id:%d name:%s\n",softmenuId, indexOfItems[-softmenu[softmenuId].menuItem].itemSoftmenuName); jm_show_comment(tmp);
    #endif // PC_BUILD
    if(softmenu[softmenuId].menuItem == -MNU_DYNAMIC) {
      userMenuId = currentUserMenu;
    }
    else {
      userMenuId = 0;
    }
    if((softmenuStack[0].softmenuId == softmenuId) && (softmenuStack[0].userMenuId == userMenuId)) {   // The menu to push on the stack is already displayed
      return;
    }

    for(i=0; i<SOFTMENU_STACK_SIZE; i++) { // Searching the stack for the menu to push on the stack
      if((softmenuStack[i].softmenuId == softmenuId) && (softmenuStack[i].userMenuId == userMenuId)) { // if found, remove it

        if(!catalog) {                                                                                 //remember the page number if the menu you are opening was already open and in the stack
          if(!getSystemFlag(FLAG_MNUp1) && (calcMode == CM_NORMAL || calcMode == CM_NIM)) {
            lastCatalogPosition[CATALOG_NONE] = softmenuStack[i].firstItem;
            calcMode = softmenuStack[i].calcMode;
          } else {
            lastCatalogPosition[CATALOG_NONE] = 0;
          }
        }
        xcopy(softmenuStack + 1, softmenuStack, i * sizeof(softmenuStack_t));                          // Remove it by lifting the stack to cover the existing entry i
        break;
      }
    }

    if(i == SOFTMENU_STACK_SIZE) { // The menu to push was not found on the stack
      xcopy(softmenuStack + 1, softmenuStack, (SOFTMENU_STACK_SIZE - 1) * sizeof(softmenuStack_t)); // shifting the entire stack
    }


    softmenuStack[0].softmenuId = softmenuId;
    softmenuStack[0].firstItem = lastCatalogPosition[catalog];
    softmenuStack[0].userMenuId = userMenuId;
    softmenuStack[0].calcMode = calcMode;


    if((softmenu[softmenuId].menuItem == -MNU_CONVCHEF || softmenu[softmenuId].menuItem == -MNU_CONVV) &&
      ((menu(1) == -MNU_UNITCONV) ||
       (menu(1) == -MNU_MENUS && menu(2) == -MNU_CATALOG) )
      ) {
      softmenuStack[0].firstItem = getSystemFlag(FLAG_US) ? 18 : 0;
    }


    doRefreshSoftMenu = true;     //dr
  }



  void popSoftmenu(void) {
    screenUpdatingMode &= ~(SCRUPD_MANUAL_MENU | SCRUPD_SKIP_MENU_ONE_TIME);
    setScreenUpdateFromMenu(currentMenu(), closeMenu);

    xcopy(softmenuStack, softmenuStack + 1, (SOFTMENU_STACK_SIZE - 1) * sizeof(softmenuStack_t)); // shifting the entire stack
    memset(softmenuStack + SOFTMENU_STACK_SIZE - 1, 0, sizeof(softmenuStack_t)); // Put MyMenu in the last stack element

    doRefreshSoftMenu = true;     //dr

    if(softmenuStack[0].softmenuId == 0 && calcMode == CM_AIM) { // MyMenu displayed and in AIM
      softmenuStack[0].softmenuId = 1; // MyAlpha
    }
    else if(softmenuStack[0].softmenuId == 1 && calcMode != CM_AIM) { // MyAlpha displayed and not in AIM
      softmenuStack[0].softmenuId = 0; // MyMenu
    }
    if(softmenuStack[0].softmenuId == 0 && getSystemFlag(FLAG_BASE_HOME) && calcMode != CM_AIM) {
      changeToHOME();
    }
    else if(softmenuStack[0].softmenuId == 0 && getSystemFlag(FLAG_BASE_MYM) && calcMode != CM_AIM) {
      //softmenuStack[0].softmenuId = 0;                                                       //already 0, not needed to change
    }
    else if(softmenuStack[0].softmenuId == 1 && calcMode == CM_AIM) {
      changeToALPHA();
    }

    softmenuStack[0].calcMode = calcMode;


//Completely remove the resetting of menu page numbers when menu items are POPPED. When new menu pages are opened, it still always defaults to p1 though.
//    switch(-currentMenu()) {               //reset menu base point only if not MODE & DISP & Graphs menus, otherwise all menues reset to p1
//      case MNU_MODE   :
//      case MNU_PREF   :                    //(see "do not drop out of SYSFLG" in keyboard.c)
//      case MNU_DISP   :
//      case MNU_PLOT_FUNC   : break;
//      default: {
//        softmenuStack[0].firstItem = 0;
//        break;
//      }
//    }

    enterAsmModeIfMenuIsACatalog(softmenu[softmenuStack[0].softmenuId].menuItem);

    if(softmenu[softmenuStack[0].softmenuId].menuItem == -MNU_MVAR) {
      setSystemFlag(FLAG_VMDISP);
    }
    else {
      clearSystemFlag(FLAG_VMDISP);
    }

    if(softmenu[softmenuStack[0].softmenuId].menuItem == -MNU_DYNAMIC) {
      currentUserMenu = softmenuStack[0].userMenuId;
    }

    #if defined(PC_BUILD)
      jm_show_calc_state("popped");
      char tmp[300]; sprintf(tmp,">>> ...... popped into [0]: Id:%d Name:%s\n",softmenuStack[0].softmenuId, indexOfItems[-softmenu[softmenuStack[0].softmenuId].menuItem].itemSoftmenuName); jm_show_comment(tmp);
    #endif // PC_BUILD
  }


  bool_t setCurrentUserMenu(int16_t item, char* funcParam) {
    if(item == -MNU_DYNAMIC) {
      for(uint32_t i = 0; i < numberOfUserMenus; ++i) {
        if(compareString(funcParam, userMenus[i].menuName, CMP_NAME) == 0) {
            currentUserMenu = i;
            return true;
        }
      }
    }
    return false;
  }


  bool_t createHOME(void) {
    int16_t itemToBeAssignedMeM = itemToBeAssigned;
    if(!setCurrentUserMenu(-MNU_DYNAMIC,"HOME")) {
      createMenu("HOME");
      if(!setCurrentUserMenu(-MNU_DYNAMIC,"HOME")) {
        itemToBeAssigned = itemToBeAssignedMeM;
        return false;
      }
    }
    #if defined (PC_BUILD)
      printf("----------- ############################ CREATING HOME #########################\n");
    #endif //PC_BUILD
    for(uint16_t ii=0; ii<18; ii++) {
      itemToBeAssigned = ITM_ENTER;
      screenUpdatingMode = ~SCRUPD_AUTO;
      doRefreshSoftMenu = false;
      last_CM = 240;
      assignToUserMenu(ii);
      itemToBeAssigned = menu_HOME[ii];
      screenUpdatingMode = ~SCRUPD_AUTO;
      doRefreshSoftMenu = false;
      last_CM = 240;
      assignToUserMenu(ii);
    }
    screenUpdatingMode = SCRUPD_AUTO;
    refreshScreen(170);
    itemToBeAssigned = itemToBeAssignedMeM;
    return true;
  }

  bool_t createPFN(void) {
    int16_t itemToBeAssignedMeM = itemToBeAssigned;
    if(!setCurrentUserMenu(-MNU_DYNAMIC,"P.FN")) {
      createMenu("P.FN");
      if(!setCurrentUserMenu(-MNU_DYNAMIC,"P.FN")) {
        itemToBeAssigned = itemToBeAssignedMeM;
        return false;
      }
    }
    #if defined (PC_BUILD)
      printf("----------- ############################ CREATING PFN #########################\n");
    #endif //PC_BUILD
    for(uint16_t ii=0; ii<18; ii++) {
      itemToBeAssigned = ITM_ENTER;
      screenUpdatingMode = ~SCRUPD_AUTO;
      doRefreshSoftMenu = false;
      last_CM = 240;
      assignToUserMenu(ii);
      itemToBeAssigned = menu_MyPFN[ii];
      screenUpdatingMode = ~SCRUPD_AUTO;
      doRefreshSoftMenu = false;
      last_CM = 240;
      assignToUserMenu(ii);
    }
    screenUpdatingMode = SCRUPD_AUTO;
    refreshScreen(171);
    itemToBeAssigned = itemToBeAssignedMeM;
    return true;
  }



  void changeToHOME(void) {
    showSoftmenu(-MNU_HOME);
  }

  void changeToPFN(void) {
    showSoftmenu(-MNU_PFN);
  }

  void changeToALPHA(void) {
    showSoftmenu(-MNU_ALPHA);
  }


  int16_t menu(uint8_t n) {
    if(softmenu[softmenuStack[n].softmenuId].menuItem == -MNU_DYNAMIC && compareString("HOME", userMenus[currentUserMenu].menuName, CMP_NAME) == 0) {
       return -MNU_HOME;
    }
    else if(softmenu[softmenuStack[n].softmenuId].menuItem == -MNU_DYNAMIC && compareString("P.FN", userMenus[currentUserMenu].menuName, CMP_NAME) == 0) {
       return -MNU_PFN;
    }
    else {
       return softmenu[softmenuStack[n].softmenuId].menuItem;
    }
  }

  int16_t currentMenu(void) {
    return menu(0);
  }


  bool_t isAlphaSubmenu(uint8_t n) {
    return menu(n) == -MNU_MyAlpha ||
            menu(n) == -MNU_ALPHA_OMEGA ||
            menu(n) == -MNU_alpha_omega ||
            menu(n) == -MNU_ALPHAMATH ||
            menu(n) == -MNU_ALPHAMISC ||
            menu(n) == -MNU_ALPHAINTL ||
            menu(n) == -MNU_ALPHAintl;
  }



  void removeUserMenuFromStack(int16_t userMenuId) {
    int i;
    bool_t all = (userMenuId == numberOfUserMenus ? 1 : 0);

    for(i=0; i<SOFTMENU_STACK_SIZE; i++) { // Searching the stack for the user menu to remove from the stack
      #if defined(PC_BUILD) && (VERBOSE_LEVEL > 0)
        printf("*** Id %d item %d menuItem %d userMenuId %d\n",userMenuId,i,softmenu[softmenuStack[i].softmenuId].menuItem,softmenuStack[i].userMenuId);
      #endif
      if(softmenu[softmenuStack[i].softmenuId].menuItem == -MNU_DYNAMIC) {
        if((softmenuStack[i].userMenuId == userMenuId) || all==true) { // if found, remove it
          #if defined(PC_BUILD) && (VERBOSE_LEVEL > 0)
            printf("*** Remove userMenuId %d\n", softmenuStack[i].userMenuId);
          #endif // PC_BUILD
          xcopy(softmenuStack + i, softmenuStack + i + 1, (SOFTMENU_STACK_SIZE - i) * sizeof(softmenuStack_t));
          softmenuStack[SOFTMENU_STACK_SIZE - 1].softmenuId = 0;  // Put MyMenu in the last stack element
          softmenuStack[SOFTMENU_STACK_SIZE - 1].firstItem = 0;
          softmenuStack[SOFTMENU_STACK_SIZE - 1].userMenuId = 0;
          softmenuStack[SOFTMENU_STACK_SIZE - 1].calcMode = 0;
          i--;
        }
        else if(softmenuStack[i].userMenuId > userMenuId) { // adjust other menuIDs             //this makes no sense!!
          softmenuStack[i].userMenuId--;
        }
      }
    }
    if(softmenuStack[0].softmenuId == 0 && getSystemFlag(FLAG_BASE_HOME) && calcMode != CM_AIM) {
      changeToHOME();
    }
  }


  void removeMenuFromStack(int16_t userMenuId) {
    for(int i=SOFTMENU_STACK_SIZE-1; i>=0; i--) { // Searching the stack for specified menu (JM: reversed action to prevent a possible endless loop +1, -1, ...)
      //printf("-%1i-",i);
      if(menu(i) == userMenuId) { // if found, remove it
        xcopy(softmenuStack + i, softmenuStack + i + 1, (SOFTMENU_STACK_SIZE - i - 1) * sizeof(softmenuStack_t));
        memset(softmenuStack + SOFTMENU_STACK_SIZE - 1, 0, sizeof(softmenuStack_t)); // Put MyMenu in the last stack element
        //printf("Blanking %i: %i %s | %i %s\n",i, softmenu[softmenuStack[i].softmenuId].menuItem, indexOfItems[abs(softmenu[softmenuStack[i].softmenuId].menuItem)].itemCatalogName, menu(i), indexOfItems[abs(menu(i))].itemCatalogName);
      }
    }
  }


  void extractPFNMenus(void) {
    for(int ii=SOFTMENU_STACK_SIZE-1; ii>=0; ii--) { // Searching the stack for specified menu
      //printf("+%1i+",ii);
      if(softmenuStack[ii].calcMode == CM_PEM || menu(ii) == -MNU_PFN) {
        //printf("Removing %i: %i %s | %i %s\n",ii, softmenu[softmenuStack[ii].softmenuId].menuItem, indexOfItems[abs(softmenu[softmenuStack[ii].softmenuId].menuItem)].itemCatalogName, menu(ii), indexOfItems[abs(menu(ii))].itemCatalogName);
        removeMenuFromStack(menu(ii));
      }
    }
  }

  void showSoftmenu(int16_t id) {
    int16_t m;
    #if defined(PC_BUILD)
      char tmp[200]; sprintf(tmp,"ShowSoftmenu: opening Softmenu, item=%i %s\n", currentMenu(), indexOfItems[currentMenu() > 0 ? currentMenu() : -currentMenu()].itemSoftmenuName);
      jm_show_comment(tmp);
    #endif // PC_BUILD

    #if !defined(INLINE_TEST)
      if(id == -MNU_INL_TST) {
        return;
      }
    #endif // !INLINE_TEST

    screenUpdatingMode &= ~(SCRUPD_MANUAL_MENU | SCRUPD_SKIP_MENU_ONE_TIME);
    setScreenUpdateFromMenu(id, openMenu);


    //* *** List of exceptions, fixed menu call finds and opens the equivalent underlying dynamic menu (P.fN and HOME are now user populated, in the user menu space)
    if(id == -MNU_HOME) {
      if(!setCurrentUserMenu(-MNU_DYNAMIC,"HOME")) {
        if(!createHOME()) {
          return;
        }
      }
      id = -MNU_DYNAMIC;
    }
    else if(id == -MNU_PFN) {
      if(!setCurrentUserMenu(-MNU_DYNAMIC,"P.FN")) {
        if(!createPFN()) {
          return;
        }
      }
      id = -MNU_DYNAMIC;
    }


    enterAsmModeIfMenuIsACatalog(id);

    if(id == 0) {
      displayBugScreen(bugScreenIdMustNotBe0);
      return;
    }
   if((softmenu[softmenuStack[0].softmenuId].menuItem == id) && (softmenuStack[0].softmenuId >= NUMBER_OF_DYNAMIC_SOFTMENUS) && !catalog) {   // The menu to push on the stack is already displayed and current, so just change back to p1
     softmenuStack[0].firstItem = 0;
     return;
   }

    screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;

    if(id == -MNU_ALPHAINTL && alphaCase == AC_LOWER) { // alphaINTL
      id = -MNU_ALPHAintl;
    }
    else if(id == -MNU_ALPHA_OMEGA && alphaCase == AC_LOWER) { // alpha...omega
      id = -MNU_alpha_omega;
    }

    else if( ((id == -MNU_Solver      ||
               id == -MNU_Grapher     ||
               id == -MNU_Sf          ||
               id == -MNU_Sf_TOOL     ||
               id == -MNU_Solver_TOOL ||
               id == -MNU_1STDERIV    ||
               id == -MNU_2NDDERIV) && numberOfFormulae >= 1)
               ||
              (id == -MNU_MVAR && (currentSolverStatus & SOLVER_STATUS_INTERACTIVE) && !(currentSolverStatus & SOLVER_STATUS_USES_FORMULA) && (currentSolverStatus & SOLVER_STATUS_EQUATION_MODE) == SOLVER_STATUS_EQUATION_INTEGRATE)
           ) {

      int32_t numberOfVars = -1;
      uint8_t *varList = NULL;
      if(id != -MNU_MVAR) {
        currentSolverStatus = SOLVER_STATUS_USES_FORMULA | SOLVER_STATUS_INTERACTIVE;
        currentMvarLabel = INVALID_VARIABLE;
      }
      switch(-id) {
        case MNU_Grapher: {
          currentSolverStatus &= ~SOLVER_STATUS_EQUATION_MODE;
          currentSolverStatus |= SOLVER_STATUS_EQUATION_GRAPHER;
          break;
        }
        case MNU_Solver_TOOL:
        case MNU_Solver: {
          currentSolverStatus &= ~SOLVER_STATUS_EQUATION_MODE;
          currentSolverStatus |= SOLVER_STATUS_EQUATION_SOLVER;
          break;
        }
        case MNU_Sf_TOOL:
        case MNU_Sf: {
          currentSolverStatus &= ~SOLVER_STATUS_EQUATION_MODE;
          currentSolverStatus |= SOLVER_STATUS_EQUATION_INTEGRATE;
          break;
        }
        case MNU_1STDERIV: {
          currentSolverStatus &= ~SOLVER_STATUS_EQUATION_MODE;
          currentSolverStatus |= SOLVER_STATUS_EQUATION_1ST_DERIVATIVE;
          break;
        }
        case MNU_2NDDERIV: {
          currentSolverStatus &= ~SOLVER_STATUS_EQUATION_MODE;
          currentSolverStatus |= SOLVER_STATUS_EQUATION_2ND_DERIVATIVE;
          break;
        }
      }
      cachedDynamicMenu = 0;
      if(id == -MNU_MVAR) {
        for(int m = 0; m < NUMBER_OF_DYNAMIC_SOFTMENUS; ++m) {
          if(softmenu[m].menuItem == -MNU_MVAR) {
            initVariableSoftmenu(m);
            varList = dynamicSoftmenu[m].menuContent;
            (getNthString(varList, dynamicSoftmenu[m].numItems))[0] = 0;
            break;
          }
        }
        if(varList == NULL) {
          displayBugScreen("In function showSoftmenu: MVAR not found!");
          varList = (uint8_t *)"\0";
        }
      }
      else {
        parseEquation(currentFormula, EQUATION_PARSER_MVAR, aimBuffer, tmpString);
        varList = (uint8_t *)tmpString;
      }

      if(id != -MNU_Solver_TOOL && id != -MNU_Sf_TOOL) {
        id = -MNU_MVAR;
      }
      while((getNthString(varList, ++numberOfVars))[0] != 0) {
      }

      if(numberOfVars > 12) {
        displayCalcErrorMessage(ERROR_EQUATION_TOO_COMPLEX, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function showSoftmenu:", "there are more than 12 variables in this equation!", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      else if( ( ((currentSolverStatus & SOLVER_STATUS_EQUATION_MODE) == SOLVER_STATUS_EQUATION_INTEGRATE) ||
                 ((currentSolverStatus & SOLVER_STATUS_EQUATION_MODE) == SOLVER_STATUS_EQUATION_SOLVER) ||
                 ((currentSolverStatus & SOLVER_STATUS_EQUATION_MODE) == SOLVER_STATUS_EQUATION_GRAPHER)
               ) && numberOfVars == 1) {
        currentSolverVariable = findOrAllocateNamedVariable((char *)getNthString(varList, 0));
      }
      else if((currentSolverStatus & SOLVER_STATUS_EQUATION_MODE) == SOLVER_STATUS_EQUATION_1ST_DERIVATIVE || (currentSolverStatus & SOLVER_STATUS_EQUATION_MODE) == SOLVER_STATUS_EQUATION_2ND_DERIVATIVE) {
        if((getNthString(varList, 1))[0] == 0) {
          currentSolverVariable = findOrAllocateNamedVariable((char *)getNthString(varList, 0));
          reallyRunFunction(ITM_STO, currentSolverVariable);
          saveForUndo();
          if((currentSolverStatus & SOLVER_STATUS_EQUATION_MODE) == SOLVER_STATUS_EQUATION_1ST_DERIVATIVE) {
            fn1stDerivEq(NOPARAM);
          }
          else {
            fn2ndDerivEq(NOPARAM);
          }
        }
      }

    }
    else if(id == -MNU_ADV || id == -MNU_EQN) {
      currentSolverStatus &= ~SOLVER_STATUS_INTERACTIVE;
      removeMenuFromStack(-MNU_MVAR);
    }

    else if(id == -MNU_GRAPHS) {
      currentSolverStatus &= ~SOLVER_STATUS_EQUATION_MODE;
      currentSolverStatus |= SOLVER_STATUS_EQUATION_GRAPHER;
    }

    else if(currentSolverVariable == INVALID_VARIABLE) {
      if(id == -MNU_Sf_TOOL    ||
         id == -MNU_Sf         ||
         id == -MNU_Solver     ||
         id == -MNU_Grapher    ||
         id == -MNU_Solver_TOOL||
         id == -MNU_1STDERIV   ||
         id == -MNU_2NDDERIV     ) {
        id = -MNU_EQN;
        displayCalcErrorMessage(ERROR_VARIABLE_NOT_SELECTED, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function showSoftmenu:", "The solver/integrator variable is not selected. Refusing access to Tools/Solver menu prior to variable selected!", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
    }

    m = 0;
    while(softmenu[m].menuItem != 0) {
      if(softmenu[m].menuItem == id) {
        if(!tam.mode) {
//Removed, as it it is clearly seems a relic of a previous incarnation! JM 2025-03-07. Originates from Martin in 2020, clearly prior to many many other changes
//          softmenuStack[0].firstItem = lastCatalogPosition[catalog];
        }
        break;
      }
      m++;
    }

    if(softmenu[m].menuItem == 0) {
      sprintf(errorMessage, "In function showSoftmenu: softmenu %" PRId16 " not found!", id);
      displayBugScreen(errorMessage);
    }
    else {
      if(tam.mode || (calcMode == CM_ASSIGN && tam.alpha)) {
        numberOfTamMenusToPop++;
      }
      pushSoftmenu(m);
      if(id == -MNU_MVAR) {
        setSystemFlag(FLAG_VMDISP);
      }
      else {
        clearSystemFlag(FLAG_VMDISP);
      }
    }
  }



  void setCatalogLastPos(void) {
    lastCatalogPosition[catalog] = (catalog ? softmenuStack[0].firstItem : 0);

    if(catalog == CATALOG_AINT) {
      lastCatalogPosition[CATALOG_aint] = softmenuStack[0].firstItem;
    }
    else if(catalog == CATALOG_aint) {
      lastCatalogPosition[CATALOG_AINT] = softmenuStack[0].firstItem;
    }
  }

  bool_t currentSoftmenuScrolls(void) {
    int16_t menuId = softmenuStack[0].softmenuId;
    return (menuId > 1 &&
      (   (menuId <  NUMBER_OF_DYNAMIC_SOFTMENUS && dynamicSoftmenu[menuId].numItems > 18)
       || (menuId >= NUMBER_OF_DYNAMIC_SOFTMENUS &&        softmenu[menuId].numItems > 18)));
  }

  bool_t isAlphabeticSoftmenu(void) {
    return isAlphaSubmenu(0);
  }

  bool_t isJMAlphaSoftmenu(int16_t menuId) {                   //JM
    int16_t menuItem = softmenu[menuId].menuItem;
    switch(menuItem) {
      case -MNU_MyAlpha:
      case -MNU_ALPHA:   //JM
        return true;
      default:
        return false;
    }
  }

  bool_t isJMAlphaOnlySoftmenu(void) {                    //JM
    return softmenu[softmenuStack[0].softmenuId].menuItem == -MNU_ALPHA;
  }


#endif // !TESTSUITE_BUILD


// input param is (PageNumber << 14) +MenuNumber
void fnPseudoMenu(uint16_t target) {
  menuPageNumber = target >> 14;
  fnOpenMenu(((int16_t)(target & 0x3fff)));
}


char *dynmenuGetLabel(int16_t menuitem) {
  return dynmenuGetLabelWithDup(menuitem, NULL);
}



char *dynmenuGetLabelWithDup(int16_t menuitem, int16_t *dupNum) {
  if(dupNum) {
    *dupNum = 0;
  }
  if(menuitem < 0 || menuitem >= dynamicSoftmenu[softmenuStack[0].softmenuId].numItems) {
    return "";
  }
  char *labelName = (char *)dynamicSoftmenu[softmenuStack[0].softmenuId].menuContent;
  char *prevLabelName = labelName;
  while(menuitem > 0) {
    labelName += stringByteLength(labelName) + 1;
    menuitem--;
    if(dupNum) {
      if(compareString(labelName, prevLabelName, CMP_BINARY) == 0) {
        ++(*dupNum);
      }
      else {
        prevLabelName = labelName;
        *dupNum = 0;
      }
    }
  }
  return labelName;
}


void fnBaseMenu(uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
    BASE_OVERRIDEONCE = true;
    showSoftmenu(-MNU_MyMenu);
  #endif // !TESTSUITE_BUILD
}


void fnExitAllMenus(uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
  uint16_t cnt = SOFTMENU_STACK_SIZE - 1;
  while((softmenu[softmenuStack[0].softmenuId].menuItem != -MNU_MyMenu && softmenu[softmenuStack[0].softmenuId].menuItem != -MNU_MyAlpha) || (softmenu[softmenuStack[1].softmenuId].menuItem != -MNU_MyMenu)) {
    popSoftmenu();
    if(cnt-- == 0) break;
  }
  softmenuStack[1].softmenuId = 0;
  popSoftmenu();

  //fnDumpMenus(0);   //Easy place to access the Dump Menus: PFN / More / ExitAll
#endif // !TESTSUITE_BUILD
}



void fnMenuDump(uint16_t menu, uint16_t item, uint16_t newFilenameformat) {                              //JMvv procedure to dump all menus. First page only. To mod todump all pages
#if defined(PC_BUILD)
  doRefreshSoftMenu = true;
  showSoftmenu(softmenu[menu].menuItem);
  softmenuStack[0].firstItem += item;
  showSoftmenuCurrentPart();

  FILE *bmp;
  char bmpFileName[600];
  int32_t x, y;
  uint32_t uint32;
  uint16_t uint16;
  uint8_t  uint8;

  // stub
  if(0) {
    gtk_main_iteration();
  }

  //printf(">>> %s\n",indexOfItems[-softmenu[menu].menuItem].itemSoftmenuName);
  char asciiString[448];
  char asciiMenuName[448];

  if(newFilenameformat == 2) {
    stringToASCII(indexOfItems[-softmenu[menu].menuItem].itemSoftmenuName, asciiMenuName);
    //printf(">>> Menustring:%s|",asciiMenuName);
    stringToFileNameChars(asciiMenuName, asciiString);
    //printf(">>> Menustring:%s|",asciiString);
    sprintf(bmpFileName,"%s.%d.bmp", asciiString, (int)(item/18)+1);
    printf(">>> filename:%s|\n",bmpFileName);
  } else   if(newFilenameformat == 1) {
    stringToASCII(indexOfItems[-softmenu[menu].menuItem].itemSoftmenuName, asciiMenuName);
    //printf(">>> Menustring:%s|",asciiMenuName);
    stringToFileNameChars(asciiMenuName, asciiString);
    //printf(">>> Menustring:%s|",asciiString);
    sprintf(bmpFileName,"Menu_%03d_p%d_%s.bmp", menu, (int)(item/18)+1, asciiString);
    printf(">>> filename:%s|\n",bmpFileName);
  }


  bmp = fopen(bmpFileName, "wb");

  fwrite("BM", 1, 2, bmp);        // Offset 0x00  0  BMP header

  uint32 = (SCREEN_WIDTH/8 * (SCREEN_HEIGHT-171)) + 610;
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x02  2  File size

  uint32 = 0;
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x06  6  unused

  uint32 = 0x00000082;
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x0a 10  Offset where the bitmap data can be found

  uint32 = 0x0000006c;
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x0e 14  Number of bytes in DIB header

  uint32 = SCREEN_WIDTH;
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x12 18  Bitmap width

  uint32 = SCREEN_HEIGHT-171;
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x16 22  Bitmap height

  uint16 = 0x0001;
  fwrite(&uint16, 1, 2, bmp);     // Offset 0x1a 26  Number of planes

  uint16 = 0x0001;
  fwrite(&uint16, 1, 2, bmp);     // Offset 0x1c 28  Number of bits per pixel

  uint32 = 0;
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x1e 30  Compression

  uint32 = 0x000030c0;
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x22 34  Size of bitmap data (including padding)

  uint32 = 0x00001a7c; // 6780 pixels/m
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x26 38  Horizontal print resolution

  uint32 = 0x00001a7c; // 6780 pixels/m
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x2a 42  Vertical print resolution

  uint32 = 0x00000002;
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x2e 46  Number of colors in the palette

  uint32 = 0x00000002;
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x32 50  Number of important colors

  uint32 = 0x73524742;
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x36  ???

  uint32 = 0;
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x3a  ???
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x3e  ???
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x42  ???
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x46  ???
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x4a  ???
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x4e  ???
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x52  ???
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x56  ???
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x5a  ???
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x5e  ???
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x62  ???
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x66  ???

  uint32 = 0x00000002;
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x6a  ???

  uint32 = 0;
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x6e  ???
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x72  ???
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x76  ???

  uint32 = 0x00dff5cc; // light green
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x7a  RGB color for 0

  uint32 = 0;
  fwrite(&uint32, 1, 4, bmp);     // Offset 0x7e  RGB color for 1

  // Offset 0x82  bit map data
  uint16 = 0;
  uint32 = 0;
  for(y=SCREEN_HEIGHT-1; y>=171; y--) {
    for(x=0; x<SCREEN_WIDTH; x++) {
      uint8 <<= 1;
      if(*(screenData + y*screenStride + x) == ON_PIXEL) {
        uint8 |= 1;
      }

      if((x % 8) == 7) {
        fwrite(&uint8, 1, 1, bmp);
        uint8 = 0;
      }
    }
    fwrite(&uint16, 1, 2, bmp); // Padding
  }


  fclose(bmp);
  popSoftmenu();
#endif // PC_BUILD
}


void fnDumpMenus(uint16_t newFilenameformat) {                      //JM
#if defined(PC_BUILD)
  int cc = currentSolverStatus;
  currentSolverStatus = currentSolverStatus & (SOLVER_STATUS_USES_FORMULA | SOLVER_STATUS_INTERACTIVE);
  printf("Dumping menus\n");
  int16_t m,n;
  m = 0;
    while(softmenu[m].menuItem != 0) {
      n=0;
      while(n < softmenu[m].numItems && softmenu[m].numItems != 0) {
        printf("m=%d n=%d softmenu[%u].numItems=%u name:%s.%u\n",m,n,m,softmenu[m].numItems, indexOfItems[m].itemCatalogName, n%18);
        switch(-softmenu[m].menuItem) {
          case MNU_1STDERIV :
          case MNU_2NDDERIV :
          case MNU_Sf       :
          case MNU_Solver   :
          case MNU_Grapher   :
          case MNU_SHOW     :
            break;
          default:
           fnMenuDump(m, n, newFilenameformat);
         }
        n += 18;
      }
      m++;
    }
  currentSolverStatus = cc;
#endif // PC_BUILD
}                                                                            //JM^^


