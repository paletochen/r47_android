// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file deltaPercentXmean.c
 ***********************************************/

#include "c47.h"

//=============================================================================
// Delta% calculation functions
//-----------------------------------------------------------------------------

bool_t deltaPercentXmeanReal(real_t *xReal, real_t *rReal, realContext_t *realContext) {
  real_t negativeSum;

  /*
   * Check x and y
   */
  if(realIsZero(xReal) && realIsZero(SIGMA_X) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function deltaPercentXmeanReal:", "cannot divide 0 by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  } else if(realIsZero(SIGMA_X) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function deltaPercentXmeanReal:", "cannot divide a real by y=0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  } else {
    realCopy(SIGMA_X, &negativeSum);
    realChangeSign(&negativeSum);
    realFMA(SIGMA_N, xReal, &negativeSum, rReal, &ctxtReal39);  // r = n*x - Sum(x)
    rReal->exponent += 2;                                      // r = (n*x - Sum(x)) * 100
    realDivide(rReal, SIGMA_X, rReal, realContext);            // r = (n*x - Sum(x)) * 100 / Sum(x)
  }
  return true;
}

//=============================================================================
// Main function
//-----------------------------------------------------------------------------

/********************************************//**
 * \brief regX ==> regL and deltaPercentXmean(regX) ==> regX
 * enables stack lift and refreshes the stack.
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/


void fnDeltaPercentXmean(uint16_t unusedButMandatoryParameter) {
  real_t xReal;
  real_t rReal;

  if(!checkMinimumDataPoints(const_1)) {
    displayCalcErrorMessage(ERROR_NO_SUMMATION_DATA, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "There is no statistical data available!");
      moreInfoOnError("In function fnDeltaPercentXmean:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(!getRegisterAsReal(REGISTER_X, &xReal)) {
    return;
  }

  if(!saveLastX()) {
    return;
  }

  realSetZero(&rReal);
  if(deltaPercentXmeanReal(&xReal, &rReal, &ctxtReal34)) {
    convertRealToResultRegister(&rReal, REGISTER_X, amNone);
    setRegisterAngularMode(REGISTER_X, amNone);
  }

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);

  temporaryInformation = TI_PERCD;
}
