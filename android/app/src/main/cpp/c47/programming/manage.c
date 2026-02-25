// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file manage.c
 ***********************************************/

#include "c47.h"

#if defined(PC_BUILD) && defined(DEBUG_PGM)
  #include <execinfo.h>
#endif //PC_BUILD

// Structure of the program memory.
// In this example the RAM is 16384 blocks (from 0 to 16383) of 4 bytes = 65536 bytes.
// The program memory occupies the end of the RAM area.
//
//  +-----+------+----+----------+
//  |Block| Step |Code|    OP    |
//  |     |loc/gl|    |          |
//  +-----+------+----+----------+
//  |16374| 1/ 1 |   1| LBL 'P1' | <-- beginOfProgramMemory
//  |     |      | 253|          |
//  |     |      |   2|          |
//  |     |      | 'P'|          |
//  |16375|      | '1'|          |  ^
//  |     | 2/ 2 | 114| 1.       |  | 1 block = 4 bytes
//  |     |      |   6|          |  |
//  |     |      |   1|          |  v
//  |16376|      | '1'|          |
//  |     | 3/ 3 |  95| +        |
//  |     | 4/ 4 | 133| END      |
//  |     |      | 168|          |
//  |16377| 1/ 5 |   1| LBL 'P2' | <-- beginOfCurrentProgram  <-- firstDisplayedStep   firstDisplayedStepNumber = 5   firstDisplayedLocalStepNumber = 1
//  |     |      | 253|          |
//  |     |      |   2|          |
//  |     |      | 'P'|          |
//  |16378|      | '2'|          |
//  |     | 2/ 6 | 114| 22.      | <-- currentStep       this inequality is always true: beginOfCurrentProgram ≤ currentStep < endOfCurrentProgram
//  |     |      |   6|          |     currentStepNumber = 6   currentLocalStepNumber = 2
//  |     |      |   2|          |     currentProgramNumber = 2
//  |16379|      | '2'|          |
//  |     |      | '2'|          |
//  |     | 3/ 7 |  95| +        |
//  |     | 4/ 8 | 133| END      |
//  |16380|      | 168|          |
//  |     | 1/ 9 |   2| LBL 'P3' | <-- endOfCurrentProgram
//  |     |      | 253|          |
//  |     |      |   2|          |
//  |16381|      | 'P'|          |
//  |     |      | '3'|          |
//  |     | 2/10 | 114| 3.       |
//  |     |      |   6|          |
//  |16382|      |   1|          |
//  |     |      | '3'|          |
//  |     | 3/11 |  95| +        |
//  |     | 4/12 | 133| END      |
//  |16383| 5/13 | 168|          |
//  |     |      | 255| .END.    | <-- firstFreeProgramByte
//  |     |      | 255|          |
//  |     |      |   ?|          | free byte     This byte is the end of the RAM area
//  +-----+------+----+----------+
//
//  freeProgramBytes = 1

static void _pemCloseAngleInput(int item);
static void _pemCloseDateInput(void);
static void _pemCloseTimeInput(void);

bool_t isAtEndOfPrograms(const uint8_t *step) {
  return (step == NULL) || (*step == 255 && *(step + 1) == 255);
}



bool_t checkOpCodeOfStep(const uint8_t *step, uint16_t op) {
  if(op < 128) {
    return step && *step == op;
  }
  else {
    return step && (*step & 0x7f) == (op >> 8) && *(step + 1) == (op & 0xff);
  }
}



void scanLabelsAndPrograms(void) {
#if !defined(SAVE_SPACE_DM42_10)
  uint32_t stepNumber = 0;
  uint8_t *nextStep, *step = beginOfProgramMemory;

  freeC47Blocks(labelList, TO_BLOCKS(sizeof(labelList_t)) * numberOfLabels);
  freeC47Blocks(programList, TO_BLOCKS(sizeof(programList_t)) * numberOfPrograms);

  numberOfLabels = 0;
  numberOfPrograms = 1;
  while(!isAtEndOfPrograms(step)) { // .END.
    if(*step == ITM_LBL) { // LBL
      numberOfLabels++;
    }
    if(isAtEndOfProgram(step)) { // END
      nextStep = findNextStep(step);
      if(!isAtEndOfPrograms(nextStep)) { // .END. following END is not the start of a new program
        numberOfPrograms++;
      }
    }
    step = findNextStep(step);
  }

  labelList = allocC47Blocks(TO_BLOCKS(sizeof(labelList_t)) * numberOfLabels);
  if(labelList == NULL) {
    // unlikely
    lastErrorCode = ERROR_RAM_FULL;
    return;
  }

  programList = allocC47Blocks(TO_BLOCKS(sizeof(programList_t)) * numberOfPrograms);
  if(programList == NULL) {
    // unlikely
    lastErrorCode = ERROR_RAM_FULL;
    return;
  }

  numberOfLabels = 0;
  step = beginOfProgramMemory;
  programList[0].instructionPointer = step;
  programList[0].step = (0 + 1);
  numberOfPrograms = 1;
  stepNumber = 1;
  while(!isAtEndOfPrograms(step)) { // .END.
    nextStep = findNextStep(step);
    if(checkOpCodeOfStep(step, ITM_LBL)) { // LBL
      labelList[numberOfLabels].program = numberOfPrograms;
      if(*(step + 1) <= LAST_LOCAL_LABEL) { // Local label
        labelList[numberOfLabels].step = -stepNumber;
        labelList[numberOfLabels].labelPointer = step + 1;
      }
      else { // Global label
        labelList[numberOfLabels].step = stepNumber;
        labelList[numberOfLabels].labelPointer = step + 2;
      }

      labelList[numberOfLabels].instructionPointer = nextStep;
      numberOfLabels++;
    }

    if(isAtEndOfProgram(step)) { // END
      if(!isAtEndOfPrograms(nextStep)) { // .END. following END is not the start of a new program
        programList[numberOfPrograms].instructionPointer = step + 2;
        programList[numberOfPrograms].step = stepNumber + 1;
        numberOfPrograms++;
      }
    }

    step = nextStep;
    stepNumber++;
  }

  //The folowing 2 lines added to address the FFFFFFFF issue in old state files
  firstFreeProgramByte = step;
  freeProgramBytes = (((uint8_t *)(ram + RAM_SIZE_IN_BLOCKS)) - firstFreeProgramByte) - 2;

  defineCurrentProgramFromCurrentStep();
  defineFirstDisplayedStep();
#endif // !SAVE_SPACE_DM42_10
}



void deleteStepsFromTo(uint8_t *from, uint8_t *to) {
  uint16_t opSize = to - from;

  xcopy(from, to, (firstFreeProgramByte - to) + 2);
  firstFreeProgramByte -= opSize;
  freeProgramBytes += opSize;
  scanLabelsAndPrograms();
}



static void _removeLabelsAssignments() {
  int16_t i;
  char label[15];
  uint8_t labelLength;
  for (i=0; i<numberOfLabels; i++) {
    if((labelList[i].program == currentProgramNumber) && (labelList[i].step > 0)) {
      labelLength = labelList[i].labelPointer[0];
      xcopy(label, labelList[i].labelPointer + 1, labelList[i].labelPointer[0]);
      label[labelLength]=0;
      removeUserItemAssignments(ITM_XEQ,label);   // Remove label assignments
    }
  }
}



void fnClPAll(uint16_t confirmation) {
  if(confirmation == NOT_CONFIRMED) {
    setConfirmationMode(fnClPAll);
  }
  else {
    // Remove assignments of all global labels, before deleting all programs
    removeUserItemAssignments(ITM_XEQ,"");   // Remove all labels assignments

    bool_t wasInRam = (programList[currentProgramNumber - 1].step > 0);
    resizeProgramMemory(1); // 1 block for an empty program
    *(beginOfProgramMemory + 0)   = (ITM_END >> 8) | 0x80;
    *(beginOfProgramMemory + 1)   =  ITM_END       & 0xff;
    *(beginOfProgramMemory + 2)   = 255; // .END.
    *(beginOfProgramMemory + 3)   = 255; // .END.
    firstFreeProgramByte          = beginOfProgramMemory + 2;
    freeProgramBytes              = 0;
    temporaryInformation          = TI_NO_INFO;
    programRunStop                = PGM_STOPPED;

    if(wasInRam) { // Not in flash
      currentStep                   = beginOfProgramMemory;
      firstDisplayedStep            = beginOfProgramMemory;
      firstDisplayedLocalStepNumber = 0;
      currentLocalStepNumber        = 1;
      beginOfCurrentProgram         = beginOfProgramMemory;
      endOfCurrentProgram           = firstFreeProgramByte;
    }

    scanLabelsAndPrograms();
    if(programRunStop != PGM_RUNNING) {
      temporaryInformation = TI_DEL_ALL_PRGMS;
    }
    else {
      temporaryInformation = TI_NO_INFO;
    }
    screenUpdatingMode = SCRUPD_AUTO;
  }
}



static int _clearProgram(void) {
  if(beginOfCurrentProgram == beginOfProgramMemory && (endOfCurrentProgram >= firstFreeProgramByte || (*endOfCurrentProgram == 255 && *(endOfCurrentProgram + 1) == 255))) { // There is only one program in memory
    fnClPAll(CONFIRMED);
    return 1;
  }
  else {
    // Remove assignments of global labels in the program being deleted, before deleting the program
    _removeLabelsAssignments();

    uint16_t savedCurrentProgramNumber = currentProgramNumber;

    goToPgmStep(currentProgramNumber, 1);  // [DL] work around for crash when label deleted is not at the beginning of the program
    firstDisplayedLocalStepNumber = 0;     // ditto

    deleteStepsFromTo(beginOfCurrentProgram, endOfCurrentProgram - ((currentProgramNumber == numberOfPrograms) ? 2 : 0));
    scanLabelsAndPrograms();
    // unlikely fails

    if(savedCurrentProgramNumber >= numberOfPrograms) { // The last program
      goToPgmStep(numberOfPrograms, 1);
    }
    else { // Not the last program
      goToPgmStep(savedCurrentProgramNumber, 1);
    }
    return 0;
  }
}



void fnClP(uint16_t label) {
  uint16_t savedCurrentLocalStepNumber = currentLocalStepNumber;
  uint16_t savedCurrentProgramNumber = currentProgramNumber;

  while(currentSubroutineLevel > 0) { // drop subroutine stack before deleting a program
    fnReturn(0);
  }
  fnReturn(0); // 1 more time to clean local registers

  goToPgmStep(savedCurrentProgramNumber, savedCurrentLocalStepNumber);

  if(label == 0 && !tam.alpha && tam.digitsSoFar == 0) {
    uint16_t savedCurrentProgramNumber = currentProgramNumber;
    const int result = _clearProgram();
    if(result == 1 && savedCurrentProgramNumber <= 1) {
      fnGotoDot(1);
    }
  }
  else if(label >= FIRST_LABEL && label <= LAST_LABEL) {
    fnGoto(label);
    const uint16_t programNumberToDelete = currentProgramNumber;
    const int result = _clearProgram();
    switch(result) {
      case 2: {
        goToPgmStep(savedCurrentProgramNumber, savedCurrentLocalStepNumber);
        break;
      }
      case 1: {
        if(savedCurrentProgramNumber <= 1) { // RAM
          fnGotoDot(1);
        }
        break;
      }
      case 0: {
        if(programNumberToDelete != savedCurrentProgramNumber) {
          if(programNumberToDelete < savedCurrentProgramNumber) {
            --savedCurrentProgramNumber;
          }
          goToPgmStep(savedCurrentProgramNumber, savedCurrentLocalStepNumber);
          break;
        }
        break;
      }
      default: {
        break;
      }
    }
  }
  else {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "label %" PRIu16 " is not a global label", label);
      moreInfoOnError("In function fnClP:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}



uint32_t _getProgramSize(void) {
  if(currentProgramNumber == numberOfPrograms) {
    uint8_t *step = programList[currentProgramNumber - 1].instructionPointer;
    while(!isAtEndOfPrograms(step)) { // .END.
      step = findNextStep(step);
    }
    return (uint32_t)(step - programList[currentProgramNumber - 1].instructionPointer + 2);
  }
  else {
    return (uint32_t)(programList[currentProgramNumber].instructionPointer - programList[currentProgramNumber - 1].instructionPointer);
  }
}



void defineCurrentProgramFromGlobalStepNumber(int16_t globalStepNumber) {
  currentProgramNumber = 0;
  while(globalStepNumber >= programList[currentProgramNumber].step) {
    currentProgramNumber++;
    if(currentProgramNumber >= numberOfPrograms) {
      break;
    }
  }

  if(currentProgramNumber == numberOfPrograms) {
    endOfCurrentProgram = programList[currentProgramNumber - 1].instructionPointer + _getProgramSize();
  }
  else {
    endOfCurrentProgram = programList[currentProgramNumber].instructionPointer;
  }
  beginOfCurrentProgram = programList[currentProgramNumber - 1].instructionPointer;
}



void defineCurrentProgramFromCurrentStep(void) {
  if(beginOfProgramMemory <= currentStep && currentStep <= firstFreeProgramByte) {
    currentProgramNumber = 0;
    while(currentStep >= programList[currentProgramNumber].instructionPointer) {
      currentProgramNumber++;
      if(currentProgramNumber >= numberOfPrograms) {
        break;
      }
    }

    if(currentProgramNumber >= numberOfPrograms) {
      endOfCurrentProgram = programList[currentProgramNumber - 1].instructionPointer + _getProgramSize();
    }
    else {
      endOfCurrentProgram = programList[currentProgramNumber].instructionPointer;
    }
    beginOfCurrentProgram = programList[currentProgramNumber - 1].instructionPointer;
  }
}



void scrollPemBackwards(void) {
  if(firstDisplayedLocalStepNumber > 0)
    --firstDisplayedLocalStepNumber;
  defineFirstDisplayedStep();
}

void scrollPemForwards(void) {
  if(getNumberOfSteps() > 6) {
    if(currentLocalStepNumber > 3) {
      ++firstDisplayedLocalStepNumber;
      firstDisplayedStep = findNextStep(firstDisplayedStep);
    }
    else if(currentLocalStepNumber == 3) {
      firstDisplayedLocalStepNumber = 1;
    }
  }
}


int32_t pemLeftOffset(int32_t y) {
  if(y > Y_POSITION_OF_REGISTER_T_LINE || X_SHIFT == X_SHIFT_R || Y_SHIFT == 0){
    return 0;
  }
  else {
    return 16; //Offset to allow for f/g
  }
}


#if !defined(TESTSUITE_BUILD)
  static bool_t _isAngleType(uint8_t literalType) {
    switch (literalType) {
      case STRING_ANGLE_RADIAN:
      case STRING_ANGLE_GRAD:
      case STRING_ANGLE_DEGREE:
      case STRING_ANGLE_MULTPI: {
        return true;
        break;
      }
      default: {
        return false;
      }
    }
  }
#endif // !TESTSUITE_BUILD


void fnPem(uint16_t unusedButMandatoryParameter) {
#if !defined(SAVE_SPACE_DM42_10)
  #if !defined(TESTSUITE_BUILD)
    ///////////////////////////////////////////////////////////////////////////////////////
    // For this function to work properly we need the following variables set properly:
    //  - currentProgramNumber
    //  - currentLocalStepNumber
    //  - firstDisplayedLocalStepNumber
    //  - firstDisplayedStep
    //
    uint32_t currentStepNumber, firstDisplayedStepNumber;
    uint16_t line, firstLine;
    uint16_t stepsThatWouldBeDisplayed = 7;
    uint8_t *step, *nextStep;
    bool_t lblOrEnd, lblOrEndOrXeq, gto;
    bool_t inTamMode = tam.mode && programList[currentProgramNumber - 1].step > 0;
    uint16_t numberOfSteps = getNumberOfSteps();
    uint16_t linesOfCurrentStep = 1;
    lastIntegerBase = 0;

    if(calcMode != CM_PEM) {
      showSoftmenu(-MNU_PFN);
      screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;
      calcMode = CM_PEM;
      hourGlassIconEnabled = false;
      aimBuffer[0] = 0;
      currentInputVariable = INVALID_VARIABLE;
      refreshScreen(227);
      return;
    }

    if(currentLocalStepNumber < firstDisplayedLocalStepNumber) {
      firstDisplayedLocalStepNumber = currentLocalStepNumber;
      defineFirstDisplayedStep();
    }

    if(currentLocalStepNumber == 0) {
      currentLocalStepNumber = 1;
      pemCursorIsZerothStep = true;
    }
    else if(currentLocalStepNumber > 1) {
      pemCursorIsZerothStep = false;
    }

    currentStepNumber        = currentLocalStepNumber        + abs(programList[currentProgramNumber - 1].step) - 1;
    firstDisplayedStepNumber = firstDisplayedLocalStepNumber + abs(programList[currentProgramNumber - 1].step) - 1;
    step                     = firstDisplayedStep;
    programListEnd           = false;
    lastProgramListEnd       = false;

    if(firstDisplayedLocalStepNumber == 0) {
      showString("0000:" STD_SPACE_4_PER_EM, &standardFont, pemLeftOffset(Y_POSITION_OF_REGISTER_T_LINE) + 1, Y_POSITION_OF_REGISTER_T_LINE, (pemCursorIsZerothStep && !tam.mode && aimBuffer[0] == 0) ? vmReverse : vmNormal, false, true);
      sprintf(tmpString, "{Prgm #%" PRIu16 "/%" PRIu16 ": %" PRIu32 " bytes / %" PRIu16 " step%s}", currentProgramNumber, numberOfPrograms, _getProgramSize(), numberOfSteps, numberOfSteps == 1 ? "" : "s");
      showString(tmpString, &standardFont, pemLeftOffset(Y_POSITION_OF_REGISTER_T_LINE) + 42, Y_POSITION_OF_REGISTER_T_LINE, vmNormal, false, false);
      firstLine = 1;
    }
    else {
      firstLine = 0;
    }

    int lineOffset = 0, lineOffsetTam = 0;

    for(line=firstLine; line<7; line++) {
      nextStep = findNextStep(step);
      //uint16_t stepSize = (uint16_t)(nextStep - step);
      sprintf(tmpString, "%04d:" STD_SPACE_4_PER_EM, firstDisplayedLocalStepNumber + line - lineOffset + lineOffsetTam);
      if(firstDisplayedStepNumber + line - lineOffset == currentStepNumber) {
        tamOverPemYPos = Y_POSITION_OF_REGISTER_T_LINE + 21 * line;
        showString(tmpString, &standardFont, pemLeftOffset(tamOverPemYPos) + 1, tamOverPemYPos, ((pemCursorIsZerothStep && !tam.mode && aimBuffer[0] == 0) || (tam.mode && (programList[currentProgramNumber - 1].step > 0))) ? vmNormal : vmReverse, false, true);
        currentStep = step;
      }
      else {
        showString(tmpString, &standardFont, pemLeftOffset(Y_POSITION_OF_REGISTER_T_LINE + 21 * line) + 1, Y_POSITION_OF_REGISTER_T_LINE + 21 * line, vmNormal,  false, true);
      }

      //Automatically, when on battery (hence low processor), change to skip long processing register printing, recovering the fragmented screen here: See timer.c fnTimerEndOfActivity() , skippedStackLines
      #if defined(DMCP_BUILD)                                                      // vvv
        if( !(!runningOnSimOrUSB && !emptyKeyBuffer() && key_empty() == 1) ||(firstDisplayedStepNumber + line - lineOffset == currentStepNumber)) {
      #endif

      lblOrEndOrXeq = checkOpCodeOfStep(step, ITM_LBL) || isAtEndOfProgram(step) || isAtEndOfPrograms(step) || checkOpCodeOfStep(step, ITM_XEQ);
      lblOrEnd =      checkOpCodeOfStep(step, ITM_LBL) || isAtEndOfProgram(step) || isAtEndOfPrograms(step);
      gto = checkOpCodeOfStep(step, ITM_GTO);
      if(programList[currentProgramNumber - 1].step > 0) {
        if((!pemCursorIsZerothStep && firstDisplayedStepNumber + line - lineOffset == currentStepNumber + 1) || (line == 1 && tam.mode && pemCursorIsZerothStep)) {
          tamOverPemYPos = Y_POSITION_OF_REGISTER_T_LINE + 21 * line;
          if(tam.mode) {
            line += 1;
            lineOffset += 1;
            lineOffsetTam += 1;
            showString(tmpString, &standardFont, pemLeftOffset(tamOverPemYPos) + 1, tamOverPemYPos, vmReverse, false, true);
            if(line >= 7) {
              break;
            }
            sprintf(tmpString, "%04d:" STD_SPACE_4_PER_EM, firstDisplayedLocalStepNumber + line - lineOffset + lineOffsetTam);
            showString(tmpString, &standardFont, pemLeftOffset(Y_POSITION_OF_REGISTER_T_LINE + 21 * line) + 1, Y_POSITION_OF_REGISTER_T_LINE + 21 * line, vmNormal, false, true);
          }
        }
        else if(firstDisplayedStepNumber + line - lineOffset == currentStepNumber && lblOrEnd && (*step != ITM_LBL)) {
          if(tam.mode) {
            line += 1;
            lineOffset += 1;
            lineOffsetTam += 1;
            showString(tmpString, &standardFont, pemLeftOffset(tamOverPemYPos) + 1, tamOverPemYPos, vmReverse, false, true);
            if(line >= 7) {
              break;
            }
            sprintf(tmpString, "%04d:" STD_SPACE_4_PER_EM, firstDisplayedLocalStepNumber + line - lineOffset + lineOffsetTam);
            showString(tmpString, &standardFont, pemLeftOffset(Y_POSITION_OF_REGISTER_T_LINE + 21 * line) + 1, Y_POSITION_OF_REGISTER_T_LINE + 21 * line, vmNormal, false, true);
          }
        }
      }
      decodeOneStep(step);
      if(firstDisplayedStepNumber + line - lineOffset == currentStepNumber && !tam.mode) {
        if(getSystemFlag(FLAG_ALPHA)) {
          char tmpChar = tmpString[4];
          tmpString[4] = 0;
          int16_t cursorInString = (strcmp(tmpString, "REM ") == 0? T_cursorPos + 4: T_cursorPos);
          tmpString[4] = tmpChar;
          xcopy(tmpString + 2 + cursorInString + 2, tmpString + 2 + cursorInString, stringByteLength(tmpString + 2 + cursorInString) + 1);
          tmpString[2 + cursorInString    ] = STD_CURSOR[0];
          tmpString[2 + cursorInString + 1] = STD_CURSOR[1];
        }
        else if(aimBuffer[0] != 0) {
          char *tstr = tmpString + stringByteLength(tmpString);
          lastIntegerBase = decodedIntegerBase;
          if((lastIntegerBase != 0) && ((nimNumberPart == NP_INT_10) || (nimNumberPart == NP_INT_16))) {
            tstr -= 2;
            *(tstr++) = STD_CURSOR[0];
            *(tstr++) = STD_CURSOR[1];
            *(tstr++) = baseChars[lastIntegerBase * 2    ];
            *(tstr++) = baseChars[lastIntegerBase * 2 + 1];
          }
          else if(_isAngleType(editingLiteralType)) {
            tstr -= 2;                    // to overwrite angle symbol with cursor
            *(tstr++) = STD_CURSOR[0];
            *(tstr++) = STD_CURSOR[1];
            *(tstr++) = angleChars[(editingLiteralType - STRING_ANGLE_RADIAN) * 2    ];
            *(tstr++) = angleChars[(editingLiteralType - STRING_ANGLE_RADIAN) * 2 + 1];
          }
          else {
            *(tstr++) = STD_CURSOR[0];
            *(tstr++) = STD_CURSOR[1];
          }
          *(tstr++) = 0;
        }
      }
      // Split long lines
      int numberOfExtraLines = 0;
      int offset = 0;
      const char *endStr = NULL;
      while(offset <= 1500 && (*(endStr = stringAfterPixels(tmpString + offset, &standardFont, 337, false, false)) != 0)) {
        int lineByteLength = endStr - (tmpString + offset);
        numberOfExtraLines++;
        xcopy(tmpString + offset + 300, tmpString + offset + lineByteLength, stringByteLength(endStr) + 1);
        tmpString[offset + lineByteLength] = 0;
        offset += 300;
      }
      stepsThatWouldBeDisplayed -= numberOfExtraLines;
      if(firstDisplayedStepNumber + line - lineOffset == currentStepNumber) {
        linesOfCurrentStep += numberOfExtraLines;
      }

      showString(tmpString, &standardFont, pemLeftOffset(Y_POSITION_OF_REGISTER_T_LINE + 21 * line) + (lblOrEndOrXeq ? 42 : gto ? 82 : 62), Y_POSITION_OF_REGISTER_T_LINE + 21 * line, vmNormal,  false, false);
      offset = 300;
      while(numberOfExtraLines && line <= 5) {
        line++;
        showString(tmpString + offset, &standardFont, pemLeftOffset(Y_POSITION_OF_REGISTER_T_LINE + 21 * (line)) + 62, Y_POSITION_OF_REGISTER_T_LINE + 21 * (line), vmNormal,  false, false);
        numberOfExtraLines--;
        offset += 300;
        lineOffset++;
      }
      if(isAtEndOfProgram(step)) {
        programListEnd = true;
        if(*nextStep == 255 && *(nextStep + 1) == 255) {
          lastProgramListEnd = true;
        }
        break;
      }
      if((*step == 255) && (*(step + 1) == 255)) {
        programListEnd = true;
        lastProgramListEnd = true;
        break;
      }
      step = nextStep;

      #if defined(DMCP_BUILD)   //^^
        }
      #endif //DMCP_BUILD
    }

    if(lastErrorCode != ERROR_NONE) {
      refreshRegisterLine(errorMessageRegisterLine);
    }

    if(aimBuffer[0] != 0 && linesOfCurrentStep > 4) { // Limited to 4 lines so as not to cause crash or freeze
      if(getSystemFlag(FLAG_ALPHA)) {
        pemAlpha(ITM_BACKSPACE);
      }
      else {
        pemAddNumber(ITM_BACKSPACE, true);
      }

      clearScreen(13);
      showSoftmenuCurrentPart();
      fnPem(NOPARAM);
    }
    if((currentLocalStepNumber + (inTamMode ? (currentLocalStepNumber < numberOfSteps ? 2 : 1) : 0)) >= (firstDisplayedLocalStepNumber + stepsThatWouldBeDisplayed)) {
      firstDisplayedLocalStepNumber = currentLocalStepNumber - stepsThatWouldBeDisplayed + 1 + (inTamMode ? (currentLocalStepNumber < numberOfSteps ? 2 : 1) : 0);
      if(inTamMode && (firstDisplayedLocalStepNumber > 1) && (currentLocalStepNumber + 1 >= (firstDisplayedLocalStepNumber + stepsThatWouldBeDisplayed))) {
        ++firstDisplayedLocalStepNumber;
      }

      defineFirstDisplayedStep();
      clearScreen(14);
      showSoftmenuCurrentPart();
      fnPem(NOPARAM);
    }
  #endif // !TESTSUITE_BUILD
#endif // !SAVE_SPACE_DM42_10
}



static void _insertInProgram(const uint8_t *dat, uint16_t size) {
  //#define printarr(fmt, dat, len) for (uint16_t i = 0; i < len; i++) printf(fmt, dat[i])
  //printf("**[DL]** _insertInProgram: ");
  //printarr("%d ", dat, size);
  //printf("\n");fflush(stdout);
  int16_t _dynamicMenuItem = dynamicMenuItem;
  uint16_t globalStepNumber;

  if(freeProgramBytes < size) {
    uint8_t *oldBeginOfProgramMemory = beginOfProgramMemory;
    uint32_t programSizeInBlocks = RAM_SIZE_IN_BLOCKS - TO_C47MEMPTR(beginOfProgramMemory);
    uint32_t newProgramSizeInBlocks = TO_BLOCKS(TO_BYTES(programSizeInBlocks) - freeProgramBytes + size);
    freeProgramBytes      += TO_BYTES(newProgramSizeInBlocks - programSizeInBlocks);
    resizeProgramMemory(newProgramSizeInBlocks);
    currentStep           = currentStep           - oldBeginOfProgramMemory + beginOfProgramMemory;
    firstDisplayedStep    = firstDisplayedStep    - oldBeginOfProgramMemory + beginOfProgramMemory;
    beginOfCurrentProgram = beginOfCurrentProgram - oldBeginOfProgramMemory + beginOfProgramMemory;
    endOfCurrentProgram   = endOfCurrentProgram   - oldBeginOfProgramMemory + beginOfProgramMemory;
  }

  for(uint8_t *pos = firstFreeProgramByte + 1 + size; pos > currentStep; --pos) {
    *pos = *(pos - size);
  }

  #define tmpA (dat[1]+((dat[0] & 0x7F) << 8))    //convert codes for >RECT and >POLAR to the relevant ones, respecting RP_HP
  if(size == 2 && (tmpA == ITM_toPOL2 || tmpA == ITM_toREC2)) {
    uint16_t tmpB = ITM_toPOL_HP + (tmpA - ITM_toPOL2) + (getSystemFlag(FLAG_HPRP) ? 0 : 2);
    *(currentStep++) = (tmpB >> 8) | 0x80;
    *(currentStep++) = tmpB & 0x00FF;
  }
  else {    //otherwise use the input data to add a step
    for(uint16_t i = 0; i < size; ++i) {
      *(currentStep++) = *(dat++);
    }
  }

  firstFreeProgramByte    += size;
  freeProgramBytes        -= size;
  currentLocalStepNumber  += 1;
  endOfCurrentProgram     += size;
  globalStepNumber = currentLocalStepNumber + programList[currentProgramNumber - 1].step - 1;
  scanLabelsAndPrograms();
  dynamicMenuItem = -1;
  goToGlobalStep(globalStepNumber);
  dynamicMenuItem = _dynamicMenuItem;
}

#if !defined(TESTSUITE_BUILD)
static void _closeAlphaMenus(void) {
  for(int i = 0; i < SOFTMENU_STACK_SIZE; ++i) {
    switch(-softmenu[softmenuStack[0].softmenuId].menuItem) {
      case MNU_ALPHAINTL:
      case MNU_ALPHAintl:
      case MNU_ALPHAMATH:
      case MNU_ALPHA_OMEGA:
      case MNU_alpha_omega:
      case MNU_ALPHA:
        popSoftmenu();
        break;

      case MNU_MyAlpha:
        switch(-softmenu[softmenuStack[1].softmenuId].menuItem) {
          case MNU_ALPHAINTL:
          case MNU_ALPHAintl:
          case MNU_ALPHAMATH:
          case MNU_ALPHA_OMEGA:
          case MNU_alpha_omega:
          case MNU_ALPHA:
            popSoftmenu();
            break;
          default:
            softmenuStack[0].softmenuId = 0; // MyMenu
            return;
        }

      default:
        return;
    }
  }
  // Just in case softmenuStack was filled with AIM-related menus
  for(int i = 0; i < SOFTMENU_STACK_SIZE; ++i) {
    softmenuStack[i].softmenuId = 0; // MyMenu
  }
}
#endif // !TESTSUITE_BUILD

void pemAlpha(int16_t item) {
  #if !defined(TESTSUITE_BUILD)

  bool_t editCommand = false;
  if(item == ITM_EDIT) {
    int16_t aimFunc = currentStep[0];

    if(aimFunc & 0x80) {
      aimFunc &= 0x7f;
      aimFunc <<= 8;
      aimFunc |= currentStep[1];
    }

    tam.function = aimFunc;
    decodeOneStep(currentStep);
    uint16_t ll = stringByteLength(tmpString);
    if(aimFunc == ITM_LITERAL)  { // literal
      xcopy(aimBuffer, tmpString + 2, ll);        //purposely overshoot aimbuffer, as there is sufficient space
      aimBuffer[ll - 2 - 2] = 0;
      T_cursorPos = stringLastGlyph(aimBuffer) + 1;
      deleteStepsFromTo(currentStep, findNextStep(currentStep));
      editCommand = true;
      item = 0;
    }
    else if(aimFunc == ITM_REM)  { // REM
      xcopy(aimBuffer, tmpString + 6, ll);        //purposely overshoot aimbuffer, as there is sufficient space
      aimBuffer[ll - 2 - 6] = 0;
      T_cursorPos = stringLastGlyph(aimBuffer) + 1;
      deleteStepsFromTo(currentStep, findNextStep(currentStep));
      tam.function = aimFunc;
      editCommand = true;
      item = aimFunc;
    }
    else {
      aimBuffer[0] = 0;
      return;
    }
  }

  if(!getSystemFlag(FLAG_ALPHA)) {
      resetShiftState();
      displayAIMbufferoffset = 0;
      if(!editCommand) {
        T_cursorPos = 0;
        aimBuffer[0] = 0;
      }


      //if(softmenuStack[0].softmenuId == 0) { // MyMenu
      //  softmenuStack[0].softmenuId = 1; // MyAlpha
      //}
      showSoftmenu(-MNU_ALPHA);
      setSystemFlag(FLAG_ALPHA);
      calcModeAimGui();

      if(tam.function < 128) { // literal
        tmpString[0] = tam.function;
        tmpString[1] = (char)STRING_LABEL_VARIABLE;
        tmpString[2] = 0;
        _insertInProgram((uint8_t *)tmpString, 3);
      }
      else { // rem
        tmpString[0] = (tam.function >> 8) | 0x80;
        tmpString[1] =  tam.function       & 0x7f;
        tmpString[2] = (char)STRING_LABEL_VARIABLE;
        tmpString[3] = 0;
        _insertInProgram((uint8_t *)tmpString, 4);
      }
      --currentLocalStepNumber;
      currentStep = findPreviousStep(currentStep);
    }
    if(indexOfItems[item].func == addItemToBuffer) {
      int32_t len = stringByteLength(aimBuffer);
      item = numlockReplacements(0, item, getSystemFlag(FLAG_NUMLOCK), shiftF, shiftG);
      if(alphaCase == AC_LOWER) {
          if(ITM_A <= item && item <= ITM_Z) {
            item += (ITM_a - ITM_A);
          }
      }

      if((nextChar == NC_NORMAL) || ((item != ITM_DOWN_ARROW) && (item != ITM_UP_ARROW))) {
        item = convertItemToSubOrSup(item, nextChar);
        int32_t inputCharLength = stringByteLength(indexOfItems[item].itemSoftmenuName);
        if(len < (256 - inputCharLength) && stringGlyphLength(aimBuffer) < 196) {
          xcopy(aimBuffer + T_cursorPos + inputCharLength, aimBuffer + T_cursorPos, stringByteLength(aimBuffer + T_cursorPos) + 1);
          xcopy(aimBuffer + T_cursorPos, indexOfItems[item].itemSoftmenuName, inputCharLength);
          T_cursorPos += inputCharLength;
        }
      }
    }
    else if(item == ITM_BACKSPACE) {
      if(aimBuffer[0] == 0) {
          deleteStepsFromTo(currentStep, findNextStep(currentStep));
        clearSystemFlag(FLAG_ALPHA);
        calcModeNormalGui();
        _closeAlphaMenus();
        return;
      }
      else if(T_cursorPos == 0) {
        return;
      }
      else {
        char cursorByte = aimBuffer[T_cursorPos];
        int16_t lastGlyphPos;
        aimBuffer[T_cursorPos] = 0;
        lastGlyphPos = stringLastGlyph(aimBuffer);
        aimBuffer[T_cursorPos] = cursorByte;
        xcopy(aimBuffer + lastGlyphPos, aimBuffer + T_cursorPos, stringByteLength(aimBuffer + T_cursorPos) + 1);
        T_cursorPos = lastGlyphPos;
      }
    }
    else if(item == ITM_ENTER) {
      pemCloseAlphaInput();
      //--firstDisplayedLocalStepNumber;
      defineFirstDisplayedStep();
        _closeAlphaMenus();
      return;
    }
    else if(item == ITM_USERMODE) {
      fnFlipFlag(FLAG_USER);
      return;
    }
    else if(item == ITM_CLA) { // JM addon
      aimBuffer[0] = 0;
      T_cursorPos = 0;
      nextChar = NC_NORMAL;
    }
    else if(item == CHR_numL && !getSystemFlag(FLAG_NUMLOCK)) { // JM addon
      alphaCase = AC_UPPER;
      SetSetting(indexOfItems[CHR_num].param);
      return;
    }
    else if(item == CHR_numU && getSystemFlag(FLAG_NUMLOCK)) { // JM addon
      alphaCase = AC_UPPER;
      SetSetting(indexOfItems[CHR_num].param);
      return;
    }
    else if(item == CHR_caseUP && alphaCase != AC_UPPER) { // JM addon
      nextChar = NC_NORMAL;
      SetSetting(indexOfItems[CHR_case].param);
      return;
    }
    else if(item == CHR_caseDN && alphaCase != AC_LOWER) { // JM addon
      nextChar = NC_NORMAL;
      SetSetting(indexOfItems[CHR_case].param);
      return;
    }
    else if(item == CHR_num) { // JM addon
      alphaCase = AC_UPPER;
      SetSetting(indexOfItems[item].param);
      return;
    }
    else if(item == CHR_case) { // JM addon
      nextChar = NC_NORMAL;
      clearSystemFlag(FLAG_NUMLOCK);
      SetSetting(indexOfItems[item].param);
      return;
    }
    else if(item == ITM_SCR) { // JM addon
      SetSetting(indexOfItems[item].param);
      return;
    }
    else if(indexOfItems[item].func == fnT_ARROW) { // JM addon
      fnT_ARROW(indexOfItems[item].param);
      return;
    }

    int16_t aimFunc = currentStep[0];
    if(aimFunc & 0x80) {
      aimFunc &= 0x7f;
      aimFunc <<= 8;
      aimFunc |= currentStep[1];
    }

    deleteStepsFromTo(currentStep, findNextStep(currentStep));
    if(aimFunc < 128) { // literal
      tmpString[0] = aimFunc;
      tmpString[1] = (char)STRING_LABEL_VARIABLE;
      tmpString[2] = stringByteLength(aimBuffer);
      xcopy(tmpString + 3, aimBuffer, stringByteLength(aimBuffer));
      _insertInProgram((uint8_t *)tmpString, stringByteLength(aimBuffer) + 3);
    }
    else { // rem
      tmpString[0] = (aimFunc >> 8) | 0x80;
      tmpString[1] =  aimFunc       & 0x7f;
      tmpString[2] = (char)STRING_LABEL_VARIABLE;
      tmpString[3] = stringByteLength(aimBuffer);
      xcopy(tmpString + 4, aimBuffer, stringByteLength(aimBuffer));
      _insertInProgram((uint8_t *)tmpString, stringByteLength(aimBuffer) + 4);
    }
    --currentLocalStepNumber;
    currentStep = findPreviousStep(currentStep);
    if(!programListEnd) {
      scrollPemBackwards();
    }
  #endif // !TESTSUITE_BUILD
}

void pemCloseAlphaInput(void) {
  #if !defined(TESTSUITE_BUILD)
    aimBuffer[0] = 0;
    clearSystemFlag(FLAG_ALPHA);
    calcModeNormalGui();
    ++currentLocalStepNumber;
    currentStep = findNextStep(currentStep);
    ++firstDisplayedLocalStepNumber;
    firstDisplayedStep = findNextStep(firstDisplayedStep);
    _closeAlphaMenus();
  #endif // !TESTSUITE_BUILD
}


void pemAlphaEdit (uint16_t unusedButMandatoryParameter) {
  if(getSystemFlag(FLAG_ALPHA) || calcMode != CM_PEM || tam.mode) {
    hourGlassIconEnabled = false;
    return;
  }
  int16_t func = currentStep[0];
  //printf("DDDDDa func[0] [1] = %i %i\n",func, currentStep[1]);
  if(func & 0x80) {
    func &= 0x7f;
    func <<= 8;
    func |= currentStep[1];
  }
  if((func == ITM_LITERAL || func == ITM_REM)) {
    pemAlpha(ITM_EDIT);
  }
  hourGlassIconEnabled = false;
}


void pemAddNumber(int16_t item, bool doInsertInProgram) {
  #if !defined(TESTSUITE_BUILD)
  if(aimBuffer[0] == 0) {
    lastIntegerBase = 0;
    editingLiteralType = 0;
    tmpString[0] = ITM_LITERAL;
    tmpString[1] = STRING_LONG_INTEGER;
    tmpString[2] = 0;
    _insertInProgram((uint8_t *)tmpString, 3);
    memset(nimBufferDisplay, 0, NIM_BUFFER_LENGTH);
    --currentLocalStepNumber;
    currentStep = findPreviousStep(currentStep);
    switch(item) {
      case ITM_EXPONENT : {
        aimBuffer[0] = '+';
        aimBuffer[1] = '1';
        aimBuffer[2] = '.';
        aimBuffer[3] = 0;
        nimNumberPart = NP_REAL_FLOAT_PART;
        break;
      }

      case ITM_PERIOD : {
        aimBuffer[0] = '+';
        aimBuffer[1] = '0';
        aimBuffer[2] = 0;
        nimNumberPart = NP_INT_10;
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
      case ITM_F : {
        aimBuffer[0] = '+';
        aimBuffer[1] = 0;
        nimNumberPart = NP_EMPTY;
        break;
      }
    }
  }

  if(item == ITM_BACKSPACE && ((aimBuffer[0] == '+' && aimBuffer[1] != 0 && aimBuffer[2] == 0) || aimBuffer[1] == 0)) {
    aimBuffer[0] = 0;
  }
  else {
    addItemToNimBuffer(item);
    if(stringByteLength(aimBuffer) > 255) {
      addItemToNimBuffer(ITM_BACKSPACE);
    }
  }
  clearSystemFlag(FLAG_ALPHA);

  if(aimBuffer[0] != '!') {
    if(doInsertInProgram) {
      deleteStepsFromTo(currentStep, findNextStep(currentStep));
    }
    if(aimBuffer[0] != 0) {
      char *tmpPtr = tmpString;
      char offset = 3;
      const char *numBuffer = aimBuffer[0] == '+' ? aimBuffer + 1 : aimBuffer;
      *tmpPtr++ = ITM_LITERAL;
      switch(nimNumberPart) {
        case NP_INT_10:
        case NP_INT_16: {
        //case NP_INT_BASE: {
          if(lastIntegerBase != 0) {
            *tmpPtr++ = STRING_SHORT_INTEGER;
            *tmpPtr++ = lastIntegerBase;
            offset++;
          }
          else {
            *tmpPtr++ = STRING_LONG_INTEGER;
          }
          break;
        }
        case NP_REAL_FLOAT_PART:
        case NP_REAL_EXPONENT:
        case NP_HP32SII_DENOMINATOR:
        case NP_FRACTION_DENOMINATOR: {
          *tmpPtr++ = STRING_REAL34;
          break;
        }
        case NP_COMPLEX_INT_PART:
        case NP_COMPLEX_FLOAT_PART:
        case NP_COMPLEX_EXPONENT: {
          *tmpPtr++ = STRING_COMPLEX34;
          break;
        }
        default: {
          *tmpPtr++ = STRING_LONG_INTEGER;
          break;
        }
      }
      if(_isAngleType(editingLiteralType)) {
        *(tmpPtr - 1) = editingLiteralType;  // [DL] force literal type when editing angles
      }
      *tmpPtr++ = stringByteLength(numBuffer);
      xcopy(tmpPtr, numBuffer, stringByteLength(numBuffer));;
      if(doInsertInProgram) {
        _insertInProgram((uint8_t *)tmpString, stringByteLength(numBuffer) + offset);
        --currentLocalStepNumber;
        currentStep = findPreviousStep(currentStep);
        if(!programListEnd) {
          scrollPemBackwards();
        }
      }
    }
    calcMode = CM_PEM;
  }
  else {
    aimBuffer[0] = 0;
    nimNumberPart = NP_EMPTY;
  }
  #endif // !TESTSUITE_BUILD
}

void pemCloseNumberInput(void) {
  #if !defined(TESTSUITE_BUILD)
  if(editingLiteralType > 0) {     // For EDIT: close number input with the right data or angle type
    switch(editingLiteralType) {
      case STRING_DATE         : _pemCloseDateInput();            break;
      case STRING_TIME         : _pemCloseTimeInput();            break;
      case STRING_ANGLE_RADIAN : _pemCloseAngleInput(ITM_RAD2);   break;
      case STRING_ANGLE_GRAD   : _pemCloseAngleInput(ITM_GRAD2);  break;
      case STRING_ANGLE_DEGREE : _pemCloseAngleInput(ITM_DEG2);   break;
      case STRING_ANGLE_DMS    : _pemCloseAngleInput(ITM_DMS2);   break;
      case STRING_ANGLE_MULTPI : _pemCloseAngleInput(ITM_MULPI2); break;
      default:     ;
    }
    return;
  }
  deleteStepsFromTo(currentStep, findNextStep(currentStep));
  if(aimBuffer[0] != 0) {
    char *numBuffer = aimBuffer[0] == '+' ? aimBuffer + 1 : aimBuffer;
    char *tmpPtr = tmpString;
    uint32_t inputLength = stringByteLength(numBuffer);
    bool_t doneWithBinaryLiteral = false;
    int16_t lastChar = strlen(aimBuffer) - 1;

    if((lastIntegerBase != 0) && (nimNumberPart == NP_INT_10 || nimNumberPart == NP_INT_16)) {
      sprintf(aimBuffer + strlen(aimBuffer), "#%" PRIu16, (int) lastIntegerBase);
      nimNumberPart = NP_INT_BASE;
    }

    *(tmpPtr++) = ITM_LITERAL;
    if((nimNumberPart == NP_COMPLEX_EXPONENT || nimNumberPart == NP_REAL_EXPONENT) && (aimBuffer[lastChar] == '+' || aimBuffer[lastChar] == '-') && aimBuffer[lastChar - 1] == 'e') {
      aimBuffer[--lastChar] = 0;
      lastChar--;
    }
    else if(nimNumberPart == NP_REAL_EXPONENT && aimBuffer[lastChar] == 'e') {
      aimBuffer[lastChar--] = 0;
    }
    switch(nimNumberPart) {
      //case NP_INT_16:
      case NP_INT_BASE: {
        char *basePtr = numBuffer;
        while(*basePtr != '#') {
          ++basePtr;
        }
        *(basePtr++) = 0;
        inputLength = stringByteLength(numBuffer); // we must update here since the content of numBuffer has been truncated
        if(inputLength >= sizeof(uint64_t) && numBuffer[0] != '-') {
          uint8_t base = (uint8_t)atoi(basePtr);
          uint64_t val = 0;
          *(tmpPtr++) = BINARY_SHORT_INTEGER;
          *(tmpPtr++) = (char)atoi(basePtr);
          for(unsigned int i = 0; i < inputLength; ++i) {
            if('0' <= numBuffer[i] && numBuffer[i] <= '9') {
              val *= base;
              val += numBuffer[i] - '0';
            }
            else if('A' <= numBuffer[i] && numBuffer[i] <= 'F') {
              val *= base;
              val += numBuffer[i] - 'A' + 10;
            }
            else if('a' <= numBuffer[i] && numBuffer[i] <= 'f') {
              val *= base;
              val += numBuffer[i] - 'a' + 10;
            }
          }
          for(unsigned int i = 0; i < sizeof(uint64_t); ++i) {
            *(tmpPtr++) = ((uint8_t *)(&val))[i];
          }
          _insertInProgram((uint8_t *)tmpString, (int32_t)(tmpPtr - tmpString));
          doneWithBinaryLiteral = true;
        }
        else {
          *(tmpPtr++) = STRING_SHORT_INTEGER;
          *(tmpPtr++) = (char)atoi(basePtr);
        }
        break;
      }
      case NP_REAL_FLOAT_PART:
      case NP_REAL_EXPONENT:
      case NP_HP32SII_DENOMINATOR:
      case NP_FRACTION_DENOMINATOR: {
        if(inputLength >= REAL34_SIZE_IN_BYTES) {
          real34_t val;
          *(tmpPtr++) = BINARY_REAL34;
          stringToReal34(numBuffer, &val);
          for(unsigned int i = 0; i < REAL34_SIZE_IN_BYTES; ++i) {
            *(tmpPtr++) = ((uint8_t *)(&val))[i];
          }
          _insertInProgram((uint8_t *)tmpString, (int32_t)(tmpPtr - tmpString));
          doneWithBinaryLiteral = true;
        }
        else {
          *(tmpPtr++) = STRING_REAL34;
        }
        break;
      }
      case NP_COMPLEX_INT_PART:
      case NP_COMPLEX_FLOAT_PART:
      case NP_COMPLEX_EXPONENT: {
        if(aimBuffer[stringByteLength(aimBuffer)-1] == 'i') {
          strcat(aimBuffer,"1");
          inputLength++;
        }

        if(inputLength >= TO_BYTES(COMPLEX34_SIZE_IN_BLOCKS)) {
          real34_t re, im;
          char *imag = numBuffer;
          while(*imag != 'i' && *imag != 0) {
            ++imag;
          }
          if(*imag == 'i') {
            if(imag > numBuffer && *(imag - 1) == '-') {
              *imag = '-'; *(imag - 1) = 0;
            }
            else if(imag > numBuffer && *(imag - 1) == '+') {
              *imag = 0; *(imag - 1) = 0;
              ++imag;
            }
            else {
              *imag = 0;
            }
          }

          *(tmpPtr++) = BINARY_COMPLEX34;
          stringToReal34(numBuffer, &re);
          stringToReal34(imag, &im);
          for(unsigned int i = 0; i < REAL34_SIZE_IN_BYTES; ++i) {
            *(tmpPtr++) = ((uint8_t *)(&re))[i];
          }
          for(unsigned int i = 0; i < REAL34_SIZE_IN_BYTES; ++i) {
            *(tmpPtr++) = ((uint8_t *)(&im))[i];
          }
          _insertInProgram((uint8_t *)tmpString, (int32_t)(tmpPtr - tmpString));
          doneWithBinaryLiteral = true;
        }
        else {
          *(tmpPtr++) = STRING_COMPLEX34;
        }
        break;
      }
      default: {
        *(tmpPtr++) = STRING_LONG_INTEGER;
        break;
      }
    }
    if(!doneWithBinaryLiteral) {
      *(tmpPtr++) = inputLength;
      xcopy(tmpPtr, numBuffer, inputLength);
      _insertInProgram((uint8_t *)tmpString, inputLength + (int32_t)(tmpPtr - tmpString));
      pemCursorIsZerothStep = false;
    }
  }

  aimBuffer[0] = '!';
  nimNumberPart = NP_EMPTY;
  lastIntegerBase = 0;
#endif // TESTSUITE_BUILD
}

static void _pemCloseTimeInput(void) {
  #if !defined(TESTSUITE_BUILD)
  switch(nimNumberPart) {
    case NP_INT_10:
    case NP_REAL_FLOAT_PART: {
      deleteStepsFromTo(currentStep, findNextStep(currentStep));
      if(aimBuffer[0] != 0) {
        char *numBuffer = aimBuffer[0] == '+' ? aimBuffer + 1 : aimBuffer;
        char *tmpPtr = tmpString;
        *(tmpPtr++) = ITM_LITERAL;
        *(tmpPtr++) = STRING_TIME;
        *(tmpPtr++) = stringByteLength(numBuffer);
        xcopy(tmpPtr, numBuffer, stringByteLength(numBuffer));
        _insertInProgram((uint8_t *)tmpString, stringByteLength(numBuffer) + (int32_t)(tmpPtr - tmpString));
      }

      aimBuffer[0] = '!';
      break;
    }
  }
  #endif // !TESTSUITE_BUILD
}

static void _pemCloseDateInput(void) {
  #if !defined(TESTSUITE_BUILD)
  if(nimNumberPart == NP_REAL_FLOAT_PART) {
    deleteStepsFromTo(currentStep, findNextStep(currentStep));
    if(aimBuffer[0] != 0) {
      char *numBuffer = aimBuffer[0] == '+' ? aimBuffer + 1 : aimBuffer;
      char *tmpPtr = tmpString;
      *(tmpPtr++) = ITM_LITERAL;
      *(tmpPtr++) = STRING_DATE;

      reallocateRegister(TEMP_REGISTER_1, dtReal34, 0, amNone);
      stringToReal34(numBuffer, REGISTER_REAL34_DATA(TEMP_REGISTER_1));
      convertReal34RegisterToDateRegister(TEMP_REGISTER_1, TEMP_REGISTER_1, !YYSystem);
      internalDateToJulianDay(REGISTER_REAL34_DATA(TEMP_REGISTER_1), REGISTER_REAL34_DATA(TEMP_REGISTER_1));

      real34ToString(REGISTER_REAL34_DATA(TEMP_REGISTER_1), tmpPtr + 1);
      *tmpPtr = stringByteLength(tmpPtr + 1);
      ++tmpPtr;

      _insertInProgram((uint8_t *)tmpString, stringByteLength(tmpPtr) + (int32_t)(tmpPtr - tmpString));
    }

    aimBuffer[0] = '!';
  }
  #endif // !TESTSUITE_BUILD
}

static void _pemCloseAngleInput(int item) {
  #if !defined(TESTSUITE_BUILD)
  switch(nimNumberPart) {
    case NP_INT_10:
    case NP_REAL_FLOAT_PART: {
      deleteStepsFromTo(currentStep, findNextStep(currentStep));
      if(aimBuffer[0] != 0) {
        char *numBuffer = aimBuffer[0] == '+' ? aimBuffer + 1 : aimBuffer;
        char *tmpPtr = tmpString;
        *(tmpPtr++) = ITM_LITERAL;
        static const int angle_ids[] = {
            [ITM_DEG2]   = STRING_ANGLE_DEGREE,
            [ITM_DMS2]   = STRING_ANGLE_DMS,
            [ITM_GRAD2]  = STRING_ANGLE_GRAD,
            [ITM_MULPI2] = STRING_ANGLE_MULTPI,
            [ITM_RAD2]   = STRING_ANGLE_RADIAN
        };
        int id = -1;
        if (item >= 0 && item < (int)(sizeof(angle_ids)/sizeof(angle_ids[0]))) {
            id = angle_ids[item];
        }
        if (id != -1) {
            *(tmpPtr++) = id;
        }
        *(tmpPtr++) = stringByteLength(numBuffer);
        xcopy(tmpPtr, numBuffer, stringByteLength(numBuffer));
        _insertInProgram((uint8_t *)tmpString, stringByteLength(numBuffer) + (int32_t)(tmpPtr - tmpString));
      }
      editingLiteralType = 0;
      aimBuffer[0] = '!';
      break;
    }
  }
  #endif // !TESTSUITE_BUILD
}

void insertStepInProgram(const int16_t func) {
                                #if defined(DEBUG_PGM)
                                  print_caller(NULL);
                                #endif

  uint32_t opBytes = (func >= 128) ? 2 : 1;

  if(func == ITM_END) {
    firstDisplayedLocalStepNumber = 0;
  }

  if(func == ITM_AIM || (!tam.mode && getSystemFlag(FLAG_ALPHA))) {
    if(aimBuffer[0] != 0 && !getSystemFlag(FLAG_ALPHA)) {
      pemCloseNumberInput();
      aimBuffer[0] = 0;
    }
    tam.function = ITM_LITERAL;
    pemAlpha(func);
    pemCursorIsZerothStep = false;
    return;
  }
  else if(func == ITM_REM || (!tam.mode && getSystemFlag(FLAG_ALPHA))) {
    if(aimBuffer[0] != 0 && !getSystemFlag(FLAG_ALPHA)) {
      pemCloseNumberInput();
      aimBuffer[0] = 0;
    }
    if(catalog) {      // If called from a catalog such as FNCS, exit catalog and Asm Mode
      #if !defined(TESTSUITE_BUILD)
        leaveAsmMode();
        popSoftmenu();
      #endif // !TESTSUITE_BUILD
    }
    tam.function = ITM_REM;
    pemAlpha(func);
    pemCursorIsZerothStep = false;
    return;
  }
  if(   indexOfItems[func].func == addItemToBuffer 
     || (!tam.mode && aimBuffer[0] != 0 && (   func == ITM_CHS || func == ITM_CC || func == ITM_op_j || func == ITM_op_j_pol || func == ITM_toINT 
                                            || (nimNumberPart == NP_INT_BASE && (   ( isR47FAM && (func == ITM_SQUAREROOTX || func == ITM_YX)) 
                                                                                 || (!isR47FAM && (func == ITM_1ONX        || func == ITM_LOG10)) 
                                                                                 || func == ITM_RCL || func == ITM_EXPONENT || func == ITM_ENTER))))) {
    pemAddNumber(func, true);
    return;
  }
  else if(nimNumberPart == NP_INT_BASE) {
    return;
  }
  else if(func == ITM_CONSTpi && aimBuffer[0] != 0 && !getSystemFlag(FLAG_ALPHA) && nimNumberPart == NP_COMPLEX_INT_PART && aimBuffer[strlen(aimBuffer) - 1] == 'i') {
    strcat(aimBuffer, "3.141592653589793238462643383279503");
    pemCloseNumberInput();
    aimBuffer[0] = 0;
    return;
  }
  else if((func == ITM_DMS || func == ITM_DMS2 || func == ITM_DEG2 || func == ITM_GRAD2 || func == ITM_RAD2 || func == ITM_MULPI2) && aimBuffer[0] != 0 && !getSystemFlag(FLAG_ALPHA) && (nimNumberPart == NP_INT_10 || nimNumberPart == NP_REAL_FLOAT_PART)) {
    _pemCloseAngleInput(func);
    aimBuffer[0] = 0;
    return;
  }
  else if ((func == ITM_dotD) && editingLiteralType != 0 && aimBuffer[0] != 0 && !getSystemFlag(FLAG_ALPHA)) {  // cancel time/date/angle type and close number input
    editingLiteralType = 0;
    pemCloseNumberInput();
    aimBuffer[0] = '!';
  }
  else if((func == ITM_DRG) && aimBuffer[0] != 0 && !getSystemFlag(FLAG_ALPHA) && (nimNumberPart == NP_INT_10 || nimNumberPart == NP_REAL_FLOAT_PART)) {
    switch(currentAngularMode) {
      case amRadian : _pemCloseAngleInput(ITM_RAD2); break;
      case amGrad   : _pemCloseAngleInput(ITM_GRAD2); break;
      case amDegree : _pemCloseAngleInput(ITM_DEG2); break;
      case amDMS    : _pemCloseAngleInput(ITM_DMS2); break;
      case amMultPi : _pemCloseAngleInput(ITM_MULPI2); break;
      default: return;
    }
    aimBuffer[0] = 0;
    return;
  }

  if(!tam.mode && !tam.alpha && aimBuffer[0] != 0 && func != ITM_HMStoTM && func != ITM_EXIT1) {
    if((func == ITM_dotD) || ((editingLiteralType == STRING_DATE) && ( func != ITM_ms)))  {
      _pemCloseDateInput();
      if(aimBuffer[0] == '!') {
        aimBuffer[0] = 0;
        return;
      }
    }
    else if((func == ITM_ms) || (editingLiteralType == STRING_TIME)) {
      _pemCloseTimeInput();
      if(aimBuffer[0] == '!') {
        aimBuffer[0] = 0;
        return;
      }
    }
    else {
      switch(editingLiteralType) {   // For EDIT: close number input with the right angle type
        case STRING_ANGLE_RADIAN : _pemCloseAngleInput(ITM_RAD2); break;
        case STRING_ANGLE_GRAD   : _pemCloseAngleInput(ITM_GRAD2); break;
        case STRING_ANGLE_DEGREE : _pemCloseAngleInput(ITM_DEG2); break;
        case STRING_ANGLE_DMS    : _pemCloseAngleInput(ITM_DMS2); break;
        case STRING_ANGLE_MULTPI : _pemCloseAngleInput(ITM_MULPI2); break;
        default:     pemCloseNumberInput();
      }
      aimBuffer[0] = 0;
      if(func == ITM_ENTER) {  // just close number editing, don't insert ENTER
        return;
      }
    }
  }

  char buffer[16];
  xcopy(buffer, tmpString, 16);    // Save tmpString content for dynamic menus

  if(func < 128) {
    tmpString[0] = func;
  }
  else {
    tmpString[0] = (func >> 8) | 0x80;
    tmpString[1] =  func       & 0xff;
  }

   switch(indexOfItems[func].status & PTP_STATUS) {
    case PTP_DISABLED: {
      switch(func) {
        case ITM_KEYG:           // 1498
        case ITM_KEYX: {         // 1499
            int opLen;
            tmpString[0] = (char)((ITM_KEY >> 8) | 0x80);
            tmpString[1] = (char)( ITM_KEY       & 0xff);
            if(tam.keyAlpha) {
              uint16_t nameLength = stringByteLength(aimBuffer + AIM_BUFFER_LENGTH / 2);
              tmpString[2] = (char)INDIRECT_VARIABLE;
              tmpString[3] = (char)nameLength;
              xcopy(tmpString + 4, aimBuffer + AIM_BUFFER_LENGTH / 2, nameLength);
              opLen = nameLength + 4;
            }
            else if(tam.keyIndirect) {
              tmpString[2] = (char)INDIRECT_REGISTER;
              tmpString[3] = tam.key;
              opLen = 4;
            }
            else {
              tmpString[2] = tam.key;
              opLen = 3;
            }

            tmpString[opLen + 0] = (func == ITM_KEYX ? ITM_XEQ : ITM_GTO);
            if(tam.alpha) {
              uint16_t nameLength = stringByteLength(aimBuffer);
              tmpString[opLen + 1] = (char)(tam.indirect ? INDIRECT_VARIABLE : STRING_LABEL_VARIABLE);
              tmpString[opLen + 2] = nameLength;
              xcopy(tmpString + opLen + 3, aimBuffer, nameLength);
              _insertInProgram((uint8_t *)tmpString, nameLength + opLen + 3);
            }
            else if(tam.indirect) {
              tmpString[opLen + 1] = (char)INDIRECT_REGISTER;
              tmpString[opLen + 2] = tam.value;
              _insertInProgram((uint8_t *)tmpString, opLen + 3);
            }
            else {
              tmpString[opLen + 1] = tam.value;
              _insertInProgram((uint8_t *)tmpString, opLen + 2);
            }
          break;
        }

        case ITM_GTOP: {         // 1482
          #if !defined(DMCP_BUILD)
            stringToUtf8(indexOfItems[func].itemCatalogName, (uint8_t *)tmpString);
            printf("insertStepInProgram: %s\n", tmpString);
          #endif // DMCP_BUILD
          break;
        }

        //case ITM_DELP: {          // 1425
        //  fnClP(NOPARAM);
        //  break;
        //}

        case ITM_DELPALL: {       // 1426
          fnClPAll(NOT_CONFIRMED);
          break;
        }

        case ITM_BST: {          // 1734
          fnBst(NOPARAM);
          break;
        }

        case ITM_SST: {          // 1736
          fnSst(NOPARAM);
          break;
        }

        case VAR_ACC: {          // 1192
          tmpString[0] = ITM_STO;
          tmpString[1] = (char)STRING_LABEL_VARIABLE;
          tmpString[2] = 3;
          tmpString[3] = 'A';
          tmpString[4] = 'C';
          tmpString[5] = 'C';
          _insertInProgram((uint8_t *)tmpString, 6);
          break;
        }

        case VAR_UEST:           // 2545
        case VAR_LEST: {         // 2546
          tmpString[0] = ITM_STO;
          tmpString[1] = (char)STRING_LABEL_VARIABLE;
          tmpString[2] = 5;
          if(func == VAR_UEST) {
            tmpString[3] = STD_UP_ARROW[0];
            tmpString[4] = STD_UP_ARROW[1];
          }
          else {
            tmpString[3] = STD_DOWN_ARROW[0];
            tmpString[4] = STD_DOWN_ARROW[1];
          }
          tmpString[5] = 'E';
          tmpString[6] = 's';
          tmpString[7] = 't';
          _insertInProgram((uint8_t *)tmpString, 8);
          break;
        }

        case VAR_ULIM:           // 1193
        case VAR_LLIM: {         // 1194
          tmpString[0] = ITM_STO;
          tmpString[1] = (char)STRING_LABEL_VARIABLE;
          tmpString[2] = 5;
          if(func == VAR_ULIM) {
            tmpString[3] = STD_UP_ARROW[0];
            tmpString[4] = STD_UP_ARROW[1];
          }
          else {
            tmpString[3] = STD_DOWN_ARROW[0];
            tmpString[4] = STD_DOWN_ARROW[1];
          }
          tmpString[5] = 'L';
          tmpString[6] = 'i';
          tmpString[7] = 'm';
          _insertInProgram((uint8_t *)tmpString, 8);
          break;
      }

        case VAR_UY:             // 2547
        case VAR_LY:             // 2548
        case VAR_UX:             // 1205
        case VAR_LX: {           // 1206
          tmpString[0] = ITM_STO;
          tmpString[1] = (char)STRING_LABEL_VARIABLE;
          tmpString[2] = 3;
          if(func == VAR_UX || func == VAR_UY) {
            tmpString[3] = STD_UP_ARROW[0];
            tmpString[4] = STD_UP_ARROW[1];
          }
          else {
            tmpString[3] = STD_DOWN_ARROW[0];
            tmpString[4] = STD_DOWN_ARROW[1];
          }
          if(func == VAR_UX || func == VAR_LX) {
            tmpString[5] = 'X';
          } else {
            tmpString[5] = 'Y';
          }
          _insertInProgram((uint8_t *)tmpString, 6);
          break;
      }

        case ITM_USERMODE: {     // 1729
          fnFlipFlag(FLAG_USER);
          break;
        }
      }
      break;
    }

    case PTP_NONE: {
      if(func == ITM_HMStoTM  && aimBuffer[0] != 0 && !getSystemFlag(FLAG_ALPHA)) {
        _pemCloseTimeInput();
        if(aimBuffer[0] != '!') {
          pemCloseNumberInput();
          aimBuffer[0] = 0;
          tmpString[0] = (func >> 8) | 0x80;
          tmpString[1] =  func       & 0xff;
          _insertInProgram((uint8_t *)tmpString, 2);
        }
        aimBuffer[0] = 0;
      }
      else {
        _insertInProgram((uint8_t *)tmpString, 1 + (func >= 128));
      }
      break;
    }

    case PTP_NUMBER_16: {
      if(isFunctionOldParam16(func)) {  // original Param16 functions without indirection support (little endian parameter)
        tmpString[2] = (char)(tam.value & 0xff); // little endian
        tmpString[3] = (char)(tam.value >> 8);
        _insertInProgram((uint8_t *)tmpString, 4);
      }
      else {                        // new Param16 functions with indirection support (big endian parameter)
        if(tam.alpha && tam.indirect) {
          uint16_t nameLength = stringByteLength(aimBuffer);
          tmpString[opBytes    ] = (char)(INDIRECT_VARIABLE);
          tmpString[opBytes + 1] = nameLength;
          xcopy(tmpString + opBytes + 2, aimBuffer, nameLength);
          _insertInProgram((uint8_t *)tmpString, nameLength + opBytes + 2);
        }
        else if(tam.indirect) {
          tmpString[opBytes    ] = (char)INDIRECT_REGISTER;
          tmpString[opBytes + 1] = tam.value + (tam.dot ? FIRST_LOCAL_REGISTER : 0);
          _insertInProgram((uint8_t *)tmpString, opBytes + 2);
        }
        else {
        tmpString[2] = (char)(tam.value >> 8);   // BIG endian
        tmpString[3] = (char)(tam.value & 0xff);
        _insertInProgram((uint8_t *)tmpString, 4);
        }
      }
      break;
    }

    case PTP_LITERAL:
    case PTP_REM: {
      // nothing to do here
      break;
    }

    case PTP_SKIP_BACK: {
        tmpString[opBytes    ] = (tam.dot ? tam.value + FIRST_LOCAL_REGISTER_IN_KS_CODE : tam.value);
        _insertInProgram((uint8_t *)tmpString, opBytes + 1);
      break;
    }

    default: {
      uint32_t opBytes = 1 + (func >= 128);

      if(tam.mode == TM_VALUE && ((indexOfItems[func].status & PTP_STATUS) == PTP_NUMBER_8_16) && tam.value > 250) {
        tmpString[opBytes    ] = (char)CNST_BEYOND_250;
        tmpString[opBytes + 1] = tam.value - 250;
        _insertInProgram((uint8_t *)tmpString, opBytes + 2);
      }
      else if(tam.mode == TM_CMP && tam.value == TEMP_REGISTER_1) {
        tmpString[opBytes    ] = (char)(real34IsZero(REGISTER_REAL34_DATA(TEMP_REGISTER_1)) ? VALUE_0 : VALUE_1);
        _insertInProgram((uint8_t *)tmpString, opBytes + 1);
      }
      else if((tam.mode == TM_FLAGR || tam.mode == TM_FLAGW) && tam.alpha && !tam.indirect) {
        tmpString[opBytes    ] = (char)SYSTEM_FLAG_NUMBER;
        tmpString[opBytes + 1] = tam.value;
        _insertInProgram((uint8_t *)tmpString, opBytes + 2);
      }
      else if((tam.mode == TM_MENU) && !tam.alpha && !tam.indirect) {
        uint16_t nameLength;
        tmpString[opBytes    ] = (char)STRING_LABEL_VARIABLE;
        if(tam.value == MNU_DYNAMIC) {
          nameLength  = stringByteLength(buffer);
          tmpString[opBytes + 1] = nameLength;
          xcopy(tmpString + opBytes + 2, buffer, nameLength);
          _insertInProgram((uint8_t *)tmpString, nameLength + opBytes + 2);
        }
        else {
          nameLength  = stringByteLength(indexOfItems[tam.value].itemCatalogName);
          tmpString[opBytes + 1] = nameLength;
          xcopy(tmpString + opBytes + 2, indexOfItems[tam.value].itemCatalogName, nameLength);
          _insertInProgram((uint8_t *)tmpString, nameLength + opBytes + 2);
        }
      }
      else if(tam.alpha) {
        uint16_t nameLength = stringByteLength(aimBuffer);
        tmpString[opBytes    ] = (char)(tam.indirect ? INDIRECT_VARIABLE : STRING_LABEL_VARIABLE);
        tmpString[opBytes + 1] = nameLength;
        xcopy(tmpString + opBytes + 2, aimBuffer, nameLength);
        _insertInProgram((uint8_t *)tmpString, nameLength + opBytes + 2);
      }
      else if(tam.indirect) {
        tmpString[opBytes    ] = (char)INDIRECT_REGISTER;
        tmpString[opBytes + 1] = (tam.dot ? tam.value + FIRST_LOCAL_REGISTER_IN_KS_CODE : regCtoKS(tam.value));
        _insertInProgram((uint8_t *)tmpString, opBytes + 2);
      }
      else {
        tmpString[opBytes    ] = (tam.dot ? tam.value + FIRST_LOCAL_REGISTER_IN_KS_CODE : (tam.mode == TM_LABEL || tam.mode == TM_LBLONLY) ? tam.value : regCtoKS(tam.value));
        _insertInProgram((uint8_t *)tmpString, opBytes + 1);
      }
  }
  }

  if(func != ITM_EDIT) {
    aimBuffer[0] = 0;
  }
}


void insertUserItemInProgram(int16_t func, char *funcParam) {
  uint32_t opBytes=0;
  uint16_t nameLength = stringByteLength(funcParam);

  if((!pemCursorIsZerothStep) && ((aimBuffer[0] == 0 && !getSystemFlag(FLAG_ALPHA)) || tam.mode) && !isAtEndOfProgram(currentStep) && !isAtEndOfPrograms(currentStep)) {
    currentStep = findNextStep(currentStep);
    ++currentLocalStepNumber;
  }
  if(func < 128) {
    tmpString[opBytes++] = func;
  }
  else {
    tmpString[opBytes++] = (func >> 8) | 0x80;
    tmpString[opBytes++] =  func       & 0x7f;
  }

  tmpString[opBytes    ] = (char)STRING_LABEL_VARIABLE;
  tmpString[opBytes + 1] = nameLength;
  xcopy(tmpString + opBytes + 2, funcParam, nameLength);
  _insertInProgram((uint8_t *)tmpString, nameLength + opBytes + 2);

  currentStep = findPreviousStep(currentStep);
  if(currentLocalStepNumber > 1) {
    --currentLocalStepNumber;
  }
  pemCursorIsZerothStep = false;
  if(!programListEnd) {
    scrollPemBackwards();
  }
}

#if defined(PC_BUILD) && defined(DEBUG_PGM)
  #include <execinfo.h>
#endif // PC_BUILD &&MONITOR_CLRSCR

void addStepInProgram(int16_t func) {
                                #if defined(DEBUG_PGM)
                                  print_caller(NULL);
                                #endif
  if((!pemCursorIsZerothStep) && ((aimBuffer[0] == 0 && !getSystemFlag(FLAG_ALPHA)) || tam.mode) && !isAtEndOfProgram(currentStep) && !isAtEndOfPrograms(currentStep)) {
    currentStep = findNextStep(currentStep);
    ++currentLocalStepNumber;
  }
  insertStepInProgram(func);
  if((aimBuffer[0] == 0 && !getSystemFlag(FLAG_ALPHA)) || tam.mode) {
    currentStep = findPreviousStep(currentStep);
    if(currentLocalStepNumber > 1) {
      --currentLocalStepNumber;
    }
    pemCursorIsZerothStep = false;
    if((indexOfItems[func].status & PTP_STATUS) == PTP_DISABLED) {
      switch(func) {
        case VAR_ACC:            // 1192
        case VAR_ULIM:           // 1193
        case VAR_LLIM:           // 1194
        case VAR_UX:             // 1205
        case VAR_LX:             // 1206
        case VAR_UEST:           // 2545
        case VAR_LEST:           // 2546
        case VAR_UY:             // 2547
        case VAR_LY:             // 2548
        case ITM_DELP:           // 1425
        case ITM_DELPALL:        // 1426
        case ITM_GTOP:           // 1482
        case ITM_KEYG:           // 1498
        case ITM_KEYX:           // 1499
        case ITM_BST:            // 1734
        case ITM_SST: {          // 1736
          break;
        }
        default: {
          return;
        }
      }
    }
    if(!programListEnd) {
      scrollPemBackwards();
    }
  }
}



calcRegister_t findNamedLabel(const char *labelName) {
  return findNamedLabelWithDuplicate(labelName, 0);
}



calcRegister_t findNamedLabelWithDuplicate(const char *labelName, int16_t dupNum) {
  for(uint16_t lbl = 0; lbl < numberOfLabels; lbl++) {
    if(labelList[lbl].step > 0) {
      xcopy(tmpString, labelList[lbl].labelPointer + 1, *(labelList[lbl].labelPointer));
      tmpString[*(labelList[lbl].labelPointer)] = 0;
      if(compareString(tmpString, labelName, CMP_BINARY) == 0) {
        if(dupNum <= 0) {
          return lbl + FIRST_LABEL;
        }
        else {
          --dupNum;
        }
      }
    }
  }
  return INVALID_VARIABLE;
}



uint16_t getNumberOfSteps(void) {
  if(currentProgramNumber == numberOfPrograms) {
    uint16_t numberOfSteps = 1;
    uint8_t *step = programList[currentProgramNumber - 1].instructionPointer;
    while(!isAtEndOfPrograms(step)) { // .END.
      ++numberOfSteps;
      step = findNextStep(step);
    }
    return numberOfSteps;
  }
  else {
    return abs(programList[currentProgramNumber].step - programList[currentProgramNumber - 1].step);
  }
}
