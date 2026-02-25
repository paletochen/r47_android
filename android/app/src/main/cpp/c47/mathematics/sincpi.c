// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file sincpi.c
 ***********************************************/
// Coded by JM, based on sin.c

#include "c47.h"

void sincpiComplex(const real_t *real, const real_t *imag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  // sin(a + ib) = sin(a)*cosh(b) + i*cos(a)*sinh(b)
  // sinc(a + ib) = sin(a + ib) / (a + ib), for the allowable conditions
  real_t rr, ii, rmdr;

  realCopy(real, &rr);
  realCopy(imag, &ii);
  realDivideRemainder(&rr, const_1, &rmdr, realContext);

  if(realIsZero(&rr) && realIsZero(&ii)) {
    realCopy(const_1, resReal);
    realCopy(const_0, resImag);
  }
  else if(realIsZero(&rmdr) && realIsZero(&ii)) {
    realCopy(const_0, resReal);
    realCopy(const_0, resImag);
  }
  else {
    real_t sina, cosa, sinhb, coshb, sinR, sinImag;

    realMultiply(&rr, const_pi, &rr, realContext);
    realMultiply(&ii, const_pi, &ii, realContext);

    WP34S_Cvt2RadSinCosTan(&rr, amRadian, &sina, &cosa, NULL, realContext);
    WP34S_SinhCosh(&ii, &sinhb, &coshb, realContext);

    realMultiply(&sina, &coshb, resReal, realContext);
    realMultiply(&cosa, &sinhb, resImag, realContext);

    realCopy(resReal, &sinR);
    realCopy(resImag, &sinImag);
    divComplexComplex(&sinR, &sinImag, &rr, &ii, resReal, resImag, realContext);
  }
}


static void sincpiReal(void) {
  real_t x, sine;
  const real_t *r = &x;
  angularMode_t xAngularMode;
  const uint32_t type = getRegisterDataType(REGISTER_X);

  if(!getRegisterAsReal(REGISTER_X, &x))
    return;

  if(realIsInfinite(&x)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      r = const_0;
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function sincpiReal:", "cannot divide a real34 by " STD_PLUS_MINUS STD_INFINITY " when flag D is not set", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  else {
    if(realIsZero(&x)) {
      r = const_1;
    }
    else if(type != dtReal34) {
      r = const_0;
    }
    else {
      xAngularMode = getRegisterAngularMode(REGISTER_X);
      if(xAngularMode != amNone)
        convertAngleFromTo(&x, xAngularMode, amRadian, &ctxtReal75);
      realMultiply(&x, const_pi, &x, &ctxtReal75);   //This pi is to convert sincpi to sinc for all input, regardless
      WP34S_Cvt2RadSinCosTan(&x, amRadian, &sine, NULL, NULL, &ctxtReal75);
      realDivide(&sine, &x, &x, &ctxtReal75);
    }
  }
  convertRealToResultRegister(r, REGISTER_X, amNone);
}


static void sincpiCplx(void) {
  real_t zReal, zImag;

  if(!getRegisterAsComplex(REGISTER_X, &zReal, &zImag))
    return;

  sincpiComplex(&zReal, &zImag, &zReal, &zImag, &ctxtReal39);

  convertComplexToResultRegister(&zReal, &zImag, REGISTER_X);
}


/********************************************//**
 * \brief regX ==> regL and sincpi(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSincpi(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&sincpiReal, &sincpiCplx);

}
