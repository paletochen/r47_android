// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"


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
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
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
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
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
      shortIntegerToDisplayString(regist, tmpString, false, noBaseOverride);
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
  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      convertLongIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    case dtReal34: {
      if(real34CompareAbsGreaterThan(REGISTER_REAL34_DATA(REGISTER_X), const34_1e6)) {
        uInt32ToLongInteger(1000000, lgInt);
      }
      else {
        convertReal34ToLongInteger(REGISTER_REAL34_DATA(REGISTER_X), lgInt, DEC_ROUND_DOWN);
      }
      break;
    }

    case dtShortInteger: {
      convertShortIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    case dtReal34Matrix: {
      convertShortIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
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
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
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

  if (regist != REGISTER_X) {
    _readDestinationRegister(regist);
  }
  else {
    tmpString[0] = 0;      // If destination register is X just return the alpha character from the character code
  }

  if(stringGlyphLength(tmpString) >= MAX_NUMBER_OF_GLYPHS_IN_STRING) {
    displayCalcErrorMessage(ERROR_STRING_WOULD_BE_TOO_LONG, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "the resulting string would be %d characters long. Maximum is %d", stringGlyphLength(tmpString) + 1, MAX_NUMBER_OF_GLYPHS_IN_STRING);
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
      if(stringGlyphLength(tmpString) + stringGlyphLength(REGISTER_STRING_DATA(REGISTER_X)) > MAX_NUMBER_OF_GLYPHS_IN_STRING) {
        displayCalcErrorMessage(ERROR_STRING_WOULD_BE_TOO_LONG, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "the resulting string would be %d (%d + %d) characters long. Maximum is %d",
            stringGlyphLength(tmpString) + stringGlyphLength(REGISTER_STRING_DATA(REGISTER_X)),
            stringGlyphLength(tmpString),
            stringGlyphLength(REGISTER_STRING_DATA(REGISTER_X)), MAX_NUMBER_OF_GLYPHS_IN_STRING);
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
      if (regist != REGISTER_X) {
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
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
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
  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      convertLongIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    case dtReal34: {
      if(real34CompareAbsGreaterThan(REGISTER_REAL34_DATA(REGISTER_X), const34_1e6)) {
        uInt32ToLongInteger(1000000u, lgInt);
      }
      else {
        convertReal34ToLongInteger(REGISTER_REAL34_DATA(REGISTER_X), lgInt, DEC_ROUND_DOWN);
      }
      break;
    }

    case dtShortInteger: {
      convertShortIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
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
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "for x" STD_RIGHT_ARROW STD_alpha ", X must be < 32768. Here X = %" PRIu32, (uint32_t)lgInt->_mp_d[0]); // OK for 32 and 64 bit limbs
      moreInfoOnError("In function fnXToAlpha:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
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
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
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



void fnAlphaRR(uint16_t regist) {
  longInteger_t lgInt;
  real_t real, mod;
  int16_t stringGlyphLen, steps, glyphPointer;
  char *ptr;

  if(getRegisterDataType(regist) != dtString) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
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
  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      convertLongIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    case dtReal34: {
      if(stringGlyphLen == 0) {
        lgInt->_mp_size = 0; // lgInt = 0
      }
      else {
        real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &real);
        realSetPositiveSign(&real);
        int32ToReal(stringGlyphLen, &mod);
        WP34S_Mod(&real, &mod, &real, &ctxtReal39);
        convertRealToLongInteger(&real, lgInt, DEC_ROUND_DOWN);
      }
      break;
    }

    case dtShortInteger: {
      convertShortIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
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
  real_t real, mod;
  int16_t stringGlyphLen, steps, glyphPointer;
  char *ptr;

  if(getRegisterDataType(regist) != dtString) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
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
  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      convertLongIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    case dtReal34: {
      if(stringGlyphLen == 0) {
        lgInt->_mp_size = 0; // lgInt = 0
      }
      else {
        real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &real);
        realSetPositiveSign(&real);
        int32ToReal(stringGlyphLen, &mod);
        WP34S_Mod(&real, &mod, &real, &ctxtReal39);
        convertRealToLongInteger(&real, lgInt, DEC_ROUND_DOWN);
      }
      break;
    }

    case dtShortInteger: {
      convertShortIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
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
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
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
  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      convertLongIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    case dtReal34: {
      if(real34CompareAbsGreaterThan(REGISTER_REAL34_DATA(REGISTER_X), const34_1e6)) {
        uInt32ToLongInteger(1000000u, lgInt);
      }
      else {
        convertReal34ToLongInteger(REGISTER_REAL34_DATA(REGISTER_X), lgInt, DEC_ROUND_DOWN);
      }
      break;
    }

    case dtShortInteger: {
      convertShortIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
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
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
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
  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      convertLongIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    case dtReal34: {
      if(real34CompareAbsGreaterThan(REGISTER_REAL34_DATA(REGISTER_X), const34_1e6)) {
        uInt32ToLongInteger(1000000u, lgInt);
      }
      else {
        convertReal34ToLongInteger(REGISTER_REAL34_DATA(REGISTER_X), lgInt, DEC_ROUND_DOWN);
      }
      break;
    }

    case dtShortInteger: {
      convertShortIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
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
    displayCalcErrorMessage(ERROR_NO_STRING_IN_ALPHA_REGISTER, ERR_REGISTER_LINE, REGISTER_T);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot use 42AROT on %s", getRegisterDataTypeName(alphaRegister, true, false));
      moreInfoOnError("In function fn42AlphaRotate:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  longIntegerInit(lgInt);
  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      convertLongIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    case dtReal34: {
      convertReal34ToLongInteger(REGISTER_REAL34_DATA(REGISTER_X), lgInt, DEC_ROUND_DOWN);
      break;
    }

    case dtShortInteger: {
      convertShortIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
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
    displayCalcErrorMessage(ERROR_NO_STRING_IN_ALPHA_REGISTER, ERR_REGISTER_LINE, REGISTER_T);
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
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
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

    copySourceRegisterToDestRegister(REGISTER_X, regist);
  }

  // restore, X, Y and L
  if((regist != REGISTER_Y) || (lastErrorCode != ERROR_NONE)) {
    copySourceRegisterToDestRegister(SAVED_REGISTER_Y, REGISTER_Y);  // Restore register Y
  }
  copySourceRegisterToDestRegister(SAVED_REGISTER_X, REGISTER_X);    // Restore register X
  copySourceRegisterToDestRegister(SAVED_REGISTER_L, REGISTER_L);    // Restore register L
}


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
