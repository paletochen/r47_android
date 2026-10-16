// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file arcsinh.c
 ***********************************************/

#include "c47.h"

static void arcsinhReal(void) {
  real_t x;

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }

  ArcsinhReal(&x, &x, &ctxtReal39);
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}



static void arcsinhCplx(void) {
  cplx_t x, r;

  if(!getRegisterAsComplex(REGISTER_X, &x)) {
    return;
  }

  ArcsinhComplex(&x, &r, &ctxtReal39);

  convertComplexToResultRegister(&r, REGISTER_X);
}


uint8_t ArcsinhReal(const real_t *x, real_t *res, realContext_t *realContext) {
  if(realIsInfinite(x)) {
    realCopy(x, res);
  }
  else {
    WP34S_ArcSinh(x, res, realContext);
  }

  return ERROR_NONE;
}


uint8_t ArcsinhComplex(const cplx_t *z, cplx_t *r, realContext_t *realContext) {
  real_t x, xAbs, xSquared, y, yAbs, q, d, t;

  realCopy(&z->Real, &x);
  realCopy(&z->Imag, &y);

  if(realIsNaN(&x) || realIsNaN(&y)) {
    realSetNaN(&r->Real);
    realSetNaN(&r->Imag);
    return ERROR_NONE;
  }

  if(realIsInfinite(&x)) {
    realCopy(realIsNegative(&x) ? const_minusInfinity : const_plusInfinity, &r->Real);
    if(realIsInfinite(&y)) {
      realCopy(const39_piOn4, &r->Imag);
      if(realIsNegative(&y)) {
        realSetNegativeSign(&r->Imag);
      }
    } else {
      realSetZero(&r->Imag);
      if(realIsNegative(&y)) {
        realSetNegativeSign(&r->Imag);
      }
    }
    return ERROR_NONE;
  }

  if(realIsInfinite(&y)) {
    realCopy(realIsNegative(&x) ? const_minusInfinity : const_plusInfinity, &r->Real);
    realCopy(const39_piOn2, &r->Imag);
    if(realIsNegative(&y)) {
      realSetNegativeSign(&r->Imag);
    }
    return ERROR_NONE;
  }

  // For asinh(x+iy)=u+iv, t=sinh(u)² is the nonnegative root of t²-(x²+y²-1)t-x²=0; this separates component signs from their magnitudes during evaluation.
  realCopyAbs(&x, &xAbs);
  realCopyAbs(&y, &yAbs);
  realMultiply(&xAbs, &xAbs, &xSquared, realContext);
  realSubtract(&yAbs, const_1, &q, realContext);
  realAdd(&yAbs, const_1, &d, realContext);
  realMultiply(&q, &d, &q, realContext);
  realFMA(&xAbs, &xAbs, &q, &q, realContext);

  // d=sqrt(q²+4x²); the q<0 expression uses d-q instead of q+d, avoiding subtraction of nearly equal operands near the finite imaginary-axis segment.
  realMultiply(&xSquared, const_4, &d, realContext);
  realFMA(&q, &q, &d, &d, realContext);
  realSquareRoot(&d, &d, realContext);
  if(realIsNegative(&q)) {
    realSubtract(&d, &q, &r->Imag, realContext);
    realMultiply(&xSquared, const_2, &t, realContext);
    realDivide(&t, &r->Imag, &t, realContext);
  } else {
    realAdd(&q, &d, &t, realContext);
    realMultiply(&t, const_1on2, &t, realContext);
  }

  realSquareRoot(&t, &xSquared, realContext);
  WP34S_ArcSinh(&xSquared, &r->Real, realContext);
  if(realIsNegative(&x)) {
    realSetNegativeSign(&r->Real);
  }

  realAdd(&t, const_1, &r->Imag, realContext);
  realSquareRoot(&r->Imag, &r->Imag, realContext);
  if(realIsZero(&t)) {
    realSubtract(const_1, &yAbs, &q, realContext);
    realAdd(const_1, &yAbs, &d, realContext);
    realMultiply(&q, &d, &q, realContext);
    realSquareRoot(&q, &q, realContext);
    C47_WP34S_Atan2(&y, &q, &r->Imag, realContext);
  } else {
    realMultiply(&y, &xSquared, &q, realContext);
    realMultiply(&xAbs, &r->Imag, &d, realContext);
    C47_WP34S_Atan2(&q, &d, &r->Imag, realContext);
  }

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
