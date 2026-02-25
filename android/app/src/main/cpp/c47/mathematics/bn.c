// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file bn.c
 ***********************************************/

#include "c47.h"

static void bn_common(bool_t bnstar) {
  real_t x, res;

  if(!getRegisterAsReal(REGISTER_X, &x))
    return;
  if(!saveLastX()) {
    return;
  }

  WP34S_Bernoulli(&x, &res, bnstar, &ctxtReal39);
  if(realIsNaN(&res)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function fnBn:", bnstar ? "k must be a non-negative integer" : "k must be a positive integer", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    convertRealToReal34ResultRegister(&res, REGISTER_X);
  }

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}

void fnBn(uint16_t unusedButMandatoryParameter) {
  bn_common(false);
}

void fnBnStar(uint16_t unusedButMandatoryParameter) {
  bn_common(true);
}
