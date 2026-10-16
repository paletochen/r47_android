// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

#if !defined(OPTION_ASTRING)                                                  // stubs for the alpha string functions (real code is guarded below): aMID aLEFT aRIGHT aTRIM aREV aLOWER aUPPER
  void fnAlphaLower(uint16_t regist) {}
  void fnAlphaUpper(uint16_t regist) {}
  void fnAlphaLeft (uint16_t regist) {}
  void fnAlphaMid  (uint16_t regist) {}
  void fnAlphaRight(uint16_t regist) {}
  void fnAlphaRev  (uint16_t regist) {}
  void fnAlphaTrim (uint16_t regist) {}
#endif // !OPTION_ASTRING

TO_QSPI const upperLower_t upperLowerTable[] = {
    { STD_A,                 STD_a                 },
    { STD_B,                 STD_b                 },
    { STD_C,                 STD_c                 },
    { STD_D,                 STD_d                 },
    { STD_E,                 STD_e                 },
    { STD_F,                 STD_f                 },
    { STD_G,                 STD_g                 },
    { STD_H,                 STD_h                 },
    { STD_I,                 STD_i                 },
    { STD_J,                 STD_j                 },
    { STD_K,                 STD_k                 },
    { STD_L,                 STD_l                 },
    { STD_M,                 STD_m                 },
    { STD_N,                 STD_n                 },
    { STD_O,                 STD_o                 },
    { STD_P,                 STD_p                 },
    { STD_Q,                 STD_q                 },
    { STD_R,                 STD_r                 },
    { STD_S,                 STD_s                 },
    { STD_T,                 STD_t                 },
    { STD_U,                 STD_u                 },
    { STD_V,                 STD_v                 },
    { STD_W,                 STD_w                 },
    { STD_X,                 STD_x                 },
    { STD_Y,                 STD_y                 },
    { STD_Z,                 STD_z                 },
    { STD_ALPHA,             STD_alpha             },
    { STD_BETA,              STD_beta              },
    { STD_GAMMA,             STD_gamma             },
    { STD_DELTA,             STD_delta             },
    { STD_EPSILON,           STD_epsilon           },
    { STD_ZETA,              STD_zeta              },
    { STD_ETA,               STD_eta               },
    { STD_THETA,             STD_theta_m           },
    { STD_IOTA,              STD_iota              },
    { STD_KAPPA,             STD_kappa             },
    { STD_LAMBDA,            STD_lambda            },
    { STD_MU,                STD_mu                },
    { STD_NU,                STD_nu                },
    { STD_XI,                STD_xi                },
    { STD_OMICRON,           STD_omicron           },
    { STD_PI,                STD_pi                },
    { STD_RHO,               STD_rho               },
    { STD_SIGMA,             STD_sigma             },
    { STD_TAU,               STD_tau               },
    { STD_UPSILON,           STD_upsilon           },
    { STD_PHI,               STD_phi               },
    { STD_CHI,               STD_chi               },
    { STD_PSI,               STD_psi               },
    { STD_OMEGA,             STD_omega             },
    { STD_IOTA_DIALYTIKA,    STD_iota_DIALYTIKA    },
    { STD_UPSILON_DIALYTIKA, STD_upsilon_DIALYTIKA },
    { STD_A_GRAVE,           STD_a_GRAVE           },
    { STD_A_ACUTE,           STD_a_ACUTE           },
    { STD_A_CIRC,            STD_a_CIRC            },
    { STD_A_TILDE,           STD_a_TILDE           },
    { STD_A_DIARESIS,        STD_a_DIARESIS        },
    { STD_A_RING,            STD_a_RING            },
    { STD_AE,                STD_ae                },
    { STD_C_CEDILLA,         STD_c_CEDILLA         },
    { STD_E_GRAVE,           STD_e_GRAVE           },
    { STD_E_ACUTE,           STD_e_ACUTE           },
    { STD_E_CIRC,            STD_e_CIRC            },
    { STD_E_DIARESIS,        STD_e_DIARESIS        },
    { STD_I_GRAVE,           STD_i_GRAVE           },
    { STD_I_ACUTE,           STD_i_ACUTE           },
    { STD_I_CIRC,            STD_i_CIRC            },
    { STD_I_DIARESIS,        STD_i_DIARESIS        },
    { STD_ETH,               STD_eth               },
    { STD_N_TILDE,           STD_n_TILDE           },
    { STD_O_GRAVE,           STD_o_GRAVE           },
    { STD_O_ACUTE,           STD_o_ACUTE           },
    { STD_O_CIRC,            STD_o_CIRC            },
    { STD_O_TILDE,           STD_o_TILDE           },
    { STD_O_DIARESIS,        STD_o_DIARESIS        },
    { STD_CROSS,             STD_DIVIDE            },
    { STD_O_STROKE,          STD_o_STROKE          },
    { STD_U_GRAVE,           STD_u_GRAVE           },
    { STD_U_ACUTE,           STD_u_ACUTE           },
    { STD_U_CIRC,            STD_u_CIRC            },
    { STD_U_DIARESIS,        STD_u_DIARESIS        },
    { STD_Y_ACUTE,           STD_y_ACUTE           },
    { STD_Y_DIARESIS,        STD_y_DIARESIS        },
    { STD_A_MACRON,          STD_a_MACRON          },
    { STD_A_BREVE,           STD_a_BREVE           },
    { STD_A_OGONEK,          STD_a_OGONEK          },
    { STD_C_ACUTE,           STD_c_ACUTE           },
    { STD_C_CARON,           STD_c_CARON           },
    { STD_D_STROKE,          STD_d_STROKE          },
    { STD_E_MACRON,          STD_e_MACRON          },
    { STD_E_BREVE,           STD_e_BREVE           },
    { STD_E_DOT,             STD_e_DOT             },
    { STD_E_OGONEK,          STD_e_OGONEK          },
    { STD_E_CARON,           STD_e_CARON           },
    { STD_G_BREVE,           STD_g_BREVE           },
    { STD_I_MACRON,          STD_i_MACRON          },
    { STD_I_BREVE,           STD_i_BREVE           },
    { STD_I_OGONEK,          STD_i_OGONEK          },
    { STD_L_ACUTE,           STD_l_ACUTE           },
    { STD_L_APOSTROPHE,      STD_l_APOSTROPHE      },
    { STD_L_STROKE,          STD_l_STROKE          },
    { STD_N_ACUTE,           STD_n_ACUTE           },
    { STD_N_CARON,           STD_n_CARON           },
    { STD_O_MACRON,          STD_o_MACRON          },
    { STD_O_BREVE,           STD_o_BREVE           },
    { STD_OE,                STD_oe                },
    { STD_R_ACUTE,           STD_r_ACUTE           },
    { STD_R_CARON,           STD_r_CARON           },
    { STD_S_ACUTE,           STD_s_ACUTE           },
    { STD_S_CEDILLA,         STD_s_CEDILLA         },
    { STD_S_CARON,           STD_s_CARON           },
    { STD_T_CEDILLA,         STD_t_CEDILLA         },
    { STD_T_CARON,           STD_t_APOSTROPHE      },
    { STD_U_TILDE,           STD_u_TILDE           },
    { STD_U_MACRON,          STD_u_MACRON          },
    { STD_U_BREVE,           STD_u_BREVE           },
    { STD_U_RING,            STD_u_RING            },
    { STD_U_OGONEK,          STD_u_OGONEK          },
    { STD_W_CIRC,            STD_w_CIRC            },
    { STD_Y_CIRC,            STD_y_CIRC            },
    { STD_Z_ACUTE,           STD_z_ACUTE           },
    { STD_Z_DOT,             STD_z_DOT             },
    { STD_Z_CARON,           STD_z_CARON           },
    { STD_QOPPA,             STD_qoppa             },
    { STD_DIGAMMA,           STD_digamma           },
    { STD_SAMPI,             STD_sampi             },
    { STD_SUP_A,             STD_SUP_a             },
    { STD_SUP_B,             STD_SUP_b             },
    { STD_SUP_C,             STD_SUP_c             },
    { STD_SUP_D,             STD_SUP_d             },
    { STD_SUP_E,             STD_SUP_e             },
    { STD_SUP_F,             STD_SUP_f             },
    { STD_SUP_G,             STD_SUP_g             },
    { STD_SUP_H,             STD_SUP_h             },
    { STD_SUP_I,             STD_SUP_i             },
    { STD_SUP_J,             STD_SUP_j             },
    { STD_SUP_K,             STD_SUP_k             },
    { STD_SUP_L,             STD_SUP_l             },
    { STD_SUP_M,             STD_SUP_m             },
    { STD_SUP_N,             STD_SUP_n             },
    { STD_SUP_O,             STD_SUP_o             },
    { STD_SUP_P,             STD_SUP_p             },
    { STD_SUP_Q,             STD_SUP_q             },
    { STD_SUP_R,             STD_SUP_r             },
    { STD_SUP_S,             STD_SUP_s             },
    { STD_SUP_T,             STD_SUP_t             },
    { STD_SUP_U,             STD_SUP_u             },
    { STD_SUP_V,             STD_SUP_v             },
    { STD_SUP_W,             STD_SUP_w             },
    { STD_SUP_X,             STD_SUP_x             },
    { STD_SUP_Y,             STD_SUP_y             },
    { STD_SUP_Z,             STD_SUP_z             },
    { STD_SUB_A,             STD_SUB_a             },
    { STD_SUB_B,             STD_SUB_b             },
    { STD_SUB_C,             STD_SUB_c             },
    { STD_SUB_D,             STD_SUB_d             },
    { STD_SUB_E,             STD_SUB_e             },
    { STD_SUB_F,             STD_SUB_f             },
    { STD_SUB_G,             STD_SUB_g             },
    { STD_SUB_H,             STD_SUB_h             },
    { STD_SUB_I,             STD_SUB_i             },
    { STD_SUB_J,             STD_SUB_j             },
    { STD_SUB_K,             STD_SUB_k             },
    { STD_SUB_L,             STD_SUB_l             },
    { STD_SUB_M,             STD_SUB_m             },
    { STD_SUB_N,             STD_SUB_n             },
    { STD_SUB_O,             STD_SUB_o             },
    { STD_SUB_P,             STD_SUB_p             },
    { STD_SUB_Q,             STD_SUB_q             },
    { STD_SUB_R,             STD_SUB_r             },
    { STD_SUB_S,             STD_SUB_s             },
    { STD_SUB_T,             STD_SUB_t             },
    { STD_SUB_U,             STD_SUB_u             },
    { STD_SUB_V,             STD_SUB_v             },
    { STD_SUB_W,             STD_SUB_w             },
    { STD_SUB_X,             STD_SUB_x             },
    { STD_SUB_Y,             STD_SUB_y             },
    { STD_SUB_Z,             STD_SUB_z             },
    { "",                    ""                    }
};

void trimLeadingSpace(char *stringToTrim) {
  int16_t len;

  if(*stringToTrim == ' ') {
    len = stringByteLength(stringToTrim);
    xcopy(stringToTrim, stringToTrim + 1, len);
  }
}



void fnClearAlpha(uint16_t regist) {
  reallocateRegister(regist, dtString, 1, amNone);
  *(REGISTER_STRING_DATA(regist)) = 0;
}



void fnAlphaLeng(uint16_t regist) {
  longInteger_t stringSize;

  if(getRegisterDataType(regist) != dtString) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot get the " STD_alpha "LENG? from %s", getRegisterDataTypeName(regist, true, false));
      moreInfoOnError("In function fnAlphaLeng:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  longIntegerInit(stringSize);
  int32ToLongInteger(stringGlyphLength(REGISTER_STRING_DATA(regist)), stringSize);

  liftStack();

  convertLongIntegerToLongIntegerRegister(stringSize, REGISTER_X);
  longIntegerFree(stringSize);
}



void fnAlphaToX(uint16_t regist) {
  unsigned char char1, char2;
  longInteger_t lgInt;

  if(getRegisterDataType(regist) != dtString) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot use " STD_alpha STD_RIGHT_ARROW "x on %s", getRegisterDataTypeName(regist, true, false));
      moreInfoOnError("In function fnAlphaToX:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(stringByteLength(REGISTER_STRING_DATA(regist)) == 0) {
    liftStack();
    longIntegerInit(lgInt);
    int32ToLongInteger(0, lgInt);
    convertLongIntegerToLongIntegerRegister(lgInt, REGISTER_X);
    longIntegerFree(lgInt);
    return;
  }

  char1 = *(REGISTER_STRING_DATA(regist));
  if(char1 & 0x80) {
    char2 = *(REGISTER_STRING_DATA(regist) + 1);
  }

  liftStack();

  longIntegerInit(lgInt);
  uInt32ToLongInteger(char1 & 0x7f, lgInt);
  if(char1 & 0x80) {
    longIntegerMultiplyUInt(lgInt, 256, lgInt);
    longIntegerAddUInt(lgInt, char2, lgInt);
  }

  convertLongIntegerToShortIntegerRegister(lgInt, 16, REGISTER_X);
  longIntegerFree(lgInt);

  if(!getSystemFlag(FLAG_ASLIFT) || regist != getStackTop()) {
    if(REGISTER_X <= regist && regist < getStackTop()) {
      regist++;
    }
    xcopy(REGISTER_STRING_DATA(regist), REGISTER_STRING_DATA(regist) + (char1 & 0x80 ? 2 : 1), stringByteLength(REGISTER_STRING_DATA(regist) + (char1 & 0x80 ? 2 : 1)) + 1);
  }
}


static void _readDestinationRegister(uint16_t regist) {
  switch(getRegisterDataType(regist)) {
    case dtLongInteger: {
      longIntegerRegisterToDisplayString(regist, tmpString, TMP_STR_LENGTH, SCREEN_WIDTH, 50, STD_SPACE_PUNCTUATION);
      break;
    }

    case dtTime: {
      timeToDisplayString(regist, tmpString, false);
      break;
    }

    case dtDate: {
      dateToDisplayString(regist, tmpString);
      break;
    }

    case dtString: {
      xcopy(tmpString, REGISTER_STRING_DATA(regist), stringByteLength(REGISTER_STRING_DATA(regist)) + 1);
      break;
    }

    case dtReal34Matrix: {
      real34MatrixToDisplayString(regist, tmpString);
      break;
    }

    case dtComplex34Matrix: {
      complex34MatrixToDisplayString(regist, tmpString);
      break;
    }

    case dtShortInteger: {
      shortIntegerToDisplayString(regist, tmpString, false, noBaseOverride, SCREEN_WIDTH);
      break;
    }

    case dtReal34: {
      real34ToDisplayString(REGISTER_REAL34_DATA(regist), getRegisterAngularMode(regist), tmpString, &standardFont, SCREEN_WIDTH, NUMBER_OF_DISPLAY_DIGITS, false, STD_SPACE_PUNCTUATION, true);
      trimLeadingSpace(tmpString);
      break;
    }

    case dtComplex34: {
      complex34ToDisplayString(REGISTER_COMPLEX34_DATA(regist), tmpString, &numericFont, SCREEN_WIDTH, NUMBER_OF_DISPLAY_DIGITS, false, STD_SPACE_PUNCTUATION, true, getComplexRegisterAngularMode(regist), getComplexRegisterPolarMode(regist));
      trimLeadingSpace(tmpString);
      break;
    }

    default: {
      tmpString[0] = 0;
      break;
    }
  }
}


static void _doXToAlpha(uint16_t regist) {
  longInteger_t lgInt;
  unsigned char char1, char2;

  longIntegerInit(lgInt);
  // lgInt pre-init. Each converter branch must longIntegerFree() the pre-init first, or it leaks.
  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      longIntegerFree(lgInt); // the converter re-initialises lgInt; free the pre-init first
      convertLongIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    case dtReal34: {
      if(real34CompareAbsGreaterThan(REGISTER_REAL34_DATA(REGISTER_X), const34_1e6)) {
        uInt32ToLongInteger(1000000, lgInt);
      }
      else {
        longIntegerFree(lgInt); // the converter re-initialises lgInt; free the pre-init first
        convertReal34ToLongInteger(REGISTER_REAL34_DATA(REGISTER_X), lgInt, DEC_ROUND_DOWN);
      }
      break;
    }

    case dtShortInteger: {
      longIntegerFree(lgInt); // the converter re-initialises lgInt; free the pre-init first
      convertShortIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "cannot x" STD_RIGHT_ARROW STD_alpha " when X is %s", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function _doXToAlpha:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      longIntegerFree(lgInt);
      return;
    }
  }

  longIntegerSetPositiveSign(lgInt);
  //if(longIntegerCompareUInt(lgInt, standardFont.glyphs[standardFont.numberOfGlyphs - 1].charCode & 0x7fff) > 0) {
  if(longIntegerCompareUInt(lgInt, 0x8000) >= 0) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "for x" STD_RIGHT_ARROW STD_alpha ", X must be < 32768. Here X = %" PRIu32, (uint32_t)lgInt->_mp_d[0]); // OK for 32 and 64 bit limbs
      moreInfoOnError("In function _doXToAlpha:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1
    longIntegerFree(lgInt);
    return;
  }

  if(longIntegerIsZero(lgInt)) {
    char1 = 0;
    char2 = 0;
  }
  else if(lgInt->_mp_d[0] < 0x0080) {      // OK for 32 and 64 bit limbs
    char1 = lgInt->_mp_d[0];               // OK for 32 and 64 bit limbs
    char2 = 0;
  }
  else {
    char1 = (lgInt->_mp_d[0] >> 8) | 0x80; // OK for 32 and 64 bit limbs
    char2 = lgInt->_mp_d[0] & 0x00ff;      // OK for 32 and 64 bit limbs
  }

  longIntegerFree(lgInt);

  if(regist != REGISTER_X) {
    _readDestinationRegister(regist);
  }
  else {
    tmpString[0] = 0;      // If destination register is X just return the alpha character from the character code
  }

  int destLen = stringByteLength(tmpString);  // captured before displayCalcErrorMessage overwrites tmpString
  if(destLen + 2 >= MAX_STRING_BYTE_LENGTH) {
    displayCalcErrorMessage(ERROR_STRING_WOULD_BE_TOO_LONG, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "the resulting string would be %d bytes long. Maximum is %d", destLen + 2, MAX_STRING_BYTE_LENGTH - 1);
      moreInfoOnError("In function _doXToAlpha:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1
  }
  else {
    int l = stringByteLength(tmpString);
    tmpString[l]       = char1;
    tmpString[l + 1]   = char2;
    if(char2) {
      tmpString[l + 2] = 0;
      ++l;
    }
    ++l;

    reallocateRegister(regist, dtString, l + 1, amNone);
    xcopy(REGISTER_STRING_DATA(regist), tmpString, l + 1);
  }

}


void fnXToAlpha(uint16_t regist) {   // new version, similar to the hp-42s ATOX function
  if(!saveLastX()) {
    return;
  }

  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger:
    case dtReal34:
    case dtShortInteger: {
      _doXToAlpha(regist);
      return;
    }

    case dtString: {
      _readDestinationRegister(regist);
      int destLen = stringByteLength(tmpString);  // captured before displayCalcErrorMessage overwrites tmpString
      int xStrLen = stringByteLength(REGISTER_STRING_DATA(REGISTER_X));
      if(destLen + xStrLen >= MAX_STRING_BYTE_LENGTH) {
        displayCalcErrorMessage(ERROR_STRING_WOULD_BE_TOO_LONG, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "the resulting string would be %d (%d + %d) bytes long. Maximum is %d",
            destLen + xStrLen,
            destLen,
            xStrLen, MAX_STRING_BYTE_LENGTH - 1);
          moreInfoOnError("In function fnXToAlpha:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1
      }
      else {
        int l = stringByteLength(tmpString);
        xcopy(tmpString + l, REGISTER_STRING_DATA(REGISTER_X), stringByteLength(REGISTER_STRING_DATA(REGISTER_X)) + 1);
        l = stringByteLength(tmpString);
        reallocateRegister(regist, dtString, l + 1, amNone);
        xcopy(REGISTER_STRING_DATA(regist), tmpString, l + 1);
      }
      return;
    }

    case dtReal34Matrix: {
      if(regist != REGISTER_X) {
        elementwiseRema_UInt16(_doXToAlpha, regist);
      }
      else {                                 // if X is the destination register, just return in X a string composed of the character codes from the matrux in X
        reallocateRegister(REGISTER_L, dtString, 1, amNone);
        xcopy(REGISTER_STRING_DATA(REGISTER_L), "", 1);
        elementwiseRema_UInt16(_doXToAlpha, REGISTER_L);
        fnSwapX(REGISTER_L);
      }
      return;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "cannot x" STD_RIGHT_ARROW STD_alpha " when X is %s", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function fnXToAlpha:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1
      return;
    }
  }

}


void fnXToAlphaOld(uint16_t unusedButMandatoryParameter) {   // deprecated version, only for backward compatibility
  longInteger_t lgInt;
  unsigned char char1, char2;

  longIntegerInit(lgInt);
  // lgInt pre-init. Each converter branch must longIntegerFree() the pre-init first, or it leaks.
  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      longIntegerFree(lgInt); // the converter re-initialises lgInt; free the pre-init first
      convertLongIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    case dtReal34: {
      if(real34CompareAbsGreaterThan(REGISTER_REAL34_DATA(REGISTER_X), const34_1e6)) {
        uInt32ToLongInteger(1000000u, lgInt);
      }
      else {
        longIntegerFree(lgInt); // the converter re-initialises lgInt; free the pre-init first
        convertReal34ToLongInteger(REGISTER_REAL34_DATA(REGISTER_X), lgInt, DEC_ROUND_DOWN);
      }
      break;
    }

    case dtShortInteger: {
      longIntegerFree(lgInt); // the converter re-initialises lgInt; free the pre-init first
      convertShortIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "cannot x" STD_RIGHT_ARROW STD_alpha " when X is %s", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function fnXToAlpha:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      longIntegerFree(lgInt);
      return;
    }
  }

  longIntegerSetPositiveSign(lgInt);
  //if(longIntegerCompareUInt(lgInt, standardFont.glyphs[standardFont.numberOfGlyphs - 1].charCode & 0x7fff) > 0) {
  if(longIntegerCompareUInt(lgInt, 0x8000) >= 0) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "for x" STD_RIGHT_ARROW STD_alpha ", X must be < 32768. Here X = %" PRIu32, (uint32_t)lgInt->_mp_d[0]); // OK for 32 and 64 bit limbs
      moreInfoOnError("In function fnXToAlpha:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    longIntegerFree(lgInt);
    return;
  }

  liftStack();

  if(longIntegerIsZero(lgInt)) {
    char1 = 0;
    char2 = 0;
  }
  else if(lgInt->_mp_d[0] < 0x0080) {      // OK for 32 and 64 bit limbs
    char1 = lgInt->_mp_d[0];               // OK for 32 and 64 bit limbs
    char2 = 0;
  }
  else {
    char1 = (lgInt->_mp_d[0] >> 8) | 0x80; // OK for 32 and 64 bit limbs
    char2 = lgInt->_mp_d[0] & 0x00ff;      // OK for 32 and 64 bit limbs
  }

  longIntegerFree(lgInt);

  reallocateRegister(REGISTER_X, dtString, 1, amNone);
  *(REGISTER_STRING_DATA(REGISTER_X))     = char1;
  *(REGISTER_STRING_DATA(REGISTER_X) + 1) = char2;
  *(REGISTER_STRING_DATA(REGISTER_X) + 2) = 0;
}



void fnAlphaPos(uint16_t regist) {
  longInteger_t lgInt;
  char *ptrTarget, *ptrRegist;
  int16_t lgTarget, lgRegist, i, j;
  bool_t found;

  if(getRegisterDataType(regist) != dtString) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot use " STD_alpha "POS? on %s (reg %" PRIu16 ")", getRegisterDataTypeName(regist, true, false), regist);
      moreInfoOnError("In function fnAlphaPos:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(programRunStop == PGM_RUNNING) {
    copySourceRegisterToDestRegister(REGISTER_X, SAVED_REGISTER_X);    // Save register X
  }

  if(getRegisterDataType(REGISTER_X) != dtString) {
    _doXToAlpha(REGISTER_X);
    if(lastErrorCode != ERROR_NONE) {
      return;
    }
  }

  if(!saveLastX()) {
    return;
  }

  longIntegerInit(lgInt);
  int32ToLongInteger(-1, lgInt);

  if(stringGlyphLength(REGISTER_STRING_DATA(regist)) != 0 && stringGlyphLength(REGISTER_STRING_DATA(REGISTER_X)) != 0) {
    ptrTarget = REGISTER_STRING_DATA(REGISTER_X);
    ptrRegist = REGISTER_STRING_DATA(regist);
    lgTarget = stringByteLength(ptrTarget);
    lgRegist = stringByteLength(ptrRegist);

    for(i=0; i<=lgRegist-lgTarget; i++) {
      found = true;
      for(j=0; j<lgTarget; j++) {
        if(*(ptrRegist+i+j) != *(ptrTarget+j)) {
          found = false;
          break;
        }
      }
      if(found) {
        int32ToLongInteger(i, lgInt);
        break;
      }
    }
  }


  copySourceRegisterToDestRegister(SAVED_REGISTER_X, REGISTER_X);    // Restore register X
  liftStack();
  convertLongIntegerToLongIntegerRegister(lgInt, REGISTER_X);
  longIntegerFree(lgInt);
}



/**
 * \brief Return floor(abs(value)) modulo modulus without a wide decimal remainder.
 *
 * \param[in] value Finite or special Real34 rotation count
 * \param[in] modulus Non-negative string glyph count
 * \return Exact reduced count, or zero for a zero modulus or special value
 *
 * Non-finite values retain the former zero-step conversion behaviour.
 */
static uint16_t real34RotationSteps(const real34_t *value, uint16_t modulus) {
  uint8_t bcd[DECQUAD_Pmax];
  int32_t exponent, coefficientDigits;
  uint32_t remainder, power;

  if(modulus == 0 || real34IsSpecial(value)) {
    return 0;
  }

  real34GetCoefficient(value, bcd);
  exponent = real34GetExponent(value);
  if(exponent <= -DECQUAD_Pmax) {
    return 0;
  }

  coefficientDigits = DECQUAD_Pmax + min(exponent, 0);
  remainder = 0;
  for(int32_t i=0; i<coefficientDigits; i++) {
    remainder = (remainder * 10u + bcd[i]) % modulus;
  }

  power = 10u % modulus;
  while(exponent > 0) {
    if(exponent & 1) {
      remainder = remainder * power % modulus;
    }
    power = power * power % modulus;
    exponent >>= 1;
  }
  return (uint16_t)remainder;
}



void fnAlphaRR(uint16_t regist) {
  longInteger_t lgInt;
  int16_t stringGlyphLen, steps, glyphPointer;
  char *ptr;

  if(getRegisterDataType(regist) != dtString) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot use " STD_alpha "RR on %s", getRegisterDataTypeName(regist, true, false));
      moreInfoOnError("In function fnAlphaRR:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  stringGlyphLen = stringGlyphLength(REGISTER_STRING_DATA(regist));
  if(stringGlyphLen == 0) {
    return;
  }

  longIntegerInit(lgInt);
  // lgInt pre-init. Each converter branch must longIntegerFree() the pre-init first, or it leaks.
  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      longIntegerFree(lgInt); // the converter re-initialises lgInt; free the pre-init first
      convertLongIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    case dtReal34: {
      uInt32ToLongInteger(real34RotationSteps(REGISTER_REAL34_DATA(REGISTER_X), stringGlyphLen), lgInt);
      break;
    }

    case dtShortInteger: {
      longIntegerFree(lgInt); // the converter re-initialises lgInt; free the pre-init first
      convertShortIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "cannot " STD_alpha "RR when X is %s", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function fnAlphaRR:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      longIntegerFree(lgInt);
      return;
    }
  }

  longIntegerSetPositiveSign(lgInt);
  steps = longIntegerModuloUInt(lgInt, stringGlyphLen);
  longIntegerFree(lgInt);

  if(!saveLastX()) {
    return;
  }

  if(steps > 0) {
    for(glyphPointer=0, steps=stringGlyphLen-steps; steps > 0; steps--) {
      glyphPointer = stringNextGlyph(REGISTER_STRING_DATA(regist), glyphPointer);
    }

    ptr = REGISTER_STRING_DATA(regist) + glyphPointer;
    steps = stringByteLength(ptr) + 1;
    xcopy(tmpString, ptr, steps);
    *(ptr) = 0;
    COPY_REGISTER_STRING_TO(tmpString + stringByteLength(tmpString), regist);
    xcopy(REGISTER_STRING_DATA(regist), tmpString, stringByteLength(tmpString) + 1);
  }
}



void fnAlphaRL(uint16_t regist) {
  longInteger_t lgInt;
  int16_t stringGlyphLen, steps, glyphPointer;
  char *ptr;

  if(getRegisterDataType(regist) != dtString) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot use " STD_alpha "RL on %s", getRegisterDataTypeName(regist, true, false));
      moreInfoOnError("In function fnAlphaRL:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  stringGlyphLen = stringGlyphLength(REGISTER_STRING_DATA(regist));
  if(stringGlyphLen == 0) {
    return;
  }

  longIntegerInit(lgInt);
  // lgInt pre-init. Each converter branch must longIntegerFree() the pre-init first, or it leaks.
  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      longIntegerFree(lgInt); // the converter re-initialises lgInt; free the pre-init first
      convertLongIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    case dtReal34: {
      uInt32ToLongInteger(real34RotationSteps(REGISTER_REAL34_DATA(REGISTER_X), stringGlyphLen), lgInt);
      break;
    }

    case dtShortInteger: {
      longIntegerFree(lgInt); // the converter re-initialises lgInt; free the pre-init first
      convertShortIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "cannot " STD_alpha "RL when X is %s", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function fnAlphaRL:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      longIntegerFree(lgInt);
      return;
    }
  }

  longIntegerSetPositiveSign(lgInt);
  steps = longIntegerModuloUInt(lgInt, stringGlyphLen);
  longIntegerFree(lgInt);

  if(!saveLastX()) {
    return;
  }

  if(steps > 0) {
    for(glyphPointer=0; steps > 0; steps--) {
      glyphPointer = stringNextGlyph(REGISTER_STRING_DATA(regist), glyphPointer);
    }

    ptr = REGISTER_STRING_DATA(regist) + glyphPointer;
    steps = stringByteLength(ptr) + 1;
    xcopy(tmpString, ptr, steps);
    *(ptr) = 0;
    COPY_REGISTER_STRING_TO(tmpString + stringByteLength(tmpString), regist);
    xcopy(REGISTER_STRING_DATA(regist), tmpString, stringByteLength(tmpString) + 1);
  }
}



void fnAlphaSR(uint16_t regist) {
  longInteger_t lgInt;
  int16_t stringGlyphLen, steps, glyphPointer;

  if(getRegisterDataType(regist) != dtString) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot use " STD_alpha "SR on %s", getRegisterDataTypeName(regist, true, false));
      moreInfoOnError("In function fnAlphaSR:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  stringGlyphLen = stringGlyphLength(REGISTER_STRING_DATA(regist));
  if(stringGlyphLen == 0) {
    return;
  }

  longIntegerInit(lgInt);
  // lgInt pre-init. Each converter branch must longIntegerFree() the pre-init first, or it leaks.
  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      longIntegerFree(lgInt); // the converter re-initialises lgInt; free the pre-init first
      convertLongIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    case dtReal34: {
      if(real34CompareAbsGreaterThan(REGISTER_REAL34_DATA(REGISTER_X), const34_1e6)) {
        uInt32ToLongInteger(1000000u, lgInt);
      }
      else {
        longIntegerFree(lgInt); // the converter re-initialises lgInt; free the pre-init first
        convertReal34ToLongInteger(REGISTER_REAL34_DATA(REGISTER_X), lgInt, DEC_ROUND_DOWN);
      }
      break;
    }

    case dtShortInteger: {
      longIntegerFree(lgInt); // the converter re-initialises lgInt; free the pre-init first
      convertShortIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "cannot " STD_alpha "SR when X is %s", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function fnAlphaSR:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      longIntegerFree(lgInt);
      return;
    }
  }

  longIntegerSetPositiveSign(lgInt);
  if(longIntegerCompareInt(lgInt, stringGlyphLen) >= 0) {
    int32ToLongInteger(stringGlyphLen, lgInt);
  }

  longIntegerToInt32(lgInt, steps);
  longIntegerFree(lgInt);

  if(!saveLastX()) {
    return;
  }

  if(steps > 0) {
    for(glyphPointer=0, steps=stringGlyphLen-steps; steps > 0; steps--) {
      glyphPointer = stringNextGlyph(REGISTER_STRING_DATA(regist), glyphPointer);
    }

    *(REGISTER_STRING_DATA(regist) + glyphPointer) = 0;
  }
}



void fnAlphaSL(uint16_t regist) {
  longInteger_t lgInt;
  int16_t stringGlyphLen, steps, glyphPointer;
  char *ptr;

  if(getRegisterDataType(regist) != dtString) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot use " STD_alpha "SL on %s", getRegisterDataTypeName(regist, true, false));
      moreInfoOnError("In function fnAlphaSL:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  ptr = REGISTER_STRING_DATA(regist);
  stringGlyphLen = stringGlyphLength(ptr);
  if(stringGlyphLen == 0) {
    return;
  }

  longIntegerInit(lgInt);
  // lgInt pre-init. Each converter branch must longIntegerFree() the pre-init first, or it leaks.
  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      longIntegerFree(lgInt); // the converter re-initialises lgInt; free the pre-init first
      convertLongIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    case dtReal34: {
      if(real34CompareAbsGreaterThan(REGISTER_REAL34_DATA(REGISTER_X), const34_1e6)) {
        uInt32ToLongInteger(1000000u, lgInt);
      }
      else {
        longIntegerFree(lgInt); // the converter re-initialises lgInt; free the pre-init first
        convertReal34ToLongInteger(REGISTER_REAL34_DATA(REGISTER_X), lgInt, DEC_ROUND_DOWN);
      }
      break;
    }

    case dtShortInteger: {
      longIntegerFree(lgInt); // the converter re-initialises lgInt; free the pre-init first
      convertShortIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "cannot " STD_alpha "SL when X is %s", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function fnAlphaSL:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      longIntegerFree(lgInt);
      return;
    }
  }

  longIntegerSetPositiveSign(lgInt);
  if(longIntegerCompareInt(lgInt, stringGlyphLen) >= 0) {
    int32ToLongInteger(stringGlyphLen, lgInt);
  }

  longIntegerToInt32(lgInt, steps);
  longIntegerFree(lgInt);

  if(!saveLastX()) {
    return;
  }

  if(steps > 0) {
    for(glyphPointer=0; steps > 0; steps--) {
      glyphPointer = stringNextGlyph(ptr, glyphPointer);
    }

   xcopy(ptr, ptr + glyphPointer, stringByteLength(ptr + glyphPointer) + 1);
  }
}

//
// New 42 alpha functions
//
void fnSet42Alpha(uint16_t regist) {
  if((regist <= LAST_SPARE_REGISTER) || (FIRST_NAMED_VARIABLE <= regist && regist < FIRST_NAMED_VARIABLE + numberOfNamedVariables)) {  // Global register or named variable only
    fnClearAlpha(regist);
    alphaRegister = regist;
  }
  else if(FIRST_RESERVED_VARIABLE <= regist && regist <= LAST_RESERVED_VARIABLE) {
    displayCalcErrorMessage(ERROR_RESERVED_VARIABLE_NAME, ERR_REGISTER_LINE);
    sprintf(errorMessage, "%s", allReservedVariables[regist - FIRST_RESERVED_VARIABLE].reservedVariableName + 1);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function fnSet42Alpha:", errorMessage, " is a reserved variable.", NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "%04d", regist);
      moreInfoOnError("In function fnSet42Alpha:", errorMessage, " is out of range.", NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}

void fnSet42AlphaX(uint16_t regist) {
  longInteger_t reg;
  int16_t xx;

  if(getRegisterDataType(regist) == dtString) {  // Named variable, created when missing
    xx = indirectAddressing(regist, INDPM_REGISTER, 0, 0, true);
    if(xx != FAILED_INDIRECTION) {
      fnSet42Alpha(xx);
    }
    return;
  }
  if(!getRegisterAsLongInt(regist, reg, NULL)) {
    goto end;
  }
  longIntegerToInt32(reg, xx);
  if((xx < 0) || (xx > 255)) {
    xx = INVALID_VARIABLE; // force out of range
  }
  fnSet42Alpha(regKStoC(xx));
end:
  longIntegerFree(reg);
}


void fnGet42Alpha(uint16_t unusedButMandatoryParameter) {
  longInteger_t alphaR;
  if(alphaRegister <= LAST_SPARE_REGISTER) {  // Global register
    liftStack();
    longIntegerInit(alphaR);
    uInt32ToLongInteger(regCtoKS(alphaRegister), alphaR);
    convertLongIntegerToLongIntegerRegister(alphaR, REGISTER_X);
    longIntegerFree(alphaR);
  }
  else if(FIRST_NAMED_VARIABLE <= alphaRegister && alphaRegister < FIRST_NAMED_VARIABLE + numberOfNamedVariables) {  // Named variable
    int16_t nameLength = 16;
    liftStack();
    reallocateRegister(REGISTER_X, dtString, TO_BLOCKS(nameLength), amNone);
    xcopy(REGISTER_STRING_DATA(REGISTER_X), allNamedVariables[alphaRegister - FIRST_NAMED_VARIABLE].variableName + 1, nameLength);
  }
  else {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "%04d", alphaRegister);
      moreInfoOnError("In function fnGet42Alpha:", errorMessage, " is out of range.", NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void truncateAlphaRegisterTo44Char() {
  int16_t stringGlyphLen, steps, glyphPointer;
  char *ptr;

  ptr = REGISTER_STRING_DATA(alphaRegister);
  stringGlyphLen = stringGlyphLength(ptr);

  if(stringGlyphLen <= 44) {
    return;
  }
  else {
    steps = (stringGlyphLen - 44);
    for(glyphPointer=0; steps > 0; steps--) {
      glyphPointer = stringNextGlyph(ptr, glyphPointer);
    }
    xcopy(ptr, ptr + glyphPointer, stringByteLength(ptr + glyphPointer) + 1);
  }
}


void fn42Alpha(uint16_t unusedButMandatoryParameter) {
  char *alphaString = (programRunStop == PGM_RUNNING ? tmpStringLabelOrVariableName : aimBuffer);
  reallocateRegister(alphaRegister, dtString, TO_BLOCKS(stringByteLength(alphaString) + 1), amNone);
  xcopy(REGISTER_STRING_DATA(alphaRegister), alphaString, stringByteLength(alphaString) + 1);
}


void fn42Append(uint16_t unusedButMandatoryParameter) {
  char *alphaString = (programRunStop == PGM_RUNNING ? tmpStringLabelOrVariableName : aimBuffer);
  copySourceRegisterToDestRegister(REGISTER_X, LAST_TEMP_REGISTER);
  reallocateRegister(REGISTER_X, dtString, TO_BLOCKS(stringByteLength(alphaString) + 1), amNone);
  xcopy(REGISTER_STRING_DATA(REGISTER_X), alphaString, stringByteLength(alphaString) + 1);
  fnStoreAdd(alphaRegister);
  truncateAlphaRegisterTo44Char();
  copySourceRegisterToDestRegister(LAST_TEMP_REGISTER, REGISTER_X);
}


void fn42AlphaRotate(uint16_t unusedButMandatoryParameter) {
  longInteger_t lgInt;

  if(getRegisterDataType(alphaRegister) != dtString) {
    displayCalcErrorMessage(ERROR_NO_STRING_IN_ALPHA_REGISTER, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot use 42AROT on %s", getRegisterDataTypeName(alphaRegister, true, false));
      moreInfoOnError("In function fn42AlphaRotate:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  longIntegerInit(lgInt);
  // lgInt pre-init. Each converter branch must longIntegerFree() the pre-init first, or it leaks.
  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      longIntegerFree(lgInt); // the converter re-initialises lgInt; free the pre-init first
      convertLongIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    case dtReal34: {
      const real34_t *count = REGISTER_REAL34_DATA(REGISTER_X);
      const bool_t negativeIntegerPart = !real34IsSpecial(count) && !real34IsZero(count) && real34IsNegative(count)
                                      && real34GetExponent(count) + real34Digits(count) > 0;

      longIntegerFree(lgInt);
      if(negativeIntegerPart) {
        fnAlphaRR(alphaRegister);
      }
      else {
        fnAlphaRL(alphaRegister);
      }
      return;
    }

    case dtShortInteger: {
      longIntegerFree(lgInt); // the converter re-initialises lgInt; free the pre-init first
      convertShortIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "cannot 42AROT when X is %s", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function fn42AlphaRotate:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      longIntegerFree(lgInt);
      return;
    }
  }
  if(longIntegerIsNegative(lgInt)) {
    fnAlphaRR(alphaRegister);
  }
  else {
    fnAlphaRL(alphaRegister);
  }
  longIntegerFree(lgInt);
}


void fn42AlphaShift(uint16_t unusedButMandatoryParameter) {
  int16_t stringGlyphLen, steps, glyphPointer;
  char *ptr;

  if(getRegisterDataType(alphaRegister) != dtString) {
    displayCalcErrorMessage(ERROR_NO_STRING_IN_ALPHA_REGISTER, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot use 42ASHF on %s", getRegisterDataTypeName(alphaRegister, true, false));
      moreInfoOnError("In function fn42AlphaShift:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  ptr = REGISTER_STRING_DATA(alphaRegister);
  stringGlyphLen = stringGlyphLength(ptr);

  steps = (stringGlyphLen < 6 ? stringGlyphLen : 6);

  for(glyphPointer=0; steps > 0; steps--) {
    glyphPointer = stringNextGlyph(ptr, glyphPointer);
  }

   xcopy(ptr, ptr + glyphPointer, stringByteLength(ptr + glyphPointer) + 1);
}


void fnAlphaIP(uint16_t regist) {
  if(programRunStop == PGM_RUNNING) {
    copySourceRegisterToDestRegister(REGISTER_Y, SAVED_REGISTER_Y);    // Save register Y
    copySourceRegisterToDestRegister(REGISTER_X, SAVED_REGISTER_X);    // Save register X
    copySourceRegisterToDestRegister(REGISTER_L, SAVED_REGISTER_L);    // Save register L
  }

  // convert X to long integer
  switch(getRegisterDataType(REGISTER_X)) {
    case dtShortInteger: {
      convertShortIntegerRegisterToLongIntegerRegister(REGISTER_X, REGISTER_X);
      break;
    }

    case dtReal34: {
      integerPartReal(DEC_ROUND_DOWN);
      fnJM_2SI(NOPARAM);
      break;
    }

    case dtLongInteger: {
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "cannot " STD_alpha "IP when X is %s", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function fnAlphaIP:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1
      return;
    }
  }

  // convert X to string without separators
  fnClearAlpha(REGISTER_Y);

  uint8_t grpGroupingLeftOld  = grpGroupingLeft;
  grpGroupingLeft  = 0;                   // remove  IP separators
  addition[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)]();  // Convert Register X to a string
  grpGroupingLeft  = grpGroupingLeftOld;  // restore IP separators

  // append X to the destination register - result is a string
  if(lastErrorCode == ERROR_NONE) {
    if(regist != REGISTER_Y) {
      copySourceRegisterToDestRegister(regist, REGISTER_Y);
    }
    else {
      copySourceRegisterToDestRegister(SAVED_REGISTER_Y, REGISTER_Y);
    }

    addition[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)]();

    if(lastErrorCode == ERROR_NONE) {                  // the append may overflow the string limit; leave the destination untouched when it does
      copySourceRegisterToDestRegister(REGISTER_X, regist);
    }
  }

  // restore, X, Y and L
  if((regist != REGISTER_Y) || (lastErrorCode != ERROR_NONE)) {
    copySourceRegisterToDestRegister(SAVED_REGISTER_Y, REGISTER_Y);  // Restore register Y
  }
  copySourceRegisterToDestRegister(SAVED_REGISTER_X, REGISTER_X);    // Restore register X
  copySourceRegisterToDestRegister(SAVED_REGISTER_L, REGISTER_L);    // Restore register L
}


#if defined(OPTION_ASTRING)
static uint16_t _getGlyphCode(char *ptrString) {
  uint16_t glyph;

  glyph = ptrString[0] & 0xff;
  if(glyph & 0x80) {
    glyph = (glyph << 8) | (ptrString[1] & 0xff);;
  }
  return (glyph);
}


static bool_t _isSameGlyph(uint16_t glyph, char *ptrString) {
    if(glyph & 0x8000) {
      if((glyph  >> 8) != (*ptrString++ & 0xff)) {
        return false;
      }
    }
    return((glyph & 0xff) == (*ptrString & 0xff) ? true : false);
}


static void _toUpperOrLowerCase(char *ptrString, bool_t toUpper) {
  int16_t i, j, lgString, pos;
  uint16_t glyph, currentGlyph, newGlyph;

  lgString = stringGlyphLength(ptrString);
  pos = 0;

  for(i=1; i<=lgString; i++) {
    glyph = _getGlyphCode(ptrString + pos);
    j = 0;
    while(upperLowerTable[j].upper[0] != 0) {
      currentGlyph = (toUpper ? _getGlyphCode((char *)upperLowerTable[j].lower) : _getGlyphCode((char *)upperLowerTable[j].upper));
      if(glyph == currentGlyph) {
        newGlyph = (toUpper ? _getGlyphCode((char *)upperLowerTable[j].upper) : _getGlyphCode((char *)upperLowerTable[j].lower));
        if(glyph & 0x8000) {
          ptrString[pos]     = newGlyph >> 8;
          ptrString[pos + 1] = newGlyph & 0xff;
        }
        else {
          ptrString[pos]     = newGlyph;
        }
        break;
      }
      j++;
    }
    pos += (glyph & 0x8000) ? 2 : 1;
  }
}

void fnAlphaLower(uint16_t regist) {
  char *ptrString;

  if(getRegisterDataType(regist) != dtString) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot convert %s to Lower Case", getRegisterDataTypeName(regist, true, false));
      moreInfoOnError("In function fnAlphaLower:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  ptrString = REGISTER_STRING_DATA(regist);
  _toUpperOrLowerCase(ptrString, SF_TO_LOWER_CASE);
}

void fnAlphaUpper(uint16_t regist) {
  char *ptrString;

  if(getRegisterDataType(regist) != dtString) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot convert %s to Upper Case", getRegisterDataTypeName(regist, true, false));
      moreInfoOnError("In function fnAlphaUpper:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  ptrString = REGISTER_STRING_DATA(regist);
  _toUpperOrLowerCase(ptrString, SF_TO_UPPER_CASE);
}

static void _alphaMid(char *ptrString, int32_t start, int32_t len) {
  int16_t pos1, pos2;
  int32_t i, lgString;

  lgString = stringGlyphLength(ptrString);
  if(start == 0) {
    len = 0;
  }
  if(start > lgString) {
    start = lgString;
    len = 0;
  }
  else if(start + len - 1 > lgString) {
    len = lgString - start + 1;
  }
  pos1 = 0;
  if(start > 1) {
    for(i=1; i<start; i++) {
      pos1 = stringNextGlyph(ptrString, pos1);
    }
  }
  pos2 = 0;
  for(i=0; i<len; i++) {
    if(ptrString[pos1] & 0x80) {
      tmpString[pos2++] = ptrString[pos1++];
    }
    tmpString[pos2++] = ptrString[pos1++];
  }
  tmpString[pos2] = 0;
  reallocateRegister(REGISTER_X, dtString, TO_BLOCKS(stringByteLength(tmpString) + 1), amNone);
  xcopy(REGISTER_STRING_DATA(REGISTER_X), tmpString, stringByteLength(tmpString) + 1);
}

static void _alphaLeftMidRight(uint16_t regist, uint8_t type) {
  char *ptrString;
  int32_t lgString, start, len;
  longInteger_t lgInt;

  if(getRegisterDataType(regist) != dtString) {
    badTypeError(regist);
    return;
  }

  ptrString = REGISTER_STRING_DATA(regist);
  lgString = stringGlyphLength(ptrString);

  if(!getRegisterAsLongInt(REGISTER_X, lgInt, NULL)) {
    goto end;
  }
  longIntegerToInt32(lgInt, len);
  if(len < 0) {
    len = 0;
  }
  else if(len > lgString) {
    len = lgString;
  }

  start = 0;
  if(type == SF_MID) {
    longIntegerFree(lgInt);
    if(!getRegisterAsLongInt(REGISTER_Y, lgInt, NULL)) {
      goto end;
    }
    longIntegerToInt32(lgInt, start);
    if(start < 0) {
      start = 0;
    }
  }

  if(!saveLastX()) {
    goto end;
  }

  switch(type) {
    case SF_LEFT: {
      _alphaMid(ptrString, 1, len);
      break;
    }
    case SF_MID: {
      _alphaMid(ptrString, start, len);
      fnDropY(NOPARAM);
      break;
    }
    case SF_RIGHT: {
      _alphaMid(ptrString, lgString - len + 1, len);
      break;
    }
  }
end:
  longIntegerFree(lgInt);
}


void fnAlphaLeft(uint16_t regist) {
  _alphaLeftMidRight(regist, SF_LEFT);
}


void fnAlphaMid(uint16_t regist) {
  _alphaLeftMidRight(regist, SF_MID);
}


void fnAlphaRight(uint16_t regist) {
  _alphaLeftMidRight(regist, SF_RIGHT);
}


void fnAlphaRev(uint16_t regist) {
  if(getRegisterDataType(regist) != dtString) {
    badTypeError(regist);
    return;
  }

  char *ptrString = REGISTER_STRING_DATA(regist);
  int32_t lgString = stringGlyphLength(ptrString);

  if(strlen(ptrString) >= TMP_STR_LENGTH) {   // >= : the reversed copy needs strlen + 1 bytes (trailing 0) in tmpString[TMP_STR_LENGTH]
    displayCalcErrorMessage(ERROR_INPUT_TOO_LONG, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "string in regist %d is too long, size %d bytes doesn't fit in tmpString (%d bytes max)", regist, (uint16_t) strlen(ptrString), TMP_STR_LENGTH);
      moreInfoOnError("In function fnAlphaRev:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  char *ptrTmp = tmpString;
  int16_t pos = stringLastGlyph(ptrString);
  uint16_t glyph;

  for(int16_t i = 1; i <= lgString; i++) {
    glyph = _getGlyphCode(ptrString + pos);
    if(glyph & 0x8000) {
      *ptrTmp++ = glyph >> 8;
    }
    *ptrTmp++ = glyph & 0xff;
    pos = stringPrevGlyph(ptrString, pos);
  }
  *ptrTmp = 0;
  strcpy(ptrString, tmpString);
  tmpString[0] = 0;  // clear tmpString
}


void fnAlphaTrim(uint16_t regist) {
  char *ptrString;
  int32_t lgString;
  int16_t glyph;

  if(getRegisterDataType(regist) != dtString) {
    badTypeError(regist);
    return;
  }

  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger:
    case dtReal34:
    case dtShortInteger: {
      _doXToAlpha(REGISTER_X);  // convert X to a single character
      break;
    }
    case dtString: {
      break;
    }
    default: {
      badTypeError(REGISTER_X);
      return;
    }
  }
  ptrString = REGISTER_STRING_DATA(REGISTER_X);
  lgString = stringGlyphLength(ptrString);
  if(lgString != 1) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "for "STD_alpha "TRIM, X must be a single character. Here X = %s", ptrString);
      moreInfoOnError("In function fnAlphaTrim:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1
    goto end;
  }

  glyph = _getGlyphCode(ptrString);
  ptrString = REGISTER_STRING_DATA(regist);
  lgString = stringGlyphLength(ptrString);

  // Trim character at the beginning of the string
  int16_t pos = 0;
  while(_isSameGlyph(glyph, ptrString + pos)) {
    pos = stringNextGlyph(ptrString, pos);
  }
  memmove(ptrString, ptrString + pos, strlen(ptrString + pos) + 1);

  // Trim character at the end of the string
  pos = stringLastGlyph(ptrString);
  while(_isSameGlyph(glyph, ptrString + pos)) {
    ptrString[pos] = 0;
    pos = stringLastGlyph(ptrString);
  }

end:
  copySourceRegisterToDestRegister(SAVED_REGISTER_X, REGISTER_X);    // Restore register X
  copySourceRegisterToDestRegister(REGISTER_L, SAVED_REGISTER_L);    // Save register L
}
#endif // OPTION_ASTRING


//
// 42 functions wrappers to 47 native functions
//
void fn42Aip(uint16_t unusedButMandatoryParameter) {
  fnAlphaIP(alphaRegister);
  truncateAlphaRegisterTo44Char();
}

void fn42Aleng(uint16_t unusedButMandatoryParameter) {
  fnAlphaLeng(alphaRegister);
}

void fn42Atox(uint16_t unusedButMandatoryParameter) {
  fnAlphaToX(alphaRegister);
}

void fn42Xtoa(uint16_t unusedButMandatoryParameter) {
  fnXToAlpha(alphaRegister);
  truncateAlphaRegisterTo44Char();
}

void fn42Aview(uint16_t unusedButMandatoryParameter) {
  lastFunc = ITM_AVIEW;
  fnAview(alphaRegister);
}

void fn42Cla(uint16_t unusedButMandatoryParameter) {
  fnClearAlpha(alphaRegister);
}

void fn42Posa(uint16_t unusedButMandatoryParameter) {
  fnAlphaPos(alphaRegister);
}

void fn42Pra(uint16_t unusedButMandatoryParameter) {
  fnP_Alpha(alphaRegister);
}

void fn42Prompt(uint16_t unusedButMandatoryParameter) {
  lastFunc = ITM_PROMPT;
  fnPrompt(alphaRegister);
}
