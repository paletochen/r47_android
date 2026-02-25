// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file percentSigma.c
 ***********************************************/

#include "c47.h"

//=============================================================================
// PercentSigma calculation function
//-----------------------------------------------------------------------------

bool_t percentSigma(real_t *xReal, real_t *rReal, realContext_t *realContext) {
  realCopy(SIGMA_X, rReal);    // r = Sum(x)
  if(realIsZero(rReal)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realCopy((realIsPositive(rReal) ? const_plusInfinity : const_minusInfinity), rReal);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function percentSigma:", "cannot divide a real by 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
  }

  realDivide(xReal, rReal, rReal, realContext); // r = x/Sum(x)
  rReal->exponent += 2;                         // r = x/Sum(x) * 100

  return true;
}

/***********************************************
 * \brief Percent(X(real34)) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
static void percentSigmaReal(void) {
  real_t xReal, rReal;

  if(!getRegisterAsReal(REGISTER_X, &xReal) || !percentSigma(&xReal, &rReal, &ctxtReal39))
    return;

  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
  convertRealToReal34ResultRegister(&rReal, REGISTER_X);
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
void fnPercentSigma(uint16_t unusedButMandatoryParameter) {
  if(!checkMinimumDataPoints(const_1)) {
    displayCalcErrorMessage(ERROR_NO_SUMMATION_DATA, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "There is no statistical data available!");
      moreInfoOnError("In function fnPercentSigma:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(!saveLastX())
    return;

  if(getRegisterDataType(REGISTER_X) == dtReal34Matrix)
    elementwiseRema(percentSigmaReal);
  else
    percentSigmaReal();

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
  temporaryInformation = TI_PERC;
}
