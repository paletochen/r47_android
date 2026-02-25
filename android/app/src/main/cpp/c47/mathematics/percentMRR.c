// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file percentMRR.c
 ***********************************************/

#include "c47.h"

static void percentMRR() {
  real_t xReal, yReal, zReal;

  /*
   * Convert register X.
   */
  if(!getRegisterAsReal(REGISTER_X, &xReal))
    return;

  /*
   * Convert register Y.
   */
  if(!getRegisterAsReal(REGISTER_Y, &yReal))
    return;

  /*
   * Convert register Z
   */
  if(!getRegisterAsReal(REGISTER_Z, &zReal))
    return;

  /*
   * Calculate %MRR
   */
  real_t q;

  if(realIsZero(&xReal) && realIsZero(&yReal)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realCopy(const_NaN, &q);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function percentMRR:", "cannot divide x=0 by y=0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  else if(realIsZero(&yReal)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realCopy((realIsPositive(&xReal) ? const_plusInfinity : const_minusInfinity), &q);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function percentMRR:", "cannot divide a real by 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }

  realDivide(&xReal, &yReal, &q, &ctxtReal75);        // q = x/y
  realDivide(const_1, &zReal, &zReal, &ctxtReal75);   // z = 1/z
  realPower(&q, &zReal, &q, &ctxtReal75);             // q = pow(x/y, 1/z)
  realSubtract(&q, const_1, &q, &ctxtReal75);         // q = pow(x/y, 1/z) - 1
  q.exponent += 2;                                    // q = 100 * ( pow(x/y, 1/z) - 1 )

  convertRealToResultRegister(&q, REGISTER_X, amNone);

  temporaryInformation = TI_PERC;
}

/********************************************//**
 * \brief regX ==> regL and %MRR(regX, RegY, RegZ) ==> regX
 * enables stack lift and refreshes the stack.
 * Calculate the %MRR.
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnPercentMRR(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }

  percentMRR();

  adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
  if(lastErrorCode == 0) {
    fnDropY(NOPARAM);
  }
}
