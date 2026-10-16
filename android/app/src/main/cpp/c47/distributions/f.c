// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file f.c
 ***********************************************/

#include "c47.h"

#if !defined(OPTION_DIST_F)
  void fnF_P(uint16_t unusedButMandatoryParameter){}
  void fnF_L(uint16_t unusedButMandatoryParameter){}
  void fnF_R(uint16_t unusedButMandatoryParameter){}
  void fnF_I(uint16_t unusedButMandatoryParameter){}
//  void WP34S_Pdf_F (const real_t *x, const real_t *d1, const real_t *d2, real_t *res, realContext_t *realContext){}
//  void WP34S_Cdfu_F(const real_t *x, const real_t *d1, const real_t *d2, real_t *res, realContext_t *realContext){}
//  void WP34S_Cdf_F (const real_t *x, const real_t *d1, const real_t *d2, real_t *res, realContext_t *realContext){}
//  void WP34S_Qf_F  (const real_t *x, const real_t *d1, const real_t *d2, real_t *res, realContext_t *realContext){}
//  void WP34S_Qf_Newton(uint32_t r_dist, const real_t *target, const real_t *estimate, const real_t *p1, const real_t *p2, const real_t *p3, real_t *res, realContext_t *realContext){}

#else
  bool_t fCheckDomain(bool_t registersValid, real_t *x, real_t *i, real_t *j, real_t *unusedThird, uint16_t unusedParameter) {
    if(!registersValid) {
      return false;
    }

    if(!checkRegisterNoFP(i) || !checkRegisterNoFP(j) || realIsSpecial(i) || realIsSpecial(j)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function fCheckDomain:", "d1 or d2 is not an integer", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    if(realIsNaN(x) || realIsNegative(x)) {
      displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function fCheckDomain:", "cannot calculate for x < 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    if(realIsZero(i) || realIsNegative(i) || realIsZero(j) || realIsNegative(j)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function fCheckDomain:", "cannot calculate for d1 " STD_LESS_EQUAL " 0 or d2 " STD_LESS_EQUAL " 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    return true;
  }


  void fnF_P(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_F * 4 + DIST_P);
  }


  void fnF_L(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_F * 4 + DIST_L);
  }


  void fnF_R(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_F * 4 + DIST_R);
  }


  void fnF_I(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_F * 4 + DIST_I);
  }



  /******************************************************
   * This functions are borrowed from the WP34S project
   ******************************************************/

  static bool_t fParametersAreValid(const real_t *d1, const real_t *d2) {
    return !realIsSpecial(d1) && !realIsSpecial(d2) && realCompareGreaterThan(d1, const_0) && realCompareGreaterThan(d2, const_0);
  }


  static void fLogitToBetaPair(const real_t *logit, real_t *y, real_t *z, realContext_t *realContext) {
    real_t exponential, denominator, value;

    realCopy(logit, &value);
    if(realIsNegative(&value)) {
      realExp(&value, &exponential, realContext);
      realAdd(const_1, &exponential, &denominator, realContext);
      realDivide(const_1, &denominator, z, realContext);
      realMultiply(&exponential, z, y, realContext);
    }
    else {
      realChangeSign(&value);
      realExp(&value, &exponential, realContext);
      realAdd(const_1, &exponential, &denominator, realContext);
      realDivide(const_1, &denominator, y, realContext);
      realMultiply(&exponential, y, z, realContext);
    }
  }


  static void fLogBetaPrefix(const real_t *a, const real_t *b, const real_t *y, const real_t *z, const real_t *logit, real_t *res,
                             realContext_t *realContext) {
    real_t x, directThreshold, sumA, sumB, sumAB, rA, rB, rAB, logA, logB, term, u, v;

    realSetOne(&directThreshold);
    directThreshold.exponent = 5;
    if(realCompareLessThan(a, &directThreshold) || realCompareLessThan(b, &directThreshold)) {
      C47_LnBetaPositiveReal(a, b, &term);
      realCopy(logit, &x);
      if(realIsNegative(&x)) {
        realExp(&x, &v, realContext);
        WP34S_Ln1P(&v, &v, realContext);
        realSubtract(&x, &v, &u, realContext);
        realChangeSign(&v);
      }
      else {
        realChangeSign(&x);
        realExp(&x, &v, realContext);
        WP34S_Ln1P(&v, &v, realContext);
        realMinus(&v, &u, realContext);
        realSubtract(&x, &v, &v, realContext);
      }
      realMultiply(a, &u, &u, realContext);
      realMultiply(b, &v, &v, realContext);
      realAdd(&u, &v, &u, realContext);
      realSubtract(&u, &term, res, realContext);
      return;
    }

    realSubtract(a, const_1, &x, realContext);
    WP34S_LanczosSum(&x, const_29, &sumA, realContext);
    realSubtract(b, const_1, &x, realContext);
    WP34S_LanczosSum(&x, const_29, &sumB, realContext);
    realAdd(a, b, &rAB, realContext);
    realSubtract(&rAB, const_1, &x, realContext);
    WP34S_LanczosSum(&x, const_29, &sumAB, realContext);
    WP34S_Ln(&sumA, &sumA, realContext);
    WP34S_Ln(&sumB, &sumB, realContext);
    WP34S_Ln(&sumAB, &sumAB, realContext);

    realAdd(a, const_gammaRm1, &rA, realContext);
    realAdd(b, const_gammaRm1, &rB, realContext);
    realAdd(&rAB, const_gammaRm1, &rAB, realContext);

    realMultiply(y, b, &u, realContext);
    realMultiply(z, a, &v, realContext);
    realSubtract(&u, &v, &u, realContext);
    realFMA(const__gammaRm1, z, &u, &u, realContext);
    realDivide(&u, &rA, &u, realContext);
    WP34S_Ln1P(&u, &logA, realContext);

    realMultiply(z, a, &u, realContext);
    realMultiply(y, b, &v, realContext);
    realSubtract(&u, &v, &u, realContext);
    realFMA(const__gammaRm1, y, &u, &u, realContext);
    realDivide(&u, &rB, &u, realContext);
    WP34S_Ln1P(&u, &logB, realContext);

    realMultiply(a, &logA, &u, realContext);
    realFMA(b, &logB, &u, &u, realContext);
    realDivide(b, &rA, &v, realContext);
    WP34S_Ln1P(&v, &v, realContext);
    realMultiply(&v, const_1on2, &v, realContext);
    realSubtract(&u, &v, &u, realContext);
    realDivide(a, &rB, &v, realContext);
    WP34S_Ln1P(&v, &v, realContext);
    realMultiply(&v, const_1on2, &v, realContext);
    realSubtract(&u, &v, &u, realContext);
    WP34S_Ln(&rAB, &v, realContext);
    realMultiply(&v, const_1on2, &v, realContext);
    realAdd(&u, &v, &u, realContext);
    realAdd(&u, const_gammaRm1, &u, realContext);
    realAdd(&sumAB, &u, &u, realContext);
    realSubtract(&u, &sumA, &u, realContext);
    realSubtract(&u, &sumB, res, realContext);
  }


  static void fLargeBetaDensity(const real_t *scaledValue, const real_t *scale, const real_t *a, const real_t *b, const real_t *modePrefix,
                                real_t *res, realContext_t *realContext) {
    real_t value, logit, y, z, sum, termA, termB, ratioA, ratioB, relativeA, relativeB, factor, n, old, divergence, total, small;

    realMultiply(scaledValue, scale, &value, realContext);
    WP34S_Ln(a, &logit, realContext);
    realAdd(&value, &logit, &logit, realContext);
    WP34S_Ln(b, &value, realContext);
    realSubtract(&logit, &value, &logit, realContext);
    fLogitToBetaPair(&logit, &y, &z, realContext);
    realAdd(a, b, &total, realContext);
    realDivide(a, &total, &ratioA, realContext);
    realDivide(b, &total, &ratioB, realContext);
    realSubtract(&y, &ratioA, &factor, realContext);
    realDivide(&factor, &ratioA, &relativeA, realContext);
    realDivide(&factor, &ratioB, &relativeB, realContext);
    realSetOne(&value);
    value.exponent = 20;
    int32ToReal(100, &small);
    realDivide(const_1, &small, &small, realContext);
    if(realCompareGreaterThan(&total, &value) && realCompareAbsLessThan(&relativeA, &small) && realCompareAbsLessThan(&relativeB, &small)) {
      realMultiply(&factor, &relativeA, &termA, realContext);
      realMultiply(&termA, const_1on2, &termA, realContext);
      realMultiply(&factor, &relativeB, &termB, realContext);
      realMultiply(&termB, const_1on2, &termB, realContext);
      realAdd(&termA, &termB, &sum, realContext);
      int32ToReal(2, &n);
      do {
        realCopy(&sum, &old);
        realAdd(&n, const_1, &n, realContext);
        realSubtract(&n, const_1, &value, realContext);
        realDivide(&value, &n, &value, realContext);
        realMultiply(&termA, &relativeA, &termA, realContext);
        realChangeSign(&termA);
        realMultiply(&termA, &value, &termA, realContext);
        realMultiply(&termB, &relativeB, &termB, realContext);
        realMultiply(&termB, &value, &termB, realContext);
        realAdd(&termA, &termB, &value, realContext);
        realAdd(&sum, &value, &sum, realContext);
      } while(!realCompareEqual(&sum, &old));
      realMultiply(&sum, &total, &divergence, realContext);
    }
    else {
      WP34S_Ln1P(&relativeA, &termA, realContext);
      realMultiply(a, &termA, &termA, realContext);
      realMinus(&relativeB, &termB, realContext);
      WP34S_Ln1P(&termB, &termB, realContext);
      realMultiply(b, &termB, &termB, realContext);
      realAdd(&termA, &termB, &divergence, realContext);
      realChangeSign(&divergence);
    }
    realSubtract(modePrefix, &divergence, &value, realContext);
    realExp(&value, res, realContext);
    realMultiply(res, scale, res, realContext);
  }


  static void fLargeBetaPair(const real_t *a, const real_t *b, const real_t *y, const real_t *logit, real_t *lower, real_t *upper,
                             realContext_t *realContext) {
    real_t total, ratioA, ratioB, modeLogit, modePrefix, scale, value, scaledValue, left, right, midpoint, halfWidth;
    real_t h, u, sinhU, coshU, argument, tanhArgument, coshArgument, node, jacobian, density, term, sum, bound;
    int32_t k;
    bool_t calculateLower;

    realAdd(a, b, &total, realContext);
    realDivide(a, &total, &ratioA, realContext);
    realDivide(b, &total, &ratioB, realContext);
    WP34S_Ln(a, &modeLogit, realContext);
    WP34S_Ln(b, &value, realContext);
    realSubtract(&modeLogit, &value, &modeLogit, realContext);
    fLogBetaPrefix(a, b, &ratioA, &ratioB, &modeLogit, &modePrefix, realContext);
    realMultiply(a, b, &value, realContext);
    realDivide(&total, &value, &scale, realContext);
    realSquareRoot(&scale, &scale, realContext);
    realSubtract(logit, &modeLogit, &value, realContext);
    realDivide(&value, &scale, &scaledValue, realContext);

    calculateLower = realCompareLessEqual(&scaledValue, const_0);
    // m, the slope of the log density at v in mode widths, sets the step to the far end: over sqrt(m^2 + 180) - m a Gaussian log density falls by 90, and for shapes
    // of 1E5 and more the logit density falls by at least 88 there while the tail is above 1E-6176, so the mass left out is below e^-88 of the tail.
    realMultiply(&total, y, &value, realContext);
    realSubtract(&value, a, &value, realContext);
    realMultiply(&value, &scale, &value, realContext);
    realCopyAbs(&value, &value);
    realFMA(&value, &value, const_180, &bound, realContext);
    realSquareRoot(&bound, &bound, realContext);
    realSubtract(&bound, &value, &bound, realContext);
    if(calculateLower) {
      realSubtract(&scaledValue, &bound, &left, realContext);
      realCopy(&scaledValue, &right);
    }
    else {
      realCopy(&scaledValue, &left);
      realAdd(&scaledValue, &bound, &right, realContext);
    }
    realAdd(&left, &right, &midpoint, realContext);
    realMultiply(&midpoint, const_1on2, &midpoint, realContext);
    realSubtract(&right, &left, &halfWidth, realContext);
    realMultiply(&halfWidth, const_1on2, &halfWidth, realContext);
    int32ToReal(32, &value);
    realDivide(const_1, &value, &h, realContext);
    realSetZero(&sum);
    for(k=-192; k<=192; k++) {
      int32ToReal(k, &u);
      realMultiply(&u, &h, &u, realContext);
      WP34S_SinhCosh(&u, &sinhU, &coshU, realContext);
      realMultiply(const75_piOn2, &sinhU, &argument, realContext);
      WP34S_Tanh(&argument, &tanhArgument, realContext);
      if(realCompareAbsEqual(&tanhArgument, const_1)) {
        continue;
      }
      realFMA(&halfWidth, &tanhArgument, &midpoint, &node, realContext);
      WP34S_SinhCosh(&argument, &value, &coshArgument, realContext);
      realMultiply(&coshArgument, &coshArgument, &value, realContext);
      realMultiply(&halfWidth, const75_piOn2, &jacobian, realContext);
      realMultiply(&jacobian, &coshU, &jacobian, realContext);
      realDivide(&jacobian, &value, &jacobian, realContext);
      fLargeBetaDensity(&node, &scale, a, b, &modePrefix, &density, realContext);
      realMultiply(&density, &jacobian, &term, realContext);
      realAdd(&sum, &term, &sum, realContext);
    }
    realMultiply(&sum, &h, &sum, realContext);
    if(calculateLower) {
      realCopy(&sum, lower);
      realSubtract(const_1, lower, upper, realContext);
    }
    else {
      realCopy(&sum, upper);
      realSubtract(const_1, upper, lower, realContext);
    }
  }


  static bool_t fBetaPair(const real_t *a, const real_t *b, const real_t *y, const real_t *z, const real_t *logit, real_t *lower, real_t *upper,
                          realContext_t *realContext) {
    real_t fraction, largeThreshold, logPrefix, threshold, value, term, wideY, wideZ;

    if(realIsZero(y)) {
      realSetZero(lower);
      realSetOne(upper);
      return true;
    }
    if(realIsZero(z)) {
      realSetOne(lower);
      realSetZero(upper);
      return true;
    }
    if(realCompareEqual(a, b) && realCompareEqual(y, z)) {
      realCopy(const_1on2, lower);
      realCopy(const_1on2, upper);
      return true;
    }
    realSetOne(&largeThreshold);
    largeThreshold.exponent = 5;
    if(realCompareGreaterEqual(a, &largeThreshold) && realCompareGreaterEqual(b, &largeThreshold)) {
      fLargeBetaPair(a, b, y, logit, lower, upper, realContext);
      return true;
    }

    fLogBetaPrefix(a, b, y, z, logit, &logPrefix, realContext);
    realExp(&logPrefix, &value, realContext);
    realAdd(a, b, &term, realContext);
    realAdd(&term, const_2, &term, realContext);
    realAdd(a, const_1, &threshold, realContext);
    realDivide(&threshold, &term, &threshold, realContext);
    if(realCompareLessThan(y, &threshold)) {  // next to 1 a step of the fraction cancels as many digits as the larger shape has: past about 5E14 it settles at p75 only
      if(!WP34S_BetaFraction(a, b, y, -42, 1000, &fraction, realContext)) {
        // The retry cancels its argument against 1 as well, which leaves the digits below its leading nines, so it takes y from the logit at p75. For a shape of
        // 5E39 its change stalls between 1E-41 and 1E-39 while the value is good to 1E-35, so it settles at 1E-39.
        fLogitToBetaPair(logit, &wideY, &wideZ, &ctxtReal75);
        if(!WP34S_BetaFraction(a, b, &wideY, -39, 1000, &fraction, &ctxtReal75)) {
          return false;
        }
      }
      realDivide(&fraction, a, &fraction, realContext);
      realMultiply(&value, &fraction, lower, realContext);
      realSubtract(const_1, lower, upper, realContext);
    }
    else {
      if(!WP34S_BetaFraction(b, a, z, -42, 1000, &fraction, realContext)) {
        fLogitToBetaPair(logit, &wideY, &wideZ, &ctxtReal75);  // z to 75 digits and 1E-39, as for the lower tail
        if(!WP34S_BetaFraction(b, a, &wideZ, -39, 1000, &fraction, &ctxtReal75)) {
          return false;
        }
      }
      realDivide(&fraction, b, &fraction, realContext);
      realMultiply(&value, &fraction, upper, realContext);
      realSubtract(const_1, upper, lower, realContext);
    }
    return true;
  }


  /********************************************//**
   * \brief Computes the logarithm of the regularised-beta prefix.
   ***********************************************/
  void distributionLogBetaPrefix(const real_t *a, const real_t *b, const real_t *y, const real_t *z, const real_t *logit,
                                 real_t *res, realContext_t *realContext) {
    fLogBetaPrefix(a, b, y, z, logit, res, realContext);
  }


  /********************************************//**
   * \brief Computes both regularised-beta tails from a probability pair and its logit.
   ***********************************************/
  bool_t distributionBetaPair(const real_t *a, const real_t *b, const real_t *y, const real_t *z, const real_t *logit,
                              real_t *lower, real_t *upper, realContext_t *realContext) {
    return fBetaPair(a, b, y, z, logit, lower, upper, realContext);
  }


  static bool_t fEvaluate(const real_t *x, const real_t *d1, const real_t *d2, real_t *lower, real_t *upper, real_t *logPrefix) {
    real_t a, b, logit, term, y, z, largeThreshold;
    realContext_t *calculationContext = &ctxtReal51;

    int32ToReal(2, &largeThreshold);
    largeThreshold.exponent = 5;
    if(realCompareGreaterEqual(d1, &largeThreshold) && realCompareGreaterEqual(d2, &largeThreshold)) {
      calculationContext = &ctxtReal75;
    }
    WP34S_Ln(x, &logit, calculationContext);
    WP34S_Ln(d1, &term, calculationContext);
    realAdd(&logit, &term, &logit, calculationContext);
    WP34S_Ln(d2, &term, calculationContext);
    realSubtract(&logit, &term, &logit, calculationContext);
    fLogitToBetaPair(&logit, &y, &z, calculationContext);
    realMultiply(d1, const_1on2, &a, calculationContext);
    realMultiply(d2, const_1on2, &b, calculationContext);
    if(logPrefix != NULL) {
      fLogBetaPrefix(&a, &b, &y, &z, &logit, logPrefix, calculationContext);
    }
    if(lower != NULL && upper != NULL) {
      return fBetaPair(&a, &b, &y, &z, &logit, lower, upper, calculationContext);
    }
    return true;
  }


  void WP34S_Pdf_F(const real_t *x, const real_t *d1, const real_t *d2, real_t *res, realContext_t *realContext) {
    real_t logDensity, logX;

    if(realIsNaN(x) || !fParametersAreValid(d1, d2) || realIsNegative(x)) {
      realSetNaN(res);
      return;
    }
    if(realIsZero(x)) {
      if(realCompareLessThan(d1, const_2)) {
        realSetPlusInfinity(res);
      }
      else if(realCompareEqual(d1, const_2)) {
        realSetOne(res);
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

    fEvaluate(x, d1, d2, NULL, NULL, &logDensity);
    WP34S_Ln(x, &logX, &ctxtReal75);
    realSubtract(&logDensity, &logX, &logDensity, &ctxtReal75);
    realExp(&logDensity, &logDensity, &ctxtReal75);
    realPlus(&logDensity, res, realContext);
  }


  void WP34S_Cdfu_F(const real_t *x, const real_t *d1, const real_t *d2, real_t *res, realContext_t *realContext) {
    real_t lower, upper;

    if(realIsNaN(x) || !fParametersAreValid(d1, d2) || realIsNegative(x)) {
      realSetNaN(res);
      return;
    }
    if(realIsZero(x)) {
      realSetOne(res);
      return;
    }
    if(realIsInfinite(x)) {
      realSetZero(res);
      return;
    }
    if(!fEvaluate(x, d1, d2, &lower, &upper, NULL)) {
      realSetNaN(res);
      return;
    }
    realPlus(&upper, res, realContext);
  }


  void WP34S_Cdf_F(const real_t *x, const real_t *d1, const real_t *d2, real_t *res, realContext_t *realContext) {
    real_t lower, upper;

    if(realIsNaN(x) || !fParametersAreValid(d1, d2) || realIsNegative(x)) {
      realSetNaN(res);
      return;
    }
    if(realIsZero(x)) {
      realSetZero(res);
      return;
    }
    if(realIsInfinite(x)) {
      realSetOne(res);
      return;
    }
    if(!fEvaluate(x, d1, d2, &lower, &upper, NULL)) {
      realSetNaN(res);
      return;
    }
    realPlus(&lower, res, realContext);
  }


  static bool_t fInverseLower(const real_t *target, const real_t *d1, const real_t *d2, real_t *res) {
    real_t a, accuracy, b, bound, derivative, estimateTolerance, high, largeThreshold, logD1, logD2, logit, low, lower, upper, midpoint, proposal, residual, step, tolerance, value, width, y, z;
    realContext_t *calculationContext = &ctxtReal51;
    int32_t i;

    int32ToReal(2, &largeThreshold);
    largeThreshold.exponent = 5;
    if(realCompareGreaterEqual(d1, &largeThreshold) && realCompareGreaterEqual(d2, &largeThreshold)) {
      calculationContext = &ctxtReal75;
    }
    realMultiply(d1, const_1on2, &a, calculationContext);
    realMultiply(d2, const_1on2, &b, calculationContext);
    WP34S_Ln(d1, &logD1, calculationContext);
    WP34S_Ln(d2, &logD2, calculationContext);
    int32ToReal(16000, &bound);
    realMinus(&bound, &low, calculationContext);
    realCopy(&bound, &high);
    realSetZero(&value);
    realSetOne(&tolerance);
    tolerance.exponent = -40;
    realSetOne(&estimateTolerance);
    estimateTolerance.exponent = -38;

    for(i=0; i<320; i++) {
      realAdd(&value, &logD1, &logit, calculationContext);
      realSubtract(&logit, &logD2, &logit, calculationContext);
      fLogitToBetaPair(&logit, &y, &z, calculationContext);
      if(!fBetaPair(&a, &b, &y, &z, &logit, &lower, &upper, calculationContext)) {
        return false;
      }
      realSubtract(&lower, target, &residual, calculationContext);
      realMultiply(target, &tolerance, &accuracy, calculationContext);
      if(realIsZero(&residual) || realCompareAbsLessThan(&residual, &accuracy)) {
        realExp(&value, res, calculationContext);
        return true;
      }
      if(realIsNegative(&residual)) {
        realCopy(&value, &low);
      }
      else {
        realCopy(&value, &high);
      }
      realSubtract(&high, &low, &width, calculationContext);
      if(realCompareAbsLessThan(&width, &tolerance)) {
        realAdd(&low, &high, &value, calculationContext);
        realMultiply(&value, const_1on2, &value, calculationContext);
        realExp(&value, res, calculationContext);
        return true;
      }

      fLogBetaPrefix(&a, &b, &y, &z, &logit, &derivative, calculationContext);
      realExp(&derivative, &derivative, calculationContext);
      realAdd(&low, &high, &midpoint, calculationContext);
      realMultiply(&midpoint, const_1on2, &midpoint, calculationContext);
      if(i >= 24 || realIsZero(&derivative)) {
        realCopy(&midpoint, &proposal);
      }
      else {
        realDivide(&residual, &derivative, &step, calculationContext);
        realSubtract(&value, &step, &proposal, calculationContext);
        if(realCompareLessEqual(&proposal, &low) || realCompareGreaterEqual(&proposal, &high)) {
          realCopy(&midpoint, &proposal);
        }
      }
      realSubtract(&proposal, &value, &step, calculationContext);
      if(realCompareAbsLessThan(&step, &estimateTolerance)) {
        realExp(&proposal, res, calculationContext);
        return true;
      }
      realCopy(&proposal, &value);
    }
    return false;
  }


  void WP34S_Qf_F(const real_t *x, const real_t *d1, const real_t *d2, real_t *res, realContext_t *realContext) {
    real_t probability, value;
    bool_t reciprocal;

    if(realIsNaN(x) || !fParametersAreValid(d1, d2) || realCompareLessEqual(x, const_0) || realCompareGreaterEqual(x, const_1)) {
      realSetNaN(res);
      return;
    }
    reciprocal = realCompareGreaterThan(x, const_1on2);
    if(reciprocal) {
      realSubtract(const_1, x, &probability, &ctxtReal75);
      if(!fInverseLower(&probability, d2, d1, &value)) {
        realSetNaN(res);
        return;
      }
      realDivide(const_1, &value, &value, &ctxtReal75);
    }
    else {
      realCopy(x, &probability);
      if(!fInverseLower(&probability, d1, d2, &value)) {
        realSetNaN(res);
        return;
      }
    }
    realPlus(&value, res, realContext);
  }


  /**************************************************************************/
  /* The Newton step solver.
   * Target value in Y, function identifier in X, estimate in Z.
   */
  void WP34S_Qf_Newton(uint32_t r_dist, const real_t *target, const real_t *estimate, const real_t *p1, const real_t *p2, const real_t *p3, real_t *res, realContext_t *realContext) {
    real_t p, q;
    real_t r_p, r_low, r_high, r_maxstep, r_r, r_z, r_w;
    uint32_t r_iterations;
    bool_t f_discrete = false, f_nonnegative = false, f_nobisect = false, f_limitjump = false;

    // qf_newton
    realCopy(target, &r_p);
    realCopy(estimate, &r_r);

    /* Set flags based on distribution */
    // f_newton_setflags
    if(r_dist != QF_NEWTON_F) {
      f_discrete = true; // Poisson or Binomial
    }

    // qf_f_flags
    realMultiply(const_3on2, const_1on2, &p, realContext);
    realMultiply(&p, &r_r, &p, realContext);
    realCopy(&p, &r_maxstep);
    f_limitjump = true;
    f_nonnegative = true;

    r_iterations = 100;
    realSetPlusInfinity(&r_high);
    realCopy(f_nonnegative ? const_0 : const_minusInfinity, &r_low);

    /* Initialisation done, now the main loop itself */
    do { // qf_newton_search
      if(realCompareGreaterEqual(&r_r, &r_high) || realCompareLessEqual(&r_r, &r_low)) { // qf_newton_try_bisect
        /* The estimate is out of the (low, high) bounds.  Check to see if we can bisect instead */
        if(realIsInfinite(&r_high)) { // qf_newton_overlow
          realMultiply(&r_low, const_2, &r_r, realContext);
        }
        else if(realIsInfinite(&r_low)) { // qf_newton_underhigh
          realMultiply(&r_high, const_2, &r_r, realContext);
        }
        else {
  qf_newton_do_bisect:
          realAdd(&r_low, &r_high, &r_r, realContext);
          realMultiply(&r_r, const_1on2, &r_r, realContext);
          f_nobisect = true;
        }
        //qf_newton_bisect_out
      }
      // qf_newton_no_bisect
      /* Dispatch to the CDF of the distribution being searched */
      switch(r_dist) { // qf_newton_cdf
        case QF_NEWTON_F:              WP34S_Cdf_F(&r_r, p1, p2, &r_w, realContext);             break;
        case QF_NEWTON_POISSON:        WP34S_Cdf_Poisson2(&r_r, p1, &r_w, realContext);          break;
        case QF_NEWTON_BINOMIAL:       WP34S_Cdf_Binomial2(&r_r, p1, p2, &r_w, realContext);     break;
        case QF_NEWTON_NEGBINOM:       cdf_NegBinomial2(&r_r, p1, p2, &r_w, realContext);        break;
        case QF_NEWTON_HYPERGEOMETRIC: cdf_Hypergeometric2(&r_r, p1, p2, p3, &r_w, &ctxtReal75); break;
      }
      realSubtract(&r_w, &r_p, &r_z, realContext);
      if(realCompareGreaterEqual(&r_z, const_0)) { // qf_newton_fix_high
        if(f_nobisect || realCompareLessThan(&r_r, &r_high) || realIsInfinite(&r_low)) {
          f_nobisect = false;
          if(realCompareLessThan(&r_r, &r_high)) { // qf_newton_fix_high2
            realCopy(&r_r, &r_high);
          }
          goto qf_newton_fixed;
        }
        goto qf_newton_do_bisect;
      }
      else { // qf_newton_fix_low
        if(f_nobisect || realCompareGreaterThan(&r_r, &r_low) || realIsInfinite(&r_high)) {
          f_nobisect = false;
          if(realCompareGreaterThan(&r_r, &r_low)) { // qf_newton_fix_low2
            realCopy(&r_r, &r_low);
          }
          goto qf_newton_fixed;
        }
        goto qf_newton_do_bisect;
      }
      /* Evaluated the CDF and it is within the bounds */
  qf_newton_fixed:
      /* Dispatch to the PDF of the distribution being searched */
      switch(r_dist) { // qf_newton_pdf
        case QF_NEWTON_F:              WP34S_Pdf_F(&r_r, p1, p2, &q, realContext);            break;
        case QF_NEWTON_POISSON:        WP34S_Pdf_Poisson(&r_r, p1, &q, realContext);          break;
        case QF_NEWTON_BINOMIAL:       WP34S_Pdf_Binomial(&r_r, p1, p2, &q, realContext);     break;
        case QF_NEWTON_NEGBINOM:       pdf_NegBinomial(&r_r, p1, p2, &q, realContext);        break;
        case QF_NEWTON_HYPERGEOMETRIC: pdf_Hypergeometric(&r_r, p1, p2, p3, &q, realContext); break;
      }
      if(realIsZero(&q)) {
        goto qf_newton_done; /* Function is flat -- can't estimate */
      }
      realDivide(&r_z, &q, &r_w, realContext);
      realCopyAbs(&r_w, &q);
      if(f_limitjump && realCompareGreaterEqual(&q, &r_maxstep)) {
        if(realCompareLessThan(&r_w, const_0)) { // qf_newton_neg_limit
          realMinus(&r_maxstep, &r_w, realContext);
        }
        else { // qf_newton_fin_limit
          realCopy(&r_maxstep, &r_w);
        }
      }
      // update estimate
      // qf_newton_no_limit
      realCopy(&r_r, &r_z);
      realSubtract(&r_r, &r_w, &r_r, realContext);

      /* If this distribution doesn't take negative values, limit outselves to positive ones */
      if(f_nonnegative && realCompareLessThan(&r_r, const_0)) {
        realCopy(&r_z, &r_r), r_r.exponent -= 5;
      }

      /* If our upper and lower limits are close enough together we give up searching */
      // qf_newton_nosign
      if(!realIsSpecial(&r_high) && !realIsSpecial(&r_low) && WP34S_RelativeError(&r_low, &r_high, const_1e_37, realContext)) {
        // qf_newton_bounds_cnvg
        realAdd(&r_high, &r_low, &p, realContext);
        realMultiply(&p, const_1on2, &p, realContext);
        goto qf_newton_done2;
      }
      /* Check for convergence of the estimates */
      else { // qf_newton_nobounds
        if(f_discrete) { // qf_newton_dis_end
          /* Check for discrete distribution convergence */
          realSubtract(&r_z, &r_r, &p, realContext);
          realSetPositiveSign(&p);
          if(realCompareGreaterThan(const_1on10, &p)) {
            goto qf_newton_done;
          }
        }
        /* Check for continuous distribution convergence */
        else if(WP34S_RelativeError(&r_r, &r_z, const_1e_37, realContext)) {
          goto qf_newton_done;
        }
      }
      // qf_newton_again
    } while(--r_iterations > 0);
    if(f_discrete) {
      goto qf_newton_done;  // the final search reaches the integer from wherever the iteration stops
    }
    realSetNaN(res);
    return;

    /* Finished.  For discrete distributions, there is a bit more to do */
  qf_newton_done:
    realCopy(&r_r, &p);
  qf_newton_done2:
    if(f_discrete) { // qf_discrete_exit
      realToIntegralValue(&p, &q, DEC_ROUND_FLOOR, realContext);
      WP34S_qf_discrete_final(r_dist - 1, &q, &r_p, p1, p2, p3, &p, realContext);
    }
    realCopy(&p, res);
    return;
  }

#endif // !OPTION_DIST_F
