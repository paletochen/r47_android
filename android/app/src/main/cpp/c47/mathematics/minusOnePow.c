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


static void setM1PowAxis(const real_t *magnitude, real_t *sector, cplx_t *res) {
  int32_t sectorIndex;

  if(realIsNegative(sector) && !realIsZero(sector)) {
    realAdd(sector, const_4, sector, &ctxtReal39);
  }
  sectorIndex = realToInt32C47(sector, NULL);

  realSetZero(&res->Real);
  realSetZero(&res->Imag);
  switch(sectorIndex) {
    case 0:
      realCopy(magnitude, &res->Real);
      break;
    case 1:
      realCopy(magnitude, &res->Imag);
      break;
    case 2:
      realCopy(magnitude, &res->Real);
      realChangeSign(&res->Real);
      break;
    case 3:
      realCopy(magnitude, &res->Imag);
      realChangeSign(&res->Imag);
      break;
    default:
      realSetNaN(&res->Real);
      realSetNaN(&res->Imag);
  }
}


static void m1PowReal(void) {
  cplx_t z;

  if(!getRegisterAsReal(REGISTER_X, &z.Real)) {
    return;
  }

  if(realIsInfinite(&z.Real) || realIsNaN(&z.Real)) {
    convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
    return;
  }

  setRegisterAngularMode(REGISTER_X, amNone);

  WP34S_Mod(&z.Real, const_2, &z.Real, &ctxtReal39);
  if(realIsZero(&z.Real)) {
    convertRealToResultRegister(const_1, REGISTER_X, amNone);
  }
  else if(realCompareAbsEqual(&z.Real, const_1)) {
    convertRealToResultRegister(const__1, REGISTER_X, amNone);
  }
  else { /* Complex result */
    fnSetFlag(FLAG_CPXRES);
    fnRefreshState();

    const angularMode_t savedAngularMode = currentAngularMode;
    currentAngularMode = amRadian;

    realMultiply(&z.Real, const_2, &z.Imag, &ctxtReal39);
    if(realIsAnInteger(&z.Imag)) {
      setM1PowAxis(const_1, &z.Imag, &z);
    }
    else {
      realMultiply(&z.Real, const39_pi, &z.Real, &ctxtReal39);
      realSetZero(&z.Imag);
      eulersFormula(&z, &z, &ctxtReal39);
    }

    convertComplexToResultRegister(&z, REGISTER_X);
    currentAngularMode = savedAngularMode;
    return;
  }
}



static void m1PowCplx(void) {
  real_t sector, magnitude;
  cplx_t z;

  fnSetFlag(FLAG_CPXRES);
  fnRefreshState();
  setRegisterAngularMode(REGISTER_X, amNone);

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &z.Real);
  WP34S_Mod(&z.Real, const_2, &z.Real, &ctxtReal39);

  real34ToReal(REGISTER_IMAG34_DATA(REGISTER_X), &z.Imag);
  if(realIsZero(&z.Imag)) {
    if(realIsZero(&z.Real)) {
      convertComplexPartsToResultRegister(const_1, const_0, REGISTER_X);
      return;
    }
    else if(realCompareAbsEqual(&z.Real, const_1)) {
      convertComplexPartsToResultRegister(const__1, const_0, REGISTER_X);
      return;
    }
  }

  if(!realIsNaN(&z.Real) && !realIsNaN(&z.Imag)) {
    realMultiply(&z.Real, const_2, &sector, &ctxtReal39);
    if(realIsAnInteger(&sector)) {
      realMultiply(&z.Imag, const75_pi, &magnitude, &ctxtReal51);
      realChangeSign(&magnitude);
      realExp(&magnitude, &magnitude, &ctxtReal51);
      setM1PowAxis(&magnitude, &sector, &z);
      convertComplexToResultRegister(&z, REGISTER_X);
      return;
    }
  }

  angularMode_t savedAngularMode = currentAngularMode;
  currentAngularMode = amRadian;

  mulComplexReal(&z, const75_pi, &z, &ctxtReal51);
  eulersFormula(&z, &z, &ctxtReal51);

  convertComplexToResultRegister(&z, REGISTER_X);
  currentAngularMode = savedAngularMode;
}


void fnM1Pow(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexMonadicFunction(&m1PowReal, &m1PowCplx, &m1PowShoI, &m1PowLonI);
}
