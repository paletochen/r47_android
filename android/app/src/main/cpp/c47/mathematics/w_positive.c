// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file w_positive.c
 ***********************************************/

#include "c47.h"

static void wPosReal(void) {
  real_t x;
  cplx_t res;

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }

  if(realIsInfinite(&x) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function wPosReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of W when flag SPCRES is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  realCopy(const39_1oneE, &res.Real);
  realSetNegativeSign(&res.Real);
  if(realIsNaN(&x) || realCompareGreaterEqual(&x, &res.Real)) {
    WP34S_LambertW(&x, &res.Real, false, &ctxtReal75);
    convertRealToResultRegister(&res.Real, REGISTER_X, amNone);
  }
  else if(getSystemFlag(FLAG_CPXRES)) {
    cplx_t z;

    realCopy(&x, &z.Real);
    realSetZero(&z.Imag);
    WP34S_ComplexLambertW(&z, &res, &ctxtReal75);
    convertComplexToResultRegister(&res, REGISTER_X);
  }
  else {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function wPosReal:", "X < -e^(-1)", "and CPXRES is not set!", NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}

static void wPosCplx(void) {
  cplx_t x, res;

  if(!getRegisterAsComplex(REGISTER_X, &x)) {
    return;
  }

  if((realIsInfinite(&x.Real) || realIsInfinite(&x.Imag)) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function wPosCplx:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of W when flag SPCRES is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  WP34S_ComplexLambertW(&x, &res, &ctxtReal75);
  convertComplexToResultRegister(&res, REGISTER_X);
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
