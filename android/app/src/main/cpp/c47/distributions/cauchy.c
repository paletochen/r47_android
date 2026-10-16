// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file cauchy.c
 ***********************************************/

#include "c47.h"

#if !defined(OPTION_DIST_CAUCH)
  void fnCauchyP              (uint16_t unusedButMandatoryParameter){}
  void fnCauchyL              (uint16_t unusedButMandatoryParameter){}
  void fnCauchyR              (uint16_t unusedButMandatoryParameter){}
  void fnCauchyI              (uint16_t unusedButMandatoryParameter){}

#else
  bool_t cauchyCheckDomain(bool_t registersValid, real_t *x, real_t *i, real_t *j, real_t *unusedThird, uint16_t unusedParameter) {
    if(!registersValid) {
      return false;
    }

    if(realIsSpecial(i) || realIsSpecial(j) || realIsZero(j) || realIsNegative(j)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function cauchyCheckDomain:", "cannot calculate for " STD_gamma " " STD_LESS_EQUAL " 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    return true;
  }


  void fnCauchyP(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_CAUCHY * 4 + DIST_P);
  }


  void fnCauchyL(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_CAUCHY * 4 + DIST_L);
  }


  void fnCauchyR(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_CAUCHY * 4 + DIST_R);
  }


  void fnCauchyI(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_CAUCHY * 4 + DIST_I);
  }



  /********************************************//**
   * \brief Tests whether the location and scale define a Cauchy distribution.
   ***********************************************/
  static bool_t cauchyParametersAreValid(const real_t *x0, const real_t *gamma) {
    return !realIsSpecial(x0) && !realIsSpecial(gamma) && !realIsZero(gamma) && !realIsNegative(gamma);
  }

  /********************************************//**
   * \brief Computes sine and cosine of an angle no greater than pi/4 in magnitude.
   ***********************************************/
  static void cauchySinCosExtended(const real_t *angle, real_t *sine, real_t *cosine, realContext_t *realContext) {
    REAL_T_PTR(cosineTerm, realContext->digits);
    REAL_T_PTR(previous, realContext->digits);
    REAL_T_PTR(sineTerm, realContext->digits);
    REAL_T_PTR(square, realContext->digits);
    real_t denominator;
    bool_t cosineUnchanged, sineUnchanged;
    uint16_t index;

    realMultiply(angle, angle, square, realContext);
    realChangeSign(square);
    realCopy(angle, sineTerm);
    realCopy(angle, sine);
    realSetOne(cosineTerm);
    realSetOne(cosine);
    for(index = 1; index < 1000; index++) {
      int32ToReal((2 * index - 1) * (2 * index), &denominator);
      realMultiply(cosineTerm, square, cosineTerm, realContext);
      realDivide(cosineTerm, &denominator, cosineTerm, realContext);
      realCopy(cosine, previous);
      realAdd(cosine, cosineTerm, cosine, realContext);
      cosineUnchanged = realCompareEqual(cosine, previous);

      int32ToReal((2 * index) * (2 * index + 1), &denominator);
      realMultiply(sineTerm, square, sineTerm, realContext);
      realDivide(sineTerm, &denominator, sineTerm, realContext);
      realCopy(sine, previous);
      realAdd(sine, sineTerm, sine, realContext);
      sineUnchanged = realCompareEqual(sine, previous);

      if(sineUnchanged && cosineUnchanged) {
        break;
      }
    }
  }

  /********************************************//**
   * \brief Recomputes a Cauchy quantile after severe location-scale cancellation.
   ***********************************************/
  static void cauchyQuantileExtended(const real_t *probability, const real_t *x0, const real_t *gamma, real_t *res, realContext_t *resultContext) {
    REAL_T_PTR(angle, 159);
    REAL_T_PTR(cosine, 159);
    REAL_T_PTR(value, 159);
    real_t p;
    realContext_t realContext = ctxtReal75;

    realContext.digits = 159;
    realContext.status = 0;
    if(realCompareLessThan(probability, const_1on4) || realCompareGreaterThan(probability, const_3on4)) {
      if(realCompareLessThan(probability, const_1on2)) {
        realCopy(probability, &p);
      }
      else {
        realSubtract(const_1, probability, &p, &realContext);
      }
      realMultiply(&p, const1071_pi, angle, &realContext);
      cauchySinCosExtended(angle, angle, cosine, &realContext);
      realDivide(cosine, angle, angle, &realContext);
      if(realCompareLessThan(probability, const_1on2)) {
        realChangeSign(angle);
      }
    }
    else {
      realSubtract(probability, const_1on2, &p, &realContext);
      realMultiply(&p, const1071_pi, angle, &realContext);
      cauchySinCosExtended(angle, angle, cosine, &realContext);
      realDivide(angle, cosine, angle, &realContext);
    }
    realFMA(angle, gamma, x0, value, &realContext);
    realPlus(value, res, resultContext);
  }

  /********************************************//**
   * \brief Computes a Cauchy quantile and the two terms of its closing sum.
   ***********************************************/
  static void cauchyQuantileAtContext(const real_t *probability, const real_t *x0, const real_t *gamma, real_t *tangent, real_t *scaled, real_t *value, realContext_t *realContext) {
    real_t angle, cosine, p, sine;
    const real_t *pi = realContext->digits > 39 ? const75_pi : const39_pi;

    if(realCompareLessThan(probability, const_1on4) || realCompareGreaterThan(probability, const_3on4)) {
      if(realCompareLessThan(probability, const_1on2)) {
        realCopy(probability, &p);
      }
      else {
        realSubtract(const_1, probability, &p, realContext);
      }
      realMultiply(&p, pi, &angle, realContext);
      cauchySinCosExtended(&angle, &sine, &cosine, realContext);
      realDivide(&cosine, &sine, tangent, realContext);
      if(realCompareLessThan(probability, const_1on2)) {
        realChangeSign(tangent);
      }
    }
    else {
      realSubtract(probability, const_1on2, &p, realContext);
      realMultiply(&p, pi, &angle, realContext);
      cauchySinCosExtended(&angle, &sine, &cosine, realContext);
      realDivide(&sine, &cosine, tangent, realContext);
    }
    realMultiply(tangent, gamma, scaled, realContext);
    realFMA(tangent, gamma, x0, value, realContext);
  }


  /******************************************************
   * This functions are borrowed from the WP34S project
   ******************************************************/

  void WP34S_Pdf_Cauchy(const real_t *x, const real_t *x0, const real_t *gamma, real_t *res, realContext_t *realContext) {
    real_t difference, factor, ratio;
    const real_t *pi = realContext->digits > 39 ? const75_pi : const39_pi;

    if(realIsNaN(x) || !cauchyParametersAreValid(x0, gamma)) {
      realSetNaN(res);
      return;
    }
    if(realIsInfinite(x)) {
      realSetZero(res);
      return;
    }
    realSubtract(x, x0, &difference, realContext);
    realSetPositiveSign(&difference);
    if(realCompareLessEqual(&difference, gamma)) {
      realDivide(&difference, gamma, &ratio, realContext);
      realFMA(&ratio, &ratio, const_1, &factor, realContext);
      realMultiply(&factor, gamma, &factor, realContext);
      realMultiply(&factor, pi, &factor, realContext);
      realDivide(const_1, &factor, res, realContext);
    }
    else {
      realDivide(gamma, &difference, &ratio, realContext);
      realFMA(&ratio, &ratio, const_1, &factor, realContext);
      realMultiply(&factor, &difference, &factor, realContext);
      realMultiply(&factor, pi, &factor, realContext);
      realDivide(&ratio, &factor, res, realContext);
    }
  }

  void WP34S_Cdfu_Cauchy(const real_t *x, const real_t *x0, const real_t *gamma, real_t *res, realContext_t *realContext) {
    WP34S_cdf_cauchy_common(x, x0, gamma, true, res, realContext);
  }

  void WP34S_Cdf_Cauchy(const real_t *x, const real_t *x0, const real_t *gamma, real_t *res, realContext_t *realContext) {
    WP34S_cdf_cauchy_common(x, x0, gamma, false, res, realContext);
  }

  void WP34S_cdf_cauchy_common(const real_t *x, const real_t *x0, const real_t *gamma, bool_t complementary, real_t *res, realContext_t *realContext) {
    real_t argument, p;
    const real_t *pi = realContext->digits > 39 ? const75_pi : const39_pi;
    bool_t smallResult;

    if(realIsNaN(x) || !cauchyParametersAreValid(x0, gamma)) {
      realSetNaN(res);
      return;
    }
    if(realIsInfinite(x)) {
      realCopy(realIsNegative(x) != complementary ? const_0 : const_1, res);
      return;
    }
    WP34S_cdf_cauchy_xform(x, x0, gamma, &p, realContext);
    smallResult = realIsNegative(&p) != complementary;
    realSetPositiveSign(&p);
    if(realCompareGreaterThan(&p, const_1)) {
      realDivide(const_1, &p, &argument, realContext);
      C47_WP34S_Atan(&argument, &argument, realContext);
      realDivide(&argument, pi, &argument, realContext);
      if(smallResult) {
        realCopy(&argument, res);
      }
      else {
        realSubtract(const_1, &argument, res, realContext);
        if(realCompareEqual(res, const_1) && !realIsZero(&argument)) {
          realNextToward(res, const_0, res, realContext);
        }
      }
    }
    else {
      C47_WP34S_Atan(&p, &argument, realContext);
      realDivide(&argument, pi, &argument, realContext);
      if(smallResult) {
        realSubtract(const_1on2, &argument, res, realContext);
      }
      else {
        realAdd(const_1on2, &argument, res, realContext);
      }
      if(realCompareEqual(res, const_1on2) && !realIsZero(&argument)) {
        realNextToward(res, smallResult ? const_0 : const_1, res, realContext);
      }
    }
  }

  void WP34S_cdf_cauchy_xform(const real_t *x, const real_t *x0, const real_t *gamma, real_t *res, realContext_t *realContext) {
    realSubtract(x, x0, res, realContext);
    realDivide(res, gamma, res, realContext);
  }

  void WP34S_Qf_Cauchy(const real_t *x, const real_t *x0, const real_t *gamma, real_t *res, realContext_t *realContext) {
    real_t scaled, tangent, value;

    if(realIsNaN(x) || !cauchyParametersAreValid(x0, gamma) || realCompareLessThan(x, const_0) || realCompareGreaterThan(x, const_1)) {
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
      realCopy(x0, res);
      return;
    }
    if(realCompareEqual(x, const_1on4)) {
      realSubtract(x0, gamma, &value, &ctxtReal75);
      realPlus(&value, res, realContext);
      return;
    }
    if(realCompareEqual(x, const_3on4)) {
      realAdd(x0, gamma, &value, &ctxtReal75);
      realPlus(&value, res, realContext);
      return;
    }

    cauchyQuantileAtContext(x, x0, gamma, &tangent, &scaled, &value, &ctxtReal51);
    if(WP34S_HasCancellation(x0, &scaled, &value, 8)) {
      cauchyQuantileAtContext(x, x0, gamma, &tangent, &scaled, &value, &ctxtReal75);
      if(WP34S_HasCancellation(x0, &scaled, &value, 34)) {
        cauchyQuantileExtended(x, x0, gamma, res, realContext);
      }
      else {
        realPlus(&value, res, realContext);
      }
    }
    else {
      realPlus(&value, res, realContext);
    }
  }

#endif // !OPTION_DIST_CAUCH
