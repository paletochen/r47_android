// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file percentMRR.c
 ***********************************************/

#include "c47.h"

static bool_t exactRegisterRatio(calcRegister_t numeratorRegister, calcRegister_t denominatorRegister, real_t *result, realContext_t *realContext) {
  int32_t numeratorExponent, denominatorExponent;
  longInteger_t numerator, denominator;

  if(!convertRegisterToScaledLongInteger(numeratorRegister, numerator, &numeratorExponent)) {
    return false;
  }
  if(!convertRegisterToScaledLongInteger(denominatorRegister, denominator, &denominatorExponent)) {
    longIntegerFree(numerator);
    return false;
  }
  const bool_t ok = divideScaledLongIntegersToReal(numerator, numeratorExponent, false, denominator, denominatorExponent, result, realContext);
  longIntegerFree(denominator);
  longIntegerFree(numerator);
  return ok;
}


static bool_t exactRegisterRatioDifference(bool_t magnitudes, real_t *result, realContext_t *realContext) {
  int32_t xExponent, yExponent, differenceExponent;
  longInteger_t x, y, difference;

  if(!convertRegisterToScaledLongInteger(REGISTER_X, x, &xExponent)) {
    return false;
  }
  if(!convertRegisterToScaledLongInteger(REGISTER_Y, y, &yExponent)) {
    longIntegerFree(x);
    return false;
  }
  if(magnitudes) {
    longIntegerSetPositiveSign(x);
    longIntegerSetPositiveSign(y);
  }
  longIntegerInit(difference);
  addSubtractScaledLongIntegers(x, xExponent, y, yExponent, true, difference, &differenceExponent);
  const bool_t ok = divideScaledLongIntegersToReal(difference, differenceExponent, false, y, yExponent, result, realContext);
  longIntegerFree(difference);
  longIntegerFree(y);
  longIntegerFree(x);
  return ok;
}


static bool_t stableNegativeRatio(const real_t *ratio, const real_t *periods, real_t *result) {
  real_t exponent, logarithm;
  bool_t conversionError;
  int32_t integerExponent;

  if(!realIsNegative(ratio) || !realCompareAbsGreaterThan(ratio, const_1on2) || !realCompareAbsLessThan(ratio, const_2)) {
    return false;
  }
  realDivide(const_1, periods, &exponent, &ctxtReal75);
  if(!realIsAnInteger(&exponent)) {
    return false;
  }
  integerExponent = realToInt32C47(&exponent, &conversionError);
  if(conversionError || integerExponent < -999999999 || integerExponent > 999999999) {
    return false;
  }
  if(!exactRegisterRatioDifference(true, &logarithm, &ctxtReal51)) {
    return false;
  }
  WP34S_Ln1P(&logarithm, &logarithm, &ctxtReal51);
  realDivide(&logarithm, periods, result, &ctxtReal51);
  WP34S_ExpM1(result, result, &ctxtReal51);
  if((integerExponent & 1) != 0) {
    realChangeSign(result);
    realSubtract(result, const_2, result, &ctxtReal51);
  }
  return true;
}


static void percentMRR() {
  real_t xReal, yReal, zReal;

  /*
   * Convert register X.
   */
  if(!getRegisterAsReal(REGISTER_X, &xReal)) {
    return;
  }

  /*
   * Convert register Y.
   */
  if(!getRegisterAsReal(REGISTER_Y, &yReal)) {
    return;
  }

  /*
   * Convert register Z
   */
  if(!getRegisterAsReal(REGISTER_Z, &zReal)) {
    return;
  }

  /*
   * Calculate %MRR
   */
  real_t q;

  if(realIsZero(&xReal) && realIsZero(&yReal)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realSetNaN(&q);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function percentMRR:", "cannot divide x=0 by y=0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  else if(realIsZero(&yReal)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realSetPlusInfinity(&q);
      q.bits |= DECNEG*(realIsNegative(&xReal) != realIsNegative(&yReal));
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function percentMRR:", "cannot divide a real by 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  else if(realIsZero(&zReal)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realSetNaN(&q);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function percentMRR:", "cannot calculate a mean rate over zero periods", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  else {
    if(!exactRegisterRatio(REGISTER_X, REGISTER_Y, &q, &ctxtReal51)) {
      realDivide(&xReal, &yReal, &q, &ctxtReal51);
    }
    if(realCompareGreaterThan(&q, const_0)) {
      if(realCompareGreaterThan(&q, const_1on2) && realCompareLessThan(&q, const_2)) {
        if(!exactRegisterRatioDifference(false, &q, &ctxtReal51)) {
          realSubtract(&xReal, &yReal, &q, &ctxtReal51); // q = x - y
          realDivide(&q, &yReal, &q, &ctxtReal51);       // q = x/y - 1
        }
        WP34S_Ln1P(&q, &q, &ctxtReal51);               // q = ln(x/y)
      }
      else {
        WP34S_Ln(&q, &q, &ctxtReal51);
      }
      realDivide(&q, &zReal, &q, &ctxtReal51);       // q = ln(x/y) / z
      WP34S_ExpM1(&q, &q, &ctxtReal51);              // q = exp(ln(x/y) / z) - 1
    }
    else {
      if(!stableNegativeRatio(&q, &zReal, &q)) {
        if(!exactRegisterRatio(REGISTER_X, REGISTER_Y, &q, &ctxtReal75)) {
          realDivide(&xReal, &yReal, &q, &ctxtReal75);
        }
        realDivide(const_1, &zReal, &zReal, &ctxtReal75);
        realPower(&q, &zReal, &q, &ctxtReal75);
        realSubtract(&q, const_1, &q, &ctxtReal75);
      }
    }
    q.exponent += 2;                                  // q = 100 * (pow(x/y, 1/z) - 1)
  }

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
