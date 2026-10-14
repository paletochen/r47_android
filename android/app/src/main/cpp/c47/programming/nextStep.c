// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors


/********************************************//**
 * \file nextStep.c
 ***********************************************/

#include "c47.h"


bool_t programBytesAvailable(const uint8_t *address, uint16_t numberOfBytes) {
  uintptr_t begin = (uintptr_t)beginOfProgramMemory;
  uintptr_t end = (uintptr_t)firstFreeProgramByte;
  uintptr_t current = (uintptr_t)address;

  return current >= begin && current <= end && numberOfBytes <= end - current;
}


// Parameter-tail grammar shared by the in-memory step walkers below and the program-file screening pass in saveRestorePrograms.c: given the parameter mode,
// the operation, needed for PARAM_NUMBER_16 only, and the first parameter byte, the tail is a fixed byte count, a length byte and that many bytes, or invalid.
int16_t paramTailBytes(uint16_t paramMode, uint16_t op, uint8_t opParam) {
  switch(paramMode) {
    case PARAM_DECLARE_LABEL: {
      if(opParam <= LAST_LOCAL_LABEL)                                                  return 0;
      if(opParam == STRING_LABEL_VARIABLE || opParam == LOCAL_LABEL_VARIABLE)          return PARAM_TAIL_LENGTH_PREFIXED;
      return PARAM_TAIL_INVALID;
    }
    case PARAM_LABEL: {
      if(opParam <= LAST_LOCAL_LABEL)                                                  return 0;
      if(opParam == STRING_LABEL_VARIABLE || opParam == INDIRECT_VARIABLE || opParam == LOCAL_LABEL_VARIABLE) return PARAM_TAIL_LENGTH_PREFIXED;
      if(opParam == INDIRECT_REGISTER)                                                 return 1;
      return PARAM_TAIL_INVALID;
    }
    case PARAM_REGISTER: {
      if(opParam <= LAST_SPARE_REGISTERS_IN_KS_CODE)                                   return 0;
      if(opParam == STRING_LABEL_VARIABLE || opParam == INDIRECT_VARIABLE)             return PARAM_TAIL_LENGTH_PREFIXED;
      if(opParam == INDIRECT_REGISTER)                                                 return 1;
      return PARAM_TAIL_INVALID;
    }
    case PARAM_COMPARE: {
      if(opParam <= LAST_SPARE_REGISTERS_IN_KS_CODE || opParam == VALUE_0 || opParam == VALUE_1) return 0;
      if(opParam == STRING_LABEL_VARIABLE || opParam == INDIRECT_VARIABLE)             return PARAM_TAIL_LENGTH_PREFIXED;
      if(opParam == INDIRECT_REGISTER)                                                 return 1;
      return PARAM_TAIL_INVALID;
    }
    case PARAM_FLAG: {
      if(opParam <= LAST_LOCAL_FLAG || (FLAG_M <= opParam && opParam <= FLAG_W))       return 0;
      if(opParam == INDIRECT_REGISTER || opParam == SYSTEM_FLAG_NUMBER)                return 1;
      if(opParam == INDIRECT_VARIABLE)                                                 return PARAM_TAIL_LENGTH_PREFIXED;
      return PARAM_TAIL_INVALID;
    }
    case PARAM_NUMBER_8: {
      if(opParam <= 249)                                                               return 0;
      if(opParam == INDIRECT_REGISTER)                                                 return 1;
      if(opParam == INDIRECT_VARIABLE)                                                 return PARAM_TAIL_LENGTH_PREFIXED;
      return PARAM_TAIL_INVALID;
    }
    case PARAM_NUMBER_8_16: {
      if(opParam <= 249)                                                               return 0;
      if(opParam == CNST_BEYOND_250 || opParam == INDIRECT_REGISTER)                   return 1;
      if(opParam == INDIRECT_VARIABLE)                                                 return PARAM_TAIL_LENGTH_PREFIXED;
      return PARAM_TAIL_INVALID;
    }
    case PARAM_NUMBER_16: {
      // original Param16 functions have no indirection support (little endian parameter)
      if(isFunctionOldParam16(op))                                                     return 1;
      if(opParam == INDIRECT_VARIABLE)                                                 return PARAM_TAIL_LENGTH_PREFIXED;
      return 1; // new Param16 form (big endian parameter), including INDIRECT_REGISTER
    }
    case PARAM_SKIP_BACK:
    case PARAM_SHUFFLE: {
      return 0;
    }
    case PARAM_MENU: {
      if(opParam == STRING_LABEL_VARIABLE || opParam == INDIRECT_VARIABLE)             return PARAM_TAIL_LENGTH_PREFIXED;
      if(opParam == INDIRECT_REGISTER)                                                 return 1;
      return PARAM_TAIL_INVALID;
    }
    default: {
      return PARAM_TAIL_INVALID;
    }
  }
}


// Literal-tail grammar, same contract, for the type byte following ITM_LITERAL.
int16_t literalTailBytes(uint8_t literalType) {
  switch(literalType) {
    case BINARY_SHORT_INTEGER:  return 9;
    case BINARY_REAL34:         return REAL34_SIZE_IN_BYTES;
    case BINARY_COMPLEX34:      return TO_BYTES(REAL34_SIZE_IN_BLOCKS * 2);
    case STRING_SHORT_INTEGER:  return PARAM_TAIL_BASE_LENGTH_PREFIXED; // base byte, then a length-prefixed string
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
    case STRING_ANGLE_MULTPI:   return PARAM_TAIL_LENGTH_PREFIXED;
    default:                    return PARAM_TAIL_INVALID;
  }
}


uint8_t *countOpBytes(uint8_t *step, uint16_t paramMode) {
  uint8_t opParam = *(step++);
  uint16_t op = 0;
  if(paramMode == PARAM_NUMBER_16) { // only PARAM_NUMBER_16 needs the operation, read back as before
    op = (((uint16_t)(*(step - 3)) << 8) + *(step - 2)) & 0x7fff;
  }
  int16_t tail = paramTailBytes(paramMode, op, opParam);
  if(tail == PARAM_TAIL_INVALID) {
    #if !defined(DMCP_BUILD)
      printf("\nIn function countOpBytes: paramMode %u, %u is not a valid parameter!\n", paramMode, opParam);
    #endif // !DMCP_BUILD
    return NULL;
  }
  if(tail == PARAM_TAIL_LENGTH_PREFIXED) {
    return step + *step + 1;
  }
  return step + tail;
}


uint8_t *countLiteralBytes(uint8_t *step) {
  int16_t tail = literalTailBytes(*step);
  step++;
  if(tail == PARAM_TAIL_INVALID) {
    #if !defined(DMCP_BUILD)
      printf("\nERROR: in countLiteralBytes() %u is not an acceptable parameter for ITM_LITERAL!\n", *(step - 1));
      printf("At address ram + %" PRIu32 "\n", (uint32_t)((step - 1) - (uint8_t *)ram));
    #endif // !DMCP_BUILD
    return NULL;
  }
  if(tail == PARAM_TAIL_BASE_LENGTH_PREFIXED) {
    return step + *(step + 1) + 2;
  }
  if(tail == PARAM_TAIL_LENGTH_PREFIXED) {
    return step + *step + 1;
  }
  return step + tail;
}


uint8_t *findNextStep(uint8_t *step) {
  if(step == NULL) {
    #if !defined(DMCP_BUILD)
      printf("\nIn function findNextStep: NULL step pointer!\n");
    #endif // !DMCP_BUILD
    return NULL;
  }
  if((checkOpCodeOfStep(step, ITM_KEY)) || (checkOpCodeOfStep(step, ITM_42KEY))) {
    uint8_t *afterFirst = findKey2ndParam(step);
    if(afterFirst == NULL) {
      return NULL;
    }
    return findKey2ndParam(afterFirst);
  }
  else {
    return findKey2ndParam(step);
  }
}



uint8_t *findKey2ndParam(uint8_t *step) {
  if(step == NULL) {
    #if !defined(DMCP_BUILD)
      printf("\nIn function findKey2ndParam: NULL step pointer!\n");
    #endif // !DMCP_BUILD
    return NULL;
  }
  if(!programBytesAvailable(step, 1)) {
    return NULL;
  }
  uint16_t op = *(step++);
  if(op & 0x80) {
    if(!programBytesAvailable(step, 1)) {
      return NULL;
    }
    op &= 0x7f;
    op <<= 8;
    op |= *(step++);
  }

  uint8_t *secondParam;
  if(op >= LAST_ITEM) { // do not index past the item table; 0x7fff (.END.) lands here too
    return NULL;
  }
  else {
    switch(indexOfItems[op].status & PTP_STATUS) {
      case PTP_NONE:
      case PTP_DISABLED: {
        secondParam = step;
        break;
      }

      case PTP_LITERAL:
      case PTP_REM: {
        secondParam = countLiteralBytes(step);
        break;
      }

      case PTP_KEYG_KEYX: {
        secondParam = countOpBytes(step, PARAM_NUMBER_8);
        break;
      }

      default: {
        secondParam = countOpBytes(step, (indexOfItems[op].status & PTP_STATUS) >> 9);
        break;
      }
    }
  }

  // countLiteralBytes()/countOpBytes() advance by lengths taken from the step
  // itself, so corrupt or imported program memory can push the result outside
  // the active program region. Reject any pointer that leaves it: the next byte
  // must be at or before firstFreeProgramByte (equal is the valid end position).
  if(secondParam != NULL && !programBytesAvailable(secondParam, 0)) {
    return NULL;
  }
  return secondParam;
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
  while(nextStep != NULL && nextStep != step) {
    searchFromStep = nextStep;
    nextStep = findNextStep(searchFromStep);
  }
  #if !defined(DMCP_BUILD)
    if(nextStep == NULL) {
      printf("\nIn function findPreviousStep: passed target without finding it!\n");
    }
  #endif // !DMCP_BUILD

  return searchFromStep;
}



static void _showStep(void) {
    char stepNumber[10];

    sprintf(stepNumber, "%04" PRIu16 ":" STD_SPACE_EM, currentLocalStepNumber);
    decodeOneStep(currentStep);
    xcopy(tmpString + stringByteLength(stepNumber), tmpString, stringByteLength(tmpString) + 1);
    xcopy(tmpString, stepNumber, stringByteLength(stepNumber));
    drawFuncName(tmpString);
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
    if(structStepIsIfOrWhile(currentStep) && currentLocalStepNumber > 1) { // a test and its IF or WHILE run as one action, so stepping back goes over the pair
      currentLocalStepNumber--;
      defineCurrentStep();
    }
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


void showStep(void) {
  temporaryInformation = TI_NO_INFO;
  refreshRegisterLine(REGISTER_Z);     // Clear previous test result, if any
  _showStep();
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
    showStep();
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
      uint8_t *next = findNextStep(currentStep);
      if(next == NULL) {
        #if !defined(DMCP_BUILD)
          printf("\nIn function fnSkip: findNextStep returned NULL!\n");
        #endif // !DMCP_BUILD
        break;
      }
      ++currentLocalStepNumber;
      currentStep = next;
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
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
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
    uint8_t *next = findNextStep(currentStep);
    if(next == NULL) {
      #if !defined(DMCP_BUILD)
        printf("\nIn function defineCurrentStep: findNextStep returned NULL at step %u!\n", i);
      #endif // !DMCP_BUILD
      break;
    }
    currentStep = next;
  }
}



void defineFirstDisplayedStep(void) {
  firstDisplayedStep = programList[currentProgramNumber - 1].instructionPointer;
  for(uint16_t i = 1; i < firstDisplayedLocalStepNumber; ++i) {
    uint8_t *next = findNextStep(firstDisplayedStep);
    if(next == NULL) {
      #if !defined(DMCP_BUILD)
        printf("\nIn function defineFirstDisplayedStep: findNextStep returned NULL at step %u!\n", i);
      #endif // !DMCP_BUILD
      break;
    }
    firstDisplayedStep = next;
  }
}
