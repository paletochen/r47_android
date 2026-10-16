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
      realSetNaN(rReal);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function percentPlusMGReal:", "cannot divide 0 by 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
  }
  else if(realCompareEqual(xReal, const_100)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realSetPlusInfinity(rReal);
      rReal->bits |= DECNEG*realIsNegative(yReal);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
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



static bool_t percentPlusMGExactRegisters(real34_t *result) {
  int32_t xExponent, yExponent, denominatorExponent;
  longInteger_t x, y, oneHundred, denominator;

  if(!convertRegisterToScaledLongInteger(REGISTER_X, x, &xExponent)) {
    return false;
  }
  if(!convertRegisterToScaledLongInteger(REGISTER_Y, y, &yExponent)) {
    longIntegerFree(x);
    return false;
  }

  longIntegerInit(oneHundred);
  uInt32ToLongInteger(1u, oneHundred);
  addSubtractScaledLongIntegers(oneHundred, 2, x, xExponent, true, denominator, &denominatorExponent);
  bool_t negativeZero = false;
  if(longIntegerIsZero(y)) {
    const bool_t numeratorNegative = getRegisterDataType(REGISTER_Y) == dtReal34 && real34IsNegative(REGISTER_REAL34_DATA(REGISTER_Y));
    negativeZero = numeratorNegative != longIntegerIsNegative(denominator);
  }
  const bool_t ok = divideScaledLongIntegersToReal34(y, yExponent + 2, negativeZero, denominator, denominatorExponent, result);
  longIntegerFree(denominator);
  longIntegerFree(oneHundred);
  longIntegerFree(y);
  longIntegerFree(x);
  return ok;
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
  real34_t exactResult;
  const uint32_t typeX = getRegisterDataType(REGISTER_X);
  const uint32_t typeY = getRegisterDataType(REGISTER_Y);
  const bool_t exactQuotient = (typeX == dtLongInteger || typeY == dtLongInteger)
                               && percentPlusMGExactRegisters(&exactResult);

  if(!exactQuotient && (!getRegisterAsReal(REGISTER_X, &xReal) || !getRegisterAsReal(REGISTER_Y, &yReal))) {
    return;
  }

  if(!saveLastX()) {
    return;
  }

  if(!exactQuotient && !percentPlusMGReal(&xReal, &yReal, &rReal, &ctxtReal39)) {
    return;
  }

  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
  if(exactQuotient) {
    real34Copy(&exactResult, REGISTER_REAL34_DATA(REGISTER_X));
  }
  else {
    convertRealToReal34ResultRegister(&rReal, REGISTER_X);
  }
  setRegisterAngularMode(REGISTER_X, amNone);
  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}
