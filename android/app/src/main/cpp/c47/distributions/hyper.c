// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file hyper.c
 ***********************************************/

#include "c47.h"

#if defined(SAVE_SPACE_DM42_17)
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
  static bool_t checkParamHyper(real_t *x, real_t *k, real_t *j, real_t *i) {
    real_t xmin, xmax;

    if(!saveLastX())
      return false;

    if(!getRegisterAsReal(REGISTER_X, x)
        || !getRegisterAsReal(REGISTER_M, i)
        || !getRegisterAsReal(REGISTER_N, j)
        || !getRegisterAsReal(REGISTER_Q, k)) {
      displayDomainErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Values in register X, I, J and K must be of the real or long integer type");
        moreInfoOnError("In function checkParamHyper:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }

    //realMultiply(i, k, &ik, &ctxtReal39), realToIntegralValue(&ik, &ik, DEC_ROUND_HALF_EVEN, &ctxtReal39);
    realAdd(j, k, &xmin, &ctxtReal39);
    realSubtract(&xmin, i, &xmin, &ctxtReal39);
    if(realIsNegative(&xmin)) {
      realCopy(const_0, &xmin);
    }
    realCopy(realCompareLessThan(k, j) ? k : j, &xmax);

    if((!checkRegisterNoFP(i)) || (!checkRegisterNoFP(j)) || (!checkRegisterNoFP(k))) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function checkParamHyper:", "N, n and/or K is not an integer", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    if(realIsNegative(x) || realCompareLessThan(x, &xmin) || realCompareGreaterThan(x, &xmax)) {
      displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function checkParamHyper:", "cannot calculate for x < max(0, n + K - N) or x > min(n, K)", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    if(realIsNegative(k) || realCompareGreaterThan(k, i) || realIsNegative(j) || realCompareGreaterThan(j, i) || realIsNegative(i)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function checkParamHyper:", "the parameters must be integer, and 0 " STD_LESS_EQUAL " K " STD_LESS_EQUAL " N, 0 " STD_LESS_EQUAL " n " STD_LESS_EQUAL " N, and N " STD_GREATER_EQUAL " 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    return true;
  }


  void fnHypergeometricP(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, spec, samp, batch;

    if(checkParamHyper(&val, &spec, &samp, &batch)) {
      if(realIsAnInteger(&val)) {
        pdf_Hypergeometric(&val, &spec, &samp, &batch, &ans, &ctxtReal39);
      }
      else {
        realZero(&ans);
      }
      if(realIsNaN(&ans)) {
        displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE, REGISTER_X);
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

    if(checkParamHyper(&val, &spec, &samp, &batch)) {
      cdf_Hypergeometric(&val, &spec, &samp, &batch, &ans, &ctxtReal75);
      if(realIsNaN(&ans)) {
        displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE, REGISTER_X);
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

    if(checkParamHyper(&val, &spec, &samp, &batch)) {
      cdfu_Hypergeometric(&val, &spec, &samp, &batch, &ans, &ctxtReal75);
      if(realIsNaN(&ans)) {
        displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE, REGISTER_X);
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

    if(checkParamHyper(&val, &spec, &samp, &batch)) {
      if(realCompareLessThan(&val, const_0) || realCompareGreaterThan(&val, const_1)) {
        displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnHypergeometricI:", "the argument must be 0 " STD_LESS_EQUAL " x " STD_LESS_EQUAL " 1", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        if(getSystemFlag(FLAG_SPCRES)) {
          convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
        }
        return;
      }
      qf_Hypergeometric(&val, &spec, &samp, &batch, &ans, &ctxtReal75);
      if(realIsNaN(&ans)) {
        displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
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


  // PDF(k; n, K, N) = (K C k) ((N-K) C (n-k)) / (N C n)
  void pdf_Hypergeometric(const real_t *x, const real_t *k0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext) {
    real_t a, b, c, q;

    realCopy(k0, &a), realCopy(x, &b), logCyxReal(&a, &b, &q, realContext);     // K C k
    realSubtract(n0, k0, &a, realContext), realSubtract(n, x, &b, realContext);
    logCyxReal(&a, &b, &c, realContext), realAdd(&q, &c, &q, realContext);       // (N-K) C (n-k)
    realCopy(n0, &a), realCopy(n, &b);
    logCyxReal(&a, &b, &c, realContext), realSubtract(&q, &c, &q, realContext);  // N C n
    realExp(&q, res, realContext);
  }

  static void cdf_Hypergeometric_common(const real_t *x, const real_t *k0, const real_t *n, const real_t *n0, bool_t complementary, real_t *res, realContext_t *realContext) {
    real_t a, b, c, binomPart;
    real_t a1, a2, a3, b1, b2, i, hypergeomPart, cvgTol;
    bool_t signHgp = false;

    realCopy(const_1, &cvgTol), cvgTol.exponent -= realContext->digits - 2;

    // (n C (k+1)) ((N-n) C (K-k-1)) / (N C K)
    realCopy(n, &a), realAdd(x, const_1, &b, realContext);
    logCyxReal(&a, &b, &binomPart, realContext);                                                // n C (k+1)
    realSubtract(n0, n, &a, realContext), realSubtract(k0, x, &b, realContext), realSubtract(&b, const_1, &b, realContext);
    logCyxReal(&a, &b, &c, realContext), realAdd(&binomPart, &c, &binomPart, realContext);      // (N-n) C (K-k-1)
    realCopy(n0, &a), realCopy(k0, &b);
    logCyxReal(&a, &b, &c, realContext), realSubtract(&binomPart, &c, &binomPart, realContext); // N C K

    // generalized hypergeometric function 3F2
    realCopy(const_1, &a1);
    realAdd(x, const_1, &a2, realContext);
    realAdd(&a2, const_1, &b1, realContext);
    realSubtract(&a2, n, &a3, realContext);
    realSubtract(&a2, k0, &a2, realContext);
    realAdd(&b1, n0, &b2, realContext);
    realSubtract(&b2, k0, &b2, realContext);
    realSubtract(&b2, n, &b2, realContext);
    realCopy(const_0, &hypergeomPart);
    realCopy(const_1, &i);

    if(complementary) {
      realExp(&binomPart, &b, realContext);
    }
    else {
      WP34S_ExpM1(&binomPart, &b, realContext);
      realSetPositiveSign(&b);
    }

    do {
      realCopy(&b, &c);

      if(realIsZero(&a2) || realIsZero(&a3) || realIsZero(&b1) || realIsZero(&b2)) {
        break;
      }

      signHgp = (realIsNegative(&a1) ? 1 : 0) ^ (realIsNegative(&a2) ? 1 : 0) ^ (realIsNegative(&a3) ? 1 : 0) ^ (realIsNegative(&b1) ? 1 : 0) ^ (realIsNegative(&b2) ? 1 : 0);
      realCopyAbs(&a1, &a), WP34S_Ln(&a, &a, realContext), realAdd(&hypergeomPart, &a, &hypergeomPart, realContext);
      realCopyAbs(&a2, &a), WP34S_Ln(&a, &a, realContext), realAdd(&hypergeomPart, &a, &hypergeomPart, realContext);
      realCopyAbs(&a3, &a), WP34S_Ln(&a, &a, realContext), realAdd(&hypergeomPart, &a, &hypergeomPart, realContext);
      realCopyAbs(&b1, &a), WP34S_Ln(&a, &a, realContext), realSubtract(&hypergeomPart, &a, &hypergeomPart, realContext);
      realCopyAbs(&b2, &a), WP34S_Ln(&a, &a, realContext), realSubtract(&hypergeomPart, &a, &hypergeomPart, realContext);
      WP34S_Ln(&i, &a, realContext), realSubtract(&hypergeomPart, &a, &hypergeomPart, realContext);

      realAdd(&binomPart, &hypergeomPart, &a, realContext);
      realExp(&a, &a, realContext);
      if(signHgp) {
        realSetNegativeSign(&a);
      }
      if(complementary) {
        realAdd(&b, &a, &b, realContext);
      }
      else {
        realSubtract(&b, &a, &b, realContext);
      }

      realAdd(&a1, const_1, &a1, realContext);
      realAdd(&a2, const_1, &a2, realContext);
      realAdd(&a3, const_1, &a3, realContext);
      realAdd(&b1, const_1, &b1, realContext);
      realAdd(&b2, const_1, &b2, realContext);
      realAdd(&i, const_1, &i, realContext);
    } while((!WP34S_RelativeError(&b, &c, &cvgTol, realContext)) && (!realIsSpecial(&b)));

    realCopy(&b, res);
    realSetPositiveSign(res);
  }

  void cdfu_Hypergeometric(const real_t *x, const real_t *k0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext) {
    real_t p;

    realToIntegralValue(x, &p, DEC_ROUND_CEILING, realContext);
    if(realCompareLessThan(&p, const_1)) {
      realCopy(const_1, res);
      return;
    }
    realSubtract(&p, const_1, &p, realContext);

    cdf_Hypergeometric_common(&p, k0, n, n0, true, res, realContext);
  }

  void cdf_Hypergeometric(const real_t *x, const real_t *k0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext) {
    real_t p;

    realToIntegralValue(x, &p, DEC_ROUND_FLOOR, realContext);
    cdf_Hypergeometric2(&p, k0, n, n0, res, realContext);
  }

  static void mode_Hypergeometric(const real_t *k0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext) {
    real_t a, q;

    realAdd(n, const_1, &a, realContext);
    WP34S_Ln(&a, &q, realContext);
    realAdd(k0, const_1, &a, realContext);
    WP34S_Ln(&a, &a, realContext), realAdd(&q, &a, &q, realContext);
    realAdd(n0, const_2, &a, realContext);
    WP34S_Ln(&a, &a, realContext), realSubtract(&q, &a, &q, realContext);
    realExp(&q, &q, realContext);
    realToIntegralValue(&q, res, DEC_ROUND_FLOOR, realContext);
  }

  void cdf_Hypergeometric2(const real_t *x, const real_t *k0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext) {
    real_t mode, pdf, i, cdf, cdf0;

    if(realCompareLessThan(x, const_0)) {
      realZero(res);
      return;
    }

    mode_Hypergeometric(k0, n, n0, &mode, realContext);

    if(realCompareLessThan(x, &mode)) {
      realCopy(x, &i);
      realCopy(const_0, &cdf);
      do {
        realCopy(&cdf, &cdf0);
        pdf_Hypergeometric(&i, k0, n, n0, &pdf, realContext);
        realAdd(&cdf, &pdf, &cdf, realContext);
        realSubtract(&i, const_1, &i, realContext);
      } while((!WP34S_RelativeError(&cdf, &cdf0, const_1e_37, realContext)) && (!realIsNegative(&i)));
      realCopy(&cdf, res);
    }
    else {
      cdf_Hypergeometric_common(x, k0, n, n0, false, res, realContext);
    }
  }

  void qf_Hypergeometric(const real_t *x, const real_t *k0, const real_t *n, const real_t *n0, real_t *res, realContext_t *realContext) {
    real_t mean, var, s;

    if(realCompareLessEqual(x, const_0)) {
      realZero(res);
      return;
    }

    WP34S_Ln(n,   &var, realContext);
    WP34S_Ln(k0,  &s,   realContext), realAdd     (&var, &s, &var, realContext);
    WP34S_Ln(n0,  &s,   realContext), realSubtract(&var, &s, &var, realContext);
    realExp(&var, &mean, realContext);      // mean = nK/N

    realSubtract(n0, k0,      &s, realContext), WP34S_Ln(&s, &s, realContext), realAdd     (&var, &s, &var, realContext);
                                                WP34S_Ln(n0, &s, realContext), realSubtract(&var, &s, &var, realContext);
    realSubtract(n0, n,       &s, realContext), WP34S_Ln(&s, &s, realContext), realAdd     (&var, &s, &var, realContext);
    realSubtract(n0, const_1, &s, realContext), WP34S_Ln(&s, &s, realContext), realSubtract(&var, &s, &var, realContext);
    realExp(&var, &var, realContext);       // variance = (nK/N) ((N-K)/N) ((N-n)/(N-1))
    realSquareRoot(&var, &var, realContext);

    WP34S_normal_moment_approx(x, &var, &mean, &s, realContext);
    WP34S_Qf_Newton(QF_NEWTON_HYPERGEOMETRIC, x, &s, k0, n, n0, res, realContext);
  }

#endif //SAVE_SPACE_DM42_17
