// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file conversionUnits.c
 ***********************************************/

#include "c47.h"

#define inverting true
#define noninverting false


TO_QSPI const fInMim_t MimFunctionsType3Conv[NUM_CONVERT_PAIRS] =
  {   //conversion
    {ITM_CtoF           },
    {ITM_FtoC           },
    {ITM_DBtoPR         },
    {ITM_FT2toHA        },
    {ITM_HAtoFT2        },
    {ITM_DBtoFR         },
    {ITM_FT2toM2        },
    {ITM_M2toFT2        },
    {ITM_PRtoDB         },
    {ITM_HAtoKM2        },
    {ITM_KM2toHA        },
    {ITM_FRtoDB         },
    {ITM_GLUKtoFZUK     },
    {ITM_FZUKtoGLUK     },
    {ITM_ACtoHA         },
    {ITM_MLtoIN3        },
    {ITM_HAtoAC         },
    {ITM_IN3toML        },
    {ITM_ACUStoHA       },
    {ITM_FT3toGLUK      },
    {ITM_HAtoACUS       },
    {ITM_GLUKtoFT3      },
    {ITM_PAtoATM        },
    {ITM_ATMtoPA        },
    {ITM_AUtoM          },
    {ITM_MtoAU          },
    {ITM_BARtoPA        },
    {ITM_PAtoBAR        },
    {ITM_BTUtoJ         },
    {ITM_JtoBTU         },
    {ITM_CALtoJ         },
    {ITM_JtoCAL         },
    {ITM_LBFFTtoNM      },
    {ITM_LtoFT3         },
    {ITM_NMtoLBFFT      },
    {ITM_FT3toL         },
    {ITM_CWTtoKG        },
    {ITM_KGtoCWT        },
    {ITM_FTtoM          },
    {ITM_MtoFT          },
    {ITM_FTUStoM        },
    {ITM_LtoQTUS        },
    {ITM_QTUStoL        },
    {ITM_MtoFTUS        },
    {ITM_IN3toFZUK      },
    {ITM_FZUKtoIN3      },
    {ITM_FZUKtoML       },
    {ITM_IN3toFZUS      },
    {ITM_MLtoFZUK       },
    {ITM_FZUStoIN3      },
    {ITM_FZUStoML       },
    {ITM_FT3toGalUS     },
    {ITM_MLtoFZUS       },
    {ITM_GalUStoFT3     },
    {ITM_GLUKtoL        },
    {ITM_LtoGLUK        },
    {ITM_GLUStoL        },
    {ITM_LtoGLUS        },
    {ITM_HPEtoW         },
    {ITM_WtoHPE         },
    {ITM_HPMtoW         },
    {ITM_WtoHPM         },
    {ITM_HPUKtoW        },
    {ITM_WtoHPUK        },
    {ITM_INCHHGtoPA     },
    {ITM_CUPCtoFZUS     },
    {ITM_PAtoINCHHG     },
    {ITM_CUPCtoML       },
    {ITM_INCHtoMM       },
    {ITM_MMtoINCH       },
    {ITM_WHtoJ          },
    {ITM_JtoWH          },
    {ITM_KGtoLBS        },
    {ITM_LBStoKG        },
    {ITM_GtoOZ          },
    {ITM_OZtoG          },
    {ITM_KGtoSCW        },
    {ITM_CUPUKtoFZUK    },
    {ITM_SCWtoKG        },
    {ITM_CUPUKtoML      },
    {ITM_KGtoSTO        },
    {ITM_FZUKtoCUPUK    },
    {ITM_STOtoKG        },
    {ITM_FZUKtoTBSPUK   },
    {ITM_KGtoST         },
    {ITM_FZUKtoTSPUK    },
    {ITM_FZUStoCUPC     },
    {ITM_STtoKG         },
    {ITM_FZUStoTBSPC    },
    {ITM_FZUStoTSPC     },
    {ITM_KGtoLT         },
    {ITM_KGtoLIANG      },
    {ITM_MLtoCUPC       },
    {ITM_LTtoKG         },
    {ITM_LIANGtoKG      },
    {ITM_MLtoCUPUK      },
    {ITM_GtoTRZ         },
    {ITM_MLtoPINTLQ     },
    {ITM_TRZtoG         },
    {ITM_MLtoPINTUK     },
    {ITM_LBFtoN         },
    {ITM_NtoLBF         },
    {ITM_LYtoM          },
    {ITM_MtoLY          },
    {ITM_MMHGtoPA       },
    {ITM_MLtoQT         },
    {ITM_PAtoMMHG       },
    {ITM_MLtoQTUS       },
    {ITM_MItoKM         },
    {ITM_KMtoMI         },
    {ITM_KMtoNMI        },
    {ITM_NMItoKM        },
    {ITM_MtoPC          },
    {ITM_PCtoM          },
    {ITM_MMtoPOINT      },
    {ITM_MLtoTBSPC      },
    {ITM_MILEtoM        },
    {ITM_POINTtoMM      },
    {ITM_MLtoTBSPUK     },
    {ITM_MtoMILE        },
    {ITM_MtoYD          },
    {ITM_YDtoM          },
    {ITM_PSItoPA        },
    {ITM_PAtoPSI        },
    {ITM_PAtoTOR        },
    {ITM_MLtoTSPC       },
    {ITM_TORtoPA        },
    {ITM_MLtoTSPUK      },
    {ITM_StoYEAR        },
    {ITM_YEARtoS        },
    {ITM_CARATtoG       },
    {ITM_PINTLQtoML     },
    {ITM_JINtoKG        },
    {ITM_GtoCARAT       },
    {ITM_PINTUKtoML     },
    {ITM_KGtoJIN        },
    {ITM_QTtoL          },
    {ITM_LtoQT          },
    {ITM_FATHOMtoM      },
    {ITM_QTtoML         },
    {ITM_NMItoM         },
    {ITM_MtoFATHOM      },
    {ITM_QTUStoML       },
    {ITM_MtoNMI         },
    {ITM_BARRELtoM3     },
    {ITM_TBSPCtoFZUS    },
    {ITM_M3toBARREL     },
    {ITM_HMStoHR        },
    {ITM_HRtoHMS        },
    {ITM_TBSPCtoML      },
    {ITM_HECTAREtoM2    },
    {ITM_M2toHECTARE    },
    {ITM_MUtoM2         },
    {ITM_M2toMU         },
    {ITM_LItoM          },
    {ITM_MtoLI          },
    {ITM_CHItoM         },
    {ITM_MtoCHI         },
    {ITM_YINtoM         },
    {ITM_MtoYIN         },
    {ITM_CUNtoM         },
    {ITM_MtoCUN         },
    {ITM_ZHANGtoM       },
    {ITM_TBSPUKtoFZUK   },
    {ITM_MtoZHANG       },
    {ITM_TBSPUKtoML     },
    {ITM_FENtoM         },
    {ITM_MtoFEN         },
    {ITM_MI2toKM2       },
    {ITM_KM2toMI2       },
    {ITM_NMI2toKM2      },
    {ITM_KM2toNMI2      },
    {ITM_TSPCtoFZUS     },
    {ITM_TSPCtoML       },
    {ITM_TSPUKtoFZUK    },
    {ITM_TSPUKtoML      },
    {ITM_GLUStoFZUS     },
    {ITM_FZUStoGLUS     },
    {ITM_KNOTtoKMH      },
    {ITM_KMHtoKNOT      },
    {ITM_KMHtoMPS       },
    {ITM_MPStoKMH       },
    {ITM_RPMtoDEGPS     },
    {ITM_DEGPStoRPM     },
    {ITM_MPHtoKMH       },
    {ITM_KMHtoMPH       },
    {ITM_MPHtoMPS       },
    {ITM_MPStoMPH       },
    {ITM_RPMtoRADPS     },
    {ITM_RADPStoRPM     },
    {ITM_DEGtoRAD       },
    {ITM_RADtoDEG       },
    {ITM_DEGtoGRAD      },
    {ITM_GRADtoDEG      },
    {ITM_GRADtoRAD      },
    {ITM_RADtoGRAD      },
    {ITM_INCHtoCM       },
    {ITM_CMtoINCH       },
    {ITM_NMItoMI        },
    {ITM_MItoNMI        },
    {ITM_FURtoM         },
    {ITM_MtoFUR         },
    {ITM_FTNtoS         },
    {ITM_StoFTN         },
    {ITM_FPFtoMPS       },
    {ITM_MPStoFPF       },
    {ITM_BRDStoM        },
    {ITM_MtoBRDS        },
    {ITM_FIRtoKG        },
    {ITM_KGtoFIR        },
    {ITM_FPFtoKPH       },
    {ITM_KPHtoFPF       },
    {ITM_BRDStoIN       },
    {ITM_INtoBRDS       },
    {ITM_FIRtoLB        },
    {ITM_LBtoFIR        },
    {ITM_FPFtoMPH       },
    {ITM_MPHtoFPF       },
    {ITM_FPStoKMH       },
    {ITM_KMHtoFPS       },
    {ITM_FPStoMPS       },
    {ITM_MPStoFPS       },
    {ITM_L100toKML      },
    {ITM_KMLtoL100      },
    {ITM_KMLEtoK100K    },
    {ITM_K100KtoKMLE    },
    {ITM_K100KtoKMK     },
    {ITM_KMKtoK100K     },
    {ITM_L100toMGUS     },
    {ITM_MGUStoL100     },
    {ITM_MGEUStoK100M   },
    {ITM_K100MtoMGEUS   },
    {ITM_K100KtoK100M   },
    {ITM_K100MtoK100K   },
    {ITM_L100toMGUK     },
    {ITM_MGUKtoL100     },
    {ITM_MGEUKtoK100M   },
    {ITM_K100MtoMGEUK   },
    {ITM_K100MtoMIK     },
    {ITM_MIKtoK100M     },
    {ITM_EVtoJ          },
    {ITM_JtoEV          },
    {ITM_BANANAtoINCH   },
    {ITM_INCHtoBANANA   },
    {ITM_BANANAtoMM     },
    {ITM_MMtoBANANA     },
    {ITM_ERGtoJ         },
    {ITM_JtoERG         },
    {ITM_FoetoJ         },
    {ITM_JtoFoe         },
    {ITM_CtoK           },
    {ITM_KtoC           },
    {ITM_RAtoK          },
    {ITM_KtoRA          },
    {ITM_RAtoF          },
    {ITM_FtoRA          },
    {ITM_EVKBtoK        },
    {ITM_KtoEVKB        },
    {ITM_FtoK           },
    {ITM_KtoF           },
    {ITM_KNOTtoMPS      },
    {ITM_MPStoKNOT      },
    {ITM_DEGPStoRADPS   },
    {ITM_RADPStoDEGPS   },
    {ITM_SLUGtoKG       },
    {ITM_KGtoSLUG       },
    {ITM_SLINCHtoKG     },
    {ITM_KGtoSLINCH     },
    {ITM_BLOBtoKG       },
    {ITM_KGtoBLOB       },
    {ITM_TONNEtoKG      },
    {ITM_KGtoTONNE      },
    {ITM_LBSFT2toPA     },
    {ITM_PAtoLBSFT2     },
    {ITM_INLBStoNM      },
    {ITM_NMtoINLBS      },
    {ITM_LBSFTtoNPM     },
    {ITM_NPMtoLBSFT     },
    {ITM_KGFtoN         },
    {ITM_NtoKGF         },
    {ITM_MS2toFTS2      },
    {ITM_FTS2toMS2      },
    {ITM_MS2toINS2      },
    {ITM_INS2toMS2      },
    {ITM_KSItoMPA       },
    {ITM_MPAtoKSI       },
    {ITM_LBSIN3toBLOBIN3},
    {ITM_BLOBIN3toLBSIN3},
    {ITM_LBSIN3toTMM3   },
    {ITM_TMM3toLBSIN3   },
    {ITM_LBSIN3toKGM3   },
    {ITM_KGM3toLBSIN3   },
    {ITM_KGM3toBLOBIN3  },
    {ITM_BLOBIN3toKGM3  },
    {ITM_KGM3toTMM3     },
    {ITM_TMM3toKGM3     },
    {ITM_LBFTtoKGM      },
    {ITM_KGMtoLBFT      },
    {ITM_IN3toMM3       },
    {ITM_MM3toIN3       },
    {ITM_IN2toMM2       },
    {ITM_MM2toIN2       },
    {ITM_IN4toMM4       },
    {ITM_MM4toIN4       },
    {ITM_IN6toMM6       },
    {ITM_MM6toIN6       },
    {ITM_KGFPMtoNPM     },
    {ITM_NPMtoKGFPM     },
    {ITM_KGFtoLBF       },
    {ITM_LBFtoKGF       },
    {ITM_UNSLUGtoKG     },
    {ITM_KGtoUNSLUG     },
    {ITM_KGtoUNSLINCH   },
    {ITM_UNSLINCHtoKG   },
     // do mimRunFunction(item, indexOfItems[item].param);
   };



//==============================================================================
// ADD A NEW CONVERSION PAIR TO THE PAIR-LIST
//==============================================================================
// 1. Add the pair to the matching menu_Conv* array in items.h. Two adjacent ITM_ entries in a row form one pair (positions 0/1, 2/3, 4/5, ...).
//
// 2. Add TWO rows below, bidirectional
//      { ITM_AtoB , ITM_BtoA },   // NNN <-> MMM
//      { ITM_BtoA , ITM_AtoB },   // MMM <-> NNN
//
// 3. KEEP THE ARRAY SORTED ASCENDING by the FIRST item code. The binary search in isOneOfAConvertPair() silently fails on unsorted entries.
//
// 4. Update NUM_CONVERT_PAIRS by 2.
//==============================================================================

typedef enum {
  UT_NOT_CONFIGURABLE = 0,    // Legacy partner-only conversion (e.g. dB/ratio, HMS/HR), not configurable using ASN
  UT_DISTANCE,                // length / position   SI base: m     store exp relative to m (mm:-3, cm:-2, m:0, km:+3)
  UT_AREA,                    // area                SI base: m^2   m^2:0, ha:+4 reached via m^2, km^2:+6
  UT_VOLUME,                  // volume              SI base: L     L (litre), most members convert to L or mL. mL:-3, L:0, m^3:+3
  UT_MASS,                    // mass                SI base: kg    g:-3, kg:0
  UT_TIME,                    // time                SI base: s     convert to seconds (year, fortnight). HMS<->HR is UT_NOT_CONFIGURABLE
  UT_TEMPERATURE,             // temperature         SI base: K     C<->F, F<->Ra via unity to land on K
  UT_PRESSURE,                // pressure            SI base: Pa    pascal = N/m^2). land on Pa with exp 0
  UT_ENERGY,                  // energy              SI base: J     joule). BTU, cal, Wh, eV, erg, foe
  UT_POWER,                   // power               SI base: W     watt = J/s. hp variants
  UT_FORCE,                   // force               SI base: N     newton. lbf, kgf
  UT_TORQUE,                  // torque / moment     SI base: N*m   distinct from energy
  UT_SPEED,                   // linear speed        SI base: m/s   km/h, mph, fps, fpf chain to m/s
  UT_ANGLE,                   // plane angle         SI base: rad   deg, grad
  UT_ANGULAR_SPEED,           // angular velocity    SI base: rad/s rpm, deg/s
  UT_FUELECON,                // fuel economy        no SI base:    linked-function pairs only. L100/km per L/mpg US/mpg UK
  UT_EVECON,                  // EV energy economy   no SI base:    linked-function pairs only. kWh/100km, kWh/100mi, km per kWh, mi per kWh, mpge US/UK
  UT_ACCELERATION,            // acceleration        SI base:       m/s^2 ft/s^2, in/s^2
  UT_DENSITY,                 // mass density        SI base:       kg/m^3 lbs/in^3, t/mm^3, blob/in^3
  UT_LINEAR_FORCE_DENSITY     // force density       SI base:  ???
} unitType_t;

typedef struct {
  int16_t  item;                                                                 // sort key. The item number
  int16_t  partner;                                                              // paired item number
  int16_t  unity;                                                                // ITM to run to push result toward SI base; ITM_NULL = already there
  int8_t   exponent;                                                             // value_in_SI = chain_result * 10^exponent
  uint8_t  type;                                                                 // unitType_t
} convPair_t;


TO_QSPI static const convPair_t convertPairs[NUM_CONVERT_PAIRS] = {              // sorted by item
  //item                  partner                unity                  exp  type
  { ITM_CtoF             /*  220 */, ITM_FtoC             , ITM_FtoK           , +0 , UT_TEMPERATURE              },
  { ITM_FtoC             /*  221 */, ITM_CtoF             , ITM_CtoK           , +0 , UT_TEMPERATURE              },
  { ITM_DBtoPR           /*  222 */, ITM_PRtoDB           , ITM_NULL           , +0 , UT_NOT_CONFIGURABLE         },
  { ITM_FT2toHA          /*  223 */, ITM_HAtoFT2          , ITM_NULL           , +4 , UT_AREA                     },
  { ITM_HAtoFT2          /*  224 */, ITM_FT2toHA          , ITM_FT2toM2        , +0 , UT_AREA                     },
  { ITM_DBtoFR           /*  225 */, ITM_FRtoDB           , ITM_NULL           , +0 , UT_NOT_CONFIGURABLE         },
  { ITM_FT2toM2          /*  226 */, ITM_M2toFT2          , ITM_NULL           , +0 , UT_AREA                     },
  { ITM_M2toFT2          /*  227 */, ITM_FT2toM2          , ITM_FT2toM2        , +0 , UT_AREA                     },
  { ITM_PRtoDB           /*  228 */, ITM_DBtoPR           , ITM_NULL           , +0 , UT_NOT_CONFIGURABLE         },
  { ITM_HAtoKM2          /*  229 */, ITM_KM2toHA          , ITM_NULL           , +6 , UT_AREA                     },
  { ITM_KM2toHA          /*  230 */, ITM_HAtoKM2          , ITM_NULL           , +4 , UT_AREA                     },
  { ITM_FRtoDB           /*  231 */, ITM_DBtoFR           , ITM_NULL           , +0 , UT_NOT_CONFIGURABLE         },
  { ITM_GLUKtoFZUK       /*  232 */, ITM_FZUKtoGLUK       , ITM_FZUKtoML       , -3 , UT_VOLUME                   },
  { ITM_FZUKtoGLUK       /*  233 */, ITM_GLUKtoFZUK       , ITM_GLUKtoL        , +0 , UT_VOLUME                   },
  { ITM_ACtoHA           /*  234 */, ITM_HAtoAC           , ITM_NULL           , +4 , UT_AREA                     },
  { ITM_MLtoIN3          /*  235 */, ITM_IN3toML          , ITM_IN3toML        , -3 , UT_VOLUME                   },
  { ITM_HAtoAC           /*  236 */, ITM_ACtoHA           , ITM_ACtoHA         , +4 , UT_AREA                     },
  { ITM_IN3toML          /*  237 */, ITM_MLtoIN3          , ITM_NULL           , -3 , UT_VOLUME                   },
  { ITM_ACUStoHA         /*  238 */, ITM_HAtoACUS         , ITM_NULL           , +4 , UT_AREA                     },
  { ITM_FT3toGLUK        /*  239 */, ITM_GLUKtoFT3        , ITM_GLUKtoL        , +0 , UT_VOLUME                   },
  { ITM_HAtoACUS         /*  240 */, ITM_ACUStoHA         , ITM_ACUStoHA       , +4 , UT_AREA                     },
  { ITM_GLUKtoFT3        /*  241 */, ITM_FT3toGLUK        , ITM_FT3toL         , +0 , UT_VOLUME                   },
  { ITM_PAtoATM          /*  242 */, ITM_ATMtoPA          , ITM_ATMtoPA        , +0 , UT_PRESSURE                 },
  { ITM_ATMtoPA          /*  243 */, ITM_PAtoATM          , ITM_NULL           , +0 , UT_PRESSURE                 },
  { ITM_AUtoM            /*  244 */, ITM_MtoAU            , ITM_NULL           , +0 , UT_DISTANCE                 },
  { ITM_MtoAU            /*  245 */, ITM_AUtoM            , ITM_AUtoM          , +0 , UT_DISTANCE                 },
  { ITM_BARtoPA          /*  246 */, ITM_PAtoBAR          , ITM_NULL           , +0 , UT_PRESSURE                 },
  { ITM_PAtoBAR          /*  247 */, ITM_BARtoPA          , ITM_BARtoPA        , +0 , UT_PRESSURE                 },
  { ITM_BTUtoJ           /*  248 */, ITM_JtoBTU           , ITM_NULL           , +0 , UT_ENERGY                   },
  { ITM_JtoBTU           /*  249 */, ITM_BTUtoJ           , ITM_BTUtoJ         , +0 , UT_ENERGY                   },
  { ITM_CALtoJ           /*  250 */, ITM_JtoCAL           , ITM_NULL           , +0 , UT_ENERGY                   },
  { ITM_JtoCAL           /*  251 */, ITM_CALtoJ           , ITM_CALtoJ         , +0 , UT_ENERGY                   },
  { ITM_LBFFTtoNM        /*  252 */, ITM_NMtoLBFFT        , ITM_NULL           , +0 , UT_TORQUE                   },
  { ITM_LtoFT3           /*  253 */, ITM_FT3toL           , ITM_FT3toL         , +0 , UT_VOLUME                   },
  { ITM_NMtoLBFFT        /*  254 */, ITM_LBFFTtoNM        , ITM_LBFFTtoNM      , +0 , UT_TORQUE                   },
  { ITM_FT3toL           /*  255 */, ITM_LtoFT3           , ITM_NULL           , +0 , UT_VOLUME                   },
  { ITM_CWTtoKG          /*  256 */, ITM_KGtoCWT          , ITM_NULL           , +0 , UT_MASS                     },
  { ITM_KGtoCWT          /*  257 */, ITM_CWTtoKG          , ITM_CWTtoKG        , +0 , UT_MASS                     },
  { ITM_FTtoM            /*  258 */, ITM_MtoFT            , ITM_NULL           , +0 , UT_DISTANCE                 },
  { ITM_MtoFT            /*  259 */, ITM_FTtoM            , ITM_FTtoM          , +0 , UT_DISTANCE                 },
  { ITM_FTUStoM          /*  260 */, ITM_MtoFTUS          , ITM_NULL           , +0 , UT_DISTANCE                 },
  { ITM_LtoQTUS          /*  261 */, ITM_QTUStoL          , ITM_QTUStoL        , +0 , UT_VOLUME                   },
  { ITM_QTUStoL          /*  262 */, ITM_LtoQTUS          , ITM_NULL           , +0 , UT_VOLUME                   },
  { ITM_MtoFTUS          /*  263 */, ITM_FTUStoM          , ITM_FTUStoM        , +0 , UT_DISTANCE                 },
  { ITM_IN3toFZUK        /*  264 */, ITM_FZUKtoIN3        , ITM_FZUKtoML       , -3 , UT_VOLUME                   },
  { ITM_FZUKtoIN3        /*  265 */, ITM_IN3toFZUK        , ITM_IN3toML        , -3 , UT_VOLUME                   },
  { ITM_FZUKtoML         /*  266 */, ITM_MLtoFZUK         , ITM_NULL           , -3 , UT_VOLUME                   },
  { ITM_IN3toFZUS        /*  267 */, ITM_FZUStoIN3        , ITM_FZUStoML       , -3 , UT_VOLUME                   },
  { ITM_MLtoFZUK         /*  268 */, ITM_FZUKtoML         , ITM_FZUKtoML       , -3 , UT_VOLUME                   },
  { ITM_FZUStoIN3        /*  269 */, ITM_IN3toFZUS        , ITM_IN3toML        , -3 , UT_VOLUME                   },
  { ITM_FZUStoML         /*  270 */, ITM_MLtoFZUS         , ITM_NULL           , -3 , UT_VOLUME                   },
  { ITM_FT3toGalUS       /*  271 */, ITM_GalUStoFT3       , ITM_GLUStoL        , +0 , UT_VOLUME                   },
  { ITM_MLtoFZUS         /*  272 */, ITM_FZUStoML         , ITM_FZUStoML       , -3 , UT_VOLUME                   },
  { ITM_GalUStoFT3       /*  273 */, ITM_FT3toGalUS       , ITM_FT3toL         , +0 , UT_VOLUME                   },
  { ITM_GLUKtoL          /*  274 */, ITM_LtoGLUK          , ITM_NULL           , +0 , UT_VOLUME                   },
  { ITM_LtoGLUK          /*  275 */, ITM_GLUKtoL          , ITM_GLUKtoL        , +0 , UT_VOLUME                   },
  { ITM_GLUStoL          /*  276 */, ITM_LtoGLUS          , ITM_NULL           , +0 , UT_VOLUME                   },
  { ITM_LtoGLUS          /*  277 */, ITM_GLUStoL          , ITM_GLUStoL        , +0 , UT_VOLUME                   },
  { ITM_HPEtoW           /*  278 */, ITM_WtoHPE           , ITM_NULL           , +0 , UT_POWER                    },
  { ITM_WtoHPE           /*  279 */, ITM_HPEtoW           , ITM_HPEtoW         , +0 , UT_POWER                    },
  { ITM_HPMtoW           /*  280 */, ITM_WtoHPM           , ITM_NULL           , +0 , UT_POWER                    },
  { ITM_WtoHPM           /*  281 */, ITM_HPMtoW           , ITM_HPMtoW         , +0 , UT_POWER                    },
  { ITM_HPUKtoW          /*  282 */, ITM_WtoHPUK          , ITM_NULL           , +0 , UT_POWER                    },
  { ITM_WtoHPUK          /*  283 */, ITM_HPUKtoW          , ITM_HPUKtoW        , +0 , UT_POWER                    },
  { ITM_INCHHGtoPA       /*  284 */, ITM_PAtoINCHHG       , ITM_NULL           , +0 , UT_PRESSURE                 },
  { ITM_CUPCtoFZUS       /*  285 */, ITM_FZUStoCUPC       , ITM_FZUStoML       , -3 , UT_VOLUME                   },
  { ITM_PAtoINCHHG       /*  286 */, ITM_INCHHGtoPA       , ITM_INCHHGtoPA     , +0 , UT_PRESSURE                 },
  { ITM_CUPCtoML         /*  287 */, ITM_MLtoCUPC         , ITM_NULL           , -3 , UT_VOLUME                   },
  { ITM_INCHtoMM         /*  288 */, ITM_MMtoINCH         , ITM_NULL           , -3 , UT_DISTANCE                 },
  { ITM_MMtoINCH         /*  289 */, ITM_INCHtoMM         , ITM_INCHtoCM       , -2 , UT_DISTANCE                 },
  { ITM_WHtoJ            /*  290 */, ITM_JtoWH            , ITM_NULL           , +0 , UT_ENERGY                   },
  { ITM_JtoWH            /*  291 */, ITM_WHtoJ            , ITM_WHtoJ          , +0 , UT_ENERGY                   },
  { ITM_KGtoLBS          /*  292 */, ITM_LBStoKG          , ITM_LBStoKG        , +0 , UT_MASS                     },
  { ITM_LBStoKG          /*  293 */, ITM_KGtoLBS          , ITM_NULL           , +0 , UT_MASS                     },
  { ITM_GtoOZ            /*  294 */, ITM_OZtoG            , ITM_OZtoG          , -3 , UT_MASS                     },
  { ITM_OZtoG            /*  295 */, ITM_GtoOZ            , ITM_NULL           , -3 , UT_MASS                     },
  { ITM_KGtoSCW          /*  296 */, ITM_SCWtoKG          , ITM_SCWtoKG        , +0 , UT_MASS                     },
  { ITM_CUPUKtoFZUK      /*  297 */, ITM_FZUKtoCUPUK      , ITM_FZUKtoML       , -3 , UT_VOLUME                   },
  { ITM_SCWtoKG          /*  298 */, ITM_KGtoSCW          , ITM_NULL           , +0 , UT_MASS                     },
  { ITM_CUPUKtoML        /*  299 */, ITM_MLtoCUPUK        , ITM_NULL           , -3 , UT_VOLUME                   },
  { ITM_KGtoSTO          /*  300 */, ITM_STOtoKG          , ITM_STOtoKG        , +0 , UT_MASS                     },
  { ITM_FZUKtoCUPUK      /*  301 */, ITM_CUPUKtoFZUK      , ITM_CUPUKtoML      , -3 , UT_VOLUME                   },
  { ITM_STOtoKG          /*  302 */, ITM_KGtoSTO          , ITM_NULL           , +0 , UT_MASS                     },
  { ITM_FZUKtoTBSPUK     /*  303 */, ITM_TBSPUKtoFZUK     , ITM_TBSPUKtoML     , -3 , UT_VOLUME                   },
  { ITM_KGtoST           /*  304 */, ITM_STtoKG           , ITM_STtoKG         , +0 , UT_MASS                     },
  { ITM_FZUKtoTSPUK      /*  305 */, ITM_TSPUKtoFZUK      , ITM_TSPUKtoML      , -3 , UT_VOLUME                   },
  { ITM_FZUStoCUPC       /*  306 */, ITM_CUPCtoFZUS       , ITM_CUPCtoML       , -3 , UT_VOLUME                   },
  { ITM_STtoKG           /*  307 */, ITM_KGtoST           , ITM_NULL           , +0 , UT_MASS                     },
  { ITM_FZUStoTBSPC      /*  308 */, ITM_TBSPCtoFZUS      , ITM_TBSPCtoML      , -3 , UT_VOLUME                   },
  { ITM_FZUStoTSPC       /*  309 */, ITM_TSPCtoFZUS       , ITM_TSPCtoML       , -3 , UT_VOLUME                   },
  { ITM_KGtoLT           /*  310 */, ITM_LTtoKG           , ITM_LTtoKG         , +0 , UT_MASS                     },
  { ITM_KGtoLIANG        /*  311 */, ITM_LIANGtoKG        , ITM_LIANGtoKG      , +0 , UT_MASS                     },
  { ITM_MLtoCUPC         /*  312 */, ITM_CUPCtoML         , ITM_CUPCtoML       , -3 , UT_VOLUME                   },
  { ITM_LTtoKG           /*  313 */, ITM_KGtoLT           , ITM_NULL           , +0 , UT_MASS                     },
  { ITM_LIANGtoKG        /*  314 */, ITM_KGtoLIANG        , ITM_NULL           , +0 , UT_MASS                     },
  { ITM_MLtoCUPUK        /*  315 */, ITM_CUPUKtoML        , ITM_CUPUKtoML      , -3 , UT_VOLUME                   },
  { ITM_GtoTRZ           /*  316 */, ITM_TRZtoG           , ITM_TRZtoG         , -3 , UT_MASS                     },
  { ITM_MLtoPINTLQ       /*  317 */, ITM_PINTLQtoML       , ITM_PINTLQtoML     , -3 , UT_VOLUME                   },
  { ITM_TRZtoG           /*  318 */, ITM_GtoTRZ           , ITM_NULL           , -3 , UT_MASS                     },
  { ITM_MLtoPINTUK       /*  319 */, ITM_PINTUKtoML       , ITM_PINTUKtoML     , -3 , UT_VOLUME                   },
  { ITM_LBFtoN           /*  320 */, ITM_NtoLBF           , ITM_NULL           , +0 , UT_FORCE                    },
  { ITM_NtoLBF           /*  321 */, ITM_LBFtoN           , ITM_LBFtoN         , +0 , UT_FORCE                    },
  { ITM_LYtoM            /*  322 */, ITM_MtoLY            , ITM_NULL           , +0 , UT_DISTANCE                 },
  { ITM_MtoLY            /*  323 */, ITM_LYtoM            , ITM_LYtoM          , +0 , UT_DISTANCE                 },
  { ITM_MMHGtoPA         /*  324 */, ITM_PAtoMMHG         , ITM_NULL           , +0 , UT_PRESSURE                 },
  { ITM_MLtoQT           /*  325 */, ITM_QTtoML           , ITM_QTtoL          , +0 , UT_VOLUME                   },
  { ITM_PAtoMMHG         /*  326 */, ITM_MMHGtoPA         , ITM_MMHGtoPA       , +0 , UT_PRESSURE                 },
  { ITM_MLtoQTUS         /*  327 */, ITM_QTUStoML         , ITM_QTUStoL        , +0 , UT_VOLUME                   },
  { ITM_MItoKM           /*  328 */, ITM_KMtoMI           , ITM_NULL           , +3 , UT_DISTANCE                 },
  { ITM_KMtoMI           /*  329 */, ITM_MItoKM           , ITM_MILEtoM        , +0 , UT_DISTANCE                 },
  { ITM_KMtoNMI          /*  330 */, ITM_NMItoKM          , ITM_NMItoM         , +0 , UT_DISTANCE                 },
  { ITM_NMItoKM          /*  331 */, ITM_KMtoNMI          , ITM_NULL           , +3 , UT_DISTANCE                 },
  { ITM_MtoPC            /*  332 */, ITM_PCtoM            , ITM_PCtoM          , +0 , UT_DISTANCE                 },
  { ITM_PCtoM            /*  333 */, ITM_MtoPC            , ITM_NULL           , +0 , UT_DISTANCE                 },
  { ITM_MMtoPOINT        /*  334 */, ITM_POINTtoMM        , ITM_POINTtoMM      , -3 , UT_DISTANCE                 },
  { ITM_MLtoTBSPC        /*  335 */, ITM_TBSPCtoML        , ITM_TBSPCtoML      , -3 , UT_VOLUME                   },
  { ITM_MILEtoM          /*  336 */, ITM_MtoMILE          , ITM_NULL           , +0 , UT_DISTANCE                 },
  { ITM_POINTtoMM        /*  337 */, ITM_MMtoPOINT        , ITM_NULL           , -3 , UT_DISTANCE                 },
  { ITM_MLtoTBSPUK       /*  338 */, ITM_TBSPUKtoML       , ITM_TBSPUKtoML     , -3 , UT_VOLUME                   },
  { ITM_MtoMILE          /*  339 */, ITM_MILEtoM          , ITM_MILEtoM        , +0 , UT_DISTANCE                 },
  { ITM_MtoYD            /*  340 */, ITM_YDtoM            , ITM_YDtoM          , +0 , UT_DISTANCE                 },
  { ITM_YDtoM            /*  341 */, ITM_MtoYD            , ITM_NULL           , +0 , UT_DISTANCE                 },
  { ITM_PSItoPA          /*  342 */, ITM_PAtoPSI          , ITM_NULL           , +0 , UT_PRESSURE                 },
  { ITM_PAtoPSI          /*  343 */, ITM_PSItoPA          , ITM_PSItoPA        , +0 , UT_PRESSURE                 },
  { ITM_PAtoTOR          /*  344 */, ITM_TORtoPA          , ITM_TORtoPA        , +0 , UT_PRESSURE                 },
  { ITM_MLtoTSPC         /*  345 */, ITM_TSPCtoML         , ITM_TSPCtoML       , -3 , UT_VOLUME                   },
  { ITM_TORtoPA          /*  346 */, ITM_PAtoTOR          , ITM_NULL           , +0 , UT_PRESSURE                 },
  { ITM_MLtoTSPUK        /*  347 */, ITM_TSPUKtoML        , ITM_TSPUKtoML      , -3 , UT_VOLUME                   },
  { ITM_StoYEAR          /*  348 */, ITM_YEARtoS          , ITM_YEARtoS        , +0 , UT_TIME                     },
  { ITM_YEARtoS          /*  349 */, ITM_StoYEAR          , ITM_NULL           , +0 , UT_TIME                     },
  { ITM_CARATtoG         /*  350 */, ITM_GtoCARAT         , ITM_NULL           , -3 , UT_MASS                     },
  { ITM_PINTLQtoML       /*  351 */, ITM_MLtoPINTLQ       , ITM_NULL           , -3 , UT_VOLUME                   },
  { ITM_JINtoKG          /*  352 */, ITM_KGtoJIN          , ITM_NULL           , +0 , UT_MASS                     },
  { ITM_GtoCARAT         /*  353 */, ITM_CARATtoG         , ITM_CARATtoG       , -3 , UT_MASS                     },
  { ITM_PINTUKtoML       /*  354 */, ITM_MLtoPINTUK       , ITM_NULL           , -3 , UT_VOLUME                   },
  { ITM_KGtoJIN          /*  355 */, ITM_JINtoKG          , ITM_JINtoKG        , +0 , UT_MASS                     },
  { ITM_QTtoL            /*  356 */, ITM_LtoQT            , ITM_NULL           , +0 , UT_VOLUME                   },
  { ITM_LtoQT            /*  357 */, ITM_QTtoL            , ITM_QTtoL          , +0 , UT_VOLUME                   },
  { ITM_FATHOMtoM        /*  358 */, ITM_MtoFATHOM        , ITM_NULL           , +0 , UT_DISTANCE                 },
  { ITM_QTtoML           /*  359 */, ITM_MLtoQT           , ITM_NULL           , -3 , UT_VOLUME                   },
  { ITM_NMItoM           /*  360 */, ITM_MtoNMI           , ITM_NULL           , +0 , UT_DISTANCE                 },
  { ITM_MtoFATHOM        /*  361 */, ITM_FATHOMtoM        , ITM_FATHOMtoM      , +0 , UT_DISTANCE                 },
  { ITM_QTUStoML         /*  362 */, ITM_MLtoQTUS         , ITM_NULL           , -3 , UT_VOLUME                   },
  { ITM_MtoNMI           /*  363 */, ITM_NMItoM           , ITM_NMItoM         , +0 , UT_DISTANCE                 },
  { ITM_BARRELtoM3       /*  364 */, ITM_M3toBARREL       , ITM_NULL           , +3 , UT_VOLUME                   },
  { ITM_TBSPCtoFZUS      /*  365 */, ITM_FZUStoTBSPC      , ITM_FZUStoML       , -3 , UT_VOLUME                   },
  { ITM_M3toBARREL       /*  366 */, ITM_BARRELtoM3       , ITM_BARRELtoM3     , +3 , UT_VOLUME                   },
  { ITM_HMStoHR          /*  367 */, ITM_HRtoHMS          , ITM_NULL           , +0 , UT_NOT_CONFIGURABLE         },
  { ITM_HRtoHMS          /*  368 */, ITM_HMStoHR          , ITM_NULL           , +0 , UT_NOT_CONFIGURABLE         },
  { ITM_TBSPCtoML        /*  369 */, ITM_MLtoTBSPC        , ITM_NULL           , -3 , UT_VOLUME                   },
  { ITM_HECTAREtoM2      /*  370 */, ITM_M2toHECTARE      , ITM_NULL           , +0 , UT_AREA                     },
  { ITM_M2toHECTARE      /*  371 */, ITM_HECTAREtoM2      , ITM_NULL           , +4 , UT_AREA                     },
  { ITM_MUtoM2           /*  372 */, ITM_M2toMU           , ITM_NULL           , +0 , UT_AREA                     },
  { ITM_M2toMU           /*  373 */, ITM_MUtoM2           , ITM_MUtoM2         , +0 , UT_AREA                     },
  { ITM_LItoM            /*  374 */, ITM_MtoLI            , ITM_NULL           , +0 , UT_DISTANCE                 },
  { ITM_MtoLI            /*  375 */, ITM_LItoM            , ITM_LItoM          , +0 , UT_DISTANCE                 },
  { ITM_CHItoM           /*  376 */, ITM_MtoCHI           , ITM_NULL           , +0 , UT_DISTANCE                 },
  { ITM_MtoCHI           /*  377 */, ITM_CHItoM           , ITM_CHItoM         , +0 , UT_DISTANCE                 },
  { ITM_YINtoM           /*  378 */, ITM_MtoYIN           , ITM_NULL           , +0 , UT_DISTANCE                 },
  { ITM_MtoYIN           /*  379 */, ITM_YINtoM           , ITM_YINtoM         , +0 , UT_DISTANCE                 },
  { ITM_CUNtoM           /*  380 */, ITM_MtoCUN           , ITM_NULL           , +0 , UT_DISTANCE                 },
  { ITM_MtoCUN           /*  381 */, ITM_CUNtoM           , ITM_CUNtoM         , +0 , UT_DISTANCE                 },
  { ITM_ZHANGtoM         /*  382 */, ITM_MtoZHANG         , ITM_NULL           , +0 , UT_DISTANCE                 },
  { ITM_TBSPUKtoFZUK     /*  383 */, ITM_FZUKtoTBSPUK     , ITM_FZUKtoML       , -3 , UT_VOLUME                   },
  { ITM_MtoZHANG         /*  384 */, ITM_ZHANGtoM         , ITM_ZHANGtoM       , +0 , UT_DISTANCE                 },
  { ITM_TBSPUKtoML       /*  385 */, ITM_MLtoTBSPUK       , ITM_NULL           , -3 , UT_VOLUME                   },
  { ITM_FENtoM           /*  386 */, ITM_MtoFEN           , ITM_NULL           , +0 , UT_DISTANCE                 },
  { ITM_MtoFEN           /*  387 */, ITM_FENtoM           , ITM_FENtoM         , +0 , UT_DISTANCE                 },
  { ITM_MI2toKM2         /*  388 */, ITM_KM2toMI2         , ITM_NULL           , +6 , UT_AREA                     },
  { ITM_KM2toMI2         /*  389 */, ITM_MI2toKM2         , ITM_MI2toKM2       , +6 , UT_AREA                     },
  { ITM_NMI2toKM2        /*  390 */, ITM_KM2toNMI2        , ITM_NULL           , +6 , UT_AREA                     },
  { ITM_KM2toNMI2        /*  391 */, ITM_NMI2toKM2        , ITM_NMI2toKM2      , +6 , UT_AREA                     },
  { ITM_TSPCtoFZUS       /*  392 */, ITM_FZUStoTSPC       , ITM_FZUStoML       , -3 , UT_VOLUME                   },
  { ITM_TSPCtoML         /*  393 */, ITM_MLtoTSPC         , ITM_NULL           , -3 , UT_VOLUME                   },
  { ITM_TSPUKtoFZUK      /*  394 */, ITM_FZUKtoTSPUK      , ITM_FZUKtoML       , -3 , UT_VOLUME                   },
  { ITM_TSPUKtoML        /*  395 */, ITM_MLtoTSPUK        , ITM_NULL           , -3 , UT_VOLUME                   },
  { ITM_GLUStoFZUS       /* 1902 */, ITM_FZUStoGLUS       , ITM_FZUStoML       , -3 , UT_VOLUME                   },
  { ITM_FZUStoGLUS       /* 1903 */, ITM_GLUStoFZUS       , ITM_GLUStoL        , +0 , UT_VOLUME                   },
  { ITM_KNOTtoKMH        /* 2084 */, ITM_KMHtoKNOT        , ITM_KMHtoMPS       , +0 , UT_SPEED                    },
  { ITM_KMHtoKNOT        /* 2085 */, ITM_KNOTtoKMH        , ITM_KNOTtoMPS      , +0 , UT_SPEED                    },
  { ITM_KMHtoMPS         /* 2086 */, ITM_MPStoKMH         , ITM_NULL           , +0 , UT_SPEED                    },
  { ITM_MPStoKMH         /* 2087 */, ITM_KMHtoMPS         , ITM_KMHtoMPS       , +0 , UT_SPEED                    },
  { ITM_RPMtoDEGPS       /* 2088 */, ITM_DEGPStoRPM       , ITM_DEGPStoRADPS   , +0 , UT_ANGULAR_SPEED            },
  { ITM_DEGPStoRPM       /* 2089 */, ITM_RPMtoDEGPS       , ITM_RPMtoRADPS     , +0 , UT_ANGULAR_SPEED            },
  { ITM_MPHtoKMH         /* 2090 */, ITM_KMHtoMPH         , ITM_KMHtoMPS       , +0 , UT_SPEED                    },
  { ITM_KMHtoMPH         /* 2091 */, ITM_MPHtoKMH         , ITM_MPHtoMPS       , +0 , UT_SPEED                    },
  { ITM_MPHtoMPS         /* 2092 */, ITM_MPStoMPH         , ITM_NULL           , +0 , UT_SPEED                    },
  { ITM_MPStoMPH         /* 2093 */, ITM_MPHtoMPS         , ITM_MPHtoMPS       , +0 , UT_SPEED                    },
  { ITM_RPMtoRADPS       /* 2094 */, ITM_RADPStoRPM       , ITM_NULL           , +0 , UT_ANGULAR_SPEED            },
  { ITM_RADPStoRPM       /* 2095 */, ITM_RPMtoRADPS       , ITM_RPMtoRADPS     , +0 , UT_ANGULAR_SPEED            },
  { ITM_DEGtoRAD         /* 2096 */, ITM_RADtoDEG         , ITM_NULL           , +0 , UT_ANGLE                    },
  { ITM_RADtoDEG         /* 2097 */, ITM_DEGtoRAD         , ITM_DEGtoRAD       , +0 , UT_ANGLE                    },
  { ITM_DEGtoGRAD        /* 2098 */, ITM_GRADtoDEG        , ITM_GRADtoRAD      , +0 , UT_ANGLE                    },
  { ITM_GRADtoDEG        /* 2099 */, ITM_DEGtoGRAD        , ITM_DEGtoRAD       , +0 , UT_ANGLE                    },
  { ITM_GRADtoRAD        /* 2100 */, ITM_RADtoGRAD        , ITM_NULL           , +0 , UT_ANGLE                    },
  { ITM_RADtoGRAD        /* 2101 */, ITM_GRADtoRAD        , ITM_GRADtoRAD      , +0 , UT_ANGLE                    },
  { ITM_INCHtoCM         /* 2163 */, ITM_CMtoINCH         , ITM_NULL           , -2 , UT_DISTANCE                 },
  { ITM_CMtoINCH         /* 2164 */, ITM_INCHtoCM         , ITM_INCHtoCM       , -2 , UT_DISTANCE                 },
  { ITM_NMItoMI          /* 2167 */, ITM_MItoNMI          , ITM_MILEtoM        , +0 , UT_DISTANCE                 },
  { ITM_MItoNMI          /* 2168 */, ITM_NMItoMI          , ITM_NMItoM         , +0 , UT_DISTANCE                 },
  { ITM_FURtoM           /* 2169 */, ITM_MtoFUR           , ITM_NULL           , +0 , UT_DISTANCE                 },
  { ITM_MtoFUR           /* 2170 */, ITM_FURtoM           , ITM_FURtoM         , +0 , UT_DISTANCE                 },
  { ITM_FTNtoS           /* 2171 */, ITM_StoFTN           , ITM_NULL           , +0 , UT_TIME                     },
  { ITM_StoFTN           /* 2172 */, ITM_FTNtoS           , ITM_FTNtoS         , +0 , UT_TIME                     },
  { ITM_FPFtoMPS         /* 2173 */, ITM_MPStoFPF         , ITM_NULL           , +0 , UT_SPEED                    },
  { ITM_MPStoFPF         /* 2174 */, ITM_FPFtoMPS         , ITM_FPFtoMPS       , +0 , UT_SPEED                    },
  { ITM_BRDStoM          /* 2175 */, ITM_MtoBRDS          , ITM_NULL           , +0 , UT_DISTANCE                 },
  { ITM_MtoBRDS          /* 2176 */, ITM_BRDStoM          , ITM_BRDStoM        , +0 , UT_DISTANCE                 },
  { ITM_FIRtoKG          /* 2177 */, ITM_KGtoFIR          , ITM_NULL           , +0 , UT_MASS                     },
  { ITM_KGtoFIR          /* 2178 */, ITM_FIRtoKG          , ITM_FIRtoKG        , +0 , UT_MASS                     },
  { ITM_FPFtoKPH         /* 2179 */, ITM_KPHtoFPF         , ITM_KMHtoMPS       , +0 , UT_SPEED                    },
  { ITM_KPHtoFPF         /* 2180 */, ITM_FPFtoKPH         , ITM_FPFtoMPS       , +0 , UT_SPEED                    },
  { ITM_BRDStoIN         /* 2181 */, ITM_INtoBRDS         , ITM_INCHtoCM       , -2 , UT_DISTANCE                 },
  { ITM_INtoBRDS         /* 2182 */, ITM_BRDStoIN         , ITM_BRDStoM        , +0 , UT_DISTANCE                 },
  { ITM_FIRtoLB          /* 2183 */, ITM_LBtoFIR          , ITM_LBStoKG        , +0 , UT_MASS                     },
  { ITM_LBtoFIR          /* 2184 */, ITM_FIRtoLB          , ITM_FIRtoKG        , +0 , UT_MASS                     },
  { ITM_FPFtoMPH         /* 2185 */, ITM_MPHtoFPF         , ITM_MPHtoMPS       , +0 , UT_SPEED                    },
  { ITM_MPHtoFPF         /* 2186 */, ITM_FPFtoMPH         , ITM_FPFtoMPS       , +0 , UT_SPEED                    },
  { ITM_FPStoKMH         /* 2187 */, ITM_KMHtoFPS         , ITM_KMHtoMPS       , +0 , UT_SPEED                    },
  { ITM_KMHtoFPS         /* 2188 */, ITM_FPStoKMH         , ITM_FPStoMPS       , +0 , UT_SPEED                    },
  { ITM_FPStoMPS         /* 2189 */, ITM_MPStoFPS         , ITM_NULL           , +0 , UT_SPEED                    },
  { ITM_MPStoFPS         /* 2190 */, ITM_FPStoMPS         , ITM_FPStoMPS       , +0 , UT_SPEED                    },
  { ITM_L100toKML        /* 2204 */, ITM_KMLtoL100        , ITM_NULL           , +0 , UT_FUELECON                 },
  { ITM_KMLtoL100        /* 2205 */, ITM_L100toKML        , ITM_NULL           , +0 , UT_FUELECON                 },
  { ITM_KMLEtoK100K      /* 2206 */, ITM_K100KtoKMLE      , ITM_NULL           , +0 , UT_EVECON                   },
  { ITM_K100KtoKMLE      /* 2207 */, ITM_KMLEtoK100K      , ITM_NULL           , +0 , UT_EVECON                   },
  { ITM_K100KtoKMK       /* 2208 */, ITM_KMKtoK100K       , ITM_NULL           , +0 , UT_EVECON                   },
  { ITM_KMKtoK100K       /* 2209 */, ITM_K100KtoKMK       , ITM_NULL           , +0 , UT_EVECON                   },
  { ITM_L100toMGUS       /* 2210 */, ITM_MGUStoL100       , ITM_NULL           , +0 , UT_FUELECON                 },
  { ITM_MGUStoL100       /* 2211 */, ITM_L100toMGUS       , ITM_NULL           , +0 , UT_FUELECON                 },
  { ITM_MGEUStoK100M     /* 2212 */, ITM_K100MtoMGEUS     , ITM_NULL           , +0 , UT_EVECON                   },
  { ITM_K100MtoMGEUS     /* 2213 */, ITM_MGEUStoK100M     , ITM_NULL           , +0 , UT_EVECON                   },
  { ITM_K100KtoK100M     /* 2214 */, ITM_K100MtoK100K     , ITM_NULL           , +0 , UT_EVECON                   },
  { ITM_K100MtoK100K     /* 2215 */, ITM_K100KtoK100M     , ITM_NULL           , +0 , UT_EVECON                   },
  { ITM_L100toMGUK       /* 2216 */, ITM_MGUKtoL100       , ITM_NULL           , +0 , UT_FUELECON                 },
  { ITM_MGUKtoL100       /* 2217 */, ITM_L100toMGUK       , ITM_NULL           , +0 , UT_FUELECON                 },
  { ITM_MGEUKtoK100M     /* 2218 */, ITM_K100MtoMGEUK     , ITM_NULL           , +0 , UT_EVECON                   },
  { ITM_K100MtoMGEUK     /* 2219 */, ITM_MGEUKtoK100M     , ITM_NULL           , +0 , UT_EVECON                   },
  { ITM_K100MtoMIK       /* 2220 */, ITM_MIKtoK100M       , ITM_NULL           , +0 , UT_EVECON                   },
  { ITM_MIKtoK100M       /* 2221 */, ITM_K100MtoMIK       , ITM_NULL           , +0 , UT_EVECON                   },
  { ITM_EVtoJ            /* 2464 */, ITM_JtoEV            , ITM_NULL           , +0 , UT_ENERGY                   },
  { ITM_JtoEV            /* 2465 */, ITM_EVtoJ            , ITM_EVtoJ          , +0 , UT_ENERGY                   },
  { ITM_BANANAtoINCH     /* 2466 */, ITM_INCHtoBANANA     , ITM_INCHtoCM       , -2 , UT_DISTANCE                 },
  { ITM_INCHtoBANANA     /* 2467 */, ITM_BANANAtoINCH     , ITM_BANANAtoMM     , -3 , UT_DISTANCE                 },
  { ITM_BANANAtoMM       /* 2468 */, ITM_MMtoBANANA       , ITM_NULL           , -3 , UT_DISTANCE                 },
  { ITM_MMtoBANANA       /* 2469 */, ITM_BANANAtoMM       , ITM_BANANAtoMM     , -3 , UT_DISTANCE                 },
  { ITM_ERGtoJ           /* 2658 */, ITM_JtoERG           , ITM_NULL           , +0 , UT_ENERGY                   },
  { ITM_JtoERG           /* 2659 */, ITM_ERGtoJ           , ITM_ERGtoJ         , +0 , UT_ENERGY                   },
  { ITM_FoetoJ           /* 2660 */, ITM_JtoFoe           , ITM_NULL           , +0 , UT_ENERGY                   },
  { ITM_JtoFoe           /* 2661 */, ITM_FoetoJ           , ITM_FoetoJ         , +0 , UT_ENERGY                   },
  { ITM_CtoK             /* 2665 */, ITM_KtoC             , ITM_NULL           , +0 , UT_TEMPERATURE              },
  { ITM_KtoC             /* 2666 */, ITM_CtoK             , ITM_CtoK           , +0 , UT_TEMPERATURE              },
  { ITM_RAtoK            /* 2667 */, ITM_KtoRA            , ITM_NULL           , +0 , UT_TEMPERATURE              },
  { ITM_KtoRA            /* 2668 */, ITM_RAtoK            , ITM_RAtoK          , +0 , UT_TEMPERATURE              },
  { ITM_RAtoF            /* 2669 */, ITM_FtoRA            , ITM_FtoK           , +0 , UT_TEMPERATURE              },
  { ITM_FtoRA            /* 2670 */, ITM_RAtoF            , ITM_RAtoK          , +0 , UT_TEMPERATURE              },
  { ITM_EVKBtoK          /* 2671 */, ITM_KtoEVKB          , ITM_NULL           , +0 , UT_TEMPERATURE              },
  { ITM_KtoEVKB          /* 2672 */, ITM_EVKBtoK          , ITM_EVKBtoK        , +0 , UT_TEMPERATURE              },
  { ITM_FtoK             /* 2673 */, ITM_KtoF             , ITM_NULL           , +0 , UT_TEMPERATURE              },
  { ITM_KtoF             /* 2674 */, ITM_FtoK             , ITM_FtoK           , +0 , UT_TEMPERATURE              },
  { ITM_KNOTtoMPS        /* 2743 */, ITM_MPStoKNOT        , ITM_NULL           , +0 , UT_SPEED                    },
  { ITM_MPStoKNOT        /* 2744 */, ITM_KNOTtoMPS        , ITM_KNOTtoMPS      , +0 , UT_SPEED                    },
  { ITM_DEGPStoRADPS     /* 2745 */, ITM_RADPStoDEGPS     , ITM_NULL           , +0 , UT_ANGULAR_SPEED            },
  { ITM_RADPStoDEGPS     /* 2746 */, ITM_DEGPStoRADPS     , ITM_DEGPStoRADPS   , +0 , UT_ANGULAR_SPEED            },
  { ITM_SLUGtoKG         /* 2747 */, ITM_KGtoSLUG         , ITM_NULL           , +0 , UT_MASS                     },
  { ITM_KGtoSLUG         /* 2748 */, ITM_SLUGtoKG         , ITM_SLUGtoKG       , +0 , UT_MASS                     },
  { ITM_SLINCHtoKG       /* 2749 */, ITM_KGtoSLINCH       , ITM_NULL           , +0 , UT_MASS                     },
  { ITM_KGtoSLINCH       /* 2750 */, ITM_SLINCHtoKG       , ITM_SLINCHtoKG     , +0 , UT_MASS                     },
  { ITM_BLOBtoKG         /* 2751 */, ITM_KGtoBLOB         , ITM_NULL           , +0 , UT_MASS                     },
  { ITM_KGtoBLOB         /* 2752 */, ITM_BLOBtoKG         , ITM_BLOBtoKG       , +0 , UT_MASS                     },
  { ITM_TONNEtoKG        /* 2753 */, ITM_KGtoTONNE        , ITM_NULL           , +0 , UT_MASS                     },
  { ITM_KGtoTONNE        /* 2754 */, ITM_TONNEtoKG        , ITM_TONNEtoKG      , +0 , UT_MASS                     },
  { ITM_LBSFT2toPA       /* 2800 */, ITM_PAtoLBSFT2       , ITM_NULL           , +0 , UT_PRESSURE                 },
  { ITM_PAtoLBSFT2       /* 2801 */, ITM_LBSFT2toPA       , ITM_LBSFT2toPA     , +0 , UT_PRESSURE                 },
  { ITM_INLBStoNM        /* 2802 */, ITM_NMtoINLBS        , ITM_NULL           , +0 , UT_TORQUE                   },
  { ITM_NMtoINLBS        /* 2803 */, ITM_INLBStoNM        , ITM_INLBStoNM      , +0 , UT_TORQUE                   },
  { ITM_LBSFTtoNPM       /* 2804 */, ITM_NPMtoLBSFT       , ITM_NULL           , +0 , UT_LINEAR_FORCE_DENSITY     },
  { ITM_NPMtoLBSFT       /* 2805 */, ITM_LBSFTtoNPM       , ITM_LBSFTtoNPM     , +0 , UT_LINEAR_FORCE_DENSITY     },
  { ITM_KGFtoN           /* 2806 */, ITM_NtoKGF           , ITM_NULL           , +0 , UT_FORCE                    },
  { ITM_NtoKGF           /* 2807 */, ITM_KGFtoN           , ITM_KGFtoN         , +0 , UT_FORCE                    },
  { ITM_MS2toFTS2        /* 2808 */, ITM_FTS2toMS2        , ITM_FTS2toMS2      , +0 , UT_ACCELERATION             },
  { ITM_FTS2toMS2        /* 2809 */, ITM_MS2toFTS2        , ITM_NULL           , +0 , UT_ACCELERATION             },
  { ITM_MS2toINS2        /* 2810 */, ITM_INS2toMS2        , ITM_INS2toMS2      , +0 , UT_ACCELERATION             },
  { ITM_INS2toMS2        /* 2811 */, ITM_MS2toINS2        , ITM_NULL           , +0 , UT_ACCELERATION             },
  { ITM_KSItoMPA         /* 2812 */, ITM_MPAtoKSI         , ITM_NULL           , +6 , UT_PRESSURE                 },
  { ITM_MPAtoKSI         /* 2813 */, ITM_KSItoMPA         , ITM_KSItoMPA       , +6 , UT_PRESSURE                 },
  { ITM_LBSIN3toBLOBIN3  /* 2814 */, ITM_BLOBIN3toLBSIN3  , ITM_BLOBIN3toKGM3  , +0 , UT_DENSITY                  },
  { ITM_BLOBIN3toLBSIN3  /* 2815 */, ITM_LBSIN3toBLOBIN3  , ITM_LBSIN3toKGM3   , +0 , UT_DENSITY                  },
  { ITM_LBSIN3toTMM3     /* 2816 */, ITM_TMM3toLBSIN3     , ITM_TMM3toKGM3     , +0 , UT_DENSITY                  },
  { ITM_TMM3toLBSIN3     /* 2817 */, ITM_LBSIN3toTMM3     , ITM_LBSIN3toKGM3   , +0 , UT_DENSITY                  },
  { ITM_LBSIN3toKGM3     /* 2818 */, ITM_KGM3toLBSIN3     , ITM_NULL           , +0 , UT_DENSITY                  },
  { ITM_KGM3toLBSIN3     /* 2819 */, ITM_LBSIN3toKGM3     , ITM_LBSIN3toKGM3   , +0 , UT_DENSITY                  },
  { ITM_KGM3toBLOBIN3    /* 2820 */, ITM_BLOBIN3toKGM3    , ITM_BLOBIN3toKGM3  , +0 , UT_DENSITY                  },
  { ITM_BLOBIN3toKGM3    /* 2821 */, ITM_KGM3toBLOBIN3    , ITM_NULL           , +0 , UT_DENSITY                  },
  { ITM_KGM3toTMM3       /* 2822 */, ITM_TMM3toKGM3       , ITM_TMM3toKGM3     , +0 , UT_DENSITY                  },
  { ITM_TMM3toKGM3       /* 2823 */, ITM_KGM3toTMM3       , ITM_NULL           , +0 , UT_DENSITY                  },
  { ITM_LBFTtoKGM        /* 2824 */, ITM_KGMtoLBFT        , ITM_NULL           , +0 , UT_NOT_CONFIGURABLE         },  // future: UT_LINEAR_MASS_DENSITY
  { ITM_KGMtoLBFT        /* 2825 */, ITM_LBFTtoKGM        , ITM_NULL           , +0 , UT_NOT_CONFIGURABLE         },  // future: UT_LINEAR_MASS_DENSITY
  { ITM_IN3toMM3         /* 2826 */, ITM_MM3toIN3         , ITM_NULL           , -6 , UT_VOLUME                   },
  { ITM_MM3toIN3         /* 2827 */, ITM_IN3toMM3         , ITM_IN3toMM3       , -6 , UT_VOLUME                   },
  { ITM_IN2toMM2         /* 2828 */, ITM_MM2toIN2         , ITM_NULL           , -6 , UT_AREA                     },
  { ITM_MM2toIN2         /* 2829 */, ITM_IN2toMM2         , ITM_IN2toMM2       , -6 , UT_AREA                     },
  { ITM_IN4toMM4         /* 2830 */, ITM_MM4toIN4         , ITM_NULL           , +0 , UT_NOT_CONFIGURABLE         },  // future: UT_SECOND_MOMENT_OF_AREA
  { ITM_MM4toIN4         /* 2831 */, ITM_IN4toMM4         , ITM_NULL           , +0 , UT_NOT_CONFIGURABLE         },  // future: UT_SECOND_MOMENT_OF_AREA
  { ITM_IN6toMM6         /* 2832 */, ITM_MM6toIN6         , ITM_NULL           , +0 , UT_NOT_CONFIGURABLE         },  // future: UT_WARPING_CONSTANT
  { ITM_MM6toIN6         /* 2833 */, ITM_IN6toMM6         , ITM_NULL           , +0 , UT_NOT_CONFIGURABLE         },  // future: UT_WARPING_CONSTANT
  { ITM_KGFPMtoNPM       /* 2834 */, ITM_NPMtoKGFPM       , ITM_NULL           , +0 , UT_LINEAR_FORCE_DENSITY     },
  { ITM_NPMtoKGFPM       /* 2835 */, ITM_KGFPMtoNPM       , ITM_KGFPMtoNPM     , +0 , UT_LINEAR_FORCE_DENSITY     },
  { ITM_KGFtoLBF         /* 2836 */, ITM_LBFtoKGF         , ITM_LBFtoN         , +0 , UT_FORCE                    },
  { ITM_LBFtoKGF         /* 2837 */, ITM_KGFtoLBF         , ITM_KGFtoN         , +0 , UT_FORCE                    },
  { ITM_UNSLUGtoKG       /* 2838 */, ITM_KGtoUNSLUG       , ITM_NULL           , +0 , UT_MASS                     },
  { ITM_KGtoUNSLUG       /* 2839 */, ITM_UNSLUGtoKG       , ITM_UNSLUGtoKG     , +0 , UT_MASS                     },
  { ITM_UNSLINCHtoKG     /* 2840 */, ITM_KGtoUNSLINCH     , ITM_NULL           , +0 , UT_MASS                     },
  { ITM_KGtoUNSLINCH     /* 2841 */, ITM_UNSLINCHtoKG     , ITM_UNSLINCHtoKG   , +0 , UT_MASS                     },
};

static const convPair_t *findPair(int16_t input) {                              // binary search; NULL if not found
  uint16_t lo = 0, hi = NUM_CONVERT_PAIRS, mid;
  while(lo < hi) {
    mid = (lo + hi) >> 1;
    if(convertPairs[mid].item < input) {
      lo = mid + 1;
    }
    else {
      hi = mid;
    }
  }
  return (lo < NUM_CONVERT_PAIRS && convertPairs[lo].item == input) ? &convertPairs[lo] : NULL;
}

int16_t conversionPartner(int16_t input, int16_t *unity, int8_t *exponent, uint8_t *type) {
  const convPair_t *entry = findPair(input);
  if(!entry) {
    return 0;                                                                    // not found
  }
  if(unity)    {*unity    = entry->unity;}
  if(exponent) {*exponent = entry->exponent;}
  if(type)     {*type     = entry->type;}
  return entry->partner;
}

bool_t isItemConversion(int16_t itemNr) {
  return findPair(itemNr) != NULL;                                               // search itemNr in one of the 260 table items
}

bool_t areBothConvertConfigurable(int16_t item1Nr, int16_t item2Nr) {
  const convPair_t *entry1 = findPair(item1Nr);
  const convPair_t *entry2 = findPair(item2Nr);
  return entry1 && entry2 && entry1->type == entry2->type && entry1->type != UT_NOT_CONFIGURABLE;  // both conversions are of the same configurable type, preventing the mixing of units
}

bool_t isStandardPair(int16_t item1Nr, int16_t item2Nr) {  // True when item2Nr is the standard table partner of item1Nr: a fixed pair, use the direct partner conversion, no SI round-trip and no configured-pair magic
  return item2Nr != 0 && conversionPartner(item1Nr, NULL, NULL, NULL) == item2Nr;
}

bool_t isOneOfAConvertPair(uint16_t x, int16_t itemNr, int16_t *oddNrPartner) {
  const convPair_t *entry = findPair(itemNr);
  if(!entry) {
    return false;                                                                // not a conversion-pair member
  }
  if((x & 1) == 0) {
    *oddNrPartner = entry->partner;                                              // even x = left softkey: report partner
  }
  return true;
}

void runConversionToSI(int16_t itemNr) {
  const convPair_t *entry = findPair(itemNr);
  if(!entry) {
    return;                                                                      // not a conversion item; nothing to do
  }
  if(entry->unity != ITM_NULL) {
    runFunction(entry->unity);                                                   // execute a conversion
  }
  if(entry->exponent != 0) {
    fnMultiplySI(100 + entry->exponent);                                         // scale by 10^exponent
  }
}

void runConversionFromSI(int16_t itemNr) {
  const convPair_t *entry = findPair(itemNr);
  if(!entry) {
    return;
  }
  if(entry->exponent != 0) {
    fnMultiplySI(100 - entry->exponent);                                         // undo the exponent
  }
  if(entry->unity != ITM_NULL) {
    runFunction(findPair(entry->unity)->partner);                                // execute the inverse of the unity step
  }
  runFunction(entry->partner);                                                   // execute the inverse of the user's choice
}



void fullConvSoftMenuItemNameInclHPCONV(int16_t item, char *outString) {
  if(!isItemConversion(item)) {                                                  // not a conversion: plain softmenu name
    stringCopy(outString, indexOfItems[item].itemSoftmenuName);
    return;
  }
  const int16_t useNameExcludingRightArrowOnLeft  = item;
  const int16_t useNameExcludingRightArrowOnRight = conversionPartner(item, NULL, NULL, NULL);
  char scratch[64];
  stringCopy(scratch, indexOfItems[useNameExcludingRightArrowOnLeft].itemSoftmenuName);  // left side: source name up to (but excluding) the arrow
  truncateAtArrow(scratch);
  stringCopy(outString, scratch);
  stringCopy(outString + stringByteLength(outString), STD_RIGHT_ARROW);                  // normal right facing arrow between the two sides
  stringCopy(scratch, indexOfItems[useNameExcludingRightArrowOnRight].itemSoftmenuName); // right side: source name up to (but excluding) the arrow
  truncateAtArrow(scratch);
  stringCopy(outString + stringByteLength(outString), scratch);
}


// Determine the conversion pair that will actually execute:
//   - Custom non-standard pair when the active softkey context provides a different pair (MyMenu or DYNAMIC user menus) of the SAME configurable UT
//   - Otherwise the standard pair (conversionPartner(item))
// Outputs optional: pass NULL for either to skip
//   itemNrPair: receives the custom partner ONLY when a custom non-standard pair applies; otherwise 0
//   pairName:   receives the combined "left" STD_RIGHT_ARROW "right" softmenu name reflecting the executed pair, honouring FLAG_HPCONV.
// Dependencies not in the signature: globals dynamicMenuItem, softmenuStack, softmenu[], userMenuItems[], userMenus[], currentUserMenu; flag FLAG_HPCONV.
void executionConversionPartner(int16_t item, int16_t *itemNrPair, char *pairName) {
  if(!isItemConversion(item)) {                                                  // not a conversion: plain softmenu name, no partner work
    if(itemNrPair != NULL) {
      *itemNrPair = 0;
    }
    if(pairName != NULL) {
      stringCopy(pairName, indexOfItems[item].itemSoftmenuName);
    }
    return;
  }
  const int16_t softKeyIx      = dynamicMenuItem ^ 1;
  const int16_t curMenu        = -softmenu[softmenuStack[0].softmenuId].menuItem;
  const int16_t softKeyPartner = (curMenu == MNU_MyMenu)  ? userMenuItems[softKeyIx].item
                               : (curMenu == MNU_DYNAMIC) ? userMenus[currentUserMenu].menuItem[softKeyIx].item
                               : 0;
  if(areBothConvertConfigurable(item, softKeyPartner) && !isStandardPair(item, softKeyPartner)) {  // custom non-standard pair of the SAME configurable UT
    if(itemNrPair != NULL) {
      *itemNrPair = softKeyPartner;
    }
    if(pairName != NULL) {
      const int16_t leftItem  = getSystemFlag(FLAG_HPCONV) ? conversionPartner(softKeyPartner, NULL, NULL, NULL) : item;
      const int16_t rightItem = getSystemFlag(FLAG_HPCONV) ? conversionPartner(item,           NULL, NULL, NULL) : softKeyPartner;
      char scratch[64];
      stringCopy(scratch, indexOfItems[leftItem].itemSoftmenuName);
      truncateAtArrow(scratch);
      stringCopy(pairName, scratch);
      stringCopy(pairName + stringByteLength(pairName), STD_RIGHT_ARROW);
      stringCopy(scratch, indexOfItems[rightItem].itemSoftmenuName);
      truncateAtArrow(scratch);
      stringCopy(pairName + stringByteLength(pairName), scratch);
    }
  }
  else {
    if(itemNrPair != NULL) {                                                     // standard pair (or mismatched UTs): no extra dispatch work
      *itemNrPair = 0;
    }
    if(pairName != NULL) {                                                       // delegate the standard-pair name to the existing helper
      fullConvSoftMenuItemNameInclHPCONV(item, pairName);
    }
  }
}


static void unitConversion(const real_t * const coefficient, uint16_t multiplyDivide, bool_t invert) {
  real_t reX;

  if(!getRegisterAsReal(REGISTER_X, &reX)) {
    return;
  }

  if(!saveLastX()) {
    return;
  }

  if(invert && realIsZero(&reX)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      convertRealToResultRegister(realIsNegative(&reX) ? const_minusInfinity : const_plusInfinity, REGISTER_X, amNone);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function unitConversion:", "cannot calculate divide by zero", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }

  if(invert) {
    realDivide(const_1, &reX, &reX, &ctxtReal39);
  }

  if(multiplyDivide == multiply) {
    realMultiply(&reX, coefficient, &reX, &ctxtReal39);
  }
  else {
    realDivide(&reX, coefficient, &reX, &ctxtReal39);
  }

  convertRealToResultRegister(&reX, REGISTER_X, amNone);
  adjustResult(REGISTER_X, false, false, -1, -1, -1);
}

TO_QSPI static const real_t * const conversionFactors[constFactorEND] = {
    [constFactorFt2Hectare]   = const_Ft2ToHa,      /*   0 */
    [constFactorFt2M2]        = const_Ft2ToM2,
    [constFactorHectareKm2]   = const_100,
    [constFactorAcreHa]       = const_AccreToHa,
    [constFactorAcreusHa]     = const39_AccreusToHa,
    [constFactorAtmPa]        = const_AtmToPa,
    [constFactorAuM]          = const_AuToM,
    [constFactorBarPa]        = const_BarToPa,
    [constFactorBtuJ]         = const_BtuToJ,
    [constFactorCalJ]         = const_CalToJ,
    [constFactorLbfftNm]      = const_LbfftToNm,    /*  10 */
    [constFactorCwtKg]        = const_CwtToKg,
    [constFactorFtM]          = const_FtToM,
    [constFactorSfeetM]       = const39_SfeetToM,
    [constFactorFlozukIn3]    = const_FlozukToIn3,
    [constFactorFlozukMl]     = const_FlozukToMl,
    [constFactorFlozusIn3]    = const_FlozusToIn3,
    [constFactorFlozusMl]     = const_FlozusToMl,
    [constFactorFt3toGalUS]   = const_Ft3ToGalUS,
    [constFactorGalukL]       = const_GalukToL,
    [constFactorGalusL]       = const_GalusToL,     /*  20 */
    [constFactorHpeW]         = const_HpeToW,
    [constFactorHpmW]         = const_HpmToW,
    [constFactorHpukW]        = const_HpukToW,
    [constFactorInhgPa]       = const_InhgToPa,
    [constFactorInchMm]       = const_InchToMm,
    [constFactorWhJ]          = const_WhToJ,
    [constFactorLbKg]         = const_LbToKg,
    [constFactorOzG]          = const_OzToG,
    [constFactorShortcwtKg]   = const_ShortcwtToKg,
    [constFactorStoneKg]      = const_StoneToKg,    /*  30 */
    [constFactorShorttonKg]   = const_ShorttonToKg,
    [constFactorTonKg]        = const_LongtonToKg,
    [constFactorLiangKg]      = const_20,
    [constFactorTrozG]        = const_TrozToG,
    [constFactorLbfN]         = const_LbfToN,
    [constFactorLyM]          = const_LyToM,
    [constFactorMmhgPa]       = const_MmhgToPa,
    [constFactorMiKm]         = const_MiToKm,
    [constFactorNmiKm]        = const_NmiToKm,
    [constFactorPcM]          = const39_PcToM,      /*  40 */
    [constFactorPointMm]      = const39_PointToMm,
    [constFactorMileM]        = const_MiToM,
    [constFactorYardM]        = const_YardToM,
    [constFactorPsiPa]        = const39_PsiToPa,
    [constFactorTorrPa]       = const39_TorrToPa,
    [constFactorYearS]        = const_YearToS,
    [constFactorCaratG]       = const_CaratToG,
    [constFactorJinKg]        = const_2,
    [constFactorQuartL]       = const_QuartToL,
    [constFactorFathomM]      = const_FathomToM,    /*  50 */
    [constFactorNMiM]         = const_NmiToM,
    [constFactorBarrelM3]     = const_BarrelToM3,
    [constFactorHectareM2]    = const_10000,
    [constFactorMuM2]         = const_MuToM2,
    [constFactorLiM]          = const_LiToM,
    [constFactorChiM]         = const_3,
    [constFactorYinM]         = const_YinToM,
    [constFactorCunM]         = const_CunToM,
    [constFactorZhangM]       = const_ZhangToM,
    [constFactorFenM]         = const_FenToM,       /*  60 */
    [constFactorMi2Km2]       = const_MiSqToKmSq,
    [constFactorNmi2Km2]      = const_NmiSqToKmSq,
    [constFactorKmphmps]      = const39_Kmphmps,
    [constFactorRpmDegps]     = const_RpmDegps,
    [constFactorMphmps]       = const_Mphmps,
    [constFactorRpmRadps]     = const39_RpmRadps,
    [constFactorInchCm]       = const_InchToCm,
    [constFactorNmiMi]        = const39_NmiToMi,
    [constFactorFurtom]       = const_furToM,
    [constFactorFtntos]       = const_ftnToS,       /*  70 */
    [constFactorFpftomps]     = const_fpfToMps,
    [constFactorBrdstom]      = const_brdsTom,
    [constFactorFirtokg]      = const_firToKg,
    [constFactorFpftokph]     = const_fpfToKph,
    [constFactorBrdstoin]     = const_brdsToIn,
    [constFactorFirtolb]      = const_firToLb,
    [constFactorFpftomph]     = const_fpfToMph,
    [constFactorFpstokph]     = const_fpsToKph,
    [constFactorFpstomps]     = const_fpsToMps,
    [constFactorL100Tokml]    = const_100,          /*  80 */
    [constFactorKmletok100K]  = NULL,
    [constFactorK100Ktokmk]   = const_100,
    [constFactorL100Tomgus]   = NULL,
    [constFactorMgeustok100M] = NULL,
    [constFactorK100Ktok100M] = const_MiToKm,
    [constFactorL100Tomguk]   = NULL,
    [constFactorMgeuktok100M] = NULL,
    [constFactorK100Mtomik]   = const_100,
    [constFactorCupcFzus]     = const_CupcFzus,
    [constFactorCupcMl]       = const_CupcMl,       /*  90 */
    [constFactorCupukFzuk]    = const_CupukFzuk,
    [constFactorCupukMl]      = const_CupukMl,
    [constFactorFzukCupuk]    = const_CupukFzuk,
    [constFactorFzukTbspuk]   = const_FzukTbspuk,
    [constFactorFzukTspuk]    = const_FzukTspuk,
    [constFactorFzusCupc]     = const_CupcFzus,
    [constFactorFzusTbspc]    = const_FzusTbspc,
    [constFactorFzusTspc]     = const_FzusTspc,
    [constFactorMlCupc]       = const_CupcMl,
    [constFactorMlCupuk]      = const_CupukMl,      /* 100 */
    [constFactorMlPintlq]     = const_PintlqMl,
    [constFactorMlPintuk]     = const_PintukMl,
    [constFactorMlQt]         = const_QtMl,
    [constFactorMlQtus]       = const_QtusMl,
    [constFactorMlTbspc]      = const_TbspcMl,
    [constFactorMlTbspuk]     = const_TbspukMl,
    [constFactorMlTspc]       = const_TspcMl,
    [constFactorMlTspuk]      = const39_TspukMl,
    [constFactorPintlqMl]     = const_PintlqMl,
    [constFactorPintukMl]     = const_PintukMl,     /* 110 */
    [constFactorQtMl]         = const_QtMl,
    [constFactorQtusMl]       = const_QtusMl,
    [constFactorTbspcFzus]    = const_FzusTbspc,
    [constFactorTbspcMl]      = const_TbspcMl,
    [constFactorTbspukFzuk]   = const_FzukTbspuk,
    [constFactorTbspukMl]     = const_TbspukMl,
    [constFactorTspcFzus]     = const_FzusTspc,
    [constFactorTspcMl]       = const_TspcMl,
    [constFactorTspukFzuk]    = const_FzukTspuk,
    [constFactorTspukMl]      = const39_TspukMl,    /* 120 */
    [constFactorMlIn3]        = const_In3Ml,
    [constFactorIn3Ml]        = const_In3Ml,
    [constFactorFt3Gluk]      = const_Ft3Gluk,
    [constFactorGlukFt3]      = const_Ft3Gluk,
    [constFactorLFt3]         = const_Ft3L,
    [constFactorFt3L]         = const_Ft3L,
    [constFactorLQtus]        = const_LQtus,
    [constFactorQtusL]        = const_LQtus,
    [constFactorGlukFzuk]     = const_GlukFzuk,
    [constFactorFzukGluk]     = const_GlukFzuk,     /* 130 */
    [constFactorGlusFzus]     = const_GlusFzus,
    [constFactorFzusGlus]     = const_GlusFzus,
    [constFactoreVJ]          = const_e,
    [constFactorJeV]          = const_e,
    [constFactormmBanana]     = const_bananamm,
    [constFactorBananamm]     = const_bananamm,
    [constFactorInchBanana]   = const39_bananaInch,
    [constFactorBananaInch]   = const39_bananaInch,
    [constFactorErgJ]         = const_ErgToJ,
    [constFactorFoeJ]         = const_FoeToJ,       /* 140 */
    [constFactorKnotMps]      = const39_KnotToMps,
    [constFactor180onPi]      = const39_180onPi,
    [constFactorSlugKg]       = const39_SlugToKg,
    [constFactorSlinchKg]     = const39_SlinchToKg,
    [constFactorBlobKg]       = const39_SlinchToKg,
    [constFactorTonneKg]      = const_1000,
    [constFactorLbsft2Pa]     = const39_Lbsft2ToPa,
    [constFactorInlbsNm]      = const_InlbsToNm,
    [constFactorLbsftNpm]     = const39_SlugToKg,
    [constFactorKgfN]         = const_gEarth,       /* 150 */
    [constFactorKsiMpa]       = const39_KsiToMpa,
    [constFactorLbsBlob]      = const39_BlobInLbs,
    [constFactorLbsin3Tmm3]   = const39_Lbsin3ToTmm3,
    [constFactorLbsin3Kgm3]   = const39_Lbsin3ToKgm3,
    [constFactorKgm3Blobin3]  = const39_Kgm3ToBlobin3,
    [constFactorKgm3Tmm3]     = const_1e_12,
    [constFactorLbsftKgm]     = const39_LbsftToKgm,
    [constFactorIn3Mm3]       = const_In3ToMm3,
    [constFactorIn2Mm2]       = const_In2ToMm2,
    [constFactorIn4Mm4]       = const_In4ToMm4,     /* 160 */
    [constFactorIn6Mm6]       = const_In6ToMm6,
    [constFactorKgmNpm]       = const_gEarth,
    [constFactorInchM]        = const_InchToM,      /* 163 */
    [constFactorLbfKgf]       = const_LbToKg,
  };


void fnUnitConvert(uint16_t arg) {
    const uint16_t multiply = arg & 0x8000;
    const bool_t invert = (arg & 0x4000) != 0;
    const uint16_t idx = arg & 0x3fff;

    unitConversion(conversionFactors[idx], multiply, invert);
}


//  {[(x - B) / C] * D} + E
TO_QSPI static const real_t * const cvtTempConsts[13][4] = {
  //   B              C             D             E
  {const_0,       const_1,       const_9on5,    const_32     }, // ITM_CtoF     ix =  0
  {const_32,      const_9on5,    const_1,       const_0      }, // ITM_FtoC     ix =  1
  {const_0,       const_1,       const_1,       const_273p15 }, // ITM_CtoK     ix =  2
  {const_273p15,  const_1,       const_1,       const_0      }, // ITM_KtoC     ix =  3
  {const_0,       const_9on5,    const_1,       const_0      }, // ITM_RAtoK    ix =  4
  {const_0,       const_1,       const_9on5,    const_0      }, // ITM_KtoRA    ix =  5
  {const_459p67,  const_1,       const_1,       const_0      }, // ITM_RAtoF    ix =  6
  {const_0,       const_1,       const_1,       const_459p67 }, // ITM_FtoRA    ix =  7
  {const_0,       const39_kBeVK, const_1,       const_0      }, // ITM_EVKBtoK  ix =  8
  {const_0,       const_1,       const39_kBeVK, const_0      }, // ITM_KtoEVKB  ix =  9
  {const_32,      const_9on5,    const_1,       const_273p15 }, // ITM_FtoK     ix = 10
  {const_273p15,  const_1,       const_9on5,    const_32     }, // ITM_KtoF     ix = 11
};

void fnCvtTemp(uint16_t ix) {
  real_t reX;

  if(!getRegisterAsReal(REGISTER_X, &reX)) {
    return;
  }

  if(!saveLastX()) {
    return;
  }

  //  (x - B) / C * D + E

  //printf("ix = %d\n",ix);
  //printRealToConsole(cvtTempConsts[ix][0], "(x - "," ) \n");
  //printRealToConsole(cvtTempConsts[ix][1], "/ "," \n");
  //printRealToConsole(cvtTempConsts[ix][2], "x "," \n");
  //printRealToConsole(cvtTempConsts[ix][3], "+ ","\n");
  if(cvtTempConsts[ix][0] != const_0) {realSubtract(&reX, cvtTempConsts[ix][0], &reX, &ctxtReal39);}
  if(cvtTempConsts[ix][1] != const_1) {realDivide  (&reX, cvtTempConsts[ix][1], &reX, &ctxtReal39);}
  if(cvtTempConsts[ix][2] != const_1) {realMultiply(&reX, cvtTempConsts[ix][2], &reX, &ctxtReal39);}
  if(cvtTempConsts[ix][3] != const_0) {realAdd     (&reX, cvtTempConsts[ix][3], &reX, &ctxtReal39);}
  //printRealToConsole(&reX, "Rex: ","\n");

  convertRealToResultRegister(&reX, REGISTER_X, amNone);

  adjustResult(REGISTER_X, false, false, -1, -1, -1);
}


void fnCvtDegRad(uint16_t multiplyDivide) {
  if(getRegisterDataType(REGISTER_X) == dtReal34 && (
    ((getRegisterAngularMode(REGISTER_X) == amDegree) && multiplyDivide == divide) || ((getRegisterAngularMode(REGISTER_X) == amRadian) && multiplyDivide == multiply) )) {
    setRegisterAngularMode(REGISTER_X, amNone);
  }
  unitConversion(const39_180onPi, multiplyDivide, noninverting);
}

void fnCvtDegGrad(uint16_t multiplyDivide) {
  if(getRegisterDataType(REGISTER_X) == dtReal34 && (
    ((getRegisterAngularMode(REGISTER_X) == amDegree) && multiplyDivide == divide) || ((getRegisterAngularMode(REGISTER_X) == amGrad) && multiplyDivide == multiply) )) {
    setRegisterAngularMode(REGISTER_X, amNone);
  }
  unitConversion(const_9on10, multiplyDivide, noninverting);
}

void fnCvtGradRad(uint16_t multiplyDivide) {
  if(getRegisterDataType(REGISTER_X) == dtReal34 && (
    ((getRegisterAngularMode(REGISTER_X) == amGrad) && multiplyDivide == divide) || ((getRegisterAngularMode(REGISTER_X) == amRadian) && multiplyDivide == multiply) )) {
    setRegisterAngularMode(REGISTER_X, amNone);
  }
  unitConversion(const39_200onPi, multiplyDivide, noninverting);
}

void fnKmletok100K   (uint16_t multiplyDivide) {
  //note multiplyDivide is not used, as the formula is biderectional!
  //100*liter_equivalent  / (value), both ways
  real_t factor;
  realMultiply(const_GaluseqE, const_100, &factor, &ctxtReal39);
  realDivide(&factor, const_GalusToL, &factor, &ctxtReal39);
  unitConversion(&factor, multiply, inverting);
}

void fnL100Tomgus   (uint16_t multiplyDivide) {
  //note multiplyDivide is not used, as the formula is biderectional!
  //100 *gallon_US/mile   /  (value), both ways
  real_t factor;
  realMultiply(const_100, const_GalusToL, &factor, &ctxtReal39);
  realDivide(&factor, const_MiToKm, &factor, &ctxtReal39);
  unitConversion(&factor, multiply, inverting);
}

void fnMgeustok100M   (uint16_t multiplyDivide) {
  //note multiplyDivide is not used, as the formula is biderectional!
  //100*gallon_US_equivalent / (value), both ways
  real_t factor;
  realMultiply(const_GaluseqE, const_100, &factor, &ctxtReal39);
  unitConversion(&factor, multiply, inverting);
}

void fnL100Tomguk   (uint16_t multiplyDivide) {
  //note multiplyDivide is not used, as the formula is biderectional!
  //100*gallon_UK/mile  / (value), both ways
  real_t factor;
  realMultiply(const_100, const_GalukToL, &factor, &ctxtReal39);
  realDivide(&factor, const_MiToKm, &factor, &ctxtReal39);
  unitConversion(&factor, multiply, inverting);
}

void fnMgeuktok100M   (uint16_t multiplyDivide) {
  //note multiplyDivide is not used, as the formula is biderectional!
  //100*gallon_UK_equivalent  / (value), both ways
  //const_GalukToL / const_GalusToL * 33.7 * 100
  real_t factor;
  realMultiply(const_GaluseqE, const_100, &factor, &ctxtReal39);
  realMultiply(&factor, const_GalukToL, &factor, &ctxtReal39);
  realDivide(&factor, const_GalusToL, &factor, &ctxtReal39);
  unitConversion(&factor, multiply, inverting);
}


void fnCvtHMSHR   (uint16_t multiplyDivide) {
  if(multiplyDivide == divide) {
    fnHMStoTM(0);
    fnToReal(0);
  }
  else {
    fnHRtoTM(0);
    fnFrom_ms(0);
  }
}





/********************************************//**
 * \brief Converts power or field ratio to dB
 * dB = (10 or 20) * log10((power or field) ratio) this is the exact formula
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnCvtRatioDb(uint16_t tenOrTwenty) { // ten: power ratio   twenty: field ratio
  real_t reX;

  if(!getRegisterAsReal(REGISTER_X, &reX)) {
    return;
  }

  if(!saveLastX()) {
    return;
  }

  WP34S_Log10(&reX, &reX, &ctxtReal39);
  realMultiply(&reX, (tenOrTwenty == 10 ? const_10 : const_20), &reX, &ctxtReal39);

  convertRealToResultRegister(&reX, REGISTER_X, amNone);

  adjustResult(REGISTER_X, false, false, -1, -1, -1);
}



/********************************************//**
 * \brief Converts dB to power or field ratio
 * (power or field) ratio = 10^(dB / 10 or 20) this is the exact formula
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnCvtDbRatio(uint16_t tenOrTwenty) { // ten: power ratio   twenty: field ratio
  real_t reX;

  if(!getRegisterAsReal(REGISTER_X, &reX)) {
    return;
  }

  if(!saveLastX()) {
    return;
  }

  realDivide(&reX, (tenOrTwenty == 10 ? const_10 : const_20), &reX, &ctxtReal39);
  realPower10(&reX, &reX, &ctxtReal39);

  convertRealToResultRegister(&reX, REGISTER_X, amNone);

  adjustResult(REGISTER_X, false, false, -1, -1, -1);
}
