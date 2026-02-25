// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"
#if defined(PC_BUILD) && defined(DEBUGUNDO)
  #include <execinfo.h>
#endif // PC_BUILD &&MONITOR_CLRSCR

void fnClX(uint16_t unusedButMandatoryParameter) {
  clearRegister(REGISTER_X);
}



void fnClearStack(uint16_t unusedButMandatoryParameter) {
  for(calcRegister_t regist=REGISTER_X; regist<=getStackTop(); regist++) {
    clearRegister(regist);
  }
}



void liftStack(void) {
  if(getSystemFlag(FLAG_ASLIFT)) {
    if(currentInputVariable != INVALID_VARIABLE) {
      currentInputVariable |= 0x8000;
    }
    freeRegisterData(getStackTop());
    for(uint16_t i=getStackTop(); i>REGISTER_X; i--) {
      globalRegister[i] = globalRegister[i-1];
    }
  }
  else {
    freeRegisterData(REGISTER_X);
  }

  setRegisterDataPointer(REGISTER_X, allocC47Blocks(REAL34_SIZE_IN_BLOCKS));
  setRegisterDataType(REGISTER_X, dtReal34, amNone);
}



void _Drop(calcRegister_t reg) {
  if(reg == getStackTop()) {
    return;
  }
  freeRegisterData(reg);
  for(uint16_t i=reg; i<getStackTop(); i++) {
    globalRegister[i] = globalRegister[i+1];
  }

  uint16_t sizeInBlocks = getRegisterFullSizeInBlocks(getStackTop());
  void *dataPtr = allocC47Blocks(sizeInBlocks);
  if(dataPtr) {
    setRegisterDataPointer(getStackTop() - 1, dataPtr);
    xcopy(getRegisterDataPointer(getStackTop() - 1), getRegisterDataPointer(getStackTop()), TO_BYTES(sizeInBlocks));
  }
  else {
    lastErrorCode = ERROR_RAM_FULL;
  }
}

void fnDrop(uint16_t unusedButMandatoryParameter) {
  _Drop(REGISTER_X);
}

void fnDropY(uint16_t unusedButMandatoryParameter) {
  _Drop(REGISTER_Y);
}

void fnDropZ(uint16_t unusedButMandatoryParameter) {
  _Drop(REGISTER_Z);
}

void fnDropT(uint16_t unusedButMandatoryParameter) {
  _Drop(REGISTER_T);
}

void fnDropN(uint16_t number) {
  for(int n = 0; n < min(8,number); n++) {
    _Drop(REGISTER_X);
  }
}


void fnRollUp(uint16_t unusedButMandatoryParameter) {
  registerHeader_t savedRegisterHeader = globalRegister[getStackTop()];

  for(uint16_t i=getStackTop(); i>REGISTER_X; i--) {
    globalRegister[i] = globalRegister[i-1];
  }
  globalRegister[REGISTER_X] = savedRegisterHeader;
}



void fnRollDown(uint16_t unusedButMandatoryParameter) {
  registerHeader_t savedRegisterHeader = globalRegister[REGISTER_X];

  for(uint16_t i=REGISTER_X; i<getStackTop(); i++) {
    globalRegister[i] = globalRegister[i+1];
  }
  globalRegister[getStackTop()] = savedRegisterHeader;
}



void fnDisplayStack(uint16_t numberOfStackLines) {
  displayStack = numberOfStackLines;
}


static void _swapRegs(uint16_t srcReg, uint16_t regist) {
  registerHeader_t savedRegisterHeader = globalRegister[srcReg];

  if(regist <= LAST_GLOBAL_REGISTER) {
    globalRegister[srcReg] = globalRegister[regist];
    globalRegister[regist] = savedRegisterHeader;
  }

  else if(regist < FIRST_NAMED_VARIABLE + numberOfNamedVariables) {
    globalRegister[srcReg] = allNamedVariables[regist - FIRST_NAMED_VARIABLE].header;
    allNamedVariables[regist - FIRST_NAMED_VARIABLE].header = savedRegisterHeader;
  }

  else if(regist < FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters) {
    globalRegister[srcReg] = currentLocalRegisters[regist - FIRST_LOCAL_REGISTER];
    currentLocalRegisters[regist - FIRST_LOCAL_REGISTER] = savedRegisterHeader;
  }

  #if defined(PC_BUILD)
    else if(regist <= LAST_LOCAL_REGISTER) {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
      sprintf(errorMessage, "local register .%02d", regist - FIRST_LOCAL_REGISTER);
      moreInfoOnError("In function _swapRegs:", errorMessage, "is not defined!", NULL);
    }
  #endif // PC_BUILD

  #if defined(PC_BUILD)
    else {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
      sprintf(errorMessage, "register %d", regist);
      moreInfoOnError("In function _swapRegs:", errorMessage, "is unsupported!", NULL);
    }
  #endif // PC_BUILD
}


void fnSwapX(uint16_t regist) {
  _swapRegs(REGISTER_X, regist);
}


void fnSwapY(uint16_t regist) {
  _swapRegs(REGISTER_Y, regist);
}


void fnSwapZ(uint16_t regist) {
  _swapRegs(REGISTER_Z, regist);
}


void fnSwapT(uint16_t regist) {
  _swapRegs(REGISTER_T, regist);
}


void fnSwapN(uint16_t number) {
  for(int n = 0; n < min(4,number); n++) {
  _swapRegs(REGISTER_X + n, REGISTER_X + number + n);
  }
}


void fnDupN(uint16_t number) {
  for(int n = 0; n < min(4, number); n++) {
    setSystemFlag(FLAG_ASLIFT);
    fnRecall(REGISTER_X + (number - 1));
  }
}


void fnSwapXY(uint16_t unusedButMandatoryParameter) {
  registerHeader_t savedRegisterHeader = globalRegister[REGISTER_X];

  globalRegister[REGISTER_X] = globalRegister[REGISTER_Y];
  globalRegister[REGISTER_Y] = savedRegisterHeader;
}

void fnShuffle(uint16_t regist_order) {
  for(int i=0; i<4; i++) {
    registerHeader_t savedRegisterHeader = globalRegister[REGISTER_X + i];
    globalRegister[REGISTER_X + i] = globalRegister[i + SAVED_REGISTER_X];
    globalRegister[i + SAVED_REGISTER_X] = savedRegisterHeader;
  }
  for(int i=0; i<4; i++) {
    uint16_t regist_offset = (regist_order >> (i*2)) & 3;
    copySourceRegisterToDestRegister(SAVED_REGISTER_X + regist_offset, REGISTER_X + i);
  }
}



void fnFillStack(uint16_t unusedButMandatoryParameter) {
  uint16_t dataTypeX         = getRegisterDataType(REGISTER_X);
  uint16_t dataSizeXinBlocks = getRegisterFullSizeInBlocks(REGISTER_X);
  uint16_t tag               = getRegisterTag(REGISTER_X);

  for(uint16_t i=REGISTER_Y; i<=getStackTop(); i++) {
    freeRegisterData(i);
    setRegisterDataType(i, dataTypeX, tag);
    void *newDataPointer = allocC47Blocks(dataSizeXinBlocks);
    if(newDataPointer) {
      setRegisterDataPointer(i, newDataPointer);
      xcopy(newDataPointer, getRegisterDataPointer(REGISTER_X), TO_BYTES(dataSizeXinBlocks));
    }
    else {
      lastErrorCode = ERROR_RAM_FULL;
      return;
    }
  }
}



void fnGetStackSize(uint16_t unusedButMandatoryParameter) {
  longInteger_t stack;

  liftStack();

  longIntegerInit(stack);
  uInt32ToLongInteger(getSystemFlag(FLAG_SSIZE8) ? 8 : 4, stack);
  convertLongIntegerToLongIntegerRegister(stack, REGISTER_X);
  longIntegerFree(stack);
}



void saveForUndo(void) {
                                #if defined(DEBUGUNDO)
                                  print_caller(NULL);
                                #endif
  if(((calcMode == CM_NIM || calcMode == CM_AIM || calcMode == CM_MIM) && thereIsSomethingToUndo) || calcMode == CM_NO_UNDO) {
    #if defined(DEBUGUNDO)
      if(thereIsSomethingToUndo) {
        printf(">>> saveForUndo; calcMode = %i, nothing stored as there is something to undo\n", calcMode);
      }
      if(calcMode == CM_NIM || calcMode == CM_AIM || calcMode == CM_MIM || calcMode == CM_NO_UNDO) {
        printf(">>> saveForUndo; calcMode = %i, nothing stored, wrong mode\n", calcMode);
      }
    #endif // DEBUGUNDO
    return;
  }
  #if defined(DEBUGUNDO)
    printf(">>> in saveForUndo, saving; calcMode = %i pre:thereIsSomethingToUndo = %i ;", calcMode, thereIsSomethingToUndo);
    printf("Clearing TEMP_REGISTER_2_SAVED_STATS\n\n");
  #endif // DEBUGUNDO

  clearRegister(TEMP_REGISTER_2_SAVED_STATS); //clear it here for every saveForUndo call, and explicitly set it in fnEditMatrix() and fnEqSolvGraph() only
  SAVED_SIGMA_lastAddRem = SIGMA_NONE;

  savedSystemFlags0 = systemFlags0;
  savedSystemFlags1 = systemFlags1;

  if(currentInputVariable != INVALID_VARIABLE) {
    if(currentInputVariable & 0x8000) {
      currentInputVariable |= 0x4000;
    }
    else {
      currentInputVariable &= 0xbfff;
    }
  }

  if(entryStatus & 0x01) {
    entryStatus |= 0x02;
  }
  else {
    entryStatus &= 0xfd;
  }

  for(calcRegister_t regist=getStackTop(); regist>=REGISTER_X; regist--) {
    copySourceRegisterToDestRegister(regist, SAVED_REGISTER_X - REGISTER_X + regist);
    if(lastErrorCode == ERROR_RAM_FULL) {
      #if defined(PC_BUILD)
        printf("In function saveForUndo: not enough space for saving register #%" PRId16 "!\n", regist);
        fflush(stdout);
      #endif // PC_BUILD
      goto failed;
    }
  }

  copySourceRegisterToDestRegister(REGISTER_L, SAVED_REGISTER_L);
  if(lastErrorCode == ERROR_RAM_FULL) {
    #if defined(PC_BUILD)
      printf("In function saveForUndo: not enough space for saving register L!\n");
      fflush(stdout);
    #endif // PC_BUILD
    goto failed;
  }

  lrSelectionUndo = lrSelection;
  if(statisticalSumsPointer == NULL) { // There are no statistical sums to save for undo
    freeC47Blocks(savedStatisticalSumsPointer, NUMBER_OF_STATISTICAL_SUMS * REAL_SIZE_IN_BLOCKS);
    savedStatisticalSumsPointer = NULL;
  }
  else { // There are statistical sums to save for undo
    lrChosenUndo = lrChosen;
    if(savedStatisticalSumsPointer == NULL) {
      savedStatisticalSumsPointer = allocC47Blocks(NUMBER_OF_STATISTICAL_SUMS * REAL_SIZE_IN_BLOCKS);
    }
    xcopy(savedStatisticalSumsPointer, statisticalSumsPointer, NUMBER_OF_STATISTICAL_SUMS * REAL_SIZE_IN_BYTES);
  }

  thereIsSomethingToUndo = true;
  return;

failed:
  for(calcRegister_t regist=getStackTop(); regist>=REGISTER_X; regist--) {
    clearRegister(SAVED_REGISTER_X - REGISTER_X + regist);
  }
  clearRegister(SAVED_REGISTER_L);
  thereIsSomethingToUndo = false;
  lastErrorCode = ERROR_RAM_FULL;
  return;
}



void fnUndo(uint16_t unusedButMandatoryParameter) {
                                #if defined(DEBUGUNDO)
                                  print_caller(NULL);
                                #endif
  if(thereIsSomethingToUndo) {
    undo();
  }
}



void undo(void) {
  #if defined(DEBUGUNDO)
    printf(">>> Undoing, calcMode = %i ...", calcMode);
  #endif // DEBUGUNDO
                                        #if defined(DEBUGUNDO)
                                          printf("Pre-existing error code: Error number %d:%s\n", lastErrorCode, errorMessages[lastErrorCode]);
                                          print_caller(NULL);
                                        #endif // DEBUGUNDO

  const bool_t wasSolving = getSystemFlag(FLAG_SOLVING);
  const bool_t wasInting = getSystemFlag(FLAG_INTING);

  const uint8_t lastErrorCodeMeM = lastErrorCode;
  lastErrorCode = ERROR_NONE;
  recallStatsMatrix();
  if(lastErrorCode == ERROR_NONE) lastErrorCode = lastErrorCodeMeM;

  if(currentInputVariable != INVALID_VARIABLE) {
    if(currentInputVariable & 0x4000) {
      currentInputVariable |= 0x8000;
    }
    else {
      currentInputVariable &= 0x7fff;
    }
  }

  if(entryStatus & 0x02) {
    entryStatus |= 0x01;
  }
  else {
    entryStatus &= 0xfe;
  }

  if(SAVED_SIGMA_lastAddRem == SIGMA_PLUS && statisticalSumsPointer != NULL) {
    fnSigmaAddRem(SIGMA_MINUS);
  }
  else if(SAVED_SIGMA_lastAddRem == SIGMA_MINUS && statisticalSumsPointer != NULL) {
    convertRealToResultRegister(&SAVED_SIGMA_LASTX, REGISTER_X, amNone);             // Can use stack, as the stack will be undone below
    convertRealToResultRegister(&SAVED_SIGMA_LASTY, REGISTER_Y, amNone);
    fnSigmaAddRem(SIGMA_PLUS);
  }

  systemFlags0 = savedSystemFlags0;
  systemFlags1 = savedSystemFlags1;

  for(calcRegister_t regist=getStackTop(); regist>=REGISTER_X; regist--) {
    copySourceRegisterToDestRegister(SAVED_REGISTER_X - REGISTER_X + regist, regist);
  }

  copySourceRegisterToDestRegister(SAVED_REGISTER_L, REGISTER_L);

  lrSelection = lrSelectionUndo;
  if(savedStatisticalSumsPointer == NULL) { // There are no statistical sums to restore
    freeC47Blocks(statisticalSumsPointer, NUMBER_OF_STATISTICAL_SUMS * REAL_SIZE_IN_BLOCKS);
    statisticalSumsPointer = NULL;
    lrChosen = 0;
  }
  else { // There are statistical sums to restore
    lrChosen = lrChosenUndo;
    if(statisticalSumsPointer == NULL) {
      statisticalSumsPointer = allocC47Blocks(NUMBER_OF_STATISTICAL_SUMS * REAL_SIZE_IN_BLOCKS);
    }
    xcopy(statisticalSumsPointer, savedStatisticalSumsPointer, NUMBER_OF_STATISTICAL_SUMS * REAL_SIZE_IN_BYTES);
  }

  SAVED_SIGMA_lastAddRem = SIGMA_NONE;
  thereIsSomethingToUndo = false;
  clearRegister(TEMP_REGISTER_2_SAVED_STATS);

  if(wasSolving != getSystemFlag(FLAG_SOLVING)) {
    flipSystemFlag(FLAG_SOLVING);
  }
  if(wasInting != getSystemFlag(FLAG_INTING)) {
    flipSystemFlag(FLAG_INTING);
  }

  #if defined(DEBUGUNDO)
    printf(">>> Undone, calcMode = %i\n", calcMode);
  #endif // DEBUGUNDO

}


void fillStackWithReal0(void) {
  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
  int32ToReal34(0, REGISTER_REAL34_DATA(REGISTER_X));
  fnFillStack(0);
}
