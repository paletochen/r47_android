// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"


bool_t isRegInRange(uint16_t regist) {
  return (regist <= LAST_LETTERED_REGISTER) ||                                                           // r00 -> r99 and includes X -> K
    (FIRST_STAT_REGISTER  <= regist && regist <= LAST_STAT_REGISTER) ||                                  // M -> S
    (FIRST_SPARE_REGISTER <= regist && regist <= LAST_SPARE_REGISTER) ||                                 // E -> W
    (FIRST_LOCAL_REGISTER <= regist && regist < FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters) || // 7000 -> 7098
    (FIRST_NAMED_VARIABLE <= regist && regist < FIRST_NAMED_VARIABLE + numberOfNamedVariables) ||        // 256  -> 1999 *
    (FIRST_RESERVED_VARIABLE <= regist && regist <= LAST_RESERVED_VARIABLE) ||                           // 2000 -> 2047 *
    (FIRST_TEMP_REGISTER  <= regist && regist <= LAST_TEMP_REGISTER);                                    // 135, 136
}

bool_t regInRange(uint16_t regist) {
  if(isRegInRange(regist)) {
    return true;
  }
  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    const char *regType;
    uint16_t offset;
    uint8_t  errorType = ERROR_OUT_OF_RANGE;

    if(regist <= LAST_LETTERED_REGISTER) {
      regType = "Lettered register";
      offset = regist - FIRST_LETTERED_REGISTER;
    }
    else if(FIRST_STAT_REGISTER <= regist && regist <= LAST_STAT_REGISTER) {
      regType = "Stat register";
      offset = regist - FIRST_STAT_REGISTER;
    }
    else if(FIRST_SPARE_REGISTER <= regist && regist <= LAST_SPARE_REGISTER) {
      regType = "Spare register";
      offset = regist - FIRST_SPARE_REGISTER;
    }
    else if(FIRST_LOCAL_REGISTER <= regist && regist < FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters) {
      regType = "local register .";
      offset = regist - FIRST_LOCAL_REGISTER;
    }
    else if(FIRST_NAMED_VARIABLE <= regist && regist < FIRST_NAMED_VARIABLE + numberOfNamedVariables) {
      regType = "named register .";
      offset = regist - FIRST_NAMED_VARIABLE;
      errorType = ERROR_UNDEF_SOURCE_VAR;
    }
    else if(FIRST_RESERVED_VARIABLE <= regist && regist <= LAST_RESERVED_VARIABLE) {
      regType = "reserved variable";
      offset = regist - FIRST_RESERVED_VARIABLE;
    }
    else if(FIRST_TEMP_REGISTER <= regist && regist <= LAST_TEMP_REGISTER) {
      regType = "temporary register";
      offset = regist - LAST_TEMP_REGISTER;
    }
    else {
      regType = "generic";
      offset = 0;
    }
    displayCalcErrorMessage(errorType, ERR_REGISTER_LINE, REGISTER_X);
    if(strcmp(regType, "generic") == 0) {
      sprintf(errorMessage, "generic");
    }
    else {
      sprintf(errorMessage, "%s %04d", regType, offset);
    }
    moreInfoOnError("In function regInRange:", errorMessage, " is not defined!", NULL);
  #else
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
  #endif
  return false;
}


static bool_t _checkReadOnlyVariable(uint16_t regist) {
  if(FIRST_RESERVED_VARIABLE <= regist && regist <= LAST_RESERVED_VARIABLE && allReservedVariables[regist - FIRST_RESERVED_VARIABLE].header.readOnly == 1) {
    displayCalcErrorMessage(ERROR_WRITE_PROTECTED_VAR, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "reserved variable %s", allReservedVariables[regist - FIRST_RESERVED_VARIABLE].reservedVariableName + 1);
      moreInfoOnError("In function _checkReadOnlyVariable:", errorMessage, " is write-protected!", NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }
  else {
    return true;
  }
}



#if !defined(TESTSUITE_BUILD)
  static bool_t storeElementReal(real34Matrix_t *matrix) {
    const int16_t i = getIRegisterAsInt(true);
    const int16_t j = getJRegisterAsInt(true);

    if(getRegisterDataType(REGISTER_X) == dtLongInteger) {
      convertLongIntegerRegisterToReal34(REGISTER_X, &matrix->matrixElements[i * matrix->header.matrixColumns + j]);
    }
    else if(getRegisterDataType(REGISTER_X) == dtReal34) {
      real34Copy(REGISTER_REAL34_DATA(REGISTER_X), &matrix->matrixElements[i * matrix->header.matrixColumns + j]);
    }
    else {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Cannot store %s in a matrix", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function storeElementReal:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    return true;
  }

  static bool_t storeElementComplex(complex34Matrix_t *matrix) {
    const int16_t i = getIRegisterAsInt(true);
    const int16_t j = getJRegisterAsInt(true);

    if(getRegisterDataType(REGISTER_X) == dtLongInteger) {
      convertLongIntegerRegisterToReal34(REGISTER_X, VARIABLE_REAL34_DATA(&matrix->matrixElements[i * matrix->header.matrixColumns + j]));
      real34Zero(VARIABLE_IMAG34_DATA(&matrix->matrixElements[i * matrix->header.matrixColumns + j]));
    }
    else if(getRegisterDataType(REGISTER_X) == dtReal34) {
      real34Copy(REGISTER_REAL34_DATA(REGISTER_X), VARIABLE_REAL34_DATA(&matrix->matrixElements[i * matrix->header.matrixColumns + j]));
      real34Zero(VARIABLE_IMAG34_DATA(&matrix->matrixElements[i * matrix->header.matrixColumns + j]));
    }
    else if(getRegisterDataType(REGISTER_X) == dtComplex34) {
      real34Copy(REGISTER_REAL34_DATA(REGISTER_X), VARIABLE_REAL34_DATA(&matrix->matrixElements[i * matrix->header.matrixColumns + j]));
      real34Copy(REGISTER_IMAG34_DATA(REGISTER_X), VARIABLE_IMAG34_DATA(&matrix->matrixElements[i * matrix->header.matrixColumns + j]));
    }
    else {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Cannot store %s in a matrix", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function storeElementReal:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    return true;
  }



  static bool_t storeIjReal(real34Matrix_t *matrix) {
    uint32_t rows, cols;
    if(getDimensionArg(&rows, &cols)) {
      if(rows > 0 && rows <= matrix->header.matrixRows && cols > 0 && cols <= matrix->header.matrixColumns) {
        copySourceRegisterToDestRegister(REGISTER_Y, REGISTER_I);
        copySourceRegisterToDestRegister(REGISTER_X, REGISTER_J);
        return true;
      }
      else {
        displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "(%" PRIu16 ", %" PRIu16 ") out of range", rows, cols);
          moreInfoOnError("In function storeIjReal:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
    }
    else if(lastErrorCode == ERROR_NONE){
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Cannot store %s as matrix index", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function storeIjReal:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    return false;
  }

  static bool_t storeIjComplex(complex34Matrix_t *matrix) {
    return storeIjReal((real34Matrix_t *)matrix);
  }
#endif // !TESTSUITE_BUILD



static void _storeValue(uint16_t regist) {
  if(regist == RESERVED_VARIABLE_UY || regist == RESERVED_VARIABLE_LY) {
    PLOT_ZMY = zoomOverride;  //PLOT EQN
  }
  if(regist == RESERVED_VARIABLE_GRAMOD) {
    copySourceRegisterToDestRegister(REGISTER_X, TEMP_REGISTER_1);
    fnLint(NOPARAM);
    if(lastErrorCode == ERROR_NONE) {
      longInteger_t x;
      convertLongIntegerRegisterToLongInteger(REGISTER_X, x);
      if(longIntegerCompareInt(x, 0) >= 0 && longIntegerCompareInt(x, 3) <= 0) {
        copySourceRegisterToDestRegister(REGISTER_X, regist);
        copySourceRegisterToDestRegister(TEMP_REGISTER_1, REGISTER_X);
      }
      else {
        displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function _storeValue:", "Invalid value for GRAMOD", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      longIntegerFree(x);
    }
  }
  else if(FIRST_RESERVED_VARIABLE <= regist && regist <= LAST_RESERVED_VARIABLE && allReservedVariables[regist - FIRST_RESERVED_VARIABLE].header.dataType == dtReal34) {
    copySourceRegisterToDestRegister(REGISTER_X, TEMP_REGISTER_1);
    fnToReal(NOPARAM);
    if(lastErrorCode == ERROR_NONE) {
      copySourceRegisterToDestRegister(REGISTER_X, regist);
      copySourceRegisterToDestRegister(TEMP_REGISTER_1, REGISTER_X);
    }
  }
  else {
    copySourceRegisterToDestRegister(REGISTER_X, regist);
  }
}

void fnStore(uint16_t regist) {
  if(_checkReadOnlyVariable(regist) && regInRange(regist)) {
    _storeValue(regist);
    uint16_t rows = 1;
    if(regist >= FIRST_NAMED_VARIABLE && regist == findNamedVariable("STATS")) {
      if(isStatsMatrixN(&rows, regist)) {
        calcSigma(0);
      } else {
        clearStatisticalSums();
      }
    }
  }
}


void fn2Sto(uint16_t regist) {
  if((/*regist >= FIRST_GLOBAL_REGISTER && */regist <= (REGISTER_X-1)-1) || (regist >= REGISTER_X && regist <= REGISTER_W-1) || (FIRST_LOCAL_REGISTER <= regist && regist < FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters - 1)) {
    setSystemFlag(FLAG_ASLIFT);
    copySourceRegisterToDestRegister(REGISTER_X, regist + 0);
    copySourceRegisterToDestRegister(REGISTER_Y, regist + 1);
  } else {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "%04d", regist);
      moreInfoOnError("In function fn2Sto:", errorMessage, " is not defined!", NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void fn3Sto(uint16_t regist) {
  if((/*regist >= FIRST_GLOBAL_REGISTER && */regist <= (REGISTER_X-1)-2) || (regist >= REGISTER_X && regist <= REGISTER_W-2) || (FIRST_LOCAL_REGISTER <= regist && regist < FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters - 2)) {
    setSystemFlag(FLAG_ASLIFT);
    copySourceRegisterToDestRegister(REGISTER_X, regist + 0);
    copySourceRegisterToDestRegister(REGISTER_Y, regist + 1);
    copySourceRegisterToDestRegister(REGISTER_Z, regist + 2);
  } else {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "%04d", regist);
      moreInfoOnError("In function fn3Sto:", errorMessage, " is not defined!", NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void fnStoreAdd(uint16_t regist) {
  if(_checkReadOnlyVariable(regist) && regInRange(regist)) {
    if(programRunStop == PGM_RUNNING) {
      copySourceRegisterToDestRegister(REGISTER_Y, SAVED_REGISTER_Y);
      copySourceRegisterToDestRegister(REGISTER_X, SAVED_REGISTER_X);
    }

    copySourceRegisterToDestRegister(regist, REGISTER_Y);
    if(getRegisterDataType(REGISTER_Y) == dtShortInteger) {
      *(REGISTER_SHORT_INTEGER_DATA(REGISTER_Y)) &= shortIntegerMask;
    }

    addition[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)]();

    copySourceRegisterToDestRegister(SAVED_REGISTER_Y, REGISTER_Y);
    _storeValue(regist);
    if(regist != REGISTER_X) {
      copySourceRegisterToDestRegister(SAVED_REGISTER_X, REGISTER_X);
    }

    adjustResult(REGISTER_X, false, true, REGISTER_X, regist, -1);
    uint16_t rows = 1;
    if(regist >= FIRST_NAMED_VARIABLE && isStatsMatrixN(&rows, regist) && regist == findNamedVariable("STATS")) {
      calcSigma(0);
    }
  }
}



void fnStoreSub(uint16_t regist) {
  if(_checkReadOnlyVariable(regist) && regInRange(regist)) {
    if(programRunStop == PGM_RUNNING) {
      copySourceRegisterToDestRegister(REGISTER_Y, SAVED_REGISTER_Y);
      copySourceRegisterToDestRegister(REGISTER_X, SAVED_REGISTER_X);
    }

    copySourceRegisterToDestRegister(regist, REGISTER_Y);
    if(getRegisterDataType(REGISTER_Y) == dtShortInteger) {
      *(REGISTER_SHORT_INTEGER_DATA(REGISTER_Y)) &= shortIntegerMask;
    }

    subtraction[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)]();

    copySourceRegisterToDestRegister(SAVED_REGISTER_Y, REGISTER_Y);
    _storeValue(regist);
    if(regist != REGISTER_X) {
      copySourceRegisterToDestRegister(SAVED_REGISTER_X, REGISTER_X);
    }

    adjustResult(REGISTER_X, false, true, REGISTER_X, regist, -1);
    uint16_t rows = 1;
    if(regist >= FIRST_NAMED_VARIABLE && isStatsMatrixN(&rows, regist) && regist == findNamedVariable("STATS")) {
      calcSigma(0);
    }
  }
}



void fnStoreMult(uint16_t regist) {
  if(_checkReadOnlyVariable(regist) && regInRange(regist)) {
    if(programRunStop == PGM_RUNNING) {
      copySourceRegisterToDestRegister(REGISTER_Y, SAVED_REGISTER_Y);
      copySourceRegisterToDestRegister(REGISTER_X, SAVED_REGISTER_X);
    }

    copySourceRegisterToDestRegister(regist, REGISTER_Y);
    if(getRegisterDataType(REGISTER_Y) == dtShortInteger) {
      *(REGISTER_SHORT_INTEGER_DATA(REGISTER_Y)) &= shortIntegerMask;
    }

    multiplication[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)]();

    copySourceRegisterToDestRegister(SAVED_REGISTER_Y, REGISTER_Y);
    _storeValue(regist);
    if(regist != REGISTER_X) {
      copySourceRegisterToDestRegister(SAVED_REGISTER_X, REGISTER_X);
    }

    adjustResult(REGISTER_X, false, true, REGISTER_X, regist, -1);
    uint16_t rows = 1;
    if(regist >= FIRST_NAMED_VARIABLE && isStatsMatrixN(&rows, regist) && regist == findNamedVariable("STATS")) {
      calcSigma(0);
    }
  }
}



void fnStoreDiv(uint16_t regist) {
  if(_checkReadOnlyVariable(regist) && regInRange(regist)) {
    if(programRunStop == PGM_RUNNING) {
      copySourceRegisterToDestRegister(REGISTER_Y, SAVED_REGISTER_Y);
      copySourceRegisterToDestRegister(REGISTER_X, SAVED_REGISTER_X);
    }

    copySourceRegisterToDestRegister(regist, REGISTER_Y);
    if(getRegisterDataType(REGISTER_Y) == dtShortInteger) {
      *(REGISTER_SHORT_INTEGER_DATA(REGISTER_Y)) &= shortIntegerMask;
    }

    division[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)]();

    copySourceRegisterToDestRegister(SAVED_REGISTER_Y, REGISTER_Y);
    _storeValue(regist);
    if(regist != REGISTER_X) {
      copySourceRegisterToDestRegister(SAVED_REGISTER_X, REGISTER_X);
    }

    adjustResult(REGISTER_X, false, true, REGISTER_X, regist, -1);
    uint16_t rows = 1;
    if(regist >= FIRST_NAMED_VARIABLE && isStatsMatrixN(&rows, regist) && regist == findNamedVariable("STATS")) {
      calcSigma(0);
    }
  }
}



void fnStoreMin(uint16_t regist) {
  if(_checkReadOnlyVariable(regist) && regInRange(regist)) {
    copySourceRegisterToDestRegister(REGISTER_X, SAVED_REGISTER_X);
    if(FIRST_RESERVED_VARIABLE <= regist && regist < LAST_RESERVED_VARIABLE && allReservedVariables[regist - FIRST_RESERVED_VARIABLE].header.pointerToRegisterData == C47_NULL) {
      copySourceRegisterToDestRegister(regist, TEMP_REGISTER_1);
      regist = TEMP_REGISTER_1;
    }
    else if(FIRST_RESERVED_VARIABLE <= regist && regist < LAST_RESERVED_VARIABLE && allReservedVariables[regist - FIRST_RESERVED_VARIABLE].header.pointerToRegisterData == C47_NULL) {
      fnToReal(NOPARAM);
      if(lastErrorCode == ERROR_NONE) {
        copySourceRegisterToDestRegister(regist, TEMP_REGISTER_1);
        regist = TEMP_REGISTER_1;
      }
    }
    registerMin(REGISTER_X, regist, regist);
    copySourceRegisterToDestRegister(SAVED_REGISTER_X, REGISTER_X);
  }
}



void fnStoreMax(uint16_t regist) {
  if(_checkReadOnlyVariable(regist) && regInRange(regist)) {
    copySourceRegisterToDestRegister(REGISTER_X, SAVED_REGISTER_X);
    if(FIRST_RESERVED_VARIABLE <= regist && regist < LAST_RESERVED_VARIABLE && allReservedVariables[regist - FIRST_RESERVED_VARIABLE].header.pointerToRegisterData == C47_NULL) {
      copySourceRegisterToDestRegister(regist, TEMP_REGISTER_1);
      regist = TEMP_REGISTER_1;
    }
    else if(FIRST_RESERVED_VARIABLE <= regist && regist < LAST_RESERVED_VARIABLE && allReservedVariables[regist - FIRST_RESERVED_VARIABLE].header.pointerToRegisterData == C47_NULL) {
      fnToReal(NOPARAM);
      if(lastErrorCode == ERROR_NONE) {
        copySourceRegisterToDestRegister(regist, TEMP_REGISTER_1);
        regist = TEMP_REGISTER_1;
      }
    }
    registerMax(REGISTER_X, regist, regist);
    copySourceRegisterToDestRegister(SAVED_REGISTER_X, REGISTER_X);
  }
}



void fnStoreConfig(uint16_t regist) {
    //uint8_t  compatibility_u8 = 0;             //defaults to use when settings are removed
  int16_t compatibility_int1  = 0;               //defaults to use when settings are removed
  bool_t compatibility_byte00 = false;           //defaults to use when settings are removed
  uint8_t compatibility_byte1 = 0;               //defaults to use when settings are removed
  bool_t compatibility_byte2  = false;           //defaults to use when settings are removed
  bool_t compatibility_byte3  = false;           //defaults to use when settings are removed
  bool_t compatibility_byte4  = false;           //defaults to use when settings are removed
  bool_t compatibility_byte5  = false;           //defaults to use when settings are removed
  bool_t compatibility_byte6  = false;           //defaults to use when settings are removed
  bool_t compatibility_byte7  = false;           //defaults to use when settings are removed
  bool_t compatibility_byte8  = false;           //defaults to use when settings are removed
  bool_t compatibility_byte9  = false;           //defaults to use when settings are removed
  bool_t compatibility_byte10 = false;           //defaults to use when settings are removed
  bool_t compatibility_byte11 = false;           //defaults to use when settings are removed
  bool_t compatibility_byte12 = false;           //defaults to use when settings are removed
  bool_t compatibility_byte13 = false;           //defaults to use when settings are removed
  bool_t compatibility_byte14 = false;           //defaults to use when settings are removed
  bool_t compatibility_byte15 = false;           //defaults to use when settings are removed
  bool_t compatibility_byte16 = false;           //defaults to use when settings are removed
  bool_t compatibility_byte17 = false;           //defaults to use when settings are removed
  bool_t compatibility_byte18 = false;           //defaults to use when settings are removed
  bool_t compatibility_byte19 = false;           //defaults to use when settings are removed
  bool_t compatibility_byte20 = false;           //defaults to use when settings are removed
  bool_t compatibility_byte21 = false;           //defaults to use when settings are removed
  bool_t compatibility_byte22 = false;           //defaults to use when settings are removed
  bool_t compatibility_byte23 = false;           //defaults to use when settings are removed
  bool_t compatibility_byte24 = false;           //defaults to use when settings are removed
  bool_t compatibility_byte25 = false;           //defaults to use when settings are removed
  bool_t compatibility_byte26 = false;           //defaults to use when settings are removed
  bool_t compatibility_byte27 = false;           //defaults to use when settings are removed
  bool_t compatibility_byte28 = false;           //defaults to use when settings are removed
  bool_t compatibility_byte29 = false;           //defaults to use when settings are removed
  bool_t compatibility_byte30 = false;           //defaults to use when settings are removed
  bool_t compatibility_byte31 = false;           //defaults to use when settings are removed
  float  compatibility_float1 = 0.1;             //defaults to use when settings are removed
  float  compatibility_float2 = 0.2;             //defaults to use when settings are removed
  reallocateRegister(regist, dtConfig, 0, amNone);
  dtConfigDescriptor_t *configToStore = REGISTER_CONFIG_DATA(regist);

  storeToDtConfigDescriptor(shortIntegerMode);
  storeToDtConfigDescriptor(shortIntegerWordSize);
  storeToDtConfigDescriptor(displayFormat);
  storeToDtConfigDescriptor(displayFormatDigits);
  storeToDtConfigDescriptor(gapItemLeft);
  storeToDtConfigDescriptor(gapItemRight);
  storeToDtConfigDescriptor(gapItemRadix);
  storeToDtConfigDescriptor(grpGroupingLeft);
  storeToDtConfigDescriptor(grpGroupingGr1LeftOverflow);
  storeToDtConfigDescriptor(grpGroupingGr1Left);
  storeToDtConfigDescriptor(grpGroupingRight);
  storeToDtConfigDescriptor(currentAngularMode);
  storeToDtConfigDescriptor(lrSelection);
  storeToDtConfigDescriptor(lrChosen);
  storeToDtConfigDescriptor(denMax);
  storeToDtConfigDescriptor(displayStack);
  storeToDtConfigDescriptor(firstGregorianDay);
  storeToDtConfigDescriptor(roundingMode);
  storeToDtConfigDescriptor(systemFlags0);
  storeToDtConfigDescriptor(systemFlags1);
  xcopy(configToStore->kbd_usr, kbd_usr, sizeof(kbd_usr));
  storeToDtConfigDescriptor(    compatibility_byte1);
  storeToDtConfigDescriptor(    compatibility_byte19);
  storeToDtConfigDescriptor(    compatibility_byte28);
  storeToDtConfigDescriptor(    compatibility_byte29);
  storeToDtConfigDescriptor(    compatibility_byte21);
  storeToDtConfigDescriptor(    compatibility_byte30);
  storeToDtConfigDescriptor(    compatibility_byte00);   //added
  storeToDtConfigDescriptor(    compatibility_int1);    //added
  storeToDtConfigDescriptor(Input_Default);
  storeToDtConfigDescriptor(dispBase);
  storeToDtConfigDescriptor(    compatibility_byte31);
  storeToDtConfigDescriptor(compatibility_byte26);
  storeToDtConfigDescriptor(compatibility_float1);
  storeToDtConfigDescriptor(compatibility_float2);
  storeToDtConfigDescriptor(Norm_Key_00.func);
  xcopy(configToStore->Norm_Key_00.funcParam, Norm_Key_00.funcParam, sizeof(Norm_Key_00.funcParam));
  storeToDtConfigDescriptor(Norm_Key_00.used);
  storeToDtConfigDescriptor(    compatibility_byte2);
  storeToDtConfigDescriptor(    compatibility_byte3);
  storeToDtConfigDescriptor(    compatibility_byte4);
  storeToDtConfigDescriptor(    compatibility_byte5);
  storeToDtConfigDescriptor(    compatibility_byte6);
  storeToDtConfigDescriptor(    compatibility_byte7);
  storeToDtConfigDescriptor(    compatibility_byte8);
  storeToDtConfigDescriptor(    compatibility_byte9);
  storeToDtConfigDescriptor(    compatibility_byte10);
  storeToDtConfigDescriptor(    compatibility_byte11);
  storeToDtConfigDescriptor(    compatibility_byte12);
  storeToDtConfigDescriptor(    compatibility_byte13);
  storeToDtConfigDescriptor(    compatibility_byte14);
  storeToDtConfigDescriptor(    compatibility_byte15);
  storeToDtConfigDescriptor(fractionDigits);
  storeToDtConfigDescriptor(    compatibility_byte23);
  storeToDtConfigDescriptor(    compatibility_byte16);
  storeToDtConfigDescriptor(    compatibility_byte20);
  storeToDtConfigDescriptor(    compatibility_byte17);
  storeToDtConfigDescriptor(IrFractionsCurrentStatus);
  storeToDtConfigDescriptor(    compatibility_byte18);
  storeToDtConfigDescriptor(displayStackSHOIDISP);
  storeToDtConfigDescriptor(    compatibility_byte25);
  storeToDtConfigDescriptor(    compatibility_byte24);
  storeToDtConfigDescriptor(bcdDisplaySign);
  storeToDtConfigDescriptor(DRG_Cycling);
  storeToDtConfigDescriptor(DM_Cycling);
  storeToDtConfigDescriptor(    compatibility_byte22);
  storeToDtConfigDescriptor(LongPressM);
  storeToDtConfigDescriptor(LongPressF);
  storeToDtConfigDescriptor(lastDenominator);
  storeToDtConfigDescriptor(significantDigits);
  storeToDtConfigDescriptor(pcg32_global);
  storeToDtConfigDescriptor(exponentLimit);
  storeToDtConfigDescriptor(exponentHideLimit);
  storeToDtConfigDescriptor(lastIntegerBase);
  storeToDtConfigDescriptor(compatibility_byte27);
  storeToDtConfigDescriptor(timeDisplayFormatDigits);
}



void fnStoreStack(uint16_t regist) {
  uint16_t size = getSystemFlag(FLAG_SSIZE8) ? 8 : 4;

  if(regist + size >= REGISTER_X && regist < REGISTER_X) {
    displayCalcErrorMessage(ERROR_STACK_CLASH, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Cannot execute STOS, destination register would overlap the stack: %d", regist);
      moreInfoOnError("In function fnStoreStack:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else if((regist >= REGISTER_X && regist < FIRST_LOCAL_REGISTER) || regist + size > FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Cannot execute STOS, destination register is out of range: %d", regist);
      moreInfoOnError("In function fnStoreStack:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    for(int i=0; i<size; i++) {
      copySourceRegisterToDestRegister(REGISTER_X + i, regist + i);
    }
  }
}


static void _fnStoreElement(bool_t stepForward);


void fnStoreVElement(uint16_t ix) {
  #if !defined(TESTSUITE_BUILD)
  const int16_t iBak = getIRegisterAsInt(true);
  const int16_t jBak = getJRegisterAsInt(true);
  real_t rx;

  if((getRegisterDataType(REGISTER_Y) == dtReal34Matrix) || (getRegisterDataType(REGISTER_Y) == dtComplex34Matrix)) {
    if(!getRegisterAsComplex(REGISTER_X, &rx, &rx) && !getRegisterAsReal(REGISTER_X, &rx)) {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      #if defined(PC_BUILD)
        sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType(REGISTER_X));
        moreInfoOnError("In function fnStoreVElement:", errorMessage, "is not a Real/Integer/Complex.", "");
      #endif
      return;
    }
    if(getRegisterDataType(REGISTER_Y) == dtReal34Matrix) {
      real34Matrix_t x;
      linkToRealMatrixRegister(REGISTER_Y, &x);
      setIRegisterAsInt(false, (ix-1) / x.header.matrixColumns+1);
      setJRegisterAsInt(false, (ix-1) % x.header.matrixColumns+1);
    }
    else { //Complex Matrix
      complex34Matrix_t x;
      linkToComplexMatrixRegister(REGISTER_Y, &x);
      setIRegisterAsInt(false, (ix-1) / x.header.matrixColumns+1);
      setJRegisterAsInt(false, (ix-1) % x.header.matrixColumns+1);
    }
    uint16_t matrixIndexBak = matrixIndex;
    matrixIndex = REGISTER_Y;
    _fnStoreElement(false);
    setIRegisterAsInt(false, iBak);
    setJRegisterAsInt(false, jBak);
    matrixIndex = matrixIndexBak;
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      #if defined(PC_BUILD)
    sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType(REGISTER_Y));
    moreInfoOnError("In function fnStoreVElement:", errorMessage, "is not a matrix.", "");
    #endif
  }
  #endif // !TESTSUITE_BUILD
}

void fnStoreElementPlus(uint16_t unusedButMandatoryParameter) {
  _fnStoreElement(true);
}

void fnStoreElement(uint16_t unusedButMandatoryParameter) {
  _fnStoreElement(false);
}

void _fnStoreElement(bool_t stepForward) {
  #if !defined(TESTSUITE_BUILD)
    if(matrixIndex == INVALID_VARIABLE) {
      displayCalcErrorMessage(ERROR_NO_MATRIX_INDEXED, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Cannot execute STOEL without a matrix indexed");
        moreInfoOnError("In function _fnStoreElement:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      if(regInRange(matrixIndex) && getRegisterDataType(matrixIndex) == dtReal34Matrix && getRegisterDataType(REGISTER_X) == dtComplex34) {
        // Real matrices turns to complex matrices by setting a complex element
        convertReal34MatrixRegisterToComplex34MatrixRegister(matrixIndex, matrixIndex);
      }
      callByIndexedMatrix(storeElementReal, storeElementComplex);
      if(stepForward) {
        fnIncDecJ(INC_FLAG);
      }
      uint16_t rows = 1;
      if(matrixIndex >= FIRST_NAMED_VARIABLE && isStatsMatrixN(&rows, matrixIndex) && matrixIndex == findNamedVariable("STATS")) {
        calcSigma(0);
      }
    }
  #endif // !TESTSUITE_BUILD
}



void fnStoreIJ(uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
    if(matrixIndex == INVALID_VARIABLE) {
      displayCalcErrorMessage(ERROR_NO_MATRIX_INDEXED, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Cannot execute STOIJ without a matrix indexed");
        moreInfoOnError("In function fnStoreIJ:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      callByIndexedMatrix(storeIjReal, storeIjComplex);
      uint16_t rows = 1;
      if(matrixIndex >= FIRST_NAMED_VARIABLE && isStatsMatrixN(&rows, matrixIndex) && matrixIndex == findNamedVariable("STATS")) {
        calcSigma(0);
      }
    }
  #endif // !TESTSUITE_BUILD
}
