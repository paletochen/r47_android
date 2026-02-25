// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

#if !defined(TESTSUITE_BUILD)
  static bool_t recallElementReal(real34Matrix_t *matrix) {
    const int16_t i = getIRegisterAsInt(true);
    const int16_t j = getJRegisterAsInt(true);

    liftStack();
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    real34Copy(&matrix->matrixElements[i * matrix->header.matrixColumns + j], REGISTER_REAL34_DATA(REGISTER_X));
    return false;
  }

  static bool_t recallElementComplex(complex34Matrix_t *matrix) {
    const int16_t i = getIRegisterAsInt(true);
    const int16_t j = getJRegisterAsInt(true);

    liftStack();
    reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
    complex34Copy(&matrix->matrixElements[i * matrix->header.matrixColumns + j], REGISTER_COMPLEX34_DATA(REGISTER_X));
    return false;
  }
#endif // !TESTSUITE_BUILD



void fnRecall(uint16_t regist) {
  if(regInRange(regist)) {
    if(REGISTER_X <= regist && regist <= getStackTop()) {
      copySourceRegisterToDestRegister(regist, TEMP_REGISTER_1);
      liftStack();
      copySourceRegisterToDestRegister(TEMP_REGISTER_1, REGISTER_X);
    }
    else {
      if(getSystemFlag(FLAG_ASLIFT)) {
        fnRollUp(NOPARAM);
      }
      copySourceRegisterToDestRegister(regist, REGISTER_X);
      if(getRegisterDataType(REGISTER_X) == dtShortInteger) {
        *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) &= shortIntegerMask;
      }
    }
  }
}


void fn2Rcl(uint16_t regist) {
  if((/*regist >= FIRST_GLOBAL_REGISTER &&*/ regist <= (REGISTER_X-1)-1) || (regist >= REGISTER_X && regist <= REGISTER_W-1) || (FIRST_LOCAL_REGISTER <= regist && regist < FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters - 1)) {
    setSystemFlag(FLAG_ASLIFT);
    fnRecall(regist + 1);
    setSystemFlag(FLAG_ASLIFT);
    fnRecall(regist + 0);
  } else {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "%04d", regist);
      moreInfoOnError("In function fn2Rcl:", errorMessage, " is not defined!", NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void fn3Rcl(uint16_t regist) {
  if((/*regist >= FIRST_GLOBAL_REGISTER &&*/ regist <= (REGISTER_X-1)-2) || (regist >= REGISTER_X && regist <= REGISTER_W-2) || (FIRST_LOCAL_REGISTER <= regist && regist < FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters - 2)) {
    setSystemFlag(FLAG_ASLIFT);
    fnRecall(regist + 2);
    setSystemFlag(FLAG_ASLIFT);
    fnRecall(regist + 1);
    setSystemFlag(FLAG_ASLIFT);
    fnRecall(regist + 0);
  } else {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "%04d", regist);
      moreInfoOnError("In function fn3Rcl:", errorMessage, " is not defined!", NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}




void fnLastX(uint16_t unusedButMandatoryParameter) {
  fnRecall(REGISTER_L);
}



void fnRecallPlusSkip(uint16_t regist) {
  fnRecall(regist);
  fnSkip(0);
}



void fnRecallAdd(uint16_t regist) {
  if(regInRange(regist)) {
    if(programRunStop == PGM_RUNNING && regist == REGISTER_L) {
      copySourceRegisterToDestRegister(REGISTER_L, SAVED_REGISTER_L);
      if(lastErrorCode != ERROR_NONE) {
        return;
      }
    }
    if(!saveLastX()) {
      return;
    }
    if(programRunStop == PGM_RUNNING) {
      copySourceRegisterToDestRegister(REGISTER_Y, SAVED_REGISTER_Y);
    }
    copySourceRegisterToDestRegister(REGISTER_X, REGISTER_Y);
    copySourceRegisterToDestRegister(regist == REGISTER_Y ? SAVED_REGISTER_Y : regist == REGISTER_L ? SAVED_REGISTER_L : regist, REGISTER_X);
    if(getRegisterDataType(REGISTER_X) == dtShortInteger) {
      *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) &= shortIntegerMask;
    }

    addition[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)]();

    copySourceRegisterToDestRegister(SAVED_REGISTER_Y, REGISTER_Y);

    adjustResult(REGISTER_X, false, true, REGISTER_X, regist, -1);
  }
}



void fnRecallSub(uint16_t regist) {
  if(regInRange(regist)) {
    if(programRunStop == PGM_RUNNING && regist == REGISTER_L) {
      copySourceRegisterToDestRegister(REGISTER_L, SAVED_REGISTER_L);
      if(lastErrorCode != ERROR_NONE) {
        return;
      }
    }
    if(!saveLastX()) {
      return;
    }
    if(programRunStop == PGM_RUNNING) {
      copySourceRegisterToDestRegister(REGISTER_Y, SAVED_REGISTER_Y);
    }
    copySourceRegisterToDestRegister(REGISTER_X, REGISTER_Y);
    copySourceRegisterToDestRegister(regist == REGISTER_Y ? SAVED_REGISTER_Y : regist == REGISTER_L ? SAVED_REGISTER_L : regist, REGISTER_X);
    if(getRegisterDataType(REGISTER_X) == dtShortInteger) {
      *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) &= shortIntegerMask;
    }

    subtraction[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)]();

    copySourceRegisterToDestRegister(SAVED_REGISTER_Y, REGISTER_Y);

    adjustResult(REGISTER_X, false, true, REGISTER_X, regist, -1);
  }
}



void fnRecallMult(uint16_t regist) {
  if(regInRange(regist)) {
    if(programRunStop == PGM_RUNNING && regist == REGISTER_L) {
      copySourceRegisterToDestRegister(REGISTER_L, SAVED_REGISTER_L);
      if(lastErrorCode != ERROR_NONE) {
        return;
      }
    }
    if(!saveLastX()) {
      return;
    }
    if(programRunStop == PGM_RUNNING) {
      copySourceRegisterToDestRegister(REGISTER_Y, SAVED_REGISTER_Y);
    }
    copySourceRegisterToDestRegister(REGISTER_X, REGISTER_Y);
    copySourceRegisterToDestRegister(regist == REGISTER_Y ? SAVED_REGISTER_Y : regist == REGISTER_L ? SAVED_REGISTER_L : regist, REGISTER_X);
    if(getRegisterDataType(REGISTER_X) == dtShortInteger) {
      *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) &= shortIntegerMask;
    }

    multiplication[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)]();

    copySourceRegisterToDestRegister(SAVED_REGISTER_Y, REGISTER_Y);

    adjustResult(REGISTER_X, false, true, REGISTER_X, regist, -1);
  }
}



void fnRecallDiv(uint16_t regist) {
  if(regInRange(regist)) {
    if(programRunStop == PGM_RUNNING && regist == REGISTER_L) {
      copySourceRegisterToDestRegister(REGISTER_L, SAVED_REGISTER_L);
      if(lastErrorCode != ERROR_NONE) {
        return;
      }
    }
    if(!saveLastX()) {
      return;
    }
    if(programRunStop == PGM_RUNNING) {
      copySourceRegisterToDestRegister(REGISTER_Y, SAVED_REGISTER_Y);
    }
    copySourceRegisterToDestRegister(REGISTER_X, REGISTER_Y);
    copySourceRegisterToDestRegister(regist == REGISTER_Y ? SAVED_REGISTER_Y : regist == REGISTER_L ? SAVED_REGISTER_L : regist, REGISTER_X);
    if(getRegisterDataType(REGISTER_X) == dtShortInteger) {
      *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) &= shortIntegerMask;
    }

    division[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)]();

    copySourceRegisterToDestRegister(SAVED_REGISTER_Y, REGISTER_Y);

    adjustResult(REGISTER_X, false, true, REGISTER_X, regist, -1);
  }
}



void fnRecallMin(uint16_t regist) {
  if(regInRange(regist)) {
    if(programRunStop == PGM_RUNNING && regist == REGISTER_L) {
      copySourceRegisterToDestRegister(REGISTER_L, SAVED_REGISTER_L);
      if(lastErrorCode != ERROR_NONE) {
        return;
      }
    }
    if(!saveLastX()) {
      return;
    }
    if(FIRST_RESERVED_VARIABLE <= regist && regist < LAST_RESERVED_VARIABLE && allReservedVariables[regist - FIRST_RESERVED_VARIABLE].header.pointerToRegisterData == C47_NULL) {
      copySourceRegisterToDestRegister(regist == REGISTER_L ? SAVED_REGISTER_L : regist, TEMP_REGISTER_1);
      regist = TEMP_REGISTER_1;
    }
    registerMin(REGISTER_X, regist, REGISTER_X);
  }
}



void fnRecallMax(uint16_t regist) {
  if(regInRange(regist)) {
    if(programRunStop == PGM_RUNNING && regist == REGISTER_L) {
      copySourceRegisterToDestRegister(REGISTER_L, SAVED_REGISTER_L);
      if(lastErrorCode != ERROR_NONE) {
        return;
      }
    }
    if(!saveLastX()) {
      return;
    }
    if(FIRST_RESERVED_VARIABLE <= regist && regist < LAST_RESERVED_VARIABLE && allReservedVariables[regist - FIRST_RESERVED_VARIABLE].header.pointerToRegisterData == C47_NULL) {
      copySourceRegisterToDestRegister(regist == REGISTER_L ? SAVED_REGISTER_L : regist, TEMP_REGISTER_1);
      regist = TEMP_REGISTER_1;
    }
    registerMax(REGISTER_X, regist, REGISTER_X);
  }
}



//static bool_t getRecalledSystemFlag(int32_t sf) {
//  int32_t flag = sf & 0x3fff;
//
//  if(flag < 64) {
//    return (configToRecall->systemFlags0 & ((uint64_t)1 << flag)) != 0;
//  }
//  else {
//    return (configToRecall->systemFlags1 & ((uint64_t)1 << (flag - 64))) != 0;
//  }
//}

void fnRecallConfig(uint16_t regist) {
    __attribute__((unused)) int16_t compatibility_int1;     //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte00;    //for use in spare slots below
    __attribute__((unused)) uint8_t compatibility_byte1;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte2 ;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte3 ;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte4 ;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte5 ;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte6 ;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte7 ;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte8 ;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte9 ;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte10;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte11;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte12;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte13;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte14;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte15;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte16;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte17;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte18;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte19;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte20;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte21;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte22;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte23;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte24;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte25;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte26;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte27;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte28;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte29;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte30;    //for use in spare slots below
    __attribute__((unused)) bool_t compatibility_byte31;    //for use in spare slots below
    __attribute__((unused)) float  compatibility_float1;    //for use in spare slots below
    __attribute__((unused)) float  compatibility_float2;    //for use in spare slots below
  if(getRegisterDataType(regist) == dtConfig) {
    dtConfigDescriptor_t *configToRecall = REGISTER_CONFIG_DATA(regist);

    recallFromDtConfigDescriptor(shortIntegerMode);
    recallFromDtConfigDescriptor(shortIntegerWordSize);
    recallFromDtConfigDescriptor(displayFormat);
    recallFromDtConfigDescriptor(displayFormatDigits);
    recallFromDtConfigDescriptor(gapItemLeft);
    recallFromDtConfigDescriptor(gapItemRight);
    recallFromDtConfigDescriptor(gapItemRadix);
    recallFromDtConfigDescriptor(grpGroupingLeft);
    recallFromDtConfigDescriptor(grpGroupingGr1LeftOverflow);
    recallFromDtConfigDescriptor(grpGroupingGr1Left);
    recallFromDtConfigDescriptor(grpGroupingRight);
    recallFromDtConfigDescriptor(currentAngularMode);
    recallFromDtConfigDescriptor(lrSelection);
    recallFromDtConfigDescriptor(lrChosen);
    recallFromDtConfigDescriptor(denMax);
    recallFromDtConfigDescriptor(displayStack);
    recallFromDtConfigDescriptor(firstGregorianDay);
    recallFromDtConfigDescriptor(roundingMode);
    recallFromDtConfigDescriptor(systemFlags0);
    recallFromDtConfigDescriptor(systemFlags1);
    xcopy(kbd_usr, configToRecall->kbd_usr, sizeof(kbd_usr));
    recallFromDtConfigDescriptor(    compatibility_byte1);
    recallFromDtConfigDescriptor(    compatibility_byte19);
    recallFromDtConfigDescriptor(    compatibility_byte28);
    recallFromDtConfigDescriptor(    compatibility_byte29);
    recallFromDtConfigDescriptor(    compatibility_byte21);
    recallFromDtConfigDescriptor(    compatibility_byte30);                       //fixed!
    recallFromDtConfigDescriptor(    compatibility_byte00);   //spare
    recallFromDtConfigDescriptor(    compatibility_int1);     //spare
    recallFromDtConfigDescriptor(Input_Default);
    recallFromDtConfigDescriptor(dispBase);
    recallFromDtConfigDescriptor(compatibility_byte31);
    recallFromDtConfigDescriptor(compatibility_byte26);
    recallFromDtConfigDescriptor(compatibility_float1);   //spare
    recallFromDtConfigDescriptor(compatibility_float2);   //spare
    recallFromDtConfigDescriptor(Norm_Key_00.func);
    xcopy(Norm_Key_00.funcParam, configToRecall->Norm_Key_00.funcParam, sizeof(Norm_Key_00.funcParam));
    recallFromDtConfigDescriptor(Norm_Key_00.used);
    recallFromDtConfigDescriptor(    compatibility_byte2);
    recallFromDtConfigDescriptor(    compatibility_byte3);
    recallFromDtConfigDescriptor(    compatibility_byte4);
    recallFromDtConfigDescriptor(    compatibility_byte5);
    recallFromDtConfigDescriptor(    compatibility_byte6);
    recallFromDtConfigDescriptor(    compatibility_byte7);
    recallFromDtConfigDescriptor(    compatibility_byte8);
    recallFromDtConfigDescriptor(    compatibility_byte9);
    recallFromDtConfigDescriptor(    compatibility_byte10);
    recallFromDtConfigDescriptor(    compatibility_byte11);
    recallFromDtConfigDescriptor(    compatibility_byte12);
    recallFromDtConfigDescriptor(    compatibility_byte13);
    recallFromDtConfigDescriptor(    compatibility_byte14);
    recallFromDtConfigDescriptor(    compatibility_byte15);
    recallFromDtConfigDescriptor(fractionDigits);
    recallFromDtConfigDescriptor(    compatibility_byte23);
    recallFromDtConfigDescriptor(    compatibility_byte16);    //spare
    recallFromDtConfigDescriptor(    compatibility_byte20);
    recallFromDtConfigDescriptor(    compatibility_byte17);
    recallFromDtConfigDescriptor(IrFractionsCurrentStatus);
    recallFromDtConfigDescriptor(    compatibility_byte18);
    recallFromDtConfigDescriptor(displayStackSHOIDISP);
    recallFromDtConfigDescriptor(    compatibility_byte25);
    recallFromDtConfigDescriptor(    compatibility_byte24);
    recallFromDtConfigDescriptor(bcdDisplaySign);
    recallFromDtConfigDescriptor(DRG_Cycling);
    recallFromDtConfigDescriptor(DM_Cycling);
    recallFromDtConfigDescriptor(    compatibility_byte22);
    recallFromDtConfigDescriptor(LongPressM);
    recallFromDtConfigDescriptor(LongPressF);
    recallFromDtConfigDescriptor(lastDenominator);
    recallFromDtConfigDescriptor(significantDigits);
    recallFromDtConfigDescriptor(pcg32_global);
    recallFromDtConfigDescriptor(exponentLimit);
    recallFromDtConfigDescriptor(exponentHideLimit);
    recallFromDtConfigDescriptor(lastIntegerBase);
    recallFromDtConfigDescriptor(compatibility_byte27);
    recallFromDtConfigDescriptor(timeDisplayFormatDigits);
  }

  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "data type %s cannot be used to recall a configuration!", getRegisterDataTypeName(regist, false, false));
      moreInfoOnError("In function fnRecallConfig:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  #if !defined(TESTSUITE_BUILD)
    forceSBupdate();
  #endif //TESTSUITE_BUILD
}



void fnRecallStack(uint16_t regist) {
  uint16_t size = getSystemFlag(FLAG_SSIZE8) ? 8 : 4;

  if(REGISTER_X - size <= regist && regist < REGISTER_X) {
    displayCalcErrorMessage(ERROR_STACK_CLASH, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Cannot execute RCLS, destination register would overlap the stack: %d", regist);
      moreInfoOnError("In function fnRecallStack:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else if((REGISTER_X <= regist && regist < FIRST_LOCAL_REGISTER) || regist + size > FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Cannot execute RCLS, destination register is out of range: %d", regist);
      moreInfoOnError("In function fnRecallStack:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    int i;

    if(!saveLastX()) {
      return;
    }

    for(i=0; i<size; i++) {
      copySourceRegisterToDestRegister(regist + i, REGISTER_X + i);
    }

    for(i=0; i<4; i++) {
      adjustResult(REGISTER_X + i, false, true, REGISTER_X + i, -1, -1);
    }
  }
}


static void _fnRecallElement(bool_t stepForward);

void fnRecallVElement(uint16_t ix) {
  #if !defined(TESTSUITE_BUILD)
  const int16_t iBak = getIRegisterAsInt(true);
  const int16_t jBak = getJRegisterAsInt(true);

  if((getRegisterDataType(REGISTER_X) == dtReal34Matrix) || (getRegisterDataType(REGISTER_X) == dtComplex34Matrix)) {
    if(getRegisterDataType(REGISTER_X) == dtReal34Matrix) {
      real34Matrix_t x;
      linkToRealMatrixRegister(REGISTER_X, &x);
      //printf("ix:%u Rows:%u Cols:%u \n",ix,x.header.matrixRows, x.header.matrixColumns);
      setIRegisterAsInt(false, (ix-1) / x.header.matrixColumns+1);
      setJRegisterAsInt(false, (ix-1) % x.header.matrixColumns+1);
    }
    else { //Complex Matrix
      complex34Matrix_t x;
      linkToComplexMatrixRegister(REGISTER_X, &x);
      setIRegisterAsInt(false, (ix-1) / x.header.matrixColumns+1);
      setJRegisterAsInt(false, (ix-1) % x.header.matrixColumns+1);
    }
    uint16_t matrixIndexBak = matrixIndex;
    matrixIndex = REGISTER_X;
    _fnRecallElement(false);
    setIRegisterAsInt(false, iBak);
    setJRegisterAsInt(false, jBak);
    matrixIndex = matrixIndexBak;
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      #if defined(PC_BUILD)
    sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType(REGISTER_X));
    moreInfoOnError("In function fnRecallVElement:", errorMessage, "is not a matrix.", "");
    #endif
  }
  #endif // !TESTSUITE_BUILD
}


void fnRecallElementPlus(uint16_t unusedButMandatoryParameter) {
  _fnRecallElement(true);
}

void fnRecallElement(uint16_t unusedButMandatoryParameter) {
  _fnRecallElement(false);
}

void _fnRecallElement(bool_t stepForward) {
  #if !defined(TESTSUITE_BUILD)
    if(matrixIndex == INVALID_VARIABLE) {
      displayCalcErrorMessage(ERROR_NO_MATRIX_INDEXED, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Cannot execute RCLEL without a matrix indexed");
        moreInfoOnError("In function fnRecallElement:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      callByIndexedMatrix(recallElementReal, recallElementComplex);
      if(stepForward) {
        fnIncDecJ(INC_FLAG);
      }
    }
  #endif // !TESTSUITE_BUILD
}


void fnRecallIJ(uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
    if(matrixIndex == INVALID_VARIABLE) {
      displayCalcErrorMessage(ERROR_NO_MATRIX_INDEXED, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Cannot execute RCLIJ without a matrix indexed");
        moreInfoOnError("In function fnRecallIJ:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      longInteger_t zero;
      longIntegerInit(zero);

      if(!saveLastX()) {
        return;
      }

      liftStack();
      liftStack();

      if(matrixIndex == INVALID_VARIABLE || !regInRange(matrixIndex) || !((getRegisterDataType(matrixIndex) == dtReal34Matrix) || (getRegisterDataType(matrixIndex) == dtComplex34Matrix))) {
        convertLongIntegerToLongIntegerRegister(zero, REGISTER_Y);
        convertLongIntegerToLongIntegerRegister(zero, REGISTER_X);
      }
      else {
        if(getRegisterDataType(REGISTER_I) == dtLongInteger) {
          copySourceRegisterToDestRegister(REGISTER_I, REGISTER_Y);
        }
        else if(getRegisterDataType(REGISTER_I) == dtReal34) {
          convertReal34ToLongIntegerRegister(REGISTER_REAL34_DATA(REGISTER_I), REGISTER_Y, DEC_ROUND_DOWN);
        }
        else {
          convertLongIntegerToLongIntegerRegister(zero, REGISTER_Y);
        }
        if(getRegisterDataType(REGISTER_J) == dtLongInteger) {
          copySourceRegisterToDestRegister(REGISTER_J, REGISTER_X);
        }
        else if(getRegisterDataType(REGISTER_J) == dtReal34) {
          convertReal34ToLongIntegerRegister(REGISTER_REAL34_DATA(REGISTER_J), REGISTER_X, DEC_ROUND_DOWN);
        }
        else {
          convertLongIntegerToLongIntegerRegister(zero, REGISTER_X);
        }
      }

      adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
      adjustResult(REGISTER_Y, false, true, REGISTER_Y, -1, -1);


      longIntegerFree(zero);
    }
  #endif // !TESTSUITE_BUILD
}
