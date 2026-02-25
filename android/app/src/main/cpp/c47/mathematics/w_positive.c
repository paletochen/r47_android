// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file w_positive.c
 ***********************************************/

#include "c47.h"

static void wPosReal(void) {
  real_t x, res, resi;

  if(!getRegisterAsReal(REGISTER_X, &x))
    return;

  realCopy(const_1oneE, &res);
  realSetNegativeSign(&res);
  if(realCompareGreaterEqual(&x, &res)) {
    WP34S_LambertW(&x, &res, false, &ctxtReal39);
    convertRealToResultRegister(&res, REGISTER_X, amNone);
  }
  else if(getSystemFlag(FLAG_CPXRES)) {
    WP34S_ComplexLambertW(&x, const_0, &res, &resi, &ctxtReal39);
    convertComplexToResultRegister(&res, &resi, REGISTER_X);
  }
  else {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function wPosReal:", "X < -e^(-1)", "and CPXRES is not set!", NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}

static void wPosCplx(void) {
  real_t xr, xi, resr, resi;

  if(!getRegisterAsComplex(REGISTER_X, &xr, &xi))
    return;
  WP34S_ComplexLambertW(&xr, &xi, &resr, &resi, &ctxtReal39);
  convertComplexToResultRegister(&resr, &resi, REGISTER_X);
}

/********************************************//**
 * \brief regX ==> regL and W(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnWpositive(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&wPosReal, &wPosCplx);
}
