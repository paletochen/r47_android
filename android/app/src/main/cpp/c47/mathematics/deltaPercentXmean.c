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
real_t yReal;

  realDivide(SIGMA_X, SIGMA_N, &yReal, &ctxtReal39);
  /*
   * Check x and y
   */
  if(realIsZero(xReal) && realCompareEqual(xReal, &yReal)) {
      if(getSystemFlag(FLAG_SPCRES)) {
        realCopy(const_NaN, rReal);
      }
      else {
        displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function deltaPercentXmeanReal:", "cannot divide 0 by 0", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return false;
      }
  }
  else if(realIsZero(&yReal)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realCopy((realCompareAbsGreaterThan(xReal, &yReal) ? const_plusInfinity : const_minusInfinity),rReal);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function deltaPercentXmeanReal:", "cannot divide a real by y=0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
  }
  else {
    realSubtract(xReal, &yReal, rReal, realContext); // r = x - y
    realDivide(rReal, &yReal, rReal, realContext);   // r = (x - y)/y
    rReal->exponent += 2;                            // r = r * 100.0
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
    displayCalcErrorMessage(ERROR_NO_SUMMATION_DATA, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "There is no statistical data available!");
      moreInfoOnError("In function fnDeltaPercentXmean:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(!getRegisterAsReal(REGISTER_X, &xReal))
    return;

  if(!saveLastX()) {
    return;
  }

  realZero(&rReal);
  if(deltaPercentXmeanReal(&xReal, &rReal, &ctxtReal75)) {
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    convertRealToReal34ResultRegister(&rReal, REGISTER_X);
    setRegisterAngularMode(REGISTER_X, amNone);
  }

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);

  temporaryInformation = TI_PERCD;
}
