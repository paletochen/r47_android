// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file deltaPercent.c
 ***********************************************/

#include "c47.h"

//=============================================================================
// Delta% calculation functions
//-----------------------------------------------------------------------------

static bool_t deltaPercentReal(real_t *xReal, real_t *yReal, real_t *rReal, realContext_t *realContext) {
  /*
   * Check x and y
   */
  if(realIsZero(xReal) && realCompareEqual(xReal, yReal)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realSetNaN(rReal);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function deltaPercentReal:", "cannot divide 0 by 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
  }
  else if(realIsZero(yReal)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      if(realIsNaN(xReal)) {
        realSetNaN(rReal);
      }
      else {
        realSetPlusInfinity(rReal);
        rReal->bits |= DECNEG*(realIsNegative(xReal) != realIsNegative(yReal));
      }
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function deltaPercentReal:", "cannot divide a real by y=0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
  }
  else {
    realSubtract(xReal, yReal, rReal, realContext);     // r = x - y
    realDivide(rReal, yReal, rReal, realContext);       // r = (x - y) / y
    rReal->exponent += 2;                               // r = (x - y) / y * 100
  }

  return true;
}



static bool_t deltaPercentExactRegisters(real34_t *result) {
  int32_t xExponent, yExponent, differenceExponent;
  longInteger_t x, y, difference;

  if(!convertRegisterToScaledLongInteger(REGISTER_X, x, &xExponent)) {
    return false;
  }
  if(!convertRegisterToScaledLongInteger(REGISTER_Y, y, &yExponent)) {
    longIntegerFree(x);
    return false;
  }

  addSubtractScaledLongIntegers(x, xExponent, y, yExponent, true, difference, &differenceExponent);
  const bool_t ok = divideScaledLongIntegersToReal34(difference, differenceExponent + 2, false, y, yExponent, result);
  longIntegerFree(difference);
  longIntegerFree(y);
  longIntegerFree(x);
  return ok;
}

//=============================================================================
// Main function
//-----------------------------------------------------------------------------

/********************************************//**
 * \brief regX ==> regL and DeltaPercent(regX, RegY) ==> regX
 * enables stack lift and refreshes the stack.
 * Calculate x*y/100
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnDeltaPercent(uint16_t unusedButMandatoryParameter) {
  real_t xReal, yReal;
  real_t rReal;
  real34_t exactResult;
  const uint32_t typeX = getRegisterDataType(REGISTER_X);
  const uint32_t typeY = getRegisterDataType(REGISTER_Y);
  const bool_t exactQuotient = (typeX == dtLongInteger || typeY == dtLongInteger)
                               && deltaPercentExactRegisters(&exactResult);

  if(!exactQuotient && (!getRegisterAsReal(REGISTER_X, &xReal) || !getRegisterAsReal(REGISTER_Y, &yReal))) {
    return;
  }

  if(!saveLastX()) {
    return;
  }

  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
  if(exactQuotient) {
    real34Copy(&exactResult, REGISTER_REAL34_DATA(REGISTER_X));
  }
  else {
    realSetZero(&rReal);
    deltaPercentReal(&xReal, &yReal, &rReal, &ctxtReal39);
    convertRealToReal34ResultRegister(&rReal, REGISTER_X);
  }
  setRegisterAngularMode(REGISTER_X, amNone);
  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);

  temporaryInformation = TI_PERCD;
}
