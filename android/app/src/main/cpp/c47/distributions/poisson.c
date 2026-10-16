// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file poisson.c
 ***********************************************/

#include "c47.h"

#if !defined(OPTION_DIST_POISS)
  void fnPoissonP                (uint16_t unusedButMandatoryParameter){}
  void fnPoissonL                (uint16_t unusedButMandatoryParameter){}
  void fnPoissonR                (uint16_t unusedButMandatoryParameter){}
  void fnPoissonI                (uint16_t unusedButMandatoryParameter){}
  //void WP34S_normal_moment_approx(const real_t *prob, const real_t *var, const real_t *mean, real_t *res, realContext_t *realContext){}
  //void WP34S_Pdf_Poisson         (const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext){}
  //void WP34S_Cdfu_Poisson        (const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext){}
  //void WP34S_Cdf_Poisson         (const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext){}
  //void WP34S_Cdf_Poisson2        (const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext){}
  //void WP34S_Qf_Poisson          (const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext){}

#else
  bool_t poissonCheckDomain(bool_t registersValid, real_t *x, real_t *i, real_t *unusedSecond, real_t *unusedThird, uint16_t unusedParameter) {
    if(!registersValid) {
      return false;
    }

    if(realIsNaN(x)) {
      displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function poissonCheckDomain:", "x is NaN", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    else if(realIsSpecial(i) || realIsZero(i) || realIsNegative(i)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function poissonCheckDomain:", "the parameter must be " STD_lambda " > 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    return true;
  }


  void fnPoissonP(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_POISSON * 4 + DIST_P);
  }


  void fnPoissonL(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_POISSON * 4 + DIST_L);
  }


  void fnPoissonR(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_POISSON * 4 + DIST_R);
  }


  void fnPoissonI(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_POISSON * 4 + DIST_I);
  }



  /******************************************************
   * This functions are borrowed from the WP34S project
   ******************************************************/

  /* Stack contains probability in Z, variance in Y and mean in X.
   * Returns a normal approximation in X.
   */
  void WP34S_normal_moment_approx(const real_t *prob, const real_t *var, const real_t *mean, real_t *res, realContext_t *realContext) {
    real_t p, q;

    WP34S_qf_q_est(prob, &p, NULL, realContext);
    realMultiply(&p, &p, &q, realContext);
    realSubtract(&q, const_1, &q, realContext);
    realDivide(&q, const_6, &q, realContext);
    realDivide(&q, var, &q, realContext);
    realAdd(&p, &q, &p, realContext);
    realMultiply(&p, var, &p, realContext);
    realAdd(&p, mean, res, realContext);
  }

  /* One parameter Poisson distribution
   * Real parameter lambda in I.
   */
  static void poissonLogDensity(const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext) {
    real_t argument, sum, r, delta, logarithm, term;

    if(realCompareLessThan(x, const_205)) {
      WP34S_Ln(lambda, &logarithm, realContext);
      realMultiply(&logarithm, x, res, realContext);
      realSubtract(res, lambda, res, realContext);
      realAdd(x, const_1, &argument, realContext);
      WP34S_LnGamma(&argument, &term, realContext);
      realSubtract(res, &term, res, realContext);
      return;
    }

    WP34S_LanczosSum(x, const_29, &sum, realContext);
    WP34S_Ln(&sum, &sum, realContext);
    realAdd(x, const_gammaR, &r, realContext);
    realSubtract(lambda, &r, &delta, realContext);
    realDivide(&delta, &r, &delta, realContext);
    if(realCompareAbsLessThan(&delta, const_1e_6)) {
      WP34S_Log1pMinus(&delta, &term, realContext);
      realMultiply(x, &term, res, realContext);
      realMultiply(const_gammaR, &delta, &term, realContext);
      realSubtract(res, &term, res, realContext);
    }
    else {
      realDivide(lambda, &r, &logarithm, realContext);
      WP34S_Ln(&logarithm, &logarithm, realContext);
      realMultiply(x, &logarithm, res, realContext);
      realSubtract(&r, lambda, &term, realContext);
      realAdd(res, &term, res, realContext);
    }
    WP34S_Ln(&r, &term, realContext);
    realMultiply(&term, const_1on2, &term, realContext);
    realSubtract(res, &term, res, realContext);
    realSubtract(res, &sum, res, realContext);
  }


  void WP34S_Pdf_Poisson(const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext) {
    real_t logarithm, value;
    realContext_t *calculationContext;

    if(!WP34S_IsPositiveFinite(lambda) || realIsNaN(x)) {
      realSetNaN(res);
      return;
    }
    if((realIsNegative(x) && !realIsZero(x)) || !realIsAnInteger(x) || realIsInfinite(x)) {
      realSetZero(res);
      return;
    }
    calculationContext = realCompareLessThan(x, const_205) ? WP34S_AtLeastContext(realContext, &ctxtReal39) : WP34S_AtLeastContext(realContext, &ctxtReal51);
    poissonLogDensity(x, lambda, &logarithm, calculationContext);
    realExp(&logarithm, &value, calculationContext);
    WP34S_AdjustPositiveUnderflow(&value);
    realPlus(&value, res, realContext);
  }


  static void poissonCdfAtInteger(const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext) {
    real_t shape, difference, previous, logarithm, mass;

    if(realCompareLessThan(x, const_0)) {
      realSetZero(res);
      return;
    }
    if(realIsInfinite(x)) {
      realSetOne(res);
      return;
    }
    realAdd(x, const_1, &shape, realContext);
    realSubtract(&shape, x, &difference, realContext);
    if(realCompareEqual(&difference, const_1)) {
      WP34S_GammaP(lambda, &shape, res, realContext, true, true);
    }
    else {
      WP34S_GammaP(lambda, x, &previous, realContext, true, true);
      poissonLogDensity(x, lambda, &logarithm, &ctxtReal51);
      realExp(&logarithm, &mass, &ctxtReal51);
      realAdd(&previous, &mass, res, realContext);
    }
  }


  static void poissonMarkInterior(real_t *value, realContext_t *realContext) {
    WP34S_AdjustPositiveUnderflow(value);
    if(realCompareGreaterEqual(value, const_1)) {
      realCopy(const_1, value);
      realNextToward(value, const_0, value, realContext);
    }
  }


  static void poissonMarkCentralSide(const real_t *integer, const real_t *lambda, real_t *value, realContext_t *realContext) {
    if(realCompareEqual(integer, lambda) && realCompareEqual(value, const_1on2)) {
      realNextToward(value, const_1, value, realContext);
    }
  }


  void WP34S_Cdfu_Poisson(const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext) {
    real_t integer, value;
    realContext_t *calculationContext = WP34S_AtLeastContext(realContext, &ctxtReal39);

    if(!WP34S_IsPositiveFinite(lambda) || realIsNaN(x)) {
      realSetNaN(res);
      return;
    }
    realToIntegralValue(x, &integer, DEC_ROUND_CEILING, calculationContext);
    if(realCompareLessThan(&integer, const_1)) {
      realSetOne(res);
      return;
    }
    if(realIsInfinite(&integer)) {
      realSetZero(res);
      return;
    }
    WP34S_GammaP(lambda, &integer, &value, calculationContext, false, true);
    poissonMarkCentralSide(&integer, lambda, &value, calculationContext);
    poissonMarkInterior(&value, calculationContext);
    realPlus(&value, res, realContext);
  }


  void WP34S_Cdf_Poisson(const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext) {
    real_t integer;

    if(!WP34S_IsPositiveFinite(lambda) || realIsNaN(x)) {
      realSetNaN(res);
      return;
    }
    realToIntegralValue(x, &integer, DEC_ROUND_FLOOR, WP34S_AtLeastContext(realContext, &ctxtReal39));
    WP34S_Cdf_Poisson2(&integer, lambda, res, realContext);
  }


  void WP34S_Cdf_Poisson2(const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext) {
    real_t value;
    realContext_t *calculationContext = WP34S_AtLeastContext(realContext, &ctxtReal39);

    if(!WP34S_IsPositiveFinite(lambda) || realIsNaN(x)) {
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
    poissonCdfAtInteger(x, lambda, &value, calculationContext);
    poissonMarkCentralSide(x, lambda, &value, calculationContext);
    poissonMarkInterior(&value, calculationContext);
    realPlus(&value, res, realContext);
  }


  void WP34S_Qf_Poisson(const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext) {
    real_t probability, low, high, width, midpoint, difference, tolerance, largeLambda, value;

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
    if(realCompareEqual(x, const_1on2) && realIsAnInteger(lambda)) {
      realCopy(lambda, res);
      return;
    }

    WP34S_SetPowerOfTen(74, &largeLambda);
    if(realCompareGreaterEqual(lambda, &largeLambda)) {
      if(realCompareEqual(x, const_1on2)) {
        realCopy(lambda, res);
      }
      else {
        realNextToward(lambda, realCompareGreaterThan(x, const_1on2) ? const_plusInfinity : const_0, &value, &ctxtReal39);
        realCopy(&value, res);
      }
      return;
    }

    poissonCdfAtInteger(const_0, lambda, &probability, &ctxtReal51);
    if(realCompareGreaterEqual(&probability, x)) {
      realSetZero(res);
      return;
    }
    realSetZero(&low);
    realToIntegralValue(lambda, &high, DEC_ROUND_CEILING, &ctxtReal39);
    if(realCompareLessThan(&high, const_1)) {
      realSetOne(&high);
    }
    for(int32_t iteration=0; iteration<32; iteration++) {
      poissonCdfAtInteger(&high, lambda, &probability, &ctxtReal51);
      if(realCompareGreaterEqual(&probability, x)) {
        break;
      }
      realCopy(&high, &low);
      realMultiply(&high, const_2, &high, &ctxtReal39);
      if(iteration == 31 || realIsSpecial(&high)) {
        realSetNaN(res);
        return;
      }
    }

    WP34S_SetPowerOfTen(-37, &tolerance);
    for(int32_t iteration=0; iteration<200; iteration++) {
      realSubtract(&high, &low, &width, &ctxtReal39);
      if(realCompareLessEqual(&width, const_1)) {
        realCopy(&high, res);
        return;
      }
      if(!realIsZero(&low) && WP34S_RelativeError(&low, &high, &tolerance, &ctxtReal39)) {
        realAdd(&low, &high, &value, &ctxtReal39);
        realMultiply(&value, const_1on2, &value, &ctxtReal39);
        realCopy(&value, res);
        return;
      }
      realAdd(&low, &high, &midpoint, &ctxtReal39);
      realMultiply(&midpoint, const_1on2, &midpoint, &ctxtReal39);
      realToIntegralValue(&midpoint, &midpoint, DEC_ROUND_FLOOR, &ctxtReal39);
      realSubtract(&midpoint, &low, &difference, &ctxtReal39);
      if(realIsZero(&difference)) {
        realCopy(&high, res);
        return;
      }
      poissonCdfAtInteger(&midpoint, lambda, &probability, &ctxtReal51);
      if(realCompareGreaterEqual(&probability, x)) {
        realCopy(&midpoint, &high);
      }
      else {
        realCopy(&midpoint, &low);
      }
    }

    realSetNaN(res);
  }

#endif // !OPTION_DIST_POISS
