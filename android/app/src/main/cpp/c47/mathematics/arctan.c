// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file arctan.c
 ***********************************************/

#include "c47.h"

static void arctanReal(void) {
  real_t x;

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }

  if(realIsInfinite(&x)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      if(realIsNegative(&x)) {
        realMinus(const_90, &x, &ctxtReal39);
      }
      else {
        realCopy(const_90, &x);
      }
      convertAngleFromTo(&x, amDegree, currentAngularMode, &ctxtReal39);
  }
  else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function arctanReal:", "X = " STD_PLUS_MINUS STD_INFINITY, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  else if(currentAngularMode == amRadian && !realIsZero(&x) && realGetExponent(&x) < -18) {   // arctan x = x - x³/3 to 72 digits, below |x| by less than half a unit at 34 digits
    WP34S_TinyOddSeries(&x, const_3, false, &x, &ctxtReal75);
  }
  else if(currentAngularMode != amRadian && realCompareAbsGreaterThan(&x, const_1)) {   // ±90° less arctan(1/x): the right angle is exact outside radians
    realDivide(const_1, &x, &x, &ctxtReal39);
    C47_WP34S_Atan(&x, &x, &ctxtReal39);
    rightAngleLessRadians(&x, realIsNegative(&x));
  }
  else {
    C47_WP34S_Atan(&x, &x, &ctxtReal39);
    convertAngleFromTo(&x, amRadian, currentAngularMode, &ctxtReal39);
  }
  convertRealToResultRegister(&x, REGISTER_X, currentAngularMode);
}



static void arctanCplx(void) {
  cplx_t x, r;

  if(!getRegisterAsComplex(REGISTER_X, &x)) {
    return;
  }

  ArctanComplex(&x, &r, &ctxtReal39);

  convertComplexToResultRegister(&r, REGISTER_X);
}

uint8_t ArctanComplex(cplx_t *x, cplx_t *r, realContext_t *realContext) {
  real_t aSquared, denominator, numerator;
  cplx_t w;

  realCopy(&x->Real, &w.Real);
  realCopy(&x->Imag, &w.Imag);

  if(realIsNaN(&w.Real) || realIsNaN(&w.Imag)) {
    realSetNaN(&r->Real);
    realSetNaN(&r->Imag);
    return ERROR_NONE;
  }

  if(realIsInfinite(&w.Real) || realIsInfinite(&w.Imag)) {
    C47_WP34S_Atan2(const_1, const_0, &r->Real, realContext);
    if(realIsNegative(&w.Real)) {
      realSetNegativeSign(&r->Real);
    }
    realSetZero(&r->Imag);
    if(realIsNegative(&w.Imag)) {
      realSetNegativeSign(&r->Imag);
    }
    return ERROR_NONE;
  }

  // Re(atan(a+bi)) = atan2(2a, (1-b)(1+b)-a²)/2.
  realSubtract(const_1, &w.Imag, &denominator, realContext);
  realAdd(const_1, &w.Imag, &numerator, realContext);
  realMultiply(&denominator, &numerator, &denominator, realContext);
  realMinus(&w.Real, &numerator, realContext);
  realFMA(&numerator, &w.Real, &denominator, &denominator, realContext);
  realMultiply(&w.Real, const_2, &numerator, realContext);
  C47_WP34S_Atan2(&numerator, &denominator, &r->Real, realContext);
  realMultiply(&r->Real, const_1on2, &r->Real, realContext);

  // Use a positive ln1p argument on either side of the real axis:
  // Im(atan(a+bi)) = sign(b)/4 * ln(1 + 4|b|/(a²+(|b|-1)²)).
  realMultiply(&w.Real, &w.Real, &aSquared, realContext);
  if(realIsNegative(&w.Imag)) {
    realAdd(&w.Imag, const_1, &denominator, realContext);
    realMultiply(&w.Imag, const__4, &numerator, realContext);
  } else {
    realSubtract(&w.Imag, const_1, &denominator, realContext);
    realMultiply(&w.Imag, const_4, &numerator, realContext);
  }
  realFMA(&denominator, &denominator, &aSquared, &denominator, realContext);
  realDivide(&numerator, &denominator, &numerator, realContext);
  WP34S_Ln1P(&numerator, &r->Imag, realContext);
  realMultiply(&r->Imag, const_1on2, &r->Imag, realContext);
  realMultiply(&r->Imag, const_1on2, &r->Imag, realContext);
  if(realIsNegative(&w.Imag)) {
    realSetNegativeSign(&r->Imag);
  }

  return ERROR_NONE;
}



/********************************************//**
 * \brief regX ==> regL and arctan(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnArctan(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&arctanReal, &arctanCplx);
}
