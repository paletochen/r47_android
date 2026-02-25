// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file percentT.c
 ***********************************************/

#include "c47.h"

//=============================================================================
// %+MG calculation functions
//-----------------------------------------------------------------------------

static bool_t percentTReal(real_t *xReal, real_t *yReal, real_t *rReal, realContext_t *realContext) {
  /*
   * Check x and y
   */
  if(realIsZero(xReal) && realIsZero(yReal)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realCopy(const_NaN, rReal);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function percentTReal:", "cannot divide x=0 by y=0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
  }
  else if(realIsZero(yReal)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realCopy((realIsPositive(xReal) ? const_plusInfinity : const_minusInfinity), rReal);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function percentTReal:", "cannot divide a real by y=0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
  }
  else {
    xReal->exponent += 2;                         // xReal = xReal * 100.0
    realDivide(xReal, yReal, rReal, realContext); // rReal = xReal * 100.0 / yReal
  }

  return true;
}

//=============================================================================
// Main function
//-----------------------------------------------------------------------------

/********************************************//**
 * \brief regX ==> regL and PercentT(regX, RegY) ==> regX
 * enables stack lift and refreshes the stack.
 * Calculate x*y/100
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnPercentT(uint16_t unusedButMandatoryParameter) {
  real_t xReal, yReal;
  real_t rReal;

  if(!getRegisterAsReal(REGISTER_X, &xReal) || !getRegisterAsReal(REGISTER_Y, &yReal))
    return;

  if(!saveLastX())
    return;

  if(percentTReal(&xReal, &yReal, &rReal, &ctxtReal39)) {
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    convertRealToReal34ResultRegister(&rReal, REGISTER_X);
  }

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);

  temporaryInformation = TI_PERC;
}

