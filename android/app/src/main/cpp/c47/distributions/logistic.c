// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file logistic.c
 ***********************************************/

#include "c47.h"

#if defined(SAVE_SPACE_DM42_15)
  void fnLogisticP     (uint16_t unusedButMandatoryParameter){}
  void fnLogisticL     (uint16_t unusedButMandatoryParameter){}
  void fnLogisticR     (uint16_t unusedButMandatoryParameter){}
  void fnLogisticI     (uint16_t unusedButMandatoryParameter){}
  void WP34S_Pdf_Logit (const real_t *x, const real_t *mu, const real_t *s, real_t *res, realContext_t *realContext){}
  void WP34S_Cdfu_Logit(const real_t *x, const real_t *mu, const real_t *s, real_t *res, realContext_t *realContext){}
  void WP34S_Cdf_Logit (const real_t *x, const real_t *mu, const real_t *s, real_t *res, realContext_t *realContext){}
  void WP34S_Qf_Logit  (const real_t *x, const real_t *mu, const real_t *s, real_t *res, realContext_t *realContext){}

#else
  static bool_t checkParamLogistic(real_t *x, real_t *i, real_t *j) {
    if(!saveLastX())
      return false;

    if(!getRegisterAsReal(REGISTER_X, x)
        || !getRegisterAsReal(REGISTER_M, i)
        || !getRegisterAsReal(REGISTER_S, j))
      goto err;

    if(realIsZero(j) || realIsNegative(j)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function checkParamLogistic:", "cannot calculate for " STD_sigma " " STD_LESS_EQUAL " 0", NULL, NULL);
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


  void fnLogisticP(uint16_t unusedButMandatoryParameter) {
    real_t val, mu, s, ans;

    if(checkParamLogistic(&val, &mu, &s)) {
      WP34S_Pdf_Logit(&val, &mu, &s, &ans, &ctxtReal39);
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }

  void fnLogisticL(uint16_t unusedButMandatoryParameter) {
    real_t val, mu, s, ans;

    if(checkParamLogistic(&val, &mu, &s)) {
      WP34S_Cdf_Logit(&val, &mu, &s, &ans, &ctxtReal39);
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }

  void fnLogisticR(uint16_t unusedButMandatoryParameter) {
    real_t val, mu, s, ans;

    if(checkParamLogistic(&val, &mu, &s)) {
      WP34S_Cdfu_Logit(&val, &mu, &s, &ans, &ctxtReal39);
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }

  void fnLogisticI(uint16_t unusedButMandatoryParameter) {
    real_t val, mu, s, ans;

    if(checkParamLogistic(&val, &mu, &s)) {
      if(realCompareLessEqual(&val, const_0) || realCompareGreaterEqual(&val, const_1)) {
        displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnLogisticI:", "the argument must be 0 < x < 1", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        if(getSystemFlag(FLAG_SPCRES)) {
          convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
        }
        return;
      }
      WP34S_Qf_Logit(&val, &mu, &s, &ans, &ctxtReal39);
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }



  /******************************************************
   * This functions are borrowed from the WP34S project
   ******************************************************/

  static void cdf_logit_common(const real_t *x, real_t *res, realContext_t *realContext) {
    real_t p;

    WP34S_Tanh(x, &p, realContext);
    realAdd(&p, const_1, &p, realContext);
    realMultiply(&p, const_1on2, res, realContext);
  }

  /* Extra the logistic rescaled parameter (x-J) / 2K */
  static void logistic_param(const real_t *x, const real_t *mu, const real_t *s, real_t *res, realContext_t *realContext) {
    real_t p;

    realSubtract(x, mu, res, realContext);
    realAdd(s, s, &p, realContext);
    realDivide(res, &p, res, realContext);
  }

  void WP34S_Pdf_Logit(const real_t *x, const real_t *mu, const real_t *s, real_t *res, realContext_t *realContext) {
    real_t xx, p;
    logistic_param(x, mu, s, &xx, realContext);
    if(realIsSpecial(&xx)) {
      realZero(res);
      return;
    }
    WP34S_SinhCosh(&xx, NULL, &p, realContext);
    realMultiply(&p, &p, &p, realContext);
    realMultiply(&p, s, &p, realContext);
    realMultiply(&p, const_4, &p, realContext);
    realDivide(const_1, &p, res, realContext);
  }

  void WP34S_Cdfu_Logit(const real_t *x, const real_t *mu, const real_t *s, real_t *res, realContext_t *realContext) {
    real_t xx;
    logistic_param(x, mu, s, &xx, realContext);
    if(realIsSpecial(&xx)) {
      realCopy(realIsNegative(&xx) ? const_0 : const_1, res);
      return;
    }
    //if(!realIsPositive(&xx)) { // WP34S returns 0 wrongly when X <= 0
    //  realCopy(const_0, res);
    //  return;
    //}
    realChangeSign(&xx);
    cdf_logit_common(&xx, res, realContext);
  }

  void WP34S_Cdf_Logit(const real_t *x, const real_t *mu, const real_t *s, real_t *res, realContext_t *realContext) {
    /* (1 + tanh( (x-J) / 2K)) / 2 */
    real_t xx;
    logistic_param(x, mu, s, &xx, realContext);
    if(realIsSpecial(&xx)) {
      realCopy(realIsNegative(&xx) ? const_0 : const_1, res);
      return;
    }
    //if(!realIsPositive(&xx)) { // WP34S returns 0 wrongly when X <= 0
    //  realCopy(const_0, res);
    //  return;
    //}
    cdf_logit_common(&xx, res, realContext);
  }

  void WP34S_Qf_Logit(const real_t *x, const real_t *mu, const real_t *s, real_t *res, realContext_t *realContext) {
    /* archtanh(2p - 1) * 2K + J */
    real_t p, q;

    realAdd(x, x, &p, realContext);
    realSubtract(&p, const_1, &p, realContext);
    WP34S_ArcTanh(&p, &q, realContext);
    realAdd(&q, &q, &p, realContext);
    realMultiply(&p, s, &p, realContext);
    realAdd(&p, mu, res, realContext);
  }

#endif //SAVE_SPACE_DM42_15
