// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file sinh.c
 ***********************************************/

#include "c47.h"

static void sinhReal(void) {
  real_t x;

  if(!getRegisterAsReal(REGISTER_X, &x))
    return;
  if(realIsInfinite(&x) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function sinhReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of sinh when flag D is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  WP34S_SinhCosh(&x, &x, NULL, &ctxtReal39);
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}



static void sinhCplx(void) {
  // sinh(a + i b) = sinh(a) cos(b) + i cosh(a) sin(b)
  real_t a, b, sinha, cosha, sinb, cosb;

  if(!getRegisterAsComplex(REGISTER_X, &a, &b))
    return;

  WP34S_SinhCosh(&a, &sinha, &cosha, &ctxtReal39);
  WP34S_Cvt2RadSinCosTan(&b, amRadian, &sinb, &cosb, NULL, &ctxtReal39);

  realMultiply(&sinha, &cosb, &a, &ctxtReal39);
  realMultiply(&cosha, &sinb, &b, &ctxtReal39);

  convertComplexToResultRegister(&a, &b, REGISTER_X);
}



/********************************************//**
 * \brief regX ==> regL and sinh(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSinh(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&sinhReal, &sinhCplx);
}
