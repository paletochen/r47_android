// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file arccos.c
 ***********************************************/

#include "c47.h"

static void arccosCplx(void) {
  real_t a, b, real, imag;

  if(!getRegisterAsComplex(REGISTER_X, &a, &b))
    return;

  // arccos(z) = -i.ln(z + i.sqrt(1 - z²))
  // calculate i.sqrt(1 - z²)
  realMinus(&b, &real, &ctxtReal39);
  sqrt1Px2Complex(&real, &a, &imag, &real, &ctxtReal39);
  realChangeSign(&real);

  // calculate z + i.sqrt(1 - z²)
  realAdd(&a, &real, &real, &ctxtReal39);
  realAdd(&b, &imag, &imag, &ctxtReal39);

  // calculate ln(z + i.sqrt(1 - z²))
  lnComplex(&real, &imag, &real, &imag, &ctxtReal39);

  // calculate = -i.ln(z + i.sqrt(1 - z²))
  realMinus(&real, &a, &ctxtReal39);
  realCopy(&imag, &b);

  convertComplexToResultRegister(&b, &a, REGISTER_X);
}



static void arccosReal(void) {
  real_t x;
  const real_t *r = &x;

  if(!getRegisterAsReal(REGISTER_X, &x))
    return;

  if(realCompareAbsGreaterThan(&x, const_1)) {
    if(getFlag(FLAG_CPXRES)) {
      arccosCplx();
      return;
    }
    else if(getSystemFlag(FLAG_SPCRES)) {
      r = const_NaN;
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function arccosReal:", "|X| > 1", "and CPXRES is not set!", NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  else {
    WP34S_Acos(&x, &x, &ctxtReal39);
    convertAngleFromTo(&x, amRadian, currentAngularMode, &ctxtReal39);
  }
  convertRealToResultRegister(r, REGISTER_X, currentAngularMode);
}



/********************************************//**
 * \brief regX ==> regL and arccos(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnArccos(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&arccosReal, &arccosCplx);
}
