// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file registerBrowser.c The register browser application
 ***********************************************/

#include "c47.h"



#if !defined(TESTSUITE_BUILD)
#if !defined(SAVE_SPACE_DM42_8)
  static void _showRegisterInRbr(calcRegister_t regist, int16_t registerNameWidth) {
    switch(getRegisterDataType(regist)) {
      case dtReal34: {
        if(showContent) {
          real34ToDisplayString(REGISTER_REAL34_DATA(regist), getRegisterAngularMode(regist), tmpString, &standardFont, SCREEN_WIDTH - 1 - registerNameWidth, 34, !LIMITEXP, !FRONTSPACE, LIMITIRFRAC);
        }
        else {
          sprintf(tmpString, "%d bytes", (int16_t)REAL34_SIZE_IN_BYTES);
        }
        break;
      }

      case dtComplex34: {
        if(showContent) {
          complex34ToDisplayString(REGISTER_COMPLEX34_DATA(regist), tmpString, &standardFont, SCREEN_WIDTH - 1 - registerNameWidth, 34, !LIMITEXP, !FRONTSPACE, LIMITIRFRAC, getComplexRegisterAngularMode(regist), getComplexRegisterPolarMode(regist));
        }
        else {
          sprintf(tmpString, "%d bytes", (int16_t)COMPLEX34_SIZE_IN_BYTES);
        }
        break;
      }

      case dtLongInteger: {
        if(showContent) {
          if(regist >= FIRST_RESERVED_VARIABLE) {
            copySourceRegisterToDestRegister(regist, TEMP_REGISTER_1);
            longIntegerRegisterToDisplayString(TEMP_REGISTER_1, tmpString, TMP_STR_LENGTH, SCREEN_WIDTH - 1 - registerNameWidth, 50, false);
          }
          else if(getRegisterLongIntegerSign(regist) == LI_NEGATIVE) {
            longIntegerRegisterToDisplayString(regist, tmpString, TMP_STR_LENGTH, SCREEN_WIDTH - 1 - registerNameWidth, 50, false);   //JM added last parameter: Allow LARGELI
          }
          else {
            longIntegerRegisterToDisplayString(regist, tmpString, TMP_STR_LENGTH, SCREEN_WIDTH - 9 - registerNameWidth, 50, false);   //JM added last parameter: Allow LARGELI
          }
        }
        else {
          if(regist >= FIRST_RESERVED_VARIABLE) {
            sprintf(tmpString, "4 bytes");
          }
          else {
            sprintf(tmpString, "%" PRIu32 " bits " STD_CORRESPONDS_TO " 4+%" PRIu32 " bytes", (uint32_t)TO_BYTES(getRegisterMaxDataLengthInBlocks(regist)) * 8, (uint32_t)TO_BYTES(getRegisterMaxDataLengthInBlocks(regist)));
          }
        }
        break;
      }

      case dtShortInteger: {
        if(showContent) {
          shortIntegerToDisplayString(regist, tmpString, false, noBaseOverride);
        }
        else {
          strcpy(tmpString, "64 bits " STD_CORRESPONDS_TO " 8 bytes");
        }
        break;
      }

      case dtString: {
        if(showContent) {
          strcpy(tmpString, STD_LEFT_SINGLE_QUOTE);
          strncat(tmpString, REGISTER_STRING_DATA(regist), stringByteLength(REGISTER_STRING_DATA(regist)) + 1);
          strcat(tmpString, STD_RIGHT_SINGLE_QUOTE);
          if(stringWidth(tmpString, &standardFont, false, true) >= SCREEN_WIDTH - 12 - registerNameWidth) { // 12 is the width of STD_ELLIPSIS
            *(stringAfterPixels(tmpString, &standardFont, SCREEN_WIDTH - 12 - registerNameWidth, false, true)) = 0;
            strcat(tmpString + stringByteLength(tmpString), STD_ELLIPSIS);
          }
        }
        else {
          sprintf(tmpString, "%" PRIu32 " character%s " STD_CORRESPONDS_TO " 4+%" PRIu32 " bytes", (uint32_t)stringGlyphLength(REGISTER_STRING_DATA(regist)), stringGlyphLength(REGISTER_STRING_DATA(regist))==1 ? "" : "s", (uint32_t)TO_BYTES(getRegisterMaxDataLengthInBlocks(regist)));
        }
        break;
      }

      case dtTime: {
        if(showContent) {
          timeToDisplayString(regist, tmpString, true);
        }
        else {
          sprintf(tmpString, "%d bytes", (int16_t)REAL34_SIZE_IN_BYTES);
        }
        break;
      }

      case dtDate: {
        if(showContent) {
          dateToDisplayString(regist, tmpString);
        }
        else {
          sprintf(tmpString, "%d bytes", (int16_t)REAL34_SIZE_IN_BYTES);
        }
        break;
      }

      case dtReal34Matrix: {
        if(showContent) {
          real34MatrixToDisplayString(regist, tmpString);
        }
        else {
          matrixHeader_t *matrixHeader = REGISTER_MATRIX_HEADER(regist);
          uint16_t elements = matrixHeader->matrixRows * matrixHeader->matrixColumns;
          sprintf(tmpString, "%" PRIu16 " element%s " STD_CORRESPONDS_TO " %" PRIu32 "+%" PRIu32 " bytes",
                             elements,           elements==1 ? "" : "s",   (uint32_t)sizeof(matrixHeader_t),
                                                                                       (uint32_t)TO_BYTES(elements * REAL34_SIZE_IN_BLOCKS));
        }
        break;
      }

      case dtComplex34Matrix: {
        if(showContent) {
          complex34MatrixToDisplayString(regist, tmpString);
        }
        else {
          matrixHeader_t* matrixHeader = REGISTER_MATRIX_HEADER(regist);
          uint16_t elements = matrixHeader->matrixRows * matrixHeader->matrixColumns;
          sprintf(tmpString, "%" PRIu16 " element%s " STD_CORRESPONDS_TO " %" PRIu32 "+%" PRIu32 " bytes",
                              elements,          elements ==1 ? "" : "s",  (uint32_t)sizeof(matrixHeader_t),
                                                                                       (uint32_t)TO_BYTES(elements * COMPLEX34_SIZE_IN_BLOCKS));
        }
        break;
      }

      case dtConfig: {
        if(showContent) {
          strcpy(tmpString, "Configuration data");
        }
        else {
          sprintf(tmpString, "%d bytes", (int16_t)CONFIG_SIZE_IN_BYTES);
        }
        break;
      }

      default: {
        sprintf(tmpString, "Data type %s: to be coded", getDataTypeName(getRegisterDataType(regist), false, true));
      }
    }
  }

  static void registerName(char *s, calcRegister_t regist) {
    if(REGISTER_X <= regist && regist <= REGISTER_W) {
      tmpString[0] = letteredRegisterName(regist);
      strcpy(tmpString + 1, ":");
    }
    else {
      sprintf(tmpString, "R%02d:", regist);
    }
  }
  #endif // !SAVE_SPACE_DM42_8

  void registerBrowser(uint16_t unusedButMandatoryParameter) {
  #if !defined(SAVE_SPACE_DM42_8)
    int16_t registerNameWidth;

    hourGlassIconEnabled = false;

    if(calcMode != CM_REGISTER_BROWSER) {
      if(calcMode == CM_AIM) {
        hideCursor();
        cursorEnabled = false;
      }

      previousCalcMode = calcMode;
      calcMode = CM_REGISTER_BROWSER;
      clearSystemFlag(FLAG_ALPHA);
      return;
    }

    if(currentRegisterBrowserScreen == 9999) { // Init
      currentRegisterBrowserScreen = REGISTER_X;
      rbrMode = RBR_GLOBAL;
      showContent = true;
      rbr1stDigit = true;
    }

    if(rbrMode == RBR_GLOBAL) { // Global registers
      for(int16_t row=0; row<10; row++) {
        calcRegister_t regist = modulo(currentRegisterBrowserScreen + row, LAST_GLOBAL_REGISTER_SCREEN + RBR_INCDEC1);
        if (regist <= LAST_SPARE_REGISTER){
          registerName(tmpString, regist);

          // register name or number
          registerNameWidth = showString(tmpString, &standardFont, 1, 219 - 22 * row, vmNormal, false, true);

          if(   (regist <  REGISTER_X && regist % 5 == 4)
            || (regist >= REGISTER_X && regist % 4 == 3)) {
            drawSinglePixelFullWidthLine(218 - 22 * row);
          }

          _showRegisterInRbr(regist, registerNameWidth);

          showString(tmpString, &standardFont, SCREEN_WIDTH - stringWidth(tmpString, &standardFont, false, true) - 1, 219-22*row, vmNormal, false, true);
        }
      }
    }

    else if(rbrMode == RBR_LOCAL) { // Local registers
      if(currentNumberOfLocalRegisters != 0) { // Local registers are allocated
        for(int16_t row=0; row<10; row++) {
          calcRegister_t regist = currentRegisterBrowserScreen + row;
          if(regist < FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters) {
            sprintf(tmpString, "R.%02d:", regist - FIRST_LOCAL_REGISTER);

            // register number
            registerNameWidth = showString(tmpString, &standardFont, 1, 219 - 22 * row, vmNormal, true, true);

            if(regist % 5 == 1) {
              drawSinglePixelFullWidthLine(218 - 22 * row);
            }

            _showRegisterInRbr(regist, registerNameWidth);

            showString(tmpString, &standardFont, SCREEN_WIDTH - stringWidth(tmpString, &standardFont, false, true), 219 - 22 * row, vmNormal, false, true);
          }
        }
      }

      else { // no local register allocated
        rbrMode = RBR_NAMED;
        registerBrowser(NOPARAM);
      }
    }

    else if(rbrMode == RBR_NAMED) { // Named variables
      for(int16_t row=0; row<10; row++) {
        calcRegister_t regist = currentRegisterBrowserScreen + row;
        if(regist < FIRST_NAMED_VARIABLE + numberOfNamedVariables) { // Named variables
          stringCopy(tmpString, (char *)allNamedVariables[regist - FIRST_NAMED_VARIABLE].variableName + 1);
          stringCopy(tmpString + stringByteLength(tmpString), ":");

          // variable name
          registerNameWidth = showString(tmpString, &standardFont, 1, 219 - 22 * row, vmNormal, true, true);

          if((regist % 5 == 1) || (regist == FIRST_NAMED_VARIABLE + numberOfNamedVariables - 1)) {
            drawSinglePixelFullWidthLine(218 - 22 * row);
          }

          _showRegisterInRbr(regist, registerNameWidth);

          showString(tmpString, &standardFont, SCREEN_WIDTH - stringWidth(tmpString, &standardFont, false, true), 219 - 22 * row, vmNormal, false, true);
        }
        else { // Reserved variables
          if(regist < FIRST_RESERVED_VARIABLE) {
            regist -= FIRST_NAMED_VARIABLE + numberOfNamedVariables;
            regist += FIRST_RESERVED_VARIABLE + NUMBER_OF_LETTERED_VARIABLES;
          }

          if(regist <= LAST_RESERVED_VARIABLE) { // Named variables
            sprintf(tmpString, "%s:", (char *)allReservedVariables[regist - FIRST_RESERVED_VARIABLE].reservedVariableName + 1);

            // variable name
            registerNameWidth = showString(tmpString, &standardFont, 1, 219 - 22 * row, vmNormal, true, true);

            if(regist % 5 == 1) {
              drawSinglePixelFullWidthLine(218 - 22 * row);
            }

            _showRegisterInRbr(regist, registerNameWidth);

            showString(tmpString, &standardFont, SCREEN_WIDTH - stringWidth(tmpString, &standardFont, false, true), 219 - 22 * row, vmNormal, false, true);
          }
        }
      }
    }
  #endif // !SAVE_SPACE_DM42_8
}
#endif // !TESTSUITE_BUILD
