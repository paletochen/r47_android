// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file weibull.c
 ***********************************************/

#include "c47.h"

#if !defined(OPTION_DIST_WEIBL)
  void fnWeibullP     (uint16_t unusedButMandatoryParameter){}
  void fnWeibullL     (uint16_t unusedButMandatoryParameter){}
  void fnWeibullR     (uint16_t unusedButMandatoryParameter){}
  void fnWeibullI     (uint16_t unusedButMandatoryParameter){}

#else
  bool_t weibullCheckDomain(bool_t registersValid, real_t *x, real_t *shape, real_t *scale, real_t *unusedThird, uint16_t unusedParameter) {
    if(!registersValid) {
      return false;
    }

    if(realIsNaN(x)) {
      if(!getSystemFlag(FLAG_SPCRES)) {
        displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function weibullCheckDomain:", "x is NaN", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      return false;
    }
    else if(realIsSpecial(shape) || realCompareLessEqual(shape, const_0) || realIsSpecial(scale) || realCompareLessEqual(scale, const_0)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function weibullCheckDomain:", "cannot calculate for b " STD_LESS_EQUAL " 0 or t " STD_LESS_EQUAL " 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    return true;
  }


  void fnWeibullP(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_WEIB * 4 + DIST_P);
  }

  void fnWeibullL(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_WEIB * 4 + DIST_L);
  }

  void fnWeibullR(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_WEIB * 4 + DIST_R);
  }

  void fnWeibullI(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_WEIB * 4 + DIST_I);
  }



  /******************************************************
   * This functions are borrowed from the WP34S project
   ******************************************************/

  /********************************************//**
   * \brief Checks a Weibull shape or scale parameter.
   ***********************************************/
  /********************************************//**
   * \brief Calculates log(x / scale) without forming an unrepresentable ratio.
   ***********************************************/
  static void weibullLogRatio(const real_t *x, const real_t *scale, real_t *logRatio, realContext_t *realContext) {
    REAL_T_PTR(delta, 75);

    realSubtract(x, scale, logRatio, realContext);
    realDivide(logRatio, scale, logRatio, realContext);
    realMinus(const_1on2, delta, realContext);
    if(!realIsSpecial(logRatio) && realCompareGreaterEqual(logRatio, delta)) {
      WP34S_Ln1P(logRatio, logRatio, realContext);
    }
    else {
      WP34S_Ln(x, logRatio, realContext);
      WP34S_Ln(scale, delta, realContext);
      realSubtract(logRatio, delta, logRatio, realContext);
    }
  }


  /********************************************//**
   * \brief Calculates log((x / scale)^shape).
   ***********************************************/
  static void weibullLogPower(const real_t *x, const real_t *scale, const real_t *shape, real_t *logPower, realContext_t *realContext) {
    weibullLogRatio(x, scale, logPower, realContext);
    realMultiply(logPower, shape, logPower, realContext);
  }


  void WP34S_Pdf_Weib(const real_t *x, const real_t *b, const real_t *t, real_t *res, realContext_t *realContext) {
    REAL_T_PTR(logPower, 75);
    REAL_T_PTR(logDensity, 75);
    REAL_T_PTR(work, 75);

    if(realIsNaN(x) || !WP34S_IsPositiveFinite(b) || !WP34S_IsPositiveFinite(t)) {
      realSetNaN(res);
      return;
    }
    if(realCompareLessThan(x, const_0) || realIsInfinite(x)) {
      realSetZero(res);
      return;
    }
    if(realIsZero(x)) {
      if(realCompareLessThan(t, const_1)) {
        realSetPlusInfinity(res);
      }
      else if(realCompareEqual(t, const_1)) {
        realDivide(const_1, b, res, realContext);
        WP34S_MarkPositive(res, realContext);
      }
      else {
        realSetZero(res);
      }
      return;
    }

    weibullLogPower(x, b, t, logPower, realContext);
    if(realIsInfinite(logPower)) {
      realSetZero(res);
      WP34S_MarkPositive(res, realContext);
      return;
    }
    WP34S_Ln(t, logDensity, realContext);
    WP34S_Ln(x, work, realContext);
    realSubtract(logDensity, work, logDensity, realContext);
    realAdd(logDensity, logPower, logDensity, realContext);
    realExp(logPower, work, realContext);
    if(realIsInfinite(work)) {
      realSetZero(res);
      WP34S_MarkPositive(res, realContext);
      return;
    }
    realSubtract(logDensity, work, logDensity, realContext);
    realExp(logDensity, res, realContext);
    WP34S_MarkPositive(res, realContext);
  }

  void WP34S_Cdfu_Weib(const real_t *x, const real_t *b, const real_t *t, real_t *res, realContext_t *realContext) {
    REAL_T_PTR(logPower, 75);
    REAL_T_PTR(power, 75);

    if(realIsNaN(x) || !WP34S_IsPositiveFinite(b) || !WP34S_IsPositiveFinite(t)) {
      realSetNaN(res);
      return;
    }
    if(realCompareLessEqual(x, const_0)) {
      realSetOne(res);
      return;
    }
    if(realIsInfinite(x)) {
      realSetZero(res);
      return;
    }
    weibullLogPower(x, b, t, logPower, realContext);
    realExp(logPower, power, realContext);
    realChangeSign(power);
    realExp(power, res, realContext);
    WP34S_MarkProbability(res, realContext);
  }

  void WP34S_Cdf_Weib(const real_t *x, const real_t *b, const real_t *t, real_t *res, realContext_t *realContext) {
    REAL_T_PTR(logPower, 75);
    REAL_T_PTR(power, 75);

    if(realIsNaN(x) || !WP34S_IsPositiveFinite(b) || !WP34S_IsPositiveFinite(t)) {
      realSetNaN(res);
      return;
    }
    if(realCompareLessEqual(x, const_0)) {
      realSetZero(res);
      return;
    }
    if(realIsInfinite(x)) {
      realSetOne(res);
      return;
    }
    weibullLogPower(x, b, t, logPower, realContext);
    realExp(logPower, power, realContext);
    realChangeSign(power);
    WP34S_ExpM1(power, res, realContext);
    realChangeSign(res);
    WP34S_MarkProbability(res, realContext);
  }

  void WP34S_Qf_Weib(const real_t *x, const real_t *b, const real_t *t, real_t *res, realContext_t *realContext) {
    REAL_T_PTR(p, 75);
    REAL_T_PTR(logValue, 75);

    if(realIsNaN(x) || !WP34S_IsPositiveFinite(b) || !WP34S_IsPositiveFinite(t) || realCompareLessThan(x, const_0) || realCompareGreaterThan(x, const_1)) {
      realSetNaN(res);
      return;
    }
    if(realIsZero(x)) {
      realSetZero(res);
      return;
    }
    if(realCompareEqual(x, const_1)) {
      realSetPlusInfinity(res);
      return;
    }
    realMinus(x, p, realContext);
    WP34S_Ln1P(p, p, realContext);
    realChangeSign(p);
    WP34S_Ln(p, logValue, realContext);
    realDivide(logValue, t, logValue, realContext);
    WP34S_Ln(b, p, realContext);
    realAdd(logValue, p, logValue, realContext);
    realExp(logValue, res, realContext);
    WP34S_MarkPositive(res, realContext);
  }

#endif // !OPTION_DIST_WEIBL
