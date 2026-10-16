// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file hyper.c
 ***********************************************/

#include "c47.h"

#if !defined(OPTION_DIST_HYPER)
  void fnHypergeometricP  (uint16_t unusedButMandatoryParameter){}
  void fnHypergeometricL  (uint16_t unusedButMandatoryParameter){}
  void fnHypergeometricR  (uint16_t unusedButMandatoryParameter){}
  void fnHypergeometricI  (uint16_t unusedButMandatoryParameter){}
  //void pdf_Hypergeometric (const real_t *x, const real_t *k0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext){}
  //void cdfu_Hypergeometric(const real_t *x, const real_t *k0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext){}
  //void cdf_Hypergeometric (const real_t *x, const real_t *k0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext){}
  //void cdf_Hypergeometric2(const real_t *x, const real_t *k0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext){}
  //void qf_Hypergeometric  (const real_t *x, const real_t *k0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext){}

#else
  static bool_t hyperCheckDomain(bool_t registersValid, real_t *x, real_t *i, real_t *j, real_t *k, uint16_t unusedParameter) {
    if(!registersValid) {
      displayDomainErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Values in register X, I, J and K must be of the real or long integer type");
        moreInfoOnError("In function hyperCheckDomain:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }

    if((!checkRegisterNoFP(i)) || (!checkRegisterNoFP(j)) || (!checkRegisterNoFP(k)) || realIsSpecial(i) || realIsSpecial(j) || realIsSpecial(k)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function hyperCheckDomain:", "N, n and/or K is not an integer", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    if(realIsNaN(x)) {
      displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function hyperCheckDomain:", "x is NaN", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    if(realIsNegative(k) || realCompareGreaterThan(k, i) || realIsNegative(j) || realCompareGreaterThan(j, i) || realIsNegative(i)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function hyperCheckDomain:", "the parameters must be integer, and 0 " STD_LESS_EQUAL " K " STD_LESS_EQUAL " N, 0 " STD_LESS_EQUAL " n " STD_LESS_EQUAL " N, and N " STD_GREATER_EQUAL " 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    return true;
  }


  void fnHypergeometricP(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, spec, samp, batch;

    if(WP34S_CheckDistributionParameters(&val, REGISTER_M, &batch, REGISTER_N, &samp, REGISTER_Q, &spec, 0, hyperCheckDomain)) {
      if(realIsAnInteger(&val)) {
        pdf_Hypergeometric(&val, &spec, &samp, &batch, &ans, &ctxtReal39);
      }
      else {
        realSetZero(&ans);
      }
      if(realIsNaN(&ans)) {
        displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnHypergeometricP:", "a parameter is invalid", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      else {
        convertRealToResultRegister(&ans, REGISTER_X, amNone);
      }
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }


  void fnHypergeometricL(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, spec, samp, batch;

    if(WP34S_CheckDistributionParameters(&val, REGISTER_M, &batch, REGISTER_N, &samp, REGISTER_Q, &spec, 0, hyperCheckDomain)) {
      cdf_Hypergeometric(&val, &spec, &samp, &batch, &ans, &ctxtReal39);
      if(realIsNaN(&ans)) {
        displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnHypergeometricL:", "a parameter is invalid", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      else {
        convertRealToResultRegister(&ans, REGISTER_X, amNone);
      }
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }


  void fnHypergeometricR(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, spec, samp, batch;

    if(WP34S_CheckDistributionParameters(&val, REGISTER_M, &batch, REGISTER_N, &samp, REGISTER_Q, &spec, 0, hyperCheckDomain)) {
      cdfu_Hypergeometric(&val, &spec, &samp, &batch, &ans, &ctxtReal39);
      if(realIsNaN(&ans)) {
        displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnHypergeometricR:", "a parameter is invalid", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      else {
        convertRealToResultRegister(&ans, REGISTER_X, amNone);
      }
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }


  void fnHypergeometricI(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, spec, samp, batch;

    if(WP34S_CheckDistributionParameters(&val, REGISTER_M, &batch, REGISTER_N, &samp, REGISTER_Q, &spec, 0, hyperCheckDomain)) {
      if(realCompareLessThan(&val, const_0) || realCompareGreaterThan(&val, const_1)) {
        displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnHypergeometricI:", "the argument must be 0 " STD_LESS_EQUAL " x " STD_LESS_EQUAL " 1", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        if(getSystemFlag(FLAG_SPCRES)) {
          convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
        }
        return;
      }
      qf_Hypergeometric(&val, &spec, &samp, &batch, &ans, &ctxtReal39);
      if(realIsNaN(&ans)) {
        displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnHypergeometricI:", "WP34S_Qf_Binomial did not converge", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        if(getSystemFlag(FLAG_SPCRES)) {
          convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
        }
        return;
      }
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }


  static bool_t hyperParametersAreValid(const real_t *k0, const real_t *n, const real_t *n0) {
    return !realIsSpecial(k0) && !realIsSpecial(n) && !realIsSpecial(n0) && realIsAnInteger(k0) && realIsAnInteger(n) && realIsAnInteger(n0) &&
           realCompareGreaterEqual(k0, const_0) && realCompareGreaterEqual(n, const_0) && realCompareGreaterEqual(n0, const_0) &&
           realCompareLessEqual(k0, n0) && realCompareLessEqual(n, n0);
  }


  static realContext_t *hyperCalculationContext(const real_t *n0, realContext_t *realContext) {
    real_t threshold;

    realSetOne(&threshold);
    threshold.exponent = 6;
    if(realContext->digits > 51) {
      return realContext;
    }
    return realCompareLessEqual(n0, &threshold) ? &ctxtReal51 : &ctxtReal75;
  }


  static void hyperSupport(const real_t *k0, const real_t *n, const real_t *n0, real_t *lower, real_t *upper, realContext_t *realContext) {
    realAdd(k0, n, lower, realContext);
    realSubtract(lower, n0, lower, realContext);
    if(realIsNegative(lower)) {
      realSetZero(lower);
    }
    realCopy(realCompareLessThan(k0, n) ? k0 : n, upper);
  }


  static void hyperStandardise(const real_t *x, const real_t *k0, const real_t *n, const real_t *n0, real_t *standardX, real_t *standardK, real_t *standardN,
                               bool_t *reflected, realContext_t *realContext) {
    real_t half;

    realCopy(x, standardX);
    realCopy(k0, standardK);
    realCopy(n, standardN);
    *reflected = false;
    realMultiply(n0, const_1on2, &half, realContext);
    if(realCompareGreaterThan(standardK, &half)) {
      realSubtract(standardN, standardX, standardX, realContext);
      realSubtract(n0, standardK, standardK, realContext);
      *reflected = !*reflected;
    }
    if(realCompareGreaterThan(standardN, &half)) {
      realSubtract(standardK, standardX, standardX, realContext);
      realSubtract(n0, standardN, standardN, realContext);
      *reflected = !*reflected;
    }
  }


  static void hyperLogCombination(const real_t *total, const real_t *selected, real_t *res, realContext_t *realContext) {
    real_t a, b, logarithm;

    if(realContext->digits <= 51) {
      realCopy(total, &a);
      realCopy(selected, &b);
      logCyxReal(&a, &b, res, realContext);
      return;
    }
    realAdd(selected, const_1, &a, &ctxtReal75);
    realSubtract(total, selected, &b, &ctxtReal75);
    realAdd(&b, const_1, &b, &ctxtReal75);
    C47_LnBetaPositiveReal(&a, &b, res);
    realAdd(total, const_1, &a, &ctxtReal75);
    WP34S_Ln(&a, &logarithm, &ctxtReal75);
    realAdd(res, &logarithm, res, &ctxtReal75);
    realChangeSign(res);
  }


  static void hyperLogOnePlusProduct(const real_t *value, real_t *res, realContext_t *realContext) {
    real_t factor, indexReal, nextIndex, previous, term;
    int32_t index;

    if(!realCompareAbsLessThan(value, const_1e_6)) {
      WP34S_Ln1P(value, &term, realContext);
      realAdd(const_1, value, &factor, realContext);
      realMultiply(&factor, &term, res, realContext);
      realSubtract(res, value, res, realContext);
      return;
    }
    realMultiply(value, value, &term, realContext);
    realMultiply(&term, const_1on2, &term, realContext);
    realCopy(&term, res);
    for(index=2; index<160; index++) {
      realCopy(res, &previous);
      realMultiply(&term, value, &term, realContext);
      realChangeSign(&term);
      int32ToReal(index - 1, &indexReal);
      int32ToReal(index + 1, &nextIndex);
      realMultiply(&term, &indexReal, &term, realContext);
      realDivide(&term, &nextIndex, &term, realContext);
      realAdd(res, &term, res, realContext);
      if(realCompareEqual(res, &previous)) {
        break;
      }
    }
  }


  static void hyperStirlingCorrection(const real_t *value, real_t *res, realContext_t *realContext) {
    real_t coefficient, denominator, inverse, inverseSquare;

    realDivide(const_1, value, &inverse, realContext);
    realMultiply(&inverse, &inverse, &inverseSquare, realContext);
    int32ToReal(43867, res);
    int32ToReal(244188, &denominator);
    realDivide(res, &denominator, res, realContext);
    int32ToReal(-3617, &coefficient);
    int32ToReal(122400, &denominator);
    realDivide(&coefficient, &denominator, &coefficient, realContext);
    realFMA(res, &inverseSquare, &coefficient, res, realContext);
    int32ToReal(7, &coefficient);
    int32ToReal(1092, &denominator);
    realDivide(&coefficient, &denominator, &coefficient, realContext);
    realFMA(res, &inverseSquare, &coefficient, res, realContext);
    int32ToReal(-691, &coefficient);
    int32ToReal(360360, &denominator);
    realDivide(&coefficient, &denominator, &coefficient, realContext);
    realFMA(res, &inverseSquare, &coefficient, res, realContext);
    int32ToReal(1, &coefficient);
    int32ToReal(1188, &denominator);
    realDivide(&coefficient, &denominator, &coefficient, realContext);
    realFMA(res, &inverseSquare, &coefficient, res, realContext);
    int32ToReal(-1, &coefficient);
    int32ToReal(1680, &denominator);
    realDivide(&coefficient, &denominator, &coefficient, realContext);
    realFMA(res, &inverseSquare, &coefficient, res, realContext);
    int32ToReal(1, &coefficient);
    int32ToReal(1260, &denominator);
    realDivide(&coefficient, &denominator, &coefficient, realContext);
    realFMA(res, &inverseSquare, &coefficient, res, realContext);
    int32ToReal(-1, &coefficient);
    int32ToReal(360, &denominator);
    realDivide(&coefficient, &denominator, &coefficient, realContext);
    realFMA(res, &inverseSquare, &coefficient, res, realContext);
    int32ToReal(1, &coefficient);
    int32ToReal(12, &denominator);
    realDivide(&coefficient, &denominator, &coefficient, realContext);
    realFMA(res, &inverseSquare, &coefficient, res, realContext);
    realMultiply(res, &inverse, res, realContext);
  }


  static void hyperDevianceContribution(const real_t *expected, const real_t *delta, bool_t negative, real_t *res, realContext_t *realContext) {
    real_t ratio;

    realDivide(delta, expected, &ratio, realContext);
    if(negative) {
      realChangeSign(&ratio);
    }
    hyperLogOnePlusProduct(&ratio, res, realContext);
    realMultiply(res, expected, res, realContext);
  }


  static void hyperSaddleLogMass(const real_t *x, const real_t *k0, const real_t *n, const real_t *n0, real_t *res) {
    real_t a, b, c, correction, d, delta, deviance, expected, logarithm, nMinusK, nMinusN, term;
    realContext_t *realContext = &ctxtReal75;

    realSubtract(k0, x, &b, realContext);
    realSubtract(n, x, &c, realContext);
    realSubtract(n0, k0, &nMinusK, realContext);
    realSubtract(n0, n, &nMinusN, realContext);
    realSubtract(&nMinusK, &c, &d, realContext);
    realMultiply(k0, n, &expected, realContext);
    realDivide(&expected, n0, &expected, realContext);
    realSubtract(x, &expected, &delta, realContext);
    hyperDevianceContribution(&expected, &delta, false, &deviance, realContext);
    realMultiply(k0, &nMinusN, &expected, realContext);
    realDivide(&expected, n0, &expected, realContext);
    hyperDevianceContribution(&expected, &delta, true, &term, realContext);
    realAdd(&deviance, &term, &deviance, realContext);
    realMultiply(&nMinusK, n, &expected, realContext);
    realDivide(&expected, n0, &expected, realContext);
    hyperDevianceContribution(&expected, &delta, true, &term, realContext);
    realAdd(&deviance, &term, &deviance, realContext);
    realMultiply(&nMinusK, &nMinusN, &expected, realContext);
    realDivide(&expected, n0, &expected, realContext);
    hyperDevianceContribution(&expected, &delta, false, &term, realContext);
    realAdd(&deviance, &term, &deviance, realContext);

    WP34S_Ln(const75_2pi, res, realContext);
    realChangeSign(res);
    WP34S_Ln(k0, &logarithm, realContext);
    realAdd(res, &logarithm, res, realContext);
    WP34S_Ln(&nMinusK, &logarithm, realContext);
    realAdd(res, &logarithm, res, realContext);
    WP34S_Ln(n, &logarithm, realContext);
    realAdd(res, &logarithm, res, realContext);
    WP34S_Ln(&nMinusN, &logarithm, realContext);
    realAdd(res, &logarithm, res, realContext);
    WP34S_Ln(n0, &logarithm, realContext);
    realSubtract(res, &logarithm, res, realContext);
    realCopy(x, &a);
    WP34S_Ln(&a, &logarithm, realContext);
    realSubtract(res, &logarithm, res, realContext);
    WP34S_Ln(&b, &logarithm, realContext);
    realSubtract(res, &logarithm, res, realContext);
    WP34S_Ln(&c, &logarithm, realContext);
    realSubtract(res, &logarithm, res, realContext);
    WP34S_Ln(&d, &logarithm, realContext);
    realSubtract(res, &logarithm, res, realContext);
    realMultiply(res, const_1on2, res, realContext);
    realSubtract(res, &deviance, res, realContext);

    realSetZero(&correction);
    hyperStirlingCorrection(k0, &term, realContext);
    realAdd(&correction, &term, &correction, realContext);
    hyperStirlingCorrection(&nMinusK, &term, realContext);
    realAdd(&correction, &term, &correction, realContext);
    hyperStirlingCorrection(n, &term, realContext);
    realAdd(&correction, &term, &correction, realContext);
    hyperStirlingCorrection(&nMinusN, &term, realContext);
    realAdd(&correction, &term, &correction, realContext);
    hyperStirlingCorrection(n0, &term, realContext);
    realSubtract(&correction, &term, &correction, realContext);
    hyperStirlingCorrection(&a, &term, realContext);
    realSubtract(&correction, &term, &correction, realContext);
    hyperStirlingCorrection(&b, &term, realContext);
    realSubtract(&correction, &term, &correction, realContext);
    hyperStirlingCorrection(&c, &term, realContext);
    realSubtract(&correction, &term, &correction, realContext);
    hyperStirlingCorrection(&d, &term, realContext);
    realSubtract(&correction, &term, &correction, realContext);
    realAdd(res, &correction, res, realContext);
  }


  static void hyperLogMass(const real_t *x, const real_t *k0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext) {
    real_t a, b, c, d, minimum, nMinusK, threshold;

    realSubtract(k0, x, &b, &ctxtReal75);
    realSubtract(n, x, &c, &ctxtReal75);
    realSubtract(n0, k0, &nMinusK, &ctxtReal75);
    realSubtract(&nMinusK, &c, &d, &ctxtReal75);
    realCopy(x, &minimum);
    if(realCompareLessThan(&b, &minimum)) {
      realCopy(&b, &minimum);
    }
    if(realCompareLessThan(&c, &minimum)) {
      realCopy(&c, &minimum);
    }
    if(realCompareLessThan(&d, &minimum)) {
      realCopy(&d, &minimum);
    }
    realSetOne(&threshold);
    threshold.exponent = 12;
    if(realContext->digits > 51 && realCompareGreaterEqual(n0, &threshold) && realCompareGreaterEqual(&minimum, const_1000)) {
      hyperSaddleLogMass(x, k0, n, n0, res);
      return;
    }
    hyperLogCombination(k0, x, res, realContext);
    hyperLogCombination(&nMinusK, &c, &a, realContext);
    realAdd(res, &a, res, realContext);
    hyperLogCombination(n0, n, &a, realContext);
    realSubtract(res, &a, res, realContext);
  }


  static void hyperMassStandard(const real_t *x, const real_t *k0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext) {
    real_t logarithm;

    hyperLogMass(x, k0, n, n0, &logarithm, realContext);
    realExp(&logarithm, res, realContext);
    if(realIsZero(res)) {
      realNextToward(res, const_1, res, realContext);
    }
  }


  static void hyperAdjustPositiveUnderflow(real_t *value, realContext_t *realContext) {
    real_t halfMinimum, minimum;
    bool_t roundUp = false;

    if(realContext->digits > 39) {
      return;
    }
    WP34S_SetPowerOfTen(-6176, &minimum);
    if(!realIsZero(value) && realCompareGreaterEqual(value, &minimum)) {
      return;
    }
    realMultiply(&minimum, const_1on2, &halfMinimum, &ctxtReal39);
    switch(roundingModeTable[roundingMode]) {
      case DEC_ROUND_HALF_EVEN:
      case DEC_ROUND_HALF_DOWN:
        roundUp = !realIsZero(value) && realCompareGreaterThan(value, &halfMinimum);
        break;
      case DEC_ROUND_HALF_UP:
        roundUp = !realIsZero(value) && realCompareGreaterEqual(value, &halfMinimum);
        break;
      case DEC_ROUND_UP:
      case DEC_ROUND_CEILING:
        roundUp = true;
        break;
      default:
        break;
    }
    realCopy(roundUp ? &minimum : const_0, value);
  }


  static void hyperRoundProbability(const real_t *value, real_t *res, realContext_t *realContext) {
    real_t adjusted;

    realCopy(value, &adjusted);
    hyperAdjustPositiveUnderflow(&adjusted, realContext);
    realPlus(&adjusted, res, realContext);
    if(realCompareEqual(res, const_1on2) && !realCompareEqual(value, const_1on2)) {
      realNextToward(res, realCompareGreaterThan(value, const_1on2) ? const_1 : const_0, res, realContext);
    }
  }


  static void hyperMode(const real_t *k0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext) {
    real_t denominator, product;

    realAdd(k0, const_1, &product, realContext);
    realAdd(n, const_1, &denominator, realContext);
    realMultiply(&product, &denominator, &product, realContext);
    realAdd(n0, const_2, &denominator, realContext);
    realDivide(&product, &denominator, &product, realContext);
    realToIntegralValue(&product, res, DEC_ROUND_FLOOR, realContext);
  }


  static void hyperMoments(const real_t *k0, const real_t *n, const real_t *n0, real_t *mean, real_t *standardDeviation, realContext_t *realContext) {
    real_t factor, variance;

    realMultiply(k0, n, mean, realContext);
    realDivide(mean, n0, mean, realContext);
    realSubtract(n0, k0, &factor, realContext);
    realDivide(&factor, n0, &factor, realContext);
    realMultiply(mean, &factor, &variance, realContext);
    realSubtract(n0, n, &factor, realContext);
    realSubtract(n0, const_1, standardDeviation, realContext);
    realDivide(&factor, standardDeviation, &factor, realContext);
    realMultiply(&variance, &factor, &variance, realContext);
    realSquareRoot(&variance, standardDeviation, realContext);
  }


  static bool_t hyperUseNormalApproximation(const real_t *k0, const real_t *n, const real_t *n0, real_t *mean, real_t *standardDeviation) {
    real_t threshold;

    hyperMoments(k0, n, n0, mean, standardDeviation, &ctxtReal75);
    realSetOne(&threshold);
    threshold.exponent = 37;
    return realCompareGreaterEqual(standardDeviation, &threshold);
  }


  static void hyperNormalProbability(const real_t *x, const real_t *mean, const real_t *standardDeviation, bool_t upper, real_t *res) {
    real_t z;

    realSubtract(x, mean, &z, &ctxtReal75);
    realDivide(&z, standardDeviation, &z, &ctxtReal75);
    if(upper) {
      WP34S_Cdfu_Q(&z, res, &ctxtReal75);
    }
    else {
      WP34S_Cdf_Q(&z, res, &ctxtReal75);
    }
    if(realIsZero(&z) && realCompareEqual(res, const_1on2)) {
      realNextToward(res, const_1, res, &ctxtReal75);
    }
    WP34S_MarkProbability(res, &ctxtReal75);
  }


  static bool_t hyperTailSum(const real_t *start, const real_t *k0, const real_t *n, const real_t *n0, bool_t upward, real_t *res,
                             realContext_t *realContext) {
    real_t current, denominator, factor, former, numerator, previous, ratio, term;

    realCopy(start, &current);
    hyperMassStandard(&current, k0, n, n0, &term, realContext);
    realCopy(&term, res);
    for(;;) {
      realCopy(res, &previous);
      realCopy(&current, &former);
      if(upward) {
        if(realCompareGreaterEqual(&current, realCompareLessThan(k0, n) ? k0 : n)) {
          return true;
        }
        realSubtract(k0, &current, &numerator, realContext);
        realSubtract(n, &current, &factor, realContext);
        realMultiply(&numerator, &factor, &numerator, realContext);
        realAdd(&current, const_1, &denominator, realContext);
        realSubtract(n0, k0, &factor, realContext);
        realSubtract(&factor, n, &factor, realContext);
        realAdd(&factor, &current, &factor, realContext);
        realAdd(&factor, const_1, &factor, realContext);
        realMultiply(&denominator, &factor, &denominator, realContext);
        realDivide(&numerator, &denominator, &ratio, realContext);
        realAdd(&current, const_1, &current, realContext);
      }
      else {
        if(realIsZero(&current)) {
          return true;
        }
        realSubtract(n0, k0, &numerator, realContext);
        realSubtract(&numerator, n, &numerator, realContext);
        realAdd(&numerator, &current, &numerator, realContext);
        realMultiply(&numerator, &current, &numerator, realContext);
        realSubtract(k0, &current, &denominator, realContext);
        realAdd(&denominator, const_1, &denominator, realContext);
        realSubtract(n, &current, &factor, realContext);
        realAdd(&factor, const_1, &factor, realContext);
        realMultiply(&denominator, &factor, &denominator, realContext);
        realDivide(&numerator, &denominator, &ratio, realContext);
        realSubtract(&current, const_1, &current, realContext);
      }
      if(realCompareEqual(&current, &former)) {
        return false;
      }
      realMultiply(&term, &ratio, &term, realContext);
      realAdd(res, &term, res, realContext);
      if(realCompareEqual(res, &previous)) {
        return true;
      }
    }
  }


  static bool_t hyperSymmetricCentre(const real_t *x, const real_t *k0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext) {
    real_t mean, mass, twice;

    realMultiply(k0, const_2, &twice, realContext);
    if(!realCompareEqual(&twice, n0)) {
      realMultiply(n, const_2, &twice, realContext);
      if(!realCompareEqual(&twice, n0)) {
        return false;
      }
    }
    realMultiply(k0, n, &mean, realContext);
    realDivide(&mean, n0, &mean, realContext);
    if(!realCompareEqual(x, &mean)) {
      return false;
    }
    hyperMassStandard(x, k0, n, n0, &mass, realContext);
    realMultiply(&mass, const_1on2, &mass, realContext);
    realAdd(const_1on2, &mass, res, realContext);
    return true;
  }


  static bool_t hyperSymmetricMedian(const real_t *k0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext) {
    real_t mean, twice;

    realMultiply(k0, const_2, &twice, &ctxtReal75);
    if(!realCompareEqual(&twice, n0)) {
      realMultiply(n, const_2, &twice, &ctxtReal75);
      if(!realCompareEqual(&twice, n0)) {
        return false;
      }
    }
    realMultiply(k0, n, &mean, &ctxtReal75);
    realDivide(&mean, n0, &mean, &ctxtReal75);
    realToIntegralValue(&mean, res, DEC_ROUND_CEILING, realContext);
    return true;
  }


  static void hyperLowerStandard(const real_t *x, const real_t *k0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext) {
    real_t mean, mode, standardDeviation, start, tail, upper;

    realCopy(realCompareLessThan(k0, n) ? k0 : n, &upper);
    if(realCompareLessThan(x, const_0)) {
      realSetZero(res);
      return;
    }
    if(realCompareGreaterEqual(x, &upper)) {
      realSetOne(res);
      return;
    }
    if(hyperSymmetricCentre(x, k0, n, n0, res, realContext)) {
      return;
    }
    if(hyperUseNormalApproximation(k0, n, n0, &mean, &standardDeviation)) {
      hyperNormalProbability(x, &mean, &standardDeviation, false, res);
      return;
    }
    hyperMode(k0, n, n0, &mode, realContext);
    if(realCompareLessThan(x, &mode)) {
      if(!hyperTailSum(x, k0, n, n0, false, res, realContext)) {
        realSetNaN(res);
      }
      return;
    }
    realAdd(x, const_1, &start, realContext);
    if(!hyperTailSum(&start, k0, n, n0, true, &tail, realContext)) {
      realSetNaN(res);
      return;
    }
    realSubtract(const_1, &tail, res, realContext);
    WP34S_MarkProbability(res, realContext);
  }


  static void hyperUpperStandard(const real_t *x, const real_t *k0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext) {
    real_t mean, mode, standardDeviation, start, tail, upper;

    realCopy(realCompareLessThan(k0, n) ? k0 : n, &upper);
    if(realCompareLessEqual(x, const_0)) {
      realSetOne(res);
      return;
    }
    if(realCompareGreaterThan(x, &upper)) {
      realSetZero(res);
      return;
    }
    if(hyperSymmetricCentre(x, k0, n, n0, res, realContext)) {
      return;
    }
    if(hyperUseNormalApproximation(k0, n, n0, &mean, &standardDeviation)) {
      hyperNormalProbability(x, &mean, &standardDeviation, true, res);
      return;
    }
    hyperMode(k0, n, n0, &mode, realContext);
    if(realCompareGreaterThan(x, &mode)) {
      if(!hyperTailSum(x, k0, n, n0, true, res, realContext)) {
        realSetNaN(res);
      }
      return;
    }
    realSubtract(x, const_1, &start, realContext);
    if(!hyperTailSum(&start, k0, n, n0, false, &tail, realContext)) {
      realSetNaN(res);
      return;
    }
    realSubtract(const_1, &tail, res, realContext);
    WP34S_MarkProbability(res, realContext);
  }


  void pdf_Hypergeometric(const real_t *x, const real_t *k0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext) {
    real_t lower, standardK, standardN, standardX, upper, value;
    realContext_t *calculationContext;
    bool_t reflected;

    if(realIsNaN(x) || !hyperParametersAreValid(k0, n, n0)) {
      realSetNaN(res);
      return;
    }
    hyperSupport(k0, n, n0, &lower, &upper, &ctxtReal75);
    if(!realIsAnInteger(x) || realIsInfinite(x) || realCompareLessThan(x, &lower) || realCompareGreaterThan(x, &upper)) {
      realSetZero(res);
      return;
    }
    if(realCompareEqual(&lower, &upper)) {
      realSetOne(res);
      return;
    }
    calculationContext = hyperCalculationContext(n0, realContext);
    hyperStandardise(x, k0, n, n0, &standardX, &standardK, &standardN, &reflected, calculationContext);
    hyperMassStandard(&standardX, &standardK, &standardN, n0, &value, calculationContext);
    hyperRoundProbability(&value, res, realContext);
  }


  void cdfu_Hypergeometric(const real_t *x, const real_t *k0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext) {
    real_t integer, lower, standardK, standardN, standardX, upper, value;
    realContext_t *calculationContext;
    bool_t reflected;

    if(realIsNaN(x) || !hyperParametersAreValid(k0, n, n0)) {
      realSetNaN(res);
      return;
    }
    calculationContext = hyperCalculationContext(n0, realContext);
    realToIntegralValue(x, &integer, DEC_ROUND_CEILING, calculationContext);
    hyperSupport(k0, n, n0, &lower, &upper, calculationContext);
    if(realCompareLessEqual(&integer, &lower)) {
      realSetOne(res);
      return;
    }
    if(realCompareGreaterThan(&integer, &upper) || realIsInfinite(&integer)) {
      realSetZero(res);
      return;
    }
    hyperStandardise(&integer, k0, n, n0, &standardX, &standardK, &standardN, &reflected, calculationContext);
    if(reflected) {
      hyperLowerStandard(&standardX, &standardK, &standardN, n0, &value, calculationContext);
    }
    else {
      hyperUpperStandard(&standardX, &standardK, &standardN, n0, &value, calculationContext);
    }
    hyperRoundProbability(&value, res, realContext);
  }


  void cdf_Hypergeometric(const real_t *x, const real_t *k0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext) {
    real_t integer;

    if(realIsNaN(x) || !hyperParametersAreValid(k0, n, n0)) {
      realSetNaN(res);
      return;
    }
    realToIntegralValue(x, &integer, DEC_ROUND_FLOOR, hyperCalculationContext(n0, realContext));
    cdf_Hypergeometric2(&integer, k0, n, n0, res, realContext);
  }


  void cdf_Hypergeometric2(const real_t *x, const real_t *k0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext) {
    real_t lower, standardK, standardN, standardX, upper, value;
    realContext_t *calculationContext;
    bool_t reflected;

    if(realIsNaN(x) || !hyperParametersAreValid(k0, n, n0)) {
      realSetNaN(res);
      return;
    }
    calculationContext = hyperCalculationContext(n0, realContext);
    hyperSupport(k0, n, n0, &lower, &upper, calculationContext);
    if(realCompareLessThan(x, &lower)) {
      realSetZero(res);
      return;
    }
    if(realCompareGreaterEqual(x, &upper) || realIsInfinite(x)) {
      realSetOne(res);
      return;
    }
    hyperStandardise(x, k0, n, n0, &standardX, &standardK, &standardN, &reflected, calculationContext);
    if(reflected) {
      hyperUpperStandard(&standardX, &standardK, &standardN, n0, &value, calculationContext);
    }
    else {
      hyperLowerStandard(&standardX, &standardK, &standardN, n0, &value, calculationContext);
    }
    hyperRoundProbability(&value, res, realContext);
  }


  void qf_Hypergeometric(const real_t *x, const real_t *k0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext) {
    real_t cdf, high, low, lower, mean, midpoint, step, tolerance, upper, width;

    if(realIsNaN(x) || !hyperParametersAreValid(k0, n, n0) || realCompareLessThan(x, const_0) || realCompareGreaterThan(x, const_1)) {
      realSetNaN(res);
      return;
    }
    hyperSupport(k0, n, n0, &lower, &upper, &ctxtReal75);
    if(realCompareEqual(&lower, &upper) || realIsZero(x)) {
      realCopy(&lower, res);
      return;
    }
    if(realCompareEqual(x, const_1)) {
      realCopy(&upper, res);
      return;
    }
    if(realCompareEqual(x, const_1on2) && hyperSymmetricMedian(k0, n, n0, res, realContext)) {
      return;
    }
    cdf_Hypergeometric2(&lower, k0, n, n0, &cdf, &ctxtReal75);
    if(realCompareGreaterEqual(&cdf, x)) {
      realCopy(&lower, res);
      return;
    }
    realMultiply(k0, n, &mean, &ctxtReal75);
    realDivide(&mean, n0, &mean, &ctxtReal75);
    realToIntegralValue(&mean, &high, DEC_ROUND_CEILING, &ctxtReal39);
    if(realCompareLessEqual(&high, &lower)) {
      realAdd(&lower, const_1, &high, &ctxtReal39);
    }
    if(realCompareGreaterThan(&high, &upper)) {
      realCopy(&upper, &high);
    }
    realCopy(&lower, &low);
    for(int32_t iteration=0; iteration<64; iteration++) {
      cdf_Hypergeometric2(&high, k0, n, n0, &cdf, &ctxtReal75);
      if(realCompareGreaterEqual(&cdf, x)) {
        break;
      }
      realSubtract(&high, &low, &step, &ctxtReal39);
      if(realCompareLessThan(&step, const_1)) {
        realSetOne(&step);
      }
      realCopy(&high, &low);
      realAdd(&high, &step, &high, &ctxtReal39);
      if(realCompareGreaterEqual(&high, &upper) || iteration == 63) {
        realCopy(&upper, &high);
        break;
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
        realAdd(&low, &high, &midpoint, &ctxtReal39);
        realMultiply(&midpoint, const_1on2, &midpoint, &ctxtReal39);
        realCopy(&midpoint, res);
        return;
      }
      realAdd(&low, &high, &midpoint, &ctxtReal39);
      realMultiply(&midpoint, const_1on2, &midpoint, &ctxtReal39);
      realToIntegralValue(&midpoint, &midpoint, DEC_ROUND_FLOOR, &ctxtReal39);
      cdf_Hypergeometric2(&midpoint, k0, n, n0, &cdf, &ctxtReal75);
      if(realCompareGreaterEqual(&cdf, x)) {
        realCopy(&midpoint, &high);
      }
      else {
        realCopy(&midpoint, &low);
      }
    }
    realSetNaN(res);
  }

#endif // !OPTION_DIST_HYPER
