// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

/*
Math changes:

1. addon.c: Added fnArg_all which uses fnArg, but gives a result of 0 for a real
   and longint input. The testSuite is not ifluenced. Not needed to modify |x|,
   as it already works for a real and longint.
   (testSuite not in use for fnArg, therefore also not added)

2. bufferize.c: closenim: changed the default for (0 CC EXIT to 0) instead of i.
   (testSuite not ifluenced).

Todo


All the below: because both Last x and savestack does not work due to multiple steps.

  5. Added Star > Delta. Change and put in separate c file, and sort out savestack.
  6. vice versa
  7. SYM>ABC
  8. ABC>SYM
  9. e^theta. redo in math file,
  10. three phase Ohms Law: 17,18,19


 Check for savestack in jm.c
*/

#if !defined(SAVE_SPACE_DM42_23_EDIT2)
#if !defined(TESTSUITE_BUILD)
  static void _getStringLabelOrVariableName(uint8_t *stringAddress) {
    uint8_t stringLength = *(uint8_t *)(stringAddress++);
    xcopy(tmpStringLabelOrVariableName, stringAddress, stringLength);
    tmpStringLabelOrVariableName[stringLength] = 0;
  }
#endif // !TESTSUITE_BUILD
#endif // !SAVE_SPACE_DM42_23_EDIT2


#if !defined(SAVE_SPACE_DM42_22_EDIT1)
void _fractionToString(calcRegister_t regist, char *displayString, int16_t *lessEqualGreater) {
  int16_t  sign;
  uint64_t intPart, numer, denom;

  fraction(regist, &sign, &intPart, &numer, &denom, lessEqualGreater);

  if(getSystemFlag(FLAG_PROPFR)) { // a b/c
    sprintf(displayString, "%s%" PRIu32 " %" PRIu32 "/%" PRIu32, (sign == -1 ? "-" : "+"), (uint32_t)intPart, (uint32_t)numer, (uint32_t)denom);
  }

  else { // FT_IMPROPER d/
    sprintf(displayString, "%s0 %" PRIu32 "/%" PRIu32, (sign == -1 ? "-" : "+"), (uint32_t)numer, (uint32_t)denom);

  }
}

void _shortIntegerToString(calcRegister_t regist, char *displayString) {
  int16_t i, j, k, unit, base;
  uint64_t number, sign;

  base    = getRegisterTag(regist);
  number  = *(REGISTER_SHORT_INTEGER_DATA(regist));

  if(base <= 1 || base >= 17) {
    sprintf(errorMessage, commonBugScreenMessages[bugMsgValueFor], "_shortIntegerToString", base, "base");
    displayBugScreen(errorMessage);
    base = 10;
  }

  //number &= shortIntegerMask;

  if(shortIntegerMode == SIM_UNSIGN || base == 2 || base == 4 || base == 8 || base == 16) {
    sign = 0;
  }
  else {
    sign = number & shortIntegerSignBit;
  }

  if(sign) {
    if(shortIntegerMode == SIM_2COMPL) {
      number |= ~shortIntegerMask;
      number = ~number + 1;
    }
    else if(shortIntegerMode == SIM_1COMPL) {
      number = ~number;
    }
    else if(shortIntegerMode == SIM_SIGNMT) {
      number &= ~shortIntegerSignBit;
    }
    else {
      sprintf(errorMessage, commonBugScreenMessages[bugMsgValueFor], "_shortIntegerToString", shortIntegerMode, "shortIntegerMode");
      displayBugScreen(errorMessage);
    }

    number &= shortIntegerMask;
  }

  i = ERROR_MESSAGE_LENGTH / 2;

  if(number == 0) {
    displayString[i++] = '0';
  }

  while(number) {
    unit = number % base;
    number /= base;
    displayString[i++] = baseDigits[unit];
  }

  if(sign) {
    displayString[i++] = '-';
  }
  else {
    displayString[i++] = '+';
  }

  for(k=i-1, j=0; k>=ERROR_MESSAGE_LENGTH / 2; k--, j++) {
    if(displayString[k] == ' ') {
      displayString[j++] = STD_SPACE_PUNCTUATION[0];
      displayString[j]   = STD_SPACE_PUNCTUATION[1];
    }
    else {
      displayString[j] = displayString[k];
    }
  }
  displayString[j] = 0;

  return;
}


#if !defined(TESTSUITE_BUILD)

static void _hmsTimeToReal() {
  int16_t i = 0;
  int16_t j = 0;
  bool decimalflag = false;

  timeToDisplayString(REGISTER_X, tmpString, true);

  while(tmpString[i] != 0) {
    switch((uint8_t)tmpString[i]) {
      case '0' :
      case '1' :
      case '2' :
      case '3' :
      case '4' :
      case '5' :
      case '6' :
      case '7' :
      case '8' :
      case '9' :
      case '+' :
      case '-' :
        tmpString[j++] = (uint8_t)tmpString[i];
        break;
      case ':' :
        if(!decimalflag) {
          decimalflag = true;
          tmpString[j++] = '.';
        }
        break;
      default:
        break;
    }
    i++;
  }
  tmpString[j] = 0;

  if(tmpString[0] != 0) {
    reallocateRegister(REGISTER_X, dtReal34, REAL34_SIZE_IN_BYTES, amNone);
    stringToReal34(tmpString, REGISTER_REAL34_DATA(REGISTER_X));
  }
}


static void _real34ToNim(const real34_t *real34, char *nimInput, char *nimDisplay) {
// nimInput   : used to fill aimBuffer
// nimDisplay : used to fill nimBufferDisplay
  uint16_t i;
  uint8_t grpGroupingLeftOld  = grpGroupingLeft;
  uint8_t grpGroupingRightOld = grpGroupingRight;

  grpGroupingLeft  = 0;
  grpGroupingRight = 0;
  real34ToDisplayString(real34, amNone, tmpString, &standardFont, SCREEN_WIDTH, NUMBER_OF_DISPLAY_DIGITS, true, STD_SPACE_PUNCTUATION, true);
  grpGroupingRight = grpGroupingRightOld;
  grpGroupingLeft  = grpGroupingLeftOld;
  //printf("**[DL]** _real34ToNim tmpString %s\n",tmpString);fflush(stdout);

  bool noDisplayExponent = true;
  for(i = 0; i < strlen(tmpString); i++) {
    if((tmpString[i] == STD_SUB_10[0]) && (tmpString[i+1] == STD_SUB_10[1])) {
      noDisplayExponent = false;
    }
  }
  grpGroupingLeft  = 0;
  grpGroupingRight = 0;
  real34ToString(real34, nimDisplay);
  grpGroupingRight = grpGroupingRightOld;
  grpGroupingLeft  = grpGroupingLeftOld;
  //printf("**[DL]** _real34ToNim nimBufferDisplay %s\n",nimBufferDisplay);fflush(stdout);
  bool dotFound = false;
  if(noDisplayExponent) {                                // if no exponent in display string but exponent in real34ToString, use the display string
    for(i = 0; i < strlen(nimDisplay); i++) {
      if((nimDisplay[i] == 'e') || (nimDisplay[i] == 'E')) {
        strcpy(nimDisplay, tmpString + (tmpString[0] == '-'? 0 : 1));
        break;
      }
      if(nimDisplay[i] == '.') {
        dotFound = true;
      }
    }
    if(dotFound) {
      for(i = strlen(nimDisplay)-1; i > 0; i--) {
        if(nimDisplay[i] == '0') {
          nimDisplay[i] = 0;              // remove trailing zeros
        }
        else {
          break;
        }
      }
    }
  }
  if(real34IsPositive(real34)) {
    nimInput[0] = '+';
    strcpy(nimInput + 1, nimDisplay);
  }
  else {
    strcpy(nimInput, nimDisplay);
  }
  //printf("**[DL]** _real34ToNim nimInput %s\n",nimInput);fflush(stdout);
  bool exponentFound = false;
  dotFound = false;
  for(i = 0; i < strlen(nimInput); i++) {
    if(nimInput[i] == 'E') {
      nimInput[i] = 'e';
      dotFound = true;
      exponentFound = true;
      exponentSignLocation = i + 1;
      nimNumberPart = NP_REAL_EXPONENT;
    }
    if(nimInput[i] == '.') {
      dotFound = true;
      nimNumberPart = NP_REAL_FLOAT_PART;
    }
  }
  if(!dotFound) {
    nimInput[i] = '.';
    nimNumberPart = NP_REAL_FLOAT_PART;
  }
  strcpy(nimDisplay, STD_SPACE_HAIR);
  nimBufferToDisplayBuffer(nimInput, nimDisplay + 2);
  //printf("**[DL]** _real34ToNim nimDisplay %s\n",nimDisplay+2);fflush(stdout);
  for(i=stringByteLength(nimDisplay) - 1; i>0; i--) {
    if(nimDisplay[i] == (char)0xab) {    //token
      nimDisplay[i] = SEPARATOR_LEFT[0];
      if(nimDisplay[i+1] == 1) {
        nimDisplay[i+1] = SEPARATOR_LEFT[1];
      }
    }
    if(nimDisplay[i] == (char)0xbb) {    //token
      nimDisplay[i] = SEPARATOR_RIGHT[0];
      if(nimDisplay[i+1] == 1) {
        nimDisplay[i+1] = SEPARATOR_RIGHT[1];
      }
    }
  }
  if(exponentFound) {
    exponentToDisplayString(stringToInt32(nimInput + exponentSignLocation), nimDisplay + stringByteLength(nimDisplay), NULL, true);
    if(nimInput[exponentSignLocation + 1] == 0 && nimInput[exponentSignLocation] == '-') {
      strcat(nimDisplay, STD_SUP_MINUS);
    }
    else if(nimInput[exponentSignLocation + 1] == '0' && nimInput[exponentSignLocation] == '+') {
      strcat(nimDisplay, STD_SUP_0);
    }
  }
}

//static void _angle34ToNim(const real34_t *angle34, uint8_t mode, char *nimInput, char *nimDisplay) {
// nimInput   : used to fill aimBuffer
// nimDisplay : used to fill nimBufferDisplay

//}
#endif // !TESTSUITE_BUILD
#endif // !SAVE_SPACE_DM42_22_EDIT1


void fnEdit (uint16_t unusedParamButMandatory) {
  //fnEdit: this is simply the stub with the currently working edit routines, linked via ITM_EDIT, which is also located on long press Backspace.
  //All might have to be changed have a propoer generic EDIT function.
  #if !defined(TESTSUITE_BUILD)
    #if !defined(SAVE_SPACE_DM42_22_EDIT1) || !defined(SAVE_SPACE_DM42_23_EDIT2)
      int16_t index;
    #endif //!defined(SAVE_SPACE_DM42_22_EDIT1) || !defined(SAVE_SPACE_DM42_23_EDIT2)

    #if !defined(SAVE_SPACE_DM42_22_EDIT1) || !defined(SAVE_SPACE_DM42_23_EDIT2)
      uint8_t grpGroupingLeftOld;
      uint8_t grpGroupingRightOld;
    #endif
    #if !defined(SAVE_SPACE_DM42_23_EDIT2)
      char    varOrLblName[8];
    #endif

    if(tam.mode != 0) goto err;
    switch(calcMode) {
      case CM_NORMAL :
        if(currentMenu() == -MNU_EQN || currentMenu() == -MNU_Sfdx || currentMenu() == -MNU_Solver_TOOL || currentMenu() == -MNU_Sf_TOOL || currentMenu() == -MNU_GRAPHS ||
           (currentMenu() == -MNU_MVAR && (currentSolverStatus & SOLVER_STATUS_USES_FORMULA) && (currentSolverStatus & SOLVER_STATUS_INTERACTIVE))         ) {
          showSoftmenu(-MNU_EQN);
          runFunction(ITM_EQ_EDI);
        }
        else {
          switch(getRegisterDataType(REGISTER_X)) {
#if !defined(SAVE_SPACE_DM42_22_EDIT1)
            case dtLongInteger: {
              #define NIM_BUFFER_EXTENDED_LENGTH    1400      // provision for very long integers (up to 1000 digits + separators)
              memset(nimBufferDisplay, 0, NIM_BUFFER_EXTENDED_LENGTH);
              longInteger_t lgInt;
              convertLongIntegerRegisterToLongInteger(REGISTER_X, lgInt);
              longIntegerToAllocatedString(lgInt, nimBufferDisplay, NIM_BUFFER_EXTENDED_LENGTH);
              if(longIntegerIsPositiveOrZero(lgInt)) {
                aimBuffer[0] = '+';
                strcpy(aimBuffer + 1, nimBufferDisplay);
              }
              else {
                strcpy(aimBuffer, nimBufferDisplay);
              }
              longIntegerFree(lgInt);
              if(grpGroupingLeft > 0) {
                int16_t len = strlen(nimBufferDisplay);
                for(int16_t i=len - grpGroupingLeft; i>0; i-=grpGroupingLeft) {
                  if(i != 1 || nimBufferDisplay[0] != '-') {
                    if(gapItemLeft != ITM_NULL) {  // insert gapCharLeft
                      uint8_t lenGapItem = strlen(indexOfItems[gapItemLeft].itemSoftmenuName);
                      xcopy(nimBufferDisplay + i + lenGapItem, nimBufferDisplay + i, len - i + 1);
                      xcopy(nimBufferDisplay + i , indexOfItems[gapItemLeft].itemSoftmenuName, lenGapItem);
                      len += lenGapItem;
                    }
                  }
                }
              }

              // Test if long inter number display string will fit on two lines in standard font, if not do nothing (cannot edit)
              if(stringWidth(nimBufferDisplay, &standardFont, true, true) < (SCREEN_WIDTH * 2)  - 8) { // 8 is the standard font cursor width
                //printf("**[DL]** aimBuffer %s \n nimBufferDisplay %s\n",aimBuffer,nimBufferDisplay);fflush(stdout);
                calcMode = CM_NIM;
                clearSystemFlag(FLAG_ALPHA);
                freeRegisterData(REGISTER_X);
                setRegisterDataPointer(REGISTER_X, allocC47Blocks(REAL34_SIZE_IN_BLOCKS));
                setRegisterDataType(REGISTER_X, dtReal34, amNone);
                real34Zero(REGISTER_REAL34_DATA(REGISTER_X));
                hexDigits = 0;
                nimNumberPart = NP_INT_10;
                //clearRegisterLine(NIM_REGISTER_LINE, true, true);
                if(!checkHP) clearRegisterLine(NIM_REGISTER_LINE, true, true);
                xCursor = 1;
                cursorEnabled = true;
                cursorFont = &numericFont;
              }
              else {
                memset(nimBufferDisplay, 0, NIM_BUFFER_EXTENDED_LENGTH);
                aimBuffer[0] = 0;
                nimBufferDisplay[0] = 0;
              }
              break;
            }

            case dtReal34: {
              edit_dtReal34:
              grpGroupingLeftOld  = grpGroupingLeft;
              grpGroupingRightOld = grpGroupingRight;
              angularMode_t xangularMode = getRegisterAngularMode(REGISTER_X);

              //printf("**[DL]** xangularMode %d\n",xangularMode);fflush(stdout);
              memset(aimBuffer, 0, AIM_BUFFER_LENGTH);
              memset(nimBufferDisplay, 0, NIM_BUFFER_LENGTH);

              if(xangularMode == amDMS) {
                real34FromDegToDms(REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
              }

              uint16_t lessEqualGreater = 0;
              if (getSystemFlag(FLAG_FRACT)) {
                grpGroupingLeft  = 0;
                grpGroupingRight = 0;
                _fractionToString(REGISTER_X, aimBuffer, (int16_t *)&lessEqualGreater);
                grpGroupingRight = grpGroupingRightOld;
                grpGroupingLeft  = grpGroupingLeftOld;

                if(lessEqualGreater == 0) {         // display fraction
                  nimNumberPart = NP_FRACTION_DENOMINATOR;
                  strcpy(nimBufferDisplay, STD_SPACE_HAIR);
                  nimBufferToDisplayBuffer(aimBuffer, nimBufferDisplay + 2);
                  strcat(nimBufferDisplay, STD_SPACE_4_PER_EM);
                  for(index=2; aimBuffer[index]!=' '; index++) {
                  }
                  supNumberToDisplayString(stringToInt32(aimBuffer + index + 1), nimBufferDisplay + stringByteLength(nimBufferDisplay), NULL, true);

                  strcat(nimBufferDisplay, "/");

                  for(; aimBuffer[index]!='/'; index++) {
                  }
                  if(aimBuffer[++index] != 0) {
                    subNumberToDisplayString(stringToInt32(aimBuffer + index), nimBufferDisplay + stringByteLength(nimBufferDisplay), NULL);
                  }
                }
                else {    // display real34
                  _real34ToNim(REGISTER_REAL34_DATA(REGISTER_X), aimBuffer, nimBufferDisplay);
                }
              }
              else {  // display real34
                _real34ToNim(REGISTER_REAL34_DATA(REGISTER_X), aimBuffer, nimBufferDisplay);
              }
              //printf("**[DL]** dtReal34 aimBuffer %s nimBufferDisplay %s\n",aimBuffer,nimBufferDisplay);fflush(stdout);

              calcMode = CM_NIM;
              clearSystemFlag(FLAG_ALPHA);
              uint16_t dataType = getRegisterDataType(REGISTER_X);
              freeRegisterData(REGISTER_X);
              setRegisterDataPointer(REGISTER_X, allocC47Blocks(REAL34_SIZE_IN_BLOCKS));
              if((dataType == dtTime) || (dataType == dtDate)) {
                setRegisterDataType(REGISTER_X, dataType, xangularMode);   // Keep time and date datatypes
              }
              else {
                setRegisterDataType(REGISTER_X, dtReal34, xangularMode);
              }
              real34Zero(REGISTER_REAL34_DATA(REGISTER_X));
              //printf("**[DL]** AngularMode %d\n",getRegisterAngularMode(REGISTER_X));fflush(stdout);
              hexDigits = 0;
              if(!checkHP) clearRegisterLine(NIM_REGISTER_LINE, true, true);
              xCursor = 1;
              cursorEnabled = true;
              cursorFont = &numericFont;
              break;
            }

/* Temporary commented out until complex editing is fully supported
            case dtComplex34: {
              uint16_t i, j;
              uint16_t tagAngle;
              bool_t tagPolar;
              uint16_t imaginaryDisplayStart;
              int16_t realExponentSignLocation;
              real34_t real34, imag34;
              real_t real, imagIc;

              memset(aimBuffer, 0, AIM_BUFFER_LENGTH);
              memset(nimBufferDisplay, 0, NIM_BUFFER_LENGTH);

              tagPolar = getComplexRegisterPolarMode(REGISTER_X) == amPolar;
              tagAngle = getComplexRegisterAngularMode(REGISTER_X);
              if(tagPolar) {  // polar mode
                temporaryFlagPolar = true;
                real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &real);
                real34ToReal(REGISTER_IMAG34_DATA(REGISTER_X), &imagIc);

                decContext c = ctxtReal39;
                int maxExponent = max(real.exponent + real.digits, imagIc.exponent + imagIc.digits);
                c.digits = (SHOWMODE ? 39 : max(0,maxExponent) + NUMBER_OF_DISPLAY_REAL_CONTEXT_DIGITS + 2); //add 2 guard digits for Taylor etc.
                realRectangularToPolar(&real, &imagIc, &real, &imagIc, &c); // imagIc in radian
                c.digits = (SHOWMODE ? 39 : 3 + NUMBER_OF_DISPLAY_REAL_CONTEXT_DIGITS); //converting from radians to grad is the worst, i.e. x 2E2 / pi, which requires 3 digits accuarcy more
                convertAngleFromTo(&imagIc, amRadian, tagAngle, &c);

                realToReal34(&real, &real34);
                realToReal34(&imagIc, &imag34);
              }
              else { // rectangular mode
                real34Copy(REGISTER_REAL34_DATA(REGISTER_X), &real34);
                real34Copy(REGISTER_IMAG34_DATA(REGISTER_X), &imag34);
              }

              _real34ToNim(&real34, aimBuffer, nimBufferDisplay);
              realExponentSignLocation = exponentSignLocation;
              imaginaryMantissaSignLocation = strlen(aimBuffer);

              if(strncmp(nimBufferDisplay + stringByteLength(nimBufferDisplay) - 2, STD_SPACE_HAIR, 2) != 0) {
                strcat(nimBufferDisplay, STD_SPACE_HAIR);
              }
              if(real34IsPositive(&imag34)) {
                strcat(aimBuffer, "+");
                strcat(nimBufferDisplay, "+");
              }
              else {
                strcat(aimBuffer, "-");
                strcat(nimBufferDisplay, "-");
              }

              if(tagPolar) { // polar mode
                strcat(nimBufferDisplay, STD_SPACE_4_PER_EM STD_MEASURED_ANGLE STD_SPACE_4_PER_EM);
                uint16_t kk = stringByteLength(nimBufferDisplay);
                angle34ToDisplayString2(&imag34, tagAngle == amNone ? currentAngularMode : tagAngle, nimBufferDisplay + kk, 39, true, false, FULLIRFRAC);
                if(strncmp(nimBufferDisplay + kk, STD_ALMOST_EQUAL, 2) == 0) {          //if almost equal char in front of IM part, transfer it to the Left (Real) side
                  nimBufferDisplay[kk] = STD_NOCHAR;    //0x01 is the new 'no char' character
                  nimBufferDisplay[kk+1] = STD_NOCHAR;  //0x01 is the new 'no char' character
                }
              }
              else { // rectangular mode
                strcat(nimBufferDisplay, COMPLEX_UNIT);
                strcat(nimBufferDisplay, PRODUCT_SIGN);
                imaginaryDisplayStart = strlen(nimBufferDisplay);
                _real34ToNim(&imag34, aimBuffer + strlen(aimBuffer), nimBufferDisplay + strlen(nimBufferDisplay));
                aimBuffer[imaginaryMantissaSignLocation + 1] = 'i';

                // Remove SPACE HAIR and - sign in front of the imaginary part
                j = (nimBufferDisplay[imaginaryDisplayStart + 2] == '-' ? 3 : 2);
                for(i = imaginaryDisplayStart; i < strlen(nimBufferDisplay); i++) {
                  nimBufferDisplay[i] = nimBufferDisplay[i+j];
                }

                nimNumberPart = NP_COMPLEX_FLOAT_PART;
                for(i = imaginaryMantissaSignLocation; i < strlen(aimBuffer); i++) {
                  if(aimBuffer[i] == 'e') {
                    imaginaryExponentSignLocation = i + 1;
                    nimNumberPart = NP_COMPLEX_EXPONENT;
                  }
                }
              }
              printf("**[DL]** dtComplex34 aimBuffer %s nimBufferDisplay %s\n",aimBuffer,nimBufferDisplay);fflush(stdout);

              exponentSignLocation = realExponentSignLocation;
              calcMode = CM_NIM;
              clearSystemFlag(FLAG_ALPHA);
              freeRegisterData(REGISTER_X);
              setRegisterDataPointer(REGISTER_X, allocC47Blocks(REAL34_SIZE_IN_BLOCKS));
              //real34Zero(REGISTER_REAL34_DATA(REGISTER_X));
              hexDigits = 0;
              if(!checkHP) clearRegisterLine(NIM_REGISTER_LINE, true, true);
              xCursor = 1;
              cursorEnabled = true;
              cursorFont = &numericFont;
              break;
            }
*/

            case dtTime: {
              _hmsTimeToReal();
              setRegisterDataType(REGISTER_X, dtTime, amNone);  // Force time data type to preserve it when closing NIM
              goto edit_dtReal34;
              break;
            }

            case dtDate: {
              convertDateRegisterToReal34Register(REGISTER_X, REGISTER_X);
              setRegisterDataType(REGISTER_X, dtDate, amNone);  // Force date data type to preserve it when closing NIM
              goto edit_dtReal34;
              break;
            }
#endif // !SAVE_SPACE_DM42_22_EDIT1

            case dtString: {
              setSystemFlag(FLAG_ASLIFT);
              if(stringByteLength(REGISTER_STRING_DATA(REGISTER_X)) < AIM_BUFFER_LENGTH) {
                strcpy(aimBuffer, REGISTER_STRING_DATA(REGISTER_X));
                T_cursorPos = stringByteLength(aimBuffer);
                fnDrop(NOPARAM);
                shiftF = false;
                shiftG = false;
                #if !defined(TESTSUITE_BUILD)
                  calcModeAim(NOPARAM); // Alpha Input Mode
                  showSoftmenu(-MNU_ALPHA);
                #endif // !TESTSUITE_BUILD
              }
              break;
            }

            case dtReal34Matrix:
            case dtComplex34Matrix: {
              fnEditMatrix(NOPARAM);
              break;
            }

#if !defined(SAVE_SPACE_DM42_22_EDIT1)
            case dtShortInteger: {
              uint16_t i;
              grpGroupingLeftOld  = grpGroupingLeft;
              grpGroupingRightOld = grpGroupingRight;

              memset(aimBuffer, 0, AIM_BUFFER_LENGTH);
              memset(nimBufferDisplay, 0, NIM_BUFFER_LENGTH);

              lastIntegerBase  = getRegisterTag(REGISTER_X);
              nimNumberPart = (lastIntegerBase <= 10 ? NP_INT_10 : NP_INT_16);

              grpGroupingLeft  = 0;
              grpGroupingRight = 0;
              _shortIntegerToString(REGISTER_X, aimBuffer);
              grpGroupingRight = grpGroupingRightOld;
              grpGroupingLeft  = grpGroupingLeftOld;

              hexDigits   = 0;
              for(i = 0; i < strlen(aimBuffer); i++) {
                if((aimBuffer[i] >= 'A') && (aimBuffer[i] <= 'F')) {
                  hexDigits++;
                }
              }

              strcpy(nimBufferDisplay, STD_SPACE_HAIR);
              nimBufferToDisplayBuffer(aimBuffer, nimBufferDisplay + 2);
              for(i=stringByteLength(nimBufferDisplay) - 1; i>0; i--) {
                if(nimBufferDisplay[i] == (char)0xab) {    //token
                  nimBufferDisplay[i] = SEPARATOR_LEFT[0];
                  if(nimBufferDisplay[i+1] == 1) {
                    nimBufferDisplay[i+1] = SEPARATOR_LEFT[1];
                  }
                }
              }
              //printf("**[DL]** dtShortInteger aimBuffer %s nimBufferDisplay %s\n",aimBuffer,nimBufferDisplay);fflush(stdout);

              calcMode = CM_NIM;
              clearSystemFlag(FLAG_ALPHA);
              freeRegisterData(REGISTER_X);
              setRegisterDataPointer(REGISTER_X, allocC47Blocks(REAL34_SIZE_IN_BLOCKS));
              setRegisterDataType(REGISTER_X, dtReal34, amNone);
              if(!checkHP) clearRegisterLine(NIM_REGISTER_LINE, true, true);
              xCursor = 1;
              cursorEnabled = true;
              cursorFont = &numericFont;
              break;
            }
#endif // !SAVE_SPACE_DM42_22_EDIT1

            // case dtConfig: Not relevant for EDIT
            default: {
              goto err;
            }
          }
        }
        break;

      case CM_AIM :
        runFunction(ITM_XEDIT);
        break;

      case CM_PEM : {
        if((pemCursorIsZerothStep) || isAtEndOfProgram(currentStep) || isAtEndOfPrograms(currentStep)) return; // Don't try to edit step 000 or END or .END.
        //printf("**[DL]** currentLocalStepNumber %d\n",currentLocalStepNumber);fflush(stdout);
        int16_t i = 0;
        int16_t func = currentStep[i++];
        if(func & 0x80) {
          func &= 0x7f;
          func <<= 8;
          func |= currentStep[i++];
        }
        uint8_t opParam  = currentStep[i++];
        #if !defined(SAVE_SPACE_DM42_23_EDIT2)
        uint8_t opParam2 = currentStep[i++];
          uint8_t opParam3 = currentStep[i];
        #endif

        //printf("**[DL]** fnEdit cmPem func %d opParam %d opParam2 %d decodedLiteralType %d\n",func,opParam,opParam2,decodedLiteralType);fflush(stdout);

        if((func == ITM_LITERAL || func == ITM_REM)) {
          memset(aimBuffer, 0, AIM_BUFFER_LENGTH);

          if(opParam == STRING_LABEL_VARIABLE) {
            pemAlphaEdit(NOPARAM);
          }
#if !defined(SAVE_SPACE_DM42_23_EDIT2)
          else if((opParam == BINARY_SHORT_INTEGER) || (opParam == STRING_SHORT_INTEGER) || (opParam == STRING_LONG_INTEGER) ||
                  (opParam == BINARY_REAL34)        || (opParam == STRING_REAL34)        ||
                  (opParam == BINARY_COMPLEX34)     || (opParam == STRING_COMPLEX34)     ||
                  (opParam == STRING_DATE)          || (opParam == STRING_TIME)          || (opParam == STRING_ANGLE_DMS)    ||
                  (opParam == STRING_ANGLE_RADIAN)  || (opParam == STRING_ANGLE_GRAD)    ||
                  (opParam == STRING_ANGLE_DEGREE)  || (opParam == STRING_ANGLE_MULTPI)) {
            char *tempBuffer = errorMessage + 3000;
            bool chsNeeded = false;
            bool isDate = (opParam == STRING_DATE ? true : false);

            if((opParam == STRING_REAL34)|| (opParam == STRING_COMPLEX34))  {
              _getStringLabelOrVariableName(&currentStep[2]);
              strcpy(tempBuffer, tmpStringLabelOrVariableName);
            }
            else {
              grpGroupingLeftOld  = grpGroupingLeft;
              grpGroupingRightOld = grpGroupingRight;
              grpGroupingRight = 0;
              grpGroupingLeft  = 0;
              decodeOneStep(currentStep);
              grpGroupingRight = grpGroupingRightOld;
              grpGroupingLeft  = grpGroupingLeftOld;
              strcpy(tempBuffer, tmpString);
            }
            lastIntegerBase = (opParam == BINARY_SHORT_INTEGER ? opParam2: opParam == STRING_SHORT_INTEGER ? opParam2: 0);
            //printf("**[DL]** fnEdit lastIntegerBase %d tempBuffer %s\n",lastIntegerBase,tempBuffer);fflush(stdout);
            deleteStepsFromTo(currentStep, findNextStep(currentStep));

            uint16_t i;
            uint16_t iMax = strlen(tempBuffer);
            bool decimalflag = false;
            for(i = 0; i < iMax; i++) {
              //printf("**[DL]** fnEdit tempBuffer[%2d] %02x aimBuffer %s\n",i,tempBuffer[i]&0xff,aimBuffer);fflush(stdout);
              switch ((uint8_t) tempBuffer[i]) {
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                  pemAddNumber(ITM_0 + tempBuffer[i] - '0', false);
                  break;
                case 'A':
                case 'B':
                case 'C':
                case 'D':
                case 'E':
                case 'F':
                  pemAddNumber(ITM_A + tempBuffer[i] - 'A', false);
                  break;
                case '.':
                  if(!decimalflag) {
                    decimalflag = true;
                    pemAddNumber(ITM_PERIOD, false);
                  }
                  break;
                case ':' :
                  if(!decimalflag) {
                    decimalflag = true;
                    pemAddNumber(ITM_PERIOD, false);
                  }
                  break;
                case '+':
                  if(chsNeeded)  pemAddNumber(ITM_CHS, false);  // '-' was already encountered, let's first negate the real part
                  chsNeeded = false;
                  if(opParam == BINARY_COMPLEX34) {
                    //printf("**[DL]** fnEdit pemAddNumber ITM_CC aimBuffer %s\n",aimBuffer);fflush(stdout);
                    pemAddNumber(ITM_CC, false);
                    decimalflag = false;
                  }
                  break;
                case '-':
                  if(isDate) {
                    if(!decimalflag) {
                      decimalflag = true;
                      pemAddNumber(ITM_PERIOD, false);
                    }
                  }
                  else {
                    if(chsNeeded) pemAddNumber(ITM_CHS, false);  // second time '-' is encountered, let's first negate the real part
                    chsNeeded = true;
                    if(opParam == BINARY_COMPLEX34) {
                      //printf("**[DL]** fnEdit pemAddNumber ITM_CC aimBuffer %s\n",aimBuffer);fflush(stdout);
                      pemAddNumber(ITM_CC, false);
                      decimalflag = false;
                    }
                  }
                  break;
                case '/':
                  if(isDate) {
                    if(!decimalflag) {
                      decimalflag = true;
                      pemAddNumber(ITM_PERIOD, false);
                    }
                  }
                  break;
                case 'e':
                  if(chsNeeded) pemAddNumber(ITM_CHS, false);           // change mantissa sign before entering exponent
                  chsNeeded = false;
                  pemAddNumber(ITM_EXPONENT, false);
                  break;
                case 'i':
                  pemAddNumber(ITM_CC, false);
                  decimalflag = false;
                  break;
                case 0x80:
                  i++;
                  //printf("**[DL]**        tempBuffer[%2d] %02x\n",i,tempBuffer[i]&0xff);fflush(stdout);
                  if((tempBuffer[i] == STD_CROSS[1]) && (nimNumberPart != NP_COMPLEX_INT_PART)) {
                    i += 2; // Skip next character (STD_BASE_10)
                    if(chsNeeded) pemAddNumber(ITM_CHS, false);         // change mantissa sign before entering exponent
                    chsNeeded = false;
                    pemAddNumber(ITM_EXPONENT, false);
                  }
                  else if((tempBuffer[i] == STD_DEGREE[1]) && (opParam == STRING_ANGLE_DMS)) {
                    pemAddNumber(ITM_PERIOD, false);
                  }
                  break;
                case 0xa1:
                  i++;
                  //printf("**[DL]**        tempBuffer[%2d] %02x\n",i,tempBuffer[i]&0xff);fflush(stdout);
                  if((tempBuffer[i] >= STD_SUP_0[1]) && (tempBuffer[i] <= STD_SUP_9[1])) {
                    pemAddNumber(ITM_0 + tempBuffer[i] - STD_SUP_0[1], false);
                  }
                  else if(tempBuffer[i] == STD_SUP_MINUS[1]) {
                    chsNeeded = true;
                  }
                  else if(((tempBuffer[i] == STD_op_i[1]) || (tempBuffer[i] == STD_op_j[1])) &&
                          (nimNumberPart != NP_COMPLEX_INT_PART)) {
                    //printf("**[DL]** fnEdit pemAddNumber ITM_op_j aimBuffer %s\n",aimBuffer);fflush(stdout);
                    pemAddNumber(ITM_CC, false);
                    decimalflag = false;
                  }
                  //printf("**[DL]** fnEdit pemAddNumber %02x aimBuffer %s\n",tempBuffer[i],aimBuffer);fflush(stdout);
                  break;
                case 0x81:
                case 0x82:
                case 0x83:
                case 0x9d:
                case 0x9e:
                case 0xa0:
                case 0xa2:
                case 0xa3:
                case 0xa4:
                case 0xa5:
                case 0xa6:
                case 0xa7:
                case 0xa9:
                case 0xab:
                case 0xac:
                  i++;   // Ignore non supported unicode characters, including base subscripts
                  //printf("**[DL]**        tempBuffer[%2d] %02x\n",i,tempBuffer[i]&0xff);fflush(stdout);
                  break;
                default:
                  //printf("**[DL]** dflt   tempBuffer[%2d] %02x\n",i,tempBuffer[i]&0xff);fflush(stdout);
                  break;
              }
              lastIntegerBase = (opParam == BINARY_SHORT_INTEGER ? opParam2: opParam == STRING_SHORT_INTEGER ? opParam2: 0);
            }
            if(chsNeeded) pemAddNumber(ITM_CHS, false);
            switch (opParam) {
              case STRING_DATE:
              case STRING_TIME:
              case STRING_ANGLE_RADIAN:
              case STRING_ANGLE_GRAD:
              case STRING_ANGLE_DEGREE:
              case STRING_ANGLE_DMS:
              case STRING_ANGLE_MULTPI: {
                editingLiteralType = opParam;
                break;
              }
              default:
                editingLiteralType = 0;
            }
            pemAddNumber(ITM_NOP, true);    // to insert the resulting number in program
            //printf("**[DL]** fnEdit editingLiteralType %d aimBuffer %s\n",editingLiteralType,aimBuffer);fflush(stdout);
          }
#endif // !SAVE_SPACE_DM42_23_EDIT2
          else {
            ;
          }
        }
#if !defined(SAVE_SPACE_DM42_23_EDIT2)
        else {
          uint16_t regNumber;
          uint16_t paramMode = (indexOfItems[func].status & PTP_STATUS) >> 9;
          if((opParam == STRING_LABEL_VARIABLE) || (opParam == INDIRECT_VARIABLE)) {
            for(index = 0;  index < opParam2; index++) {
              varOrLblName[index] = currentStep[i++];
            }
            varOrLblName[index] = 0;
          }
          switch (paramMode) {
            case PARAM_DECLARE_LABEL:
            case PARAM_LABEL:
            case PARAM_REGISTER:
            case PARAM_FLAG:
            case PARAM_NUMBER_8:
            case PARAM_NUMBER_16:            // Used only for "BestF", "RNG", "DMX", "YY"
            case PARAM_COMPARE:
            case PARAM_SKIP_BACK:
            case PARAM_NUMBER_8_16:          // Used only for "CNST
            case PARAM_SHUFFLE:              // Used only for "<>"
            case PARAM_MENU: {               // Used only for "OPENM"
              deleteStepsFromTo(currentStep, findNextStep(currentStep));
              if(!pemCursorIsZerothStep) fnBst(NOPARAM);
              tamEnterMode(func);

              uint8_t maxDigits = tam.max < 10 ? 1 : (tam.max < 100 ? 2 : (tam.max < 1000 ? 3 : (tam.max < 10000 ? 4 : 5)));

              if((opParam == INDIRECT_REGISTER) && (!isFunctionOldParam16(func)))  {
                tam.indirect = true;
                tam.max = 99;
                maxDigits = 2;
                opParam = opParam2;
                opParam2 = opParam3;
                popSoftmenu();
                showSoftmenu(-MNU_TAM);
                --numberOfTamMenusToPop;
              }
              else if((opParam == INDIRECT_VARIABLE) && (!isFunctionOldParam16(func)))   {
                tam.indirect = true;
                opParam = STRING_LABEL_VARIABLE;
                popSoftmenu();
                showSoftmenu(-MNU_TAM);
                --numberOfTamMenusToPop;
              }

              regNumber = opParam;
              if((paramMode == PARAM_REGISTER) || (paramMode == PARAM_COMPARE) || tam.indirect) {
                if(opParam <= LAST_SPARE_REGISTERS_IN_KS_CODE) { // Global register from 00 to 99, Lettered register from X to K, or Local register from .00 to .98
                  regNumber = regKStoC(opParam);
                }
              }

              if ((paramMode == PARAM_FLAG) && opParam == SYSTEM_FLAG_NUMBER) {                 // System flag
                tam.digitsSoFar = 0;
                tam.value = 0;
              }
              else if(opParam == STRING_LABEL_VARIABLE) {      // Variable name
                tam.digitsSoFar = 0;
                tam.value = 0;
              }
              else if ((paramMode == PARAM_COMPARE) && ((opParam == VALUE_0) ||(opParam == VALUE_1)))  {  // Comparison to 0 or 1
                tam.digitsSoFar = 0;
                tam.value = 0;
              }
              else if((paramMode == PARAM_FLAG) && opParam > LAST_GLOBAL_FLAG) {                // Local flag
                tam.dot = true;
                tam.digitsSoFar = maxDigits - 1;
                tam.value = (opParam - FIRST_LOCAL_FLAG) / 10;
              }
              else if(((paramMode == PARAM_REGISTER) || (paramMode == PARAM_COMPARE) || tam.indirect) && (regNumber > LAST_GLOBAL_REGISTER)) {    // Local register
                tam.dot = true;
                tam.digitsSoFar = maxDigits - 1;
                tam.value = (regNumber - FIRST_LOCAL_REGISTER) / 10;
              }
              else if(((paramMode == PARAM_REGISTER) || (paramMode == PARAM_FLAG) || (paramMode == PARAM_COMPARE)|| tam.indirect) && opParam >= REGISTER_X) {    // Lettered flag or register from X to K
                tam.digitsSoFar = 0;
                tam.value = 0;
              }
              else if(((paramMode == PARAM_DECLARE_LABEL) || (paramMode == PARAM_LABEL)) && opParam >= 100) {    // Local label from A to E or Label name
                tam.digitsSoFar = 0;
                tam.value = 0;
              }
              else if((paramMode == PARAM_NUMBER_16) && !tam.indirect) {     // BestF, RNG, DMX, YY parameter
                tam.digitsSoFar =  maxDigits - 1;
                if(isFunctionOldParam16(func)) {  // original Param16 functions without indirection support (little endian parameter)
                  tam.value = ((opParam2 << 8) + opParam) / 10;
                }
                else {                        // new Param16 functions with indirection support (big endian parameter)
                  tam.value = ((opParam << 8) + opParam2) / 10;
                }
                //tam.value = (opParam & 0X3F) + 0X1500;     // remove last shuffled register
              }
              else if(paramMode == PARAM_SHUFFLE) {       // Stack registers shuffle
                tam.digitsSoFar = 3;
                tam.value = (opParam & 0X3F) + 0X1500;    // remove last shuffled register
              }
              else if ((paramMode == PARAM_NUMBER_8_16) && opParam == CNST_BEYOND_250) {         // Constant from 250 to 499
                tam.digitsSoFar = maxDigits - 1;
                tam.value = (opParam2 / 10) + 25;
              }
              else {                                    // Number, numbered register 0-99, local label 0-99
                tam.digitsSoFar =  maxDigits - 1;
                tam.value = opParam / 10;
              }
              //printf("**[DL]** tamProcessInput func %d aimBuffer %s\n",func,aimBuffer);fflush(stdout);
              tamProcessInput(func);
              //scrollPemBackwards();
              if(opParam == STRING_LABEL_VARIABLE) {      // Variable name : Label or  edit name string
                tamProcessInput(ITM_alpha);
                varOrLblName[6] = 0;  // Ensure name is 6 characters maximum
                strcpy(aimBuffer, varOrLblName);
                alphaCursor = strlen(varOrLblName);
                tamProcessInput(ITM_NOP);                 // to insert the resulting string in program
              }

              break;
            }


            case PARAM_KEYG_KEYX: {                            // Key Goto or Key eXecute
              func = (opParam2 == ITM_GTO ? ITM_KEYG : ITM_KEYX);
              deleteStepsFromTo(currentStep, findNextStep(currentStep));
              runFunction(func);
              tamProcessInput(ITM_0 + opParam/10);
              tamProcessInput(ITM_0 + (opParam % 10));
              if((opParam3 == INDIRECT_REGISTER) || (opParam3 == INDIRECT_VARIABLE)) {
                tamProcessInput(ITM_INDIRECTION);
              }
              scrollPemBackwards();
              break;
            }

            default: {
              ;
            }
          }
        }
#endif // !SAVE_SPACE_DM42_23_EDIT2
        break;
      }

      default:
err:
        displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "Calculator mode or type not supported for EDIT command");
          moreInfoOnError("In function fnEdit:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        break;
    }
  #endif
}


#ifdef DMCP_BUILD
  void standardScreenDump(void) {
  resetShiftState();                  //JM To avoid f or g top left of the screen, clear to make sure
  int32_t vol = 0;
  vol = getBeepVolume();
  fnSetVolume(11);
  _Buzz(100,5);
  xcopy(tmpString, errorMessage, ERROR_MESSAGE_LENGTH + AIM_BUFFER_LENGTH + NIM_BUFFER_LENGTH + TAM_BUFFER_LENGTH);       //backup portion of the "message buffer" area in DMCP used by ERROR..AIM..NIM buffers, to the tmpstring area in DMCP. DMCP uses this area during create_screenshot.
  create_screenshot(0);      //Screen dump
  xcopy(errorMessage, tmpString, ERROR_MESSAGE_LENGTH + AIM_BUFFER_LENGTH + NIM_BUFFER_LENGTH + TAM_BUFFER_LENGTH);        //   This total area must be less than the tmpString storage area, which it is.
  _Buzz(100,5);
  fnSetVolume((uint16_t)vol);
}
#endif //DMCP_BUILD


bool_t anyKeyWaiting(void) {
  #if defined(DMCP_BUILD)
    return key_empty() == 0 || key_tail() != -1;
  #elif defined(PC_BUILD) // !DMCP_BUILD
    //printf("KeyWaiting keyCode=%u",currentKeyCode);
    return currentKeyCode == 32; //EXIT1 / EXIT key //Do not us gtk_events_pending() as it triggers for timers too
  #endif // PC_BUILD
  return false;
}



bool_t exitKeyWaiting(void) {
  #if defined(DMCP_BUILD)
    bool_t checkKey = C47PopKeyNoBuffer(DISPLAY_WAIT_FOR_RELEASE) == 32;
    if(!checkKey) {
      key_pop_all();
      clearKeyBuffer();
    }
    return checkKey;
  #elif defined(PC_BUILD) // !DMCP_BUILD
    //printf("KeyWaiting keyCode=%u",currentKeyCode);
    return currentKeyCode == 32; //EXIT1 / EXIT key //Do not us gtk_events_pending() as it triggers for timers too
  #endif // PC_BUILD
  return false;
}


int C47PopKeyNoBuffer(bool_t displayWaitForRelease) {
  int tmpf = -1;
  #if defined(DMCP_BUILD)
    if(!anyKeyWaiting()) return -1;
    if(displayWaitForRelease) {
      #if !defined(TESTSUITE_BUILD) && defined(VERBOSEKEYS_BUFFERED)
        showString("Key(s) buffered ...", &standardFont, 20, 40, vmNormal, false, false);
      #endif //!TESTSUITE_BUILD
      force_refresh(force);
////Monitor key codes on screen
//char sss[22];
//sprintf(sss,"%i   AA ",sys_last_key());
//print_linestr(sss,true);
    }
    wait_for_key_release(0);
    bool_t signalToDoScreenDump = false;
    bool_t signalToDoEXIT       = false;
    bool_t signalToDoRS         = false;
    int tmpz = -1;
    while(anyKeyWaiting()) {
      tmpz = key_pop();
      if(tmpz > 0) tmpf = tmpz;                     //use the last key in the buffer
      if(tmpz == 44) signalToDoScreenDump = true;   //if any key in the buffer is 44
      if(tmpz == 33) signalToDoEXIT = true;
      if(tmpz == 36) signalToDoRS = true;
    }

    if(signalToDoScreenDump) {
      standardScreenDump();
      tmpf = 0;
    }
    if(signalToDoRS) {
      tmpf = 36;
    }
    if(signalToDoEXIT) {
      tmpf = 33;
    }
    return tmpf - 1;        //EXIT = 33-1
  #else // !DMCP_BUILD
    tmpf = currentKeyCode;
    currentKeyCode = 255;
    return tmpf;           //EXIT = 32
  #endif // DMCP_BUILD
}


void fnShoiXRepeats(uint16_t numberOfRepeats) {           //JM SHOIDISP
  displayStackSHOIDISP = numberOfRepeats;                 //   0-3
  fnRefreshState();

  //if(getRegisterDataType(REGISTER_X) == dtShortInteger) {
  //  fnChangeBaseJM(getRegisterTag(REGISTER_X));
  //}
  //else {
  //  if(lastIntegerBase > 1 && lastIntegerBase <= 16) {
  //    fnChangeBaseJM(lastIntegerBase);
  //  }
  //}
}


void fnCFGsettings(uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
    runFunction(ITM_FF);
    showSoftmenu(-MNU_SYSFL);
  #endif // !TESTSUITE_BUILD
}


//=-=-=-=-=-=-==-=-
//input is date
//output is ymd coded decimal yyyy.mmdd in the form of a normal decimal
void fnFrom_ymd(uint16_t unusedButMandatoryParameter){
  #if !defined(TESTSUITE_BUILD)
    if(getRegisterDataType(REGISTER_X) == dtDate) {
      fnToReal(NOPARAM);
    }
  #endif // !TESTSUITE_BUILD
}



//=-=-=-=-=-=-==-=-
//input is time or DMS
//output is sexagesima coded decimal ddd.mmsssssss in the form of a normal decimal
void fnFrom_ms(uint16_t unusedButMandatoryParameter){
  #if !defined(TESTSUITE_BUILD)
    char tmpString100[100];
    char tmpString100_OUT[100];
    tmpString100[0] = 0;
    tmpString100_OUT[0] = 0;

    if(getRegisterDataType(REGISTER_X) == dtTime) {
      temporaryInformation = TI_FROM_MS_TIME;
    }
    else if(getRegisterDataType(REGISTER_X) == dtReal34 && getRegisterAngularMode(REGISTER_X) != amNone) {
      if(getRegisterAngularMode(REGISTER_X) != amDMS) {
        fnAngularModeJM(amDMS);
      }
      temporaryInformation = TI_FROM_MS_DEG;
    }
    else {
      temporaryInformation = TI_NO_INFO;
    }

    if(temporaryInformation != TI_NO_INFO) {
      if(temporaryInformation == TI_FROM_MS_TIME) {
        copyRegisterToClipboardString2(REGISTER_X, tmpString100);
      }
      if(temporaryInformation == TI_FROM_MS_DEG) {
        real34ToDisplayString(REGISTER_REAL34_DATA(REGISTER_X), getRegisterAngularMode(REGISTER_X), tmpString100, &standardFont, SCREEN_WIDTH, NUMBER_OF_DISPLAY_DIGITS, !LIMITEXP, FRONTSPACE, NOIRFRAC);
        int16_t tmp_i = 0;
        while(tmpString100[tmp_i] != 0 && tmpString100[tmp_i+1] != 0) { //pre-condition the dd.mmssss to replaxce spaces with zeroes
          //printf("%c %d", tmpString100[tmp_i], tmpString100[tmp_i]);
          if((uint8_t)tmpString100[tmp_i] == 128 && (uint8_t)tmpString100[tmp_i+1] == 176) {
            tmpString100[tmp_i]   = ' ';
            tmpString100[tmp_i+1] = 'o';
          }
          if((uint8_t)tmpString100[tmp_i] == 'o' && (uint8_t)tmpString100[tmp_i+1] == ' ') {
            tmpString100[tmp_i+1] = '0';
          }
          if((uint8_t)tmpString100[tmp_i] == ':' && (uint8_t)tmpString100[tmp_i+1] == ' ') {
            tmpString100[tmp_i+1] = '0';
          }
          if((uint8_t)tmpString100[tmp_i] == '\'' && (uint8_t)tmpString100[tmp_i+1] == ' ') {
            tmpString100[tmp_i+1] = '0';
          }
          tmp_i++;
        }
      }

      //printf(" ------- 002 >>>%s<<<\n", tmpString100);

      int16_t tmp_j, tmp_i;
      tmp_i = tmp_j = 0;
      bool_t decimalflag = false;
      while(tmpString100[tmp_i] != 0) {
        //printf("%c %d",(uint8_t)tmpString100[tmp_i], (uint8_t)tmpString100[tmp_i]);
        switch((uint8_t)tmpString100[tmp_i]) {
          case '0' :
          case '1' :
          case '2' :
          case '3' :
          case '4' :
          case '5' :
          case '6' :
          case '7' :
          case '8' :
          case '9' :
          case '+' :
          case '-' :
            //printf("-\n");
            tmpString100_OUT[tmp_j] = (uint8_t)tmpString100[tmp_i];
            tmpString100_OUT[++tmp_j] = 0;
            break;
          case 'o' :
          case ':' :
          case '.' :
          case ',' :
            if(!decimalflag) {
              //printf("decimal\n");
              decimalflag = true;
              tmpString100_OUT[tmp_j] = '.';
              tmpString100_OUT[++tmp_j] = 0;
            }
            break;
          default:
            //printf("ignore.\n");
            break;
        }
        tmp_i++;
      }

      if(tmpString100_OUT[0] != 0) {
        reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
        stringToReal34(tmpString100_OUT, REGISTER_REAL34_DATA(REGISTER_X));
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          printf("\n ------- 003 >>>%s<<<\n",tmpString100_OUT);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
    }

    //stringToReal(tmpString100, &value, &ctxtReal39);
  #endif // !TESTSUITE_BUILD
}


/*
* If in direct entry, accept h.ms, example 1.23 [.ms] would be 1:23:00. Do not change the ADM.
* If closed in X: and X is REAL/integer, then convert this to h.ms. Do not change the ADM.
* If closed in X: and X is already a Time in visible hms like 1:23:45, then change the time to REAL, then tag the REAL with d.ms (‘’) in the form 1°23’45.00’’. Do not change the ADM.
* if closed in X: and X is already d.ms, then convert X to time in h:ms.Do not change the ADM.
*/
//


// 2023-09-07
// Current operation:
//   A.    From NIM press .ms: always real/integer (no angle), converting the digits to “h” ”m” ”s”:
//   a.    Example 1.2345 .ms -> 1:23:45, No change.
//
//   B.    With H:M:S (1:23:45) in X, press .ms (again): rewrite the hexadecimal digits and tag as angle
//   a.    Example 1:23:45 in X, .ms -> 1°23’45’’ tagged angle, No change
//
//   C.    With Real/Integer (hours) in X press .ms: -> convert X hours to HMS:
//   a.    Example 1.2345 ENTER, .ms -> 1:14:04.2, No change.
//
//   D.    Tagged angle in X: RAD GRAD MULpi in X, .ms: no action. Proposed change.
//
//   E.    Tagged angle in X: DMS, press .ms: rewrite D:M:S to H:M:S. No change.
//   a.    Press .ms again, see (B)
//
//   F.    Tagged angle in X: DEG, .ms: do >>DMS
//   a.    Press again, see (E), the cyclic continue as now, no change
//
// I propose these CHANGES for Tagged angles only:
//   D.    Tagged angle in X: RAD GRAD MULpi in X, .ms: Change to do: >>DMS
//   a.    Press again, see (E), the cyclic continue as now, no change




void fnTo_ms(uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
    switch(calcMode) { //JM
      case CM_NIM:
        addItemToNimBuffer(ITM_ms);
        break;

      case CM_NORMAL:
        copySourceRegisterToDestRegister(REGISTER_L, TEMP_REGISTER_1); // STO TMP

        switch(getRegisterDataType(REGISTER_X)) { //if integer, make a real
          case dtShortInteger:
            convertShortIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
            break;
          case dtLongInteger:
            convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
            break;
          default:
            break;
        }

        if(getRegisterDataType(REGISTER_X) == dtReal34) {
          if(getRegisterAngularMode(REGISTER_X) == amDMS) {
            if(calcMode == CM_NORMAL) {
              fnToReal(0);
            }
            else if(calcMode == CM_NIM) {
              addItemToNimBuffer(ITM_dotD);
            }
            fnHRtoTM(0);
          }
          else if(getRegisterAngularMode(REGISTER_X) == amDegree // ||
//             getRegisterAngularMode(REGISTER_X) == amRadian ||
//             getRegisterAngularMode(REGISTER_X) == amGrad   ||
//             getRegisterAngularMode(REGISTER_X) == amMultPi
            ) {
            fnAngularModeJM(amDMS);
          }
          else if(getRegisterAngularMode(REGISTER_X) == amNone) {
            fnHRtoTM(0);
          }
          else {
            displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "cannot calculate specific type/tag");
              moreInfoOnError("In function fnTo_ms:", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          }
        }
        else if(getRegisterDataType(REGISTER_X) == dtTime) {
          fnToHr(0);
          setRegisterAngularMode(REGISTER_X, amDegree);
          fnCvtFromCurrentAngularMode(amDMS);
        }

        copySourceRegisterToDestRegister(TEMP_REGISTER_1, REGISTER_L); // STO TMP
        break;

      case CM_REGISTER_BROWSER:
      case CM_ASN_BROWSER:
      case CM_FLAG_BROWSER:
      case CM_FONT_BROWSER:
      case CM_PLOT_STAT:
      case CM_LISTXY: //JM
      case CM_GRAPH:  //JM
        break;

      default:
        sprintf(errorMessage, commonBugScreenMessages[bugMsgCalcModeWhileProcKey], "fnTo_ms", calcMode, ".ms");
        displayBugScreen(errorMessage);
    }
  #endif // !TESTSUITE_BUILD
}


void addzeroes(char *st, uint8_t ix) {
  uint_fast8_t iy;
  strcpy(st, "1");
  for(iy = 0; iy < ix; iy++) {
    strcat(st, "0");
  }
}


void fnMultiplySI(uint16_t multiplier) {
  copySourceRegisterToDestRegister(REGISTER_L, TEMP_REGISTER_1); // STO TMP
  char mult[64];
  char divi[64];
  mult[0] = 0;
  divi[0] = 0;

  uint16_t base = 10;

  if(multiplier > 100 && multiplier <= 100 + 18) {
    addzeroes(mult, multiplier - 100);
    base = 10;
  }
  else if(multiplier < 100 && multiplier >= 100 - 18) {
    addzeroes(divi, 100 - multiplier);
    base = 10;
  }
  else if(multiplier == 100) {
    strcpy(mult, "1");
    base = 10;
  }
  else if(multiplier > 200 && multiplier <= 200 + 50) {
    addzeroes(mult, multiplier - 200);
    base = 2;
  }
  else if(multiplier == 200) {
    strcpy(mult, "1");
    base = 2;
  }


  setSystemFlag(FLAG_ASLIFT);
  liftStack();
  longInteger_t lgInt;
  longIntegerInit(lgInt);

  if(mult[0] != 0) {
    stringToLongInteger(mult + (mult[0] == '+' ? 1 : 0), base, lgInt);
    convertLongIntegerToLongIntegerRegister(lgInt, REGISTER_X);
    longIntegerFree(lgInt);
    fnMultiply(0);
  }
  else if(divi[0] != 0) {
    stringToLongInteger(divi + (divi[0] == '+' ? 1 : 0), base, lgInt);
    convertLongIntegerToLongIntegerRegister(lgInt, REGISTER_X);
    longIntegerFree(lgInt);
    fnDivide(0);
  }

  adjustResult(REGISTER_X, false, false, REGISTER_X, REGISTER_Y, -1);
  copySourceRegisterToDestRegister(TEMP_REGISTER_1, REGISTER_L); // STO TMP
}


#define forcedLiftTheStack true

static void cpxToStk(const real_t *real1, const real_t *real2, const bool_t sl) {
  if(sl == forcedLiftTheStack) setSystemFlag(FLAG_ASLIFT);
  liftStack();
//  reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
//  realToReal34(real1, REGISTER_REAL34_DATA(REGISTER_X));
//  realToReal34(real2, REGISTER_IMAG34_DATA(REGISTER_X));
//  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  convertComplexToResultRegister(real1, real2, REGISTER_X);
}


void fn_cnst_op_j(uint16_t unusedButMandatoryParameter) {
  if(calcMode == CM_NIM || calcMode == CM_MIM) {
    fnKeyCC(ITM_op_j);
  }
  else {
    cpxToStk(const_0, const_1, !forcedLiftTheStack);
  }
}


void fn_cnst_op_j_pol(uint16_t unusedButMandatoryParameter) {
  if(calcMode == CM_NIM || calcMode == CM_MIM) {
    fnKeyCC(ITM_op_j_pol);
  }
  else {
    cpxToStk(const_0, const_1, !forcedLiftTheStack);
    fnToPolar2(0);
  }
}


void fn_cnst_op_aa(uint16_t unusedButMandatoryParameter) {
  cpxToStk(const_1on2, const_root3on2, !forcedLiftTheStack);
  chsCplx();
}


void fn_cnst_op_a(uint16_t unusedButMandatoryParameter) {
  fn_cnst_op_aa(0);
  conjCplx();
}


void fn_cnst_0_cpx(uint16_t unusedButMandatoryParameter) {
  cpxToStk(const_0, const_0, !forcedLiftTheStack);
}


void fn_cnst_1_cpx(uint16_t unusedButMandatoryParameter) {
  cpxToStk(const_1, const_0, !forcedLiftTheStack);
}

struct cmplxPair {
  real_t r, i;
};

void fn_cnst_op_A(uint16_t option) {
  complex34Matrix_t matrixC;
  bool_t inverted = option == ITM_MATX_A_1;

  if(!saveLastX()) {
    return;
  }

  setSystemFlag(FLAG_ASLIFT);
  liftStack();
  convertRealToResultRegister(const_0, REGISTER_X,amNone);

  //Initialize Memory for Matrix
  if(initMatrixRegister(REGISTER_X, 3, 3, true)) {
  }
  else {
    displayCalcErrorMessage(ERROR_NOT_ENOUGH_MEMORY_FOR_NEW_MATRIX, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Not enough memory for a %" PRIu32 STD_CROSS "%" PRIu32 " matrix", 1, 1);
      moreInfoOnError("In function fn_cnst_op_A:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }
  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);

  linkToComplexMatrixRegister(REGISTER_X,  &matrixC);

  real_t const__rt3on2, const_rt3on2, const__1on2;
  realDivide(const_rt3,const_2,&const_rt3on2,&ctxtReal39);
  realDivide(const_rt3,const_2,&const__rt3on2,&ctxtReal39);
  realSetNegativeSign(&const__rt3on2);
  realCopy(const_1on2,&const__1on2);
  realSetNegativeSign(&const__1on2);

  realToReal34(&const__1on2, VARIABLE_REAL34_DATA(&matrixC.matrixElements[4]));
  realToReal34(!inverted ? &const_rt3on2  : &const__rt3on2, VARIABLE_IMAG34_DATA(&matrixC.matrixElements[4]));
  realToReal34(&const__1on2, VARIABLE_REAL34_DATA(&matrixC.matrixElements[5]));
  realToReal34(!inverted ? &const__rt3on2 : &const_rt3on2, VARIABLE_IMAG34_DATA(&matrixC.matrixElements[5]));
  realToReal34(&const__1on2, VARIABLE_REAL34_DATA(&matrixC.matrixElements[7]));
  realToReal34(!inverted ? &const__rt3on2 : &const_rt3on2, VARIABLE_IMAG34_DATA(&matrixC.matrixElements[7]));
  realToReal34(&const__1on2, VARIABLE_REAL34_DATA(&matrixC.matrixElements[8]));
  realToReal34(!inverted ? &const_rt3on2  : &const__rt3on2, VARIABLE_IMAG34_DATA(&matrixC.matrixElements[8]));

  for (int i = 0; i < 3; i++) {
    realToReal34(const_1, VARIABLE_REAL34_DATA(&matrixC.matrixElements[i]));
    real34Zero(VARIABLE_IMAG34_DATA(&matrixC.matrixElements[i]));
    if(i != 0) {
      realToReal34(const_1, VARIABLE_REAL34_DATA(&matrixC.matrixElements[i*3]));
      real34Zero(VARIABLE_IMAG34_DATA(&matrixC.matrixElements[i*3]));
    }
  }
  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}



TO_QSPI static const struct {
    unsigned rows  : 2;
    unsigned cols  : 2;
    unsigned x     : 2;
    unsigned y     : 2;
    unsigned z     : 2;
    unsigned xdef  : 2;
    unsigned ydef  : 2;
    unsigned zdef  : 2;
} vecCreate[] = {
//  type     r  c   x    y    z   xdef ydef zdef
    [ 1] = { 3, 1,  0 ,  1,   2,   2,   2,   2 },     // 3x1 vector created from xyz FOR ELEC menu //changed sequence to zyx
    [ 2] = { 1, 3,  0 ,  1,   2,   2,   2,   2 },     // 1x3 vector created from zyx FOR MATX menu
    [ 3] = { 1, 3,  0 ,  1,   2,   0,   0,   1 },     // 1x3 unity vectors 100
    [ 4] = { 1, 3,  0 ,  1,   2,   0,   1,   0 },     // 1x3 unity vectors 010
    [ 5] = { 1, 3,  0 ,  1,   2,   1,   0,   0 },     // 1x3 unity vectors 001

    [ 6] = { 1, 2,  0 ,  1,   3,   2,   2,   3 },     // 1x2 vector created from yx
    [ 7] = { 1, 2,  0 ,  1,   3,   0,   1,   3 },     // 1x2 unity vectors 10
    [ 8] = { 1, 2,  0 ,  1,   3,   1,   0,   3 }      // 1x2 unity vectors 01

    // r c is the size of the matrix to be created, eg. 1x3 or 2x1 etc.
    // x y z are the matrix element number mapping to the register name, eg. x=2 means x register goes to internal matrix element 2; 3 means not copied
    // xdef/ydef/zdef
    //   = 1/0 : the value to be pre-loaded into the matrix element
    //   = 2   : the value is copied from register X, Y or Z to the specified matrix element
    //   = 3   : not copied
  };


static bool_t processDefaultVector(calcRegister_t regist, uint8_t p, uint8_t d, struct cmplxPair *x, bool_t *complexCoefs) {
  if(d == 2) {
    if(!getRegisterAsComplexOrReal(regist, &x[p].r, &x[p].i, complexCoefs)) {
      return false;
    }
  }
  else if(d < 2) {
    realCopy(d == 1 ? const_1 : const_0, &x[p].r);
  }
  return true;
}


void fnConvertStkToMx(uint16_t constVector) {
  bool_t complexCoefs = false;
  struct cmplxPair x[3];
  real34Matrix_t matrix;
  complex34Matrix_t matrixC;
  uint16_t elements;

  elements = vecCreate[constVector].rows * vecCreate[constVector].cols;

  if(!processDefaultVector(REGISTER_X, vecCreate[constVector].x, vecCreate[constVector].xdef, x, &complexCoefs)) return;
  if(!processDefaultVector(REGISTER_Y, vecCreate[constVector].y, vecCreate[constVector].ydef, x, &complexCoefs)) return;
  if(max(vecCreate[constVector].z, vecCreate[constVector].zdef) != 3 &&
     !processDefaultVector(REGISTER_Z, vecCreate[constVector].z, vecCreate[constVector].zdef, x, &complexCoefs)) return;

  if(!saveLastX()) {
    return;
  }

  if(vecCreate[constVector].xdef < 2 || vecCreate[constVector].ydef < 2 || vecCreate[constVector].zdef < 2) {
    setSystemFlag(FLAG_ASLIFT);
    liftStack();
  } else {
    fnDrop(NOPARAM);
    if(elements > 2) {
      fnDrop(NOPARAM);
    }
  }


  if(getRegisterDataType(REGISTER_X) != dtReal34Matrix && getRegisterDataType(REGISTER_X) != dtComplex34Matrix) {
    //Initialize Memory for Matrix
    if(initMatrixRegister(REGISTER_X, vecCreate[constVector].rows, vecCreate[constVector].cols, complexCoefs)) {
    }
    else {
      displayCalcErrorMessage(ERROR_NOT_ENOUGH_MEMORY_FOR_NEW_MATRIX, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Not enough memory for a %" PRIu32 STD_CROSS "%" PRIu32 " matrix", 1, 1);
        moreInfoOnError("In function fnConvertStkToMx:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }


  if(complexCoefs) {
    linkToComplexMatrixRegister(REGISTER_X,  &matrixC);
  } else {
    linkToRealMatrixRegister(REGISTER_X,  &matrix);
  }


  for (int i = 0; i < elements; i++) {
    if(complexCoefs) {
      realToReal34(&x[elements-1-i].r, VARIABLE_REAL34_DATA(&matrixC.matrixElements[i]));
      realToReal34(&x[elements-1-i].i, VARIABLE_IMAG34_DATA(&matrixC.matrixElements[i]));
    }
    else {
      realToReal34(&x[elements-1-i].r, &matrix.matrixElements[i]);
    }
  }

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);

}

void fnConvertMxToStk(uint16_t param) {
  real34Matrix_t matrix;
  complex34Matrix_t matrixC;

  if(!(getRegisterDataType(REGISTER_X) == dtReal34Matrix || getRegisterDataType(REGISTER_X) == dtComplex34Matrix)) {
    return;
  }

  if(!saveLastX()) {
    return;
  }

  copySourceRegisterToDestRegister(REGISTER_X,TEMP_REGISTER_1);
  setSystemFlag(FLAG_ASLIFT);
  liftStack();
  setSystemFlag(FLAG_ASLIFT);
  liftStack();
  convertRealToResultRegister(const_0, REGISTER_X,amNone);
  convertRealToResultRegister(const_0, REGISTER_Y,amNone);
  convertRealToResultRegister(const_0, REGISTER_Z,amNone);

  if(getRegisterDataType(TEMP_REGISTER_1) == dtComplex34Matrix) {
    linkToComplexMatrixRegister(TEMP_REGISTER_1,  &matrixC);
  } else {
    linkToRealMatrixRegister(TEMP_REGISTER_1,  &matrix);
  }


  for (int i = 0; i < 3; i++) {
    uint16_t rg = vecCreate[param].x == 2-i ? REGISTER_X : vecCreate[param].y == 2-i ? REGISTER_Y : vecCreate[param].z == 2-i ? REGISTER_Z : 0;
    if(getRegisterDataType(TEMP_REGISTER_1) == dtComplex34Matrix) {
      real34Copy(VARIABLE_REAL34_DATA(&matrixC.matrixElements[i]),REGISTER_REAL34_DATA(rg));
      real34Copy(VARIABLE_REAL34_DATA(&matrixC.matrixElements[i]),REGISTER_IMAG34_DATA(rg));
    }
    else {
      real34Copy(&matrix.matrixElements[i],REGISTER_REAL34_DATA(rg));
    }
    adjustResult(rg, false, false, rg, -1, -1);
  }
}


//Rounding
void fnJM_2SI(uint16_t unusedButMandatoryParameter) { //Convert Real to Longint; Longint to shortint; shortint to longint
  if(calcMode == CM_NIM) {
    if((   (nimNumberPart == NP_INT_BASE && aimBuffer[strlen(aimBuffer) - 1] == '#')
        || (nimNumberPart == NP_INT_10 && lastIntegerBase > 0)   )) {
      #if defined (PC_BUILD)
        printf("Do not react when in NIM SI\n");
      #endif //PC_BUILD
      return;
    }
  }
  longInteger_t tmp1, tmp3;
  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger:
      convertLongIntegerRegisterToLongInteger(REGISTER_X, tmp3);
      if(shortIntegerMode == SIM_UNSIGN && longIntegerIsNegative(tmp3)) {
        temporaryInformation = TI_DATA_NEG_OVRFL;
      }
      convertLongIntegerRegisterToShortIntegerRegister(REGISTER_X, REGISTER_X); //default to 10
      if(lastIntegerBase >= 2 && lastIntegerBase <= 16 && lastIntegerBase != 10) {
        fnChangeBase(lastIntegerBase);
      }
      convertShortIntegerRegisterToLongInteger(REGISTER_X, tmp1);

      if(longIntegerCompare(tmp1,tmp3) != 0) {
        if(temporaryInformation != TI_DATA_NEG_OVRFL) {
          temporaryInformation = TI_DATA_LOSS;             // I cannot think of which condition will reach here, without other overflows overriding, but leaving it in
        }
        setSystemFlag(FLAG_OVERFLOW);
      }
      longIntegerFree(tmp1);
      longIntegerFree(tmp3);

      break;
    case dtReal34:
      fnRoundi(0);
      break;
    case dtShortInteger:
      convertShortIntegerRegisterToLongIntegerRegister(REGISTER_X, REGISTER_X); //This shortint to longint!
      lastIntegerBase = 0;                                                      //JMNIM clear lastintegerbase, to switch off hex mode
      fnRefreshState();                                                         //JMNIM
      break;
    default:
      break;
  }
}


/* JM UNIT********************************************//**                                                JM UNIT
 * \brief Adds the power of 10 using numeric font to displayString                                        JM UNIT
 *        Converts to units like m, M, k, etc.                                                            JM UNIT
 * \param[out] displayString char*     Result string                                                      JM UNIT
 * \param[in]  exponent int32_t Power of 10 to format                                                     JM UNIT
 * \return void                                                                                           JM UNIT
 ***********************************************                                                          JM UNIT */
void exponentToUnitDisplayString(int32_t exponent, bool_t flag2To10, char *displayString, char *displayValueString, bool_t nimMode) {               //JM UNIT

TO_QSPI static const char SIprefixes[64]  = "q  r  y  z  a  f  p  n  u  m     k  M  G  T  P  E  Z  Y  R  Q  ";
TO_QSPI static const char ITSIprefixes[22] = "K  M  G  T  P  E  Z  ";

  displayString[0] = ' ';
  displayString[1] = 0;
  displayString[2] = 0;
  displayString[3] = 0;

  if(!flag2To10 && !getSystemFlag(FLAG_2TO10)) {
      if((-15 <= exponent && exponent <= 15) ||
        (-30 <= exponent && exponent <= 30 && getSystemFlag(FLAG_PFX_ALL))) {
        displayString[1] = SIprefixes[exponent + 30];
        if(displayString[1] == 'u') {
          displayString[1] = STD_mu[0]; displayString[2] = STD_mu[1];
        }
      }
  }
  else if(flag2To10) {                            //exponent of 2^(10/3)
      if((3 <= exponent && exponent <= 15) ||
         (3 <= exponent && exponent <= 21 && getSystemFlag(FLAG_PFX_ALL))) {
        displayString[1] = ITSIprefixes[exponent - 3];
        displayString[2] = 'i';
      }
    }

  if(displayString[1] == 0) {
    strcpy(displayString, PRODUCT_SIGN);                                                                //JM UNIT Below, copy of
    displayString += 2;                                                                                 //JM UNIT exponentToDisplayString in display.c
    strcpy(displayString, STD_SUB_10);                                                                  //JM UNIT
    displayString += 2;                                                                                 //JM UNIT
    displayString[0] = 0;                                                                               //JM UNIT
    if(nimMode) {                                                                                       //JM UNIT
      if(exponent != 0) {                                                                               //JM UNIT
        supNumberToDisplayString(exponent, displayString, displayValueString, false);                   //JM UNIT
      }                                                                                                 //JM UNIT
    }                                                                                                   //JM UNIT
    else {                                                                                              //JM UNIT
      supNumberToDisplayString(exponent, displayString, displayValueString, false);                     //JM UNIT
    }                                                                                                   //JM UNIT
  }                                                                                                     //JM UNIT
}                                                                                                       //JM UNIT


void fnDisplayFormatCycle (uint16_t unusedButMandatoryParameter) {
  if(DM_Cycling == 0 && softmenu[softmenuStack[0].softmenuId].menuItem == -MNU_PREFIX) {
    fnDisplayFormatUnit(displayFormatDigits);
  }
  else if(displayFormat == DF_UN) {
    fnDisplayFormatSigFig(displayFormatDigits);
  }
  else if(displayFormat == DF_SF ) {
    fnDisplayFormatAll(displayFormatDigits);
  }
  else if(displayFormat == DF_ALL) {
    fnDisplayFormatFix(displayFormatDigits);
  }
  else if(displayFormat == DF_FIX) {
    fnDisplayFormatSci(displayFormatDigits);
  }
  else if(displayFormat == DF_SCI) {
    fnDisplayFormatEng(displayFormatDigits);
  }
  else if(displayFormat == DF_ENG) {
    fnDisplayFormatUnit(displayFormatDigits);
  }
  DM_Cycling = 1;
}


//change the current state from the old state?
void fnAngularModeJM(uint16_t AMODE) { //Setting to HMS does not change AM
  #if !defined(TESTSUITE_BUILD)

  copySourceRegisterToDestRegister(REGISTER_X, TEMP_REGISTER_1);
  if(AMODE == TM_HMS) {
    if(getRegisterDataType(REGISTER_X) == dtTime) {
      goto to_return;
    }
    if(getRegisterDataType(REGISTER_X) == dtReal34 && getRegisterAngularMode(REGISTER_X) != amNone) {
      fnCvtFromCurrentAngularMode(amDegree);
    }

    if(calcMode == CM_NORMAL) {
      fnToReal(0);
    }
    else if(calcMode == CM_NIM) {
      addItemToNimBuffer(ITM_dotD);
    }

    fnHRtoTM(0); //covers longint & real
  }
  else {
    if(getRegisterDataType(REGISTER_X) == dtTime) {
      fnToHr(0); //covers time
      setRegisterAngularMode(REGISTER_X, amDegree);
      fnCvtFromCurrentAngularMode(AMODE);
      //fnAngularMode(AMODE);                             Remove updating of ADM to the same mode
    }

    if(getRegisterDataType(REGISTER_X) == dtComplex34 || getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
      //printf("###AA fnAngularModeJM (%i)<= %i\n",REGISTER_X, AMODE);
      //printf("###BB fnAngularModeJM (%i)=> %i\n",REGISTER_X, getRegisterTag(REGISTER_X));

      setComplexRegisterAngularMode(REGISTER_X, AMODE);
      setComplexRegisterPolarMode(REGISTER_X, amPolar);
      //printf("###CC fnAngularModeJM (%i)=> %i\n",REGISTER_X, getRegisterTag(REGISTER_X));
    }
    else {
      if((getRegisterDataType(REGISTER_X) != dtReal34) || ((getRegisterDataType(REGISTER_X) == dtReal34) && getRegisterAngularMode(REGISTER_X) == amNone)) {

        if(calcMode == CM_NORMAL) {         //convert longint, and strip all angles to real.
          fnToReal(0);
        }
        else if(calcMode == CM_NIM) {
          addItemToNimBuffer(ITM_dotD);
        }

        uint16_t currentAngularModeOld = currentAngularMode;
        fnAngularMode(AMODE);
        fnCvtFromCurrentAngularMode(currentAngularMode);
        currentAngularMode = currentAngularModeOld;       //Remove updating of ADM to the same mode (set in fnCvtFromCurrentAngularMode())
      }
      else { //convert existing tagged angle
        fnCvtFromCurrentAngularMode(AMODE);
      }
    }
  }

  to_return:
  copySourceRegisterToDestRegister(TEMP_REGISTER_1, REGISTER_L);
  #endif //TESTSUITE_BUILD
}



void DRG_cyc(uint16_t * dest) {
    DRG_Cycling = 1;
    switch(*dest) {
      case amNone:   *dest = currentAngularMode; break; //converts from to the same, i.e. get to current angle mode
      case amRadian: *dest = amGrad;             break;
      case amGrad:   *dest = amDegree;           break;
      case amDegree: *dest = amRadian;           break;
      case amDMS:    *dest = amDegree;           break;
      case amMultPi: *dest = amRadian;           break; //do not support Mulpi but at least get out of it
      default: ;
    }
  }


void fnDRG(uint16_t unusedButMandatoryParameter) {
  switch(getRegisterDataType(REGISTER_X)) {
    case dtTime:
    case dtDate:
    case dtString:
    case dtReal34Matrix:
    case dtConfig:
      goto to_return_noLastX;
      break;
    default: ;
  }

  copySourceRegisterToDestRegister(REGISTER_X, TEMP_REGISTER_1);
  uint16_t dest = 9999;

  if(getRegisterDataType(REGISTER_X) == dtComplex34 || getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
    setComplexRegisterPolarMode(REGISTER_X, amPolar);      //re-set it to Polar iven if it was there already
    dest = getComplexRegisterAngularMode(REGISTER_X);
    DRG_cyc(&dest);
    setComplexRegisterAngularMode(REGISTER_X,dest);

  }
  else if(getRegisterDataType(REGISTER_X) == dtShortInteger) {           // If shortinteger in X, convert to real
    convertShortIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
    setRegisterAngularMode(REGISTER_X, amNone);                          //is probably none already
  }
  else if(getRegisterDataType(REGISTER_X) == dtLongInteger) {            // If longinteger in X, convert to real
    convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
    setRegisterAngularMode(REGISTER_X, amNone);                          //is probably none already
  }

  if(getRegisterDataType(REGISTER_X) == dtReal34) {                      // if real
    dest = getRegisterAngularMode(REGISTER_X);

    if(dest != amNone && dest != currentAngularMode && DRG_Cycling != 1) {   //first step: convert tagged angle to ADM
      fnCvtToCurrentAngularMode(dest);
      goto to_return;
    }

    DRG_cyc(&dest);
    fnCvtFromCurrentAngularMode(dest);
    //currentAngularMode = dest;          //remove setting of ADM!
  }

  to_return:
  copySourceRegisterToDestRegister(TEMP_REGISTER_1, REGISTER_L);
  to_return_noLastX:
  return;
}


void shrinkNimBuffer(void) {                      //JMNIM vv
  int16_t lastChar; //if digits in NIMBUFFER, ensure switch to NIM,
  int16_t hexD = 0;
  bool_t reached_end = false;
  lastChar = strlen(aimBuffer) - 1;
  if(lastChar >= 1) {
    uint_fast16_t ix = 0;
    while(aimBuffer[ix] != 0) { //if # found in nimbuffer, return and do nothing
      if(aimBuffer[ix] >= 'A') {
        hexD++;
      }
      if(aimBuffer[ix] == '#' || aimBuffer[ix] == '.' || reached_end) { //chr(35) = "#"
        aimBuffer[ix] = 0;
        reached_end = true;
        //printf(">>> ***A # found. hexD=%d\n",hexD);
      }
      else {
        //printf(">>> ***B # not found in %s:%d=%d hexD=%d\n",nimBuffer,ix,nimBuffer[ix],hexD);
      }
      ix++;
    }
    if(hexD > 0) {
      nimNumberPart = NP_INT_16;
    }
    else {
      nimNumberPart = NP_INT_10;
    }
    //calcMode = CM_NIM;
  }
}                                                 //JMNIM ^^


void fnChangeBaseJM(uint16_t BASE) {
  #if !defined(TESTSUITE_BUILD)
    //printf(">>> §§§ fnChangeBaseJMa Calmode:%d, nimbuffer:%s, lastbase:%d, nimnumberpart:%d\n", calcMode, nimBuffer, lastIntegerBase, nimNumberPart);
    shrinkNimBuffer();
    fnChangeBase(BASE);

    if(getSystemFlag(FLAG_HPBASE)) {
      for(uint16_t regist = REGISTER_X + 1; regist < REGISTER_X + (getSystemFlag(FLAG_SSIZE8) ? 8 : 4); regist ++ ) {
        if(getRegisterDataType(regist) == dtShortInteger) {
          if(2 <= BASE && BASE <= 16) {
            setRegisterTag(regist, BASE);
          }
          fnRefreshState();
        }
      }
    }

    nimBufferToDisplayBuffer(aimBuffer, nimBufferDisplay + 2);
  #endif // !TESTSUITE_BUILD
}


void fnChangeBaseMNU(uint16_t BASE) {
  #if !defined(TESTSUITE_BUILD)

    if(calcMode == CM_AIM) {
      addItemToBuffer(ITM_toINT);
      return;
    }

    //printf(">>> §§§ fnChangeBaseMNUa Calmode:%d, nimbuffer:%s, lastbase:%d, nimnumberpart:%d\n", calcMode, nimBuffer, lastIntegerBase, nimNumberPart);
    shrinkNimBuffer();
    //printf(">>> §§§ fnChangeBaseMNUb Calmode:%d, nimbuffer:%s, lastbase:%d, nimnumberpart:%d\n", calcMode, nimBuffer, lastIntegerBase, nimNumberPart);

    if(lastIntegerBase == 0 && calcMode == CM_NORMAL && BASE > 1 && BASE <= 16) {
      //printf(">>> §§§fnChangeBaseMNc CM_NORMAL: convert non-shortint-mode to %d & return\n", BASE);
      fnChangeBaseJM(BASE);
      return;
    }

    if(calcMode == CM_NORMAL && BASE == NOPARAM) {
      //printf(">>> §§§fnChangeBaseMNd CM_NORMAL: convert non-shortint-mode to TAM\n");
      runFunction(ITM_toINT);
      return;
    }

    if(BASE > 1 && BASE <= 16) { //BASIC CONVERSION IN X REGISTER, OR DIGITS IN NIMBUFFER IF IN CM_NORMAL
      //printf(">>> §§§1 convert base to %d & return\n", BASE);
      fnChangeBaseJM(BASE);
      nimBufferToDisplayBuffer(aimBuffer, nimBufferDisplay + 2);
      return;
    }

    if(aimBuffer[0] == 0 && calcMode == CM_NORMAL && BASE == NOPARAM) { //IF # FROM MENU & TAM.
      //printf(">>> §§§2 # FROM MENU: nimBuffer[0]=0; use runfunction\n");
      runFunction(ITM_toINT);
      nimBufferToDisplayBuffer(aimBuffer, nimBufferDisplay + 2);
      return;
    }

    if(aimBuffer[0] != 0 && calcMode == CM_NIM) { //IF # FROM MENU, while in NIM, just add to NimBuffer
      //printf(">>> §§§3 # nimBuffer in use, addItemToNimBuffer\n");
      addItemToNimBuffer(ITM_toINT);
      nimBufferToDisplayBuffer(aimBuffer, nimBufferDisplay + 2);
      return;
    }

  #endif // !TESTSUITE_BUILD
}

/********************************************//**
 * \brief Set Input_Default
 *
 * \param[in] inputDefault uint16_t
 * \return void
 ***********************************************/
void fnInDefault(uint16_t inputDefault) {
  Input_Default = inputDefault;
  lastIntegerBase = 0;
  fnRefreshState();
}


void fnByteShortcutsS(uint16_t size) { //JM POC BASE2 vv
  fnSetWordSize(size);
  fnIntegerMode(SIM_2COMPL);
}


void fnByteShortcutsU(uint16_t size) {
  fnSetWordSize(size);
  fnIntegerMode(SIM_UNSIGN);
}


void fnP_Alpha(void) {
  #if !defined(TESTSUITE_BUILD)
    if(calcMode != CM_AIM) {
      #if defined(DMCP_BUILD)
        beep(440, 50);
        beep(4400, 50);
        beep(440, 50);
      #endif // DMCP_BUILD
      return;
    }
    xcopy(tmpString, aimBuffer, ERROR_MESSAGE_LENGTH + AIM_BUFFER_LENGTH + NIM_BUFFER_LENGTH);       //backup portion of the "message buffer" area in DMCP used by ERROR..AIM..NIM buffers, to the tmpstring area in DMCP. DMCP uses this area during create_screenshot.
    create_filename(".REGS.TSV");

    #if (VERBOSE_LEVEL >= 1)
      clearScreen(2);
      print_linestr("Output Aim Buffer to drive:", true);
      print_linestr(filename_csv, false);
    #endif // VERBOSE_LEVEL >= 1

    tmpString_csv_out(5);          //aimBuffer now already copied to tmpString
    xcopy(aimBuffer,tmpString, ERROR_MESSAGE_LENGTH + AIM_BUFFER_LENGTH + NIM_BUFFER_LENGTH);        //   This total area must be less than the tmpString storage area, which it is.
    //print_linestr(aimBuffer,false);
  #endif
}



void fnP_Regs (uint16_t registerNo) {
  #if !defined(TESTSUITE_BUILD)
    if(calcMode != CM_NORMAL) {
      #if defined(DMCP_BUILD)
        beep(440, 50);
        beep(4400, 50);
        beep(440, 50);
      #endif // DMCP_BUILD
      return;
    }

    create_filename(".REGS.TSV");

    #if (VERBOSE_LEVEL >= 1)
      clearScreen(3);
      print_linestr("Output regs to drive:", true);
      print_linestr(filename_csv, false);
    #endif // VERBOSE_LEVEL >= 1

    stackregister_csv_out((int16_t)registerNo, (int16_t)registerNo, !ONELINE);

  #endif // !TESTSUITE_BUILD
}



void fnP_All_Regs(uint16_t option) {
  #if !defined(TESTSUITE_BUILD)
    if(calcMode != CM_NORMAL && calcMode != CM_NO_UNDO) {
      #if defined(DMCP_BUILD)
        beep(440, 50);
        beep(4400, 50);
        beep(440, 50);
      #endif // DMCP_BUILD
      return;
    }

    create_filename(".REGS.TSV");

    #if (VERBOSE_LEVEL >= 1)
      clearScreen(4);
      print_linestr("Output regs to drive:", true);
      print_linestr(filename_csv, false);
    #endif // VERBOSE_LEVEL >= 1

    switch(option) {
      case PRN_ALL:
        stackregister_csv_out(REGISTER_X, REGISTER_W, !ONELINE);
        stackregister_csv_out(0, 99, !ONELINE);
        if(currentNumberOfLocalRegisters > 0) stackregister_csv_out(FIRST_LOCAL_REGISTER, FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters - 1, !ONELINE);
        if(numberOfNamedVariables > 0) stackregister_csv_out(FIRST_NAMED_VARIABLE, FIRST_NAMED_VARIABLE + numberOfNamedVariables - 1, !ONELINE);


        //stackregister_csv_out(FIRST_LOCAL_REGISTER, FIRST_LOCAL_REGISTER+100, !ONELINE);
        break;

      case PRN_STK:
        if(getSystemFlag(FLAG_SSIZE8)) {
          stackregister_csv_out(REGISTER_X, REGISTER_D, !ONELINE);
        }
        else {
          stackregister_csv_out(REGISTER_X, REGISTER_T, !ONELINE);
        }
        break;

      case PRN_GLOBALr:
        stackregister_csv_out(0, 99, !ONELINE);
        break;

      case PRN_LOCALr:
        if(currentNumberOfLocalRegisters > 0) stackregister_csv_out(FIRST_LOCAL_REGISTER, FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters - 1, !ONELINE);
        break;

      case PRN_NAMEDr:
        if(numberOfNamedVariables > 0) stackregister_csv_out(FIRST_NAMED_VARIABLE, FIRST_NAMED_VARIABLE + numberOfNamedVariables - 1, !ONELINE);
        break;

      case PRN_Xr:
        stackregister_csv_out(REGISTER_X, REGISTER_X, !ONELINE);
        break;

      case PRN_TMP:
        stackregister_csv_out(TEMP_REGISTER_1, TEMP_REGISTER_1, !ONELINE);
        break;

      case PRN_XYr:
        stackregister_csv_out(REGISTER_X, REGISTER_Y, ONELINE);
        break;

      default: ;
    }
  #endif // !TESTSUITE_BUILD
}


void doubleToXRegisterReal34(double x) { //Convert from double to X register REAL34
  setSystemFlag(FLAG_ASLIFT);
  liftStack();
  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
  snprintf(tmpString, TMP_STR_LENGTH, "%.16e", x);
  stringToReal34(tmpString, REGISTER_REAL34_DATA(REGISTER_X));
  //adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  setSystemFlag(FLAG_ASLIFT);
}


void fnStrtoReg(const char buffer[], calcRegister_t regist) {                             //DONE
  int16_t mem = stringByteLength(buffer) + 1;
  reallocateRegister(regist, dtString, TO_BLOCKS(mem), amNone);
  xcopy(REGISTER_STRING_DATA(regist), buffer, mem);
}

void fnStrtoX(const char buffer[]) {                             //DONE
  setSystemFlag(FLAG_ASLIFT); // 5
  liftStack();
  fnStrtoReg(buffer, REGISTER_X);
  setSystemFlag(FLAG_ASLIFT);
}


void fnStrInputReal34(char inp1[]) { // CONVERT STRING to REAL IN X      //DONE
  tmpString[0] = 0;
  strcat(tmpString, inp1);
  setSystemFlag(FLAG_ASLIFT); // 5
  liftStack();
  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
  stringToReal34(tmpString, REGISTER_REAL34_DATA(REGISTER_X));
  setSystemFlag(FLAG_ASLIFT);
}


void fnStrInputLongint(char inp1[]) { // CONVERT STRING to Longint X      //DONE
  tmpString[0] = 0;
  strcat(tmpString, inp1);
  setSystemFlag(FLAG_ASLIFT); // 5
  liftStack();

  longInteger_t lgInt;
  longIntegerInit(lgInt);
  stringToLongInteger(tmpString + (tmpString[0] == '+' ? 1 : 0), 10, lgInt);
  convertLongIntegerToLongIntegerRegister(lgInt, REGISTER_X);
  longIntegerFree(lgInt);
  setSystemFlag(FLAG_ASLIFT);
}


void fnRCL(int16_t inp) { //DONE
  setSystemFlag(FLAG_ASLIFT);
  if(inp == TEMP_REGISTER_1) {
    liftStack();
    copySourceRegisterToDestRegister(inp, REGISTER_X);
  }
  else {
    fnRecall(inp);
  }
}


double convert_to_double(calcRegister_t regist) { //Convert from X register to double
  double y;
  real_t tmpy;
  switch(getRegisterDataType(regist)) {
    case dtLongInteger:
      convertLongIntegerRegisterToReal(regist, &tmpy, &ctxtReal39);
      break;
    case dtReal34:
      real34ToReal(REGISTER_REAL34_DATA(regist), &tmpy);
      break;
    default:
      return 0;
  }
  realToString(&tmpy, tmpString);
  y = strtof(tmpString, NULL);
  return y;
}


void timeToReal34(uint16_t hms) { //always 24 hour time;
  calcRegister_t regist = REGISTER_X;
  real34_t real34, value34, tmp34, h34, m34, s34;
  int32_t sign;
  uint32_t digits, tDigits = 0u, bDigits;
  bool_t isValid12hTime = false; //, isAfternoon = false;

  real34Copy(REGISTER_REAL34_DATA(regist), &real34);
  sign = real34IsNegative(&real34);

  // Pre-rounding
  int32ToReal34(10, &value34);
  int32ToReal34(10, &tmp34);
  for(bDigits = 0; bDigits < (isValid12hTime ? 14 : 16); ++bDigits) {
    if(real34CompareAbsLessThan(&h34, &value34)) {
      break;
    }
    real34Multiply(&value34, &tmp34, &value34);
  }
  tDigits = (isValid12hTime ? 14 : 16);
  isValid12hTime = false;

  for(digits = bDigits; digits < tDigits; ++digits) {
    real34Multiply(&real34, &value34, &real34);
  }
  real34ToIntegralValue(&real34, &real34, DEC_ROUND_HALF_UP);
  for(digits = bDigits; digits < tDigits; ++digits) {
    real34Divide(&real34, &value34, &real34);
  }
  tDigits = 0u;
  real34SetPositiveSign(&real34);

  if(hms == 3) {
    //total seconds
    reallocateRegister(regist, dtReal34, 0, amNone);
    real34Copy(&real34, REGISTER_REAL34_DATA(regist));
    return;
  }

  // Seconds
  //real34ToIntegralValue(&real34, &s34, DEC_ROUND_DOWN);
  real34Copy(&real34, &s34);
  real34Subtract(&real34, &s34, &real34); // Fractional part
  int32ToReal34(60, &value34);
  // Minutes
  real34Divide(&s34, &value34, &m34);
  real34ToIntegralValue(&m34, &m34, DEC_ROUND_DOWN);
  real34DivideRemainder(&s34, &value34, &s34);
  // Hours
  real34Divide(&m34, &value34, &h34);
  real34ToIntegralValue(&h34, &h34, DEC_ROUND_DOWN);
  real34DivideRemainder(&m34, &value34, &m34);

  switch(hms) {
    case 0: //h
      int32ToReal34(sign ? -1 : +1, &value34);
      real34Multiply(&h34, &value34, &h34);
      reallocateRegister(regist, dtReal34, 0, amNone);
      real34Copy(&h34, REGISTER_REAL34_DATA(regist));
      break;

    case 1: //m
      int32ToReal34(sign ? -1 : +1, &value34);
      real34Multiply(&m34, &value34, &m34);
      reallocateRegister(regist, dtReal34, 0, amNone);
      real34Copy(&m34, REGISTER_REAL34_DATA(regist));
      break;

    case 2: //s
      int32ToReal34(sign ? -1 : +1, &value34);
      real34Multiply(&s34, &value34, &s34);
      reallocateRegister(regist, dtReal34, 0, amNone);
      real34Copy(&s34, REGISTER_REAL34_DATA(regist));
      break;

    default: ;
  }
}


void dms34ToReal34(uint16_t dms) {
  real34_t angle34;
  calcRegister_t regist = REGISTER_X;
  real34_t value34, d34, m34, s34, fs34;
  real34Copy(REGISTER_REAL34_DATA(regist), &angle34);

  uint32_t m, s, fs;
  int16_t sign;

  real_t temp, degrees, minutes, seconds;

  real34ToReal(&angle34, &temp);

  sign = realIsNegative(&temp) ? -1 : 1;
  realSetPositiveSign(&temp);

  // Get the degrees
  realToIntegralValue(&temp, &degrees, DEC_ROUND_DOWN, &ctxtReal39);

  // Get the minutes
  realSubtract(&temp, &degrees, &temp, &ctxtReal39);
  temp.exponent += 2; // temp = temp * 100
  realToIntegralValue(&temp, &minutes, DEC_ROUND_DOWN, &ctxtReal39);

  // Get the seconds
  realSubtract(&temp, &minutes, &temp, &ctxtReal39);
  temp.exponent += 2; // temp = temp * 100
  realToIntegralValue(&temp, &seconds, DEC_ROUND_DOWN, &ctxtReal39);

  // Get the fractional seconds
  realSubtract(&temp, &seconds, &temp, &ctxtReal39);
  temp.exponent += 2; // temp = temp * 100

  fs = realToUint32C47(&temp);
  s  = realToUint32C47(&seconds);
  m  = realToUint32C47(&minutes);

  if(fs >= 100) {
    fs -= 100;
    s++;
  }

  if(s >= 60) {
    s -= 60;
    m++;
  }

  if(m >= 60)  {
    m -= 60;
    realAdd(&degrees, const_1, &degrees, &ctxtReal39);
  }

  switch(dms)  {
    case 0: //d
      int32ToReal34(sign, &value34);
      realToReal34(&degrees, &d34);
      real34Multiply(&d34, &value34, &d34);
      reallocateRegister(regist, dtReal34, 0, amNone);
      real34Copy(&d34, REGISTER_REAL34_DATA(regist));
      break;

    case 1: //m
      int32ToReal34(m, &m34);
      int32ToReal34(sign, &value34);
      real34Multiply(&m34, &value34, &m34);
      reallocateRegister(regist, dtReal34, 0, amNone);
      real34Copy(&m34, REGISTER_REAL34_DATA(regist));
      break;

    case 2: //s
      int32ToReal34(fs, &fs34);
      int32ToReal34(100, &value34);
      real34Divide(&fs34, &value34, &fs34);

      int32ToReal34(s, &s34);
      real34Add(&s34, &fs34, &s34);

      int32ToReal34(sign, &value34);
      real34Multiply(&s34, &value34, &s34);
      reallocateRegister(regist, dtReal34, 0, amNone);
      real34Copy(&s34, REGISTER_REAL34_DATA(regist));
      break;

    default: ;
  }
}


void notSexa(void) {
  copySourceRegisterToDestRegister(REGISTER_L, REGISTER_X);
  displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    sprintf(errorMessage, "data type %s cannot be converted!", getRegisterDataTypeName(REGISTER_X, false, false));
    moreInfoOnError("In function notSexa:", errorMessage, NULL, NULL);
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
}


void fnHrDeg(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }
  if(getRegisterAngularMode(REGISTER_X) == amDMS && getRegisterDataType(REGISTER_X) == dtReal34) {
    dms34ToReal34(0);
  }
  else if(getRegisterDataType(REGISTER_X) == dtTime) {
    timeToReal34(0);
  }
  else {
    notSexa();
  }
}


void fnMinute(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }
  if(getRegisterAngularMode(REGISTER_X) == amDMS && getRegisterDataType(REGISTER_X) == dtReal34) {
    dms34ToReal34(1);
  }
  else if(getRegisterDataType(REGISTER_X) == dtTime) {
    timeToReal34(1);
  }
  else {
    notSexa();
  }
}


void fnSecond(uint16_t unusedButMandatoryParameter){
  if(!saveLastX()) {
    return;
  }
  if(getRegisterAngularMode(REGISTER_X) == amDMS && getRegisterDataType(REGISTER_X) == dtReal34) {
    dms34ToReal34(2);
  }
  else if(getRegisterDataType(REGISTER_X) == dtTime) {
    timeToReal34(2);
  }
  else {
    notSexa();
  }
}


void fnTimeTo(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }

  if(getRegisterAngularMode(REGISTER_X) == amDMS && getRegisterDataType(REGISTER_X) == dtReal34) {
    dms34ToReal34(0);
    liftStack();
    copySourceRegisterToDestRegister(REGISTER_L, REGISTER_X);
    dms34ToReal34(1);
    liftStack();
    copySourceRegisterToDestRegister(REGISTER_L, REGISTER_X);
    dms34ToReal34(2);
  }
  else if(getRegisterDataType(REGISTER_X) == dtTime) {
    timeToReal34(0);
    liftStack();
    copySourceRegisterToDestRegister(REGISTER_L, REGISTER_X);
    timeToReal34(1);
    liftStack();
    copySourceRegisterToDestRegister(REGISTER_L, REGISTER_X);
    timeToReal34(2);
  }
  else {
    notSexa();
    return;
  }
}


/********************************************//**
 * \brief Check if time is valid (e.g. 10:61:61 is invalid)
 *
 * \param[in] hour real34_t*
 * \param[in] minute real34_t*
 * \param[in] second real34_t*
 * \return bool_t true if valid
 ***********************************************/
bool_t isValidTime(const real34_t *hour, const real34_t *minute, const real34_t *second) {
  real34_t val;

  // second
  real34ToIntegralValue(second, &val, DEC_ROUND_FLOOR), real34Subtract(second, &val, &val);
  if(!real34IsZero(&val)) {
    return false;
  }

  int32ToReal34(0, &val), real34Compare(second, &val, &val);
  if(real34ToInt32(&val) < 0) {
    return false;
  }

  int32ToReal34(59, &val), real34Compare(second, &val, &val);
  if(real34ToInt32(&val) > 0) {
    return false;
  }

  // minute
  real34ToIntegralValue(minute, &val, DEC_ROUND_FLOOR), real34Subtract(minute, &val, &val);
  if(!real34IsZero(&val)) {
    return false;
  }

  int32ToReal34(0, &val), real34Compare(minute, &val, &val);
  if(real34ToInt32(&val) < 0) {
    return false;
  }

  int32ToReal34(59, &val), real34Compare(minute, &val, &val);
  if(real34ToInt32(&val) > 0) {
    return false;
  }

  // hour
  real34ToIntegralValue(hour, &val, DEC_ROUND_FLOOR), real34Subtract(hour, &val, &val);
  if(!real34IsZero(&val)) {
    return false;
  }

  int32ToReal34(0, &val), real34Compare(hour, &val, &val);
  if(real34ToInt32(&val) < 0) {
    return false;
  }

  int32ToReal34(23, &val), real34Compare(hour, &val, &val);
  if(real34ToInt32(&val) > 0) {
    return false;
  }

  // Valid time
  return true;
}


TO_QSPI const calcRegister_t toTimeParamReg[3] = {REGISTER_Z, REGISTER_Y, REGISTER_X};
void fnToTime(uint16_t unusedButMandatoryParameter) {
  real34_t hr, m, s;
  real34_t *part[3];
  uint_fast8_t i;

  if(!saveLastX()) {
    return;
  }

  part[0] = &hr;
  part[1] = &m;
  part[2] = &s; //hrMs

  for(i=0; i<3; ++i) {
    switch(getRegisterDataType(toTimeParamReg[i])) {
      case dtLongInteger:
        convertLongIntegerRegisterToReal34(toTimeParamReg[i], part[i]);
        break;

      case dtReal34:
        if(getRegisterAngularMode(toTimeParamReg[i])) {
          real34ToIntegralValue(REGISTER_REAL34_DATA(toTimeParamReg[i]), part[i], DEC_ROUND_DOWN);
          break;
        }
        /* fallthrough */

      default:
        displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "data type %s cannot be converted to a time!", getRegisterDataTypeName(toTimeParamReg[i], false, false));
          moreInfoOnError("In function fnToTime:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return;
    }
  }

  // valid date
  fnDropY(NOPARAM);
  fnDropY(NOPARAM);

  real34Multiply(const34_3600, &hr, &hr); //hr is now seconds
  real34Multiply(const34_60, &m, &m); //m is now seconds
  real34Add(&hr, &m, &hr);
  real34Add(&hr, &s, &hr);

  reallocateRegister(REGISTER_X, dtTime, 0, amNone);
  real34Copy(&hr, REGISTER_REAL34_DATA(REGISTER_X));
}



// ** ITFRAC *****************************************************************************
// ** IRFRAC parameters: input minimum: 1E-16
// **                  : input maximum: 999 999 999 excluding the IR factor
// **                  : DMX maximum setting 32500
// **                  : Output numerator, excluding IR factor: 999 999 999
// **                  : internal max 1E9-1 after IR constant divided
// **                  : Accuracy 24 digits;
// **                  : Internally uses 12 digits in denom seeker for integer conversions
// **                  : Internally uses 26 digits for denom seeker real
// **                  : Internally uses 26 digits for fraction comparison,
// ** ************************************************************************************
// **
// ** 24 digits guaranteed. 24+2 used, as this has proven to need only 24+1
// ** decNumber number of digits needed for IRFRAC
// ** optimised and verified for the input range offered
#define K_ctxtReal_denominator_finder                           26  //Continuous fraction representation
#define K_ctxtReal_integer_conversion_find_lowest_err_fraction  12  //Determine correct fraction to use from continuous fraction expansion matrix
#define K_ctxtReal_irrational_detection                         26  //This is used for multiple of input constant too large, as well as for irrational tolerance, relative.
#define K_ctxtReal_find_multiple_of_irr                         26  //This is used to determine the whole multiple.

int32_t getSmallestDenom(const real_t *val) { // ignore numerator determined, as this needs to be re-calculated in the main algo
  /*
  ** Adapted from:
  ** https://www.ics.uci.edu/~eppstein/numth/frap.c
  **
  ** find rational approximation to given real number
  ** David Eppstein / UC Irvine / 8 Aug 1993
  ** With corrections from Arno Formella, May 2008
  **
  ** based on the theory of continued fractions
  ** if x = a1 + 1/(a2 + 1/(a3 + 1/(a4 + ...)))
  ** then best approximation is found by truncating this series
  ** (with some adjustments in the last term).
  **
  ** Note the fraction can be recovered as the first column of the matrix
  **  ( a1 1 ) ( a2 1 ) ( a3 1 ) ...
  **  ( 1  0 ) ( 1  0 ) ( 1  0 )
  ** Instead of keeping the sequence of continued fraction terms,
  ** we just keep the last partial product of these matrices.
  */

  realContext_t ctxtReal_denom_finder = ctxtReal39;
  ctxtReal_denom_finder.digits = K_ctxtReal_denominator_finder;
  real_t xx, temp;
  realCopy(val, &xx);

  int32_t m[2][2];
  m[0][0] = 1;

  int32_t maxden, ai, dd;
  if(denMax == 0 || denMax > MAX_DENMAX) {
    maxden = MAX_INTERNAL_DENMAX;
  } else {
    maxden = denMax;
  }
  int32ToReal(maxden,&temp);
  realDivide(const_1on4,&temp,&temp,&ctxtReal_denom_finder);
  if(realCompareLessThan(&xx,&temp)) {
    //printf("Lower than 0.25/DMX, quitting before fraction loop.\n");  // Any value lower than 0.5/DMX will be deemed 0. Make the threshold 1/2 of 0.5/DMX
    dd = 1;
    goto nothingTodo;
  }


  /* initialize matrix */
  m[0][0] = m[1][1] = 1;
  m[0][1] = m[1][0] = 0;

  /* loop finding terms until denom gets too big */
  while(m[1][0] *  ( ai = realToInt32C47(&xx) ) + m[1][1] <= maxden) {
    //printf("  ai = %12i  condition:%8i<%6i ",ai, m[1][0] * ai + m[1][1], maxden ); printf("  m00=%8i m11=%8i m01=%8i m10=%8i   ", m[0][0], m[1][1], m[0][1], m[1][0]); printRealToConsole(&xx,"  xx="," + m[1][1] \n");
    int32_t t;
    t = m[0][0] * ai + m[0][1];
    m[0][1] = m[0][0];
    m[0][0] = t;
    t = m[1][0] * ai + m[1][1];
    m[1][1] = m[1][0];
    m[1][0] = t;

    int32ToReal(ai,&temp);
    realSubtract(&xx,&temp,&xx,&ctxtReal_denom_finder);
    //printf("                                               "); printf("  m00=%8i m11=%8i m01=%8i m10=%8i   ", m[0][0], m[1][1], m[0][1], m[1][0]); printRealToConsole(&xx,"  xx="," + m[1][1] \n");
    if(realIsZero(&xx) || realCompareAbsLessThan(&xx, const_1e_24)) {
      break;  // AF: division by zero
    }
    realDivide(const_1,&xx,&xx,&ctxtReal_denom_finder);
    if(realCompareGreaterThan(&xx,const_10p9__1)) {         // let 1/xx ceiling to const_10p9__1
      realCopy(const_10p9__1,&xx);
    }
    if(realIsSpecial(&xx)) {
      #if defined(PC_BUILD)
        errorf("Representation failure. Quitting fraction loop.");
        printRealToConsole(&xx,"xx:","\n");
        fflush(stderr);
      #endif //PC_BUILD
      dd = 1;
      goto nothingTodo;
    }
  }

  //Pick the correct num/denom from the matrix
  realContext_t ctxtReal_integer_conversion_find_lowest_err_fraction = ctxtReal39;
  ctxtReal_integer_conversion_find_lowest_err_fraction.digits = K_ctxtReal_integer_conversion_find_lowest_err_fraction;
  real_t num1, den1, num2, den2;
  real_t frac1, frac2;
  real_t err1, err2;
  // Convert int32_t integers to reals
  int32ToReal(m[0][0], &num1);
  int32ToReal(m[1][0], &den1);
  int32ToReal(m[0][1], &num2);
  int32ToReal(m[1][1], &den2);
  //decNumberFromInt32(&num1, m[0][0]);
  //decNumberFromInt32(&den1, m[1][0]);
  //decNumberFromInt32(&num2, m[0][1]);
  //decNumberFromInt32(&den2, m[1][1]);

  // Compute fractions num/den: frac = num / den
  realDivide(&num1, &den1, &frac1, &ctxtReal_integer_conversion_find_lowest_err_fraction);
  realDivide(&num2, &den2, &frac2, &ctxtReal_integer_conversion_find_lowest_err_fraction);
  // Compute errors: err = |value - fraction|
  realSubtract(val, &frac1, &err1, &ctxtReal_integer_conversion_find_lowest_err_fraction);
  realCopyAbs(&err1, &err1);
  realSubtract(val, &frac2, &err2, &ctxtReal_integer_conversion_find_lowest_err_fraction);
  realCopyAbs(&err2, &err2);
  real_t cmpResult;           // to store comparison result
  // Compare errors: if err2 < err1, update m accordingly
  realCompare(&err2, &err1, &cmpResult, &ctxtReal_integer_conversion_find_lowest_err_fraction);
  // cmpResult will hold -1 if err2 < err1, 0 if equal, 1 if err2 > err1
  // Check if err2 < err1 and swap output if needed
  // printRealToConsole(&err1,"\nerr1: "," "); printf("%d/%d      ",m[0][0],m[1][0]);
  // printRealToConsole(&err2,  "err2: "," "); printf("%d/%d\n",    m[0][1],m[1][1]);
  if (realIsNegative(&cmpResult)) {
      m[0][0] = m[0][1];
      m[1][0] = m[1][1];
  }
  //ignore numerator for the IRFRAC application - that is re-done later, report the denominator
  dd = m[1][0];
  // printf("----> Selected %d\n",dd);
  if(dd == 0) {
    dd = 1;
  }
nothingTodo:
  //printf(">>> %i / %i \n",  m[0][0], dd);
  return dd;
}

//** Helper to help create and format output string
void changeToSup(uint64_t numer, char *str) {  //numerator
  int16_t  endingZero = 0;
  str[0]=0;
  _numerator(numer, str, &endingZero);

}

//** Helper to help create and format output string
void changeToSub(uint64_t denom, char *str) {  //denominator
  int16_t  endingZero = 1;
  str[0]='/';
  str[1]=0;
  _denominator(denom, str, &endingZero);
}

//** Helper to help create and format output string
void changeToWholeString(int32_t intt, char *str, char *str1) {
  str[0]=0;
  longInteger_t lgInt;
  longIntegerInit(lgInt);
  int32ToLongInteger(intt, lgInt);
  longIntegerToDisplayString(lgInt, str, 30, SCREEN_WIDTH, 20, true);
  strcat(str,str1);
  longIntegerFree(lgInt);
}


//** Main IRFRAC search and conversion function
bool_t checkForAndChange(char *displayString, const real_t *valueReal, const real_t *valueRealAbs, const real_t *constant, const real_t *findingIrrationalTolerance, const char *constantStr,  bool_t frontSpace, bool_t complexMixedNumbers) {
    #define DISALLOW_MIXED_NUMBER_CONSTANTS true // Dont allow 1 e + e/3, rather write 1 1/3 e
    #define DISALLOW_MIXED_NUMBER_COMPLEX   false  // Dont allow 1 2/3 and 1e+2e/3, rather use 5/3 and 5e/3
    realContext_t ctxtReal_irrational_detection = ctxtReal39;
    ctxtReal_irrational_detection.digits = K_ctxtReal_irrational_detection;
    realContext_t ctxtReal_find_multiple_of_irr = ctxtReal39;
    ctxtReal_find_multiple_of_irr.digits = K_ctxtReal_find_multiple_of_irr;

    char cStr[16];
    bool_t useMixedNumbers = getSystemFlag(FLAG_PROPFR) && (DISALLOW_MIXED_NUMBER_COMPLEX ? !complexMixedNumbers : true);
    //printf(">>>## useMixedNumbers %u\n",useMixedNumbers);
    real_t smallestDenomR, newConstant, multipleOfNewConstant, multipleOfNewConstant_ip, multipleOfNewConstant_fp, multConstant;

    char denomStr[20], wholePart[30], resultingIntStr[100], tmpstr[50];
    tmpstr[0]=0;
    denomStr[0] = 0;
    wholePart[0] = 0;
    resultingIntStr[0] = 0;
    int32_t multipleOfNewConstantInteger = 0;
    char sign[2];

    if(realIsPositive(valueReal)) {
      strcpy(sign, "+");
    }
    else {
      strcpy(sign, "-");
    }

    //Returning: Real is too small
    if(realCompareLessThan(valueRealAbs, const_1e_16)) {
      return false;
    }
    //Returning: Multiple of constant is too large
    realDivide(valueRealAbs,constant,&multConstant,&ctxtReal_irrational_detection);                                               //TRYOUT 12 instead of 27
    if(realCompareGreaterThan(&multConstant, const_10p9__1)) {   //reduce whole multiple range to 34-24 = 10 digits. Use 10p9__1 = 999 999 999. (was const_2p31__1 = 2 147 483 647)
      return false;
    }


    //See if the multiplier to the constant has a whole denominator

#define IRFRAC_ENGINE
#ifndef IRFRAC_ENGINE
    //* This section uses the standard fraction() to calculate the denominator
    int16_t sign1, lessEqualGreater;
    uint64_t intPart, numer, denom;
    reallocateRegister(TEMP_REGISTER_1, dtReal34, 0, amNone);
    realToReal34(&multConstant,REGISTER_REAL34_DATA(TEMP_REGISTER_1));
    fraction(TEMP_REGISTER_1, &sign1, &intPart, &numer, &denom, &lessEqualGreater);   //does not yet work in all the frac modes.
    //printf("aaaaaaa: %i%llu + %llu / %llu \n",sign1,intPart,numer,denom);
    int32_t smallestDenom = denom;
#endif //FRACT_ENGINE
#ifdef IRFRAC_ENGINE
    //* This section uses the new special demoninator search engine
    int32_t smallestDenom = getSmallestDenom(&multConstant);                                                    //denominator
#endif //IRFRAC_ENGINE


    //Create a new constant comprising the constant divided by the whole denominator
    int32ToReal(smallestDenom, &smallestDenomR);
    realDivide(constant, &smallestDenomR, &newConstant, &ctxtReal39);

    //See if there is a whole multiple of the new constant
    realDivide(valueRealAbs, &newConstant, &multipleOfNewConstant, &ctxtReal_find_multiple_of_irr);
    realToIntegralValue(&multipleOfNewConstant, &multipleOfNewConstant_ip, DEC_ROUND_HALF_UP, &ctxtReal_find_multiple_of_irr);
    realSubtract(&multipleOfNewConstant, &multipleOfNewConstant_ip, &multipleOfNewConstant_fp, &ctxtReal_find_multiple_of_irr);
    multipleOfNewConstantInteger = abs(realToInt32C47(&multipleOfNewConstant_ip));                              //numerator

    //See if the ip is out of range (use the Real check not the integer check to protect agains > 32 bit integer max)
    if(realCompareAbsGreaterThan(&multipleOfNewConstant_ip, const_10p9__1)) {   //reduce whole multiple range to 34-24 = 10 digits. Use 10p9__1 = 999 999 999. (was const_2p31__1 = 2 147 483 647)
      return false;
    }

  real_t findingIrrationalTolerance1;
  realMultiply(findingIrrationalTolerance, &smallestDenomR, &findingIrrationalTolerance1, &ctxtReal_irrational_detection);         // do relative convergence // MUST TRY 12. SEEMS TO WORK ON 12



// DEBUG CODE
//                                printRealToConsole(constant,"\n\nconstant=","\n");
//                                printRealToConsole(valueReal,"valueReal=","\n");
//                                printRealToConsole(&multConstant,"multConstant=","\n");
//                                printf("smallestDenom:%i\n",smallestDenom);
//                                printRealToConsole(&newConstant,"newConstant=","\n");
//                                printRealToConsole(&multipleOfNewConstant,"multipleOfNewConstant=","\n");
//                                printRealToConsole(&multipleOfNewConstant_ip,"multipleOfNewConstant_ip=","\n");
//                                printRealToConsole(&multipleOfNewConstant_fp,"multipleOfNewConstant_fp=","\n");

//                                printf(">>>multipleOfNewConstantInteger:%i>1? SmallestDenom:%i\n", multipleOfNewConstantInteger, smallestDenom);
//                                printf(" Numer=%i Denom:%i\n---\n", multipleOfNewConstantInteger, smallestDenom);
//                                printRealToConsole(&multipleOfNewConstant_fp,"fp:","--\n");
//                                printRealToConsole(findingIrrationalTolerance,"tol:","--\n");
//                                printf("realCompareAbsLessThan(&multipleOfNewConstant_fp,findingIrrationalTolerance):%i\n",realCompareAbsLessThan(&multipleOfNewConstant_fp,findingIrrationalTolerance));
//                                printf(">>> %i ", multipleOfNewConstantInteger);
//                                printf("QQ:%s§\n",displayString);
//                                char teststr[1000];
//                                char teststr1[1000];
//                                sprintf(teststr,">>>@@@1 |%s|%s|%s| %i %i\n", resultingIntStr, constantStr, denomStr, (int16_t)stringByteLength(resultingIntStr)-1, resultingIntStr[stringByteLength(resultingIntStr)-1]);
//                                stringToASCII(teststr,teststr1);
//                                printf("%s\n",teststr1);
//                                printf(">>>multipleOfNewConstantInteger:%i>=1? realCompareAbsLessThan(&multipleOfNewConstant_fp,findingIrrationalTolerance):%i\n", multipleOfNewConstantInteger, realCompareAbsLessThan(&multipleOfNewConstant_fp,findingIrrationalTolerance));


    if((DISALLOW_MIXED_NUMBER_CONSTANTS && constantStr[0]!=0 && multipleOfNewConstantInteger > smallestDenom) && useMixedNumbers && smallestDenom != 1) {   //remove this last "&& useMixedNumbers" to change to "3/4 e" instead of "3e/4"
      cStr[0] = 0;
    }
    else {
      strcpy(cStr,constantStr);
    }


// DEBUG CODE
//                                printRealToConsole(valueReal,"\n\nInputvalue: valueReal=","\n");
//                                printRealToConsole(constant,"    constant=","\n");
//                                printf("    §%s§   §%s§   §%s§\n", resultingIntStr, constantStr, denomStr);
//                                printRealToConsole(&findingIrrationalTolerance1,"findingIrrationalTolerance1=","\n");
//                                char displayString2[200];
//                                stringToASCII(constantStr,displayString2);
//                                printf("constantStr:%s\n",displayString2);
//                                printf("Numerator: multipleOfNewConstantInteger   %i\n",multipleOfNewConstantInteger);
//                                printf("Denominator: smallestDenom                         /            %i\n",smallestDenom);
//                                printRealToConsole(&multipleOfNewConstant_fp,"&multipleOfNewConstant_fp=","\n");
//                                char displayString1[200];
//                                stringToASCII(resultingIntStr,displayString1); printf("BBB1 ---> %s %u %u %u %u %u %u %u %u\n",displayString1,(uint8_t)(displayString[0]),(uint8_t)(displayString[1]),(uint8_t)(displayString[2]),(uint8_t)(displayString[3]),(uint8_t)(displayString[4]),(uint8_t)(displayString[5]),(uint8_t)(displayString[6]),(uint8_t)(displayString[7]));

    if(multipleOfNewConstantInteger >= 1 && realCompareAbsLessThan(&multipleOfNewConstant_fp,&findingIrrationalTolerance1)) {

// DEBUG CODE
//                                printf("A whole multiple %i of the 'new' constant exists\n", multipleOfNewConstantInteger);
//                                printf("  useMixedNumbers = %u\n", useMixedNumbers);

      if(multipleOfNewConstantInteger > smallestDenom  &&  smallestDenom > 1  && multipleOfNewConstantInteger != 0 && useMixedNumbers && smallestDenom != 1) {   // Numer > Denom;
        int32_t wholeInteger = multipleOfNewConstantInteger / smallestDenom;
        multipleOfNewConstantInteger = multipleOfNewConstantInteger - (wholeInteger * smallestDenom);

// DEBUG CODE
//                                printf("B  wholeInteger %i, multipleOfNewConstantInteger %i of the 'new' constant exists\n", wholeInteger, multipleOfNewConstantInteger);
        char useMixedNumbersSep[3];
        if(cStr[0]==0) {                                                                                          // no constant
          useMixedNumbersSep[0] = STD_SPACE_4_PER_EM[0];
          useMixedNumbersSep[1] = STD_SPACE_4_PER_EM[1];
          useMixedNumbersSep[2] = 0;
          changeToWholeString(wholeInteger,wholePart,useMixedNumbersSep);
          strcat(wholePart,useMixedNumbersSep);                                                                   // "1 "
        }
        else {                                                                                                    // constant with numbers
          useMixedNumbersSep[0] = sign[0];
          useMixedNumbersSep[1] = sign[1];
          useMixedNumbersSep[2] = 0;
          if(wholeInteger == 1) {
            sprintf(wholePart, "%s%s", cStr, useMixedNumbersSep);                                                 // "e+"
          }
          else {
            changeToWholeString(wholeInteger,wholePart,PRODUCT_SIGN);
            strcat(wholePart,cStr);
            strcat(wholePart,useMixedNumbersSep);                                                                 // "2xe+"
          }
        }
      }

      if(cStr[0] == 0) {                                                                                          // no constant
        if(smallestDenom > 1) {
          changeToSup(multipleOfNewConstantInteger, tmpstr);                                                      // numerator
        }
        else {
          return false;
          //return false, to abort and use standard decimal instead of "1."
          //sprintf(tmpstr,"%i",(int)multipleOfNewConstantInteger);                                                 //==1
          //strcat(tmpstr,RADIX34_MARK_STRING);
        }
        sprintf(resultingIntStr, "%s%s", wholePart, tmpstr);                                                        // "1 1"
      }
      else {                                                                                                      // constant
        if(multipleOfNewConstantInteger == 1) {
          sprintf(resultingIntStr,"%s", wholePart);                                                                 // "e+" or "2xe+"
        }
        else {
          sprintf(tmpstr,"%i%s",(int)multipleOfNewConstantInteger,PRODUCT_SIGN);
          sprintf(resultingIntStr, "%s%s", wholePart, tmpstr);                                                      // "e+1" or "2xe+1"
        }
      }
    } else {                                                                                  // A whole multiple %i of the 'new' constant does not exist
        if(smallestDenom == 1) {
          return false;   //unlikely
          //return false, to abort and use standard decimal instead of "1."
          //sprintf(resultingIntStr,"%i", (int)multipleOfNewConstantInteger);                                       // if denom = 1, then use large font
        }
        else {
          changeToSup(multipleOfNewConstantInteger, resultingIntStr);                                             // if denom <> 0, then use superscript, knowing there is a denom
        }

    }

// DEBUG CODE
//                                printf("QQ1 %s\n",wholePart);       printf("BBB1 ---> %u %u %u %u %u %u %u %u\n",(uint8_t)(wholePart[0]),(uint8_t)(wholePart[1]),(uint8_t)(wholePart[2]),(uint8_t)(wholePart[3]),(uint8_t)(wholePart[4]),(uint8_t)(wholePart[5]),(uint8_t)(wholePart[6]),(uint8_t)(wholePart[7]));
//                                printf("  2 %s\n",tmpstr);          printf("BBB2 ---> %u %u %u %u %u %u %u %u\n",(uint8_t)(tmpstr[0]),(uint8_t)(tmpstr[1]),(uint8_t)(tmpstr[2]),(uint8_t)(tmpstr[3]),(uint8_t)(tmpstr[4]),(uint8_t)(tmpstr[5]),(uint8_t)(tmpstr[6]),(uint8_t)(tmpstr[7]));
//                                printf("  3 %s\n",resultingIntStr); printf("BBB3 ---> %u %u %u %u %u %u %u %u\n",(uint8_t)(resultingIntStr[0]),(uint8_t)(resultingIntStr[1]),(uint8_t)(resultingIntStr[2]),(uint8_t)(resultingIntStr[3]),(uint8_t)(resultingIntStr[4]),(uint8_t)(resultingIntStr[5]),(uint8_t)(resultingIntStr[6]),(uint8_t)(resultingIntStr[7]));
//                                char teststr[1000];
//                                sprintf(teststr,">>>@@@2 |%s|%s|%s| %i %i\n", resultingIntStr, constantStr, denomStr, (int16_t)stringByteLength(resultingIntStr)-1, resultingIntStr[stringByteLength(resultingIntStr)-1]);
//                                char teststr2[1000];
//                                stringToASCII(teststr,teststr2);
//                                printf("%s\n",teststr2);

    if(smallestDenom > 1) {
      changeToSub(smallestDenom, denomStr);                                                                     // "/12"
    }

    if((resultingIntStr[stringByteLength(resultingIntStr)-1]==' ' || resultingIntStr[max(0,stringByteLength(resultingIntStr)-1)]==0) &&  denomStr[0]=='/' && cStr[0]==0) {
      sprintf(tmpstr, STD_SUP_1 "%s", denomStr);
      strcpy(denomStr, tmpstr);
    }

  real_t roundingTolerance1;
  irfractionTolerence(smallestDenom * 6 + 1, &roundingTolerance1);                                              // relative convergence, add (6x+1) i.e about 0.43 digits + 1 for safety margin)

// DEBUG CODE
//                               printRealToConsole(&multipleOfNewConstant_fp,"&multipleOfNewConstant_fp=","\n");
//                               printRealToConsole(&roundingTolerance1,"roundingTolerance1=","\n");
//                               printRealToConsole(&findingIrrationalTolerance1,"findingIrrationalTolerance1=","\n");

    displayString[0] = 0;
    if(!realCompareAbsGreaterThan(&multipleOfNewConstant_fp,&findingIrrationalTolerance1)) {                                     // irrational tolerance found, show irrational and fraction
      if(!realCompareAbsLessThan(&multipleOfNewConstant_fp,&roundingTolerance1) ){                                               // prepend the tags; FDIGS=34 is normal, i.e. no lying, meaning opening up the tolerance band for zero
        strcat(displayString, STD_ALMOST_EQUAL);
      }

      if(sign[0]=='+') {
        if(frontSpace) {
          strcat(displayString, STD_SPACE_4_PER_EM);  //changed, not allowing for a space equal length to "-"
          if(resultingIntStr[0] !=0 ) {
            strcat(displayString, resultingIntStr);
          }
          strcat(displayString,cStr);
          strcat(displayString,denomStr);                              // " 2xe+" "e" "/3"
        }
        else {
          if(resultingIntStr[0] != 0) {
            strcat(displayString, resultingIntStr);
          }
          strcat(displayString,cStr);
          strcat(displayString,denomStr);                              // "2xe+" "e" "/3"
        }
      }
      else { // "-"
        strcat(displayString, STD_SPACE_4_PER_EM "-");
        if(resultingIntStr[0] !=0 ) {
          strcat(displayString, resultingIntStr);
        }
        strcat(displayString,cStr);
        strcat(displayString,denomStr);                                // "-2xe+" "e" "/3"
      }

      if(cStr[0] == 0 && constantStr[0] !=0) {                         // "-2/3" "e"
        strcat(displayString,STD_SPACE_4_PER_EM);
        strcat(displayString,PRODUCT_SIGN);
        strcat(displayString,STD_SPACE_4_PER_EM);
        strcat(displayString,constantStr);
      }

      return true;             //successful IRFRAC conversion, displaying as fraction
    }
    else {
      return false;            //unsuccessful IRFRAC conversion, displaying as decimal
    }
  }


void fnSafeReset (uint16_t unusedButMandatoryParameter) {
  if(!getSystemFlag(FLAG_G_DOUBLETAP) && !getSystemFlag(FLAG_SHFT_4s) && !getSystemFlag(FLAG_HOME_TRIPLE) && !getSystemFlag(FLAG_MYM_TRIPLE)) {
    setSystemFlag  (FLAG_FGLNFUL);
    clearSystemFlag(FLAG_FGLNLIM);
    setSystemFlag  (FLAG_G_DOUBLETAP);
    setSystemFlag  (FLAG_SHFT_4s);
    setSystemFlag  (FLAG_HOME_TRIPLE);
    clearSystemFlag(FLAG_MYM_TRIPLE);
    clearSystemFlag(FLAG_BASE_HOME);
    setSystemFlag  (FLAG_BASE_MYM);
  }
  else {
    clearSystemFlag(FLAG_FGLNFUL);
    clearSystemFlag(FLAG_FGLNLIM);
    clearSystemFlag(FLAG_G_DOUBLETAP);
    clearSystemFlag(FLAG_SHFT_4s);
    clearSystemFlag(FLAG_HOME_TRIPLE);
    clearSystemFlag(FLAG_MYM_TRIPLE);
    clearSystemFlag(FLAG_BASE_HOME);
    setSystemFlag  (FLAG_BASE_MYM);
  }
}


#if !defined(TESTSUITE_BUILD)
  static void assignToMyMenu_(uint16_t position) {
    if(position < 18) {
      _assignItem(&userMenuItems[position]);
    }
    cachedDynamicMenu = 0;
  }


  static void assignToMyAlpha_(uint16_t position) {
    if(position < 18) {
      _assignItem(&userAlphaItems[position]);
    }
    cachedDynamicMenu = 0;
  }
#endif // !TESTSUITE_BUILD



// Pseudo ITM constants for conditional ENG C47/R47 ribbons
#define ITM_RIBBON_ENG_C47  32000
#define ITM_RIBBON_ENG_R47  32001

// Ribbon mappings: [param, fn1, fn2, fn3, fn4, fn5, fn6]
#if !defined(TESTSUITE_BUILD)
TO_QSPI static const int16_t ribbonMappings[][7] = {
  {ITM_RIBBON_ENG_C47,  -MNU_CPX,      -MNU_MATX,     ITM_CONSTpi,   ITM_op_j,      ITM_EXP,       -MNU_TRG_C47},
  {ITM_RIBBON_ENG_R47,  ITM_op_j,      -MNU_CPX,      ITM_CONSTpi,   -MNU_MATX,     -MNU_TRG_R47,  ITM_EXP},

  {ITM_RIBBON_SAV,      ITM_SYSTEM2,   ITM_ACTUSB,    ITM_SAVE,      ITM_LOAD,      ITM_SAVEST,    ITM_LOADST},
  {ITM_RIBBON_SAV2,     ITM_SYSTEM2,   ITM_ACTUSB,    ITM_WRITEP,    ITM_READP,     ITM_SAVEST,    ITM_LOADST},
  {ITM_RIBBON_FIN,      ITM_PC,        ITM_DELTAPC,   ITM_YX,        ITM_SQUARE,    ITM_10x,       -MNU_FIN},
  {ITM_RIBBON_FIN2,     ITM_PCPMG,     ITM_PCT,       ITM_PC,        ITM_DELTAPC,   -MNU_TVM,      -MNU_FIN},
  {ITM_RIBBON_CPX,      ITM_DRG,       ITM_CC,        ITM_EE_EXP_TH, ITM_EXP,       ITM_op_j_pol,  ITM_op_j},
  {ITM_RIBBON_C47,      ITM_DRG,       ITM_YX,        ITM_SQUARE,    ITM_10x,       ITM_EXP,       ITM_op_j_pol},
  {ITM_RIBBON_C47PL,    ITM_DRG,       ITM_DSP,       ITM_DREAL,     ITM_FF,        ITM_Rup,       ITM_XFACT},
  {ITM_RIBBON_R47,      ITM_op_j,      ITM_op_j_pol,  ITM_XFACT,     ITM_XTHROOT,   ITM_10x,       ITM_EXP},
  {ITM_RIBBON_R47PL,    ITM_TIMER,     ITM_DSP,       ITM_DREAL,     ITM_FF,        -MNU_LOOP,     -MNU_TEST},
};
#endif //!TESTSUITE_BUILD


void fnRESET_MyM(uint16_t param) {
  //Pre-assign the MyMenu
  #if !defined(TESTSUITE_BUILD)
    clearSystemFlag(FLAG_BASE_MYM);                                                   //switch off menu to prevent slow updating of 6 menu items

    int16_t searchParam = param;
    if(param == ITM_RIBBON_ENG) {
      searchParam = isR47FAM ? ITM_RIBBON_ENG_R47 : ITM_RIBBON_ENG_C47;
    }

    uint16_t i;
    for(int8_t fn = 1; fn <= 6; fn++) {
      for(i = 0; i < nbrOfElements(ribbonMappings); i++) {
        if(ribbonMappings[i][0] == searchParam) {
          if(fn >= 1 && fn <= 6) {
            itemToBeAssigned = ribbonMappings[i][fn];
          }
          else {
            itemToBeAssigned = ASSIGN_CLEAR;
          }
          break;
        }
      }
      if(i >= nbrOfElements(ribbonMappings)) {
        itemToBeAssigned = ASSIGN_CLEAR;
      }



      if(itemToBeAssigned == -MNU_PFN) {
        strcpy(aimBuffer,"P.FN");
        assignGetName1();
      }
      else if(itemToBeAssigned == -MNU_HOME) {
        strcpy(aimBuffer,"HOME");
        assignGetName1();
      }

      assignToMyMenu_(fn - 1);
      if(param == 0) {
        itemToBeAssigned = ASSIGN_CLEAR;
        assignToMyMenu_(6 + fn - 1);
        itemToBeAssigned = ASSIGN_CLEAR;
        assignToMyMenu_(12 + fn - 1);
      }
    }
    setSystemFlag(FLAG_BASE_MYM);                                           //JM Menu system default (removed from reset_jm_defaults)
    refreshScreen(42);
  #endif // !TESTSUITE_BUILD
}


void fnRESET_Mya(void){
  //Pre-assign the MyAlpha Menu                   //JM
  #if !defined(TESTSUITE_BUILD)
    for(int8_t fn = 1; fn <= 6; fn++) {
      itemToBeAssigned = ASSIGN_CLEAR;
      assignToMyAlpha_(fn - 1);
      itemToBeAssigned = ASSIGN_CLEAR;
      assignToMyAlpha_(6 + fn - 1);
      itemToBeAssigned = ASSIGN_CLEAR;
      assignToMyAlpha_(12 + fn - 1);
    }
//    itemToBeAssigned = -MNU_ALPHA;
//    assignToMyAlpha_(5);
    refreshScreen(43);
  #endif // !TESTSUITE_BUILD
}


//Softmenus:
//--------------------------------------------------------------------------------------------
int16_t mm(int16_t id) {
  int16_t m;
  m = 0;
  if(id != 0) { // Search by ID
    while(softmenu[m].menuItem != 0) {
      //printf(">>> mm %d %d %d %s \n",id, m, softmenu[m].menuItem, indexOfItems[-softmenu[m].menuItem].itemSoftmenuName);
      if(softmenu[m].menuItem == id) {
        //printf("####>> mm() broken out id=%i m=%i\n",id,m);
        break;
      }
      m++;
    }
  }
  return m;
}


#if !defined(TESTSUITE_BUILD)
//EXTRA DRAWINGS FOR RADIO_BUTTON AND CHECK_BOX AND MB_MACRO

  // Helper function to draw pixel array
  static inline void drawPixelArray(uint32_t xx, uint32_t yy, const uint8_t coords[][2], uint8_t count, bool white) {
    for(uint8_t i = 0; i < count; i++) {
      if(white) {
        setWhitePixel(xx + coords[i][0], yy + coords[i][1]);
      }
      else {
        setBlackPixel(xx + coords[i][0], yy + coords[i][1]);
      }
    }
  }

  // Radio button checked: 11×11 circle with filled center
  // Draws visible black ring with black center dot
  void RB_CHECKED(uint32_t xx, uint32_t yy) {
    // BLACK pixels: outer ring + center dot (visible parts)
    TO_QSPI static const uint8_t rbBlack[][2] = {
      // Column 1: yy+3..yy+7
      {1,3},{1,4},{1,5},{1,6},{1,7},
      // Column 2: yy+2..yy+8
      {2,2},{2,3},{2,4},{2,5},{2,6},{2,7},{2,8},
      // Column 3: yy+1..yy+3, yy+7..yy+9
      {3,1},{3,2},{3,3},{3,7},{3,8},{3,9},
      // Column 4: yy+1, yy+2, yy+4..yy+6, yy+8, yy+9
      {4,1},{4,2},{4,4},{4,5},{4,6},{4,8},{4,9},
      // Column 5: yy+1, yy+2, yy+4..yy+6, yy+8, yy+9
      {5,1},{5,2},{5,4},{5,5},{5,6},{5,8},{5,9},
      // Column 6: yy+1, yy+2, yy+4..yy+6, yy+8, yy+9
      {6,1},{6,2},{6,4},{6,5},{6,6},{6,8},{6,9},
      // Column 7: yy+1..yy+3, yy+7..yy+9
      {7,1},{7,2},{7,3},{7,7},{7,8},{7,9},
      // Column 8: yy+2..yy+8
      {8,2},{8,3},{8,4},{8,5},{8,6},{8,7},{8,8},
      // Column 9: yy+3..yy+7
      {9,3},{9,4},{9,5},{9,6},{9,7}
    };

    // WHITE pixels: background borders and gaps creating ring shape
    TO_QSPI static const uint8_t rbWhite[][2] = {
      // Column 0: yy+2..yy+8 (left border)
      {0,2},{0,3},{0,4},{0,5},{0,6},{0,7},{0,8},
      // Column 1: yy+1, yy+8, yy+9
      {1,1},{1,8},{1,9},
      // Column 2: yy+0, yy+1, yy+9, yy+10
      {2,0},{2,1},{2,9},{2,10},
      // Column 3: yy+0, yy+4..yy+6, yy+10
      {3,0},{3,4},{3,5},{3,6},{3,10},
      // Column 4: yy+0, yy+3, yy+7, yy+10
      {4,0},{4,3},{4,7},{4,10},
      // Column 5: yy+0, yy+3, yy+7, yy+10
      {5,0},{5,3},{5,7},{5,10},
      // Column 6: yy+0, yy+3, yy+7, yy+10
      {6,0},{6,3},{6,7},{6,10},
      // Column 7: yy+0, yy+4..yy+6, yy+10
      {7,0},{7,4},{7,5},{7,6},{7,10},
      // Column 8: yy+0, yy+1, yy+9, yy+10
      {8,0},{8,1},{8,9},{8,10},
      // Column 9: yy+1, yy+8, yy+9
      {9,1},{9,8},{9,9}
    };

    drawPixelArray(xx, yy, rbBlack, sizeof(rbBlack)/sizeof(rbBlack[0]), false);
    drawPixelArray(xx, yy, rbWhite, sizeof(rbWhite)/sizeof(rbWhite[0]), true);
  }

  // Radio button unchecked: 11×11 circle with hollow center
  void RB_UNCHECKED(uint32_t xx, uint32_t yy) {
    // BLACK pixels: outer ring only (visible parts)
    TO_QSPI static const uint8_t rbBlack[][2] = {
      // Column 1: yy+3..yy+7
      {1,3},{1,4},{1,5},{1,6},{1,7},
      // Column 2: yy+2, yy+3, yy+7, yy+8
      {2,2},{2,3},{2,7},{2,8},
      // Column 3: yy+1, yy+2, yy+8, yy+9
      {3,1},{3,2},{3,8},{3,9},
      // Column 4: yy+1, yy+9
      {4,1},{4,9},
      // Column 5: yy+1, yy+9
      {5,1},{5,9},
      // Column 6: yy+1, yy+9
      {6,1},{6,9},
      // Column 7: yy+1, yy+2, yy+8, yy+9
      {7,1},{7,2},{7,8},{7,9},
      // Column 8: yy+2, yy+3, yy+7, yy+8
      {8,2},{8,3},{8,7},{8,8},
      // Column 9: yy+3..yy+7
      {9,3},{9,4},{9,5},{9,6},{9,7}
    };

    // WHITE pixels: background borders, gaps, and hollow center
    TO_QSPI static const uint8_t rbWhite[][2] = {
      // Column 0: yy+2..yy+8 (left border)
      {0,2},{0,3},{0,4},{0,5},{0,6},{0,7},{0,8},
      // Column 1: yy+1, yy+8, yy+9
      {1,1},{1,8},{1,9},
      // Column 2: yy+0, yy+1, yy+9, yy+10
      {2,0},{2,1},{2,9},{2,10},
      // Column 3: yy+0, yy+10
      {3,0},{3,10},
      // Column 4: yy+0, yy+10
      {4,0},{4,10},
      // Column 5: yy+0, yy+10
      {5,0},{5,10},
      // Column 6: yy+0, yy+10
      {6,0},{6,10},
      // Column 7: yy+0, yy+10
      {7,0},{7,10},
      // Column 8: yy+0, yy+1, yy+9, yy+10
      {8,0},{8,1},{8,9},{8,10},
      // Column 9: yy+1, yy+8, yy+9
      {9,1},{9,8},{9,9}
    };

    drawPixelArray(xx, yy, rbBlack, sizeof(rbBlack)/sizeof(rbBlack[0]), false);
    drawPixelArray(xx, yy, rbWhite, sizeof(rbWhite)/sizeof(rbWhite[0]), true);
  }

  // Checkbox checked: 11×11 square with filled center
  void CB_CHECKED(uint32_t xx, uint32_t yy) {
    lcd_fill_rect(xx, yy-1, 10, 11, 0);  // Clear background area

    // BLACK pixels: outer square + inner checkmark pattern
    TO_QSPI static const uint8_t cbBlack[][2] = {
      // Column 1: yy+1..yy+9 (full)
      {1,1},{1,2},{1,3},{1,4},{1,5},{1,6},{1,7},{1,8},{1,9},
      // Column 2: yy+1..yy+9 (full)
      {2,1},{2,2},{2,3},{2,4},{2,5},{2,6},{2,7},{2,8},{2,9},
      // Column 3: yy+1, yy+2, yy+8, yy+9
      {3,1},{3,2},{3,8},{3,9},
      // Column 4: yy+1, yy+2, yy+4..yy+6, yy+8, yy+9
      {4,1},{4,2},{4,4},{4,5},{4,6},{4,8},{4,9},
      // Column 5: yy+1, yy+2, yy+4..yy+6, yy+8, yy+9
      {5,1},{5,2},{5,4},{5,5},{5,6},{5,8},{5,9},
      // Column 6: yy+1, yy+2, yy+4..yy+6, yy+8, yy+9
      {6,1},{6,2},{6,4},{6,5},{6,6},{6,8},{6,9},
      // Column 7: yy+1, yy+2, yy+8, yy+9
      {7,1},{7,2},{7,8},{7,9},
      // Column 8: yy+1..yy+9 (full)
      {8,1},{8,2},{8,3},{8,4},{8,5},{8,6},{8,7},{8,8},{8,9},
      // Column 9: yy+1..yy+9 (full)
      {9,1},{9,2},{9,3},{9,4},{9,5},{9,6},{9,7},{9,8},{9,9}
    };

    // WHITE pixels: background borders and hollow center area
    TO_QSPI static const uint8_t cbWhite[][2] = {
      // Column 0: yy+0..yy+10 (left border)
      {0,0},{0,1},{0,2},{0,3},{0,4},{0,5},{0,6},{0,7},{0,8},{0,9},{0,10},
      // Column 1: yy+0, yy+10
      {1,0},{1,10},
      // Column 2: yy+0, yy+10
      {2,0},{2,10},
      // Column 3: yy+0, yy+3..yy+7, yy+10 (creates hollow)
      {3,0},{3,3},{3,4},{3,5},{3,6},{3,7},{3,10},
      // Column 4: yy+0, yy+3, yy+7, yy+10 (creates hollow)
      {4,0},{4,3},{4,7},{4,10},
      // Column 5: yy+0, yy+3, yy+7, yy+10 (creates hollow)
      {5,0},{5,3},{5,7},{5,10},
      // Column 6: yy+0, yy+3, yy+7, yy+10 (creates hollow)
      {6,0},{6,3},{6,7},{6,10},
      // Column 7: yy+0, yy+3..yy+7, yy+10 (creates hollow)
      {7,0},{7,3},{7,4},{7,5},{7,6},{7,7},{7,10},
      // Column 8: yy+0, yy+10
      {8,0},{8,10},
      // Column 9: yy+0, yy+10
      {9,0},{9,10}
    };

    drawPixelArray(xx, yy, cbBlack, sizeof(cbBlack)/sizeof(cbBlack[0]), false);
    drawPixelArray(xx, yy, cbWhite, sizeof(cbWhite)/sizeof(cbWhite[0]), true);
  }

  // Checkbox unchecked: 11×11 square with hollow center
  void CB_UNCHECKED(uint32_t xx, uint32_t yy) {
    lcd_fill_rect(xx, yy-1, 10, 11, 0);  // Clear background area

    // BLACK pixels: outer square only (hollow inside)
    TO_QSPI static const uint8_t cbBlack[][2] = {
      // Column 1: yy+1..yy+9 (full)
      {1,1},{1,2},{1,3},{1,4},{1,5},{1,6},{1,7},{1,8},{1,9},
      // Column 2: yy+1, yy+9 (top/bottom only)
      {2,1},{2,9},
      // Column 3: yy+1, yy+9
      {3,1},{3,9},
      // Column 4: yy+1, yy+9
      {4,1},{4,9},
      // Column 5: yy+1, yy+9
      {5,1},{5,9},
      // Column 6: yy+1, yy+9
      {6,1},{6,9},
      // Column 7: yy+1, yy+9
      {7,1},{7,9},
      // Column 8: yy+1, yy+9
      {8,1},{8,9},
      // Column 9: yy+1..yy+9 (full)
      {9,1},{9,2},{9,3},{9,4},{9,5},{9,6},{9,7},{9,8},{9,9}
    };

    // WHITE pixels: background borders
    TO_QSPI static const uint8_t cbWhite[][2] = {
      // Column 0: yy+0..yy+10 (left border)
      {0,0},{0,1},{0,2},{0,3},{0,4},{0,5},{0,6},{0,7},{0,8},{0,9},{0,10},
      // Column 1: yy+0, yy+10
      {1,0},{1,10},
      // Column 2: yy+0, yy+10
      {2,0},{2,10},
      // Column 3: yy+0, yy+10
      {3,0},{3,10},
      // Column 4: yy+0, yy+10
      {4,0},{4,10},
      // Column 5: yy+0, yy+10
      {5,0},{5,10},
      // Column 6: yy+0, yy+10
      {6,0},{6,10},
      // Column 7: yy+0, yy+10
      {7,0},{7,10},
      // Column 8: yy+0, yy+10
      {8,0},{8,10},
      // Column 9: yy+0, yy+10
      {9,0},{9,10}
    };

    drawPixelArray(xx, yy, cbBlack, sizeof(cbBlack)/sizeof(cbBlack[0]), false);
    drawPixelArray(xx, yy, cbWhite, sizeof(cbWhite)/sizeof(cbWhite[0]), true);
  }

  // ◇ Diamond/macro button outline: 11×11 diamond shape
  void MB_MACRO(uint32_t xx, uint32_t yy) {
    // Diamond outline coordinates (x_offset, y_offset from xx, yy)
    // Forms diamond shape: top point → widest middle → bottom point
    TO_QSPI static const uint8_t diamond[][2] = {
      {5, 0},                                       // Top point
      {4, 1}, {6, 1},                               // Row 1: 2 pixels
      {3, 2}, {7, 2},                               // Row 2: 2 pixels
      {2, 3}, {8, 3},                               // Row 3: 2 pixels
      {1, 4}, {9, 4},                               // Row 4: 2 pixels
      {0, 5}, {10, 5},                              // Row 5: widest point (2 pixels)
      {1, 6}, {9, 6},                               // Row 6: 2 pixels
      {2, 7}, {8, 7},                               // Row 7: 2 pixels
      {3, 8}, {7, 8},                               // Row 8: 2 pixels
      {4, 9}, {6, 9},                               // Row 9: 2 pixels
      {5, 10}                                       // Bottom point
    };
    #define DOUBLE                                  // Draw double-width for visibility
    #define offs 1                                  // X-offset adjustment
    for(uint8_t i = 0; i < sizeof(diamond) / sizeof(diamond[0]); i++) {
      placePixel(xx + diamond[i][0] - offs, yy + diamond[i][1]);
      #ifdef DOUBLE
        placePixel(xx + diamond[i][0] - offs, yy + diamond[i][1] - 1);  // Duplicate row above for thickness
      #endif
    }
  }

  // ◇ Diamond/macro button checked: diamond outline + center fill
  void MB_MACRO_CHECKED(uint32_t xx, uint32_t yy) {
    MB_MACRO(xx,yy);  // Draw outline first

    // Diamond interior fill coordinates (x_offset, y_offset from xx, yy)
    // Fills center area to indicate checked state
    TO_QSPI static const uint8_t diamond[][2] = {
      {5, 3},                                       // Row 3: 1 pixel
      {4, 4}, {5, 4}, {6, 4},                       // Row 4: 3 pixels
      {3, 5}, {4, 5}, {5, 5}, {6, 5}, {7, 5},       // Row 5: 5 pixels (widest)
      {3, 6}, {4, 6}, {5, 6}, {6, 6}, {7, 6},       // Row 6: 5 pixels (widest)
      {4, 7}, {5, 7}, {6, 7},                       // Row 7: 3 pixels
      {5, 8}                                        // Row 8: 1 pixel
    };
    for(uint8_t i = 0; i < sizeof(diamond) / sizeof(diamond[0]); i++) {
      placePixel(xx + diamond[i][0] - offs, yy + diamond[i][1] - 1);  // Match DOUBLE offset from outline
    }
  }

#endif // !TESTSUITE_BUILD


void fnSetBCD (uint16_t bcd) {
  switch(bcd) {
    case BCD9c:
    case BCD10c:
    case BCDu:
      bcdDisplaySign = bcd;
      break;
    default: ;
  }
}


void fnLongPressSwitches (uint16_t option) {
  switch(option) {
    case RBX_F14:
    case RBX_F124:
    case RBX_F1234:
      LongPressF = option;
      break;
    case RBX_M14   :
    case RBX_M124  :
    case RBX_M1234 :
      LongPressM = option;
      break;
    default:
      LongPressM = RBX_M1234;
      LongPressF = RBX_F124;
  }
}


