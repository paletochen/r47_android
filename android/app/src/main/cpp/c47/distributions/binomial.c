// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file binomial.c
 ***********************************************/

#include "c47.h"

#if defined(SAVE_SPACE_DM42_17)
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
  static bool_t checkParamBinomial(real_t *x, real_t *i, real_t *j) {
    if(!saveLastX())
      return false;

    if( !getRegisterAsReal(REGISTER_X, x)
        || !getRegisterAsReal(REGISTER_P, i)
        || !getRegisterAsReal(REGISTER_N, j))
      goto err;

    if(!checkRegisterNoFP(j)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function checkParamBinomial:", "n is not an integer", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      goto err;
    }
    else if(realIsNegative(x)) {
      displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function checkParamBinomial:", "cannot calculate for x < 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      goto err;
    }
    else if(realIsNegative(i) || realCompareGreaterThan(i, const_1) || realIsNegative(j)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function checkParamBinomial:", "the parameters must be 0 " STD_LESS_EQUAL " p " STD_LESS_EQUAL " 1 and n > 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      goto err;
    }
    return true;

  err:
    if(getSystemFlag(FLAG_SPCRES)) {
      convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
    }
    return false;
  }


  void fnBinomialP(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, prob, num;

    if(checkParamBinomial(&val, &prob, &num)) {
      if(realIsAnInteger(&val)) {
        WP34S_Pdf_Binomial(&val, &prob, &num, &ans, &ctxtReal39);
      }
      else {
        realZero(&ans);
      }
      if(realIsNaN(&ans)) {
        displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnBinomialP:", "a parameter is invalid", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      else {
        convertRealToResultRegister(&ans, REGISTER_X, amNone);
      }
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }


  void fnBinomialL(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, prob, num;

    if(checkParamBinomial(&val, &prob, &num)) {
      WP34S_Cdf_Binomial(&val, &prob, &num, &ans, &ctxtReal39);
      if(realIsNaN(&ans)) {
        displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnBinomialL:", "a parameter is invalid", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      else {
        convertRealToResultRegister(&ans, REGISTER_X, amNone);
      }
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }


  void fnBinomialR(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, prob, num;

    if(checkParamBinomial(&val, &prob, &num)) {
      WP34S_Cdfu_Binomial(&val, &prob, &num, &ans, &ctxtReal39);
      if(realIsNaN(&ans)) {
        displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnBinomialR:", "a parameter is invalid", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      else {
        convertRealToResultRegister(&ans, REGISTER_X, amNone);
      }
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }


  void fnBinomialI(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, prob, num;

    if(checkParamBinomial(&val, &prob, &num)) {
      if(realCompareLessEqual(&val, const_0) || realCompareGreaterEqual(&val, const_1)) {
        displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnBinomialI:", "the argument must be 0 < x < 1", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        if(getSystemFlag(FLAG_SPCRES)) {
          convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
        }
        return;
      }
      WP34S_Qf_Binomial(&val, &prob, &num, &ans, &ctxtReal39);
      if(realIsNaN(&ans)) {
        displayDomainErrorMessage(ERROR_NO_ROOT_FOUND, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnBinomialI:", "WP34S_Qf_Binomial did not converge", NULL, NULL);
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



  /******************************************************
   * This functions are borrowed from the WP34S project
   ******************************************************/

  bool_t binomial_param(const real_t *n, real_t *res) {
    if(realIsSpecial(n)) {
      realCopy(const_NaN, res);
      return false;
    }
    if(realIsNegative(n) || (!realIsAnInteger(n))) {
      realCopy(const_0, res);
      return false;
    }
    return true;
  }

  void WP34S_Pdf_Binomial(const real_t *x, const real_t *p0, const real_t *n, real_t *res, realContext_t *realContext) {
    real_t p, q, nn, xx;

    if(!binomial_param(n, res)) {
      return;
    }
    if(realIsNegative(x) || realCompareGreaterThan(x, n)) {
      realZero(res);
      return;
    }
    realMultiply(p0, const__1, &p, realContext);
    WP34S_Ln1P(&p, &p, realContext);
    realSubtract(n, x, &q, realContext);
    realMultiply(&p, &q, &p, realContext);
    // Rewrote below with ln(yCx) function
    realCopy(n, &nn);
    realCopy(x, &xx);
    logCyxReal(&nn, &xx, &q, realContext);
    realAdd(&p, &q, &p, realContext);
    WP34S_Ln(p0, &q, realContext);
    realMultiply(&q, x, &q, realContext);
    realAdd(&p, &q, &p, realContext);
    realExp(&p, res, realContext);
  }

  void WP34S_Cdfu_Binomial(const real_t *x, const real_t *p0, const real_t *n, real_t *res, realContext_t *realContext) {
    real_t p, q, r;

    if(!binomial_param(n, res)) {
      return;
    }
    realToIntegralValue(x, &p, DEC_ROUND_CEILING, realContext);
    if(realCompareLessThan(&p, const_1)) {
      realCopy(const_1, res);
      return;
    }
    realSubtract(&p, const_1, &p, realContext);
    if(realCompareGreaterThan(&p, n)) {
      realZero(res);
      return;
    }

    realSubtract(n, &p, &q, realContext);
    realAdd(const_1, &p, &r, realContext);
    WP34S_betai(&q, &r, p0, res, realContext);
  }

  void WP34S_Cdf_Binomial(const real_t *x, const real_t *p0, const real_t *n, real_t *res, realContext_t *realContext) {
    real_t p;

    if(!binomial_param(n, res)) {
      return;
    }
    realToIntegralValue(x, &p, DEC_ROUND_FLOOR, realContext);
    WP34S_Cdf_Binomial2(&p, p0, n, res, realContext);
  }

  void WP34S_Cdf_Binomial2(const real_t *x, const real_t *p0, const real_t *n, real_t *res, realContext_t *realContext) {
    real_t p, q, r;

    if(realCompareLessThan(x, const_0)) {
      realZero(res);
      return;
    }
    if(realCompareGreaterThan(x, n)) {
      realCopy(const_1, res);
      return;
    }

    realSubtract(n, x, &q, realContext);
    realAdd(const_1, x, &r, realContext);
    realSubtract(const_1, p0, &p, realContext);
    WP34S_betai(&r, &q, &p, res, realContext);
  }

  void WP34S_Qf_Binomial(const real_t *x, const real_t *p0, const real_t *n, real_t *res, realContext_t *realContext) {
    real_t p, q, r;

    if(!binomial_param(n, res)) {
      return;
    }
    if(realCompareLessEqual(x, const_0)) {
      realZero(res);
     return;
    }
    realMultiply(p0, n, &p, realContext);       // mean = np
    realSubtract(const_1, p0, &q, realContext);
    realMultiply(&q, &p, &p, realContext);      // variance = np(1-p)
    realSquareRoot(&q, &q, realContext);
    WP34S_normal_moment_approx(x, &q, &p, &r, realContext);
    WP34S_Qf_Newton(QF_NEWTON_BINOMIAL, x, &r, p0, n, NULL, &p, realContext);
    realCopy(realCompareLessEqual(&p, n) ? &p : n, res);
  }

#endif //SAVE_SPACE_DM42_17
