// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

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
  }
  else {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
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
  }
  else {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
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



// Same fold as the store variants: one body, four dispatch tables.
static void _recallOp(uint16_t regist, void (* const op[NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS][NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS])(void)) {
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

    op[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)]();

    copySourceRegisterToDestRegister(SAVED_REGISTER_Y, REGISTER_Y);

    adjustResult(REGISTER_X, false, true, REGISTER_X, regist, -1);
  }
}



void fnRecallAdd(uint16_t regist) {
  _recallOp(regist, addition);
}



void fnRecallSub(uint16_t regist) {
  _recallOp(regist, subtraction);
}



void fnRecallMult(uint16_t regist) {
  _recallOp(regist, multiplication);
}



void fnRecallDiv(uint16_t regist) {
  _recallOp(regist, division);
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
    shortIntegerWordSize = boundShortIntegerWordSize(shortIntegerWordSize);
    updateShortIntegerMasks();  // rederive shortIntegerMask and shortIntegerSignBit from the recalled word size; the config descriptor stores the size but neither mask
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
    if(roundingMode > RM_FLOOR) {
      roundingMode = RM_HALF_EVEN;
    }
    setRoundingMode(roundingMode);
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
    set_def_Input_Default();
    recallFromDtConfigDescriptor(dispBase);
    recallFromDtConfigDescriptor(compatibility_byte31);
    recallFromDtConfigDescriptor(compatibility_byte26);
    recallFromDtConfigDescriptor(compatibility_float1);   //spare
    recallFromDtConfigDescriptor(compatibility_float2);   //spare
    recallFromDtConfigDescriptor(Norm_Key_00.func);
    xcopy(Norm_Key_00.funcParam, configToRecall->Norm_Key_00.funcParam, sizeof(Norm_Key_00.funcParam));
    {                                                                         // The descriptor is a raw byte image, so a crafted register can arrive unterminated. Terminating at
      size_t cut = 0;                                                         //   a fixed offset would leave the lead byte of a two byte glyph as the last byte, which every
      while(cut < sizeof(Norm_Key_00.funcParam) && Norm_Key_00.funcParam[cut] != 0) {   // consumer of the name would then read as a glyph one byte past the string. Walk the
        size_t next = cut + ((Norm_Key_00.funcParam[cut] & 0x80) ? 2 : 1);    //   glyphs instead and cut at the last boundary the field can hold, terminator included.
        if(next >= sizeof(Norm_Key_00.funcParam) || Norm_Key_00.funcParam[next - 1] == 0) {   // the second byte of a two byte glyph can itself be the terminator, and
          break;                                                              //   stepping over it would leave that glyph's lead byte as the last byte of the string
        }
        cut = next;
      }
      Norm_Key_00.funcParam[cut] = 0;                                         // a descriptor that carries its own terminator inside the field is unchanged
    }
    recallFromDtConfigDescriptor(Norm_Key_00.used);
    recallFromDtConfigDescriptor(grpGroupingHex);
    recallFromDtConfigDescriptor(grpGroupingBin);
    grpGroupingHexBinDefault();
    recallFromDtConfigDescriptor(graMod);
    if(graMod > 4) {
      graMod = 0;
    }
    #if defined(OPTION_ATEXT_FONTS)
      recallFromDtConfigDescriptor(graFont);
      graFontCheck();
    #endif // OPTION_ATEXT_FONTS
    recallFromDtConfigDescriptor(    compatibility_byte6);
    recallFromDtConfigDescriptor(    compatibility_byte7);
    recallFromDtConfigDescriptor(    compatibility_byte8);
    recallFromDtConfigDescriptor(    compatibility_byte9);
    recallFromDtConfigDescriptor(    compatibility_byte10);
    recallFromDtConfigDescriptor(    compatibility_byte11);
    recallFromDtConfigDescriptor(    compatibility_byte12);
    recallFromDtConfigDescriptor(    compatibility_byte13);
    displayRoundingMode = configToRecall->displayRoundingMode ^ DRM_DFLT;
    if(displayRoundingMode > RM_FLOOR) {
      displayRoundingMode = DRM_DFLT;
    }
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
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "data type %s cannot be used to recall a configuration!", getRegisterDataTypeName(regist, false, false));
      moreInfoOnError("In function fnRecallConfig:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  forceSBupdate();
}



void fnRecallStack(uint16_t regist) {
  uint16_t size = getSystemFlag(FLAG_SSIZE8) ? 8 : 4;

  if(REGISTER_X - size <= regist && regist < REGISTER_X) {
    displayCalcErrorMessage(ERROR_STACK_CLASH, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Cannot execute RCLS, destination register would overlap the stack: %d", regist);
      moreInfoOnError("In function fnRecallStack:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else if((REGISTER_X <= regist && regist < FIRST_LOCAL_REGISTER) || regist + size > FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
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
  uint16_t rows, cols;
  if(getMatrixDims(REGISTER_X, "In function fnRecallVElement:", &rows, &cols)) {
    matrixIndexState_t bak;
    saveMatrixIndexState(&bak);
    setIRegisterAsInt(false, (ix-1) / cols + 1);
    setJRegisterAsInt(false, (ix-1) % cols + 1);
    matrixIndex = REGISTER_X;
    _fnRecallElement(false);
    restoreMatrixIndexState(&bak);
  }
}

static void _elementToRegister(uint16_t ix, calcRegister_t r) {               // element ix of the vector in X into register r, copied out before r is reallocated
  if(getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
    complex34_t c;
    complex34Copy(REGISTER_COMPLEX34_MATRIX_ELEMENTS(REGISTER_X) + ix - 1, &c);
    reallocateRegister(r, dtComplex34, COMPLEX34_SIZE_IN_BLOCKS, amNone);
    complex34Copy(&c, REGISTER_COMPLEX34_DATA(r));
  }
  else {
    real34_t v;
    real34Copy(REGISTER_REAL34_MATRIX_ELEMENTS(REGISTER_X) + ix - 1, &v);
    reallocateRegister(r, dtReal34, REAL34_SIZE_IN_BLOCKS, amNone);
    real34Copy(&v, REGISTER_REAL34_DATA(r));
  }
}

void fnRecallVector(uint16_t   regist) {
  uint16_t rows, cols;
  if(!getMatrixDims(REGISTER_X, "In function fnRecallVector:", &rows, &cols)) {
    return;
  }
  bool_t stack = (regist >= REGISTER_X && regist <= REGISTER_D);             // a stack target is written top down, the order the stack is shown in; X, if in the span, comes last
  if(!vectorSpanOk(regist, rows * cols, "In function fnRecallVector:")) {
    return;
  }
  for(uint16_t ix = 1; ix <= rows * cols; ix++) {  //for 5x5, from 1 to 25
    calcRegister_t r = stack ? regist + rows * cols - ix : regist + ix - 1;
    if(!regInRange(r)) {
      return;
    }
    _elementToRegister(ix, r);
  }
}

void fnRecallElementPlus(uint16_t unusedButMandatoryParameter) {
  _fnRecallElement(true);
}

void fnRecallElement(uint16_t unusedButMandatoryParameter) {
  _fnRecallElement(false);
}

void _fnRecallElement(bool_t stepForward) {
  if(matrixIndex == INVALID_VARIABLE) {
    displayCalcErrorMessage(ERROR_NO_MATRIX_INDEXED, ERR_REGISTER_LINE);
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
}


void fnRecallIJ(uint16_t unusedButMandatoryParameter) {
    if(matrixIndex == INVALID_VARIABLE) {
      displayCalcErrorMessage(ERROR_NO_MATRIX_INDEXED, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Cannot execute RCLIJ without a matrix indexed");
        moreInfoOnError("In function fnRecallIJ:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      longInteger_t zero;
      longIntegerInit(zero);

      if(!saveLastX()) {
        longIntegerFree(zero);
        return;
      }

      liftStack();
      setSystemFlag(FLAG_ASLIFT);
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
}

void fn42AlphaRecall(uint16_t regist) {
  if(regInRange(regist)) {
    if(getRegisterDataType(alphaRegister) == dtString) {
      if(programRunStop == PGM_RUNNING) {
        copySourceRegisterToDestRegister(REGISTER_Y, SAVED_REGISTER_Y);
        copySourceRegisterToDestRegister(REGISTER_X, SAVED_REGISTER_X);
      }

      copySourceRegisterToDestRegister(regist, REGISTER_X);
      copySourceRegisterToDestRegister(alphaRegister, REGISTER_Y);

      if(getRegisterDataType(REGISTER_X) == dtShortInteger) {
        *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) &= shortIntegerMask;
      }

      addition[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)]();

      copySourceRegisterToDestRegister(REGISTER_X, alphaRegister);
      truncateAlphaRegisterTo44Char();

      copySourceRegisterToDestRegister(SAVED_REGISTER_Y, REGISTER_Y);
      copySourceRegisterToDestRegister(SAVED_REGISTER_X, REGISTER_X);
    }
    else {
      displayCalcErrorMessage(ERROR_NO_STRING_IN_ALPHA_REGISTER, ERR_REGISTER_LINE);
    }
  }
}
