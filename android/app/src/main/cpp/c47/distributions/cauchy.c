// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file cauchy.c
 ***********************************************/

#include "c47.h"

#if defined(SAVE_SPACE_DM42_15)
  void fnCauchyP              (uint16_t unusedButMandatoryParameter){}
  void fnCauchyL              (uint16_t unusedButMandatoryParameter){}
  void fnCauchyR              (uint16_t unusedButMandatoryParameter){}
  void fnCauchyI              (uint16_t unusedButMandatoryParameter){}
  void WP34S_Pdf_Cauchy       (const real_t *x, const real_t *x0, const real_t *gamma, real_t *res, realContext_t *realContext){}
  void WP34S_Cdfu_Cauchy      (const real_t *x, const real_t *x0, const real_t *gamma, real_t *res, realContext_t *realContext){}
  void WP34S_Cdf_Cauchy       (const real_t *x, const real_t *x0, const real_t *gamma, real_t *res, realContext_t *realContext){}
  void WP34S_Qf_Cauchy        (const real_t *x, const real_t *x0, const real_t *gamma, real_t *res, realContext_t *realContext){}
  void WP34S_cdf_cauchy_common(const real_t *x, const real_t *x0, const real_t *gamma, bool_t complementary, real_t *res, realContext_t *realContext){}
  void WP34S_cdf_cauchy_xform (const real_t *x, const real_t *x0, const real_t *gamma, real_t *res, realContext_t *realContext){}

#else
  static bool_t checkParamCauchy(real_t *x, real_t *i, real_t *j) {
    if(!saveLastX())
      return false;

    if( !getRegisterAsReal(REGISTER_X, x)
        || !getRegisterAsReal(REGISTER_M, i)
        || !getRegisterAsReal(REGISTER_S, j))
      goto err;

    if(realIsZero(j) || realIsNegative(j)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function checkParamCauchy:", "cannot calculate for " STD_gamma " " STD_LESS_EQUAL " 0", NULL, NULL);
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


  void fnCauchyP(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, x0, gamma;

    if(checkParamCauchy(&val, &x0, &gamma)) {
      WP34S_Pdf_Cauchy(&val, &x0, &gamma, &ans, &ctxtReal39);
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }


  void fnCauchyL(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, x0, gamma;

    if(checkParamCauchy(&val, &x0, &gamma)) {
      WP34S_Cdf_Cauchy(&val, &x0, &gamma, &ans, &ctxtReal39);
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }


  void fnCauchyR(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, x0, gamma;

    if(checkParamCauchy(&val, &x0, &gamma)) {
      WP34S_Cdfu_Cauchy(&val, &x0, &gamma, &ans, &ctxtReal39);
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }


  void fnCauchyI(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, x0, gamma;

    if(checkParamCauchy(&val, &x0, &gamma)) {
      if(realCompareLessEqual(&val, const_0) || realCompareGreaterEqual(&val, const_1)) {
        displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnCauchyI:", "the argument must be 0 < x < 1", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        if(getSystemFlag(FLAG_SPCRES)) {
          convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
        }
        return;
      }
      WP34S_Qf_Cauchy(&val, &x0, &gamma, &ans, &ctxtReal39);
      if(realIsNaN(&ans)) {
        displayDomainErrorMessage(ERROR_NO_ROOT_FOUND, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnCauchyI:", "WP34S_Qf_Chi2 did not converge", NULL, NULL);
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

  void WP34S_Pdf_Cauchy(const real_t *x, const real_t *x0, const real_t *gamma, real_t *res, realContext_t *realContext) {
    WP34S_cdf_cauchy_xform(x, x0, gamma, res, realContext);
    if(realIsSpecial(res)) {
      realZero(res); /* Can only be infinite which has zero probability */
      return;
    }
    realMultiply(res, res, res, realContext);
    realAdd(res, const_1, res, realContext);
    realMultiply(res, gamma, res, realContext);
    realMultiply(res, const_pi, res, realContext);
    realDivide(const_1, res, res, realContext);
  }

  void WP34S_Cdfu_Cauchy(const real_t *x, const real_t *x0, const real_t *gamma, real_t *res, realContext_t *realContext) {
    WP34S_cdf_cauchy_common(x, x0, gamma, true, res, realContext);
  }

  void WP34S_Cdf_Cauchy(const real_t *x, const real_t *x0, const real_t *gamma, real_t *res, realContext_t *realContext) {
    WP34S_cdf_cauchy_common(x, x0, gamma, false, res, realContext);
  }

  void WP34S_cdf_cauchy_common(const real_t *x, const real_t *x0, const real_t *gamma, bool_t complementary, real_t *res, realContext_t *realContext) {
    real_t p;

    WP34S_cdf_cauchy_xform(x, x0, gamma, &p, realContext);
    if(realIsSpecial(&p)) {
      realCopy(const_plusInfinity, res);
      return;
    }
    WP34S_Atan(&p, &p, realContext);
    realDivide(&p, const_pi, &p, realContext);
    if(complementary) {
      realChangeSign(&p);
    }
    realAdd(&p, const_1on2, res, realContext);
  }

  void WP34S_cdf_cauchy_xform(const real_t *x, const real_t *x0, const real_t *gamma, real_t *res, realContext_t *realContext) {
    realSubtract(x, x0, res, realContext);
    realDivide(res, gamma, res, realContext);
  }

  void WP34S_Qf_Cauchy(const real_t *x, const real_t *x0, const real_t *gamma, real_t *res, realContext_t *realContext) {
    real_t p, s, c;

    realSubtract(x, const_1on2, &p, realContext);
    realMultiply(&p, const_pi, &p, realContext);
    WP34S_SinCosTanTaylor(&p, false, &s, &c, &p, realContext);
    realMultiply(&p, gamma, &p, realContext);
    realAdd(&p, x0, res, realContext);
  }

#endif //SAVE_SPACE_DM42_15
