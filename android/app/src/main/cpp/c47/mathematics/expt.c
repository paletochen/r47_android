// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file expt.c
 ***********************************************/

#include "c47.h"

static bool_t getExponent(int32_t *res) {
  real_t x;

  if (!getRegisterAsReal(REGISTER_X, &x))
    return false;

  if(realIsNaN(&x)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function getExponent:", "cannot use NaN as X input of EXPT", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }

  if(realIsInfinite(&x)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function getExponent:", "cannot use ±∞ as an input of EXPT", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }

  *res = realIsZero(&x) ? 0 : x.exponent + x.digits - 1;
  return true;
}

static void exptLonI(void) {
  int32_t expt;
  longInteger_t lgInt;

  if (!getExponent(&expt))
    return;

  longIntegerInit(lgInt);
  int32ToLongInteger(expt, lgInt);
  convertLongIntegerToLongIntegerRegister(lgInt, REGISTER_X);
  longIntegerFree(lgInt);
}

static void exptReal(void) {
  int32_t expt;
  real_t x;

  if (!getExponent(&expt))
    return;
  int32ToReal(expt, &x);
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}

/********************************************//**
 * \brief regX ==> regL and expt(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnExpt(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexMonadicFunction(&exptReal, NULL, NULL, &exptLonI);
}
