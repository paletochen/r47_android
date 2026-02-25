// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file weibull.c
 ***********************************************/

#include "c47.h"

#if defined(SAVE_SPACE_DM42_15)
  void fnWeibullP     (uint16_t unusedButMandatoryParameter){}
  void fnWeibullL     (uint16_t unusedButMandatoryParameter){}
  void fnWeibullR     (uint16_t unusedButMandatoryParameter){}
  void fnWeibullI     (uint16_t unusedButMandatoryParameter){}
  void WP34S_Pdf_Weib (const real_t *x, const real_t *b, const real_t *t, real_t *res, realContext_t *realContext){}
  void WP34S_Cdfu_Weib(const real_t *x, const real_t *b, const real_t *t, real_t *res, realContext_t *realContext){}
  void WP34S_Cdf_Weib (const real_t *x, const real_t *b, const real_t *t, real_t *res, realContext_t *realContext){}
  void WP34S_Qf_Weib  (const real_t *x, const real_t *b, const real_t *t, real_t *res, realContext_t *realContext){}

#else
  static bool_t checkParamWeibull(real_t *x, real_t *shape, real_t *scale) {
    if(!saveLastX())
      return false;

    if(!getRegisterAsReal(REGISTER_X, x)
        || !getRegisterAsReal(REGISTER_Q, shape)
        || !getRegisterAsReal(REGISTER_S, scale))
      goto err;

    if(realIsNegative(x)) {
      displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function checkParamWeibull:", "cannot calculate for x < 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      goto err;
    }
    else if(realIsZero(shape) || realIsNegative(shape)
            || realIsZero(scale) || realIsNegative(scale)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function checkParamWeibull:", "cannot calculate for b " STD_LESS_EQUAL " 0 or t " STD_LESS_EQUAL " 0", NULL, NULL);
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


  void fnWeibullP(uint16_t unusedButMandatoryParameter) {
    real_t val, shape, lifetime, ans;

    if(checkParamWeibull(&val, &shape, &lifetime)) {
      WP34S_Pdf_Weib(&val, &lifetime, &shape, &ans, &ctxtReal39);
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }

  void fnWeibullL(uint16_t unusedButMandatoryParameter) {
    real_t val, shape, lifetime, ans;

    if(checkParamWeibull(&val, &shape, &lifetime)) {
      WP34S_Cdf_Weib(&val, &lifetime, &shape, &ans, &ctxtReal39);
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }

  void fnWeibullR(uint16_t unusedButMandatoryParameter) {
    real_t val, shape, lifetime, ans;

    if(checkParamWeibull(&val, &shape, &lifetime)) {
      WP34S_Cdfu_Weib(&val, &lifetime, &shape, &ans, &ctxtReal39);
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }

  void fnWeibullI(uint16_t unusedButMandatoryParameter) {
    real_t val, shape, lifetime, ans;

    if(checkParamWeibull(&val, &shape, &lifetime)) {
      if(realCompareLessEqual(&val, const_0) || realCompareGreaterEqual(&val, const_1)) {
        displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnWeibullI:", "the argument must be 0 < x < 1", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        if(getSystemFlag(FLAG_SPCRES)) {
          convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
        }
        return;
      }
      WP34S_Qf_Weib(&val, &lifetime, &shape, &ans, &ctxtReal39);
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }



  /******************************************************
   * This functions are borrowed from the WP34S project
   ******************************************************/

  void WP34S_Pdf_Weib(const real_t *x, const real_t *b, const real_t *t, real_t *res, realContext_t *realContext) {
    real_t p, q, r;

    realDivide(x, b, &p, realContext);
    if(realIsSpecial(&p) || realIsNegative(&p) || realIsZero(&p)) {
      realZero(res);
      return;
    }
    realPower(&p, t, &q, realContext);
    realMinus(&q, &r, realContext);
    realExp(&r, &r, realContext);
    realMultiply(&r, &q, &r, realContext);
    realDivide(&r, &p, &r, realContext);
    realMultiply(&r, t, &r, realContext);
    realDivide(&r, b, res, realContext);
  }

  void WP34S_Cdfu_Weib(const real_t *x, const real_t *b, const real_t *t, real_t *res, realContext_t *realContext) {
    real_t p;

    realDivide(x, b, &p, realContext);
    if(realIsNegative(&p) || realIsZero(&p)) {
      realCopy(const_1, res);
      return;
    }
    if(realIsSpecial(&p)) {
      realZero(res);
      return;
    }
    realPower(&p, t, &p, realContext);
    realChangeSign(&p);
    realExp(&p, res, realContext);
  }

  void WP34S_Cdf_Weib(const real_t *x, const real_t *b, const real_t *t, real_t *res, realContext_t *realContext) {
    real_t p;

    realDivide(x, b, &p, realContext);
    if(realIsNegative(&p) || realIsZero(&p)) {
      realZero(res);
      return;
    }
    if(realIsSpecial(&p)) {
      realCopy(const_1, res);
      return;
    }
    realPower(&p, t, &p, realContext);
    realChangeSign(&p);
    WP34S_ExpM1(&p, res, realContext);
    realChangeSign(res);
  }

  void WP34S_Qf_Weib(const real_t *x, const real_t *b, const real_t *t, real_t *res, realContext_t *realContext) {
    /* (-ln(1-p) ^ (1/k)) * J */
    real_t p, q;

    realMultiply(x, const__1, &p, realContext);
    WP34S_Ln1P(&p, &p, realContext);
    realChangeSign(&p);
    realDivide(const_1, t, &q, realContext);
    realPower(&p, &q, &p, realContext);
    realMultiply(&p, b, res, realContext);
  }

#endif //SAVE_SPACE_DM42_15
