// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file exponential.c
 ***********************************************/

#include "c47.h"

#if !defined(OPTION_DIST_EXPON)
  void fnExponentialP  (uint16_t unusedButMandatoryParameter){}
  void fnExponentialL  (uint16_t unusedButMandatoryParameter){}
  void fnExponentialR  (uint16_t unusedButMandatoryParameter){}
  void fnExponentialI  (uint16_t unusedButMandatoryParameter){}

#else
  bool_t exponentialCheckDomain(bool_t registersValid, real_t *x, real_t *i, real_t *unusedSecond, real_t *unusedThird, uint16_t unusedParameter) {
    if(!registersValid) {
      return false;
    }

    if(realIsNaN(x)) {
      if(!getSystemFlag(FLAG_SPCRES)) {
        displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function exponentialCheckDomain:", "x is NaN", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      return false;
    }
    else if(realIsSpecial(i) || realCompareLessEqual(i, const_0)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function exponentialCheckDomain:", "the parameter must be " STD_lambda " > 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    return true;
  }


  void fnExponentialP(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_EXPON * 4 + DIST_P);
  }


  void fnExponentialL(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_EXPON * 4 + DIST_L);
  }


  void fnExponentialR(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_EXPON * 4 + DIST_R);
  }


  void fnExponentialI(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_EXPON * 4 + DIST_I);
  }



  /******************************************************
   * This functions are borrowed from the WP34S project
   ******************************************************/

  /********************************************//**
   * \brief Checks the exponential rate parameter.
   ***********************************************/
  void WP34S_Pdf_Expon(const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext) {
    REAL_T_PTR(scaled, 75);
    REAL_T_PTR(value, 75);

    if(realIsNaN(x) || !WP34S_IsPositiveFinite(lambda)) {
      realSetNaN(res);
      return;
    }
    if(realCompareLessThan(x, const_0) || realIsInfinite(x)) {
      realSetZero(res);
      return;
    }
    if(realIsZero(x)) {
      realCopy(lambda, res);
      return;
    }
    realMultiply(x, lambda, scaled, realContext);
    if(realCompareLessEqual(scaled, const_10000)) {
      realMinus(scaled, value, realContext);
      realExp(value, value, realContext);
      if(realCompareEqual(value, const_1)) {
        realNextToward(value, const_0, value, realContext);
      }
      realMultiply(value, lambda, res, realContext);
    }
    else {
      WP34S_Ln(lambda, value, realContext);
      realSubtract(value, scaled, value, realContext);
      realExp(value, res, realContext);
      WP34S_MarkPositive(res, realContext);
    }
  }

  void WP34S_Cdfu_Expon(const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext) {
    if(realIsNaN(x) || !WP34S_IsPositiveFinite(lambda)) {
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
    realMultiply(x, lambda, res, realContext);
    realChangeSign(res);
    realExp(res, res, realContext);
    WP34S_MarkProbability(res, realContext);
  }

  void WP34S_Cdf_Expon(const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext) {
    if(realIsNaN(x) || !WP34S_IsPositiveFinite(lambda)) {
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
    realMultiply(x, lambda, res, realContext);
    realChangeSign(res);
    WP34S_ExpM1(res, res, realContext);
    realChangeSign(res);
    WP34S_MarkProbability(res, realContext);
  }

  void WP34S_Qf_Expon(const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext) {
    REAL_T_PTR(p, 75);

    if(realIsNaN(x) || !WP34S_IsPositiveFinite(lambda) || realCompareLessThan(x, const_0) || realCompareGreaterThan(x, const_1)) {
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
    realDivide(p, lambda, res, realContext);
    WP34S_MarkPositive(res, realContext);
  }

#endif // !OPTION_DIST_EXPON
