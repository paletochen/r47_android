// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file arccosh.c
 ***********************************************/

#include "c47.h"

static void arccoshCplx(void) {
  real_t a, b, real, imag;

  if(!getRegisterAsComplex(REGISTER_X, &a, &b))
    return;

  // arccosh(z) = ln(z + sqrt(z² - 1))
  // calculate z²   real part
  realMultiply(&b, &b, &real, &ctxtReal39);
  realChangeSign(&real);
  realFMA(&a, &a, &real, &real, &ctxtReal39);

  // calculate z²   imaginary part
  realMultiply(&a, &b, &imag, &ctxtReal39);
  realMultiply(&imag, const_2, &imag, &ctxtReal39);

  // calculate z² - 1
  realSubtract(&real, const_1, &real, &ctxtReal39);

  // calculate sqrt(z² - 1)
  realRectangularToPolar(&real, &imag, &real, &imag, &ctxtReal39);
  realSquareRoot(&real, &real, &ctxtReal39);
  realMultiply(&imag, const_1on2, &imag, &ctxtReal39);
  realPolarToRectangular(&real, &imag, &real, &imag, &ctxtReal39);

  // calculate z + sqrt(z² - 1)
  realAdd(&a, &real, &real, &ctxtReal39);
  realAdd(&b, &imag, &imag, &ctxtReal39);

  // calculate ln(z + sqtr(z² - 1))
  realRectangularToPolar(&real, &imag, &a, &b, &ctxtReal39);
  WP34S_Ln(&a, &a, &ctxtReal39);

  convertComplexToResultRegister(&a, &b, REGISTER_X);
}


static void arccoshReal(void) {
  real_t x;
  const real_t *r = &x;

  if(!getRegisterAsReal(REGISTER_X, &x))
    return;

  if(realCompareLessThan(&x, const_1)) {
    if(getFlag(FLAG_CPXRES)) {
      arccoshCplx();
      return;
    }
    if(getSystemFlag(FLAG_SPCRES)) {
      r = const_NaN;
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function arccoshReal:", "X < 1", "and CPXRES is not set!", NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  else {
    realArcosh(&x, &x, &ctxtReal75);
  }

  convertRealToResultRegister(r, REGISTER_X, amNone);
}



void realArcosh(const real_t *x, real_t *res, realContext_t *realContext) {
  real_t xSquared;

  // arccosh(x) = ln(x + sqrt(x² - 1))
  realMultiply(x, x, &xSquared, realContext);
  realSubtract(&xSquared, const_1, &xSquared, realContext);
  realSquareRoot(&xSquared, &xSquared, realContext);
  realAdd(&xSquared, x, res, realContext);
  WP34S_Ln(res, res, realContext);
}



/********************************************//**
 * \brief regX ==> regL and arccosh(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnArccosh(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&arccoshReal, &arccoshCplx);
}
