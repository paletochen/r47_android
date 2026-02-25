// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file minusOnePow.c
 ***********************************************/

#include "c47.h"


/**********************************************************************
 * In all the functions below:
 * if X is a number then X = a + ib
 * The variables a and b are used for intermediate calculations
 ***********************************************************************/

static void m1PowLonI(void) {
  longInteger_t lgInt, exponent;

  longIntegerInit(lgInt);
  uInt32ToLongInteger(1u, lgInt);

  convertLongIntegerRegisterToLongInteger(REGISTER_X, exponent);
  if(longIntegerIsOdd(exponent)) {
    longIntegerChangeSign(lgInt);
  }

  convertLongIntegerToLongIntegerRegister(lgInt, REGISTER_X);

  longIntegerFree(lgInt);
  longIntegerFree(exponent);
}



static void m1PowShoI(void) {
  int32_t signExponent;
  uint64_t valueExponent = WP34S_extract_value(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)), &signExponent);
  int32_t odd = valueExponent & 1;

  if(shortIntegerMode == SIM_UNSIGN && odd) {
    setSystemFlag(FLAG_OVERFLOW);
  }
  else {
    clearSystemFlag(FLAG_OVERFLOW);
  }

  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_build_value((uint64_t)1, odd);
}

static void m1PowReal(void) {
  real_t x, y;

  if (!getRegisterAsReal(REGISTER_X, &x))
    return;
  if(realIsInfinite(&x) || realIsNaN(&x)) {
    convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
    return;
  }

  setRegisterAngularMode(REGISTER_X, amNone);

  WP34S_Mod(&x, const_2, &x, &ctxtReal39);
  if(realIsZero(&x)) {
    convertRealToResultRegister(const_1, REGISTER_X, amNone);
  }
  else if(realCompareEqual(&x, const_1)) {
    convertRealToResultRegister(const__1, REGISTER_X, amNone);
  }
  else { /* Complex result */
    fnSetFlag(FLAG_CPXRES);
    fnRefreshState();

    const angularMode_t savedAngularMode = currentAngularMode;
    currentAngularMode = amRadian;

    realMultiply(&x, const_pi, &x, &ctxtReal75);
    eulersFormula(&x, const_0, &x, &y, &ctxtReal39);

    convertComplexToResultRegister(&x, &y, REGISTER_X);
    currentAngularMode = savedAngularMode;
    return;
  }
}



static void m1PowCplx(void) {
  real_t zReal, zImag;

  fnSetFlag(FLAG_CPXRES);
  fnRefreshState();
  setRegisterAngularMode(REGISTER_X, amNone);

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &zReal);
  WP34S_Mod(&zReal, const_2, &zReal, &ctxtReal39);

  real34ToReal(REGISTER_IMAG34_DATA(REGISTER_X), &zImag);
  if(realIsZero(&zImag)) {
    if(realIsZero(&zReal)) {
      convertComplexToResultRegister(const_1, const_0, REGISTER_X);
      return;
    }
    else if(realCompareEqual(&zReal, const_1)) {
      convertComplexToResultRegister(const__1, const_0, REGISTER_X);
      return;
    }
  }
  angularMode_t savedAngularMode = currentAngularMode;
  currentAngularMode = amRadian;

  mulComplexReal(&zReal, &zImag, const_pi, &zReal, &zImag, &ctxtReal75);
  eulersFormula(&zReal, &zImag, &zReal, &zImag, &ctxtReal75);

  convertComplexToResultRegister(&zReal, &zImag, REGISTER_X);
  currentAngularMode = savedAngularMode;
}


void fnM1Pow(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexMonadicFunction(&m1PowReal, &m1PowCplx, &m1PowShoI, &m1PowLonI);
}
