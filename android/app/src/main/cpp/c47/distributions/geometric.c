// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file geometric.c
 ***********************************************/

#include "c47.h"

#if defined(SAVE_SPACE_DM42_17)
  void fnGeometricP           (uint16_t unusedButMandatoryParameter){}
  void fnGeometricL           (uint16_t unusedButMandatoryParameter){}
  void fnGeometricR           (uint16_t unusedButMandatoryParameter){}
  void fnGeometricI           (uint16_t unusedButMandatoryParameter){}
  //void WP34S_Pdf_Geom         (const real_t *x, const real_t *p0, real_t *res, realContext_t *realContext){}
  //void WP34S_Cdfu_Geom        (const real_t *x, const real_t *p0, real_t *res, realContext_t *realContext){}
  //void WP34S_Cdf_Geom         (const real_t *x, const real_t *p0, real_t *res, realContext_t *realContext){}
  //void WP34S_Qf_Geom          (const real_t *x, const real_t *p0, real_t *res, realContext_t *realContext){}
  //void WP34S_qf_discrete_final(uint16_t dist, const real_t *r, const real_t *p, const real_t *i, const real_t *j, const real_t *k, real_t *res, realContext_t *realContext){}

#else
  static bool_t checkParamGeometric(real_t *x, real_t *i) {
    if(!saveLastX())
      return false;

    if(!getRegisterAsReal(REGISTER_X, x) || !getRegisterAsReal(REGISTER_P, i))
      goto err;

    if(realIsNegative(x)) {
      displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function checkParamGeometric:", "cannot calculate for x < 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      goto err;
    }
    else if(realIsZero(i) || realIsNegative(i) || realCompareGreaterThan(i, const_1)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function checkParamGeometric:", "the parameter must be 0 < p " STD_LESS_EQUAL " 1", NULL, NULL);
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


  void fnGeometricP(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, prob;

    if(checkParamGeometric(&val, &prob)) {
      WP34S_Pdf_Geom(&val, &prob, &ans, &ctxtReal39);
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }


  void fnGeometricL(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, prob;

    if(checkParamGeometric(&val, &prob)) {
      WP34S_Cdf_Geom(&val, &prob, &ans, &ctxtReal39);
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }


  void fnGeometricR(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, prob;

    if(checkParamGeometric(&val, &prob)) {
      WP34S_Cdfu_Geom(&val, &prob, &ans, &ctxtReal39);
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }


  void fnGeometricI(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, prob;

    if(checkParamGeometric(&val, &prob)) {
      if(realCompareLessEqual(&val, const_0) || realCompareGreaterEqual(&val, const_1)) {
        displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnGeometricI:", "the argument must be 0 < x < 1", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        if(getSystemFlag(FLAG_SPCRES)) {
          convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
        }
        return;
      }
      WP34S_Qf_Geom(&val, &prob, &ans, &ctxtReal39);
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }



  /******************************************************
   * This functions are borrowed from the WP34S project
   ******************************************************/

  void WP34S_Pdf_Geom(const real_t *x, const real_t *p0, real_t *res, realContext_t *realContext) {
    real_t p;

    if(realIsNegative(x) || (!realIsAnInteger(x))) {
      realZero(res);
      return;
    }
    realMultiply(p0, const__1, &p, realContext);
    WP34S_Ln1P(&p, &p, realContext);
    realMultiply(x, &p, &p, realContext);
    realExp(&p, &p, realContext);
    realMultiply(&p, p0, res, realContext);
  }

  void WP34S_Cdfu_Geom(const real_t *x, const real_t *p0, real_t *res, realContext_t *realContext) {
    real_t p, q;

    realToIntegralValue(x, &p, DEC_ROUND_CEILING, realContext);
    if(realCompareLessThan(&p, const_1)) {
      realCopy(const_1, res);
      return;
    }
    if(realIsInfinite(&p)) {
      realCopy(const_0, res);
      return;
    }
    realSubtract(const_1, p0, &q, realContext);
    realPower(&q, &p, res, realContext);
  }

  void WP34S_Cdf_Geom(const real_t *x, const real_t *p0, real_t *res, realContext_t *realContext) {
    real_t p, q;

    if(realCompareLessThan(x, const_0)) {
      realCopy(const_0, res);
      return;
    }
    if(realIsInfinite(x)) {
      realCopy(const_1, res);
      return;
    }
    realToIntegralValue(x, &p, DEC_ROUND_FLOOR, realContext);
    realAdd(&p, const_1, &p, realContext);
    realMultiply(p0, const__1, &q, realContext);
    WP34S_Ln1P(&q, &q, realContext);
    realMultiply(&p, &q, &p, realContext);
    WP34S_ExpM1(&p, res, realContext);
    realChangeSign(res);
  }

  void WP34S_Qf_Geom(const real_t *x, const real_t *p0, real_t *res, realContext_t *realContext) {
    real_t p, q;

    if(realCompareLessEqual(x, const_0)) {
      realZero(res);
      return;
    }
    realMultiply(x, const__1, &p, realContext);
    WP34S_Ln1P(&p, &p, realContext);
    realMultiply(p0, const__1, &q, realContext);
    WP34S_Ln1P(&q, &q, realContext);
    realDivide(&p, &q, &p, realContext);
    realSubtract(&p, const_1, &p, realContext);
    realToIntegralValue(&p, &p, DEC_ROUND_FLOOR, realContext);
    WP34S_qf_discrete_final(QF_DISCRETE_CDF_GEOMETRIC, &p, x, p0, NULL, NULL, res, realContext);
  }

  void WP34S_qf_discrete_final(uint16_t dist, const real_t *r, const real_t *p, const real_t *i, const real_t *j, const real_t *k, real_t *res, realContext_t *realContext) {
    real_t q;

    switch(dist) { // qf_discrete_cdf
      case QF_DISCRETE_CDF_POISSON:        WP34S_Cdf_Poisson2(r, i, &q, &ctxtReal51);        break;
      case QF_DISCRETE_CDF_BINOMIAL:       WP34S_Cdf_Binomial2(r, i, j, &q, &ctxtReal51);    break;
      case QF_DISCRETE_CDF_GEOMETRIC:      WP34S_Cdf_Geom(r, i, &q, &ctxtReal51);            break;
      case QF_DISCRETE_CDF_NEGBINOM:       cdf_NegBinomial2(r, i, j, &q, &ctxtReal51);       break;
      case QF_DISCRETE_CDF_HYPERGEOMETRIC: cdf_Hypergeometric2(r, i, j, k, &q, &ctxtReal75); break;
      default:                             realZero(&q);
    }
    realAdd(&q, const_0, &q, realContext);
    if(realCompareLessThan(&q, p)) {
      realAdd(r, const_1, res, realContext);
    }
    else { // qf_discrere_out
      realCopy(r, res);
    }
  }

#endif //SAVE_SPACE_DM42_17
