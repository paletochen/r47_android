// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file cos.c
 ***********************************************/

#include "c47.h"

void cosComplex(const real_t *real, const real_t *imag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  // cos(a + i b) = cos(a)*cosh(b) - i*sin(a)*sinh(b)
  real_t sina, cosa, sinhb, coshb;

  WP34S_Cvt2RadSinCosTan(real, amRadian, &sina, &cosa, NULL, realContext);
  WP34S_SinhCosh(imag, &sinhb, &coshb, realContext);

  realMultiply(&cosa, &coshb, resReal, realContext);
  realMultiply(&sina, &sinhb, resImag, realContext);
  realChangeSign(resImag);
}



static void cosReal(void) {
  real_t x;
  const real_t *r = &x;
  angularMode_t xAngularMode;

  if(!getRegisterAsRealAngle(REGISTER_X, &x, &xAngularMode))
    return;

  if(realIsSpecial(&x))
    r = const_NaN;
  else
    WP34S_Cvt2RadSinCosTan(r = &x, xAngularMode, NULL, &x, NULL, &ctxtReal75);
  convertRealToResultRegister(r, REGISTER_X, amNone);
}



static void cosCplx(void) {
  real_t zReal, zImag;

  if(!getRegisterAsComplex(REGISTER_X, &zReal, &zImag))
    return;

  cosComplex(&zReal, &zImag, &zReal, &zImag, &ctxtReal75);

  convertComplexToResultRegister(&zReal, &zImag, REGISTER_X);
}



/********************************************//**
 * \brief regX ==> regL and cos(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnCos(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&cosReal, &cosCplx);
}
