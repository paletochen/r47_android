// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file t.c
 ***********************************************/

#include "c47.h"

#if !defined(OPTION_DIST_T)
  void fnT_P   (uint16_t unusedButMandatoryParameter){}
  void fnT_L   (uint16_t unusedButMandatoryParameter){}
  void fnT_R   (uint16_t unusedButMandatoryParameter){}
  void fnT_I   (uint16_t unusedButMandatoryParameter){}

#else
  bool_t studentTCheckDomain(bool_t registersValid, real_t *x, real_t *i, real_t *unusedSecond, real_t *unusedThird, uint16_t unusedParameter) {
    if(!registersValid) {
      return false;
    }

    if(realIsNaN(i) || realIsZero(i) || realIsNegative(i)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function studentTCheckDomain:", "cannot calculate for " STD_nu " " STD_LESS_EQUAL " 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    return true;
  }


  void fnT_P(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_T * 4 + DIST_P);
  }


  void fnT_L(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_T * 4 + DIST_L);
  }


  void fnT_R(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_T * 4 + DIST_R);
  }


  void fnT_I(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_T * 4 + DIST_I);
  }



  /******************************************************
   * This functions are borrowed from the WP34S project
   ******************************************************/

  static void studentTLogASeriesOverA(const real_t *a, real_t *res) {
    real_t term, squared, cubed, zeta3, three;

    realCopy(const51_twoLn2, res);
    realMultiply(a, a, &squared, &ctxtReal51);
    realMultiply(&squared, a, &cubed, &ctxtReal51);
    realMultiply(const51_zeta2, a, &term, &ctxtReal51);
    realSubtract(res, &term, res, &ctxtReal51);
    int32ToReal(3, &three);
    WP34S_Zeta(&three, &zeta3, &ctxtReal51);
    realMultiply(&zeta3, &squared, &term, &ctxtReal51);
    realMultiply(&term, const_2, &term, &ctxtReal51);
    realAdd(res, &term, res, &ctxtReal51);
    realMultiply(const51_7zeta2SqOn5, &cubed, &term, &ctxtReal51);
    realSubtract(res, &term, res, &ctxtReal51);
  }


  static void studentTLogNormalisation(const real_t *nu, real_t *res, realContext_t *realContext) {
    real_t halfNu, x, shiftedX, sum, shiftedSum, r, term, value;
    realContext_t *calculationContext = realContext->digits > 51 ? realContext : &ctxtReal51;

    realMultiply(nu, const_1on2, &halfNu, calculationContext);
    if(realCompareLessEqual(&halfNu, const_1e_12)) {
      studentTLogASeriesOverA(&halfNu, &value);
      realMultiply(&halfNu, &value, &value, calculationContext);
      WP34S_Ln(nu, &term, calculationContext);
      realMultiply(&term, const_1on2, &term, calculationContext);
      realSubtract(&term, const75_ln2, &term, calculationContext);
      realSubtract(&term, &value, res, calculationContext);
      return;
    }
    realSubtract(&halfNu, const_1, &x, calculationContext);
    realAdd(&x, const_1on2, &shiftedX, calculationContext);
    WP34S_LanczosSum(&x, const_29, &sum, calculationContext);
    WP34S_LanczosSum(&shiftedX, const_29, &shiftedSum, calculationContext);

    realDivide(&shiftedSum, &sum, &value, calculationContext);
    WP34S_Ln(&value, &value, calculationContext);

    realAdd(&x, const_gammaR, &r, calculationContext);
    realDivide(const_1on2, &r, &term, calculationContext);
    WP34S_Ln1P(&term, &term, calculationContext);
    realMultiply(&halfNu, &term, &term, calculationContext);
    realSubtract(&term, const_1on2, &term, calculationContext);
    realAdd(&value, &term, &value, calculationContext);

    realDivide(&r, nu, &term, calculationContext);
    realDivide(&term, const75_pi, &term, calculationContext);
    WP34S_Ln(&term, &term, calculationContext);
    realMultiply(&term, const_1on2, &term, calculationContext);
    realAdd(&value, &term, res, calculationContext);
  }


  static void studentTLogBetaHalf(const real_t *nu, real_t *res, realContext_t *realContext) {
    real_t logNormalisation, logNu;
    realContext_t *calculationContext = realContext->digits > 51 ? realContext : &ctxtReal51;

    studentTLogNormalisation(nu, &logNormalisation, calculationContext);
    WP34S_Ln(nu, &logNu, calculationContext);
    realMultiply(&logNu, const_1on2, &logNu, calculationContext);
    realAdd(&logNormalisation, &logNu, res, calculationContext);
    realChangeSign(res);
  }


  static void studentTBetaUpper(const real_t *a, const real_t *b, const real_t *x, const real_t *oneMinusX, const real_t *logPrefix, real_t *res,
                                realContext_t *realContext) {
    real_t threshold, fraction, value, term;

    if(realIsZero(x)) {
      realSetOne(res);
      return;
    }
    if(realIsZero(oneMinusX)) {
      realSetZero(res);
      return;
    }

    realExp(logPrefix, &value, realContext);
    realAdd(a, b, &term, realContext);
    realAdd(&term, const_2, &term, realContext);
    realAdd(a, const_1, &threshold, realContext);
    realDivide(&threshold, &term, &threshold, realContext);

    if(realCompareLessThan(x, &threshold)) {
      if(!WP34S_BetaFraction(a, b, x, -37, 500, &fraction, realContext)) {
        realSetNaN(res);
        return;
      }
      realDivide(&fraction, a, &fraction, realContext);
      realMultiply(&value, &fraction, &value, realContext);
      realSubtract(const_1, &value, res, realContext);
    }
    else {
      if(!WP34S_BetaFraction(b, a, oneMinusX, -37, 500, &fraction, realContext)) {
        realSetNaN(res);
        return;
      }
      realDivide(&fraction, b, &fraction, realContext);
      realMultiply(&value, &fraction, res, realContext);
    }
  }


  static void studentTBetaLowerDirect(const real_t *a, const real_t *b, const real_t *x, const real_t *logPrefix, real_t *res,
                                      realContext_t *realContext) {
    real_t fraction, value;

    if(realIsZero(x)) {
      realSetZero(res);
      return;
    }
    if(!WP34S_BetaFraction(a, b, x, -37, 500, &fraction, realContext)) {
      realSetNaN(res);
      return;
    }
    realExp(logPrefix, &value, realContext);
    realDivide(&fraction, a, &fraction, realContext);
    realMultiply(&value, &fraction, res, realContext);
  }


  static void studentTPolynomial(const real_t *x, const real_t *xSquared, const int32_t *coefficients, int32_t coefficientCount, int32_t denominator, real_t *res,
                                 realContext_t *realContext) {
    real_t coefficient, value, divisor;
    int32_t i;

    int32ToReal(coefficients[0], &value);
    for(i=1; i<coefficientCount; i++) {
      int32ToReal(coefficients[i], &coefficient);
      realFMA(&value, xSquared, &coefficient, &value, realContext);
    }
    realMultiply(&value, x, &value, realContext);
    int32ToReal(denominator, &divisor);
    realDivide(&value, &divisor, res, realContext);
  }


  static void studentTNormalAdjustment(const real_t *x, const real_t *nu, real_t *res, realContext_t *realContext) {
    TO_QSPI static const int32_t q1[] = { -1, -1 };
    TO_QSPI static const int32_t q2[] = { -3, 7, 5, 3 };
    TO_QSPI static const int32_t q3[] = { -1, 11, -14, -6, 3, 15 };
    TO_QSPI static const int32_t q4[] = { -15, 375, -2225, 2141, 939, 213, -915, -945 };
    real_t xSquared, inverseNu, p1, p2, p3, p4, correction, density;

    WP34S_Pdf_Q(x, &density, realContext);
    realMultiply(x, x, &xSquared, realContext);
    realDivide(const_1, nu, &inverseNu, realContext);
    studentTPolynomial(x, &xSquared, q1, 2, 4, &p1, realContext);
    studentTPolynomial(x, &xSquared, q2, 4, 96, &p2, realContext);
    studentTPolynomial(x, &xSquared, q3, 6, 384, &p3, realContext);
    studentTPolynomial(x, &xSquared, q4, 8, 92160, &p4, realContext);
    realFMA(&p4, &inverseNu, &p3, &correction, realContext);
    realFMA(&correction, &inverseNu, &p2, &correction, realContext);
    realFMA(&correction, &inverseNu, &p1, &correction, realContext);
    realMultiply(&correction, &inverseNu, &correction, realContext);
    realMultiply(&density, &correction, res, realContext);
  }


  static void studentTNormalUpper(const real_t *x, const real_t *nu, real_t *res, realContext_t *realContext) {
    real_t adjustment, tail;

    WP34S_Cdfu_Q(x, &tail, realContext);
    studentTNormalAdjustment(x, nu, &adjustment, realContext);
    realSubtract(&tail, &adjustment, res, realContext);
    if(realCompareLessEqual(res, const_0)) {
      realCopy(const_1e_10000, res);
    }
  }


  static void studentTNormalCentral(const real_t *x, const real_t *nu, real_t *res, realContext_t *realContext) {
    real_t rootTwo, scaled, central, adjustment;

    realSquareRoot(const_2, &rootTwo, realContext);
    realDivide(x, &rootTwo, &scaled, realContext);
    WP34S_Erf(&scaled, &central, realContext);
    realMultiply(&central, const_1on2, &central, realContext);
    if(!realIsInfinite(nu)) {
      studentTNormalAdjustment(x, nu, &adjustment, realContext);
      realAdd(&central, &adjustment, &central, realContext);
    }
    realCopy(&central, res);
  }


  static void studentTUpperMagnitudeKnownBeta(const real_t *x, const real_t *nu, const real_t *knownLogBeta, real_t *res, realContext_t *realContext) {
    real_t largeNu, xSquared, ratio, denominator, betaX, betaComplement, halfNu, logBeta, logRatio, logPrefix, tail;
    realContext_t *calculationContext = realContext->digits > 51 ? realContext : &ctxtReal51;

    if(realIsNaN(x) || realIsNaN(nu) || realCompareLessEqual(nu, const_0)) {
      realSetNaN(res);
      return;
    }
    if(realIsZero(x)) {
      realCopy(const_1on2, res);
      return;
    }
    if(realIsInfinite(x)) {
      realSetZero(res);
      return;
    }
    if(realIsInfinite(nu)) {
      WP34S_Cdfu_Q(x, res, realContext);
      return;
    }

    WP34S_SetPowerOfTen(17, &largeNu);
    if(realCompareGreaterEqual(nu, &largeNu)) {
      studentTNormalUpper(x, nu, res, calculationContext);
      return;
    }

    realMultiply(x, x, &xSquared, calculationContext);
    realAdd(&xSquared, nu, &denominator, calculationContext);
    realDivide(&xSquared, &denominator, &betaX, calculationContext);
    realDivide(nu, &denominator, &betaComplement, calculationContext);
    realDivide(&xSquared, nu, &ratio, calculationContext);
    WP34S_Ln(&ratio, &logRatio, calculationContext);
    WP34S_Ln1P(&ratio, &logPrefix, calculationContext);
    realSubtract(&logRatio, &logPrefix, &logRatio, calculationContext);
    realMultiply(&logRatio, const_1on2, &logRatio, calculationContext);
    realMultiply(nu, const_1on2, &halfNu, calculationContext);
    realMultiply(&halfNu, &logPrefix, &logPrefix, calculationContext);
    realChangeSign(&logPrefix);
    realAdd(&logRatio, &logPrefix, &logPrefix, calculationContext);
    if(knownLogBeta == NULL) {
      studentTLogBetaHalf(nu, &logBeta, calculationContext);
    }
    else {
      realCopy(knownLogBeta, &logBeta);
    }
    realSubtract(&logPrefix, &logBeta, &logPrefix, calculationContext);
    studentTBetaUpper(const_1on2, &halfNu, &betaX, &betaComplement, &logPrefix, &tail, calculationContext);
    realMultiply(&tail, const_1on2, &tail, calculationContext);
    if(realIsZero(&tail)) {
      realCopy(const_1e_10000, &tail);
    }
    realPlus(&tail, res, realContext);
  }


  static void studentTUpperMagnitude(const real_t *x, const real_t *nu, real_t *res, realContext_t *realContext) {
    studentTUpperMagnitudeKnownBeta(x, nu, NULL, res, realContext);
  }


  static void studentTCentralMagnitudeKnownBeta(const real_t *x, const real_t *nu, const real_t *knownLogBeta, real_t *res,
                                                realContext_t *realContext) {
    real_t largeNu, xSquared, ratio, denominator, betaX, halfNu, logBeta, logRatio, logPrefix, central;
    realContext_t *calculationContext = realContext->digits > 51 ? realContext : &ctxtReal51;

    if(realIsZero(x)) {
      realSetZero(res);
      return;
    }
    if(realIsInfinite(x)) {
      realCopy(const_1on2, res);
      return;
    }
    WP34S_SetPowerOfTen(17, &largeNu);
    if(realIsInfinite(nu) || realCompareGreaterEqual(nu, &largeNu)) {
      studentTNormalCentral(x, nu, res, calculationContext);
      return;
    }

    realMultiply(x, x, &xSquared, calculationContext);
    realAdd(&xSquared, nu, &denominator, calculationContext);
    realDivide(&xSquared, &denominator, &betaX, calculationContext);
    realDivide(&xSquared, nu, &ratio, calculationContext);
    WP34S_Ln(&ratio, &logRatio, calculationContext);
    WP34S_Ln1P(&ratio, &logPrefix, calculationContext);
    realSubtract(&logRatio, &logPrefix, &logRatio, calculationContext);
    realMultiply(&logRatio, const_1on2, &logRatio, calculationContext);
    realMultiply(nu, const_1on2, &halfNu, calculationContext);
    realMultiply(&halfNu, &logPrefix, &logPrefix, calculationContext);
    realChangeSign(&logPrefix);
    realAdd(&logRatio, &logPrefix, &logPrefix, calculationContext);
    if(knownLogBeta == NULL) {
      studentTLogBetaHalf(nu, &logBeta, calculationContext);
    }
    else {
      realCopy(knownLogBeta, &logBeta);
    }
    realSubtract(&logPrefix, &logBeta, &logPrefix, calculationContext);
    studentTBetaLowerDirect(const_1on2, &halfNu, &betaX, &logPrefix, &central, calculationContext);
    realMultiply(&central, const_1on2, &central, calculationContext);
    realPlus(&central, res, realContext);
  }


  static void studentTInverseResidual(const real_t *value, const real_t *nu, const real_t *knownLogBeta, const real_t *probability, const real_t *delta,
                                      real_t *res, realContext_t *realContext) {
    real_t probabilityValue;

    if(realCompareLessThan(value, const_1)) {
      studentTCentralMagnitudeKnownBeta(value, nu, knownLogBeta, &probabilityValue, realContext);
      realSubtract(delta, &probabilityValue, res, realContext);
    }
    else {
      studentTUpperMagnitudeKnownBeta(value, nu, knownLogBeta, &probabilityValue, realContext);
      realSubtract(&probabilityValue, probability, res, realContext);
    }
  }

  void WP34S_Pdf_T(const real_t *x, const real_t *nu, real_t *res, realContext_t *realContext) {
    real_t xSquared, ratio, exponent, coefficient, logNormalisation;
    realContext_t *calculationContext = realContext->digits > 51 ? realContext : &ctxtReal51;

    if(realIsNaN(x) || realIsNaN(nu) || realCompareLessEqual(nu, const_0)) {
      realSetNaN(res);
      return;
    }
    if(realIsInfinite(nu)) {
      WP34S_Pdf_Q(x, res, realContext);
      return;
    }
    if(realIsInfinite(x)) {
      realSetZero(res);
      return;
    }

    studentTLogNormalisation(nu, &logNormalisation, calculationContext);
    realMultiply(x, x, &xSquared, calculationContext);
    realDivide(&xSquared, nu, &ratio, calculationContext);
    WP34S_Ln1P(&ratio, &exponent, calculationContext);
    realAdd(nu, const_1, &coefficient, calculationContext);
    realMultiply(&coefficient, const_1on2, &coefficient, calculationContext);
    realMultiply(&exponent, &coefficient, &exponent, calculationContext);
    realSubtract(&logNormalisation, &exponent, &exponent, calculationContext);
    realExp(&exponent, &exponent, calculationContext);
    if(realIsZero(&exponent)) {
      realCopy(const_1e_10000, &exponent);
    }
    realPlus(&exponent, res, realContext);
  }

  static void cdf_t(const real_t *x, const real_t *nu, real_t *res, realContext_t *realContext) {
    real_t magnitude, tail, value;
    realContext_t *calculationContext = realContext->digits > 51 ? realContext : &ctxtReal51;

    realCopy(x, &magnitude);
    realSetPositiveSign(&magnitude);
    studentTUpperMagnitude(&magnitude, nu, &tail, calculationContext);
    if(realIsNegative(x)) {
      realCopy(&tail, &value);
    }
    else {
      realSubtract(const_1, &tail, &value, calculationContext);
    }
    realPlus(&value, res, realContext);
  }

  void WP34S_Cdfu_T(const real_t *x, const real_t *nu, real_t *res, realContext_t *realContext) {
    real_t xn;

    realMinus(x, &xn, realContext);
    cdf_t(&xn, nu, res, realContext);
  }

  void WP34S_Cdf_T(const real_t *x, const real_t *nu, real_t *res, realContext_t *realContext) {
    cdf_t(x, nu, res, realContext);
  }

  static void studentTLogAOverA(const real_t *nu, const real_t *a, real_t *aBeta, real_t *res) {
    real_t logBeta, logA;

    if(realCompareLessEqual(a, const_1e_12)) {
      studentTLogASeriesOverA(a, res);
      realMultiply(a, res, &logA, &ctxtReal51);
    }
    else {
      studentTLogBetaHalf(nu, &logBeta, &ctxtReal51);
      WP34S_Ln(a, &logA, &ctxtReal51);
      realAdd(&logA, &logBeta, &logA, &ctxtReal51);
      realDivide(&logA, a, res, &ctxtReal51);
    }
    realExp(&logA, aBeta, &ctxtReal51);
  }


  static bool_t studentTCentralSeries(const real_t *a, const real_t *q, real_t *res) {
    real_t sum, old, term, k, factor, numerator, denominator;
    int32_t i;

    realSetOne(&sum);
    realSetOne(&term);
    for(i=1; i<=500; i++) {
      realCopy(&sum, &old);
      int32ToReal(i, &k);
      realSubtract(&k, const_1on2, &factor, &ctxtReal51);
      realSubtract(&k, a, &numerator, &ctxtReal51);
      realMultiply(&factor, &numerator, &numerator, &ctxtReal51);
      realMultiply(&numerator, q, &numerator, &ctxtReal51);
      realAdd(&k, const_1on2, &factor, &ctxtReal51);
      realMultiply(&factor, &k, &denominator, &ctxtReal51);
      realDivide(&numerator, &denominator, &factor, &ctxtReal51);
      realMultiply(&term, &factor, &term, &ctxtReal51);
      realAdd(&sum, &term, &sum, &ctxtReal51);
      if(realCompareEqual(&old, &sum)) {
        realCopy(&sum, res);
        return true;
      }
    }
    realSetNaN(res);
    return false;
  }


  static bool_t studentTTailSeries(const real_t *a, const real_t *z, real_t *logHOverA, real_t *derivative) {
    real_t j, weighted, old, base, term, k, factor, denominator, aJ, h;
    int32_t i;

    realSetZero(&j);
    realSetZero(&weighted);
    realSetOne(&base);
    for(i=1; i<=500; i++) {
      realCopy(&j, &old);
      int32ToReal(i, &k);
      realSubtract(&k, const_1on2, &factor, &ctxtReal51);
      realMultiply(&factor, z, &factor, &ctxtReal51);
      realDivide(&factor, &k, &factor, &ctxtReal51);
      realMultiply(&base, &factor, &base, &ctxtReal51);
      realAdd(a, &k, &denominator, &ctxtReal51);
      realDivide(&base, &denominator, &term, &ctxtReal51);
      realAdd(&j, &term, &j, &ctxtReal51);
      realFMA(&k, &term, &weighted, &weighted, &ctxtReal51);
      if(realCompareEqual(&old, &j)) {
        realMultiply(a, &j, &aJ, &ctxtReal51);
        realAdd(const_1, &aJ, &h, &ctxtReal51);
        WP34S_Ln1P(&aJ, logHOverA, &ctxtReal51);
        realDivide(logHOverA, a, logHOverA, &ctxtReal51);
        realDivide(&weighted, &h, derivative, &ctxtReal51);
        realAdd(derivative, const_1, derivative, &ctxtReal51);
        return true;
      }
    }
    realSetNaN(logHOverA);
    realSetNaN(derivative);
    return false;
  }


  static bool_t studentTTailEquation(const real_t *s, const real_t *a, const real_t *logAOverA, const real_t *logTwoPOverA, real_t *value,
                                     real_t *derivative) {
    real_t z, logHOverA;

    realMinus(s, &z, &ctxtReal51);
    realExp(&z, &z, &ctxtReal51);
    if(!studentTTailSeries(a, &z, &logHOverA, derivative)) {
      realSetNaN(value);
      return false;
    }
    realAdd(s, logAOverA, value, &ctxtReal51);
    realSubtract(value, &logHOverA, value, &ctxtReal51);
    realAdd(value, logTwoPOverA, value, &ctxtReal51);
    return true;
  }


  static void studentTSmallNuInverse(const real_t *probability, const real_t *nu, real_t *res, realContext_t *realContext) {
    real_t a, aBeta, logAOverA, delta, target, q, r, series, boundary, low, high, midpoint, value, tolerance, oneMinusQ, rootNu, denominator;
    real_t logTwoP, logTwoPOverA, base, lowS, highS, estimate, derivative, step, lnNu, limit, maximum, exponentTerm;
    int32_t iterations;

    realMultiply(nu, const_1on2, &a, &ctxtReal51);
    studentTLogAOverA(nu, &a, &aBeta, &logAOverA);
    realSubtract(const_1on2, probability, &delta, &ctxtReal51);
    realMultiply(&delta, &aBeta, &target, &ctxtReal51);
    realDivide(&target, &a, &target, &ctxtReal51);
    realCopy(const_1on4, &q);
    realCopy(const_1on2, &r);
    if(!studentTCentralSeries(&a, &q, &series)) {
      realSetNaN(res);
      return;
    }
    realMultiply(&r, &series, &boundary, &ctxtReal51);
    WP34S_SetPowerOfTen(-40, &tolerance);

    if(realCompareLessEqual(&target, &boundary)) {
      realSetZero(&low);
      realCopy(const_1on2, &high);
      for(iterations=0; iterations<300; iterations++) {
        realAdd(&low, &high, &midpoint, &ctxtReal51);
        realMultiply(&midpoint, const_1on2, &midpoint, &ctxtReal51);
        realMultiply(&midpoint, &midpoint, &q, &ctxtReal51);
        if(!studentTCentralSeries(&a, &q, &series)) {
          realSetNaN(res);
          return;
        }
        realMultiply(&midpoint, &series, &value, &ctxtReal51);
        if(realCompareLessThan(&value, &target)) {
          realCopy(&midpoint, &low);
        }
        else {
          realCopy(&midpoint, &high);
        }
        if(WP34S_RelativeError(&high, &low, &tolerance, &ctxtReal51)) {
          break;
        }
      }
      realAdd(&low, &high, &r, &ctxtReal51);
      realMultiply(&r, const_1on2, &r, &ctxtReal51);
      realMultiply(&r, &r, &q, &ctxtReal51);
      realSubtract(const_1, &q, &oneMinusQ, &ctxtReal51);
      realSquareRoot(nu, &rootNu, &ctxtReal51);
      realSquareRoot(&oneMinusQ, &denominator, &ctxtReal51);
      realMultiply(&rootNu, &r, &value, &ctxtReal51);
      realDivide(&value, &denominator, &value, &ctxtReal51);
      realPlus(&value, res, realContext);
      return;
    }

    if(realCompareLessThan(probability, const_1on4)) {
      WP34S_Ln(probability, &logTwoP, &ctxtReal51);
      realAdd(&logTwoP, const75_ln2, &logTwoP, &ctxtReal51);
    }
    else {
      realMultiply(&delta, const_2, &logTwoP, &ctxtReal51);
      realChangeSign(&logTwoP);
      WP34S_Ln1P(&logTwoP, &logTwoP, &ctxtReal51);
    }
    realDivide(&logTwoP, &a, &logTwoPOverA, &ctxtReal51);
    realAdd(&logAOverA, &logTwoPOverA, &base, &ctxtReal51);
    realChangeSign(&base);

    WP34S_Ln(nu, &lnNu, &ctxtReal51);
    int32ToReal(12290, &limit);
    realMultiply(&limit, const75_ln10, &limit, &ctxtReal51);
    realAdd(&lnNu, &base, &exponentTerm, &ctxtReal51);
    realSubtract(&exponentTerm, const_1, &exponentTerm, &ctxtReal51);
    if(realCompareGreaterThan(&exponentTerm, &limit)) {
      WP34S_SetPowerOfTen(10000, &maximum);
      realCopy(&maximum, res);
      return;
    }

    realCopy(const51_1on3, &lowS);
    WP34S_Ln1P(&lowS, &lowS, &ctxtReal51);
    realAdd(&base, const_10, &highS, &ctxtReal51);
    if(realCompareLessEqual(&highS, &lowS)) {
      realMultiply(&lowS, const_2, &highS, &ctxtReal51);
    }
    for(iterations=0; iterations<100; iterations++) {
      if(!studentTTailEquation(&highS, &a, &logAOverA, &logTwoPOverA, &value, &derivative)) {
        realSetNaN(res);
        return;
      }
      if(!realIsNegative(&value)) {
        break;
      }
      realMultiply(&highS, const_2, &highS, &ctxtReal51);
    }
    if(iterations == 100) {
      realSetNaN(res);
      return;
    }

    realAdd(&lowS, &highS, &estimate, &ctxtReal51);
    realMultiply(&estimate, const_1on2, &estimate, &ctxtReal51);
    for(iterations=0; iterations<200; iterations++) {
      if(!studentTTailEquation(&estimate, &a, &logAOverA, &logTwoPOverA, &value, &derivative)) {
        realSetNaN(res);
        return;
      }
      if(realIsZero(&value)) {
        break;
      }
      if(realIsNegative(&value)) {
        realCopy(&estimate, &lowS);
      }
      else {
        realCopy(&estimate, &highS);
      }
      if(WP34S_RelativeError(&highS, &lowS, &tolerance, &ctxtReal51)) {
        realAdd(&lowS, &highS, &estimate, &ctxtReal51);
        realMultiply(&estimate, const_1on2, &estimate, &ctxtReal51);
        break;
      }
      if(iterations < 24) {
        realDivide(&value, &derivative, &step, &ctxtReal51);
        realSubtract(&estimate, &step, &midpoint, &ctxtReal51);
      }
      if(iterations >= 24 || realCompareLessEqual(&midpoint, &lowS) || realCompareGreaterEqual(&midpoint, &highS)) {
        realAdd(&lowS, &highS, &midpoint, &ctxtReal51);
        realMultiply(&midpoint, const_1on2, &midpoint, &ctxtReal51);
      }
      realCopy(&midpoint, &estimate);
    }

    WP34S_ExpM1(&estimate, &value, &ctxtReal51);
    realMultiply(nu, &value, &value, &ctxtReal51);
    realSquareRoot(&value, &value, &ctxtReal51);
    realPlus(&value, res, realContext);
  }


  void WP34S_Qf_T(const real_t *x, const real_t *nu, real_t *res, realContext_t *realContext) {
    real_t probability, logProbability, logNu, logBeta, logEstimate, estimate, normalEstimate, test, low, high, value, density, residual, step, correction,
           denominator, midpoint, tolerance, maximum, delta;
    bool_t neg = false, useBisection;
    int32_t iterations;
    realContext_t *workContext = &ctxtReal39;

    if(realIsNaN(x) || realCompareLessEqual(x, const_0) || realCompareGreaterEqual(x, const_1) || realIsNaN(nu) || realCompareLessEqual(nu, const_0)) {
      realSetNaN(res);
      return;
    }
    if(realCompareEqual(x, const_1on2)) {
      realSetZero(res);
      return;
    }
    if(realIsInfinite(nu)) {
      WP34S_Qf_Q(x, res, realContext);
      return;
    }
    WP34S_SetPowerOfTen(10000, &maximum);

    realSubtract(const_1, x, &probability, workContext);
    if(realCompareLessThan(x, &probability)) {
      neg = true;
      realCopy(x, &probability);
    }
    if(realCompareLessThan(nu, const_1)) {
      studentTSmallNuInverse(&probability, nu, res, &ctxtReal51);
      if(neg) {
        realChangeSign(res);
      }
      realPlus(res, res, realContext);
      return;
    }
    realSubtract(const_1on2, &probability, &delta, workContext);

    WP34S_qf_q_est(&probability, &normalEstimate, NULL, workContext);
    realSetPositiveSign(&normalEstimate);
    studentTLogBetaHalf(nu, &logBeta, workContext);
    WP34S_Ln(nu, &logNu, workContext);
    WP34S_Ln(&probability, &logProbability, workContext);
    realAdd(&logNu, &logBeta, &logEstimate, workContext);
    realAdd(&logEstimate, &logProbability, &logEstimate, workContext);
    realDivide(&logEstimate, nu, &logEstimate, workContext);
    realMultiply(&logNu, const_1on2, &estimate, workContext);
    realSubtract(&estimate, &logEstimate, &logEstimate, workContext);
    realMultiply(&logEstimate, const_2, &test, workContext);
    realAdd(&logNu, const75_ln10, &estimate, workContext);
    if(realCompareGreaterThan(&test, &estimate)) {
      realExp(&logEstimate, &estimate, workContext);
      if(realIsInfinite(&estimate)) {
        realCopy(&maximum, &estimate);
      }
    }
    else {
      realCopy(&normalEstimate, &estimate);
    }

    studentTInverseResidual(&estimate, nu, &logBeta, &probability, &delta, &residual, workContext);
    if(realIsNaN(&residual)) {
      realSetNaN(res);
      goto qf_t_exit;
    }
    if(realIsZero(&residual)) {
      realCopy(&estimate, res);
      goto qf_t_exit;
    }
    if(realIsPositive(&residual)) {
      realCopy(&estimate, &low);
      for(;;) {
        realMultiply(&estimate, const_2, &high, workContext);
        if(realCompareGreaterEqual(&high, &maximum)) {
          realCopy(&maximum, &high);
        }
        studentTInverseResidual(&high, nu, &logBeta, &probability, &delta, &residual, workContext);
        if(realIsNaN(&residual)) {
          realSetNaN(res);
          goto qf_t_exit;
        }
        if(!realIsPositive(&residual)) {
          break;
        }
        if(realCompareEqual(&high, &maximum)) {
          realCopy(&high, res);
          goto qf_t_exit;
        }
        realCopy(&high, &low);
        realCopy(&high, &estimate);
      }
    }
    else {
      realCopy(&estimate, &high);
      for(;;) {
        realMultiply(&estimate, const_1on2, &low, workContext);
        studentTInverseResidual(&low, nu, &logBeta, &probability, &delta, &residual, workContext);
        if(realIsNaN(&residual)) {
          realSetNaN(res);
          goto qf_t_exit;
        }
        if(!realIsNegative(&residual) || realIsZero(&low)) {
          break;
        }
        realCopy(&low, &high);
        realCopy(&low, &estimate);
      }
    }

    realCopy(&estimate, &value);
    WP34S_SetPowerOfTen(-37, &tolerance);
    for(iterations=0; iterations<160; iterations++) {
      studentTInverseResidual(&value, nu, &logBeta, &probability, &delta, &residual, workContext);
      if(realIsNaN(&residual)) {
        realSetNaN(res);
        goto qf_t_exit;
      }
      if(realIsZero(&residual)) {
        realCopy(&value, res);
        goto qf_t_exit;
      }
      if(realIsPositive(&residual)) {
        realCopy(&value, &low);
      }
      else {
        realCopy(&value, &high);
      }
      if(WP34S_RelativeError(&high, &low, &tolerance, workContext)) {
        realAdd(&low, &high, &midpoint, workContext);
        realMultiply(&midpoint, const_1on2, res, workContext);
        goto qf_t_exit;
      }

      WP34S_Pdf_T(&value, nu, &density, workContext);
      useBisection = iterations >= 24 || realIsZero(&density) || realIsSpecial(&density);
      if(!useBisection) {
        realDivide(&residual, &density, &step, workContext);
        realAdd(nu, const_1, &correction, workContext);
        realMultiply(&correction, &value, &correction, workContext);
        realMultiply(&correction, &step, &correction, workContext);
        realMultiply(&value, &value, &denominator, workContext);
        realAdd(&denominator, nu, &denominator, workContext);
        realMultiply(&denominator, const_2, &denominator, workContext);
        realDivide(&correction, &denominator, &correction, workContext);
        realSubtract(const_1, &correction, &correction, workContext);
        realDivide(&step, &correction, &step, workContext);
        realAdd(&value, &step, &estimate, workContext);
        useBisection = realIsSpecial(&estimate) || realCompareLessEqual(&estimate, &low) || realCompareGreaterEqual(&estimate, &high);
      }
      if(useBisection) {
        realAdd(&low, &high, &estimate, workContext);
        realMultiply(&estimate, const_1on2, &estimate, workContext);
      }
      realCopy(&estimate, &value);
    }

    realAdd(&low, &high, &midpoint, workContext);
    realMultiply(&midpoint, const_1on2, res, workContext);

    qf_t_exit:
    if(neg) {
      realChangeSign(res);
    }
  }

#endif // !OPTION_DIST_T
