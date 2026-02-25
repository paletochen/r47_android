// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file sin.c
 ***********************************************/

#include "c47.h"

void sinComplex(const real_t *real, const real_t *imag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  // sin(a + ib) = sin(a)*cosh(b) + i*cos(a)*sinh(b)
  real_t sina, cosa, sinhb, coshb;

  WP34S_Cvt2RadSinCosTan(real, amRadian, &sina, &cosa, NULL, realContext);
  WP34S_SinhCosh(imag, &sinhb, &coshb, realContext);

  realMultiply(&sina, &coshb, resReal, realContext);
  realMultiply(&cosa, &sinhb, resImag, realContext);
}


static void sinReal(void) {
  real_t x;
  const real_t *r = &x;
  angularMode_t xAngularMode;

  if(!getRegisterAsRealAngle(REGISTER_X, &x, &xAngularMode))
    return;
  if(realIsSpecial(&x))
    r = const_NaN;
  else
    WP34S_Cvt2RadSinCosTan(r = &x, xAngularMode, &x, NULL, NULL, &ctxtReal75);
  convertRealToResultRegister(r, REGISTER_X, amNone);
}



static void sinCplx(void) {
  real_t zReal, zImag;

  if(!getRegisterAsComplex(REGISTER_X, &zReal, &zImag))
    return;

  sinComplex(&zReal, &zImag, &zReal, &zImag, &ctxtReal75);

  convertComplexToResultRegister(&zReal, &zImag, REGISTER_X);
}


/********************************************//**
 * \brief regX ==> regL and sin(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSin(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&sinReal, &sinCplx);
}
