// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file bessel.c
 ***********************************************/

#include "c47.h"

void fnBesselJ(uint16_t unusedButMandatoryParameter) {
#if defined(OPTION_BESSEL)
  real_t x, n;
  cplx_t res;

  if(!saveLastX()) {
    return;
  }

 if(getRegisterAsReal(REGISTER_X, &x) && getRegisterAsReal(REGISTER_Y, &n)) {
    if(realIsAnInteger(&n) || (!realIsNegative(&x))) {
      WP34S_BesselJ(&n, &x, &res.Real, &ctxtReal75);
      reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
      convertRealToReal34ResultRegister(&res.Real, REGISTER_X);
    }
    else if(getSystemFlag(FLAG_CPXRES)) { // Real -> Complex
      realSetPositiveSign(&x);
      WP34S_BesselJ(&n, &x, &res.Mag, &ctxtReal75);
      WP34S_Mod(&n, const_2, &res.Arg, &ctxtReal75);
      realMultiply(&res.Arg, const75_pi, &res.Arg, &ctxtReal75);
      realPolarToRectangular(&res.Mag, &res.Arg, &res, &ctxtReal75);
      reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
      convertComplexToResultRegister(&res, REGISTER_X);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function fnBesselJ:", "negative argument for Bessel function of non-integer degree", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }

  adjustResult(REGISTER_X, true, true, REGISTER_X, REGISTER_Y, -1);
#endif // OPTION_BESSEL
}

void fnBesselY(uint16_t unusedButMandatoryParameter) {
#if defined(OPTION_BESSEL)
  real_t x, n, b, c;
  cplx_t res;

  if(!saveLastX()) {
    return;
  }

  if(getRegisterAsReal(REGISTER_X, &x) && getRegisterAsReal(REGISTER_Y, &n)) {
    if(!realIsNegative(&x)) {
      WP34S_BesselY(&n, &x, &res.Real, &ctxtReal75);
      reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
      convertRealToReal34ResultRegister(&res.Real, REGISTER_X);
    }
    else if(getSystemFlag(FLAG_CPXRES)) { // Real -> Complex
      realSetPositiveSign(&x);
      WP34S_BesselY(&n, &x, &res.Mag, &ctxtReal75);
      WP34S_Mod(&n, const_2, &res.Arg, &ctxtReal75);
      realMultiply(&res.Arg, const75_pi, &b, &ctxtReal75);
      realMinus(&b, &res.Arg, &ctxtReal75);
      realPolarToRectangular(&res.Mag, &res.Arg, &res, &ctxtReal75);

      C47_WP34S_Cvt2RadSinCosTan(&b, amRadian, NULL, &b, NULL, &ctxtReal75);
      WP34S_BesselJ(&n, &x, &c, &ctxtReal75);
      realMultiply(&b, &c, &b, &ctxtReal75);
      realAdd(&b, &b, &b, &ctxtReal75);
      realAdd(&res.Imag, &b, &res.Imag, &ctxtReal75);

      reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
      convertComplexToResultRegister(&res, REGISTER_X);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function fnBesselY:", "negative argument for Bessel function", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }

  adjustResult(REGISTER_X, true, true, REGISTER_X, REGISTER_Y, -1);
#endif // OPTION_BESSEL
}



// Hankel's asymptotic expansion (based on Abramowitz and Stegun, p.364)
#if defined(OPTION_BESSEL)
static void bessel_asymptotic_large_x(const real_t *alpha, const real_t *x, bool_t is_y, real_t *res, realContext_t *realContext) {
  real_t p, q, pp, qq, chi, sChi, cChi, mu, z8, k21, k21sq, nm, tmp;
  int32_t k;

  realMultiply(alpha, const_2, &chi, realContext);
  realAdd(&chi, const_1, &chi, realContext);
  realMultiply(&chi, const75_piOn4, &chi, realContext);
  mod2Pi(x, &tmp, realContext);
  realSubtract(&tmp, &chi, &tmp, realContext);
  mod2Pi(&tmp, &chi, realContext);

  if(is_y) {
    C47_WP34S_SinCosTanTaylor(&chi, false, &cChi, &sChi, NULL, realContext);
  }
  else {
    C47_WP34S_SinCosTanTaylor(&chi, false, &sChi, &cChi, NULL, realContext);
  }

  realMultiply(alpha, alpha, &mu, realContext);
  realMultiply(&mu, const_4, &mu, realContext);

  realSetOne(&p);
  realSubtract(&mu, const_1, &q, realContext);
  realMultiply(x, const_8, &z8, realContext);
  realDivide(&q, &z8, &q, realContext);
  realSetOne(&k21);
  realCopy(&q, &nm);
  realSetOne(&qq);
  for(k = 2; k < 1000; ++k) {
    int32_t loop = k;
    if(monitorExit(&loop, "Iter: ")) {
      break;
    }

    if(k % 2 == 0) {
      realCopy(&p, &pp);
    }
    else {
      realCopy(&q, &qq);
    }

    realAdd(&k21, const_2, &k21, realContext);
    realMultiply(&k21, &k21, &k21sq, realContext);
    realSubtract(&mu, &k21sq, &k21sq, realContext);
    int32ToReal(k, &tmp);
    realMultiply(&tmp, &z8, &tmp, realContext);
    // The series is asymptotic. Once (2k - 1)^2 exceeds mu, the ratio |mu - (2k - 1)^2| / (z8 k) of a term to the one before it rises with k, so the first term
    // larger than the one before it comes right after the smallest term, and the sum diverges from there.
    if(realIsNegative(&k21sq) && realCompareAbsGreaterThan(&k21sq, &tmp)) {
      break;
    }
    realMultiply(&nm, &k21sq, &nm, realContext);
    realDivide(&nm, &tmp, &nm, realContext);

    if(k % 4 < 2) {
      realAdd(     k % 2 ? &q : &p, &nm, k % 2 ? &q : &p, realContext);
    }
    else {
      realSubtract(k % 2 ? &q : &p, &nm, k % 2 ? &q : &p, realContext);
    }

    realSetOne(&tmp);
    tmp.exponent -= 73;
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

  realDivide(const_2, const75_pi, &q, realContext);
  realDivide(&q, x, &q, realContext);
  realSquareRoot(&q, &q, realContext);
  realMultiply(&p, &q, res, realContext);
}



// Polynomial U[k] (based on Abramowitz and Stegun, p.366)
#define NUMBER_OF_COEFF   100
#define COEFF_BUFFER_SIZE_IN_BLOCKS (REAL_SIZE_IN_BLOCKS(75) * NUMBER_OF_COEFF)
static void u_k(uint32_t k, const real_t *coeff/*array*/, const cplx_t *t, cplx_t *res, realContext_t *realContext) {
  cplx_t t_n, tmp;
  uint32_t i;

  realCopy(&t->Real, &t_n.Real);
  realCopy(&t->Imag, &t_n.Imag);
  realSetZero(&res->Real);
  realSetZero(&res->Imag);
  for(i = 1; i < k; ++i) {
    mulComplexComplex(&t_n, t, &t_n, realContext);
  }
  for(i = 0; i < NUMBER_OF_COEFF; ++i) {
    if(realIsZero(&coeff[i])) {
      break;
    }
    realCopy(&coeff[i], &tmp.Real);
    realSetZero(&tmp.Imag);
    mulComplexComplex(&tmp, &t_n, &tmp, realContext);
    realAdd(&res->Real, &tmp.Real, &res->Real, realContext), realAdd(&res->Imag, &tmp.Imag, &res->Imag, realContext);
    mulComplexComplex(&t_n, t, &t_n, realContext);
    mulComplexComplex(&t_n, t, &t_n, realContext);
  }
}


static void Sigma_u_k(const real_t *nu, const cplx_t *t, int32_t odd, int32_t even, cplx_t *res, realContext_t *realContext) {
  real_t *coeff_current, *coeff_deriv, *coeff_next, *coeff_tmpptr = NULL;
  real_t prev_r, prev_i, coeff;
  cplx_t nu_k, tmp;
  uint32_t i, j;

  if((coeff_current = allocC47Blocks(COEFF_BUFFER_SIZE_IN_BLOCKS))) {
    if((coeff_deriv = allocC47Blocks(COEFF_BUFFER_SIZE_IN_BLOCKS))) {
      if((coeff_next = allocC47Blocks(COEFF_BUFFER_SIZE_IN_BLOCKS))) {
        realSetZero(&prev_r);
        realSetZero(&prev_i);
        realCopy(even ? const_1 : const_0, &res->Real);
        realSetZero(&res->Imag);
        realCopy(nu, &nu_k.Real);
        realSetZero(&nu_k.Imag);

        realDivide(const_3, const_24, &coeff_current[0], realContext);
        realDivide(const_5, const_24, &coeff_current[1], realContext);
        realChangeSign(&coeff_current[1]);
        for(i = 2; i < NUMBER_OF_COEFF; ++i) {
          realSetZero(&coeff_current[i]);
        }

        for(i = 0; i < NUMBER_OF_COEFF; ++i) {
          realSetZero(&coeff_deriv[i]);
          realSetZero(&coeff_next[i]);
        }

        for(i = 1; i < NUMBER_OF_COEFF; ++i) {
         if(((i % 2 == 1) && odd) || ((i % 2 == 0) && even)) {
           u_k(i, coeff_current, t, &tmp, realContext);
           divComplexComplex(&tmp, &nu_k, &tmp, realContext);
           if((!realIsSpecial(&tmp.Real)) && !realIsSpecial(&tmp.Imag)) {
             int32ToReal((i % 2 == 1) ? odd : even, &coeff);
             realMultiply(&tmp.Real, &coeff, &tmp.Real, realContext);
             realMultiply(&tmp.Imag, &coeff, &tmp.Imag, realContext);
             realAdd(&res->Real, &tmp.Real, &res->Real, realContext);
             realAdd(&res->Imag, &tmp.Imag, &res->Imag, realContext);
           }
           realSetOne(&tmp.Real);
           tmp.Real.exponent -= 73;
           if(WP34S_RelativeError(&res->Real, &prev_r, &tmp.Real, realContext) && WP34S_RelativeError(&res->Imag, &prev_i, &tmp.Real, realContext)) {
             break;
           }
           realCopy(&res->Real, &prev_r);
           realCopy(&res->Imag, &prev_i);
         }

         // for the next iteration
         realMultiply(&nu_k.Real, nu, &nu_k.Real, realContext);

         for(j = 0; j < NUMBER_OF_COEFF; ++j) { // coefficients of the derivative
           int32ToReal(i + j * 2, &tmp.Real);
           realMultiply(&coeff_current[j], &tmp.Real, &coeff_deriv[j], realContext);
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
           int32ToReal(i + j * 2 + 1, &tmp.Real);
           realDivide(&coeff_current[j], &tmp.Real, &coeff_deriv[j], realContext);
           if(realIsZero(&coeff_deriv[j])) {
             break;
           }
         }
         for(j = 0; j < NUMBER_OF_COEFF; ++j) { // 1/8
           realDivide(&coeff_deriv[j], const_8, &tmp.Real, realContext);
           realAdd(&coeff_next[j], &tmp.Real, &coeff_next[j], realContext);
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
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      }
      freeC47Blocks(coeff_deriv, COEFF_BUFFER_SIZE_IN_BLOCKS);
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    }
    freeC47Blocks(coeff_current, COEFF_BUFFER_SIZE_IN_BLOCKS);
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
  }
  return;
}
#undef COEFF_BUFFER_SIZE_IN_BLOCKS
#undef NUMBER_OF_COEFF

// Debye's asymptotic expansion (based on Abramowitz and Stegun, p.366)
// Its terms shrink to about exp(-2 nu xi) before they grow again, with xi = alpha - tanh(alpha) here and tan(beta) - beta in the trigonometric form.
// Below nu xi = 45 the smallest term is above 1E-39, and both forms return false and leave res unset.
static bool_t bessel_asymptotic_large_order_hyp(const real_t *nu, const real_t *x, bool_t is_y, real_t *res, realContext_t *realContext) {
  real_t alpha, tanh_alpha, coefficient, tmp;
  cplx_t itrval, t;

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
  if(realCompareAbsLessThan(&coefficient, const_45)) {
    return false;
  }
  realExp(&coefficient, &coefficient, realContext);
  if(is_y) {
    realChangeSign(&coefficient);
  }

  // coefficient denominator
  realMultiply(is_y ? const75_piOn2 : const75_2pi, nu, &tmp, realContext);
  realMultiply(&tmp, &tanh_alpha, &tmp, realContext);
  realSquareRoot(&tmp, &tmp, realContext);
  realDivide(&coefficient, &tmp, &coefficient, realContext);

  realDivide(const_1, &tanh_alpha, &t.Real, realContext);
  realSetZero(&t.Imag);
  Sigma_u_k(nu, &t, is_y ? -1 : 1, 1, &itrval, realContext);
  realMultiply(&coefficient, &itrval.Real, res, realContext);
  return true;
}

static bool_t bessel_asymptotic_large_order_trig(const real_t *nu, const real_t *x, bool_t is_y, real_t *res, realContext_t *realContext) {
  real_t beta, sin_beta, cos_beta, tan_beta, coefficient, psi, sin_psi, cos_psi;
  cplx_t t, l, m;

  // nu * sec(beta) = nu / cos(beta) = x
  realDivide(nu, x, &cos_beta, realContext);
  C47_WP34S_Acos(&cos_beta, &beta, realContext);

  realMultiply(&cos_beta, &cos_beta, &sin_beta, realContext); // cos²β
  realSubtract(const_1, &sin_beta, &sin_beta, realContext); // sin²β
  realSquareRoot(&sin_beta, &sin_beta, realContext); // sinβ
  realDivide(&sin_beta, &cos_beta, &tan_beta, realContext); // tanβ

  // coefficient
  realDivide(const_2, const75_pi, &coefficient, realContext);
  realDivide(&coefficient, nu, &coefficient, realContext);
  realDivide(&coefficient, &tan_beta, &coefficient, realContext);
  realSquareRoot(&coefficient, &coefficient, realContext);

  // psi = nu * (tan(beta) - beta) - pi/4
  realSubtract(&tan_beta, &beta, &psi, realContext);
  realMultiply(&psi, nu, &psi, realContext);
  if(realCompareLessThan(&psi, const_45)) {
    return false;
  }
  realSubtract(&psi, const75_piOn4, &psi, realContext);
  mod2Pi(&psi, &psi, realContext);
  if(is_y) {
    C47_WP34S_SinCosTanTaylor(&psi, false, &cos_psi, &sin_psi, NULL, realContext);
  }
  else {
    C47_WP34S_SinCosTanTaylor(&psi, false, &sin_psi, &cos_psi, NULL, realContext);
  }

  realSetZero(&t.Real);
  realDivide(const_1, &tan_beta, &t.Imag, realContext);        // t = i.cot(beta)
  Sigma_u_k(nu, &t, 0, 1, &l, realContext);
  Sigma_u_k(nu, &t, 1, 0, &m, realContext);
  realCopy(const__1, &t.Imag);                                  // t = -i
  mulComplexComplex(&m, &t, &m, realContext);
  // l.Imag == m.Imag == 0 ?

  realMultiply(&l.Real, &cos_psi, &l.Real, realContext);
  realMultiply(&m.Real, &sin_psi, &m.Real, realContext);
  if(is_y) {
    realChangeSign(&m.Real);
  }
  realAdd(&l.Real, &m.Real, res, realContext);
  realMultiply(res, &coefficient, res, realContext);
  return true;
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
  int32_t loop = 0;

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

    if(monitorExit(&loop, "Iter: ")) {
      displayCalcErrorMessage(ERROR_SOLVER_ABORT, ERR_REGISTER_LINE);
      return;
    }
  }
}


// Digamma function (integer arguments only)
static void digamma(const real_t *x, real_t *res, realContext_t *realContext) {
  if(realIsAnInteger(x) && realCompareGreaterThan(x, const_0)) {
    real_t a, ar;
    realMinus(const75_egamma, res, realContext);
    realSetOne(&a);
    while(realCompareLessThan(&a, x)) {
      realDivide(const_1, &a, &ar, realContext);
      realAdd(res, &ar, res, realContext);
      realAdd(&a, const_1, &a, realContext);
    }
  }
  else {
    realSetNaN(res);
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

  realSetZero(&m);

  for(n=0; n<1000; n++) {
    int32_t loop = n;
    if(monitorExit(&loop, "Iter: ")) {
      break;
    }

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
  realSetNaN(res);
  return;
}
#endif // OPTION_BESSEL

#if defined(OPTION_BESSEL)
/**
 * Copies the order alpha to a, without its sign for an integer order, and returns whether alpha is an odd integer: J and Y of order -n are (-1)^n times those of order n.
 */
static bool_t bessel_order(const real_t *alpha, real_t *a, realContext_t *realContext) {
  real_t parity;

  realCopy(alpha, a);
  if(!realIsAnInteger(alpha)) {
    return false;
  }
  realSetPositiveSign(a);
  realDivideRemainder(alpha, const_2, &parity, realContext);
  return !realIsZero(&parity);
}

/**
 * The routes for an order a above 90, and for x above 90 and above 1.25 a: Debye's expansions, the recurrence and Hankel's expansion. Returns false, with res unset,
 * where none of them applies and the caller takes its power series.
 */
static bool_t bessel_large(const real_t *a, const real_t *x, bool_t is_y, real_t *res, realContext_t *realContext) {
  real_t beta, gamma;

  realSubtract(const_1, const_1on4, &beta, realContext);
  realMultiply(a, &beta, &beta, realContext);
  realAdd(const_1, const_1on4, &gamma, realContext);
  realMultiply(a, &gamma, &gamma, realContext);
  if(realCompareGreaterThan(a, const_90)) {
    bool_t recur = true;

    if(realCompareAbsLessThan(x, &beta)) {
      recur = !bessel_asymptotic_large_order_hyp(a, x, is_y, res, realContext);
    }
    else if(realCompareAbsGreaterThan(x, &gamma)) {
      recur = !bessel_asymptotic_large_order_trig(a, x, is_y, res, realContext);
    }
    if(recur) {
      bessel_recur(a, x, is_y, !is_y && realCompareAbsLessThan(x, a), res, realContext);
    }
    return true;
  }
  if(realCompareAbsGreaterThan(x, const_90) && realCompareAbsGreaterThan(x, &gamma)) {
    bessel_asymptotic_large_x(a, x, is_y, res, realContext);
    return true;
  }
  return false;
}

void WP34S_BesselJ(const real_t *alpha, const real_t *x, real_t *res, realContext_t *realContext) {
  real_t a, absx;
  bool_t odd, negate;

  if(realIsSpecial(alpha) || realIsNaN(x)) {
    realSetNaN(res);
    return;
  }
  if(realIsInfinite(x)) {
    realSetZero(res);
    return;
  }
  odd = bessel_order(alpha, &a, realContext);
  negate = odd && realIsNegative(alpha);
  if(realIsZero(x)) {
    if(realIsZero(alpha)) {
      realSetOne(res);
      return;
    }
    realSetZero(res);
    if(negate) {
      realChangeSign(res);
    }
    return;
  }
  if(realIsNegative(x) && realIsAnInteger(alpha)) {
    // J(n, -x) = (-1)^n J(n, x)
    negate = (negate != odd);
    realCopyAbs(x, &absx);
    x = &absx;
  }
  if(!bessel_large(&a, x, false, res, realContext)) {
    bessel(&a, x, true, res, realContext);
  }
  if(negate) {
    realChangeSign(res);
  }
}
#endif // OPTION_BESSEL

// See A&S page 360 section 9.1.11
#if defined(OPTION_BESSEL)
static void bessel2_int_series(const real_t *n, const real_t *x, real_t *res, realContext_t *realContext) {
  real_t factor;
  real_t xon2, xon2n, x2on4;
  real_t k, npk, t, u, v, s, p, nf, absn;
  int32_t i, in, n_neg;
  int32_t loop = 0;

  realDivide(const__1, const75_pi, &factor, realContext);

  if(realIsNegative(n)) {
    realCopyAbs(n, &absn);
    n = &absn;
    n_neg = 1;
  }
  else {
    n_neg = 0;
  }
  in = realToInt32C47(n, NULL);

  realMultiply(x, const_1on2, &xon2, realContext); // xon2 = x/2
  realPower(&xon2, n, &xon2n, realContext);        // xon2n = (x/2)^n
  realMultiply(&xon2, &xon2, &x2on4, realContext); // x2on4 = +/- x^2/4

  if(in > 0) {
    realSubtract(n, const_1, &v, realContext);     // v = n-k-1 = n-1
    realSetZero(&k);
    WP34S_Gamma(n, &p, realContext);               // p = (n-1)!
    realCopy(&p, &s);
    realMultiply(&p, n, &nf, realContext);         // nf = n!  (for later)
    realSetOne(&u);
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
    realSetZero(res);
    realSetOne(&nf);
  }

  WP34S_BesselJ(n, x, &u, realContext);
  realDivide(&u, const75_piOn2, &t, realContext);

  WP34S_Ln(&xon2, &u, realContext);
  realMultiply(&u, &t, &v, realContext);
  realAdd(res, &v, res, realContext);

  realChangeSign(&x2on4);
  realAdd(n, const_1, &t, realContext);              // t = n+1
  digamma(&t, &u, realContext);                      // u = Psi(n+1)
  realSubtract(&u, const75_egamma, &v, realContext); // v = psi(k+1) + psi(n+k+1)
  realSetZero(&k);
  realCopy(n, &npk);
  realDivide(const_1, &nf, &p, realContext);         // p = (x^2/4)^k/(k!(n+k)!)
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

    if(monitorExit(&loop, "Iter: ")) {
      displayCalcErrorMessage(ERROR_SOLVER_ABORT, ERR_REGISTER_LINE);
      return;
    }
  }
  realMultiply(&s, &xon2n, &t, realContext);

  realDivide(&t, const75_pi, &u, realContext);

  realSubtract(res, &u, res, realContext);
  if(n_neg) {
    realChangeSign(res);
  }
}
#endif // OPTION_BESSEL

#if defined(OPTION_BESSEL)
void WP34S_BesselY(const real_t *alpha, const real_t *x, real_t *res, realContext_t *realContext) {
  real_t a, t, u, s, c;
  bool_t negate;

  if(realIsNaN(alpha) || realIsNaN(x)) {
    realSetNaN(res);
    return;
  }
  if(realIsInfinite(x)) {
    realSetZero(res);
    return;
  }
  negate = bessel_order(alpha, &a, realContext) && realIsNegative(alpha);
  if(realIsZero(x)) {
    realSetMinusInfinity(res);
    if(negate) {
      realChangeSign(res);
    }
    return;
  }
  else if(realIsInfinite(alpha) || realIsNegative(x)) {
    realSetNaN(res);
    return;
  }
  if(!realIsAnInteger(&a)) {
    WP34S_Mod(&a, const_2, &t, realContext);
    realMultiply(&t, const75_pi, &t, realContext);
    C47_WP34S_Cvt2RadSinCosTan(&t, amRadian, &s, &c, NULL, realContext);
    WP34S_BesselJ(&a, x, &t, realContext);
    realMultiply(&t, &c, &u, realContext);
    realMinus(&a, &c, realContext);
    WP34S_BesselJ(&c, x, &t, realContext);
    realSubtract(&u, &t, &c, realContext);
    realDivide(&c, &s, res, realContext);
  }
  else if(!bessel_large(&a, x, true, res, realContext)) {
    bessel2_int_series(&a, x, res, realContext);
  }
  if(negate) {
    realChangeSign(res);
  }
}
#endif // OPTION_BESSEL
