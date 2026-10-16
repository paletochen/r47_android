// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file tan.c
 ***********************************************/

#include "c47.h"

static void tanReal(void) {
  real_t sin, cos, tan;
  const real_t *r = &tan;
  angularMode_t xAngularMode;

  if(!getRegisterAsRealAngle(REGISTER_X, &tan, &xAngularMode, ifLongIntegerDoAngleReduction)) {
    return;
  }

  if(realIsSpecial(&tan)) {
    r = const_NaN;
  }
  else if(xAngularMode == amRadian && !realIsZero(&tan) && realGetExponent(&tan) < -18) {   // tan x = x + x³/3 to 72 digits, above |x| by less than half a unit at 34 digits
    WP34S_TinyOddSeries(&tan, const_3, true, &tan, &ctxtReal75);
  }
  else {
    C47_WP34S_Cvt2RadSinCosTan(&tan, xAngularMode, &sin, &cos, &tan, &ctxtReal34);
    if(realIsZero(&sin)) {
       realSetPositiveSign(&tan);
    }

    if(realIsZero(&cos) && !getSystemFlag(FLAG_SPCRES)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function tanReal:", "X = " STD_PLUS_MINUS "90" STD_DEGREE, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
    else {
      if(realIsZero(&cos)) {
        r = const_NaN;
      }
    }
  }
  convertRealToResultRegister(r, REGISTER_X, amNone);
}



static void tanCplx(void) {
  //                sin(a)*cosh(b) + i*cos(a)*sinh(b)
  // tan(a + ib) = -----------------------------------
  //                cos(a)*cosh(b) - i*sin(a)*sinh(b)

  cplx_t x;

  if(!getRegisterAsComplex(REGISTER_X, &x)) {
    return;
  }

  TanComplex(&x, &x, &ctxtReal39);

  convertComplexToResultRegister(&x, REGISTER_X);
}

uint8_t TanComplex(const cplx_t *x, cplx_t *r, realContext_t *realContext) {
  real_t sina, cosa, expTerm, oneMinusExp, onePlusExp;
  real_t twiceImag, denominator, temporary;
  cplx_t numer;

  C47_WP34S_Cvt2RadSinCosTan(&x->Real, amRadian, &sina, &cosa, NULL, realContext);
  if(realIsZero(&x->Imag)) {
    realDivide(&sina, &cosa, &r->Real, realContext);
    realCopy(&x->Imag, &r->Imag);
    return ERROR_NONE;
  }

  realCopyAbs(&x->Imag, &twiceImag);
  realAdd(&twiceImag, &twiceImag, &twiceImag, realContext);
  realSetNegativeSign(&twiceImag);

  realExp(&twiceImag, &expTerm, realContext);
  WP34S_ExpM1(&twiceImag, &oneMinusExp, realContext);
  realChangeSign(&oneMinusExp);
  realAdd(&expTerm, const_1, &onePlusExp, realContext);

  realMultiply(&cosa, &cosa, &denominator, realContext);
  realMultiply(&denominator, const_4, &denominator, realContext);
  realMultiply(&denominator, &expTerm, &denominator, realContext);
  realMultiply(&oneMinusExp, &oneMinusExp, &temporary, realContext);
  realAdd(&denominator, &temporary, &denominator, realContext);

  realMultiply(&sina, &cosa, &numer.Real, realContext);
  realMultiply(&numer.Real, const_4, &numer.Real, realContext);
  realMultiply(&numer.Real, &expTerm, &numer.Real, realContext);
  realMultiply(&oneMinusExp, &onePlusExp, &numer.Imag, realContext);
  if(realIsNegative(&x->Imag)) {
    realSetNegativeSign(&numer.Imag);
  }

  realDivide(&numer.Real, &denominator, &r->Real, realContext);
  realDivide(&numer.Imag, &denominator, &r->Imag, realContext);

  return ERROR_NONE;
}

/********************************************//**
 * \brief regX ==> regL and tan(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnTan(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&tanReal, &tanCplx);
}
