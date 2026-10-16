// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file chi2.c
 ***********************************************/

#include "c47.h"

#if !defined(OPTION_DIST_CHI2)
  void fnChi2P(uint16_t unusedButMandatoryParameter){}
  void fnChi2L(uint16_t unusedButMandatoryParameter){}
  void fnChi2R(uint16_t unusedButMandatoryParameter){}
  void fnChi2I(uint16_t unusedButMandatoryParameter){}

#else
  bool_t checkRegisterNoFP(const real_t *reg) {
    real_t flooredI;

    realToIntegralValue(reg, &flooredI, DEC_ROUND_FLOOR, &ctxtReal39);
    return realCompareEqual(reg, &flooredI);
  }

  bool_t chi2CheckDomain(bool_t registersValid, real_t *x, real_t *i, real_t *unusedSecond, real_t *unusedThird, uint16_t unusedParameter) {
    if(!registersValid) {
      return false;
    }

    if(!checkRegisterNoFP(i) || realIsSpecial(i)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function chi2CheckDomain:", "k is not an integer", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    if(realIsNaN(x) || (realIsNegative(x) && !realIsZero(x))) {
      displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function chi2CheckDomain:", "cannot calculate for x < 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    if(realIsZero(i) || realIsNegative(i)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function chi2CheckDomain:", "cannot calculate for k " STD_LESS_EQUAL " 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    return true;
  }


  void fnChi2P(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_CHI2 * 4 + DIST_P);
  }


  void fnChi2L(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_CHI2 * 4 + DIST_L);
  }


  void fnChi2R(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_CHI2 * 4 + DIST_R);
  }


  void fnChi2I(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_CHI2 * 4 + DIST_I);
  }



  /******************************************************
   * This functions are borrowed from the WP34S project
   ******************************************************/

  static void chi2LogDensity(const real_t *x, const real_t *k, real_t *res, realContext_t *realContext) {
    real_t a, y, argument, sum, r, delta, logarithm, term, coefficient;

    realMultiply(k, const_1on2, &a, realContext);
    realMultiply(x, const_1on2, &y, realContext);
    realSubtract(&a, const_1, &argument, realContext);
    WP34S_LanczosSum(&argument, const_29, &sum, realContext);
    WP34S_Ln(&sum, &sum, realContext);
    realAdd(&a, const_gammaRm1, &r, realContext);
    realSubtract(&y, &r, &delta, realContext);
    realDivide(&delta, &r, &delta, realContext);

    if(realCompareAbsLessThan(&delta, const_1e_6)) {
      WP34S_Ln1P(&delta, &logarithm, realContext);
      WP34S_Log1pMinus(&delta, &term, realContext);
      realMultiply(&a, &term, res, realContext);
      realSubtract(res, &logarithm, res, realContext);
      realMultiply(const_gammaRm1, &delta, &term, realContext);
      realSubtract(res, &term, res, realContext);
    }
    else {
      realDivide(&y, &r, &logarithm, realContext);
      WP34S_Ln(&logarithm, &logarithm, realContext);
      realSubtract(&a, const_1, &coefficient, realContext);
      realMultiply(&coefficient, &logarithm, res, realContext);
      realSubtract(&r, &y, &term, realContext);
      realAdd(res, &term, res, realContext);
    }
    WP34S_Ln(&r, &term, realContext);
    realMultiply(&term, const_1on2, &term, realContext);
    realSubtract(res, &term, res, realContext);
    realSubtract(res, &sum, res, realContext);
    realSubtract(res, const75_ln2, res, realContext);
  }


  static void chi2ProbabilityWide(const real_t *x, const real_t *k, real_t *res, bool_t upper, realContext_t *realContext) {
    real_t argument, shape;

    realMultiply(x, const_1on2, &argument, realContext);
    realMultiply(k, const_1on2, &shape, realContext);
    WP34S_GammaP(&argument, &shape, res, realContext, upper, true);
  }


  static void chi2MarkInterior(real_t *value, realContext_t *realContext) {
    WP34S_AdjustPositiveUnderflow(value);
    if(realCompareEqual(value, const_1)) {
      realNextToward(value, const_0, value, realContext);
    }
  }


  void WP34S_Pdf_Chi2(const real_t *x, const real_t *k, real_t *res, realContext_t *realContext) {
    real_t logarithm, value;
    realContext_t *calculationContext = WP34S_AtLeastContext(realContext, &ctxtReal51);

    if(!WP34S_IsPositiveFinite(k) || realIsNaN(x) || (realIsNegative(x) && !realIsZero(x))) {
      realSetNaN(res);
      return;
    }
    if(realIsZero(x)) {
      if(realCompareLessThan(k, const_2)) {
        realSetPlusInfinity(res);
      }
      else if(realCompareEqual(k, const_2)) {
        realCopy(const_1on2, res);
      }
      else {
        realSetZero(res);
      }
      return;
    }
    if(realIsInfinite(x)) {
      realSetZero(res);
      return;
    }

    chi2LogDensity(x, k, &logarithm, calculationContext);
    realExp(&logarithm, &value, calculationContext);
    WP34S_AdjustPositiveUnderflow(&value);
    realPlus(&value, res, realContext);
  }


  static void chi2Probability(const real_t *x, const real_t *k, real_t *res, bool_t upper, realContext_t *realContext) {
    real_t value;
    realContext_t *calculationContext = WP34S_AtLeastContext(realContext, &ctxtReal39);

    if(!WP34S_IsPositiveFinite(k) || realIsNaN(x) || (realIsNegative(x) && !realIsZero(x))) {
      realSetNaN(res);
      return;
    }
    if(realIsZero(x)) {
      if(upper) {
        realSetOne(res);
      }
      else {
        realSetZero(res);
      }
      return;
    }
    if(realIsInfinite(x)) {
      if(upper) {
        realSetZero(res);
      }
      else {
        realSetOne(res);
      }
      return;
    }

    chi2ProbabilityWide(x, k, &value, upper, calculationContext);
    if(!realIsNaN(&value)) {
      chi2MarkInterior(&value, calculationContext);
    }
    realPlus(&value, res, realContext);
  }


  void WP34S_Cdfu_Chi2(const real_t *x, const real_t *k, real_t *res, realContext_t *realContext) {
    chi2Probability(x, k, res, true, realContext);
  }


  void WP34S_Cdf_Chi2(const real_t *x, const real_t *k, real_t *res, realContext_t *realContext) {
    chi2Probability(x, k, res, false, realContext);
  }


  static bool_t chi2InverseResidual(const real_t *logValue, const real_t *k, const real_t *target, bool_t upper, real_t *value, real_t *residual,
                                    real_t *derivative) {
    real_t probability, logDensity;

    realExp(logValue, value, &ctxtReal51);
    chi2ProbabilityWide(value, k, &probability, upper, &ctxtReal51);
    if(realIsNaN(&probability)) {
      return false;
    }
    if(upper) {
      realSubtract(target, &probability, residual, &ctxtReal51);
    }
    else {
      realSubtract(&probability, target, residual, &ctxtReal51);
    }
    chi2LogDensity(value, k, &logDensity, &ctxtReal51);
    realExp(&logDensity, derivative, &ctxtReal51);
    realMultiply(value, derivative, derivative, &ctxtReal51);
    return true;
  }


  static bool_t chi2InverseBracket(const real_t *initial, const real_t *k, const real_t *target, bool_t upper, real_t *low, real_t *high) {
    real_t value, residual, derivative, distance, proposal, lowBound, highBound;

    int32ToReal(-30000, &lowBound);
    int32ToReal(16000, &highBound);
    if(!chi2InverseResidual(initial, k, target, upper, &value, &residual, &derivative)) {
      return false;
    }
    if(realIsNegative(&residual)) {
      realCopy(initial, low);
      realDivide(const_2, k, &distance, &ctxtReal51);
      realSquareRoot(&distance, &distance, &ctxtReal51);
      for(int32_t iteration=0; iteration<20; iteration++) {
        realAdd(initial, &distance, &proposal, &ctxtReal51);
        if(realCompareGreaterThan(&proposal, &highBound)) {
          realCopy(&highBound, &proposal);
        }
        if(!chi2InverseResidual(&proposal, k, target, upper, &value, &residual, &derivative)) {
          return false;
        }
        if(!realIsNegative(&residual)) {
          realCopy(&proposal, high);
          return true;
        }
        realCopy(&proposal, low);
        if(realCompareEqual(&proposal, &highBound)) {
          return false;
        }
        realMultiply(&distance, const_2, &distance, &ctxtReal51);
      }
    }
    else {
      realCopy(initial, high);
      realDivide(const_2, k, &distance, &ctxtReal51);
      realSquareRoot(&distance, &distance, &ctxtReal51);
      for(int32_t iteration=0; iteration<20; iteration++) {
        realSubtract(initial, &distance, &proposal, &ctxtReal51);
        if(realCompareLessThan(&proposal, &lowBound)) {
          realCopy(&lowBound, &proposal);
        }
        if(!chi2InverseResidual(&proposal, k, target, upper, &value, &residual, &derivative)) {
          return false;
        }
        if(!realIsPositive(&residual) || realIsZero(&residual)) {
          realCopy(&proposal, low);
          return true;
        }
        realCopy(&proposal, high);
        if(realCompareEqual(&proposal, &lowBound)) {
          return false;
        }
        realMultiply(&distance, const_2, &distance, &ctxtReal51);
      }
    }
    return false;
  }


  void WP34S_Qf_Chi2(const real_t *x, const real_t *k, real_t *res, realContext_t *realContext) {
    real_t target, initial, low, high, logValue, value, residual, derivative, width, tolerance, step, proposal, midpoint, largeK;
    bool_t upper;

    if(realIsNaN(x) || !WP34S_IsPositiveFinite(k) || realCompareLessThan(x, const_0) || realCompareGreaterThan(x, const_1)) {
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
    WP34S_SetPowerOfTen(80, &largeK);
    if(realCompareGreaterEqual(k, &largeK)) {
      realNextToward(k, realCompareGreaterThan(x, const_1on2) ? const_plusInfinity : const_0, &value, &ctxtReal39);
      realPlus(&value, res, realContext);
      return;
    }

    upper = realCompareGreaterThan(x, const_1on2);
    if(upper) {
      realSubtract(const_1, x, &target, &ctxtReal51);
    }
    else {
      realCopy(x, &target);
    }
    WP34S_Ln(k, &initial, &ctxtReal51);
    if(!chi2InverseBracket(&initial, k, &target, upper, &low, &high)) {
      realSetNaN(res);
      return;
    }

    realSetOne(&tolerance);
    tolerance.exponent = -40;
    realAdd(&low, &high, &logValue, &ctxtReal51);
    realMultiply(&logValue, const_1on2, &logValue, &ctxtReal51);
    for(int32_t iteration=0; iteration<240; iteration++) {
      if(!chi2InverseResidual(&logValue, k, &target, upper, &value, &residual, &derivative)) {
        realSetNaN(res);
        return;
      }
      if(realIsNegative(&residual)) {
        realCopy(&logValue, &low);
      }
      else {
        realCopy(&logValue, &high);
      }
      realSubtract(&high, &low, &width, &ctxtReal51);
      if(realCompareAbsLessThan(&width, &tolerance)) {
        realAdd(&low, &high, &logValue, &ctxtReal51);
        realMultiply(&logValue, const_1on2, &logValue, &ctxtReal51);
        realExp(&logValue, &value, &ctxtReal51);
        WP34S_AdjustPositiveUnderflow(&value);
        realPlus(&value, res, realContext);
        return;
      }

      realAdd(&low, &high, &midpoint, &ctxtReal51);
      realMultiply(&midpoint, const_1on2, &midpoint, &ctxtReal51);
      if(iteration >= 24 || realIsZero(&derivative) || realIsSpecial(&derivative)) {
        realCopy(&midpoint, &proposal);
      }
      else {
        realDivide(&residual, &derivative, &step, &ctxtReal51);
        realSubtract(&logValue, &step, &proposal, &ctxtReal51);
        if(realCompareLessEqual(&proposal, &low) || realCompareGreaterEqual(&proposal, &high)) {
          realCopy(&midpoint, &proposal);
        }
      }
      realCopy(&proposal, &logValue);
    }
    realSetNaN(res);
  }

#endif // !OPTION_DIST_CHI2
