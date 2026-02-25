// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file percentPlusMG.c
 ***********************************************/

#include "c47.h"

//=============================================================================
// %+MG calculation functions
//-----------------------------------------------------------------------------

static bool_t percentPlusMGReal(const real_t *xReal, const real_t *yReal, real_t *rReal, realContext_t *realContext) {
  /*
   * Check x and y
   */
  if(realCompareEqual(xReal, const_100) && realIsZero(yReal)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realCopy(const_NaN, rReal);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function percentPlusMGReal:", "cannot divide 0 by 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
  }
  else if(realCompareEqual(xReal, const_100)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realCopy((realIsPositive(yReal) ? const_plusInfinity : const_minusInfinity), rReal);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function percentPlusMGReal:", "cannot divide a real by 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
  }
  else {
    realDivide(xReal, const_100, rReal, realContext);    // r = x / 100.0
    realSubtract(const_1, rReal, rReal, realContext);    // r = 1 - x/100
    realDivide(yReal, rReal, rReal, realContext);        // r = y / (1 - x/100)
  }

  return true;
}

//=============================================================================
// Main function
//-----------------------------------------------------------------------------

/********************************************//**
 * \brief regX ==> regL and PercentSigma(regX) ==> regX
 * enables stack lift and refreshes the stack.
 * Calculate %Sigma
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnPercentPlusMG(uint16_t unusedButMandatoryParameter) {
  real_t xReal, yReal;
  real_t rReal;

  if(!getRegisterAsReal(REGISTER_X, &xReal) || !getRegisterAsReal(REGISTER_Y, &yReal))
    return;

  if(!saveLastX())
    return;

  if(!percentPlusMGReal(&xReal, &yReal, &rReal, &ctxtReal34))
    return;

  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
  convertRealToReal34ResultRegister(&rReal, REGISTER_X);
  setRegisterAngularMode(REGISTER_X, amNone);
  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}

