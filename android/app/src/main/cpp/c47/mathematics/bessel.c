// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file bessel.c
 ***********************************************/

#include "c47.h"

void fnBesselJ(uint16_t unusedButMandatoryParameter) {
#if !defined(SAVE_SPACE_DM42_12BESSEL)
  real_t x, n, r, a;

  if(!saveLastX()) {
    return;
  }

 if(getRegisterAsReal(REGISTER_X, &x) && getRegisterAsReal(REGISTER_Y, &n)) {
    if(realIsAnInteger(&n) || (!realIsNegative(&x))) {
      WP34S_BesselJ(&n, &x, &r, &ctxtReal75);
      reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
      convertRealToReal34ResultRegister(&r, REGISTER_X);
    }
    else if(getSystemFlag(FLAG_CPXRES)) { // Real -> Complex
      realSetPositiveSign(&x);
      WP34S_BesselJ(&n, &x, &r, &ctxtReal75);
      WP34S_Mod(&n, const_2, &a, &ctxtReal75);
      realMultiply(&a, const_pi, &a, &ctxtReal75);
      realPolarToRectangular(&r, &a, &r, &a, &ctxtReal75);
      reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
      convertComplexToResultRegister(&r, &a, REGISTER_X);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function fnBesselJ:", "negative argument for Bessel function of non-integer degree", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }

  adjustResult(REGISTER_X, true, true, REGISTER_X, REGISTER_Y, -1);
#endif // !SAVE_SPACE_DM42_12BESSEL
}

void fnBesselY(uint16_t unusedButMandatoryParameter) {
#if !defined(SAVE_SPACE_DM42_12BESSEL)
  real_t x, n, r, a, b, c;

  if(!saveLastX()) {
    return;
  }

  if(getRegisterAsReal(REGISTER_X, &x) && getRegisterAsReal(REGISTER_Y, &n)) {
    if(!realIsNegative(&x)) {
      WP34S_BesselY(&n, &x, &r, &ctxtReal75);
      reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
      convertRealToReal34ResultRegister(&r, REGISTER_X);
    }
    else if(getSystemFlag(FLAG_CPXRES)) { // Real -> Complex
      realSetPositiveSign(&x);
      WP34S_BesselY(&n, &x, &r, &ctxtReal75);
      WP34S_Mod(&n, const_2, &a, &ctxtReal75);
      realMultiply(&a, const_pi, &b, &ctxtReal75);
      realMinus(&b, &a, &ctxtReal75);
      realPolarToRectangular(&r, &a, &r, &a, &ctxtReal75);

      WP34S_Cvt2RadSinCosTan(&b, amRadian, NULL, &b, NULL, &ctxtReal75);
      WP34S_BesselJ(&n, &x, &c, &ctxtReal75);
      realMultiply(&b, &c, &b, &ctxtReal75);
      realAdd(&b, &b, &b, &ctxtReal75);
      realAdd(&a, &b, &a, &ctxtReal75);

      reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
      convertComplexToResultRegister(&r, &a, REGISTER_X);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function fnBesselY:", "negative argument for Bessel function", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }

  adjustResult(REGISTER_X, true, true, REGISTER_X, REGISTER_Y, -1);
#endif // !SAVE_SPACE_DM42_12BESSEL
}



// Hankel's asymptotic expansion (based on Abramowitz and Stegun, p.364)
#if !defined(SAVE_SPACE_DM42_12BESSEL)
static void bessel_asymptotic_large_x(const real_t *alpha, const real_t *x, bool_t is_y, real_t *res, realContext_t *realContext) {
  real_t p, q, pp, qq, chi, sChi, cChi, mu, z8, k21, k21sq, nm, tmp;
  int32_t k;

  realMultiply(alpha, const_2, &chi, realContext);
  realAdd(&chi, const_1, &chi, realContext);
  realMultiply(&chi, const_piOn4, &chi, realContext);
  mod2Pi(x, &tmp, realContext);
  realSubtract(&tmp, &chi, &tmp, realContext);
  mod2Pi(&tmp, &chi, realContext);

  if(is_y) {
    WP34S_SinCosTanTaylor(&chi, false, &cChi, &sChi, NULL, realContext);
  }
  else {
    WP34S_SinCosTanTaylor(&chi, false, &sChi, &cChi, NULL, realContext);
  }

  realMultiply(alpha, alpha, &mu, realContext);
  realMultiply(&mu, const_4, &mu, realContext);

  realCopy(const_1, &p);
  realSubtract(&mu, const_1, &q, realContext);
  realMultiply(x, const_8, &z8, realContext);
  realDivide(&q, &z8, &q, realContext);
  realCopy(const_1, &k21);
  realCopy(&q, &nm), realCopy(const_1, &qq);
  for(k = 2; k < 1000; ++k) {
    #if !defined(TESTSUITE_BUILD)
      int32_t loop = k;
      if(monitorExit(&loop, "Iter: ")) {
        break;
      }
    #endif //!TESTSUITE_BUILD

    if(k % 2 == 0) {
      realCopy(&p, &pp);
    }
    else {
      realCopy(&q, &qq);
    }

    realAdd(&k21, const_2, &k21, realContext);
    realMultiply(&k21, &k21, &k21sq, realContext);
    realSubtract(&mu, &k21sq, &k21sq, realContext);
    realMultiply(&nm, &k21sq, &nm, realContext);
    realDivide(&nm, &z8, &nm, realContext);
    int32ToReal(k, &tmp);
    realDivide(&nm, &tmp, &nm, realContext);

    if(k % 4 < 2) {
      realAdd(     k % 2 ? &q : &p, &nm, k % 2 ? &q : &p, realContext);
    }
    else {
      realSubtract(k % 2 ? &q : &p, &nm, k % 2 ? &q : &p, realContext);
    }

    realCopy(const_1, &tmp), tmp.exponent -= 73;
    if(WP34S_RelativeError(&p, &pp, &tmp, realContext) && WP34S_RelativeError(&q, &qq, &tmp, realContext)) {
      break;
    }
  }

  realMultiply(&p, &cChi, &p, realContext);
  realMultiply(&q, &sChi, &q, realContext);
  if(is_y) {
    realAdd(&p, &q, &p, realContext);
  }
  else {
    realSubtract(&p, &q, &p, realContext);
  }

  realDivide(const_2, const_pi, &q, realContext);
  realDivide(&q, x, &q, realContext);
  realSquareRoot(&q, &q, realContext);
  realMultiply(&p, &q, res, realContext);
}



// Polynomial U[k] (based on Abramowitz and Stegun, p.366)
#define NUMBER_OF_COEFF   100
#define COEFF_BUFFER_SIZE_IN_BLOCKS (REAL_SIZE_IN_BLOCKS * NUMBER_OF_COEFF)
static void u_k(uint32_t k, const real_t *coeff/*array*/, const real_t *t_r, const real_t *t_i, real_t *res_r, real_t *res_i, realContext_t *realContext) {
  real_t t_n_r, t_n_i, tmp_r, tmp_i;
  uint32_t i;

  realCopy(t_r, &t_n_r), realCopy(t_i, &t_n_i);
  realCopy(const_0, res_r), realCopy(const_0, res_i);
  for(i = 1; i < k; ++i) {
    mulComplexComplex(&t_n_r, &t_n_i, t_r, t_i, &t_n_r, &t_n_i, realContext);
  }
  for(i = 0; i < NUMBER_OF_COEFF; ++i) {
    if(realIsZero(&coeff[i])) {
      break;
    }
    mulComplexComplex(&coeff[i], const_0, &t_n_r, &t_n_i, &tmp_r, &tmp_i, realContext);
    realAdd(res_r, &tmp_r, res_r, realContext), realAdd(res_i, &tmp_i, res_i, realContext);
    mulComplexComplex(&t_n_r, &t_n_i, t_r, t_i, &t_n_r, &t_n_i, realContext);
    mulComplexComplex(&t_n_r, &t_n_i, t_r, t_i, &t_n_r, &t_n_i, realContext);
  }
}


static void Sigma_u_k(const real_t *nu, const real_t *t_r, const real_t *t_i, int32_t odd, int32_t even, real_t *res_r, real_t *res_i, realContext_t *realContext) {
  real_t *coeff_current, *coeff_deriv, *coeff_next, *coeff_tmpptr = NULL;
  real_t nu_k, tmp, tmp2, prev_r, prev_i, coeff;
  uint32_t i, j;

  if((coeff_current = allocC47Blocks(COEFF_BUFFER_SIZE_IN_BLOCKS))) {
    if((coeff_deriv = allocC47Blocks(COEFF_BUFFER_SIZE_IN_BLOCKS))) {
      if((coeff_next = allocC47Blocks(COEFF_BUFFER_SIZE_IN_BLOCKS))) {
        realCopy(const_0, &prev_r), realCopy(const_0, &prev_i);
        realCopy(even ? const_1 : const_0, res_r), realCopy(const_0, res_i);
        realCopy(nu, &nu_k);

        int32ToReal(24, &tmp);
        realDivide(const_3, &tmp, &coeff_current[0], realContext);
        realDivide(const_5, &tmp, &coeff_current[1], realContext);
        realChangeSign(&coeff_current[1]);
        for(i = 2; i < NUMBER_OF_COEFF; ++i) {
          realZero(&coeff_current[i]);
        }

        for(i = 0; i < NUMBER_OF_COEFF; ++i) {
          realZero(&coeff_deriv[i]);
          realZero(&coeff_next[i]);
        }

        for(i = 1; i < NUMBER_OF_COEFF; ++i) {
         if(((i % 2 == 1) && odd) || ((i % 2 == 0) && even)) {
           u_k(i, coeff_current, t_r, t_i, &tmp, &tmp2, realContext);
           divComplexComplex(&tmp, &tmp2, &nu_k, const_0, &tmp, &tmp2, realContext);
           if((!realIsSpecial(&tmp)) && !realIsSpecial(&tmp2)) {
             int32ToReal((i % 2 == 1) ? odd : even, &coeff);
             realMultiply(&tmp, &coeff, &tmp, realContext), realMultiply(&tmp2, &coeff, &tmp2, realContext);
             realAdd(res_r, &tmp, res_r, realContext), realAdd(res_i, &tmp2, res_i, realContext);
           }
           realCopy(const_1, &tmp), tmp.exponent -= 73;
           if(WP34S_RelativeError(res_r, &prev_r, &tmp, realContext) && WP34S_RelativeError(res_i, &prev_i, &tmp, realContext)) {
             break;
           }
           realCopy(res_r, &prev_r), realCopy(res_i, &prev_i);
         }

         // for the next iteration
         realMultiply(&nu_k, nu, &nu_k, realContext);

         for(j = 0; j < NUMBER_OF_COEFF; ++j) { // coefficients of the derivative
           int32ToReal(i + j * 2, &tmp);
           realMultiply(&coeff_current[j], &tmp, &coeff_deriv[j], realContext);
           if(realIsZero(&coeff_deriv[j])) {
             break;
           }
         }
         for(j = 0; j < NUMBER_OF_COEFF; ++j) { // x^2
           realCopy(&coeff_deriv[j], &coeff_next[j]);
           if(realIsZero(&coeff_deriv[j])) {
             break;
           }
         }
         for(j = 1; j < NUMBER_OF_COEFF; ++j) { // -x^4
           realSubtract(&coeff_next[j], &coeff_deriv[j - 1], &coeff_next[j], realContext);
           if(realIsZero(&coeff_deriv[j - 1])) {
             break;
           }
         }
         for(j = 0; j < NUMBER_OF_COEFF; ++j) { // /2
           realMultiply(&coeff_next[j], const_1on2, &coeff_next[j], realContext);
           if(realIsZero(&coeff_next[j])) {
             break;
           }
         }

         for(j = 0; j < NUMBER_OF_COEFF; ++j) { // 5x
           realMultiply(&coeff_current[j], const_5, &coeff_deriv[j], realContext);
           if(realIsZero(&coeff_deriv[j])) {
             break;
           }
         }
         for(j = 1; j < NUMBER_OF_COEFF; ++j) { // (1-5t^2)x
           realSubtract(&coeff_current[j], &coeff_deriv[j - 1], &coeff_current[j], realContext);
           if(realIsZero(&coeff_deriv[j - 1])) {
             break;
           }
         }
         for(j = 0; j < NUMBER_OF_COEFF; ++j) { // integrate
           int32ToReal(i + j * 2 + 1, &tmp);
           realDivide(&coeff_current[j], &tmp, &coeff_deriv[j], realContext);
           if(realIsZero(&coeff_deriv[j])) {
             break;
           }
         }
         for(j = 0; j < NUMBER_OF_COEFF; ++j) { // 1/8
           realDivide(&coeff_deriv[j], const_8, &tmp, realContext);
           realAdd(&coeff_next[j], &tmp, &coeff_next[j], realContext);
           if(realIsZero(&coeff_deriv[j])) {
             break;
           }
         }

         coeff_tmpptr = coeff_current;
         coeff_current = coeff_next;
         coeff_next = coeff_tmpptr;
         coeff_tmpptr = NULL;
        }
        freeC47Blocks(coeff_next, COEFF_BUFFER_SIZE_IN_BLOCKS);
      }
      else {
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      }
      freeC47Blocks(coeff_deriv, COEFF_BUFFER_SIZE_IN_BLOCKS);
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
    }
    freeC47Blocks(coeff_current, COEFF_BUFFER_SIZE_IN_BLOCKS);
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
  }
  return;
}
#undef COEFF_BUFFER_SIZE_IN_BLOCKS
#undef NUMBER_OF_COEFF

// Debye's asymptotic expansion (based on Abramowitz and Stegun, p.366)
static void bessel_asymptotic_large_order_hyp(const real_t *nu, const real_t *x, bool_t is_y, real_t *res, realContext_t *realContext) {
  real_t alpha, tanh_alpha, coefficient, itrval, t, tmp;

  // nu * sech(alpha) = nu / cosh(alpha) = x
  realDivide(nu, x, &alpha, realContext);
  realArcosh(&alpha, &alpha, realContext);

  // coefficient numerator
  WP34S_Tanh(&alpha, &tanh_alpha, realContext);
  realSubtract(&tanh_alpha, &alpha, &coefficient, realContext);
  if(is_y) {
    realChangeSign(&coefficient);
  }
  realMultiply(nu, &coefficient, &coefficient, realContext);
  realExp(&coefficient, &coefficient, realContext);
  if(is_y) {
    realChangeSign(&coefficient);
  }

  // coefficient denominator
  realMultiply(is_y ? const_piOn2_75 : const_2pi_75, nu, &tmp, realContext);
  realMultiply(&tmp, &tanh_alpha, &tmp, realContext);
  realSquareRoot(&tmp, &tmp, realContext);
  realDivide(&coefficient, &tmp, &coefficient, realContext);

  realDivide(const_1, &tanh_alpha, &t, realContext);
  Sigma_u_k(nu, &t, const_0, is_y ? -1 : 1, 1, &itrval, &tmp, realContext);
  realMultiply(&coefficient, &itrval, res, realContext);
}

static void bessel_asymptotic_large_order_trig(const real_t *nu, const real_t *x, bool_t is_y, real_t *res, realContext_t *realContext) {
  real_t beta, sin_beta, cos_beta, tan_beta, cot_beta, coefficient, psi, sin_psi, cos_psi, lr, li, mr, mi;

  // nu * sec(beta) = nu / cos(beta) = x
  realDivide(nu, x, &cos_beta, realContext);
  WP34S_Acos(&cos_beta, &beta, realContext);

  realMultiply(&cos_beta, &cos_beta, &sin_beta, realContext); // cos²β
  realSubtract(const_1, &sin_beta, &sin_beta, realContext); // sin²β
  realSquareRoot(&sin_beta, &sin_beta, realContext); // sinβ
  realDivide(&sin_beta, &cos_beta, &tan_beta, realContext); // tanβ

  // coefficient
  realDivide(const_2, const_pi, &coefficient, realContext);
  realDivide(&coefficient, nu, &coefficient, realContext);
  realDivide(&coefficient, &tan_beta, &coefficient, realContext);
  realSquareRoot(&coefficient, &coefficient, realContext);

  // psi = nu * (tan(beta) - beta) - pi/4
  realSubtract(&tan_beta, &beta, &psi, realContext);
  realMultiply(&psi, nu, &psi, realContext);
  realSubtract(&psi, const_piOn4, &psi, realContext);
  mod2Pi(&psi, &psi, realContext);
  if(is_y) {
    WP34S_SinCosTanTaylor(&psi, false, &cos_psi, &sin_psi, NULL, realContext);
  }
  else {
    WP34S_SinCosTanTaylor(&psi, false, &sin_psi, &cos_psi, NULL, realContext);
  }

  realDivide(const_1, &tan_beta, &cot_beta, realContext);
  Sigma_u_k(nu, const_0, &cot_beta, 0, 1, &lr, &li, realContext);
  Sigma_u_k(nu, const_0, &cot_beta, 1, 0, &mr, &mi, realContext);
  mulComplexComplex(&mr, &mi, const_0, const__1, &mr, &mi, realContext);
  // li == mi == 0 ?

  realMultiply(&lr, &cos_psi, &lr, realContext);
  realMultiply(&mr, &sin_psi, &mr, realContext);
  if(is_y) {
    realChangeSign(&mr);
  }
  realAdd(&lr, &mr, res, realContext);
  realMultiply(res, &coefficient, res, realContext);
}



// Recurrence relations (Abramowitz and Stegun, p.361)
static void plusMinus(bool_t subtracting, const real_t *a, const real_t *b, real_t *res, realContext_t *realContext) {
  if(subtracting) {
    realSubtract(a, b, res, realContext);
  }
  else {
    realAdd(a, b, res, realContext);
  }
}
static void bessel_recur(const real_t *nu, const real_t *x, bool_t is_y, bool_t descending, real_t *res, realContext_t *realContext) {
  real_t jnx, jn_1x, alpha, floor_nu;
  #if !defined(TESTSUITE_BUILD)
    int32_t loop = 0;
  #endif //!TESTSUITE_BUILD

  realToIntegralValue(nu, &floor_nu, DEC_ROUND_FLOOR, realContext);
  plusMinus(!descending, nu, &floor_nu, &alpha, realContext);
  if(is_y) {
    WP34S_BesselY(&alpha, x, &jn_1x, realContext);
  }
  else {
    WP34S_BesselJ(&alpha, x, &jn_1x, realContext);
  }
  plusMinus(descending, &alpha, const_1, &alpha, realContext);
  if(is_y) {
    WP34S_BesselY(&alpha, x, &jnx, realContext);
  }
  else {
    WP34S_BesselJ(&alpha, x, &jnx, realContext);
  }
  while(realCompareLessThan(descending ? nu : &alpha, descending ? &alpha : nu)) {
    realMultiply(const_2, &alpha, res, realContext);
    realDivide(res, x, res, realContext);
    realMultiply(res, &jnx, res, realContext);
    realSubtract(res, &jn_1x, res, realContext);

    plusMinus(descending, &alpha, const_1, &alpha, realContext);
    realCopy(&jnx, &jn_1x);
    realCopy(res, &jnx);

    #if !defined(TESTSUITE_BUILD)
      if(monitorExit(&loop, "Iter: ")) {
        displayCalcErrorMessage(ERROR_SOLVER_ABORT, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
        return;
      }
    #endif //!TESTSUITE_BUILD
  }
}


// Digamma function (integer arguments only)
static void digamma(const real_t *x, real_t *res, realContext_t *realContext) {
  if(realIsAnInteger(x) && realCompareGreaterThan(x, const_0)) {
    real_t a, ar;
    realMinus(const_gammaEM, res, realContext);
    realCopy(const_1, &a);
    while(realCompareLessThan(&a, x)) {
      realDivide(const_1, &a, &ar, realContext);
      realAdd(res, &ar, res, realContext);
      realAdd(&a, const_1, &a, realContext);
    }
  }
  else {
    realCopy(const_NaN, res);
  }
}



/* The code below is ported from the WP 34s repository,
 * but never implemented in it. */
static void bessel(const real_t *alpha, const real_t *x, bool_t neg, real_t *res, realContext_t *realContext) {
  real_t q, r, m;
  real_t x2on4, term, gfac;
  int16_t n;

  realMultiply(x, const_1on2, &q, realContext);     // q = x/2
  realMultiply(&q, &q, &x2on4, realContext); // factor each time around
  realPower(&q, alpha, &r, realContext);       // (x/2)^(2m+alpha)

  realAdd(alpha, const_1, &gfac, realContext);
  WP34S_Gamma(&gfac, &q, realContext);
  realDivide(&r, &q, &term, realContext);
  realCopy(&term, res);                        // first term in series

  realZero(&m);

  for(n=0; n<1000; n++) {
    #if !defined(TESTSUITE_BUILD)
      int32_t loop = n;
      if(monitorExit(&loop, "Iter: ")) {
        break;
      }
    #endif //!TESTSUITE_BUILD

    realMultiply(&term, &x2on4, &q, realContext);
    realAdd(&m, const_1, &m, realContext);     // m = m+1
    realDivide(&q, &m, &r, realContext);
    realDivide(&r, &gfac, &term, realContext);
    realAdd(&gfac, const_1, &gfac, realContext);
    if(neg) {
      realChangeSign(&term);
    }
    realAdd(&term, res, &q, realContext);
    if(realCompareEqual(res, &q)) {
      return;
    }
    realCopy(&q, res);
  }
  realCopy(const_NaN, res);
  return;
}
#endif // !SAVE_SPACE_DM42_12BESSEL

void WP34S_BesselJ(const real_t *alpha, const real_t *x, real_t *res, realContext_t *realContext) {
#if !defined(SAVE_SPACE_DM42_12BESSEL)
  real_t a, beta, gamma;

  if(realIsNaN(alpha) || realIsSpecial(x)) {
    realCopy(const_NaN, res);
    return;
  }
  if(realIsZero(x)) {
    if(realIsZero(alpha)) {
      realCopy(const_1, res);
      return;
    }
    realZero(res);
    return;
  }
  realCopy(alpha, &a);
  if(realIsNegative(alpha) && realIsAnInteger(alpha)) {
    realSetPositiveSign(&a);
  }
  realSubtract(const_1, const_1on4, &beta, realContext);
  realMultiply(&a, &beta, &beta, realContext);
  realAdd(const_1, const_1on4, &gamma, realContext);
  realMultiply(&a, &gamma, &gamma, realContext);
  if(realCompareGreaterThan(&a, const_90)) {
    if(realCompareAbsLessThan(x, &beta)) {
      bessel_asymptotic_large_order_hyp(&a, x, false, res, realContext);
    }
    else if(realCompareAbsGreaterThan(x, &gamma)) {
      bessel_asymptotic_large_order_trig(&a, x, false, res, realContext);
    }
    else {
      bessel_recur(&a, x, false, realCompareAbsLessThan(x, &a), res, realContext);
    }
  }
  else if(realCompareAbsGreaterThan(x, const_90) && realCompareAbsGreaterThan(x, &gamma)) {
    bessel_asymptotic_large_x(&a, x, false, res, realContext);
  }
  else {
    bessel(&a, x, true, res, realContext);
  }
#endif // !SAVE_SPACE_DM42_12BESSEL
}

// See A&S page 360 section 9.1.11
#if !defined(SAVE_SPACE_DM42_12BESSEL)
static void bessel2_int_series(const real_t *n, const real_t *x, real_t *res, realContext_t *realContext) {
  real_t factor;
  real_t xon2, xon2n, x2on4;
  real_t k, npk, t, u, v, s, p, nf, absn;
  int32_t i, in, n_neg;
  #if !defined(TESTSUITE_BUILD)
    int32_t loop = 0;
  #endif //!TESTSUITE_BUILD

  realDivide(const__1, const_pi, &factor, realContext);

  if(realIsNegative(n)) {
    realCopyAbs(n, &absn);
    n = &absn;
    n_neg = 1;
  }
  else {
    n_neg = 0;
  }
  in = realToInt32C47(n);

  realMultiply(x, const_1on2, &xon2, realContext);      // xon2 = x/2
  realPower(&xon2, n, &xon2n, realContext);        // xon2n = (x/2)^n
  realMultiply(&xon2, &xon2, &x2on4, realContext); // x2on4 = +/- x^2/4

  if(in > 0) {
    realSubtract(n, const_1, &v, realContext);     // v = n-k-1 = n-1
    realZero(&k);
    WP34S_Gamma(n, &p, realContext);               // p = (n-1)!
    realCopy(&p, &s);
    realMultiply(&p, n, &nf, realContext);         // nf = n!  (for later)
    realCopy(const_1, &u);
    for(i = 1; i < in; i++) {
      realDivide(&p, &v, &t, realContext);
      realSubtract(&v, const_1, &v, realContext);
      realAdd(&k, const_1, &k, realContext);
      realDivide(&t, &k, &t, realContext);
      realMultiply(&t, &x2on4, &p, realContext);
      realAdd(&s, &p, &s, realContext);
    }
    realMultiply(&s, &factor, &t, realContext);
    realDivide(&t, &xon2n, res, realContext);
  }
  else {
    realZero(res);
    realCopy(const_1, &nf);
  }

  WP34S_BesselJ(n, x, &u, realContext);
  realDivide(&u, const_piOn2_75, &t, realContext);

  WP34S_Ln(&xon2, &u, realContext);
  realMultiply(&u, &t, &v, realContext);
  realAdd(res, &v, res, realContext);

  realChangeSign(&x2on4);
  realAdd(n, const_1, &t, realContext);            // t = n+1
  digamma(&t, &u, realContext);                    // u = Psi(n+1)
  realSubtract(&u, const_egamma, &v, realContext); // v = psi(k+1) + psi(n+k+1)
  realZero(&k);
  realCopy(n, &npk);
  realDivide(const_1, &nf, &p, realContext);       // p = (x^2/4)^k/(k!(n+k)!)
  realMultiply(&v, &p, &s, realContext);

  for(i = 0; i < 1000; i++) {
    realAdd(&k, const_1, &k, realContext);
    realAdd(&npk, const_1, &npk, realContext);
    realMultiply(&p, &x2on4, &t, realContext);
    realMultiply(&k, &npk, &u, realContext);
    realDivide(&t, &u, &p, realContext);

    realDivide(const_1, &k, &t, realContext);
    realAdd(&v, &t, &u, realContext);
    realDivide(const_1, &npk, &t, realContext);
    realAdd(&u, &t, &v, realContext);

    realMultiply(&v, &p, &t, realContext);
    realAdd(&t, &s, &u, realContext);
    if(realCompareEqual(&u, &s)) {
      break;
    }
    realCopy(&u, &s);

    #if !defined(TESTSUITE_BUILD)
      if(monitorExit(&loop, "Iter: ")) {
        displayCalcErrorMessage(ERROR_SOLVER_ABORT, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
        return;
      }
    #endif //!TESTSUITE_BUILD
  }
  realMultiply(&s, &xon2n, &t, realContext);

  realDivide(&t, const_pi, &u, realContext);

  realSubtract(res, &u, res, realContext);
  if(n_neg) {
    realChangeSign(res);
  }
}
#endif // !SAVE_SPACE_DM42_12BESSEL

void WP34S_BesselY(const real_t *alpha, const real_t *x, real_t *res, realContext_t *realContext) {
#if !defined(SAVE_SPACE_DM42_12BESSEL)
  real_t a, t, u, s, c, beta, gamma;

  if(realIsNaN(alpha) || realIsSpecial(x)) {
    realCopy(const_NaN, res);
    return;
  }
  else if(realIsZero(x)) {
    realCopy(const_minusInfinity, res);
    return;
  }
  else if(realIsInfinite(alpha) || realIsNegative(x)) {
    realCopy(const_NaN, res);
    return;
  }
  realCopy(alpha, &a);
  if(realIsNegative(alpha) && realIsAnInteger(alpha)) {
    realSetPositiveSign(&a);
  }
  realSubtract(const_1, const_1on4, &beta, realContext);
  realMultiply(&a, &beta, &beta, realContext);
  realAdd(const_1, const_1on4, &gamma, realContext);
  realMultiply(&a, &gamma, &gamma, realContext);
  if(!realIsAnInteger(&a)) {
    WP34S_Mod(&a, const_2, &t, realContext);
    realMultiply(&t, const_pi, &t, realContext);
    WP34S_Cvt2RadSinCosTan(&t, amRadian, &s, &c, NULL, realContext);
    WP34S_BesselJ(&a, x, &t, realContext);
    realMultiply(&t, &c, &u, realContext);
    realMinus(&a, &c, realContext);
    WP34S_BesselJ(&c, x, &t, realContext);
    realSubtract(&u, &t, &c, realContext);
    realDivide(&c, &s, res, realContext);
  }
  else if(realCompareGreaterThan(&a, const_90)) {
    if(realCompareAbsLessThan(x, &beta)) {
      bessel_asymptotic_large_order_hyp(&a, x, true, res, realContext);
    }
    else if(realCompareAbsGreaterThan(x, &gamma)) {
      bessel_asymptotic_large_order_trig(&a, x, true, res, realContext);
    }
    else {
      bessel_recur(&a, x, true, false, res, realContext);
    }
  }
  else if(realCompareAbsGreaterThan(x, const_90) && realCompareAbsGreaterThan(x, &gamma)) {
    bessel_asymptotic_large_x(alpha, x, true, res, realContext);
  }
  else {
    bessel2_int_series(alpha, x, res, realContext);
  }
#endif // !SAVE_SPACE_DM42_12BESSEL
}
