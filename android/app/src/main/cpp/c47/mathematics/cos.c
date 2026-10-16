// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file cos.c
 ***********************************************/

#include "c47.h"

void cosComplex(const cplx_t *z, cplx_t *res, realContext_t *realContext) {
  // cos(a + i b) = cos(a)*cosh(b) - i*sin(a)*sinh(b)
  real_t sina, cosa, sinhb, coshb;

  C47_WP34S_Cvt2RadSinCosTan(&z->Real, amRadian, &sina, &cosa, NULL, realContext);
  WP34S_SinhCosh(&z->Imag, &sinhb, &coshb, realContext);

  multiplyTrigByHyperbolic(&cosa, &coshb, &res->Real, realContext);
  multiplyTrigByHyperbolic(&sina, &sinhb, &res->Imag, realContext);
  realChangeSign(&res->Imag);
}



static void cosReal(void) {
  sinCosReal(trigCos);
}



static void cosCplx(void) {
  sinCosCplx(trigCos);
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
