// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

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
    displayCalcErrorMessage(ERROR_EMPTY_STRING, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot use " STD_alpha STD_RIGHT_ARROW "x on an empty string");
      moreInfoOnError("In function fnAlphaToX:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
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



void fnXToAlpha(uint16_t unusedButMandatoryParameter) {
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

  if(getRegisterDataType(REGISTER_X) != dtString) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot use " STD_alpha "POS? on %s (reg X)", getRegisterDataTypeName(regist, true, false));
      moreInfoOnError("In function fnAlphaPos:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(stringGlyphLength(REGISTER_STRING_DATA(regist)) == 0 || stringGlyphLength(REGISTER_STRING_DATA(REGISTER_X)) == 0) {
    displayCalcErrorMessage(ERROR_EMPTY_STRING, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot use " STD_alpha "POS? on or with an empty string");
      moreInfoOnError("In function fnAlphaPos:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(!saveLastX()) {
    return;
  }

  longIntegerInit(lgInt);
  ptrTarget = REGISTER_STRING_DATA(REGISTER_X);
  ptrRegist = REGISTER_STRING_DATA(regist);
  lgTarget = stringByteLength(ptrTarget);
  lgRegist = stringByteLength(ptrRegist);

  int32ToLongInteger(-1, lgInt);
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
    displayCalcErrorMessage(ERROR_EMPTY_STRING, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot use " STD_alpha "RR on an empty string");
      moreInfoOnError("In function fnAlphaRR:", errorMessage, NULL, NULL);
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
    xcopy(tmpString + stringByteLength(tmpString), REGISTER_STRING_DATA(regist), stringByteLength(REGISTER_STRING_DATA(regist)) + 1);
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
    displayCalcErrorMessage(ERROR_EMPTY_STRING, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot use " STD_alpha "RL on an empty string");
      moreInfoOnError("In function fnAlphaRL:", errorMessage, NULL, NULL);
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
    xcopy(tmpString + stringByteLength(tmpString), REGISTER_STRING_DATA(regist), stringByteLength(REGISTER_STRING_DATA(regist)) + 1);
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
    displayCalcErrorMessage(ERROR_EMPTY_STRING, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot use " STD_alpha "SR on an empty string");
      moreInfoOnError("In function fnAlphaSR:", errorMessage, NULL, NULL);
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
    displayCalcErrorMessage(ERROR_EMPTY_STRING, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot use " STD_alpha "SL on an empty string");
      moreInfoOnError("In function fnAlphaSL:", errorMessage, NULL, NULL);
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
