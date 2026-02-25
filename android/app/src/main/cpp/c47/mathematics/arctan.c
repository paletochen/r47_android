// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file arctan.c
 ***********************************************/

#include "c47.h"

static void arctanReal(void) {
  real_t x;

  if(!getRegisterAsReal(REGISTER_X, &x))
    return;

  if(realIsInfinite(&x)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realCopy(realIsPositive(&x) ? const_90 : const__90, &x);
      convertAngleFromTo(&x, amDegree, currentAngularMode, &ctxtReal39);
  }
  else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function arctanReal:", "X = " STD_PLUS_MINUS STD_INFINITY, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  else {
    WP34S_Atan(&x, &x, &ctxtReal39);
    convertAngleFromTo(&x, amRadian, currentAngularMode, &ctxtReal39);
  }
  convertRealToResultRegister(&x, REGISTER_X, currentAngularMode);
}



static void arctanCplx(void) {
  real_t xReal, xImag, rReal, rImag;

  if(!getRegisterAsComplex(REGISTER_X, &xReal, &xImag))
    return;

  ArctanComplex(&xReal, &xImag, &rReal, &rImag, &ctxtReal39);

  convertComplexToResultRegister(&rReal, &rImag, REGISTER_X);
}

uint8_t ArctanComplex(real_t *xReal, real_t *xImag, real_t *rReal, real_t *rImag, realContext_t *realContext) {
  real_t a, b, numer, denom;

  realCopy(xReal, &a);
  realCopy(xImag, &b);

  // arctan(z) = i/2 . ln((1 - iz) / (1 + iz))

  // calculate (1 - iz) / (1 + iz)  with z = a + bi

  // 1 - (a + bi)i      1 - (a² + b²)             - 2a
  // -------------  =  ----------------  +  ---------------- i
  // 1 + (a + bi)i     a² + b² - 2b + 1     a² + b² - 2b + 1

  realMultiply(&a, &a, &denom, realContext);         // denom = a²
  realFMA(&b, &b, &denom, &denom, realContext);   // denom = a² + b²
  realSubtract(const_1, &denom, &numer, realContext);         // numer = 1 - (a² + b²)
  realChangeSign(&b);                                              // b = -b
  realFMA(&b, const_2, &denom, &denom, realContext);     // denom = a² + b² - 2b
  realAdd(&denom, const_1, &denom, realContext);             // denom = a² + b² - 2b + 1
  realMultiply(&a, const_2, &b, realContext);                // imag part = 2a
  realChangeSign(&b);                                             // imag part = -2a
  realDivide(&numer, &denom, &a, realContext);      // real part = numer / denom
  realDivide(&b, &denom, &b, realContext);          // imag part = -2a / denom

  // calculate ln((1 - iz) / (1 + iz))
  realRectangularToPolar(&a, &b, &a, &b, realContext);
  WP34S_Ln(&a, &a, realContext);

  // arctan(z) = i/2 . ln((1 - iz) / (1 + iz))
  realMultiply(&a, const_1on2, &a, realContext);
  realMultiply(&b, const_1on2, &b, realContext);
  realChangeSign(&b);

  realCopy(&b, rReal);
  realCopy(&a, rImag);

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
