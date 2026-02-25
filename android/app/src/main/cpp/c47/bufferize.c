// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

bool_t delayCloseNim = false;

#if !defined(TESTSUITE_BUILD)

TO_QSPI static const char bugScreenNoParam[] = "In function addItemToBuffer:item should not be NOPARAM=7654!";

  void fnAim(uint16_t unusedButMandatoryParameter) {
    if(calcMode != CM_AIM && calcMode != CM_EIM && (calcMode != CM_PEM || tam.mode || !getSystemFlag(FLAG_ALPHA))) {
      resetShiftState();  //JM
      displayAIMbufferoffset = 0;
      T_cursorPos = 0;
      aimBuffer[0] = 0;
    }
    setLastintegerBasetoZero();
    calcModeAim(NOPARAM); // Alpha Input Mode
    if(programRunStop != PGM_RUNNING) {
      entryStatus |= 0x01;
      setSystemFlag(FLAG_ALPIN);
    }
  }


uint16_t convertItemToSubOrSup(uint16_t item, int16_t subOrSup) {
    if(subOrSup == NC_SUBSCRIPT) {
      nextChar = NC_NORMAL;            //JM de-latching superscript / suscript /sup/sub, removing the lock. Comment out to let sup/sub lock
      if(item >= ITM_0 && item <= ITM_9) return (uint16_t)((int16_t)item + (int16_t)ITM_SUB_0 - (int16_t)ITM_0); else //JM optimized
      if(item >= ITM_a && item <= ITM_z) return (uint16_t)((int16_t)item + (int16_t)ITM_SUB_a - (int16_t)ITM_a); else //JM optimized
      if(item >= ITM_A && item <= ITM_Z) return (uint16_t)((int16_t)item + (int16_t)ITM_SUB_A - (int16_t)ITM_A); else //JM optimized
      switch(item) {
        case ITM_alpha    :return ITM_SUB_alpha;
        case ITM_delta    :return ITM_SUB_delta;
        case ITM_mu       :return ITM_SUB_mu;
        case ITM_pi       :return ITM_SUB_pi;
        case ITM_SUN      :return ITM_SUB_SUN;
        case ITM_INFINITY :return ITM_SUB_INFINITY;
        case ITM_PLUS     :return ITM_SUB_PLUS;
        case ITM_MINUS    :return ITM_SUB_MINUS;
        default           :return item;
      }
    }
    else if(subOrSup == NC_SUPERSCRIPT) {
      nextChar = NC_NORMAL;            //JM de-latching superscript / suscript /sup/sub, removing the lock. Comment out to let sup/sub lock
      if(item >= ITM_0 && item <= ITM_9) return (uint16_t)((int16_t)item + (int16_t)ITM_SUP_0 - (int16_t)ITM_0); else //JM optimized
      if(item >= ITM_a && item <= ITM_z) return (uint16_t)((int16_t)item + (int16_t)ITM_SUP_a - (int16_t)ITM_a); else //JM optimized
      if(item >= ITM_A && item <= ITM_Z) return (uint16_t)((int16_t)item + (int16_t)ITM_SUP_A - (int16_t)ITM_A); else //JM optimized
      switch(item) {
        case ITM_INFINITY :return ITM_SUP_INFINITY;
        case ITM_PLUS     :return ITM_SUP_PLUS;
        case ITM_MINUS    :return ITM_SUP_MINUS;
        case ITM_pi       :return ITM_SUP_pi;
        default           :return item;
      }
    }
    return item;
  }


  int32_t findFirstItem(const char *twoLetters) {
    int16_t menuId = softmenuStack[0].softmenuId;

    if(menuId < NUMBER_OF_DYNAMIC_SOFTMENUS) { // Dynamic softmenu
      uint8_t *firstString, *middleString;
      int16_t first, middle, last;

      first = 0;
      firstString = dynamicSoftmenu[menuId].menuContent;

      last = dynamicSoftmenu[menuId].numItems - 1;

      middle = first + (last - first) / 2;
      middleString = getNthString(dynamicSoftmenu[menuId].menuContent, middle);

      while(first + 1 < last) {
        if(compareString(twoLetters, (char *)middleString, CMP_CLEANED_STRING_ONLY) <= 0) {
          last = middle;
        }
        else {
          first = middle;
          firstString = middleString;
        }

        middle = first + (last - first) / 2;
        middleString = getNthString(dynamicSoftmenu[menuId].menuContent, middle);
      }

      if(compareString(twoLetters, (char *)firstString, CMP_CLEANED_STRING_ONLY) <= 0) {
        return first;
      }
      else {
        return last;
      }
    }

    else { // Static softmenu
      const int16_t *first, *middle, *last;
      first = softmenu[menuId].softkeyItem;
      last = first + softmenu[menuId].numItems - 1;
      while(*last == ITM_NULL) {
        last--;
      }

      middle = first + (last - first) / 2;
      //const int16_t *f = softmenu[menuId].softkeyItem;
      //printf("\n----------------------------------\nfirst  = %3" PRIu64 "   %3d\n", (int64_t)(first - f), *first);
      //printf("middle = %3" PRIu64 "   %3d\n", (int64_t)(middle - f), *middle);
      //printf("last   = %3" PRIu64 "   %3d\n", (int64_t)(last - f), *last);
      while(first + 1 < last) {
        if(compareString(twoLetters, indexOfItems[abs(*middle)].itemCatalogName, CMP_CLEANED_STRING_ONLY) <= 0) {
          last = middle;
        }
        else {
          first = middle;
        }

        middle = first + (last - first) / 2;
      //printf("\nfirst  = %3" PRIu64 "   %3d\n", (int64_t)(first - f), *first);
      //printf("middle = %3" PRIu64 "   %3d\n", (int64_t)(middle - f), *middle);
      //printf("last   = %3" PRIu64 "   %3d\n", (int64_t)(last - f), *last);
      }

      if(compareString(twoLetters, indexOfItems[abs(*first)].itemCatalogName, CMP_CLEANED_STRING_ONLY) <= 0) {
        return first - softmenu[menuId].softkeyItem;
      }
      else {
        return last - softmenu[menuId].softkeyItem;
      }
    }
  }


  void resetAlphaSelectionBuffer(void) {
    asmBuffer[0] = 0;
    fnKeyInCatalog = 0;
    fnTimerStop(TO_ASM_ACTIVE);
    #if !defined(TESTSUITE_BUILD)                   //JMvv
      kill_ASB_icon();
    #endif // TESTSUITE_BUILD                       //JM^^
  }




typedef struct {
  uint16_t itemNr;
} fInMim_t;


TO_QSPI const fInMim_t MimFunctionsType0[] =
  {   //function
    {ITM_EXPONENT },
    {ITM_PERIOD   },
    {ITM_0        },
    {ITM_1        },
    {ITM_2        },
    {ITM_3        },
    {ITM_4        },
    {ITM_5        },
    {ITM_6        },
    {ITM_7        },
    {ITM_8        },
    {ITM_9        },
    {ITM_CHS      },
    {ITM_CONSTpi  }, // do mimAddNumber(item);
  };


TO_QSPI const fInMim_t MimFunctionsType1[] =
  {   //function
    {ITM_STO         },
    {ITM_STOADD      },
    {ITM_STOSUB      },
    {ITM_STOMULT     },
    {ITM_STODIV      },
    {ITM_STOMAX      },
    {ITM_STOMIN      },
    {ITM_RCL         },
    {ITM_RCLADD      },
    {ITM_RCLSUB      },
    {ITM_RCLMULT     },
    {ITM_RCLDIV      },
    {ITM_RCLMAX      },
    {ITM_RCLMIN      },
    {ITM_CF          },
    {ITM_SF          },
    {ITM_FF          },
    {ITM_CNST        },
    {ITM_ALL         },
    {ITM_ENG         },
    {ITM_FIX         },
    {ITM_DSP         },
    {ITM_SCI         },
    {ITM_SIGFIG      },
    {ITM_UNIT        },
    {ITM_IRFRAC      },
    {ITM_DENMAX2     },
    {ITM_SDL         },
    {ITM_SDR         },
    {ITM_RDP         },
    {ITM_RMODE       },
    {ITM_RSD         },
    {ITM_DEG         },
    {ITM_GRAD        },
    {ITM_MULPI       },
    {ITM_RAD         }, // do lastErrorCode = ERROR_NONE; mimEnter(true); runFunction(item);
  };

TO_QSPI const fInMim_t MimFunctionsType2[] =
  {   //function
    {ITM_SQUARE      },
    {ITM_CUBE        },
    {ITM_SQUAREROOTX },
    {ITM_CUBEROOT    },
    {ITM_2X          },
    {ITM_EXP         },
    {ITM_ROUND2      },
    {ITM_10x         },
    {ITM_LOG2        },
    {ITM_LN          },
    {ITM_LOG10       },
    {ITM_1ONX        },
    {ITM_cos         },
    {ITM_cosh        },
    {ITM_sin         },
    {ITM_sinh        },
    {ITM_tan         },
    {ITM_tanh        },
    {ITM_arccos      },
    {ITM_arcosh      },
    {ITM_arcsin      },
    {ITM_arsinh      },
    {ITM_arctan      },
    {ITM_artanh      },
    {ITM_CEIL        },
    {ITM_FLOOR       },
    {ITM_DECR        },
    {ITM_INC         },
    {ITM_IP          },
    {ITM_FP          },
    {ITM_MAGNITUDE   },
    {ITM_NEXTP       },
    {ITM_XFACT       },
    {ITM_LOGICALNOT  },
    {CST_01          },
    {CST_02          },
    {CST_03          },
    {CST_04          },
    {CST_05          },
    {CST_06          },
    {CST_07          },
    {CST_08          },
    {CST_09          },
    {CST_10          },
    {CST_11          },
    {CST_12          },
    {CST_13          },
    {CST_14          },
    {CST_15          },
    {CST_16          },
    {CST_17          },
    {CST_18          },
    {CST_19          },
    {CST_20          },
    {CST_21          },
    {CST_22          },
    {CST_23          },
    {CST_24          },
    {CST_25          },
    {CST_26          },
    {CST_27          },
    {CST_28          },
    {CST_29          },
    {CST_30          },
    {CST_31          },
    {CST_32          },
    {CST_33          },
    {CST_34          },
    {CST_35          },
    {CST_36          },
    {CST_37          },
    {CST_38          },
    {CST_39          },
    {CST_40          },
    {CST_41          },
    {CST_42          },
    {CST_43          },
    {CST_44          },
    {CST_45          },
    {CST_46          },
    {CST_47          },
    {CST_48          },
    {CST_49          },
    {CST_50          },
    {CST_51          },
    {CST_52          },
    {CST_53          },
    {CST_54          },
    {CST_55          },
    {CST_56          },
    {CST_57          },
    {CST_58          },
    {CST_59          },
    {CST_60          },
    {CST_61          },
    {CST_62          },
    {CST_63          },
    {CST_64          },
    {CST_65          },
    {CST_66          },
    {CST_67          },
    {CST_68          },
    {CST_69          },
    {CST_70          },
    {CST_71          },
    {CST_72          },
    {CST_73          },
    {CST_74          },
    {CST_75          },
    {CST_76          },
    {CST_77          },
    {CST_78          },
    {CST_79          },
    {CST_80          },
    {CST_81          },
    {CST_82          },
    {CST_83          },
    {CST_84          },
    {ITM_CtoF        },
    {ITM_FtoC        },
    {ITM_DBtoPR      },
    {ITM_FT2toHA     },
    {ITM_HAtoFT2     },
    {ITM_DBtoFR      },
    {ITM_FT2toM2     },
    {ITM_M2toFT2     },
    {ITM_PRtoDB      },
    {ITM_HAtoKM2     },
    {ITM_KM2toHA     },
    {ITM_FRtoDB      },
    {ITM_GLUKtoFZUK  },
    {ITM_FZUKtoGLUK  },
    {ITM_ACtoHA      },
    {ITM_MLtoIN3     },
    {ITM_HAtoAC      },
    {ITM_IN3toML     },
    {ITM_ACUStoHA    },
    {ITM_FT3toGLUK   },
    {ITM_HAtoACUS    },
    {ITM_GLUKtoFT3   },
    {ITM_PAtoATM     },
    {ITM_ATMtoPA     },
    {ITM_AUtoM       },
    {ITM_MtoAU       },
    {ITM_BARtoPA     },
    {ITM_PAtoBAR     },
    {ITM_BTUtoJ      },
    {ITM_JtoBTU      },
    {ITM_CALtoJ      },
    {ITM_JtoCAL      },
    {ITM_LBFFTtoNM   },
    {ITM_LtoFT3      },
    {ITM_NMtoLBFFT   },
    {ITM_FT3toL      },
    {ITM_CWTtoKG     },
    {ITM_KGtoCWT     },
    {ITM_FTtoM       },
    {ITM_MtoFT       },
    {ITM_FTUStoM     },
    {ITM_LtoQTUS     },
    {ITM_QTUStoL     },
    {ITM_MtoFTUS     },
    {ITM_IN3toFZUK   },
    {ITM_FZUKtoIN3   },
    {ITM_FZUKtoML    },
    {ITM_IN3toFZUS   },
    {ITM_MLtoFZUK    },
    {ITM_FZUStoIN3   },
    {ITM_FZUStoML    },
    {ITM_FT3toGalUS  },
    {ITM_MLtoFZUS    },
    {ITM_GalUStoFT3  },
    {ITM_GLUKtoL     },
    {ITM_LtoGLUK     },
    {ITM_GLUStoL     },
    {ITM_LtoGLUS     },
    {ITM_HPEtoW      },
    {ITM_WtoHPE      },
    {ITM_HPMtoW      },
    {ITM_WtoHPM      },
    {ITM_HPUKtoW     },
    {ITM_WtoHPUK     },
    {ITM_INCHHGtoPA  },
    {ITM_CUPCtoFZUS  },
    {ITM_PAtoINCHHG  },
    {ITM_CUPCtoML    },
    {ITM_INCHtoMM    },
    {ITM_MMtoINCH    },
    {ITM_WHtoJ       },
    {ITM_JtoWH       },
    {ITM_KGtoLBS     },
    {ITM_LBStoKG     },
    {ITM_GtoOZ       },
    {ITM_OZtoG       },
    {ITM_KGtoSCW     },
    {ITM_CUPUKtoFZUK },
    {ITM_SCWtoKG     },
    {ITM_CUPUKtoML   },
    {ITM_KGtoSTO     },
    {ITM_FZUKtoCUPUK },
    {ITM_STOtoKG     },
    {ITM_FZUKtoTBSPUK},
    {ITM_KGtoST      },
    {ITM_FZUKtoTSPUK },
    {ITM_FZUStoCUPC  },
    {ITM_STtoKG      },
    {ITM_FZUStoTBSPC },
    {ITM_FZUStoTSPC  },
    {ITM_KGtoTON     },
    {ITM_KGtoLIANG   },
    {ITM_MLtoCUPC    },
    {ITM_TONtoKG     },
    {ITM_LIANGtoKG   },
    {ITM_MLtoCUPUK   },
    {ITM_GtoTRZ      },
    {ITM_MLtoPINTLQ  },
    {ITM_TRZtoG      },
    {ITM_MLtoPINTUK  },
    {ITM_LBFtoN      },
    {ITM_NtoLBF      },
    {ITM_LYtoM       },
    {ITM_MtoLY       },
    {ITM_MMHGtoPA    },
    {ITM_MLtoQT      },
    {ITM_PAtoMMHG    },
    {ITM_MLtoQTUS    },
    {ITM_MItoKM      },
    {ITM_KMtoMI      },
    {ITM_KMtoNMI     },
    {ITM_NMItoKM     },
    {ITM_MtoPC       },
    {ITM_PCtoM       },
    {ITM_MMtoPOINT   },
    {ITM_MLtoTBSPC   },
    {ITM_MILEtoM     },
    {ITM_POINTtoMM   },
    {ITM_MLtoTBSPUK  },
    {ITM_MtoMILE     },
    {ITM_MtoYD       },
    {ITM_YDtoM       },
    {ITM_PSItoPA     },
    {ITM_PAtoPSI     },
    {ITM_PAtoTOR     },
    {ITM_MLtoTSPC    },
    {ITM_TORtoPA     },
    {ITM_MLtoTSPUK   },
    {ITM_StoYEAR     },
    {ITM_YEARtoS     },
    {ITM_CARATtoG    },
    {ITM_PINTLQtoML  },
    {ITM_JINtoKG     },
    {ITM_GtoCARAT    },
    {ITM_PINTUKtoML  },
    {ITM_KGtoJIN     },
    {ITM_QTtoL       },
    {ITM_LtoQT       },
    {ITM_FATHOMtoM   },
    {ITM_QTtoML      },
    {ITM_NMItoM      },
    {ITM_MtoFATHOM   },
    {ITM_QTUStoML    },
    {ITM_MtoNMI      },
    {ITM_BARRELtoM3  },
    {ITM_TBSPCtoFZUS },
    {ITM_M3toBARREL  },
    {ITM_HMStoHR     },
    {ITM_HRtoHMS     },
    {ITM_TBSPCtoML   },
    {ITM_HECTAREtoM2 },
    {ITM_M2toHECTARE },
    {ITM_MUtoM2      },
    {ITM_M2toMU      },
    {ITM_LItoM       },
    {ITM_MtoLI       },
    {ITM_CHItoM      },
    {ITM_MtoCHI      },
    {ITM_YINtoM      },
    {ITM_MtoYIN      },
    {ITM_CUNtoM      },
    {ITM_MtoCUN      },
    {ITM_ZHANGtoM    },
    {ITM_TBSPUKtoFZUK},
    {ITM_MtoZHANG    },
    {ITM_TBSPUKtoML  },
    {ITM_FENtoM      },
    {ITM_MtoFEN      },
    {ITM_MI2toKM2    },
    {ITM_KM2toMI2    },
    {ITM_NMI2toKM2   },
    {ITM_KM2toNMI2   },
    {ITM_TSPCtoFZUS  },
    {ITM_TSPCtoML    },
    {ITM_TSPUKtoFZUK },
    {ITM_TSPUKtoML   },
    {ITM_GLUStoFZUS  },
    {ITM_FZUStoGLUS  },
    {ITM_KNOTtoKMH   },
    {ITM_KMHtoKNOT   },
    {ITM_KMHtoMPS    },
    {ITM_MPStoKMH    },
    {ITM_RPMtoDEGPS  },
    {ITM_DEGPStoRPM  },
    {ITM_MPHtoKMH    },
    {ITM_KMHtoMPH    },
    {ITM_MPHtoMPS    },
    {ITM_MPStoMPH    },
    {ITM_RPMtoRADPS  },
    {ITM_RADPStoRPM  },
    {ITM_DEGtoRAD    },
    {ITM_RADtoDEG    },
    {ITM_DEGtoGRAD   },
    {ITM_GRADtoDEG   },
    {ITM_GRADtoRAD   },
    {ITM_RADtoGRAD   },
    {ITM_INCHtoCM    },
    {ITM_NMItoMI     },
    {ITM_MItoNMI     },
    {ITM_FURtoM      },
    {ITM_MtoFUR      },
    {ITM_FTNtoS      },
    {ITM_StoFTN      },
    {ITM_FPFtoMPS    },
    {ITM_MPStoFPF    },
    {ITM_BRDStoM     },
    {ITM_MtoBRDS     },
    {ITM_FIRtoKG     },
    {ITM_KGtoFIR     },
    {ITM_FPFtoKPH    },
    {ITM_KPHtoFPF    },
    {ITM_BRDStoIN    },
    {ITM_INtoBRDS    },
    {ITM_FIRtoLB     },
    {ITM_LBtoFIR     },
    {ITM_FPFtoMPH    },
    {ITM_MPHtoFPF    },
    {ITM_FPStoKMH    },
    {ITM_KMHtoFPS    },
    {ITM_FPStoMPS    },
    {ITM_MPStoFPS    },
    {ITM_L100toKML   },
    {ITM_KMLtoL100   },
    {ITM_KMLEtoK100K },
    {ITM_K100KtoKMLE },
    {ITM_K100KtoKMK  },
    {ITM_KMKtoK100K  },
    {ITM_L100toMGUS  },
    {ITM_MGUStoL100  },
    {ITM_MGEUStoK100M},
    {ITM_K100MtoMGEUS},
    {ITM_K100KtoK100M},
    {ITM_K100MtoK100K},
    {ITM_L100toMGUK  },
    {ITM_MGUKtoL100  },
    {ITM_MGEUKtoK100M},
    {ITM_K100MtoMGEUK},
    {ITM_K100MtoMIK  },
    {ITM_MIKtoK100M  },
    {ITM_NSIGMA      },
    {ITM_SIGMAx      },
    {ITM_SIGMAy      },
    {ITM_SIGMAx2     },
    {ITM_SIGMAx2y    },
    {ITM_SIGMAy2     },
    {ITM_SIGMAxy     },
    {ITM_SIGMAlnxy   },
    {ITM_SIGMAlnx    },
    {ITM_SIGMAln2x   },
    {ITM_SIGMAylnx   },
    {ITM_SIGMAlny    },
    {ITM_SIGMAln2y   },
    {ITM_SIGMAxlny   },
    {ITM_SIGMAx2lny  },
    {ITM_SIGMAlnyonx },
    {ITM_SIGMAx2ony  },
    {ITM_SIGMA1onx   },
    {ITM_SIGMA1onx2  },
    {ITM_SIGMAxony   },
    {ITM_SIGMA1ony   },
    {ITM_SIGMA1ony2  },
    {ITM_SIGMAx3     },
    {ITM_SIGMAx4     },
    {ITM_BATT        },
    {ITM_BN          },
    {ITM_BNS         },
    {ITM_CONJ        },
    {ITM_CORR        },
    {ITM_COV         },
    {ITM_BESTFQ      },
    {ITM_DAY         },
    {ITM_DtoJ        },
    {ITM_DTtoJ       },
    {ITM_ERF         },
    {ITM_ERFC        },
    {ITM_EXPT        },
    {ITM_FIB         },
    {ITM_DISK        },
    {ITM_GD          },
    {ITM_GDM1        },
    {ITM_IM          },
    {ITM_JtoDT       },
    {ITM_LASTX       },
    {ITM_LNGAMMA     },
    {ITM_LocRQ       },
    {ITM_MANT        },
    {ITM_MEM         },
    {ITM_MONTH       },
    {ITM_sincpi      },
    {ITM_RAN         },
    {ITM_RE          },
    {ITM_REexIM      },
    {ITM_RMODEQ      },
    {ITM_EX1         },
    {ITM_ROUNDI2     },
    {ITM_SETSIG2     },
    {ITM_SIGN        },
    {ITM_ISM         },
    {ITM_SMW         },
    {ITM_SSIZE       },
    {ITM_LN1X        },
    {ITM_SW          },
    {ITM_SXY         },
    {ITM_TICKS       },
    {ITM_UNITV       },
    {ITM_WDAY        },
    {ITM_WM          },
    {ITM_WP          },
    {ITM_WM1         },
    {ITM_XW          },
    {ITM_YEAR        },
    {ITM_GAMMAX      },
    {ITM_zetaX       },
    {ITM_STDDEVPOP   },
    {ITM_M1X         },
    {ITM_HRtoTM      },
    {ITM_toREAL      },
    {ITM_Kk          },
    {ITM_Ek          },
    {ITM_ARG         },
    {ITM_op_a        },
    {ITM_op_a2       },
    {ITM_EE_EXP_TH   }, // do mimRunFunction(item, indexOfItems[item].param);
   };

  static bool_t isFunctionInMim(int16_t com, uint8_t fType) {
    uint_fast16_t n = fType == 0 ? nbrOfElements(MimFunctionsType0) : \
                      fType == 1 ? nbrOfElements(MimFunctionsType1) : \
                                   nbrOfElements(MimFunctionsType2);
    for(uint_fast16_t i=0; i<n; i++) {
      if(fType == 0 ? com == MimFunctionsType0[i].itemNr : fType == 1 ? com == MimFunctionsType1[i].itemNr : com == MimFunctionsType2[i].itemNr) {
        return true;
      }
    }
    return false;
  }



typedef struct {
  char     noStr[3];
} numStr;

#if !defined(TESTSUITE_BUILD)
  TO_QSPI static const numStr NumMsg[] = {
    { "^0" },
    { "^1" },
    { "^2" },
    { "^3" },
    { "^4" },
    { "^5" },
    { "^6" },
    { "^7" },
    { "^8" },
    { "^9" },
  };
#endif //TESTSUITE_BUILD

  void nimFractionToDisplayBuffer(const char *buffer, char *displayBuffer);

  void nimRealToDisplayBuffer(const char *buffer, int16_t exponentLocation, char *displayBuffer);

  void addItemToBuffer(uint16_t item) {
    #if defined(PC_BUILD)
      char tmp[200]; sprintf(tmp,"bufferize.c:addItemToBuffer item=%d tam.mode=%d\n",item,tam.mode); jm_show_calc_state(tmp);
    #endif // PC_BUILD
    //resetKeytimers();  //JM

    if(item == NOPARAM) {
      displayBugScreen(bugScreenNoParam);
    }
    else {
      screenUpdatingMode &= ~(SCRUPD_MANUAL_STACK | SCRUPD_MANUAL_SHIFT_STATUS);
      currentSolverStatus &= ~SOLVER_STATUS_READY_TO_EXECUTE;
      if(calcMode == CM_NORMAL && fnKeyInCatalog && (isAlphabeticSoftmenu() || isJMAlphaOnlySoftmenu()) && !tam.mode) {
        fnAim(NOPARAM);
      }
      else if((fnKeyInCatalog || !catalog || catalog == CATALOG_MVAR) && (((calcMode == CM_AIM || calcMode == CM_EIM) && !tam.mode) || tam.alpha)) {
        item = convertItemToSubOrSup(item, nextChar);
        if(tam.alpha){
          insertAlphaCharacter(item, &alphaCursor);
        }
        else if(stringByteLength(aimBuffer) + (item == ITM_poly_SIGN ? 24 : stringByteLength(indexOfItems[item].itemSoftmenuName)) >= AIM_BUFFER_LENGTH) { /// TODO this error should never happen but who knows!
          sprintf(errorMessage, "In function addItemToBuffer:the AIM input buffer is full! %d bytes for now", AIM_BUFFER_LENGTH);
          displayBugScreen(errorMessage);
        }
        else if(calcMode == CM_EIM) {
          const char *addChar0 = item == ITM_EEXCHR               ? "E":                      //PRODUCT_SIGN "10^" :
                                 item == ITM_PAIR_OF_PARENTHESES  ? "()" :
                                 item == ITM_VERTICAL_BAR         ? "||" :
                                 item == ITM_MAGNITUDE            ? "||" :
                                 item == ITM_ROOT_SIGN            ? STD_SQUARE_ROOT "()" :
                                 item == ITM_SQUAREROOTX          ? STD_SQUARE_ROOT "()" :
                                 item == ITM_CUBEROOT             ? STD_CUBE_ROOT "()" :
                                 item == ITM_XTHROOT              ? STD_xTH_ROOT "(:)" :
                                 item == ITM_EXP                  ? STD_EulerE "^()" :
                                 item == ITM_ALOG_SIGN            ? STD_EulerE "^()" :
                                 item == ITM_LG_SIGN              ? "LOG()" :
                                 item == ITM_LN_SIGN              ? "LN()"  :
                                 item == ITM_LOG2                 ? "LB()" :
                                 item == ITM_SIN_SIGN             ? "SIN()" :
                                 item == ITM_COS_SIGN             ? "COS()" :
                                 item == ITM_TAN_SIGN             ? "TAN()" :
                                 item == ITM_OBELUS               ? STD_SLASH :
                                 item == ITM_poly_SIGN            ? "a4" STD_DOT "x^4+a3" STD_DOT "x^3+a2" STD_DOT "x^2+a1" STD_DOT "x+a0" :
                                 item == ITM_op_j_SIGN            ? COMPLEX_UNIT :
                                 item == ITM_zetaX                ? STD_zeta "()" :
                                 item == ITM_GAMMAX               ? STD_GAMMA "()" :
                                 item == ITM_XFACT                ? "!" :
                                 item == ITM_M1X                  ? "(-1)^()" :
                                 item == ITM_COMB                 ? "COMB(:)" :
                                 item == ITM_PERM                 ? "PERM(:)" :

                                 item >= FIRST_CONSTANT &&
                                    item <= LAST_CONSTANT         ? indexOfItems[item].itemCatalogName :
                                 item >= ITM_SUP_0 &&
                                    item <= ITM_SUP_9             ? NumMsg[item - ITM_SUP_0].noStr : "";


          char addChar[100];
          int16_t jj = 0;
          addChar[0] = 0;
          if(addChar0[0] == 0) {
            if(item != ITM_EQUAL) {       //block the entry of "="
              stringCopy(addChar, indexOfItems[item].itemSoftmenuName);
              if((indexOfItems[item].itemSoftmenuName[0]!=0) && (indexOfItems[item].status & EIM_STATUS) == EIM_ENABLED) {
                if(isDyadicFunction(item)) {
                  stringCopy(addChar + stringByteLength(addChar), "(:)");
                  jj = 2;
                }
                else {
                  stringCopy(addChar + stringByteLength(addChar), "()");
                  jj = 1;
                }
              }
            }
          }
          else {
            stringCopy(addChar, addChar0);
            if(addChar0[0] == '^') {
              if(scrLock == NC_SUPERSCRIPT) {
                scrLock = NC_NORMAL;
                fnRefreshState();
              }
            }
          }

          char *aimCursorPos = aimBuffer;
          char *aimBottomPos = aimBuffer + stringByteLength(aimBuffer);
          uint32_t itemLen = stringByteLength(addChar);
          for(uint32_t i = 0; i < xCursor; ++i) {
            aimCursorPos += (*aimCursorPos & 0x80) ? 2 :1;
          }
          for(; aimBottomPos >= aimCursorPos; --aimBottomPos) {
            *(aimBottomPos + itemLen) = *aimBottomPos;
          }
          xcopy(aimCursorPos, addChar, itemLen);
          if(jj != 0) {
            xCursor += stringGlyphLength(addChar) - jj;
          }
          else {
            switch(item) {
              case ITM_poly_SIGN: {
                xCursor += 0;
                break;
              }
              case ITM_M1X: {
                xCursor += 6;
                break;
              }
              case ITM_COMB:
              case ITM_PERM: {
                xCursor += 5;
                break;
              }
              case ITM_LG_SIGN:
              case ITM_SIN_SIGN:
              case ITM_COS_SIGN:
              case ITM_TAN_SIGN: {
                xCursor += 4;
                break;
              }
              case ITM_ALOG_SIGN:
              case ITM_LOG2:
              case ITM_EXP:
              case ITM_LN_SIGN: {
                xCursor += 3;
                break;
              }
              case ITM_ROOT_SIGN:
              case ITM_SQUAREROOTX:
              case ITM_CUBEROOT:
              case ITM_XTHROOT:
              case ITM_GAMMAX:
              case ITM_zetaX: {
                xCursor += 2;
                break;
              }
              case ITM_PAIR_OF_PARENTHESES:
              case ITM_VERTICAL_BAR:
              case ITM_MAGNITUDE: {
                xCursor += 1;
                break;
              }
              case ITM_XFACT: {
                break;
              }

              default: {
                xCursor += stringGlyphLength(addChar);
              }
            }
          }
        }
        else if(stringByteLength(aimBuffer) <= AIM_BUFFER_LENGTH-1 &&
                 stringWidthWithLimitC47(aimBuffer, stdNoEnlarge, nocompress, SCREEN_WIDTH * MAXLINES - 16, true, true) < SCREEN_WIDTH * MAXLINES - 16 //0 means small standard font
               ) {    //JM
          #if defined(TEXT_MULTILINE_EDIT)
            //JMCURSOR vv ADD THE CHARACTER MID-STRING =======================================================
            uint16_t ix = 0;
            uint16_t in = 0;
            while(ix<T_cursorPos && in<T_cursorPos) {              //search the ix position in aimBuffer before the cursor
              in = stringNextGlyphNoEndCheck_JM(aimBuffer, in);                  //find the in position in aimBuffer which is then the cursor position
              ix++;
            }
            T_cursorPos = in;
            char ixaa[AIM_BUFFER_LENGTH];                           //prepare temporary aimBuffer
            xcopy(ixaa, aimBuffer,in);                              //copy everything up to the cursor position
            ixaa[in]=0;                                             //stop new buffer at cursor position to be able to insert new character

            //          strcat(ixaa,indexOfItems[item].itemSoftmenuName);       //add new character
            uint16_t nq = stringByteLength(indexOfItems[item].itemSoftmenuName);
            xcopy(ixaa + in, indexOfItems[item].itemSoftmenuName, nq+1);
            ixaa[in + nq]=0;

            //          strcat(ixaa,aimBuffer + in);                            //copy rest of the aimbuffer
            uint16_t nr = stringByteLength(aimBuffer + in);
            xcopy(ixaa + in + nq, aimBuffer + in, nr+1);
            ixaa[in + nq + nr]=0;

            //          strcpy(aimBuffer,ixaa);                                 //return temporary string to aimBuffer
            xcopy(aimBuffer,ixaa,stringByteLength(ixaa)+1);

            T_cursorPos = stringNextGlyph(aimBuffer, T_cursorPos);  //place the cursor at the next glyph boundary
            //JMCURSOR ^^ REPLACES THE FOLLOWING XCOPY, WHICH NORMALLY JUST ADDS A CHARACTER TO THE END OF THE STRING
            // xcopy(aimBuffer + stringNextGlyph(aimBuffer, stringLastGlyph(aimBuffer)), indexOfItems[item].itemSoftmenuName, stringByteLength(indexOfItems[item].itemSoftmenuName) + 1);
            switch(item) { // NOTE:cursor must jump on 3 places for the new COS_SIGN etc.
              case ITM_LG_SIGN :   //JM C47
              case ITM_SIN_SIGN :  //JM C47
              case ITM_COS_SIGN :  //JM C47
              case ITM_TAN_SIGN :  //JM C47
              case ITM_ROOT_SIGN:
                T_cursorPos += 2;
                break;
              case ITM_LN_SIGN :  //JM C47
                T_cursorPos += 1;
                break;
              case ITM_PAIR_OF_PARENTHESES:
                T_cursorPos += 1;
                break;
              default:;
            }

            incOffset();

          #else // !TEXT_MULTILINE_EDIT
            xcopy(aimBuffer + stringByteLength(aimBuffer), indexOfItems[item].itemSoftmenuName, stringByteLength(indexOfItems[item].itemSoftmenuName) + 1);
          #endif // TEXT_MULTILINE_EDIT
        }
      }

      #undef SCROLL_ASM          // define this to have the ASM letters scroll if you type more than two. Alternative is it takes two, then you wait 3 and type again another word
      #ifdef SCROLL_ASM
        #define Scroll_Asm 2
      #else
        #define Scroll_Asm 1
      #endif

      if(catalog && catalog != CATALOG_MVAR && !fnKeyInCatalog) {
        if(item == ITM_BACKSPACE) {
          calcModeNormal();
          return;
        }

        // NOP if not a single character input for search
        // or if we already have two characters in the search buffer
        else if(stringGlyphLength(indexOfItems[item].itemSoftmenuName) == 1 &&
                stringGlyphLength(asmBuffer) <= Scroll_Asm &&
                item != ITM_CR && item != ITM_ROOT_SIGN &&
                currentSoftmenuScrolls()) {
          #ifdef SCROLL_ASM
            if(stringGlyphLength(asmBuffer) == 2) {  //2 glyphs <= 4 bytes
              xcopy(asmBuffer, asmBuffer + stringNextGlyphNoEndCheck_JM(asmBuffer, 0), 3);  //lalways leaving char 0 or 01, copy char nos '123' to '012' | or chars '234' to '012' of (01234) characters, including the terminating 0
            }
          #endif //SCROLL_ASM

          stringCopy(asmBuffer + stringByteLength(asmBuffer), indexOfItems[item].itemSoftmenuName);

          softmenuStack[0].firstItem = findFirstItem(asmBuffer);
          setCatalogLastPos();
          fnTimerStart(TO_ASM_ACTIVE, TO_ASM_ACTIVE, 3000);
          #if !defined(TESTSUITE_BUILD)
            light_ASB_icon();
          #endif // !TESTSUITE_BUILD
        }
        if(calcMode == CM_PEM) {
          hourGlassIconEnabled = false;
        }
      }

      else if(tam.mode) {
        if((item == ITM_INDIRECT_X) || (item == ITM_INDIRECT_Y) || (item == ITM_INDIRECT_Z) || (item == ITM_INDIRECT_T)) {
          tamProcessInput(ITM_INDIRECTION);
          tamProcessInput(item - (ITM_INDIRECT_X - ITM_REG_X));
        }
        else {
          tamProcessInput(item);
        }
      }

      else if(calcMode == CM_NIM) {
        addItemToNimBuffer(item);
      }

      //Probably wrong place for this function?! Should Arrow be processed in buffercize.c in this case? //Switch statement better.
      else if(calcMode == CM_MIM) {
        if(temporaryInformation == TI_SHOW_REGISTER) {
          temporaryInformation = TI_NO_INFO;
        }

        if(item == ITM_RIGHT_ARROW) {
          mimEnter(true);
          setJRegisterAsInt(true, getJRegisterAsInt(true) + 1);
          refreshScreen(51);
        }
        else if(item == ITM_LEFT_ARROW) {
          mimEnter(true);
          setJRegisterAsInt(true, getJRegisterAsInt(true) - 1);
          refreshScreen(52);
        }
        else if(item == ITM_UP_ARROW) {
          mimEnter(true);
          setIRegisterAsInt(true, getIRegisterAsInt(true) - 1);
          refreshScreen(53);
        }
        else if(item == ITM_DOWN_ARROW) {
          mimEnter(true);
          setIRegisterAsInt(true, getIRegisterAsInt(true) + 1);
          refreshScreen(54);
        }

        if((int16_t)item < 0) {
          showSoftmenu(item);
          refreshScreen(55);
          return;
        }

        switch(item) {

          case ITM_SHOW: {
            mimEnter(true);
            temporaryInformation = TI_SHOW_REGISTER;
            break;
          }

          case ITM_OFF: {
            runFunction(ITM_OFF);
            break;
          }

          default: {
            if(isFunctionInMim(item,0)) {
                mimAddNumber(item);
                break;
            }
            else if(isFunctionInMim(item,1)) {
                lastErrorCode = ERROR_NONE;
                mimEnter(true);
                runFunction(item);
                break;
            } else if(isFunctionInMim(item,2)) {
                mimRunFunction(item, indexOfItems[item].param);
                break;
            }
          }
        }
      }

      else if(calcMode != CM_AIM && calcMode != CM_EIM && calcMode != CM_ASSIGN && (item >= ITM_A && item <= ITM_F)) {
        // We are not in NIM, but should enter NIM - this should be handled here
        // unlike digits 0 to 9 which are handled by processKeyAction
        addItemToNimBuffer(item);
      }

      else if(calcMode != CM_AIM && calcMode != CM_EIM) {
        funcOK = false;
        return;
      }

      funcOK = true;
    }
  }


  bool_t validShortIntegerInX(void) {
    uint16_t lg = strlen(aimBuffer);
    uint16_t posHash = lg;
    uint16_t i;
    if(nimNumberPart == NP_INT_BASE) {
      return true;
    } else {
      for(i=1; i<lg-1; i++) {      //do not check the # on the very begin or very end as that is not valid
        if(aimBuffer[i] == '#') {
          posHash = i;
        }
      }
    }
    for(i=0; i<posHash; i++) {
      if(aimBuffer[i] == 'e' || aimBuffer[i] == 'i' || aimBuffer[i] == ',' || aimBuffer[i] == '.') {
        return false;
      }
    }
    uint8_t base = 0;
    if((aimBuffer[lg-2]) == '#' && (aimBuffer[lg-1] >= '0' && aimBuffer[lg-1] <= '9')) base = aimBuffer[lg-1];
    if((aimBuffer[lg-3]) == '#' && (aimBuffer[lg-2] >= '0' && aimBuffer[lg-2] <= '1') && (aimBuffer[lg-1] >= '0' && aimBuffer[lg-1] <= '9')) base = aimBuffer[lg-2]*10 + aimBuffer[lg-1];
    if(base < 2 || base > 16) {
      return false;
    }
    int start = 0;
    if(aimBuffer[0] == '-' || aimBuffer[0] == '+' || aimBuffer[0] == ' ') start++;
    for(i=start; i<posHash; i++) {
      if(!(aimBuffer[i] >= '0' && aimBuffer[i] <= '9') && !(aimBuffer[i] >= 'A' && aimBuffer[i] <= 'F')) {
        return false;
      }
    }
    return posHash > 0;
  }

  void addItemToNimBuffer(int16_t item) {
    int16_t lastChar, index;
    uint8_t savedNimNumberPart;
    bool_t done;
    char *strBase;

    if((calcMode == CM_NIM || calcMode == CM_NORMAL) && Input_Default == ID_LI && item == ITM_PERIOD) {
      return;
    }

    if(item >= ITM_A && item <= ITM_F && lastIntegerBase == 0) {
      lastIntegerBase = 16;
    }
//    if(item != ITM_EXIT1) resetKeytimers();  //JM

    screenUpdatingMode &= ~(SCRUPD_MANUAL_STACK | SCRUPD_MANUAL_SHIFT_STATUS);
    currentSolverStatus &= ~SOLVER_STATUS_READY_TO_EXECUTE;

    if(calcMode == CM_NORMAL) {
      switch(item) {
        case ITM_EXPONENT: {
          calcModeNim(NOPARAM);
          aimBuffer[0] = '+';
          aimBuffer[1] = '1';
          aimBuffer[2] = '.';
          aimBuffer[3] = 0;
          nimNumberPart = NP_REAL_FLOAT_PART;
          setLastintegerBasetoZero();
          break;
        }

        case ITM_PERIOD: {
          calcModeNim(NOPARAM);
          aimBuffer[0] = '+';
          aimBuffer[1] = '0';
          aimBuffer[2] = 0;
          nimNumberPart = NP_INT_10;
          setLastintegerBasetoZero();
          break;
        }

        case ITM_0 :
        case ITM_1 :
        case ITM_2 :
        case ITM_3 :
        case ITM_4 :
        case ITM_5 :
        case ITM_6 :
        case ITM_7 :
        case ITM_8 :
        case ITM_9 :
        case ITM_A :
        case ITM_B :
        case ITM_C :
        case ITM_D :
        case ITM_E :
        case ITM_F: {
          calcModeNim(NOPARAM);
          aimBuffer[0] = '+';
          aimBuffer[1] = 0;
          nimNumberPart = NP_EMPTY;
          break;
        }

        default: {
          sprintf(errorMessage, "In function addItemToNimBuffer:%d is an unexpected item value when initializing NIM!", item);
          displayBugScreen(errorMessage);
          return;
      }
      }

      if(programRunStop != PGM_RUNNING) {
        entryStatus |= 0x01;
        setSystemFlag(FLAG_NUMIN);
      }

      //debugNIM();
    }

    else if(calcMode == CM_NIM) {
      screenUpdatingMode |= SCRUPD_SKIP_STACK_ONE_TIME;
    }

    done = false;

    switch(item) {
      case ITM_0 :
      case ITM_1 :
      case ITM_2 :
      case ITM_3 :
      case ITM_4 :
      case ITM_5 :
      case ITM_6 :
      case ITM_7 :
      case ITM_8 :
      case ITM_9: {
        done = true;

        switch(nimNumberPart) {
          case NP_INT_10: {
            if(item == ITM_0) {
              //if(aimBuffer[1] != '0') {  //JM_vv TYPE0; Allow starting the NIM buffer with 0000
                strcat(aimBuffer, "0");
              //}
            }
            else {
              if(aimBuffer[1] == '0') {
                //aimBuffer[1] = 0;        //JM_^^ TYPE0
              }

              strcat(aimBuffer, indexOfItems[item].itemSoftmenuName);
            }
            break;
          }

          case NP_REAL_EXPONENT: {
            if(item == ITM_0) {
              if(aimBuffer[exponentSignLocation + 1] == '0') {
                aimBuffer[strlen(aimBuffer) - 1] = 0;
              }

              if(aimBuffer[exponentSignLocation + 1] != 0 || aimBuffer[exponentSignLocation] == '+') {
                strcat(aimBuffer, "0");
              }

              if(stringToInt16(aimBuffer + exponentSignLocation) > exponentLimit || stringToInt16(aimBuffer + exponentSignLocation) < -exponentLimit) {
                aimBuffer[strlen(aimBuffer) - 1] = 0;
              }
            }
            else {
              if(aimBuffer[exponentSignLocation + 1] == '0') {
                aimBuffer[strlen(aimBuffer) - 1] = 0;
              }

              strcat(aimBuffer, indexOfItems[item].itemSoftmenuName);

              if(stringToInt16(aimBuffer + exponentSignLocation) > exponentLimit || stringToInt16(aimBuffer + exponentSignLocation) < -exponentLimit) {
                aimBuffer[strlen(aimBuffer) - 1] = 0;
              }
            }
            break;
          }

          case NP_HP32SII_DENOMINATOR:
          case NP_FRACTION_DENOMINATOR: {
            if(item == ITM_0) {
              strcat(aimBuffer, "0");

              if(aimBuffer[denominatorLocation] == '0') {
                aimBuffer[denominatorLocation] = 0;
              }
              if(toInt32(aimBuffer + denominatorLocation) < 0 || toInt32(aimBuffer + denominatorLocation) > 999999999) {  // 999999999 is the largest clean decimal number lower than < 2147483647 being the 2^31-1 limit, enlarged beyond the DENMAX as it has nothing to do with that, it is a fraction input limit
                aimBuffer[strlen(aimBuffer) - 1] = 0;
              }
            }
            else {
              strcat(aimBuffer, indexOfItems[item].itemSoftmenuName);
              if(toInt32(aimBuffer + denominatorLocation) < 0 || toInt32(aimBuffer + denominatorLocation) > 999999999) {
                aimBuffer[strlen(aimBuffer) - 1] = 0;
              }
            }
            break;
          }

          case NP_COMPLEX_INT_PART: {
            if(item == ITM_0) {
              if(aimBuffer[imaginaryMantissaSignLocation + 2] != '0') {
                strcat(aimBuffer, "0");
              }
            }
            else {
              if(aimBuffer[imaginaryMantissaSignLocation + 2] == '0') {
                aimBuffer[imaginaryMantissaSignLocation + 2] = 0;
              }

              strcat(aimBuffer, indexOfItems[item].itemSoftmenuName);
            }
            break;
          }

          case NP_COMPLEX_EXPONENT: {
            if(item == ITM_0) {
              if(aimBuffer[imaginaryExponentSignLocation + 1] == '0') {
                aimBuffer[strlen(aimBuffer) - 1] = 0;
              }

              if(aimBuffer[imaginaryExponentSignLocation + 1] != 0 || aimBuffer[imaginaryExponentSignLocation] == '+') {
                strcat(aimBuffer, "0");
              }

              if(stringToInt16(aimBuffer + imaginaryExponentSignLocation) > exponentLimit || stringToInt16(aimBuffer + imaginaryExponentSignLocation) < -exponentLimit) {
                aimBuffer[strlen(aimBuffer) - 1] = 0;
              }
            }
            else {
              if(aimBuffer[imaginaryExponentSignLocation + 1] == '0') {
                aimBuffer[strlen(aimBuffer) - 1] = 0;
              }

              strcat(aimBuffer, indexOfItems[item].itemSoftmenuName);

              if(stringToInt16(aimBuffer + imaginaryExponentSignLocation) > exponentLimit || stringToInt16(aimBuffer + imaginaryExponentSignLocation) < -exponentLimit) {
                aimBuffer[strlen(aimBuffer) - 1] = 0;
              }
            }
            break;
          }

          case NP_COMPLEX_HP32SII_DENOMINATOR:
          case NP_COMPLEX_FRACTION_DENOMINATOR: {
            if(item == ITM_0) {
              strcat(aimBuffer, "0");

              if(aimBuffer[imaginaryDenominatorLocation] == '0') {
                aimBuffer[imaginaryDenominatorLocation] = 0;
              }
              if(toInt32(aimBuffer + imaginaryDenominatorLocation) < 0 || toInt32(aimBuffer + imaginaryDenominatorLocation) > 999999999) {  // 999999999 is the largest clean decimal number lower than < 2147483647 being the 2^31-1 limit, enlarged beyond the DENMAX as it has nothing to do with that, it is a fraction input limit
                aimBuffer[strlen(aimBuffer) - 1] = 0;
              }
            }
            else {
              strcat(aimBuffer, indexOfItems[item].itemSoftmenuName);
              if(toInt32(aimBuffer + imaginaryDenominatorLocation) < 0 || toInt32(aimBuffer + imaginaryDenominatorLocation) > 999999999) {
                aimBuffer[strlen(aimBuffer) - 1] = 0;
              }
            }
            break;
          }

          case NP_INT_BASE: {
            strcat(aimBuffer, indexOfItems[item].itemSoftmenuName);
            strBase = strchr(aimBuffer, '#') + 1;
            if(atoi(strBase) > 16) {
              strBase[1] = 0;
            }
            else if(atoi(strBase) >= 2) {
              goto addItemToNimBuffer_exit;
            }
            break;
          }

          default: {
            if(nimNumberPart == NP_EMPTY) {
              nimNumberPart = NP_INT_10;
              //debugNIM();
            }

            strcat(aimBuffer, indexOfItems[item].itemSoftmenuName);
        }
        }
        break;
      }

      case ITM_A :
      case ITM_B :
      case ITM_C :
      case ITM_D :
      case ITM_E :
      case ITM_F: {
        done = true;

        if(nimNumberPart == NP_EMPTY || nimNumberPart == NP_INT_10 || nimNumberPart == NP_INT_16) {
          if(aimBuffer[1] == '0') {
            //aimBuffer[1] = 0;       //JM_TYPE0 start NIM entry with 0. Why not?
          }

          strcat(aimBuffer, indexOfItems[item].itemSoftmenuName);
          hexDigits++;

          nimNumberPart = NP_INT_16;
          if(lastIntegerBase <= 10) lastIntegerBase = 16;       // [DL] auto set base to hex when entering A-F digit
          //debugNIM();
        }
        break;
      }

      case ITM_PERIOD: {
        done = true;

        if(aimBuffer[strlen(aimBuffer)-1] == 'i') {
          strcat(aimBuffer, "0");
        }

        setLastintegerBasetoZero();

        switch(nimNumberPart) {
          case NP_INT_10: {
            strcat(aimBuffer, ".");

            nimNumberPart = NP_REAL_FLOAT_PART;
            //debugNIM();
            break;
          }

          case NP_REAL_FLOAT_PART: {
            bool_t HP32SII = false;
            if(aimBuffer[strlen(aimBuffer) - 1] == '.') {
              HP32SII = true; //This is the 123 .. 4 meaning 123/4 shortcut
            }

            int16_t numeratorLocation = HP32SII ? 1 : strchr(aimBuffer, '.') - aimBuffer + 1;
            int32_t numerator = toInt32(aimBuffer + numeratorLocation);
            if (numerator < 0 || numerator > 999999999) {
              break;
            }

            if(HP32SII) {     //Changed the part below for 123..4 to mean 0 123/4
              aimBuffer[strlen(aimBuffer) - 1] = 0;
            }

            else { //!HP32SII .. situation (standard code)
              aimBuffer[numeratorLocation - 1] = ' ';
            }

            strcat(aimBuffer, "/");

            denominatorLocation = strlen(aimBuffer);
            nimNumberPart = HP32SII ? NP_HP32SII_DENOMINATOR : NP_FRACTION_DENOMINATOR;
            //debugNIM();
            break;
          }

          case NP_COMPLEX_INT_PART: {
            strcat(aimBuffer, ".");

            nimNumberPart = NP_COMPLEX_FLOAT_PART;
            //debugNIM();
            break;
          }

          case NP_COMPLEX_FLOAT_PART: {
            bool_t HP32SII = false;
            if(aimBuffer[strlen(aimBuffer) - 1] == '.') {
              HP32SII = true; //This is the 123 .. 4 meaning 123/4 shortcut
            }

            int16_t numeratorLocation = HP32SII ? imaginaryMantissaSignLocation + 2 : strchr(aimBuffer + imaginaryMantissaSignLocation + 2, '.') - aimBuffer + 1;
            int32_t numerator = toInt32(aimBuffer + numeratorLocation);
            if (numerator < 0 || numerator > 999999999) {
              break;
            }

            if(HP32SII) {     //Changed the part below for 123..4 to mean 0 123/4
              aimBuffer[strlen(aimBuffer) - 1] = 0;
            }

            else { //!HP32SII .. situation (standard code)
              aimBuffer[numeratorLocation - 1] = ' ';
            }

            strcat(aimBuffer, "/");

            imaginaryDenominatorLocation = strlen(aimBuffer);
            nimNumberPart = HP32SII ? NP_COMPLEX_HP32SII_DENOMINATOR : NP_COMPLEX_FRACTION_DENOMINATOR;
            //debugNIM();
            break;
          }

          default: {
          }
        }
        break;
      }

      case ITM_EXPONENT: {
        if(INTEGERSHORTCUTS && nimNumberPart == NP_INT_BASE && aimBuffer[strlen(aimBuffer) - 1] == '#') { //JM "BASE OCT" See below
          strcat(aimBuffer, "8");
          goto addItemToNimBuffer_exit;
        }

        done = true;

        if(aimBuffer[strlen(aimBuffer)-1] == 'i') {
          strcat(aimBuffer, "1");
        }

        setLastintegerBasetoZero();

        switch(nimNumberPart) {
          case NP_INT_10: {
            strcat(aimBuffer, "."); // no break here
            #if !defined(OSX)
              __attribute__ ((fallthrough));
            #endif // !OSX
          }
          case NP_REAL_FLOAT_PART: {
            strcat(aimBuffer, "e+");
            exponentSignLocation = strlen(aimBuffer) - 1;

            nimNumberPart = NP_REAL_EXPONENT;
            //debugNIM();
            break;
          }
          case NP_COMPLEX_INT_PART: {
            strcat(aimBuffer, "."); // no break here
            #if !defined(OSX)
              __attribute__ ((fallthrough));
            #endif // !OSX
          }
          case NP_COMPLEX_FLOAT_PART: {
            strcat(aimBuffer, "e+");
            imaginaryExponentSignLocation = strlen(aimBuffer) - 1;

            nimNumberPart = NP_COMPLEX_EXPONENT;
            //debugNIM();
            break;
          }
          default: {
          }
        }
        break;
      }

      case ITM_HASH_JM:
      case ITM_toINT: { // #
        done = true;

        setLastintegerBasetoZero();

        if(nimNumberPart == NP_INT_10 || nimNumberPart == NP_INT_16) {
          strcat(aimBuffer, "#");

          nimNumberPart = NP_INT_BASE;
          //debugNIM();
        }
        break;
      }

      case ITM_CHS: { // +/-
        done = true;

        switch(nimNumberPart) {
          case NP_INT_10 :
          case NP_INT_16 :
          case NP_INT_BASE :
          case NP_REAL_FLOAT_PART :
          case NP_HP32SII_DENOMINATOR:
          case NP_FRACTION_DENOMINATOR: {
            if(aimBuffer[0] == '+') {
              aimBuffer[0] = '-';
            }
            else {
              aimBuffer[0] = '+';
            }
            break;
          }

          case NP_REAL_EXPONENT: {
            if(aimBuffer[exponentSignLocation] == '+') {
              aimBuffer[exponentSignLocation] = '-';
              if(aimBuffer[exponentSignLocation + 1] == '0') {
                aimBuffer[strlen(aimBuffer) - 1] = 0;
              }
            }
            else {
              aimBuffer[exponentSignLocation] = '+';
            }
            break;
          }

          case NP_COMPLEX_INT_PART :
          case NP_COMPLEX_FLOAT_PART:
          case NP_COMPLEX_HP32SII_DENOMINATOR:
          case NP_COMPLEX_FRACTION_DENOMINATOR: {
            if(aimBuffer[imaginaryMantissaSignLocation] == '+') {
              aimBuffer[imaginaryMantissaSignLocation] = '-';
            }
            else {
              aimBuffer[imaginaryMantissaSignLocation] = '+';
            }
            break;
          }

          case NP_COMPLEX_EXPONENT: {
            if(aimBuffer[imaginaryExponentSignLocation] == '+') {
              aimBuffer[imaginaryExponentSignLocation] = '-';
              if(aimBuffer[imaginaryExponentSignLocation + 1] == '0') {
                aimBuffer[strlen(aimBuffer) - 1] = 0;
              }
            }
            else {
              aimBuffer[imaginaryExponentSignLocation] = '+';
            }
            break;
          }

          default: {
          }
        }
        break;
      }


        case ITM_op_j_pol :
        case ITM_op_j :                         //JM HP35s compatible, in NIM
        case ITM_CC: {
        if(item == ITM_op_j || item == ITM_op_j_pol) {
          resetShiftState();    //JM HP35s compatible, in NIM
        }
        lastChar = strlen(aimBuffer) - 1;

        done = true;

        setLastintegerBasetoZero();

        switch(nimNumberPart) {
          case NP_REAL_EXPONENT: {
            if((aimBuffer[lastChar] == '+' || aimBuffer[lastChar] == '-') && aimBuffer[lastChar - 1] == 'e') {
              aimBuffer[lastChar - 1] = 0;
            }
            else if(aimBuffer[lastChar] == 'e') {
              aimBuffer[lastChar] = 0;
            }
            else {
              imaginaryMantissaSignLocation = strlen(aimBuffer);
              strcat(aimBuffer, "+i");

              nimRealPart = nimNumberPart;
              nimNumberPart = NP_COMPLEX_INT_PART;
              //debugNIM();
            }
            break;
          }

          case NP_INT_10: {
            strcat(aimBuffer, "."); // no break here
            #if !defined(OSX)
              __attribute__ ((fallthrough));
            #endif // !OSX
          }

          case NP_REAL_FLOAT_PART:
          case NP_FRACTION_DENOMINATOR:
          case NP_HP32SII_DENOMINATOR: {
            imaginaryMantissaSignLocation = strlen(aimBuffer);
            strcat(aimBuffer, "+i");

            nimRealPart = nimNumberPart;
            nimNumberPart = NP_COMPLEX_INT_PART;
            //debugNIM();
            break;
          }
          default: {
          }
        }
        break;
      }

      case ITM_CONSTpi: {
        if(nimNumberPart == NP_COMPLEX_INT_PART && aimBuffer[strlen(aimBuffer) - 1] == 'i') {
          done = true;
          strcat(aimBuffer, "3.141592653589793238462643383279503");
          reallyRunFunction(ITM_EXIT1, NOPARAM);
          setLastintegerBasetoZero();
        }
        break;
      }

      case ITM_BACKSPACE: {
        lastChar = strlen(aimBuffer) - 1;

        done = true;

        switch(nimNumberPart) {
          case NP_INT_10: {
            break;
          }

          case NP_INT_16: {
            if(aimBuffer[lastChar] >= 'A') {
              hexDigits--;
            }

            if(hexDigits == 0) {
              nimNumberPart = NP_INT_10;
              //debugNIM();
            }
            break;
          }

          case NP_INT_BASE: {
            if(aimBuffer[lastChar] == '#') {
              if(hexDigits > 0) {
                nimNumberPart = NP_INT_16;
              }
              else {
                nimNumberPart = NP_INT_10;
              }
              //debugNIM();
            }
            break;
          }

          case NP_REAL_FLOAT_PART: {
            if(aimBuffer[lastChar] == '.') {
              nimNumberPart = NP_INT_10;
              //debugNIM();
            }
            break;
          }

          case NP_REAL_EXPONENT: {
            if(aimBuffer[lastChar] == '+' || aimBuffer[lastChar] == '-') {
              aimBuffer[lastChar--] = 0;
            }

            if(aimBuffer[lastChar] == 'e') {
              nimNumberPart = NP_INT_10;
              for(int16_t i=1; i<lastChar; i++) {
                if(aimBuffer[i] == '.') {
                  nimNumberPart = NP_REAL_FLOAT_PART;
                  break;
                }
              }
              //debugNIM();
            }
            break;
          }

          case NP_HP32SII_DENOMINATOR: {
            if(aimBuffer[lastChar] == '/') {
              nimNumberPart = NP_REAL_FLOAT_PART;
              aimBuffer[lastChar++] = '.';
              //debugNIM();
            }
            break;
          }

          case NP_FRACTION_DENOMINATOR: {
            if(aimBuffer[lastChar] == '/') {
              nimNumberPart = NP_REAL_FLOAT_PART;
              for(int16_t i=1; i<lastChar; i++) {
                if(aimBuffer[i] == ' ') {
                  aimBuffer[i] = '.';
                  break;
                }
              }
              //debugNIM();
            }
            break;
          }

          case NP_COMPLEX_INT_PART: {
            if(aimBuffer[lastChar] == 'i') {
              nimNumberPart = nimRealPart;
              //debugNIM();
              lastChar--;
            }
            break;
          }

          case NP_COMPLEX_FLOAT_PART: {
            if(aimBuffer[lastChar] == '.') {
              nimNumberPart = NP_COMPLEX_INT_PART;
              //debugNIM();
            }
            break;
          }

          case NP_COMPLEX_EXPONENT: {
            if(aimBuffer[lastChar] == '+' || aimBuffer[lastChar] == '-') {
              aimBuffer[lastChar--] = 0;
            }

            if(aimBuffer[lastChar] == 'e') {
              nimNumberPart = NP_COMPLEX_INT_PART;
              for(int16_t i=imaginaryMantissaSignLocation+2; i<lastChar; i++) {
                if(aimBuffer[i] == '.') {
                  nimNumberPart = NP_COMPLEX_FLOAT_PART;
                  break;
                }
              }
              //debugNIM();
            }
            break;
          }

          case NP_COMPLEX_HP32SII_DENOMINATOR: {
            if(aimBuffer[lastChar] == '/') {
              nimNumberPart = NP_COMPLEX_FLOAT_PART;
              aimBuffer[lastChar++] = '.';
              //debugNIM();
            }
            break;
          }

          case NP_COMPLEX_FRACTION_DENOMINATOR: {
            if(aimBuffer[lastChar] == '/') {
              nimNumberPart = NP_COMPLEX_FLOAT_PART;
              for(int16_t i=imaginaryMantissaSignLocation+2; i<lastChar; i++) {
                if(aimBuffer[i] == ' ') {
                  aimBuffer[i] = '.';
                  break;
                }
              }
              //debugNIM();
            }
            break;
          }

          default: {
          }
        }

        aimBuffer[lastChar--] = 0;

        if((calcMode != CM_MIM) && (lastChar == -1 || (lastChar == 0 && aimBuffer[0] == '+'))) {
          screenUpdatingMode &= ~SCRUPD_SKIP_STACK_ONE_TIME;
          calcModeNormal();
          #if defined(DEBUGUNDO)
            printf(">>> undo from addItemToNimBuffer\n");
          #endif // DEBUGUNDO
          undo();
        }
        break;
      }

      case ITM_EXIT1: {
        addItemToNimBuffer_exit:
        done = true;
        screenUpdatingMode &= ~SCRUPD_SKIP_STACK_ONE_TIME;
        closeNim();
        if(calcMode != CM_NIM && lastErrorCode == 0) {
          setSystemFlag(FLAG_ASLIFT);
          if(item == ITM_EXIT1) {
            #if defined(DEBUGUNDO)
              printf(">>> saveForUndo from bufferizeA:");
            #endif // DEBUGUNDO
            saveForUndo();
            if(lastErrorCode == ERROR_RAM_FULL) {
              lastErrorCode = 0;
              temporaryInformation = TI_UNDO_DISABLED;
              #if (EXTRA_INFO_ON_CALC_ERROR == 1)
                moreInfoOnError("In function addItemToNimBuffer:", "there is not enough memory to save for undo!", NULL, NULL);
              #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
            }
          }
          return;
        }
        if(item == ITM_EXIT1) {
          #if defined(DEBUGUNDO)
            printf(">>> saveForUndo from bufferizeB:");
          #endif // DEBUGUNDO
          saveForUndo();
          if(lastErrorCode == ERROR_RAM_FULL) {
            lastErrorCode = 0;
            temporaryInformation = TI_UNDO_DISABLED;
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              moreInfoOnError("In function addItemToNimBuffer:", "there is not enough memory to save for undo!", NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          }
        }
        break;
      }

      //  key C47                                //R47                       //BASE mode shortcuts
      //  1:  B = ITM_1ONX            = BIN      B = ITM_SQUAREROOTX
      //  3:  D = ITM_LOG = ITM_ENTER = DEC      D = ITM_YX
      //  7:  H = ITM_RCL             = HEX      H = ITM_RCL
      //      O = ITM_EXPONENT        = OCT      ---
      //  8:  I = ITM_Rdown           = INT      I = ITM_Rdown

      // F not active in NIM, per definition not possible if a period is in the input string.

      //JM Only works in direct NIM, that is only when the input buffer already contains #
      case ITM_1ONX:          // C47: B for binary base
      case ITM_SQUAREROOTX: { // R47:
        if((!isR47FAM && item == ITM_SQUAREROOTX) || (isR47FAM && item == ITM_1ONX)) {
          keyActionProcessed = false;
          break;
        }
        if(INTEGERSHORTCUTS && nimNumberPart == NP_INT_BASE && aimBuffer[strlen(aimBuffer) - 1] == '#') {
          strcat(aimBuffer, "2");
          goto addItemToNimBuffer_exit;
        }
        else {
          keyActionProcessed = false;
        }
        break;
      }

      case ITM_ENTER:       // D: for decimal base          //JM
      case ITM_LOG10:       // C47
      case ITM_YX: {        // R47
        if((!isR47FAM && item == ITM_YX) || (isR47FAM && item == ITM_LOG10)) {
          keyActionProcessed = false;
          break;
        }
        if(INTEGERSHORTCUTS && nimNumberPart == NP_INT_BASE && aimBuffer[strlen(aimBuffer) - 1] == '#') {
          strcat(aimBuffer, "10");
          goto addItemToNimBuffer_exit;
        }
        else {
          keyActionProcessed = false;
        }
        break;
      }

      case ITM_RCL: { // H for hexadecimal base
        if(INTEGERSHORTCUTS && nimNumberPart == NP_INT_BASE && aimBuffer[strlen(aimBuffer) - 1] == '#') {
          strcat(aimBuffer, "16");
          goto addItemToNimBuffer_exit;
        }
        else {
          keyActionProcessed = false;
        }
        break;
      }

      case ITM_Rdown: { // I for longinteger
        if(INTEGERSHORTCUTS && nimNumberPart == NP_INT_BASE && aimBuffer[strlen(aimBuffer) - 1] == '#') {
          aimBuffer[strlen(aimBuffer)-1]=0;
          nimNumberPart = NP_INT_10;
          goto addItemToNimBuffer_exit;
        }
        else {
          keyActionProcessed = false;
        }
        break;
      }

      //JM See abovr "BASE OCT", O for octal base: (INTEGERSHORTCUTS)



      case ITM_DMS: {
        if(nimNumberPart == NP_INT_10 || nimNumberPart == NP_REAL_FLOAT_PART) {
          done = true;
          setLastintegerBasetoZero();

          screenUpdatingMode &= ~SCRUPD_SKIP_STACK_ONE_TIME;
          closeNim();
          if(calcMode != CM_NIM && lastErrorCode == 0) {
            if(getRegisterDataType(REGISTER_X) == dtLongInteger) {
              convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
            }

            real34FromDmsToDeg(REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
            setRegisterAngularMode(REGISTER_X, amDMS);

            setSystemFlag(FLAG_ASLIFT);
            return;
          }
        }
        break;
      }

      case ITM_dotD: {
        angularMode_t xangularMode;
        xangularMode = ((getRegisterDataType(REGISTER_X) == dtReal34) == dtReal34 ? getRegisterAngularMode(REGISTER_X) : amNone);

        if(xangularMode < amNone) {  // If editing with angular mode, then cancel angular mode
          xangularMode = amNone;
        }
        else if(nimNumberPart == NP_REAL_FLOAT_PART) {
          done = true;

          screenUpdatingMode &= ~SCRUPD_SKIP_STACK_ONE_TIME;

          //Accommodate 2-digit xx.xxYY, and change to xx.xx00YY
          int16_t tmplen = stringByteLength(aimBuffer);
          if(!(lastCenturyHighUsed & 0x8000) && !getSystemFlag(FLAG_YMD) && (
               (tmplen == 8 && (isValidNumber(aimBuffer, "sdd.dddd")))                                //+11.1123
            || (tmplen == 7 && (isValidNumber(aimBuffer, "sd.dddd")))                                 // +1.1123  +1.1120
             )) {
            aimBuffer[tmplen] = aimBuffer[tmplen - 2];
            aimBuffer[tmplen+1] = aimBuffer[tmplen - 1];
            aimBuffer[tmplen - 2] = '0';
            aimBuffer[tmplen - 1] = '0';
            aimBuffer[tmplen + 2] = 0;
          }

          closeNim();
          if(calcMode != CM_NIM && lastErrorCode == 0 && getRegisterDataType(REGISTER_X) != dtDate) {
            convertReal34RegisterToDateRegister(REGISTER_X, REGISTER_X, YYSystem);
            checkDateRange(REGISTER_REAL34_DATA(REGISTER_X));
            temporaryInformation = TI_DAY_OF_WEEK;

            if(lastErrorCode == 0) {
              setSystemFlag(FLAG_ASLIFT);
            }
            else {
              #if defined(DEBUGUNDO)
                printf(">>> undo from addItemToNimBufferB\n");
              #endif // DEBUGUNDO
              undo();
            }
            return;
          }
        }
        else {                      //JM
          done = true;              //JM
          closeNim();               //JM
        }
        break;
      }

      case ITM_ms : {                      //JM
        if(nimNumberPart == NP_INT_10 || nimNumberPart == NP_REAL_FLOAT_PART || nimNumberPart == NP_REAL_EXPONENT) {
          done = true;
          setLastintegerBasetoZero();

          screenUpdatingMode &= ~SCRUPD_SKIP_STACK_ONE_TIME;
          closeNim();
          if(calcMode != CM_NIM && lastErrorCode == 0 && getRegisterDataType(REGISTER_X) != dtTime) {
            if(getRegisterDataType(REGISTER_X) == dtLongInteger) {
              convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
            }

            hmmssInRegisterToSeconds(REGISTER_X);
            if(lastErrorCode == 0) {
              setSystemFlag(FLAG_ASLIFT);
            }
            else {
              #if defined(DEBUGUNDO)
                printf(">>> undo from addItemToNimBufferC\n");
              #endif // DEBUGUNDO
              undo();
            }
            return;
          }
        }
        break;
      }

      case ITM_DMS2:{                       //JM
        if(nimNumberPart == NP_INT_10 || nimNumberPart == NP_REAL_FLOAT_PART || nimNumberPart == NP_REAL_EXPONENT) {
          done = true;
          closeNim();
          fnAngularModeJM(amDMS); //it cannot be an angle at this point. If closed input, it is only real or longint
        }
        break;
      }

      case ITM_DRG:{                       //JM
        DRG_Cycling = 0;
        if(nimNumberPart == NP_INT_10 || nimNumberPart == NP_REAL_FLOAT_PART || nimNumberPart == NP_REAL_EXPONENT) {
          done = true;

          closeNim();

          if(calcMode != CM_NIM && lastErrorCode == 0) {
            copySourceRegisterToDestRegister(REGISTER_X, TEMP_REGISTER_1);
            if(getRegisterDataType(REGISTER_X) == dtLongInteger) {
              convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
            }
            if(getRegisterDataType(REGISTER_X) == dtReal34 && getRegisterAngularMode(REGISTER_X) == amNone) {
              if(currentAngularMode == amDMS) fnCvtFromCurrentAngularMode(amDMS); else
                if(currentAngularMode == amMultPi) fnCvtFromCurrentAngularMode(amMultPi); else
                  setRegisterAngularMode(REGISTER_X, currentAngularMode);
            }

            if(lastErrorCode == 0) {
              setSystemFlag(FLAG_ASLIFT);
            }
            else {
              undo();
            }
            copySourceRegisterToDestRegister(TEMP_REGISTER_1, REGISTER_L);
            return;
          }
        }
        break;
      }

      case ITM_NOP: {   // NOP: do nothing in NIM
        break;
      }

      default: {
        keyActionProcessed = false;
      }
    }

    if(done) {
      //Convert nimBuffer to display string
      strcpy(nimBufferDisplay, STD_SPACE_HAIR);

      switch(nimNumberPart) {
        case NP_INT_10 :         // +12345
        case NP_INT_16 :         // +123AB
        case NP_INT_BASE: { // +123AB#16
          nimBufferToDisplayBuffer(aimBuffer, nimBufferDisplay + 2);
          break;
        }

        case NP_REAL_FLOAT_PART: { // +12345.6789
          nimBufferToDisplayBuffer(aimBuffer, nimBufferDisplay + 2);
          break;
        }

        case NP_REAL_EXPONENT: { // +12345.678e+3
          nimRealToDisplayBuffer(aimBuffer, exponentSignLocation, nimBufferDisplay + 2);
          break;
        }

        case NP_HP32SII_DENOMINATOR:
        case NP_FRACTION_DENOMINATOR: { // +123 12/7
          nimFractionToDisplayBuffer(aimBuffer, nimBufferDisplay + 2);
          break;
        }

        case NP_COMPLEX_INT_PART :  // +1.2+i15
        case NP_COMPLEX_FLOAT_PART :// +1.2+i15.69
        case NP_COMPLEX_EXPONENT:   // +1.2+i15.69e2
        case NP_COMPLEX_HP32SII_DENOMINATOR:    // +1.2+i123/7
        case NP_COMPLEX_FRACTION_DENOMINATOR: { // +1.2+i123 12/7
            // Real part
          savedNimNumberPart = nimNumberPart;
          nimNumberPart = nimRealPart;

          if (nimNumberPart == NP_FRACTION_DENOMINATOR || nimNumberPart == NP_HP32SII_DENOMINATOR) {
            nimFractionToDisplayBuffer(aimBuffer, nimBufferDisplay + 2);
          }
          else {
            nimRealToDisplayBuffer(aimBuffer, exponentSignLocation, nimBufferDisplay + 2);
          }

          nimNumberPart = savedNimNumberPart;
          // Complex "separator"
          if((getSystemFlag(FLAG_POLAR) && !temporaryFlagRect) || temporaryFlagPolar) { // polar mode
            strcat(nimBufferDisplay, STD_SPACE_4_PER_EM STD_MEASURED_ANGLE STD_SPACE_4_PER_EM);
            if(aimBuffer[imaginaryMantissaSignLocation] == '-') {
              strcat(nimBufferDisplay, "-");
            }
          }
          else if((!getSystemFlag(FLAG_POLAR) && !temporaryFlagPolar) || temporaryFlagRect) { // rectangular mode
            if(strncmp(nimBufferDisplay + stringByteLength(nimBufferDisplay) - 2, STD_SPACE_HAIR, 2) != 0) {
              strcat(nimBufferDisplay, STD_SPACE_HAIR);
            }

            if(aimBuffer[imaginaryMantissaSignLocation] == '-') {
              strcat(nimBufferDisplay, "-");
            }
            else {
              strcat(nimBufferDisplay, "+");
            }
            strcat(nimBufferDisplay, COMPLEX_UNIT);
            strcat(nimBufferDisplay, PRODUCT_SIGN);
          }

          // Imaginary part
          if(aimBuffer[imaginaryMantissaSignLocation+2] != 0) {
            if (nimNumberPart == NP_COMPLEX_FRACTION_DENOMINATOR || nimNumberPart == NP_COMPLEX_HP32SII_DENOMINATOR) {
              nimFractionToDisplayBuffer(aimBuffer + imaginaryMantissaSignLocation + 1, nimBufferDisplay + stringByteLength(nimBufferDisplay));
            }
            else {
              nimRealToDisplayBuffer(aimBuffer + imaginaryMantissaSignLocation + 1, imaginaryExponentSignLocation - imaginaryMantissaSignLocation - 1, nimBufferDisplay + stringByteLength(nimBufferDisplay));
            }
          }
          break;
        }

        default: {
          sprintf(errorMessage, "In function addItemToNimBuffer: %d is an unexpected nimNumberPart value while converting buffer to display!", nimNumberPart);
          displayBugScreen(errorMessage);
        }
      }
      uint16_t _len = stringByteLength(nimBufferDisplay);
      if(RADIX34_MARK_STRING[0] != '.') {
        for(index=_len - 1; index>0; index--) {
          if(nimBufferDisplay[index] == '.') {
            nimBufferDisplay[index] = RADIX34_MARK_STRING[0];
            if(RADIX34_MARK_STRING[1] != 1) {
              uint16_t index2=_len;
              while(index2 >= index) {
                nimBufferDisplay[index2+1] = nimBufferDisplay[index2];
                index2--;
              }
              nimBufferDisplay[index+1] = RADIX34_MARK_STRING[1];
            }
          }
        }
      }
      for(index=stringByteLength(nimBufferDisplay) - 1; index>0; index--) {
        if(nimBufferDisplay[index] == (char)0xab) {
          nimBufferDisplay[index] = SEPARATOR_LEFT[0];
          if(nimBufferDisplay[index+1] == 1) {
            nimBufferDisplay[index+1] = SEPARATOR_LEFT[1];
          }
        }
        if(nimBufferDisplay[index] == (char)0xbb) {
          nimBufferDisplay[index] = SEPARATOR_RIGHT[0];
          if(nimBufferDisplay[index+1] == 1) {
            nimBufferDisplay[index+1] = SEPARATOR_RIGHT[1];
          }
        }
      }
    }

    else if(item != ITM_NOP) {
      #if defined (PC_BUILD) && defined(VERBOSE_MINIMUM)
        printf("addItemToNimBuffer: delayCloseNim=%u\n",delayCloseNim);
        fflush(stdout);
      #endif
      if(!delayCloseNim) {      //delayCloseNim can only be activaed by ITM.ms in bufferize
        switch(item) {          //JMCLOSE remove auto closenim directly after KEY PRESSED for these functions only.
          case ITM_HASH_JM:     //closeNim simply not needed because we need to type the base while NIM remains open, and the BASE, INTS and BITS A-F and HEX/DEC commands are active on NIM
          case ITM_toINT:
          case -MNU_BASE:
          case -MNU_INTS:
          case -MNU_BITS: {
            break;
          }
          default:
            screenUpdatingMode &= ~SCRUPD_SKIP_STACK_ONE_TIME;
            closeNim();
        }
      }
      if(calcMode != CM_NIM) {
        if(item == ITM_CONSTpi || (item >= 0 && indexOfItems[item].func == fnConstant)) {
          setSystemFlag(FLAG_ASLIFT);
          setLastintegerBasetoZero();
        }

        if(lastErrorCode == 0) {
          showFunctionName(item, 1000, NULL); // 1000ms = 1s
          // Fix: removed "showFunctionName(item, 1000, "SF:Q"); // 1000ms = 1s" and replaced with "keyActionProcessed = false;"
          //      removed the local action, just let it go back unprocessed,
          //      and the showFunctionName is called after this in keyboard.c, with the correct parameters.
          // ReFix: Removed previous fix "keyActionProcessed = false;" and restored "showFunctionName" but with NULL parameter.
        }
      }
    }
  }


  static int16_t insertGapIP(char *displayBuffer, int16_t numDigits, int16_t nth) {
    if(GROUPLEFT_DISABLED) {
      return 0; // no gap when none is required!
    }
    if(numDigits <= GROUPWIDTH_LEFT) {
      return 0; // there are less than GROUPWIDTH_LEFT digits
    }
    if(nth + 1 == numDigits) {
      return 0; // no gap after the last digit
    }

    if((numDigits - nth) % GROUPWIDTH_LEFT == 1 || GROUPWIDTH_LEFT == 1) {
      char tt[4];
      if(SEPARATOR_LEFT[1]!=1) {
        //strcpy(tt,SEPARATOR_LEFT);
        tt[0] = 0xab;  //token
        tt[1] = 1;
        tt[2] = 0;
        strcpy(displayBuffer, tt);
        return 2;
      }
      else {
        //tt[0] = SEPARATOR_LEFT[0]; tt[1] = 0;
        tt[0] = 0xab;  //token
        tt[1] = 0;
        strcpy(displayBuffer, tt);
        return 1;
      }
    }

    return 0;
  }


  static int16_t insertGapFP(char *displayBuffer, int16_t numDigits, int16_t nth) {
    if(GROUPRIGHT_DISABLED) {
      return 0; // no gap when none is required!
    }
    if(numDigits <= GROUPWIDTH_RIGHT) {
      return 0; // there are less than GROUPWIDTH_LEFT digits
    }
    if(nth + 1 == numDigits) {
      return 0; // no gap after the last digit
    }

    if(nth % GROUPWIDTH_RIGHT == GROUPWIDTH_RIGHT - 1) {
      char tt[4];
      if(SEPARATOR_RIGHT[1]!=1) {
        //strcpy(tt,SEPARATOR_RIGHT);
        tt[0] = 0xbb;   //token
        tt[1] = 1;
        tt[2] = 0;
        strcpy(displayBuffer, tt);
        return 2;
      }
      else {
        //tt[0] = SEPARATOR_RIGHT[0]; tt[1] = 0;
        tt[0] = 0xbb;  //token
        tt[1] = 0;
        strcpy(displayBuffer, tt);
        return 1;
      }
    }

    return 0;
  }


  void nimBufferToDisplayBuffer(const char *buffer, char *displayBuffer) {
    int16_t numDigits, source, dest;

    if(*buffer == '-') {
      *(displayBuffer++) = '-';
    }
    buffer++;

    int16_t GROUPWIDTH_LEFTM = GROUPWIDTH_LEFT;                       //JMGAP vv
    switch(nimNumberPart) {
      case NP_INT_10:                    // +12345 longint; Do not change GROUPWIDTH_LEFT. Leave at user setting, default 3.
      case NP_INT_BASE:                  // +123AB#16.    ; Change GROUPWIDTH_LEFT from user selection to this table, for entry
        switch(lastIntegerBase) {
          case  0: GROUPWIDTH_LEFT = GROUPWIDTH_LEFTM; break;
          case  2: GROUPWIDTH_LEFT = 4; break;
          case  3: GROUPWIDTH_LEFT = 3; break;
          case  4: GROUPWIDTH_LEFT = 2; break;
          case  5:
          case  6:
          case  7:
          case  8:
          case  9:
          case 10:
          case 11:
          case 12:
          case 13:
          case 14:
          case 15: GROUPWIDTH_LEFT = 3; break;
          case 16: GROUPWIDTH_LEFT = 2; break;
          default: ;
        }
        break;
      case NP_INT_16:                    // +123AB.       ; Change to 2 for hex.
        GROUPWIDTH_LEFT = 2;
        break;
      default: ;
    }                                                         //JMGAP ^^

    for(numDigits=0; buffer[numDigits]!=0 && buffer[numDigits]!='e' && buffer[numDigits]!='.' && buffer[numDigits]!=' ' && buffer[numDigits]!='#' && buffer[numDigits]!='+' && buffer[numDigits]!='-'; numDigits++) {
    }

    for(source=0, dest=0; source<numDigits; source++) {
      displayBuffer[dest++] = buffer[source];
      dest += insertGapIP(displayBuffer + dest, numDigits, source);
    }

    GROUPWIDTH_LEFT = GROUPWIDTH_LEFTM;                               //JMGAP
    displayBuffer[dest] = 0;

    if(nimNumberPart == NP_REAL_FLOAT_PART || nimNumberPart == NP_REAL_EXPONENT || nimNumberPart == NP_COMPLEX_FLOAT_PART || nimNumberPart == NP_COMPLEX_EXPONENT) {
      displayBuffer[dest++] = '.';

      buffer += source + 1;

      for(numDigits=0; buffer[numDigits]!=0 && buffer[numDigits]!='e' && buffer[numDigits]!='+' && buffer[numDigits]!='-'; numDigits++) {
      }

      for(source=0; source<numDigits; source++) {
        displayBuffer[dest++] = buffer[source];
        dest += insertGapFP(displayBuffer + dest, numDigits, source);
      }

      displayBuffer[dest] = 0;
    }

    else if(nimNumberPart == NP_INT_BASE) {
      strcpy(displayBuffer + dest, buffer + numDigits);
    }
  }

  void nimFractionToDisplayBuffer(const char *buffer, char *displayBuffer) {
    int16_t index;

    if (nimNumberPart == NP_FRACTION_DENOMINATOR || nimNumberPart == NP_COMPLEX_FRACTION_DENOMINATOR) {
      nimBufferToDisplayBuffer(buffer, displayBuffer);
      strcat(displayBuffer, STD_SPACE_4_PER_EM);

      for(index=2; buffer[index]!=' '; index++) {
      }
    }
    else {
      if (buffer[0] == '-') {
        strcat(displayBuffer, "-");
      }
      index = 0;
    }

    supNumberToDisplayString(toInt32(buffer + index + 1), displayBuffer + stringByteLength(displayBuffer), NULL, true);

    strcat(displayBuffer, "/");

    for(; buffer[index]!='/'; index++) {
    }
    if (buffer[++index] == '+') { // There is an imaginary part
      subNumberToDisplayString(lastDenominator, displayBuffer + stringByteLength(displayBuffer), NULL);
    }
    else if (buffer[index] != 0) {
      int32_t denominator = toInt32(buffer + index);
      subNumberToDisplayString(denominator, displayBuffer + stringByteLength(displayBuffer), NULL);
      for(; buffer[index] >= '0' && buffer[index] <= '9'; index++) {
      }
      if (buffer[index] == '+') {
        lastDenominator = denominator;
      }
    }
  }

  void nimFractionToReal34(char* source, real34_t *dest) {
    int16_t i, posSpace, posSlash, lg;
    int32_t integer, numer, denom;
    real34_t temp;

    setSystemFlag(getSystemFlag(FLAG_IRFRQ) ? FLAG_IRFRAC : FLAG_FRACT);          //1     //NOTE CHANGE HERE TO SWITCH OFF AUTO FRAC MODE AFTER FRACTION INPUT

    lg = strlen(source);

    posSpace = 0;
    for(i=2; i<lg; i++) {
      if(source[i] == ' ') {
        posSpace = i;
        break;
      }
    }

    for(i=1; i<posSpace; i++) {
      if(source[i]<'0' || source[i]>'9') { // This should never happen
        displayCalcErrorMessage(ERROR_BAD_INPUT, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function nimFractionToReal34:", "there is a non numeric character in the integer part of the fraction!", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return;
      }
    }

    posSlash = 0;
    for(i=posSpace+2; i<lg; i++) {
      if(source[i] == '/') {
        posSlash = i;
        break;
      }
    }

    for(i=posSpace+1; i<posSlash; i++) {
      if(source[i]<'0' || source[i]>'9') { // This should never happen
       displayCalcErrorMessage(ERROR_BAD_INPUT, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
       #if (EXTRA_INFO_ON_CALC_ERROR == 1)
         moreInfoOnError("In function nimFractionToReal34:", "there is a non numeric character in the numerator part of the fraction!", NULL, NULL);
       #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
       return;
      }
    }

    if(source[posSlash + 1] != 0) {
      for(i=posSlash+1; i<lg; i++) {
        if(source[i]<'0' || source[i]>'9') {
          displayCalcErrorMessage(ERROR_BAD_INPUT, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            moreInfoOnError("In function nimFractionToReal34:", "there is a non numeric character in the denominator part of the fraction!", NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          return;
        }
      }
    }

    if (posSpace != 0) {
      source[posSpace] = 0;
      integer = toInt32(source + 1);
    }
    else {
      integer = 0;
    }
    source[posSlash] = 0;
    numer   = toInt32(source + posSpace + 1);
    if(source[posSlash + 1] == 0) {
      denom = lastDenominator;
    }
    else {
      denom = toInt32(source + posSlash + 1);
      if(denom != 0) {
        lastDenominator = denom;
      }
    }

    if(denom == 0 && !getSystemFlag(FLAG_SPCRES)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function nimFractionToReal34:", "the denominator of the fraction should not be 0!", "Unless D flag (Danger) is set.", NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }

    int32ToReal34(numer, dest);
    int32ToReal34(denom, &temp);
    real34Divide(dest, &temp, dest);
    int32ToReal34(integer, &temp);
    real34Add(dest, &temp, dest);
    if(source[0] == '-') {
      real34SetNegativeSign(dest);
    }
  }

  void nimRealToDisplayBuffer(const char *buffer, int16_t exponentLocation, char *displayBuffer) {
    nimBufferToDisplayBuffer(buffer, displayBuffer);

    if (nimNumberPart == NP_REAL_EXPONENT || nimNumberPart == NP_COMPLEX_EXPONENT) {
      exponentToDisplayString(stringToInt32(buffer + exponentLocation), displayBuffer + stringByteLength(displayBuffer), NULL, true);
      if(buffer[exponentLocation + 1] == 0 && buffer[exponentLocation] == '-') {
        strcat(displayBuffer, STD_SUP_MINUS);
      }
      else if(buffer[exponentLocation + 1] == '0' && buffer[exponentLocation] == '+') {
        strcat(displayBuffer, STD_SUP_0);
      }
    }
  }

  void closeNimWithFraction(real34_t *dest) {
    // Set Fraction mode
    nimFractionToReal34(aimBuffer, dest);
  }


  void closeNimWithComplex(real34_t *dest_r, real34_t *dest_i) {
    aimBuffer[imaginaryMantissaSignLocation+1] = aimBuffer[imaginaryMantissaSignLocation];
    aimBuffer[imaginaryMantissaSignLocation] = 0;

    if (nimRealPart == NP_FRACTION_DENOMINATOR || nimRealPart == NP_HP32SII_DENOMINATOR) {
      nimFractionToReal34(aimBuffer, dest_r);
    }
    else {
      stringToReal34(aimBuffer, dest_r);
    }

    if (nimNumberPart == NP_COMPLEX_FRACTION_DENOMINATOR || nimNumberPart == NP_COMPLEX_HP32SII_DENOMINATOR) {
      nimFractionToReal34(aimBuffer + imaginaryMantissaSignLocation + 1, dest_i);
    }
    else {
      stringToReal34(aimBuffer + imaginaryMantissaSignLocation + 1, dest_i);
    }

    if((getSystemFlag(FLAG_POLAR) && !temporaryFlagRect) || temporaryFlagPolar) { // polar mode
      if(real34CompareEqual(dest_r, const34_0)) {
        real34Zero(dest_i);
      }
      else {
        real_t magnitude, theta;
        real34ToReal(dest_r, &magnitude);
        real34ToReal(dest_i, &theta);
        convertAngleFromTo(&theta, currentAngularMode, amRadian, &ctxtReal39);
        if(realCompareLessThan(&magnitude, const_0)) {
          realSetPositiveSign(&magnitude);
          realAdd(&theta, const_pi, &theta, &ctxtReal39);
        }
        realPolarToRectangular(&magnitude, &theta, &magnitude, &theta, &ctxtReal39); // theta in radian
        realToReal34(&magnitude, dest_r);
        realToReal34(&theta,     dest_i);
      }
      fnToPolar2(0);
    }

    else if((!getSystemFlag(FLAG_POLAR) && !temporaryFlagPolar) || temporaryFlagRect) { // rect mode
      fnToRect2(0);
    }
    temporaryFlagRect = false;
    temporaryFlagPolar = false;
    fnSetFlag(FLAG_CPXRES);
  }


  void closeNim(void) {
    setSystemFlag(FLAG_ASLIFT);
    //printf("closeNim\n");
    screenUpdatingMode &= ~(SCRUPD_MANUAL_STACK | SCRUPD_SKIP_STACK_ONE_TIME);

    if(nimNumberPart == NP_INT_10) {                //JM Input default type vv
      switch(Input_Default) {
      case ID_43S:                                 //   Do nothing, this is default LI/DP
      case ID_LI:                                  //   Do nothing, because default is LI/DP
        break;
      case ID_DP:                                  //   Do Real default for DP
      case ID_CPXDP:                               //                       CPX
        if(lastIntegerBase == 0) {
          nimNumberPart = NP_REAL_FLOAT_PART;
        }
        break;
      default:;
      }
    }                                               //JM ^^
    if((nimNumberPart == NP_INT_10 || nimNumberPart == NP_INT_16) && lastIntegerBase != 0) {
      strcat(aimBuffer, "#0");
      int16_t basePos = strlen(aimBuffer) - 1;
      if(lastIntegerBase <= 9) {
        aimBuffer[basePos] += lastIntegerBase;
      }
      else {
        aimBuffer[basePos++] = '1';
        aimBuffer[basePos] = '0';
        aimBuffer[basePos + 1] = 0;
        aimBuffer[basePos] += lastIntegerBase - 10;
      }

      nimNumberPart = NP_INT_BASE;
    }
    else {
      setLastintegerBasetoZero();
    }

    if(calcMode == CM_PEM) {
      pemCloseNumberInput();
      return;
    }
    
    bool_t delayedShortIntegerCHS = false;
    //#if defined(PC_BUILD)
    //  printf("closeNIM: aimBuffer=%s volid=%d nimNumberPart=%d NP_INT_BASE=%d\n",aimBuffer, validShortIntegerInX(), nimNumberPart, NP_INT_BASE);
    //  fflush(stdout);
    //#endif //PC_BUILD
    if((aimBuffer[0] == '-' && validShortIntegerInX() != 0)) {
      aimBuffer[0] = ' ';
      delayedShortIntegerCHS = true;
    }

    int16_t lastChar = strlen(aimBuffer) - 1;

    if(nimNumberPart != NP_INT_16) { // We need a # and a base
      if(nimNumberPart != NP_INT_BASE || aimBuffer[lastChar] != '#') { // We need a base
        if((nimNumberPart == NP_COMPLEX_EXPONENT || nimNumberPart == NP_REAL_EXPONENT) && (aimBuffer[lastChar] == '+' || aimBuffer[lastChar] == '-') && aimBuffer[lastChar - 1] == 'e') {
          aimBuffer[--lastChar] = 0;
          lastChar--;
        }
        else if(nimNumberPart == NP_REAL_EXPONENT && aimBuffer[lastChar] == 'e') {
          aimBuffer[lastChar--] = 0;
        }
        bool_t is_i     = nimNumberPart == NP_COMPLEX_INT_PART && aimBuffer[lastChar] == 'i';
        bool_t is_theta = nimNumberPart == NP_COMPLEX_INT_PART && aimBuffer[lastChar-1]*256 + aimBuffer[lastChar]*256 == 0xa221;
        if((is_i || is_theta) && !getSystemFlag(FLAG_POLAR)) { // complex i
          aimBuffer[++lastChar] = '1';                                                   //JM 2020-06-22 CHANGED FROM "1" to "0". DEFAULTING TO 0+0xi WHEN ABORTING CC ENTRY. #6072aee
          aimBuffer[lastChar + 1] = 0;                                                   //JM 2023-09-18 reverted partially (for RECT) from "0" to "1", specifically for a blank i
        }
        else if((is_i || is_theta) && getSystemFlag(FLAG_POLAR)) { // complex measured angle
          aimBuffer[++lastChar] = '0';                                                   //JM 2020-06-22 CHANGED FROM "1" to '0'. DEFAULTING TO 0+0xi WHEN ABORTING CC ENTRY. #6072aee
          aimBuffer[lastChar + 1] = 0;
        }

        if((aimBuffer[0] != '-' || aimBuffer[1] != 0) && (aimBuffer[lastChar] != '-')) { // The buffer is not just the minus sign AND The last char of the buffer is not the minus sign
          calcModeNormal();

          if(nimNumberPart == NP_INT_10) {
            longInteger_t lgInt;
            angularMode_t xangularMode;
            xangularMode = ((getRegisterDataType(REGISTER_X) == dtReal34) == dtReal34 ? getRegisterAngularMode(REGISTER_X) : amNone);

            if(xangularMode < amNone) {  // If editing with angular mode, then convert to real and preserve angular mode
              reallocateRegister(REGISTER_X, dtReal34, 0, getRegisterAngularMode(REGISTER_X));
              stringToReal34(aimBuffer, REGISTER_REAL34_DATA(REGISTER_X));
              if(xangularMode == amDMS) {
                real34FromDmsToDeg(REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
              }
            }
            else {
              longIntegerInit(lgInt);
              stringToLongInteger(aimBuffer + (aimBuffer[0] == '+' ? 1 : 0), 10, lgInt);
              convertLongIntegerToLongIntegerRegister(lgInt, REGISTER_X);
              longIntegerFree(lgInt);
            }
          }
          else if(nimNumberPart == NP_INT_BASE) {
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // Any change in this part must be reported in the function strToShortInteger from file testSuite.c after the line:else if(nimNumberPart == NP_INT_BASE) {
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            longInteger_t minVal, value, maxVal;
            int16_t posHash, i, lg;
            int32_t base;

            lg = strlen(aimBuffer);
            posHash = 0;
            for(i=1; i<lg; i++) {
              if(aimBuffer[i] == '#') {
                posHash = i;
                break;
              }
            }

            for(i=posHash+1; i<lg; i++) {
              if(aimBuffer[i]<'0' || aimBuffer[i]>'9') {
                // This should never happen
                displayCalcErrorMessage(ERROR_INVALID_INTEGER_INPUT, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
                #if (EXTRA_INFO_ON_CALC_ERROR == 1)
                  moreInfoOnError("In function closeNIM:", "there is a non numeric character in the base of the integer!", NULL, NULL);
                #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
                return;
              }
            }

            base = stringToInt32(aimBuffer + posHash + 1);
            if(base < 2 || base > 16) {
              displayCalcErrorMessage(ERROR_INVALID_INTEGER_INPUT, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
              #if (EXTRA_INFO_ON_CALC_ERROR == 1)
               moreInfoOnError("In function closeNIM:", "the base of the integer must be from 2 to 16!", NULL, NULL);
              #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
              return;
            }

            for(i=aimBuffer[0] == '-' ? 1 :0; i<posHash; i++) {
              if((aimBuffer[i] > '9' ? aimBuffer[i] - 'A' + 10 :aimBuffer[i] - '0') >= base) {
                displayCalcErrorMessage(ERROR_INVALID_INTEGER_INPUT, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
                #if (EXTRA_INFO_ON_CALC_ERROR == 1)
                  sprintf(errorMessage, "digit %c is not allowed in base %d!", aimBuffer[i], base);
                  moreInfoOnError("In function closeNIM:", errorMessage, NULL, NULL);
                #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)

                #if defined(DEBUGUNDO)
                  printf(">>> undo from addItemToNimBufferD\n");
                #endif // DEBUGUNDO
                undo();
                return;
              }
            }

            longIntegerInit(value);
            aimBuffer[posHash] = 0;
            stringToLongInteger(aimBuffer + (aimBuffer[0] == '+' ? 1 :0), base, value);

            // maxVal = 2^shortIntegerWordSize
            longIntegerInit(maxVal);
            if(shortIntegerWordSize >= 1 && shortIntegerWordSize <= 64) {
              longInteger2Pow(shortIntegerWordSize, maxVal);
            }
            else {
              sprintf(errorMessage, commonBugScreenMessages[bugMsgValueFor], "closeNIM", "shortIntegerWordSize", shortIntegerWordSize);
              displayBugScreen(errorMessage);
              longIntegerFree(maxVal);
              longIntegerFree(value);
              return;
            }

            // minVal = -maxVal/2
            longIntegerInit(minVal);
            longIntegerSetZero(minVal); // Mandatory! Else segmentation fault at next instruction
            longIntegerDivideUInt(maxVal, 2, minVal); // minVal = maxVal / 2
            longIntegerSetNegativeSign(minVal); // minVal = -minVal

            if((base != 2) && (base != 4) && (base != 8) && (base != 16) && (shortIntegerMode != SIM_UNSIGN)) {
              longIntegerDivideUInt(maxVal, 2, maxVal); // maxVal /= 2
            }

            longIntegerSubtractUInt(maxVal, 1, maxVal); // maxVal--

            if(shortIntegerMode == SIM_UNSIGN) {
              longIntegerSetZero(minVal); // minVal = 0
            }

            if(shortIntegerMode == SIM_1COMPL || shortIntegerMode == SIM_SIGNMT) {
              longIntegerAddUInt(minVal, 1, minVal); // minVal++
            }

            if(longIntegerCompare(value, minVal) < 0 || longIntegerCompare(value, maxVal) > 0) {
              displayCalcErrorMessage(ERROR_WORD_SIZE_TOO_SMALL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
              #if (EXTRA_INFO_ON_CALC_ERROR == 1)
                char strMin[22], strMax[22];
                longIntegerToAllocatedString(minVal, strMin, sizeof(strMin));
                longIntegerToAllocatedString(maxVal, strMax, sizeof(strMax));
                sprintf(errorMessage, "For word size of %d bit%s and integer mode %s,", shortIntegerWordSize, shortIntegerWordSize>1 ? "s" :"", getShortIntegerModeName(shortIntegerMode));
                sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "the entered number must be from %s to %s!", strMin, strMax);
                moreInfoOnError("In function closeNIM:", errorMessage, errorMessage + ERROR_MESSAGE_LENGTH/2, NULL);
              #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
              longIntegerFree(maxVal);
              longIntegerFree(minVal);
              longIntegerFree(value);
              return;
            }

            reallocateRegister(REGISTER_X, dtShortInteger, 0, base);

            char strValue[22];
            longIntegerToAllocatedString(value, strValue, sizeof(strValue));

            uint64_t val = strtoull(strValue + (longIntegerIsNegative(value) ? 1 :0), NULL, 10); // when value is negative:discard the minus sign

            if(shortIntegerMode == SIM_UNSIGN) {
            }
            else if(shortIntegerMode == SIM_2COMPL) {
              if(longIntegerIsNegative(value)) {
                val = (~val + 1) & shortIntegerMask;
              }
            }
            else if(shortIntegerMode == SIM_1COMPL) {
              if(longIntegerIsNegative(value)) {
                val = ~val & shortIntegerMask;
              }
            }
            else if(shortIntegerMode == SIM_SIGNMT) {
              if(longIntegerIsNegative(value)) {
                val = (val & shortIntegerMask) | shortIntegerSignBit;
              }
            }
            else {
              sprintf(errorMessage, commonBugScreenMessages[bugMsgValueFor], "closeNIM", "shortIntegerMode", shortIntegerMode);
              displayBugScreen(errorMessage);
              *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = 0;
            }

            *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = val;
            lastIntegerBase = base;
            fnRefreshState();                                                //JMNIM
            aimBuffer[0]=0;                                      //JMNIM Clear the NIM input buffer once written to register successfully.

            longIntegerFree(maxVal);
            longIntegerFree(minVal);
            longIntegerFree(value);
          }
          else if(nimNumberPart == NP_REAL_FLOAT_PART || nimNumberPart == NP_REAL_EXPONENT) {

            uint16_t dataType = getRegisterDataType(REGISTER_X);
            if(dataType == dtTime) {
              reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
              stringToReal34(aimBuffer, REGISTER_REAL34_DATA(REGISTER_X));

              if(calcMode != CM_NIM && lastErrorCode == 0) {
                if(getRegisterDataType(REGISTER_X) == dtLongInteger) {
                  convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
                }

                hmmssInRegisterToSeconds(REGISTER_X);
                if(lastErrorCode == 0) {
                  setSystemFlag(FLAG_ASLIFT);
                }
                else {
                  #if defined(DEBUGUNDO)
                    printf(">>> undo from addItemToNimBufferC\n");
                  #endif // DEBUGUNDO
                  undo();
                }
              }
            }
            else if(dataType == dtDate) {
              reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
              stringToReal34(aimBuffer, REGISTER_REAL34_DATA(REGISTER_X));

              if(calcMode != CM_NIM && lastErrorCode == 0) {
                convertReal34RegisterToDateRegister(REGISTER_X, REGISTER_X, YYSystem);
                checkDateRange(REGISTER_REAL34_DATA(REGISTER_X));
                temporaryInformation = TI_DAY_OF_WEEK;

                if(lastErrorCode == 0) {
                  setSystemFlag(FLAG_ASLIFT);
                }
                else {
                  #if defined(DEBUGUNDO)
                    printf(">>> undo from addItemToNimBufferB\n");
                  #endif // DEBUGUNDO
                  undo();
                }
                //return;
              }
            }
            else {

              if(lastIntegerBase == 0 && Input_Default == ID_CPXDP) {                                         //JM Input default type
                reallocateRegister(REGISTER_X, dtComplex34, 0, amNone); //JM Input default type
                stringToReal34(aimBuffer, REGISTER_REAL34_DATA(REGISTER_X));          //JM Input default type
                stringToReal34("0", REGISTER_IMAG34_DATA(REGISTER_X));                //JM Input default type
              }                                                                       //JM Input default type
              else {                                                                  //JM Input default type
                //reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
                angularMode_t xangularMode;
                xangularMode = ((getRegisterDataType(REGISTER_X) == dtReal34) == dtReal34 ? getRegisterAngularMode(REGISTER_X) : amNone);

                reallocateRegister(REGISTER_X, dtReal34, 0, xangularMode);
                stringToReal34(aimBuffer, REGISTER_REAL34_DATA(REGISTER_X));
                if(xangularMode == amDMS) {
                  real34FromDmsToDeg(REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
                }
              }              //JM Input default type
            }

          }
          else if(nimNumberPart == NP_FRACTION_DENOMINATOR || nimNumberPart == NP_HP32SII_DENOMINATOR) {
            //reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
            reallocateRegister(REGISTER_X, dtReal34, 0, getRegisterAngularMode(REGISTER_X));
            closeNimWithFraction(REGISTER_REAL34_DATA(REGISTER_X));
          }
          else if(nimNumberPart == NP_COMPLEX_INT_PART || nimNumberPart == NP_COMPLEX_FLOAT_PART || nimNumberPart == NP_COMPLEX_EXPONENT || nimNumberPart == NP_COMPLEX_FRACTION_DENOMINATOR || nimNumberPart == NP_COMPLEX_HP32SII_DENOMINATOR) {
            reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
            closeNimWithComplex(REGISTER_REAL34_DATA(REGISTER_X), REGISTER_IMAG34_DATA(REGISTER_X));
          }
          else {
            sprintf(errorMessage, commonBugScreenMessages[bugMsgUnexpectedSValue], "closeNIM", nimNumberPart, "nimNumberPart");
            displayBugScreen(errorMessage);
          }
        }
      }
    }
    if(delayedShortIntegerCHS) {
      //#if defined(PC_BUILD)
      //  printf("Launching delayed CHS\n");
      //  fflush(stdout);
      //#endif //PC_BUILD
      chsShoI();
  //    if(getSystemFlag(FLAG_OVERFLOW)) {
  //      temporaryInformation = TI_DATA_NEG_OVRFL;   //removeod, as CHS is overridden by ENTER, clearing the TI before it is shown/or directly after
  //      screenUpdatingMode &= ~(SCRUPD_MANUAL_STACK);
  //    }
    }
  }


  void closeAim(void) {
    calcModeNormal();
    popSoftmenu();

    if(aimBuffer[0] == 0) {
      #if defined(DEBUGUNDO)
        printf(">>> undo from closeAim\n");
      #endif // DEBUGUNDO
      undo();
    }
    else {
      int16_t len = stringByteLength(aimBuffer) + 1;

      reallocateRegister(REGISTER_X, dtString, TO_BLOCKS(len), amNone);

      xcopy(REGISTER_STRING_DATA(REGISTER_X), aimBuffer, len);
      aimBuffer[0] = 0;

      setSystemFlag(FLAG_ASLIFT);
    }
  }


  void insertAlphaCharacter(uint16_t item, int16_t *currentCursor) {
    const char *addChar = item == ITM_PAIR_OF_PARENTHESES ? "()" :
                          item == ITM_VERTICAL_BAR        ? "||" :
                          item == ITM_ROOT_SIGN           ? STD_SQUARE_ROOT "()" :
      #if USE_ITALIC_CONSTANT != 0
                          item == ITM_ALOG_SYMBOL         ? STD_EULER_e "^()" :
      #endif /* USE_ITALIC_CONSTANT != 0 */
                          indexOfItems[item].itemSoftmenuName;
    char *aimCursorPos = aimBuffer;
    char *aimBottomPos = aimBuffer + stringByteLength(aimBuffer);
    uint32_t itemLen = stringByteLength(addChar);
    for(int32_t i = 0; i < *currentCursor; ++i) {
      aimCursorPos += (*aimCursorPos & 0x80) ? 2 : 1;
    }
    for(; aimBottomPos >= aimCursorPos; --aimBottomPos) {
      *(aimBottomPos + itemLen) = *aimBottomPos;
    }
    xcopy(aimCursorPos, addChar, itemLen);
    switch(item) {
      case ITM_ROOT_SIGN: {
        *currentCursor += 2;
        break;
      }
      case ITM_PAIR_OF_PARENTHESES:
      case ITM_VERTICAL_BAR: {
        *currentCursor += 1;
        break;
      }
      default: {
        *currentCursor += stringGlyphLength(indexOfItems[item].itemSoftmenuName);
      }
    }
  }


  void deleteAlphaCharacter(int16_t *currentCursor) {
    char *srcPos = aimBuffer;
    char *dstPos = aimBuffer;
    char *lstPos = aimBuffer + stringNextGlyph(aimBuffer, stringLastGlyph(aimBuffer));
    --*currentCursor;
    for(int16_t i = 0; i < *currentCursor; ++i) {
      dstPos += (*dstPos & 0x80) ? 2 : 1;
    }
    srcPos = dstPos + ((*dstPos & 0x80) ? 2 : 1);
    for(; srcPos <= lstPos;) {
      *(dstPos++) = *(srcPos++);
    }
  }


  void fnAlphaCursorLeft(uint16_t unusedButMandatoryParameter) {
    if(alphaCursor > 0) {
      --alphaCursor;
    }
  }


  void fnAlphaCursorRight(uint16_t unusedButMandatoryParameter) {
    if(alphaCursor < (uint16_t)stringGlyphLength(aimBuffer)) {
      ++alphaCursor;
    }
  }


  void fnAlphaCursorHome(uint16_t unusedButMandatoryParameter) {
    alphaCursor = 0;
  }


  void fnAlphaCursorEnd(uint16_t unusedButMandatoryParameter) {
    alphaCursor = (uint16_t)stringGlyphLength(aimBuffer);
  }

#endif // !TESTSUITE_BUILD
