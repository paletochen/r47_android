// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file arcsinh.c
 ***********************************************/

#include "c47.h"

static void arcsinhReal(void) {
  real_t x;

  if(!getRegisterAsReal(REGISTER_X, &x))
    return;

  ArcsinhReal(&x, &x, &ctxtReal51);
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}



static void arcsinhCplx(void) {
  real_t xReal, xImag, rReal, rImag;

  if(!getRegisterAsComplex(REGISTER_X, &xReal, &xImag))
    return;

  ArcsinhComplex(&xReal, &xImag, &rReal, &rImag, &ctxtReal39);

  convertComplexToResultRegister(&rReal, &rImag, REGISTER_X);
}


uint8_t ArcsinhReal(const real_t *x, real_t *res, realContext_t *realContext) {
  if(realIsInfinite(x)) {
    realCopy(realIsNegative(x) ? const_minusInfinity : const_plusInfinity, res);
  }
  else {
    WP34S_ArcSinh(x, res, realContext);
  }

  return ERROR_NONE;
}


uint8_t ArcsinhComplex(const real_t *xReal, const real_t *xImag, real_t *rReal, real_t *rImag, realContext_t *realContext) {
  real_t a, b;

  realCopy(xReal, &a);
  realCopy(xImag, &b);

  // arcsinh(z) = ln(z + sqrt(z� + 1))
  // calculate z�   real part
  realMultiply(&b, &b, rReal, realContext);
  realChangeSign(rReal);
  realFMA(&a, &a, rReal, rReal, realContext);

  // calculate z�   imaginary part
  realMultiply(&a, &b, rImag, realContext);
  realMultiply(rImag, const_2, rImag, realContext);

  // calculate z� + 1
  realAdd(rReal, const_1, rReal, realContext);

  // calculate sqrt(z� + 1)
  realRectangularToPolar(rReal, rImag, rReal, rImag, realContext);
  realSquareRoot(rReal, rReal, realContext);
  realMultiply(rImag, const_1on2, rImag, realContext);
  realPolarToRectangular(rReal, rImag, rReal, rImag, realContext);

  // calculate z + sqrt(z� + 1)
  realAdd(&a, rReal, rReal, realContext);
  realAdd(&b, rImag, rImag, realContext);

  // calculate ln(z + sqtr(z� + 1))
  realRectangularToPolar(rReal, rImag, &a, &b, realContext);
  WP34S_Ln(&a, &a, realContext);

  realCopy(&a, rReal);
  realCopy(&b, rImag);

  return ERROR_NONE;
}



/********************************************//**
 * \brief regX ==> regL and arcsinh(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnArcsinh(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&arcsinhReal, &arcsinhCplx);
}
