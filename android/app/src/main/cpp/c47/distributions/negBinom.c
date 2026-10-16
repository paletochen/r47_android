// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file negBinom.c
 ***********************************************/

#include "c47.h"

#if !defined(OPTION_DIST_NBIN)
  void fnNegBinomialP  (uint16_t unusedButMandatoryParameter){}
  void fnNegBinomialL  (uint16_t unusedButMandatoryParameter){}
  void fnNegBinomialR  (uint16_t unusedButMandatoryParameter){}
  void fnNegBinomialI  (uint16_t unusedButMandatoryParameter){}

#else
  bool_t negativeBinomialCheckDomain(bool_t registersValid, real_t *x, real_t *i, real_t *j, real_t *unusedThird, uint16_t unusedParameter) {
    if(!registersValid) {
      return false;
    }

    if(!checkRegisterNoFP(j) || realIsSpecial(j)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function negativeBinomialCheckDomain:", "r is not a finite integer", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    else if(realIsNaN(x)) {
      if(!getSystemFlag(FLAG_SPCRES)) {
        displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function negativeBinomialCheckDomain:", "x is NaN", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      return false;
    }
    else if(realIsSpecial(i) || realCompareLessThan(i, const_0) || realCompareGreaterEqual(i, const_1) || realCompareLessEqual(j, const_0)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function negativeBinomialCheckDomain:", "the parameters must be 0 " STD_LESS_EQUAL " p < 1 and r > 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    return true;
  }


  void fnNegBinomialP(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_NEGBINOM * 4 + DIST_P);
  }


  void fnNegBinomialL(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_NEGBINOM * 4 + DIST_L);
  }


  void fnNegBinomialR(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_NEGBINOM * 4 + DIST_R);
  }


  void fnNegBinomialI(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_NEGBINOM * 4 + DIST_I);
  }


  /********************************************//**
   * \brief Checks the complete negative-binomial parameter pair.
   ***********************************************/
  static bool_t negativeBinomialParametersAreValid(const real_t *p, const real_t *r) {
    return !realIsSpecial(p) && !realIsSpecial(r) && realCompareGreaterEqual(p, const_0) && realCompareLessThan(p, const_1) &&
           realCompareGreaterThan(r, const_0) && realIsAnInteger(r);
  }



  // PDF[NB(r, p)](k) = [(k + r - 1) C k] p^k (1 - p)^r
  void pdf_NegBinomial(const real_t *x, const real_t *p0, const real_t *r, real_t *res, realContext_t *realContext) {
    real_t a, logit, logMass, logP, logQ, q0, term, value;
    realContext_t *calculationContext = &ctxtReal51;

    if(realIsNaN(x) || !negativeBinomialParametersAreValid(p0, r)) {
      realSetNaN(res);
      return;
    }
    if(realIsNegative(x) || realIsInfinite(x)) {
      realSetZero(res);
      return;
    }
    if(realIsZero(p0)) {
      realCopy(realIsZero(x) ? const_1 : const_0, res);
      return;
    }

    WP34S_DistributionProbabilityData(p0, &q0, &logP, &logQ, &logit, calculationContext);
    if(realIsZero(x)) {
      realMultiply(r, &logQ, &logMass, calculationContext);
    }
    else {
      realAdd(x, const_1, &a, calculationContext);
      distributionLogBetaPrefix(&a, r, p0, &q0, &logit, &logMass, calculationContext);
      realSubtract(&logMass, &logP, &logMass, calculationContext);
      realAdd(x, r, &term, calculationContext);
      WP34S_Ln(&term, &term, calculationContext);
      realSubtract(&logMass, &term, &logMass, calculationContext);
    }
    realExp(&logMass, &value, calculationContext);
    if(realIsZero(&value)) {
      realNextToward(&value, const_1, &value, calculationContext);
    }
    realPlus(&value, res, realContext);
  }

  // I[p](ceil(x), r)
  void cdfu_NegBinomial(const real_t *x, const real_t *p0, const real_t *r, real_t *res, realContext_t *realContext) {
    real_t logit, logP, logQ, lower, p, q0, upper;
    realContext_t *calculationContext;

    if(realIsNaN(x) || !negativeBinomialParametersAreValid(p0, r)) {
      realSetNaN(res);
      return;
    }
    realToIntegralValue(x, &p, DEC_ROUND_CEILING, realContext);
    if(realCompareLessThan(&p, const_1)) {
      realSetOne(res);
      return;
    }
    if(realIsInfinite(&p)) {
      realSetZero(res);
      return;
    }
    if(realIsZero(p0)) {
      realSetZero(res);
      return;
    }

    calculationContext = WP34S_BetaCalculationContext(&p, r);
    WP34S_DistributionProbabilityData(p0, &q0, &logP, &logQ, &logit, calculationContext);
    if(!distributionBetaPair(&p, r, p0, &q0, &logit, &lower, &upper, calculationContext)) {
      realSetNaN(res);
      return;
    }
    WP34S_MarkProbability(&lower, calculationContext);
    realPlus(&lower, res, realContext);
  }

  // I[1-p](r, floor(x) + 1)
  void cdf_NegBinomial(const real_t *x, const real_t *p0, const real_t *r, real_t *res, realContext_t *realContext) {
    real_t p;

    if(realIsNaN(x) || !negativeBinomialParametersAreValid(p0, r)) {
      realSetNaN(res);
      return;
    }
    realToIntegralValue(x, &p, DEC_ROUND_FLOOR, realContext);
    cdf_NegBinomial2(&p, p0, r, res, realContext);
  }

  void cdf_NegBinomial2(const real_t *x, const real_t *p0, const real_t *r, real_t *res, realContext_t *realContext) {
    real_t b, logit, logP, logQ, lower, q0, upper;
    realContext_t *calculationContext;

    if(realIsNaN(x) || !negativeBinomialParametersAreValid(p0, r)) {
      realSetNaN(res);
      return;
    }
    if(realCompareLessThan(x, const_0)) {
      realSetZero(res);
      return;
    }
    if(realIsInfinite(x)) {
      realSetOne(res);
      return;
    }
    if(realIsZero(p0)) {
      realSetOne(res);
      return;
    }

    realAdd(x, const_1, &b, &ctxtReal51);
    calculationContext = WP34S_BetaCalculationContext(r, &b);
    realAdd(x, const_1, &b, calculationContext);
    WP34S_DistributionProbabilityData(p0, &q0, &logP, &logQ, &logit, calculationContext);
    realChangeSign(&logit);
    if(!distributionBetaPair(r, &b, &q0, p0, &logit, &lower, &upper, calculationContext)) {
      realSetNaN(res);
      return;
    }
    WP34S_MarkProbability(&lower, calculationContext);
    realPlus(&lower, res, realContext);
  }

  void qf_NegBinomial(const real_t *x, const real_t *p0, const real_t *r, real_t *res, realContext_t *realContext) {
    real_t estimate, mean, pr, q0, standardDeviation, variance;

    if(realIsNaN(x) || !negativeBinomialParametersAreValid(p0, r)) {
      realSetNaN(res);
      return;
    }
    if(realCompareLessEqual(x, const_0)) {
      realSetZero(res);
      return;
    }
    if(realIsZero(p0)) {
      realSetZero(res);
      return;
    }
    if(realCompareGreaterEqual(x, const_1)) {
      realSetPlusInfinity(res);
      return;
    }

    realSubtract(const_1, p0, &q0, realContext); // 1 - p
    realMultiply(p0, r, &pr, realContext);
    realDivide(&pr, &q0, &mean, realContext);    // mean = pr/(1-p)

    realDivide(&mean, &q0, &variance, realContext); // variance = pr/((1-p)^2)
    realSquareRoot(&variance, &standardDeviation, realContext);

    WP34S_normal_moment_approx(x, &standardDeviation, &mean, &estimate, realContext);
    WP34S_Qf_Newton(QF_NEWTON_NEGBINOM, x, &estimate, p0, r, NULL, res, realContext);
  }

#endif // !OPTION_DIST_NBIN
