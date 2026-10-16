// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file normal.c
 ***********************************************/

#include "c47.h"

#if !defined(OPTION_DIST_NORML)
  void fnNormalP   (uint16_t unusedButMandatoryParameter){}
  void fnNormalL   (uint16_t unusedButMandatoryParameter){}
  void fnNormalR   (uint16_t unusedButMandatoryParameter){}
  void fnNormalI   (uint16_t unusedButMandatoryParameter){}
  void fnLogNormalP(uint16_t unusedButMandatoryParameter){}
  void fnLogNormalL(uint16_t unusedButMandatoryParameter){}
  void fnLogNormalR(uint16_t unusedButMandatoryParameter){}
  void fnLogNormalI(uint16_t unusedButMandatoryParameter){}
  void fnStdNormalP(uint16_t unusedButMandatoryParameter){}
  void fnStdNormalL(uint16_t unusedButMandatoryParameter){}
  void fnStdNormalR(uint16_t unusedButMandatoryParameter){}
  void fnStdNormalI(uint16_t unusedButMandatoryParameter){}
#else
  enum normalType {stdNormal, paramNormal, logNormal};

  static void normalCdfExtended(const real_t *x, real_t *res, realContext_t *realContext, bool_t upper);
  static void normalLogCdfExtended(const real_t *x, const real_t *mu, const real_t *sigma, real_t *res, bool_t upper);
  static void normalLogPdfExtended(const real_t *x, const real_t *mu, const real_t *sigma, real_t *res);
  static void normalPdfExtended(const real_t *x, real_t *res, realContext_t *realContext);
  static void normalQuantileResultExtended(const real_t *probability, const real_t *estimate, const real_t *mu, const real_t *sigma, real_t *res, bool_t logn);
  static void normalQuantileExtended(const real_t *probability, const real_t *estimate, real_t *res, realContext_t *realContext);

  static bool_t normalCheckDomain(bool_t registersValid, real_t *x, real_t *i, real_t *j, real_t *unusedThird, uint16_t type) {
    if(!registersValid) {
      return false;
    }

    if(type == stdNormal) {
      return true;
    }

    if(realIsSpecial(i) || realIsSpecial(j) || realIsZero(j) || realIsNegative(j)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function normalCheckDomain:", "parameters must be finite and " STD_sigma " > 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    return true;
  }

  static bool_t normalParameters(enum normalType type, real_t *x, real_t *i, real_t *j) {
    real_t *mu = type == stdNormal ? NULL : i;
    real_t *sigma = type == stdNormal ? NULL : j;

    return WP34S_CheckDistributionParameters(x, REGISTER_M, mu, REGISTER_S, sigma, 0, NULL, type, normalCheckDomain);
  }

  /********************************************//**
   * \brief Computes the log-normal argument and reports severe cancellation.
   ***********************************************/
  static bool_t normalLogTransform(const real_t *x, const real_t *mu, const real_t *sigma, real_t *standard, real_t *original) {
    real_t logarithm;

    realCopy(x, original);
    WP34S_Ln(original, &logarithm, &ctxtReal75);
    realSubtract(&logarithm, mu, standard, &ctxtReal75);
    if(WP34S_HasCancellation(&logarithm, mu, standard, 32)) {
      return true;
    }
    realDivide(standard, sigma, standard, &ctxtReal75);
    return false;
  }


  static void normalP(enum normalType type) {
    real_t val, alval, mu, sigma, ans;
    const int stdn = type == stdNormal;
    const int logn = type == logNormal;
    realContext_t *calculationContext = logn ? &ctxtReal75 : &ctxtReal51;
    bool_t extended = false;

    if(normalParameters(type, &val, &mu, &sigma)) {
      if(logn && realIsZero(&val)) {
        realSetZero(&ans);
      }
      else if(logn && realIsNegative(&val)) {
        if(getSystemFlag(FLAG_SPCRES)) {
          realCopy(const_NaN, &ans);
        }
        else {
          displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            moreInfoOnError("In function normalP:", "cannot calculate for x < 0", NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          return;
        }
      }
      else {
        if(!stdn) {
          if(logn) {
            extended = normalLogTransform(&val, &mu, &sigma, &val, &alval);
          }
          else {
            realSubtract(&val, &mu, &val, calculationContext);
            realDivide(&val, &sigma, &val, calculationContext);
          }
        }
        if(extended) {
          normalLogPdfExtended(&alval, &mu, &sigma, &ans);
        }
        else {
          WP34S_Pdf_Q(&val, &ans, calculationContext);
          if(!stdn) {
            realDivide(&ans, &sigma, &ans, calculationContext);
          }
          if(logn) {
            realDivide(&ans, &alval, &ans, calculationContext);
          }
          if(!realIsSpecial(&val) && realIsZero(&ans)) {
            realNextToward(&ans, const_1, &ans, calculationContext);
          }
        }
      }
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }


  static void normalL(enum normalType type) {
    real_t val, original, mu, sigma, ans;
    const int stdn = type == stdNormal;
    const int logn = type == logNormal;
    realContext_t *calculationContext = logn ? &ctxtReal75 : &ctxtReal51;
    bool_t extended = false;

    if(normalParameters(type, &val, &mu, &sigma)) {
      if(logn && realIsZero(&val)) {
        realSetZero(&ans);
      }
      else if(logn && realIsNegative(&val)) {
        if(getSystemFlag(FLAG_SPCRES)) {
          realCopy(const_NaN, &ans);
        }
        else {
          displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            moreInfoOnError("In function normalL:", "cannot calculate for x < 0", NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          return;
        }
      }
      else {
        if(!stdn) {
          if(logn) {
            extended = normalLogTransform(&val, &mu, &sigma, &val, &original);
          }
          else {
            realSubtract(&val, &mu, &val, calculationContext);
            realDivide(&val, &sigma, &val, calculationContext);
          }
        }
        if(extended) {
          normalLogCdfExtended(&original, &mu, &sigma, &ans, false);
        }
        else {
          WP34S_Cdf_Q(&val, &ans, calculationContext);
        }
      }
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }


  static void normalR(enum normalType type) {
    real_t val, original, mu, sigma, ans;
    const int stdn = type == stdNormal;
    const int logn = type == logNormal;
    realContext_t *calculationContext = logn ? &ctxtReal75 : &ctxtReal51;
    bool_t extended = false;

    if(normalParameters(type, &val, &mu, &sigma)) {
      if(logn && realIsZero(&val)) {
        realSetOne(&ans);
      }
      else if(logn && realIsNegative(&val)) {
        if(getSystemFlag(FLAG_SPCRES)) {
          realCopy(const_NaN, &ans);
        }
        else {
          displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            moreInfoOnError("In function normalR:", "cannot calculate for x < 0", NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          return;
        }
      }
      else {
        if(!stdn) {
          if(logn) {
            extended = normalLogTransform(&val, &mu, &sigma, &val, &original);
          }
          else {
            realSubtract(&val, &mu, &val, calculationContext);
            realDivide(&val, &sigma, &val, calculationContext);
          }
        }
        if(extended) {
          normalLogCdfExtended(&original, &mu, &sigma, &ans, true);
        }
        else {
          WP34S_Cdfu_Q(&val, &ans, calculationContext);
        }
      }
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }


  static void normalI(enum normalType type) {
    real_t val, mu, sigma, ans, quantile, scaled;
    const int stdn = type == stdNormal;
    const int logn = type == logNormal;
    realContext_t *calculationContext = stdn ? &ctxtReal39 : &ctxtReal75;
    bool_t extended = false;

    if(normalParameters(type, &val, &mu, &sigma)) {
      if(realIsNaN(&val) || realCompareLessEqual(&val, const_0) || realCompareGreaterEqual(&val, const_1)) {
        if(getSystemFlag(FLAG_SPCRES)) {
          convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
        }
        else {
          displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            moreInfoOnError("In function normalI:", "the argument must be 0 < x < 1", NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        }
        return;
      }
      WP34S_Qf_Q(&val, &quantile, calculationContext);
      if(!stdn) {
        realMultiply(&quantile, &sigma, &scaled, calculationContext);
        realFMA(&quantile, &sigma, &mu, &ans, calculationContext);
        if(WP34S_HasCancellation(&mu, &scaled, &ans, 32)) {
          normalQuantileResultExtended(&val, &quantile, &mu, &sigma, &ans, logn);
          extended = true;
        }
        if(logn && !extended) {
          realExp(&ans, &ans, calculationContext);
          if(!realIsSpecial(&mu) && !realIsSpecial(&sigma)) {
            if(realIsZero(&ans)) {
              realNextToward(&ans, const_1, &ans, calculationContext);
            }
            else if(realIsInfinite(&ans)) {
              realNextToward(&ans, const_0, &ans, calculationContext);
            }
          }
        }
      }
      else {
        realCopy(&quantile, &ans);
      }
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }



  void fnStdNormalP(uint16_t unusedButMandatoryParameter) {
    normalP(stdNormal);
  }

  void fnStdNormalL(uint16_t unusedButMandatoryParameter) {
    normalL(stdNormal);
  }

  void fnStdNormalR(uint16_t unusedButMandatoryParameter) {
    normalR(stdNormal);
  }

  void fnStdNormalI(uint16_t unusedButMandatoryParameter) {
    normalI(stdNormal);
  }



  void fnNormalP(uint16_t unusedButMandatoryParameter) {
    normalP(paramNormal);
  }

  void fnNormalL(uint16_t unusedButMandatoryParameter) {
    normalL(paramNormal);
  }

  void fnNormalR(uint16_t unusedButMandatoryParameter) {
    normalR(paramNormal);
  }

  void fnNormalI(uint16_t unusedButMandatoryParameter) {
    normalI(paramNormal);
  }



  void fnLogNormalP(uint16_t unusedButMandatoryParameter) {
    normalP(logNormal);
  }

  void fnLogNormalL(uint16_t unusedButMandatoryParameter) {
    normalL(logNormal);
  }

  void fnLogNormalR(uint16_t unusedButMandatoryParameter) {
    normalR(logNormal);
  }

  void fnLogNormalI(uint16_t unusedButMandatoryParameter) {
    normalI(logNormal);
  }

  /********************************************//**
   * \brief Computes a central normal probability from its alternating power series.
   ***********************************************/
  static void normalCentralCdf(const real_t *x, const real_t *offset, real_t *res, realContext_t *realContext, bool_t upper) {
    REAL_T_PTR(square, realContext->digits);
    REAL_T_PTR(term, realContext->digits);
    REAL_T_PTR(sum, realContext->digits);
    REAL_T_PTR(previous, realContext->digits);
    REAL_T_PTR(numerator, realContext->digits);
    REAL_T_PTR(denominator, realContext->digits);
    REAL_T_PTR(rootTwoPi, realContext->digits);
    REAL_T_PTR(delta, realContext->digits);
    const real_t *twoPi = realContext->digits > 75 ? const2139_2pi : realContext->digits > 39 ? const75_2pi : const39_2pi;

    realMultiply(x, x, square, realContext);
    realMultiply(square, const_1on2, square, realContext);
    realChangeSign(square);
    realSetOne(term);
    realSetOne(sum);
    for(int32_t n = 1; n < 1000; n++) {
      realMultiply(term, square, term, realContext);
      int32ToReal(2 * n - 1, numerator);
      realMultiply(term, numerator, term, realContext);
      int32ToReal(n * (2 * n + 1), denominator);
      realDivide(term, denominator, term, realContext);
      realCopy(sum, previous);
      realAdd(sum, term, sum, realContext);
      if(realCompareEqual(sum, previous)) {
        break;
      }
    }
    realMultiply(x, sum, delta, realContext);
    realSquareRoot(twoPi, rootTwoPi, realContext);
    realDivide(delta, rootTwoPi, delta, realContext);
    if(upper) {
      realSubtract(offset, delta, res, realContext);
    }
    else {
      realAdd(offset, delta, res, realContext);
    }
  }

  /********************************************//**
   * \brief Computes a normal density in an extended context.
   ***********************************************/
  static void normalPdfExtended(const real_t *x, real_t *res, realContext_t *realContext) {
    REAL_T_PTR(rootTwoPi, realContext->digits);

    realMultiply(x, x, res, realContext);
    realMultiply(res, const_1on2, res, realContext);
    realChangeSign(res);
    realExp(res, res, realContext);
    realSquareRoot(const2139_2pi, rootTwoPi, realContext);
    realDivide(res, rootTwoPi, res, realContext);
    if(!realIsSpecial(x) && realIsZero(res)) {
      realNextToward(res, const_1, res, realContext);
    }
  }

  /********************************************//**
   * \brief Computes the upper normal tail by a modified Lentz fraction.
   ***********************************************/
  static void normalUpperTailExtended(const real_t *x, real_t *res, realContext_t *realContext) {
    REAL_T_PTR(an, realContext->digits);
    REAL_T_PTR(b, realContext->digits);
    REAL_T_PTR(c, realContext->digits);
    REAL_T_PTR(d, realContext->digits);
    REAL_T_PTR(h, realContext->digits);
    REAL_T_PTR(index, realContext->digits);
    REAL_T_PTR(pdf, realContext->digits);
    REAL_T_PTR(square, realContext->digits);
    REAL_T_PTR(t, realContext->digits);
    REAL_T_PTR(u, realContext->digits);
    REAL_T_PTR(v, realContext->digits);

    realMultiply(x, x, square, realContext);
    realMultiply(square, const_1on2, square, realContext);
    realAdd(square, const_1on2, b, realContext);
    realSetPlusInfinity(c);
    realDivide(const_1, b, d, realContext);
    realCopy(d, h);
    realSetZero(index);
    for(int32_t n = 0; n < 1000; n++) {
      realAdd(index, const_1, index, realContext);
      realSubtract(const_1on2, index, t, realContext);
      realMultiply(index, t, an, realContext);
      realAdd(b, const_2, b, realContext);
      realFMA(an, d, b, v, realContext);
      realDivide(const_1, v, d, realContext);
      realDivide(an, c, t, realContext);
      realAdd(b, t, c, realContext);
      realMultiply(d, c, t, realContext);
      realMultiply(h, t, u, realContext);
      if(realCompareEqual(u, h)) {
        break;
      }
      realCopy(u, h);
    }
    normalPdfExtended(x, pdf, realContext);
    realMultiply(pdf, x, res, realContext);
    realMultiply(res, h, res, realContext);
    realMultiply(res, const_1on2, res, realContext);
  }

  /********************************************//**
   * \brief Computes a normal probability in an extended context.
   ***********************************************/
  static void normalCdfExtended(const real_t *x, real_t *res, realContext_t *realContext, bool_t upper) {
    REAL_T_PTR(absX, realContext->digits);
    REAL_T_PTR(tail, realContext->digits);
    bool_t negative = realIsNegative(x);

    realCopyAbs(x, absX);
    if(realCompareLessThan(absX, const_8)) {
      normalCentralCdf(x, const_1on2, res, realContext, upper);
    }
    else {
      normalUpperTailExtended(absX, tail, realContext);
      if((upper && !negative) || (!upper && negative)) {
        realCopy(tail, res);
      }
      else {
        realSubtract(const_1, tail, res, realContext);
      }
    }
    if(realIsZero(res)) {
      realNextToward(res, const_1, res, realContext);
    }
    else if(realCompareEqual(res, const_1)) {
      realNextToward(res, const_0, res, realContext);
    }
  }

  /********************************************//**
   * \brief Computes a log-normal density after severe logarithmic cancellation.
   ***********************************************/
  static void normalLogPdfExtended(const real_t *x, const real_t *mu, const real_t *sigma, real_t *res) {
    REAL_T_PTR(extendedResult, 159);
    REAL_T_PTR(standard, 159);
    realContext_t realContext = ctxtReal75;

    realContext.digits = 159;
    realContext.status = 0;
    WP34S_LnExtended(x, standard, &realContext);
    realSubtract(standard, mu, standard, &realContext);
    realDivide(standard, sigma, standard, &realContext);
    normalPdfExtended(standard, extendedResult, &realContext);
    realDivide(extendedResult, sigma, extendedResult, &realContext);
    realDivide(extendedResult, x, extendedResult, &realContext);
    if(realIsZero(extendedResult)) {
      realNextToward(extendedResult, const_1, extendedResult, &realContext);
    }
    // Keep the source and destination at the same capacity until the value has
    // been shortened. roundToSignificantDigits copies before rounding when its
    // pointers differ, so writing a p159 source directly to a real_t would
    // overrun the latter's p75 storage.
    roundToSignificantDigits(extendedResult, extendedResult, 34, &realContext);
    realCopy(extendedResult, res);
  }

  /********************************************//**
   * \brief Computes a log-normal probability after severe logarithmic cancellation.
   ***********************************************/
  static void normalLogCdfExtended(const real_t *x, const real_t *mu, const real_t *sigma, real_t *res, bool_t upper) {
    REAL_T_PTR(extendedResult, 159);
    REAL_T_PTR(standard, 159);
    realContext_t realContext = ctxtReal75;

    realContext.digits = 159;
    realContext.status = 0;
    WP34S_LnExtended(x, standard, &realContext);
    realSubtract(standard, mu, standard, &realContext);
    realDivide(standard, sigma, standard, &realContext);
    normalCdfExtended(standard, extendedResult, &realContext, upper);
    roundToSignificantDigits(extendedResult, extendedResult, 34, &realContext);
    realCopy(extendedResult, res);
  }

  /********************************************//**
   * \brief Computes a location-scale quantile after severe final cancellation.
   ***********************************************/
  static void normalQuantileResultExtended(const real_t *probability, const real_t *estimate, const real_t *mu, const real_t *sigma, real_t *res, bool_t logn) {
    REAL_T_PTR(extendedResult, 159);
    realContext_t realContext = ctxtReal75;

    realContext.digits = 159;
    realContext.status = 0;
    normalQuantileExtended(probability, estimate, extendedResult, &realContext);
    realFMA(extendedResult, sigma, mu, extendedResult, &realContext);
    if(logn) {
      realExp(extendedResult, extendedResult, &realContext);
      if(realIsZero(extendedResult)) {
        realNextToward(extendedResult, const_1, extendedResult, &realContext);
      }
      else if(realIsInfinite(extendedResult)) {
        realNextToward(extendedResult, const_0, extendedResult, &realContext);
      }
    }
    roundToSignificantDigits(extendedResult, extendedResult, 34, &realContext);
    realCopy(extendedResult, res);
  }

  /********************************************//**
   * \brief Refines a normal quantile after severe location-scale cancellation.
   ***********************************************/
  static void normalQuantileExtended(const real_t *probability, const real_t *estimate, real_t *res, realContext_t *realContext) {
    REAL_T_PTR(p, realContext->digits);
    REAL_T_PTR(q, realContext->digits);
    REAL_T_PTR(r, realContext->digits);
    REAL_T_PTR(s, realContext->digits);
    REAL_T_PTR(target, realContext->digits);
    bool_t lower = realCompareLessEqual(probability, const_1on2);

    realCopyAbs(estimate, p);
    if(lower) {
      realCopy(probability, target);
    }
    else {
      realSubtract(const_1, probability, target, realContext);
    }
    for(int32_t n = 0; n < 2; n++) {
      if(realCompareLessThan(p, const_8)) {
        normalCentralCdf(p, const_1on2, q, realContext, true);
      }
      else {
        normalUpperTailExtended(p, q, realContext);
      }
      realSubtract(q, target, q, realContext);
      realMultiply(p, p, r, realContext);
      realMultiply(r, const_1on2, r, realContext);
      realChangeSign(r);
      realExp(r, r, realContext);
      realSquareRoot(const2139_2pi, s, realContext);
      realDivide(r, s, r, realContext);
      realDivide(q, r, q, realContext);
      realMultiply(q, q, r, realContext);
      realMultiply(r, q, r, realContext);
      realMultiply(p, p, s, realContext);
      realMultiply(s, const_2, s, realContext);
      realAdd(s, const_1, s, realContext);
      realMultiply(r, s, r, realContext);
      realDivide(r, const_6, r, realContext);
      realMultiply(p, const_1on2, s, realContext);
      realMultiply(s, q, s, realContext);
      realMultiply(s, q, s, realContext);
      realAdd(s, r, r, realContext);
      realAdd(r, q, q, realContext);
      realAdd(q, p, p, realContext);
    }
    if(lower) {
      realChangeSign(p);
    }
    realCopy(p, res);
  }

  /******************************************************
   * This functions are borrowed from the WP34S project
   ******************************************************/

  static void cdf_q(const real_t *x, real_t *res, realContext_t *realContext, bool_t upper) {
    real_t p;

    realSquareRoot(const_2, &p, realContext);
    realDivide(x, &p, res, realContext);
    if(!upper) {
      realChangeSign(res);
    }
    WP34S_Erfc(res, res, realContext);
    realMultiply(res, const_1on2, res, realContext);
    if(!realIsSpecial(x)) {
      if(realIsZero(res)) {
        realNextToward(res, const_1, res, realContext);
      }
      else if(realCompareEqual(res, const_1)) {
        realNextToward(res, const_0, res, realContext);
      }
    }
  }

  void WP34S_Pdf_Q(const real_t *x, real_t *res, realContext_t *realContext) {
    real_t p;
    const real_t *twoPi = realContext->digits > 39 ? const75_2pi : const39_2pi;
  realMultiply(x, x, res, realContext);
  realMultiply(res, const_1on2, res, realContext);
    realChangeSign(res);
   realExp(res, res, realContext);
   realSquareRoot(twoPi, &p, realContext);
   realDivide(res, &p, res, realContext);
   if(!realIsSpecial(x) && realIsZero(res)) {
     realNextToward(res, const_1, res, realContext);
   }
 }

  void WP34S_Cdfu_Q(const real_t *x, real_t *res, realContext_t *realContext) {
    cdf_q(x, res, realContext, true);
  }

  void WP34S_Cdf_Q(const real_t *x, real_t *res, realContext_t *realContext) {
    cdf_q(x, res, realContext, false);
  }

  /* This routine that returns a signed guess for the Normal quantile.
   * GNQ takes any 0 < p < 1 and returns a positive or negative estimate
   * for the Normal quantile.
   */
  void WP34S_qf_q_est(const real_t *x, real_t *res, real_t* resY, realContext_t *realContext) {
    real_t p, q, r, s;
    bool_t isSmall = false;

    // qf_q_int_est
    realMinus(x, &p, realContext);
    realAdd(&p, const_1, &p, realContext);
    if(realCompareLessThan(x, &p)) {
      isSmall = true;
      realCopy(x, &p);
    }

  realMultiply(const_2, const_1on10, &q, realContext);

    if(realCompareLessThan(&p, &q)) {
      WP34S_Ln(&p, &q, realContext);
      realMultiply(&q, const_2, &q, realContext);
      realChangeSign(&q);
      realSubtract(&q, const_1, &r, realContext);
      realMultiply(&r, const39_2pi, &r, realContext);
      realSquareRoot(&r, &r, realContext);
      realMultiply(&r, &p, &r, realContext);
      WP34S_Ln(&r, &r, realContext);
      realMultiply(&r, const_2, &r, realContext);
      realChangeSign(&r);
      realSquareRoot(&r, &r, realContext);
      int32ToReal(264, &s);
      s.exponent -= 3; // s = s / 1000
      realDivide(&s, &q, &q, realContext);
      realAdd(&q, &r, &q, realContext);
    }
    else { // qf_q_mid
      realMinus(&p, &q, realContext);
      realAdd(&q, const_1on2, &q, realContext);
      realSquareRoot(const39_2pi, &r, realContext);
      realMultiply(&q, &r, &q, realContext);
      realMultiply(&q, &q, &r, realContext);
      realMultiply(&r, &q, &r, realContext);
      realDivide(&r, const_5, &r, realContext);
      realAdd(&q, &r, &q, realContext);
    }

    realCopy(&q, res);
    if(resY) {
      realCopy(&p, resY);
    }
    // qf_q_signfix
    if(isSmall) {
      realChangeSign(res);
    }
  }

  void WP34S_Qf_Q(const real_t *x, real_t *res, realContext_t *realContext) {
    real_t p, q, r, s, reg0;
    bool_t half = false;
    int32_t loops;
    WP34S_qf_q_est(x, &p, &reg0, realContext);
    if(realIsNegative(&p)) {
      half = true;
    }
    loops = realContext->digits > 39 ? 3 : 2;
    realSetPositiveSign(&p);
    realAdd(&p, const_1, &q, realContext);
    roundToSignificantDigits(&q, &q, 3, realContext);
    if(realCompareEqual(&q, const_1)) {
      --loops;
      realCopy(&p, &q);
      q.exponent += 16; // q = q * 1e16
      if(realCompareGreaterThan(&q, const_1)) {
        goto qf_q_calc;
      }
    }
    else {
      qf_q_calc:
      do { // qf_q_refine
        if(realCompareGreaterEqual(&p, const_1)) {
          WP34S_Cdfu_Q(&p, &q, realContext);
          realSubtract(&q, &reg0, &q, realContext);
        }
        else { // qf_q_small
          realSubtract(const_1on2, &reg0, &q, realContext);  // 1/2 - reg0 is exact, so the residual (1/2 - reg0) - delta is rounded once
          normalCentralCdf(&p, &q, &q, realContext, true);
        }
        // qf_q_common
        WP34S_Pdf_Q(&p, &r, realContext);
        realDivide(&q, &r, &q, realContext);
        realMultiply(&q, &q, &r, realContext);
        realMultiply(&r, &q, &r, realContext);
        realMultiply(&p, &p, &s, realContext);
        realMultiply(&s, const_2, &s, realContext);
        realAdd(&s, const_1, &s, realContext);
        realMultiply(&r, &s, &r, realContext);
        realDivide(&r, const_6, &r, realContext);
        realMultiply(&p, const_1on2, &s, realContext);
        realMultiply(&s, &q, &s, realContext);
        realMultiply(&s, &q, &s, realContext);
        realAdd(&s, &r, &r, realContext);
        realAdd(&r, &q, &q, realContext);
        realAdd(&q, &p, &p, realContext);
        // SHOW_CONVERGENCE
      } while(--loops > 0);
    }
    // qf_q_out
    if(half) {
      realChangeSign(&p);
    }
    realCopy(&p, res);
  }

#endif // !OPTION_DIST_NORML
