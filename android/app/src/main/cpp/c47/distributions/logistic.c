// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file logistic.c
 ***********************************************/

#include "c47.h"

#if !defined(OPTION_DIST_LOGIS)
  void fnLogisticP     (uint16_t unusedButMandatoryParameter){}
  void fnLogisticL     (uint16_t unusedButMandatoryParameter){}
  void fnLogisticR     (uint16_t unusedButMandatoryParameter){}
  void fnLogisticI     (uint16_t unusedButMandatoryParameter){}

#else
  bool_t logisticCheckDomain(bool_t registersValid, real_t *x, real_t *i, real_t *j, real_t *unusedThird, uint16_t unusedParameter) {
    if(!registersValid) {
      return false;
    }

    if(realIsSpecial(i) || realIsSpecial(j) || realIsZero(j) || realIsNegative(j)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function logisticCheckDomain:", "parameters must be finite and " STD_sigma " > 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    return true;
  }


  void fnLogisticP(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_LOGIT * 4 + DIST_P);
  }

  void fnLogisticL(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_LOGIT * 4 + DIST_L);
  }

  void fnLogisticR(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_LOGIT * 4 + DIST_R);
  }

  void fnLogisticI(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_LOGIT * 4 + DIST_I);
  }



  /********************************************//**
   * \brief Tests whether the location and scale define a logistic distribution.
   ***********************************************/
  static bool_t logisticParametersAreValid(const real_t *mu, const real_t *s) {
    return !realIsSpecial(mu) && !realIsSpecial(s) && !realIsZero(s) && !realIsNegative(s);
  }

  /********************************************//**
   * \brief Selects a forward calculation context from the exponential sensitivity.
   ***********************************************/
  static realContext_t *logisticForwardContext(const real_t *x, const real_t *mu, const real_t *s, real_t *standard, realContext_t *resultContext) {
    realContext_t *calculationContext = resultContext;

    realSubtract(x, mu, standard, calculationContext);
    realDivide(standard, s, standard, calculationContext);
    if(calculationContext->digits <= 39 && realCompareAbsGreaterThan(standard, const_100)) {
      calculationContext = &ctxtReal51;
      realSubtract(x, mu, standard, calculationContext);
      realDivide(standard, s, standard, calculationContext);
    }
    return calculationContext;
  }

  /********************************************//**
   * \brief Computes log(p/(1-p)) with a central series at a standard working precision.
   ***********************************************/
  static void logisticLogit(const real_t *probability, real_t *res, realContext_t *realContext) {
    real_t addend, divisor, previous, square, sum, tail, term;

    if(realCompareLessEqual(probability, const_1on4) || realCompareGreaterEqual(probability, const_3on4)) {
      if(realCompareLessThan(probability, const_1on2)) {
        realCopy(probability, &tail);
      }
      else {
        realSubtract(const_1, probability, &tail, realContext);
      }
      realMinus(&tail, &term, realContext);
      WP34S_Ln1P(&term, &sum, realContext);
      WP34S_Ln(&tail, &term, realContext);
      realSubtract(&sum, &term, res, realContext);
      if(realCompareLessThan(probability, const_1on2)) {
        realChangeSign(res);
      }
      return;
    }

    realAdd(probability, probability, &term, realContext);
    realSubtract(&term, const_1, &term, realContext);
    realMultiply(&term, &term, &square, realContext);
    realCopy(&term, &sum);
    int32ToReal(3, &divisor);
    for(int32_t n = 0; n < 1000; n++) {
      realMultiply(&term, &square, &term, realContext);
      realDivide(&term, &divisor, &addend, realContext);
      realCopy(&sum, &previous);
      realAdd(&sum, &addend, &sum, realContext);
      if(realCompareEqual(&sum, &previous)) {
        break;
      }
      realAdd(&divisor, const_2, &divisor, realContext);
    }
    realMultiply(&sum, const_2, res, realContext);
  }

  /********************************************//**
   * \brief Computes log(p/(1-p)) at an extended precision.
   ***********************************************/
  static void logisticLogitExtended(const real_t *probability, real_t *res, realContext_t *realContext) {
    REAL_T_PTR(addend, realContext->digits);
    REAL_T_PTR(divisor, realContext->digits);
    REAL_T_PTR(logOneMinusTail, realContext->digits);
    REAL_T_PTR(logTail, realContext->digits);
    REAL_T_PTR(oneMinusTail, realContext->digits);
    REAL_T_PTR(previous, realContext->digits);
    REAL_T_PTR(square, realContext->digits);
    REAL_T_PTR(sum, realContext->digits);
    REAL_T_PTR(tail, realContext->digits);
    REAL_T_PTR(term, realContext->digits);
    if(realCompareLessEqual(probability, const_1on4) || realCompareGreaterEqual(probability, const_3on4)) {
      if(realCompareLessThan(probability, const_1on2)) {
        realCopy(probability, tail);
      }
      else {
        realSubtract(const_1, probability, tail, realContext);
      }
      realSubtract(const_1, tail, oneMinusTail, realContext);
      WP34S_LnExtended(oneMinusTail, logOneMinusTail, realContext);
      WP34S_LnExtended(tail, logTail, realContext);
      realSubtract(logOneMinusTail, logTail, res, realContext);
      if(realCompareLessThan(probability, const_1on2)) {
        realChangeSign(res);
      }
      return;
    }

    realAdd(probability, probability, term, realContext);
    realSubtract(term, const_1, term, realContext);
    realMultiply(term, term, square, realContext);
    realCopy(term, sum);
    int32ToReal(3, divisor);
    for(int32_t n = 0; n < 1000; n++) {
      realMultiply(term, square, term, realContext);
      realDivide(term, divisor, addend, realContext);
      realCopy(sum, previous);
      realAdd(sum, addend, sum, realContext);
      if(realCompareEqual(sum, previous)) {
        break;
      }
      realAdd(divisor, const_2, divisor, realContext);
    }
    realMultiply(sum, const_2, res, realContext);
  }

  /********************************************//**
   * \brief Computes a logistic quantile and the terms of its closing sum.
   ***********************************************/
  static void logisticQuantileAtContext(const real_t *probability, const real_t *mu, const real_t *s, real_t *logit, real_t *scaled, real_t *value,
                                        realContext_t *realContext) {
    logisticLogit(probability, logit, realContext);
    realMultiply(logit, s, scaled, realContext);
    realFMA(logit, s, mu, value, realContext);
  }

  /********************************************//**
   * \brief Recomputes a logistic quantile after deep location-scale cancellation.
   ***********************************************/
  static void logisticQuantileExtended(const real_t *probability, const real_t *mu, const real_t *s, real_t *res, realContext_t *resultContext) {
    REAL_T_PTR(logit, 159);
    REAL_T_PTR(value, 159);
    realContext_t realContext = ctxtReal75;

    realContext.digits = 159;
    realContext.status = 0;
    logisticLogitExtended(probability, logit, &realContext);
    realFMA(logit, s, mu, value, &realContext);
    realPlus(value, res, resultContext);
  }


  /******************************************************
   * This functions are borrowed from the WP34S project
   ******************************************************/

  void WP34S_Pdf_Logit(const real_t *x, const real_t *mu, const real_t *s, real_t *res, realContext_t *realContext) {
    real_t denominator, exponential, onePlusExponential, standard, value;
    realContext_t *calculationContext;

    if(realIsNaN(x) || !logisticParametersAreValid(mu, s)) {
      realSetNaN(res);
      return;
    }
    if(realIsInfinite(x)) {
      realSetZero(res);
      return;
    }
    calculationContext = logisticForwardContext(x, mu, s, &standard, realContext);
    realSetPositiveSign(&standard);
    realChangeSign(&standard);
    realExp(&standard, &exponential, calculationContext);
    if(realIsZero(&exponential)) {
      realNextToward(&exponential, const_1, &exponential, calculationContext);
    }
    realAdd(const_1, &exponential, &onePlusExponential, calculationContext);
    realMultiply(&onePlusExponential, &onePlusExponential, &denominator, calculationContext);
    realMultiply(&denominator, s, &denominator, calculationContext);
    realDivide(&exponential, &denominator, &value, calculationContext);
    if(realIsZero(&value)) {
      realNextToward(&value, const_1, &value, calculationContext);
    }
    realPlus(&value, res, realContext);
    if(realIsZero(res) && !realIsZero(&value)) {
      realNextToward(res, const_1, res, realContext);
    }
  }

  /********************************************//**
   * \brief Computes either logistic probability from the smaller tail.
   ***********************************************/
  static void logisticCdfCommon(const real_t *x, const real_t *mu, const real_t *s, bool_t upper, real_t *res, realContext_t *realContext) {
    real_t exponential, small, standard, value;
    realContext_t *calculationContext;
    bool_t smallResult;

    if(realIsNaN(x) || !logisticParametersAreValid(mu, s)) {
      realSetNaN(res);
      return;
    }
    if(realIsInfinite(x)) {
      realCopy(realIsNegative(x) != upper ? const_0 : const_1, res);
      return;
    }
    calculationContext = logisticForwardContext(x, mu, s, &standard, realContext);
    smallResult = realIsNegative(&standard) != upper;
    realCopy(&standard, &exponential);
    realSetPositiveSign(&exponential);
    realChangeSign(&exponential);
    realExp(&exponential, &exponential, calculationContext);
    if(realIsZero(&exponential)) {
      realNextToward(&exponential, const_1, &exponential, calculationContext);
    }
    realAdd(const_1, &exponential, &standard, calculationContext);
    realDivide(&exponential, &standard, &small, calculationContext);
    if(smallResult) {
      realCopy(&small, &value);
    }
    else {
      realSubtract(const_1, &small, &value, calculationContext);
      if(realCompareEqual(&value, const_1) && !realIsZero(&small)) {
        realNextToward(&value, const_0, &value, calculationContext);
      }
    }
    realPlus(&value, res, realContext);
    if(smallResult && realIsZero(res) && !realIsZero(&value)) {
      realNextToward(res, const_1, res, realContext);
    }
    else if(!smallResult && realCompareEqual(res, const_1) && !realCompareEqual(&value, const_1)) {
      realNextToward(res, const_0, res, realContext);
    }
  }

  void WP34S_Cdfu_Logit(const real_t *x, const real_t *mu, const real_t *s, real_t *res, realContext_t *realContext) {
    logisticCdfCommon(x, mu, s, true, res, realContext);
  }

  void WP34S_Cdf_Logit(const real_t *x, const real_t *mu, const real_t *s, real_t *res, realContext_t *realContext) {
    logisticCdfCommon(x, mu, s, false, res, realContext);
  }

  void WP34S_Qf_Logit(const real_t *x, const real_t *mu, const real_t *s, real_t *res, realContext_t *realContext) {
    real_t logit, scaled, value;

    if(realIsNaN(x) || !logisticParametersAreValid(mu, s) || realCompareLessThan(x, const_0) || realCompareGreaterThan(x, const_1)) {
      realSetNaN(res);
      return;
    }
    if(realIsZero(x) || realCompareEqual(x, const_1)) {
      realSetPlusInfinity(res);
      if(realIsZero(x)) {
        realSetNegativeSign(res);
      }
      return;
    }
    if(realCompareEqual(x, const_1on2)) {
      realCopy(mu, res);
      return;
    }

    logisticQuantileAtContext(x, mu, s, &logit, &scaled, &value, &ctxtReal39);
    if(WP34S_HasCancellation(mu, &scaled, &value, 1)) {
      logisticQuantileAtContext(x, mu, s, &logit, &scaled, &value, &ctxtReal51);
      if(WP34S_HasCancellation(mu, &scaled, &value, 13)) {
        logisticQuantileAtContext(x, mu, s, &logit, &scaled, &value, &ctxtReal75);
        if(WP34S_HasCancellation(mu, &scaled, &value, 37)) {
          logisticQuantileExtended(x, mu, s, res, realContext);
        }
        else {
          realPlus(&value, res, realContext);
        }
      }
      else {
        realPlus(&value, res, realContext);
      }
    }
    else {
      realPlus(&value, res, realContext);
    }
  }

#endif // !OPTION_DIST_LOGIS
