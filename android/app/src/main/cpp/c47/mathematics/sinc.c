// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file sinc.c
 ***********************************************/
// Coded by JM, based on sin.c

#include "c47.h"

void sincComplex(const real_t *real, const real_t *imag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  // sin(a + ib) = sin(a)*cosh(b) + i*cos(a)*sinh(b)
  // sinc (a + ib) = sin(a + ib) / (a + ib), for the allowable conditions
  real_t rr;
  real_t ii;

  //fnCvtFromCurrentAngularMode(amRadian);

  realCopy(real, &rr);
  realCopy(imag, &ii);

  if(realIsZero(&rr) && realIsZero(&ii)) {
    realCopy(const_1, resReal);
    realCopy(const_0, resImag);
  }
  else {
    real_t sina, cosa, sinhb, coshb, sinR, sinImag;

    WP34S_Cvt2RadSinCosTan(real, amRadian, &sina, &cosa, NULL, realContext);
    WP34S_SinhCosh(imag, &sinhb, &coshb, realContext);

    realMultiply(&sina, &coshb, resReal, realContext);
    realMultiply(&cosa, &sinhb, resImag, realContext);

    realCopy(resReal, &sinR);
    realCopy(resImag, &sinImag);
    divComplexComplex(&sinR, &sinImag, &rr, &ii, resReal, resImag, realContext);
  }
}


static void sincReal(void) {
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
        moreInfoOnError("In function sincReal:", "cannot divide a real34 by " STD_PLUS_MINUS STD_INFINITY " when flag D is not set", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  else {
    if(realIsZero(&x)) {
      r = const_1;
    }
    else {
      if(type == dtReal34) {
        xAngularMode = getRegisterAngularMode(REGISTER_X);
        if(xAngularMode != amNone)
          convertAngleFromTo(&x, xAngularMode, amRadian, &ctxtReal39);
      }
      WP34S_Cvt2RadSinCosTan(&x, amRadian, &sine, NULL, NULL, &ctxtReal39);
      realDivide(&sine, &x, &x, &ctxtReal75);
    }
  }
  convertRealToResultRegister(r, REGISTER_X, amNone);
}



static void sincCplx(void) {
  real_t zReal, zImag;

  if(!getRegisterAsComplex(REGISTER_X, &zReal, &zImag))
    return;

  sincComplex(&zReal, &zImag, &zReal, &zImag, &ctxtReal75);

  convertComplexToResultRegister(&zReal, &zImag, REGISTER_X);
}



/********************************************//**
 * \brief regX ==> regL and sinc(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSinc(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&sincReal, &sincCplx);
}
