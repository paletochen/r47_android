// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file geometric.c
 ***********************************************/

#include "c47.h"

#if !defined(OPTION_DIST_GEOM)
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
  bool_t geometricCheckDomain(bool_t registersValid, real_t *x, real_t *i, real_t *unusedSecond, real_t *unusedThird, uint16_t unusedParameter) {
    if(!registersValid) {
      return false;
    }

    if(realIsNaN(x)) {
      if(!getSystemFlag(FLAG_SPCRES)) {
        displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function geometricCheckDomain:", "x is NaN", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      return false;
    }
    else if(realIsSpecial(i) || realCompareLessEqual(i, const_0) || realCompareGreaterThan(i, const_1)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function geometricCheckDomain:", "the parameter must be 0 < p " STD_LESS_EQUAL " 1", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
    return true;
  }


  void fnGeometricP(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_GEOM * 4 + DIST_P);
  }


  void fnGeometricL(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_GEOM * 4 + DIST_L);
  }


  void fnGeometricR(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_GEOM * 4 + DIST_R);
  }


  void fnGeometricI(uint16_t unusedButMandatoryParameter) {
    fnDistribution(DIST_GEOM * 4 + DIST_I);
  }



  /******************************************************
   * This functions are borrowed from the WP34S project
   ******************************************************/

  /********************************************//**
   * \brief Checks the geometric probability parameter.
   ***********************************************/
  static bool_t geometricParameterIsValid(const real_t *p) {
    return !realIsSpecial(p) && realCompareGreaterThan(p, const_0) && realCompareLessEqual(p, const_1);
  }


  /********************************************//**
   * \brief Evaluates log(1-p) without losing a small p or a small 1-p.
   ***********************************************/
  static void geometricLogSurvival(const real_t *p, real_t *res, realContext_t *realContext) {
    real_t argument;

    if(realCompareLessEqual(p, const_1on2)) {
      realMinus(p, &argument, realContext);
      WP34S_Ln1P(&argument, res, realContext);
    }
    else {
      realSubtract(const_1, p, &argument, realContext);
      WP34S_Ln(&argument, res, realContext);
    }
  }


  void WP34S_Pdf_Geom(const real_t *x, const real_t *p0, real_t *res, realContext_t *realContext) {
    real_t p;

    if(realIsNaN(x) || !geometricParameterIsValid(p0)) {
      realSetNaN(res);
      return;
    }
    if(realCompareLessThan(x, const_0) || realIsInfinite(x) || !realIsAnInteger(x)) {
      realSetZero(res);
      return;
    }
    if(realIsZero(x)) {
      realCopy(p0, res);
      return;
    }
    geometricLogSurvival(p0, &p, realContext);
    realMultiply(x, &p, &p, realContext);
    realExp(&p, &p, realContext);
    realMultiply(&p, p0, res, realContext);
    if(realCompareLessThan(p0, const_1)) {
      WP34S_MarkProbability(res, realContext);
    }
  }

  void WP34S_Cdfu_Geom(const real_t *x, const real_t *p0, real_t *res, realContext_t *realContext) {
    real_t p, q;

    if(realIsNaN(x) || !geometricParameterIsValid(p0)) {
      realSetNaN(res);
      return;
    }
    realToIntegralValue(x, &p, DEC_ROUND_CEILING, realContext);
    if(realCompareLessThan(&p, const_1)) {
      realSetOne(res);
      return;
    }
    if(realIsInfinite(&p)) {
      realSetZero(res);
      return;
    }
    geometricLogSurvival(p0, &q, realContext);
    realMultiply(&p, &q, &p, realContext);
    realExp(&p, res, realContext);
    if(realCompareLessThan(p0, const_1)) {
      WP34S_MarkProbability(res, realContext);
    }
  }

  void WP34S_Cdf_Geom(const real_t *x, const real_t *p0, real_t *res, realContext_t *realContext) {
    real_t p, q;

    if(realIsNaN(x) || !geometricParameterIsValid(p0)) {
      realSetNaN(res);
      return;
    }
    if(realCompareLessThan(x, const_0)) {
      realSetZero(res);
      return;
    }
    if(realIsInfinite(x)) {
      realSetOne(res);
      return;
    }
    realToIntegralValue(x, &p, DEC_ROUND_FLOOR, realContext);
    realAdd(&p, const_1, &p, realContext);
    geometricLogSurvival(p0, &q, realContext);
    realMultiply(&p, &q, &p, realContext);
    WP34S_ExpM1(&p, res, realContext);
    realChangeSign(res);
    if(realCompareLessThan(p0, const_1)) {
      WP34S_MarkProbability(res, realContext);
    }
  }

  static bool_t qfDiscreteBelow(uint16_t dist, const real_t *r, const real_t *p, const real_t *i, const real_t *j, const real_t *k);

  void WP34S_Qf_Geom(const real_t *x, const real_t *p0, real_t *res, realContext_t *realContext) {
    real_t p, q;

    if(realIsNaN(x) || !geometricParameterIsValid(p0) || realCompareLessThan(x, const_0) || realCompareGreaterThan(x, const_1)) {
      realSetNaN(res);
      return;
    }
    if(realIsZero(x)) {
      realSetZero(res);
      return;
    }
    if(realCompareEqual(x, const_1)) {
      realSetPlusInfinity(res);
      return;
    }
    realMinus(x, &p, realContext);
    WP34S_Ln1P(&p, &p, realContext);
    geometricLogSurvival(p0, &q, realContext);
    realDivide(&p, &q, &p, realContext);
    realSubtract(&p, const_1, &p, realContext);
    realToIntegralValue(&p, &p, DEC_ROUND_FLOOR, realContext);
    if(qfDiscreteBelow(QF_DISCRETE_CDF_GEOMETRIC, &p, x, p0, NULL, NULL)) {  // the closed form is accurate to the context, so the quantile is its floor or the next integer
      realAdd(&p, const_1, &p, realContext);
    }
    realCopy(&p, res);
  }

  /********************************************//**
   * \brief Returns true when the CDF of a discrete distribution at r is below p.
   ***********************************************/
  static bool_t qfDiscreteBelow(uint16_t dist, const real_t *r, const real_t *p, const real_t *i, const real_t *j, const real_t *k) {
    realContext_t tieContext = ctxtReal51;
    real_t q;

    switch(dist) { // qf_discrete_cdf
      case QF_DISCRETE_CDF_POISSON:        WP34S_Cdf_Poisson2(r, i, &q, &ctxtReal51);        break;
      case QF_DISCRETE_CDF_BINOMIAL:       WP34S_Cdf_Binomial2(r, i, j, &q, &ctxtReal51);    break;
      case QF_DISCRETE_CDF_GEOMETRIC:      WP34S_Cdf_Geom(r, i, &q, &ctxtReal51);            break;
      case QF_DISCRETE_CDF_NEGBINOM:       cdf_NegBinomial2(r, i, j, &q, &ctxtReal51);       break;
      case QF_DISCRETE_CDF_HYPERGEOMETRIC: cdf_Hypergeometric2(r, i, j, k, &q, &ctxtReal75); break;
      default:                             realSetZero(&q);
    }
    // At an exact tie, such as L(0) = u = 1E-34 for NB(1, 1 - 1E-34), the CDF comes out up to about 1E-45 of u either side. Rounded to 43 digits a tie meets
    // u, and the documented near-ties, 4E-40 below u, stay below.
    tieContext.digits = 43;
    realPlus(&q, &q, &tieContext);
    return realCompareLessThan(&q, p);
  }


  void WP34S_qf_discrete_final(uint16_t dist, const real_t *r, const real_t *p, const real_t *i, const real_t *j, const real_t *k, real_t *res, realContext_t *realContext) {
    real_t low, high, step;
    real_t *nearEnd, *farEnd;
    bool_t below;

    // The quantile is the smallest integer whose CDF reaches p. farEnd moves from r, up when the CDF at r is below p and down otherwise, by a step that doubles until
    // the CDF at farEnd crosses p, and nearEnd follows one position behind. The bracket from low to high is then halved, so a start d away costs about 2 log2(d) CDF
    // evaluations. Where the integers are further apart than the step, as beyond 1E39 at 39 digits, a step that leaves the value unchanged ends the doubling and a
    // midpoint outside the bracket ends the halving.
    below = qfDiscreteBelow(dist, r, p, i, j, k);
    nearEnd = below ? &low : &high;
    farEnd = below ? &high : &low;
    realCopy(below ? const_1 : const__1, &step);
    realCopy(r, farEnd);
    do {
      realCopy(farEnd, nearEnd);
      realAdd(nearEnd, &step, farEnd, realContext);
      realAdd(&step, &step, &step, realContext);
    } while(!realCompareEqual(&high, &low) && qfDiscreteBelow(dist, farEnd, p, i, j, k) == below);
    while(true) {
      realAdd(&low, &high, &step, realContext);
      realMultiply(&step, const_1on2, &step, realContext);
      realToIntegralValue(&step, &step, DEC_ROUND_FLOOR, realContext);
      if(!realCompareGreaterThan(&step, &low) || !realCompareLessThan(&step, &high)) {
        break;
      }
      if(qfDiscreteBelow(dist, &step, p, i, j, k)) {
        realCopy(&step, &low);
      }
      else {
        realCopy(&step, &high);
      }
    }
    realCopy(&high, res);
  }

#endif // !OPTION_DIST_GEOM
