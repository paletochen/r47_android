// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file gammaX.c
 ***********************************************/

#include "c47.h"

/********************************************//**
 * \brief regX ==> regL and gamma(regX, regY) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnGammaX(uint16_t gammaType) {
  real_t x, y, res;

  if (!getRegisterAsReal(REGISTER_X, &x) || !getRegisterAsReal(REGISTER_Y, &y) || !saveLastX())
    return;

  if(!getSystemFlag(FLAG_SPCRES) && (realCompareLessEqual(&x, const_0) || realCompareLessThan(&y, const_0))) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function fnGammaX:", "Y must be non-negative and X must be positive", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    WP34S_GammaP(&y, &x, &res, &ctxtReal39, gammaType >> 1, gammaType & 0x0001);
    convertRealToResultRegister(&res, REGISTER_X, amNone);
  }

  adjustResult(REGISTER_X, true, true, REGISTER_X, REGISTER_Y, -1);
}
