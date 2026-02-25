// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file exp.c
 ***********************************************/

#include "c47.h"

bool_t realExpLimitCheck(const real_t *x, real_t *res, const real_t *zero) {
  if(realIsSpecial(x)) {
    if(realIsInfinite(x)) {
inf:  if(realIsPositive(x)) {
        realCopy(const_plusInfinity, res);
      }
      else {
        realCopy(zero, res);
      }
    }
    else {
      realCopy(const_NaN, res);
    }
    return false;
  }
  if(realCompareAbsGreaterThan(x, const_2e6))
    goto inf;
  return true;
}

void realExp(const real_t *x, real_t *res, realContext_t *set) {
  if(realExpLimitCheck(x, res, const_0))
    decNumberExp(res, x, set);
}

void expComplex(const real_t *real, const real_t *imag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  real_t expa, sin, cos;

  if(realIsZero(imag)) {
   realExp(real, resReal, realContext);
   realZero(resImag);
   return;
  }

  if(realIsSpecial(real) || realIsSpecial(imag)) {
    realCopy(const_NaN, resReal);
    realCopy(const_NaN, resImag);
    return;
  }

 realExp(real, &expa, realContext);
 WP34S_Cvt2RadSinCosTan(imag, amRadian, &sin, &cos, NULL, realContext);
 realMultiply(&expa, &cos, resReal, realContext);
 realMultiply(&expa, &sin, resImag, realContext);
}

static void expReal(void) {
  real_t x;

  if(!getRegisterAsReal(REGISTER_X, &x))
    return;

  if(realIsInfinite(&x) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function expReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of exp when flag D is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  realExp(&x, &x, &ctxtReal39);
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}



static void expCplx(void) {
  real_t zReal, zImag;

  if(getRegisterAsComplex(REGISTER_X, &zReal, &zImag)) {
    expComplex(&zReal, &zImag, &zReal, &zImag, &ctxtReal39);
    convertComplexToResultRegister(&zReal, &zImag, REGISTER_X);
  }
}


/********************************************//**
 * \brief regX ==> regL and exp(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnExp(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&expReal, &expCplx);
}
