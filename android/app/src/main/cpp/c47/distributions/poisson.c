// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file poisson.c
 ***********************************************/

#include "c47.h"

#if defined(SAVE_SPACE_DM42_17)
  void fnPoissonP                (uint16_t unusedButMandatoryParameter){}
  void fnPoissonL                (uint16_t unusedButMandatoryParameter){}
  void fnPoissonR                (uint16_t unusedButMandatoryParameter){}
  void fnPoissonI                (uint16_t unusedButMandatoryParameter){}
  //void WP34S_normal_moment_approx(const real_t *prob, const real_t *var, const real_t *mean, real_t *res, realContext_t *realContext){}
  //void WP34S_Pdf_Poisson         (const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext){}
  //void WP34S_Cdfu_Poisson        (const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext){}
  //void WP34S_Cdf_Poisson         (const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext){}
  //void WP34S_Cdf_Poisson2        (const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext){}
  //void WP34S_Qf_Poisson          (const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext){}

#else
  static bool_t checkParamPoisson(real_t *x, real_t *i) {
    if(!saveLastX())
      return false;

    if(!getRegisterAsReal(REGISTER_X, x)
        || !getRegisterAsReal(REGISTER_R, i))
        goto err;
    if(realIsNegative(x)) {
      displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function checkParamPoisson:", "cannot calculate for x < 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      goto err;
    }
    else if(realIsZero(i) || realIsNegative(i)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function checkParamPoisson:", "the parameter must be " STD_lambda " > 0", NULL, NULL);
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


  void fnPoissonP(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, prob;

    if(checkParamPoisson(&val, &prob)) {
      WP34S_Pdf_Poisson(&val, &prob, &ans, &ctxtReal39);
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }


  void fnPoissonL(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, prob;

    if(checkParamPoisson(&val, &prob)) {
      WP34S_Cdf_Poisson(&val, &prob, &ans, &ctxtReal39);
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }


  void fnPoissonR(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, prob;

    if(checkParamPoisson(&val, &prob)) {
      WP34S_Cdfu_Poisson(&val, &prob, &ans, &ctxtReal39);
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }


  void fnPoissonI(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, prob;

    if(checkParamPoisson(&val, &prob)) {
      if(realCompareLessEqual(&val, const_0) || realCompareGreaterEqual(&val, const_1)) {
        displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnPoissonI:", "the argument must be 0 < x < 1", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        if(getSystemFlag(FLAG_SPCRES)) {
          convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
        }
        return;
      }
      WP34S_Qf_Poisson(&val, &prob, &ans, &ctxtReal39);
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }



  /******************************************************
   * This functions are borrowed from the WP34S project
   ******************************************************/

  /* Stack contains probability in Z, variance in Y and mean in X.
   * Returns a normal approximation in X.
   */
  void WP34S_normal_moment_approx(const real_t *prob, const real_t *var, const real_t *mean, real_t *res, realContext_t *realContext) {
    real_t p, q, r;

    WP34S_qf_q_est(prob, &p, NULL, realContext);
    realMultiply(&p, &p, &q, realContext);
    realSubtract(&q, const_1, &q, realContext);
    int32ToReal(6, &r);
    realDivide(&q, &r, &q, realContext);
    realDivide(&q, var, &q, realContext);
    realAdd(&p, &q, &p, realContext);
    realMultiply(&p, var, &p, realContext);
    realAdd(&p, mean, res, realContext);
  }

  /* One parameter Poission distribution
   * Real parameter lambda in I.
   */
  void WP34S_Pdf_Poisson(const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext) {
    real_t p, q, r;

    if(realIsNegative(x) /* poission1_param */ || (!realIsAnInteger(x) /* pdf_poisson_xout */)) {
      realZero(res);
      return;
    }
    WP34S_Ln(lambda, &p, realContext);
    realMultiply(&p, x, &p, realContext);
    realSubtract(&p, lambda, &p, realContext);
    realAdd(x, const_1, &q, realContext);
    WP34S_LnGamma(&q, &r, realContext);
    realSubtract(&p, &r, &q, realContext); // ln(PDF) = x*ln(lambda) - lambda - lngamma(x+1)
    realExp(&q, res, realContext);
  }

  void WP34S_Cdfu_Poisson(const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext) {
    real_t p;

    if(realCompareLessEqual(lambda, const_0)) { // poission1_param
      realZero(res);
      return;
    }
    // cdfu_poisson_xout
    realToIntegralValue(x, &p, DEC_ROUND_CEILING, realContext);
    if(realCompareLessThan(&p, const_1)) {
      realCopy(const_1, res);
      return;
    }
    if(realIsInfinite(&p)) {
      realCopy(const_0, res);
      return;
    }
    WP34S_GammaP(lambda, &p, res, realContext, false, true);
  }

  void WP34S_Cdf_Poisson(const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext) {
    real_t p;

    if(realCompareLessEqual(lambda, const_0)) { // poission1_param
      realZero(res);
      return;
    }
    // cdf_poisson_xout
    realToIntegralValue(x, &p, DEC_ROUND_FLOOR, realContext);
    WP34S_Cdf_Poisson2(&p, lambda, res, realContext);
  }

  void WP34S_Cdf_Poisson2(const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext) {
    real_t p;

    // cdf_poisson
    if(realCompareLessThan(x, const_0)) {
      realCopy(const_0, res);
      return;
    }
    if(realIsInfinite(x)) {
      realCopy(const_1, res);
      return;
    }
    realAdd(x, const_1, &p, realContext);
    WP34S_GammaP(lambda, &p, res, realContext, true, true);
  }

  void WP34S_Qf_Poisson(const real_t *x, const real_t *lambda, real_t *res, realContext_t *realContext) {
    real_t p, q;

    if(realCompareLessEqual(lambda, const_0)) { // poission1_param
      realZero(res);
      return;
    }

    // qf_poisson_xout
    realSquareRoot(lambda, &p, realContext);
    WP34S_normal_moment_approx(x, &p, lambda, &q, realContext);
    WP34S_Qf_Newton(QF_NEWTON_POISSON, x, &q, lambda, NULL, NULL, res, realContext);
  }

#endif //SAVE_SPACE_DM42_17
