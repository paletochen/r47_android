// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file f.c
 ***********************************************/

#include "c47.h"

#if defined(SAVE_SPACE_DM42_17)
  void fnF_P(uint16_t unusedButMandatoryParameter){}
  void fnF_L(uint16_t unusedButMandatoryParameter){}
  void fnF_R(uint16_t unusedButMandatoryParameter){}
  void fnF_I(uint16_t unusedButMandatoryParameter){}
//  void WP34S_Pdf_F (const real_t *x, const real_t *d1, const real_t *d2, real_t *res, realContext_t *realContext){}
//  void WP34S_Cdfu_F(const real_t *x, const real_t *d1, const real_t *d2, real_t *res, realContext_t *realContext){}
//  void WP34S_Cdf_F (const real_t *x, const real_t *d1, const real_t *d2, real_t *res, realContext_t *realContext){}
//  void WP34S_Qf_F  (const real_t *x, const real_t *d1, const real_t *d2, real_t *res, realContext_t *realContext){}
//  void WP34S_Qf_Newton(uint32_t r_dist, const real_t *target, const real_t *estimate, const real_t *p1, const real_t *p2, const real_t *p3, real_t *res, realContext_t *realContext){}

#else
  static bool_t checkParamF(real_t *x, real_t *i, real_t *j) {
    if(!saveLastX())
      return false;

    if(!getRegisterAsReal(REGISTER_X, x)
        || !getRegisterAsReal(REGISTER_M, i)
        || !getRegisterAsReal(REGISTER_N, j))
      goto err;

    if(!(checkRegisterNoFP(i) || checkRegisterNoFP(j))) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function checkParamF:", "d1 or d2 is not an integer", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      goto err;
    }
    if(realIsNegative(x)) {
      displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function checkParamF:", "cannot calculate for x < 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      goto err;
    }
    if(realIsZero(i) || realIsNegative(i) || realIsZero(j) || realIsNegative(j)) {
      displayDomainErrorMessage(ERROR_INVALID_DISTRIBUTION_PARAM, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function checkParamF:", "cannot calculate for d1 " STD_LESS_EQUAL " 0 or d2 " STD_LESS_EQUAL " 0", NULL, NULL);
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


  void fnF_P(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, d1, d2;

    if(checkParamF(&val, &d1, &d2)) {
      WP34S_Pdf_F(&val, &d1, &d2, &ans, &ctxtReal39);
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }


  void fnF_L(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, d1, d2;

    if(checkParamF(&val, &d1, &d2)) {
      WP34S_Cdf_F(&val, &d1, &d2, &ans, &ctxtReal39);
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }


  void fnF_R(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, d1, d2;

    if(checkParamF(&val, &d1, &d2)) {
      WP34S_Cdfu_F(&val, &d1, &d2, &ans, &ctxtReal39);
      convertRealToResultRegister(&ans, REGISTER_X, amNone);
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }


  void fnF_I(uint16_t unusedButMandatoryParameter) {
    real_t val, ans, d1, d2;

    if(checkParamF(&val, &d1, &d2)) {
      if(realCompareLessEqual(&val, const_0) || realCompareGreaterEqual(&val, const_1)) {
        displayDomainErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnF_I:", "the argument must be 0 < x < 1", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        if(getSystemFlag(FLAG_SPCRES)) {
          convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
        }
        return;
      }
      WP34S_Qf_F(&val, &d1, &d2, &ans, &ctxtReal39);
      if(realIsNaN(&ans)) {
        displayDomainErrorMessage(ERROR_NO_ROOT_FOUND, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnF_I:", "WP34S_Qf_F did not converge", NULL, NULL);
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

  void WP34S_Pdf_F(const real_t *x, const real_t *d1, const real_t *d2, real_t *res, realContext_t *realContext) {
    real_t p, q, r;

    WP34S_Ln(d2, &p, realContext);
    realMultiply(&p, d2, &p, realContext);
    realMultiply(d1, x, &q, realContext);
    WP34S_Ln(&q, &r, realContext);
    realMultiply(&r, d1, &r, realContext);
    realAdd(&p, &r, &p, realContext);
    realAdd(&q, d2, &q, realContext);
    WP34S_Ln(&q, &q, realContext);
    realAdd(d1, d2, &r, realContext);
    realMultiply(&q, &r, &q, realContext);
    realSubtract(&p, &q, &p, realContext);
    realMultiply(&p, const_1on2, &p, realContext);
    realMultiply(d1, const_1on2, &q, realContext);
    realMultiply(d2, const_1on2, &r, realContext);
    LnBeta(&r, &q, &q, realContext);
    realSubtract(&p, &q, &p, realContext);
    realExp(&p, &p, realContext);
    realDivide(&p, x, res, realContext);
  }

  void WP34S_Cdfu_F(const real_t *x, const real_t *d1, const real_t *d2, real_t *res, realContext_t *realContext) {
    real_t p, q, r;

    if(realCompareLessEqual(x, const_0)) {
      realCopy(const_1, res);
      return;
    }
    if(realIsInfinite(x)) { // assume x is positive here...
      realCopy(const_0, res);
      return;
    }

    realMultiply(x, d1, &p, realContext);
    realAdd(&p, d2, &p, realContext);
    realDivide(d2, &p, &p, realContext);
    realMultiply(d2, const_1on2, &q, realContext);
    realMultiply(d1, const_1on2, &r, realContext);
    WP34S_betai(&r, &q, &p, res, realContext);
  }

  void WP34S_Cdf_F(const real_t *x, const real_t *d1, const real_t *d2, real_t *res, realContext_t *realContext) {
    real_t p, q, r;

    if(realCompareLessEqual(x, const_0)) {
      realCopy(const_0, res);
      return;
    }
    if(realIsInfinite(x)) { // assume x is positive here...
      realCopy(const_1, res);
      return;
    }

    realMultiply(x, d1, &p, realContext);
    realAdd(&p, d2, &q, realContext);
    realDivide(&p, &q, &p, realContext);
    realMultiply(d2, const_1on2, &q, realContext);
    realMultiply(d1, const_1on2, &r, realContext);
    WP34S_betai(&q, &r, &p, res, realContext);
  }

void WP34S_Qf_F(const real_t *x, const real_t *d1, const real_t *d2, real_t *res, realContext_t *realContext) {
  real_t p, q, r, s, reg0, reg1, reg2;

  // qf_f_est
  realCopy(x, &reg0);
  WP34S_qf_q_est(x, &p, NULL, realContext);
  realCopy(d1, &r);
  if(realCompareGreaterThan(&r, const_1)) {
    realSubtract(&r, const_1, &r, realContext);
  }
  realDivide(const_1, &r, &r, realContext);
  realCopy(d2, &s);
  if(realCompareGreaterThan(&s, const_1)) {
    realSubtract(&s, const_1, &s, realContext);
  }
  realDivide(const_1, &s, &s, realContext);
  realCopy(&s, &reg1), realCopy(&r, &reg2);
  realAdd(&r, &s, &r, realContext);
  realDivide(const_2, &r, &r, realContext);
  realMultiply(&p, &p, &s, realContext);
  realSubtract(&s, const_3, &s, realContext);
  int32ToReal(6, &q);
  realDivide(&s, &q, &s, realContext);
  realAdd(&s, &r, &q, realContext);
  realSquareRoot(&q, &q, realContext);
  realMultiply(&q, &p, &q, realContext);
  realDivide(&q, &r, &q, realContext);
  realDivide(const_2, &r, &p, realContext);
  realMultiply(&p, const_1on3, &p, realContext);
  realChangeSign(&p);
  realAdd(&p, &s, &p, realContext);
  realDivide(const_5, const_60, &r, realContext), r.exponent += 1;
  realAdd(&p, &r, &p, realContext);
  realSubtract(&reg2, &reg1, &r, realContext);
  realMultiply(&p, &r, &p, realContext);
  realSubtract(&q, &p, &q, realContext);
  realMultiply(&q, const_2, &q, realContext);
  realExp(&q, &q, realContext);

  WP34S_Qf_Newton(QF_NEWTON_F, &reg0, &q, d1, d2, NULL, res, realContext);
}


  /**************************************************************************/
  /* The Newton step solver.
   * Target value in Y, function identifier in X, estimate in Z.
   */
  void WP34S_Qf_Newton(uint32_t r_dist, const real_t *target, const real_t *estimate, const real_t *p1, const real_t *p2, const real_t *p3, real_t *res, realContext_t *realContext) {
    real_t p, q;
    real_t r_p, r_low, r_high, r_maxstep, r_r, r_z, r_w;
    uint32_t r_iterations;
    bool_t f_discrete = false, f_nonnegative = false, f_nobisect = false, f_limitjump = false;

    // qf_newton
    realCopy(target, &r_p);
    realCopy(estimate, &r_r);

    /* Set flags based on distribution */
    // f_newton_setflags
    if(r_dist != QF_NEWTON_F) {
      f_discrete = true; // Poisson or Binomial
    }

    // qf_f_flags
    realMultiply(const_3on2, const_1on2, &p, realContext);
    realMultiply(&p, &r_r, &p, realContext);
    realCopy(&p, &r_maxstep);
    f_limitjump = true;
    f_nonnegative = true;

    r_iterations = 100;
    realCopy(const_plusInfinity, &r_high);
    realCopy(f_nonnegative ? const_0 : const_minusInfinity, &r_low);

    /* Initialisation done, now the main loop itself */
    do { // qf_newton_search
      if(realCompareGreaterEqual(&r_r, &r_high) || realCompareLessEqual(&r_r, &r_low)) { // qf_newton_try_bisect
        /* The estimate is out of the (low, high) bounds.  Check to see if we can bisect instead */
        if(realIsInfinite(&r_high)) { // qf_newton_overlow
          realMultiply(&r_low, const_2, &r_r, realContext);
        }
        else if(realIsInfinite(&r_low)) { // qf_newton_underhigh
          realMultiply(&r_high, const_2, &r_r, realContext);
        }
        else {
  qf_newton_do_bisect:
          realAdd(&r_low, &r_high, &r_r, realContext);
          realMultiply(&r_r, const_2, &r_r, realContext);
          f_nobisect = true;
        }
        //qf_newton_bisect_out
      }
      // qf_newton_no_bisect
      /* Dispatch to the CDF of the distribution being searched */
      switch(r_dist) { // qf_newton_cdf
        case QF_NEWTON_F:              WP34S_Cdf_F(&r_r, p1, p2, &r_w, realContext);             break;
        case QF_NEWTON_POISSON:        WP34S_Cdf_Poisson2(&r_r, p1, &r_w, realContext);          break;
        case QF_NEWTON_BINOMIAL:       WP34S_Cdf_Binomial2(&r_r, p1, p2, &r_w, realContext);     break;
        case QF_NEWTON_NEGBINOM:       cdf_NegBinomial2(&r_r, p1, p2, &r_w, realContext);        break;
        case QF_NEWTON_HYPERGEOMETRIC: cdf_Hypergeometric2(&r_r, p1, p2, p3, &r_w, &ctxtReal75); break;
      }
      realSubtract(&r_w, &r_p, &r_z, realContext);
      if(realCompareGreaterEqual(&r_z, const_0)) { // qf_newton_fix_high
        if(f_nobisect || realCompareLessThan(&r_r, &r_high) || realIsInfinite(&r_low)) {
          f_nobisect = false;
          if(realCompareLessThan(&r_r, &r_high)) { // qf_newton_fix_high2
            realCopy(&r_r, &r_high);
          }
          goto qf_newton_fixed;
        }
        goto qf_newton_do_bisect;
      }
      else { // qf_newton_fix_low
        if(f_nobisect || realCompareGreaterThan(&r_r, &r_low) || realIsInfinite(&r_high)) {
          f_nobisect = false;
          if(realCompareGreaterThan(&r_r, &r_low)) { // qf_newton_fix_low2
            realCopy(&r_r, &r_low);
          }
          goto qf_newton_fixed;
        }
        goto qf_newton_do_bisect;
      }
      /* Evaluated the CDF and it is within the bounds */
  qf_newton_fixed:
      /* Dispatch to the PDF of the distribution being searched */
      switch(r_dist) { // qf_newton_pdf
        case QF_NEWTON_F:              WP34S_Pdf_F(&r_r, p1, p2, &q, realContext);            break;
        case QF_NEWTON_POISSON:        WP34S_Pdf_Poisson(&r_r, p1, &q, realContext);          break;
        case QF_NEWTON_BINOMIAL:       WP34S_Pdf_Binomial(&r_r, p1, p2, &q, realContext);     break;
        case QF_NEWTON_NEGBINOM:       pdf_NegBinomial(&r_r, p1, p2, &q, realContext);        break;
        case QF_NEWTON_HYPERGEOMETRIC: pdf_Hypergeometric(&r_r, p1, p2, p3, &q, realContext); break;
      }
      if(realIsZero(&q)) {
        goto qf_newton_done; /* Function is flat -- can't estimate */
      }
      realDivide(&r_z, &q, &r_w, realContext);
      realCopyAbs(&r_w, &q);
      if(f_limitjump && realCompareGreaterEqual(&q, &r_maxstep)) {
        if(realCompareLessThan(&r_w, const_0)) { // qf_newton_neg_limit
          realMultiply(&r_maxstep, const__1, &r_w, realContext);
        }
        else { // qf_newton_fin_limit
          realCopy(&r_maxstep, &r_w);
        }
      }
      // update estimate
      // qf_newton_no_limit
      realCopy(&r_r, &r_z);
      realSubtract(&r_r, &r_w, &r_r, realContext);

      /* If this distribution doesn't take negative values, limit outselves to positive ones */
      if(f_nonnegative && realCompareLessThan(&r_r, const_0)) {
        realCopy(&r_z, &r_r), r_r.exponent -= 5;
      }

      /* If our upper and lower limits are close enough together we give up searching */
      // qf_newton_nosign
      if(!realIsSpecial(&r_high) && !realIsSpecial(&r_low) && WP34S_RelativeError(&r_low, &r_high, const_1e_37, realContext)) {
        // qf_newton_bounds_cnvg
        realAdd(&r_high, &r_low, &p, realContext);
        realMultiply(&p, const_1on2, &p, realContext);
        goto qf_newton_done2;
      }
      /* Check for convergence of the estimates */
      else { // qf_newton_nobounds
        if(f_discrete) { // qf_newton_dis_end
          /* Check for discrete distribution convergence */
          realSubtract(&r_z, &r_r, &p, realContext);
          realSetPositiveSign(&p);
          if(realCompareGreaterThan(const_1on10, &p)) {
            goto qf_newton_done;
          }
        }
        /* Check for continuous distribution convergence */
        else if(WP34S_RelativeError(&r_r, &r_z, const_1e_37, realContext)) {
          goto qf_newton_done;
        }
      }
      // qf_newton_again
    } while(--r_iterations > 0);
    realCopy(const_NaN, res);
    return;

    /* Finished.  For discrete distributions, there is a bit more to do */
  qf_newton_done:
    realCopy(&r_r, &p);
  qf_newton_done2:
    if(f_discrete) { // qf_discrete_exit
      realToIntegralValue(&p, &q, DEC_ROUND_FLOOR, realContext);
      WP34S_qf_discrete_final(r_dist - 1, &q, &r_p, p1, p2, p3, &p, realContext);
    }
    realCopy(&p, res);
    return;
  }

#endif //SAVE_SPACE_DM42_17
