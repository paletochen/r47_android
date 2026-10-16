// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file binomial.c
 ***********************************************/

#include "c47.h"

#if !defined(OPTION_DIST_BINOM)
  void fnBinomialP           (uint16_t unusedButMandatoryParameter){}
  void fnBinomialL           (uint16_t unusedButMandatoryParameter){}
  void fnBinomialR           (uint16_t unusedButMandatoryParameter){}
  void fnBinomialI           (uint16_t unusedButMandatoryParameter){}
  //void WP34S_Pdf_Binomial (const real_t *x, const real_t *p0, const real_t *n, real_t *res, realContext_t *realContext){}
  //void WP34S_Cdfu_Binomial(const real_t *x, const real_t *p0, const real_t *n, real_t *res, realContext_t *realContext){}
  //void WP34S_Cdf_Binomial (const real_t *x, const real_t *p0, const real_t *n, real_t *res, realContext_t *realContext){}
  //void WP34S_Cdf_Binomial2(const real_t *x, const real_t *p0, const real_t *n, real_t *res, realContext_t *realContext){}
  //void WP34S_Qf_Binomial  (const real_t *x, const real_t *p0, const real_t *n, real_t *res, realContext_t *realContext){}

#else
  bool_t binomialCheckDomain(bool_t registersValid, real_t *x, real_t *i, real_t *j, real_t *unusedThird, uint16_t unusedParameter) {
    if(!registersValid) {
      return false;
    }

    if(!checkRegisterNoFP(j) || realIsSpecial(j)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function binomialCheckDomain:", "n is not a finite integer", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    else if(realIsNaN(x)) {
      if(!getSystemFlag(FLAG_SPCRES)) {
        displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function binomialCheckDomain:", "x is NaN", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      return false;
    }
    else if(realIsSpecial(i) || realCompareLessThan(i, const_0) || realCompareGreaterThan(i, const_1) || realCompareLessThan(j, const_0)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function binomialCheckDomain:", "the parameters must be 0 " STD_LESS_EQUAL " p " STD_LESS_EQUAL " 1 and n " STD_GREATER_EQUAL " 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    return true;
  }


  void fnBinomialP(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_BINOM * 4 + DIST_P);
  }


  void fnBinomialL(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_BINOM * 4 + DIST_L);
  }


  void fnBinomialR(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_BINOM * 4 + DIST_R);
  }


  void fnBinomialI(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_BINOM * 4 + DIST_I);
  }



  /******************************************************
   * This functions are borrowed from the WP34S project
   ******************************************************/

  /********************************************//**
   * \brief Checks the complete binomial parameter pair.
   ***********************************************/
  static bool_t binomialParametersAreValid(const real_t *p, const real_t *n) {
    return !realIsSpecial(p) && !realIsSpecial(n) && realCompareGreaterEqual(p, const_0) &&
           realCompareLessEqual(p, const_1) && realCompareGreaterEqual(n, const_0) && realIsAnInteger(n);
  }


  void WP34S_Pdf_Binomial(const real_t *x, const real_t *p0, const real_t *n, real_t *res, realContext_t *realContext) {
    real_t a, b, logP, logQ, logit, logMass, nPlusOne, q0, value;
    realContext_t *calculationContext = &ctxtReal51;

    if(realIsNaN(x) || !binomialParametersAreValid(p0, n)) {
      realSetNaN(res);
      return;
    }
    if(realCompareLessThan(x, const_0) || realCompareGreaterThan(x, n)) {
      realSetZero(res);
      return;
    }
    if(realIsZero(n)) {
      realCopy(realIsZero(x) ? const_1 : const_0, res);
      return;
    }
    if(realIsZero(p0)) {
      realCopy(realIsZero(x) ? const_1 : const_0, res);
      return;
    }
    if(realCompareEqual(p0, const_1)) {
      realCopy(realCompareEqual(x, n) ? const_1 : const_0, res);
      return;
    }

    realAdd(x, const_1, &a, calculationContext);
    realSubtract(n, x, &b, calculationContext);
    realAdd(&b, const_1, &b, calculationContext);
    WP34S_DistributionProbabilityData(p0, &q0, &logP, &logQ, &logit, calculationContext);
    if(realIsZero(x)) {
      realMultiply(n, &logQ, &logMass, calculationContext);
    }
    else if(realCompareEqual(x, n)) {
      realMultiply(n, &logP, &logMass, calculationContext);
    }
    else {
      distributionLogBetaPrefix(&a, &b, p0, &q0, &logit, &logMass, calculationContext);
      realSubtract(&logMass, &logP, &logMass, calculationContext);
      realSubtract(&logMass, &logQ, &logMass, calculationContext);
      realAdd(n, const_1, &nPlusOne, calculationContext);
      WP34S_Ln(&nPlusOne, &value, calculationContext);
      realSubtract(&logMass, &value, &logMass, calculationContext);
    }
    realExp(&logMass, &value, calculationContext);
    if(realIsZero(&value)) {
      realNextToward(&value, const_1, &value, calculationContext);
    }
    realPlus(&value, res, realContext);
  }

  void WP34S_Cdfu_Binomial(const real_t *x, const real_t *p0, const real_t *n, real_t *res, realContext_t *realContext) {
    real_t a, b, logP, logQ, logit, lower, p, q0, upper;
    realContext_t *calculationContext;

    if(realIsNaN(x) || !binomialParametersAreValid(p0, n)) {
      realSetNaN(res);
      return;
    }
    realToIntegralValue(x, &p, DEC_ROUND_CEILING, realContext);
    if(realCompareLessEqual(&p, const_0)) {
      realSetOne(res);
      return;
    }
    if(realCompareGreaterThan(&p, n) || realIsZero(p0)) {
      realSetZero(res);
      return;
    }
    if(realCompareEqual(p0, const_1)) {
      realSetOne(res);
      return;
    }

    realCopy(&p, &a);
    realSubtract(n, &p, &b, &ctxtReal51);
    realAdd(&b, const_1, &b, &ctxtReal51);
    calculationContext = WP34S_BetaCalculationContext(&a, &b);
    realCopy(&p, &a);
    realSubtract(n, &p, &b, calculationContext);
    realAdd(&b, const_1, &b, calculationContext);
    WP34S_DistributionProbabilityData(p0, &q0, &logP, &logQ, &logit, calculationContext);
    if(!distributionBetaPair(&a, &b, p0, &q0, &logit, &lower, &upper, calculationContext)) {
      realSetNaN(res);
      return;
    }
    WP34S_MarkProbability(&lower, calculationContext);
    realPlus(&lower, res, realContext);
  }

  void WP34S_Cdf_Binomial(const real_t *x, const real_t *p0, const real_t *n, real_t *res, realContext_t *realContext) {
    real_t p;

    if(realIsNaN(x) || !binomialParametersAreValid(p0, n)) {
      realSetNaN(res);
      return;
    }
    realToIntegralValue(x, &p, DEC_ROUND_FLOOR, realContext);
    WP34S_Cdf_Binomial2(&p, p0, n, res, realContext);
  }

  void WP34S_Cdf_Binomial2(const real_t *x, const real_t *p0, const real_t *n, real_t *res, realContext_t *realContext) {
    real_t a, b, logP, logQ, logit, lower, q0, upper;
    realContext_t *calculationContext;

    if(realIsNaN(x) || !binomialParametersAreValid(p0, n)) {
      realSetNaN(res);
      return;
    }
    if(realCompareLessThan(x, const_0)) {
      realSetZero(res);
      return;
    }
    if(realCompareGreaterEqual(x, n) || realIsZero(p0)) {
      realSetOne(res);
      return;
    }
    if(realCompareEqual(p0, const_1)) {
      realSetZero(res);
      return;
    }

    realSubtract(n, x, &a, &ctxtReal51);
    realAdd(x, const_1, &b, &ctxtReal51);
    calculationContext = WP34S_BetaCalculationContext(&a, &b);
    realSubtract(n, x, &a, calculationContext);
    realAdd(x, const_1, &b, calculationContext);
    WP34S_DistributionProbabilityData(p0, &q0, &logP, &logQ, &logit, calculationContext);
    realChangeSign(&logit);
    if(!distributionBetaPair(&a, &b, &q0, p0, &logit, &lower, &upper, calculationContext)) {
      realSetNaN(res);
      return;
    }
    WP34S_MarkProbability(&lower, calculationContext);
    realPlus(&lower, res, realContext);
  }

  void WP34S_Qf_Binomial(const real_t *x, const real_t *p0, const real_t *n, real_t *res, realContext_t *realContext) {
    real_t estimate, mean, q, standardDeviation, variance;

    if(!binomialParametersAreValid(p0, n) || realIsNaN(x)) {
      realSetNaN(res);
      return;
    }
    if(realCompareLessEqual(x, const_0)) {
      realSetZero(res);
      return;
    }
    if(realCompareGreaterEqual(x, const_1) || realCompareEqual(p0, const_1)) {
      realCopy(n, res);
      return;
    }
    if(realIsZero(n) || realIsZero(p0)) {
      realSetZero(res);
      return;
    }

    realMultiply(p0, n, &mean, realContext);
    realSubtract(const_1, p0, &q, realContext);
    realMultiply(&q, &mean, &variance, realContext);
    realSquareRoot(&variance, &standardDeviation, realContext);
    WP34S_normal_moment_approx(x, &standardDeviation, &mean, &estimate, realContext);
    WP34S_Qf_Newton(QF_NEWTON_BINOMIAL, x, &estimate, p0, n, NULL, &mean, realContext);
    if(realIsNaN(&mean)) {
      realSetNaN(res);
    }
    else {
      realCopy(realCompareLessEqual(&mean, n) ? &mean : n, res);
    }
  }

#endif // !OPTION_DIST_BINOM
