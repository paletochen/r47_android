// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors


/********************************************//**
 * \file nextStep.c
 ***********************************************/

#include "c47.h"


uint8_t *countOpBytes(uint8_t *step, uint16_t paramMode) {
  uint8_t opParam = *(uint8_t *)(step++);

  switch(paramMode) {
    case PARAM_DECLARE_LABEL: {
      if(opParam <= LAST_LOCAL_LABEL) { // Local labels from 00 to 99 and from A to l
        return step;
      }
      else if(opParam == STRING_LABEL_VARIABLE) {
        return step + *step + 1;
      }
      else {
        #if !defined(DMCP_BUILD)
          printf("\nIn function countOpBytes case PARAM_DECLARE_LABEL: opParam %u is not a valid label!\n", opParam);
        #endif // !DMCP_BUILD
        return NULL;
      }
    }

    case PARAM_LABEL: {
      if(opParam <= LAST_LOCAL_LABEL) { // Local labels from 00 to 99 and from A to l
        return step;
      }
      else if(opParam == STRING_LABEL_VARIABLE || opParam == INDIRECT_VARIABLE) {
        return step + *step + 1;
      }
      else if(opParam == INDIRECT_REGISTER) {
        return step + 1;
      }
      else {
        #if !defined(DMCP_BUILD)
          printf("\nIn function countOpBytes: case PARAM_LABEL, %u is not a valid parameter!", opParam);
        #endif // !DMCP_BUILD
        return NULL;
      }
    }

    case PARAM_REGISTER: {
      if(opParam <= LAST_SPARE_REGISTERS_IN_KS_CODE) { // Global registers from 00 to 99, lettered registers from X to W, and local registers from .00 to .98
        return step;
      }
      else if(opParam == STRING_LABEL_VARIABLE || opParam == INDIRECT_VARIABLE) {
        return step + *step + 1;
      }
      else if(opParam == INDIRECT_REGISTER) {
        return step + 1;
      }
      else {
        #if !defined(DMCP_BUILD)
          printf("\nIn function countOpBytes: case PARAM_REGISTER, %u is not a valid parameter!", opParam);
        #endif // !DMCP_BUILD
        return NULL;
      }
    }

    case PARAM_FLAG: {
      if(opParam <= LAST_LOCAL_FLAG) { // Global flags from 00 to 99, lettered flags from X to K, and local flags from .00 to .31
        return step;
      }
      else if(FLAG_M <= opParam && opParam <= FLAG_W) { // Global flags from M to W
        return step;
      }
      else if(opParam == INDIRECT_REGISTER || opParam == SYSTEM_FLAG_NUMBER) {
        return step + 1;
      }
      else if(opParam == INDIRECT_VARIABLE) {
        return step + *step + 1;
      }
      else {
        #if !defined(DMCP_BUILD)
          printf("\nIn function countOpBytes: case PARAM_FLAG, %u is not a valid parameter!", opParam);
        #endif // !DMCP_BUILD
        return NULL;
      }
    }

    case PARAM_NUMBER_8: {
      if(opParam <= 249) { // Value from 0 to 99
        return step;
      }
      else if(opParam == INDIRECT_REGISTER) {
        return step + 1;
      }
      else if(opParam == INDIRECT_VARIABLE) {
        return step + *step + 1;
      }
      else {
        #if !defined(DMCP_BUILD)
          printf("\nIn function countOpBytes: case PARAM_NUMBER, %u is not a valid parameter!", opParam);
        #endif // !DMCP_BUILD
        return NULL;
      }
    }

    case PARAM_NUMBER_8_16: {
      if(opParam <= 249) { // Value from 0 to 249
        return step;
      }
      else if(opParam == CNST_BEYOND_250) { // Value from 250 to 499
        return step + 1;
      }
      else if(opParam == INDIRECT_REGISTER) {
        return step + 1;
      }
      else if(opParam == INDIRECT_VARIABLE) {
        return step + *step + 1;
      }
      else {
        #if !defined(DMCP_BUILD)
          printf("\nIn function countOpBytes: case PARAM_NUMBER, %u is not a valid parameter!", opParam);
        #endif // !DMCP_BUILD
        return NULL;
      }
    }

    case PARAM_NUMBER_16: {
      uint16_t func = (*(step-3) << 8) + *(step -2);
      func &= 0x7fff;
      if(isFunctionOldParam16(func)) {  // original Param16 functions without indirection support (little endian parameter)
        return step + 1;
      }
      else {                        // new Param1- functions with indirection support (big endian parameter)
        if(opParam == INDIRECT_REGISTER) {
          return step + 1;
        }
        else if(opParam == INDIRECT_VARIABLE) {
          return step + *step + 1;
        }
        else {
          return step + 1;
        }
      }
    }

    case PARAM_COMPARE: {
      if(opParam <= LAST_SPARE_REGISTERS_IN_KS_CODE || opParam == VALUE_0 || opParam == VALUE_1) { // Global registers from 00 to 99, lettered registers from X to W, and local registers from .00 to .98 OR value 0 OR value 1
        return step;
      }
      else if(opParam == STRING_LABEL_VARIABLE || opParam == INDIRECT_VARIABLE) {
        return step + *step + 1;
      }
      else if(opParam == INDIRECT_REGISTER) {
        return step + 1;
      }
      else {
        #if !defined(DMCP_BUILD)
          printf("\nIn function countOpBytes: case PARAM_COMPARE, %u is not a valid parameter!", opParam);
        #endif // !DMCP_BUILD
        return NULL;
      }
    }

    case PARAM_SKIP_BACK:
    case PARAM_SHUFFLE: {
      return step;
    }

    case PARAM_MENU: {
      if(opParam == STRING_LABEL_VARIABLE || opParam == INDIRECT_VARIABLE) {
        return step + *step + 1;
      }
      else if(opParam == INDIRECT_REGISTER) {
        return step + 1;
      }
      else {
        #if !defined(DMCP_BUILD)
          printf("\nIn function countOpBytes: case PARAM_MENU, %u is not a valid parameter!", opParam);
        #endif // !DMCP_BUILD
        return NULL;
      }
    }

    default: {
      #if !defined(DMCP_BUILD)
        printf("\nIn function countOpBytes: paramMode %u is not valid!\n", paramMode);
      #endif // !DMCP_BUILD
      return NULL;
    }
  }
}


uint8_t *countLiteralBytes(uint8_t *step) {
  switch(*(step++)) {
    case BINARY_SHORT_INTEGER: {
      return step + 9;
    }

    //case BINARY_LONG_INTEGER: {
    //  break;
    //}

    case BINARY_REAL34: {
      return step + REAL34_SIZE_IN_BYTES;
    }

    case BINARY_COMPLEX34: {
      return step + TO_BYTES(REAL34_SIZE_IN_BLOCKS * 2);
    }

    //case BINARY_DATE: {
    //  break;
    //}

    //case BINARY_TIME: {
    //  break;
    //}

    case STRING_SHORT_INTEGER: {
      return step + *(step + 1) + 2;
    }

    case STRING_LONG_INTEGER:
    case STRING_REAL34:
    case STRING_LABEL_VARIABLE:
    case STRING_COMPLEX34:
    case STRING_DATE:
    case STRING_TIME:
    case STRING_ANGLE_DMS:
    case STRING_ANGLE_RADIAN:
    case STRING_ANGLE_GRAD:
    case STRING_ANGLE_DEGREE:
    case STRING_ANGLE_MULTPI:

    {
      return step + *step + 1;
    }

    default: {
      #if !defined(DMCP_BUILD)
        printf("\nERROR: in countLiteralBytes() %u is not an acceptable parameter for ITM_LITERAL!\n", *(step - 1));
        printf("At address ram + %" PRIu32 "\n", (uint32_t)((step - 1) - (uint8_t *)ram));
      #endif // !DMCP_BUILD
      return NULL;
    }
  }
}



uint8_t *findNextStep(uint8_t *step) {
  if(checkOpCodeOfStep(step, ITM_KEY)) {
    return findKey2ndParam(findKey2ndParam(step));
  }
  else {
    return findKey2ndParam(step);
  }
}



uint8_t *findKey2ndParam(uint8_t *step) {
  uint16_t op = *(step++);
  if(op & 0x80) {
    op &= 0x7f;
    op <<= 8;
    op |= *(step++);
  }

  if(op == 0x7fff) { // .END.
    return NULL;
  }
  else {
    switch(indexOfItems[op].status & PTP_STATUS) {
      case PTP_NONE:
      case PTP_DISABLED: {
        return step;
      }

      case PTP_LITERAL:
      case PTP_REM: {
        return countLiteralBytes(step);
      }

      case PTP_KEYG_KEYX: {
        return countOpBytes(step, PARAM_NUMBER_8);
      }

      default: {
        return countOpBytes(step, (indexOfItems[op].status & PTP_STATUS) >> 9);
      }
    }
  }
}



uint8_t *findPreviousStep(uint8_t *step) {
  uint8_t *searchFromStep, *nextStep;
  searchFromStep = NULL;

  if(step == beginOfProgramMemory) {
    return step;
  }

  if(numberOfLabels == 0 || step <= labelList[0].instructionPointer) {
    searchFromStep = beginOfProgramMemory;
  }
  else {
    for(int16_t label=numberOfLabels - 1; label >= 0; label--) {
      if(labelList[label].instructionPointer < step) {
        searchFromStep = labelList[label].instructionPointer;
        break;
      }
    }
  }

  nextStep = findNextStep(searchFromStep);
  while(nextStep != step) {
    searchFromStep = nextStep;
    nextStep = findNextStep(searchFromStep);
  }

  return searchFromStep;
}



static void _showStep(void) {
  #if !defined(TESTSUITE_BUILD)
    bool_t lblOrEnd;
    uint8_t *tmpStep;

    tmpStep = currentStep;
    lblOrEnd = checkOpCodeOfStep(tmpStep, ITM_LBL) || isAtEndOfProgram(tmpStep) || isAtEndOfPrograms(tmpStep);
    int16_t xPos = (lblOrEnd ? 42 : 62);
    int16_t maxWidth = SCREEN_WIDTH - xPos;

//    lcd_fill_rect(1, Y_POSITION_OF_REGISTER_T_LINE, xPos+1,  REGISTER_LINE_HEIGHT, LCD_SET_VALUE);
    sprintf(tmpString, "%04" PRIu16 ":" STD_SPACE_4_PER_EM, currentLocalStepNumber);
    showString(tmpString, &standardFont, 1, Y_POSITION_OF_REGISTER_T_LINE + 6, vmNormal, true, true);

    decodeOneStep(tmpStep);
    if(stringWidth(tmpString, &standardFont, true, true) >= maxWidth) {
      char *xstr = tmpString;
      char *xstrOrig = tmpString;
      char *glyph = tmpString + TMP_STR_LENGTH - 4;
      maxWidth -= stringWidth(STD_ELLIPSIS, &standardFont, true, true);
      while(maxWidth > 0) {
        xstrOrig = xstr;
        glyph[0] = *(xstr++);
        if(glyph[0] & 0x80) {
          glyph[1] = *(xstr++);
          glyph[2] = 0;
        }
        else {
          glyph[1] = 0;
        }
        maxWidth -= stringWidth(glyph, &standardFont, true, true);
      }
      xstrOrig[0] = STD_ELLIPSIS[0];
      xstrOrig[1] = STD_ELLIPSIS[1];
      xstrOrig[2] = 0;
    }
//    lcd_fill_rect(xPos, Y_POSITION_OF_REGISTER_T_LINE, stringWidth(tmpString, &standardFont, true, true)+20,  REGISTER_LINE_HEIGHT, LCD_SET_VALUE);
    showString(tmpString, &standardFont, xPos, Y_POSITION_OF_REGISTER_T_LINE + 6, vmNormal, true, true);
  #endif // !TESTSUITE_BUILD
}



static void _bstInPem(void) {
  //  - currentProgramNumber
  //  - currentLocalStepNumber
  //  - firstDisplayedLocalStepNumber
  //  - firstDisplayedStep
  if(currentLocalStepNumber > 1) {
    if(firstDisplayedLocalStepNumber > 0 && currentLocalStepNumber <= firstDisplayedLocalStepNumber + 3) {
      --firstDisplayedLocalStepNumber;
    }
    currentLocalStepNumber--;
  }
  else if(currentLocalStepNumber == 1 && !pemCursorIsZerothStep) {
    currentLocalStepNumber = 1;
    firstDisplayedLocalStepNumber = 0;
    pemCursorIsZerothStep = true;
  }
  else {
    uint16_t numberOfSteps = getNumberOfSteps();
    currentLocalStepNumber = numberOfSteps;
    pemCursorIsZerothStep = false;
    if(numberOfSteps <= 6) {
      firstDisplayedLocalStepNumber = 0;
    }
    else {
      firstDisplayedLocalStepNumber = numberOfSteps - 6;
    }
  }
  defineFirstDisplayedStep();
}

void fnBst(uint16_t unusedButMandatoryParameter) {
  screenUpdatingMode = SCRUPD_AUTO;
  if(calcMode == CM_PEM) {
    if(aimBuffer[0] != 0) {
      if(getSystemFlag(FLAG_ALPHA)) {
        pemCloseAlphaInput();
      }
      else if(nimNumberPart == NP_INT_BASE) {
        return;
      }
      else {
        pemCloseNumberInput();
      }
      aimBuffer[0] = 0;
      --currentLocalStepNumber;
      currentStep = findPreviousStep(currentStep);
      if(!programListEnd) {
        scrollPemBackwards();
      }
    }
  }
  currentInputVariable = INVALID_VARIABLE;
  _bstInPem();
  if(calcMode != CM_PEM) {
    defineCurrentStep();
    _showStep();
  }
}



static void _sstInPem(void) {
  uint16_t numberOfSteps = getNumberOfSteps();

  if(currentLocalStepNumber == 1 && pemCursorIsZerothStep) {
    currentLocalStepNumber = 1;
    firstDisplayedLocalStepNumber = 0;
    pemCursorIsZerothStep = false;
  }
  else if(currentLocalStepNumber < numberOfSteps) {
    if(currentLocalStepNumber++ >= 3) {
      if(!programListEnd) {
        ++firstDisplayedLocalStepNumber;
      }
    }

    if(firstDisplayedLocalStepNumber + 7 > numberOfSteps) {
      if(numberOfSteps <= 6) {
        firstDisplayedLocalStepNumber = 0;
      }
      else {
        firstDisplayedLocalStepNumber = numberOfSteps - 6;
      }
    }
  }
  else {
    currentLocalStepNumber = 1;
    firstDisplayedLocalStepNumber = 0;
    pemCursorIsZerothStep = true;
  }

  defineFirstDisplayedStep();
}

void fnSst(uint16_t unusedButMandatoryParameter) {
  screenUpdatingMode = SCRUPD_AUTO;
  if(calcMode == CM_PEM) {
    if(aimBuffer[0] != 0) {
      if(getSystemFlag(FLAG_ALPHA)) {
        pemCloseAlphaInput();
      }
      else if(nimNumberPart == NP_INT_BASE) {
        return;
      }
      else {
        pemCloseNumberInput();
      }
      aimBuffer[0] = 0;
      --currentLocalStepNumber;
      currentStep = findPreviousStep(currentStep);
      if(!programListEnd) {
        scrollPemBackwards();
      }
    }
    currentInputVariable = INVALID_VARIABLE;
    _sstInPem();
  }
  else {
    temporaryInformation = TI_NO_INFO;
    refreshRegisterLine(REGISTER_T);     // Clear previous VIEW or AVIEW data, if any
    refreshRegisterLine(REGISTER_Z);     // Clear previous test result, if any
    _showStep();
    if(currentInputVariable != INVALID_VARIABLE) {
      if(currentInputVariable & 0x8000) {
        fnDropY(NOPARAM);
      }
      fnStore(currentInputVariable & 0x3fff);
      currentInputVariable = INVALID_VARIABLE;
    }
    dynamicMenuItem = -1;
    programRunStop = PGM_SINGLE_STEP;
    //runProgram(true, INVALID_VARIABLE); // [DL] Not executed here, delayed until SST key released
  }
}



void fnBack(uint16_t numberOfSteps) {
  if(numberOfSteps >= (currentLocalStepNumber - 1)) {
    currentLocalStepNumber = 1;
    currentStep = programList[currentProgramNumber - 1].instructionPointer;
  }
  else {
    currentLocalStepNumber -= numberOfSteps;
    defineCurrentStep();
  }
}



void fnSkip(uint16_t numberOfSteps) {
  for(uint16_t i = 0; i <= numberOfSteps; ++i) { // '<=' is intended here because the pointer must be moved at least by 1 step
    uint8_t *tmpStep;
    tmpStep = currentStep;
    if(!isAtEndOfProgram(tmpStep) && !isAtEndOfPrograms(tmpStep)) {
      ++currentLocalStepNumber;
      currentStep = findNextStep(currentStep);
    }
    else {
      break;
    }
  }
}



void fnCase(uint16_t regist) {
  real34_t arg;

  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
  switch(getRegisterDataType(regist)) {
    case dtLongInteger: {
      convertLongIntegerRegisterToReal34(regist, &arg);
      break;
    }
    case dtReal34: {
      if(getRegisterAngularMode(regist) == amNone) {
        real34ToIntegralValue(REGISTER_REAL34_DATA(regist), &arg, DEC_ROUND_DOWN);
        break;
      }
      /* fallthrough */
    }
    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "cannot use %s for the parameter of CASE", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function fnCase:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  #pragma GCC diagnostic pop

  if(real34CompareLessThan(&arg, const34_1)) {
    fnSkip(0);
  }
  else if(real34CompareGreaterEqual(&arg, const34_65535)) {
    fnSkip(65534u);
  }
  else {
    fnSkip(real34ToUInt32(&arg) - 1);
  }
}




void defineCurrentStep(void) {
  currentStep = programList[currentProgramNumber - 1].instructionPointer;
  for(uint16_t i = 1; i < currentLocalStepNumber; ++i) {
    currentStep = findNextStep(currentStep);
  }
}



void defineFirstDisplayedStep(void) {
  firstDisplayedStep = programList[currentProgramNumber - 1].instructionPointer;
  for(uint16_t i = 1; i < firstDisplayedLocalStepNumber; ++i) {
    firstDisplayedStep = findNextStep(firstDisplayedStep);
  }
}
