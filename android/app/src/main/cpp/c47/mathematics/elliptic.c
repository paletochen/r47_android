// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file elliptic.c
 ***********************************************/

#include "c47.h"

#define ELLIPTIC_N 16

static void _calc_real_elliptic(real_t *sn, real_t *cn, real_t *dn, const real_t *u, const real_t *m, realContext_t *realContext) {
  real_t a, b, e, f, g;
  real_t *MU, *NU, *C, *D;
  real_t sin_umu, cos_umu, t, r;
  int n = 0;

  if((MU = allocC47Blocks(ELLIPTIC_N * REAL_SIZE_IN_BLOCKS))) {
    if((NU = allocC47Blocks(ELLIPTIC_N * REAL_SIZE_IN_BLOCKS))) {
      if((C = allocC47Blocks(ELLIPTIC_N * REAL_SIZE_IN_BLOCKS))) {
        if((D = allocC47Blocks(ELLIPTIC_N * REAL_SIZE_IN_BLOCKS))) {
          #define mu(n) (MU + (n))
          #define nu(n) (NU + (n))
          #define c(n)  (C + (n))
          #define d(n)  (D + (n))

          if(realIsNegative(m) || realCompareLessThan(const_1, m)) {
            realCopy(const_NaN, sn);
            realCopy(const_NaN, cn);
            realCopy(const_NaN, dn);
            freeC47Blocks(MU, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
            freeC47Blocks(NU, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
            freeC47Blocks(C, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
            freeC47Blocks(D, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
            return;
          }
          if(realCompareLessThan(m, const_1e_32)) {
            WP34S_Cvt2RadSinCosTan(u, amRadian, sn, cn, NULL, realContext);
            realCopy(const_1, dn);
            freeC47Blocks(MU, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
            freeC47Blocks(NU, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
            freeC47Blocks(C, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
            freeC47Blocks(D, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
            return;
          }
          realSubtract(m, const_1, &a, realContext);
          if(realCompareAbsLessThan(&a, const_1e_32)) {
            WP34S_SinhCosh(u, &a, &b, realContext);
            realDivide(const_1, &b, cn, realContext);
            realMultiply(&a, cn, sn, realContext);
            realCopy(cn, dn);
            freeC47Blocks(MU, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
            freeC47Blocks(NU, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
            freeC47Blocks(C, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
            freeC47Blocks(D, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
            return;
          }

          realSubtract(const_1, m, &a, realContext);
          realSquareRoot(&a, &a, realContext);
          n = realAgmStep(const_1, &a, &b, MU, NU, ELLIPTIC_N, realContext);
          realSubtract(mu(n-1), nu(n-1), &e, realContext);

          realMultiply(u, mu(n), &a, realContext);
          WP34S_Cvt2RadSinCosTan(&a, amRadian, &sin_umu, &cos_umu, NULL, realContext);
          realCopyAbs(&cos_umu, &b);
          //if(realCompareAbsLessThan(&sin_umu, &b))
          //  realDivide(&sin_umu, &cos_umu, &t, realContext);
          //else
            realDivide(&cos_umu, &sin_umu, &t, realContext);
          if(realIsZero(&sin_umu)) {
            realCopy(const_0, sn);
            realCopy(const_1, cn);
            realCopy(const_1, dn);
            freeC47Blocks(MU, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
            freeC47Blocks(NU, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
            freeC47Blocks(C, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
            freeC47Blocks(D, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
            return;
          }

          realMultiply(mu(n), &t, c(n), realContext);
          realCopy(const_1, d(n));

          while(n > 0) {
            n--;
            realMultiply(d(n+1), c(n+1), c(n), realContext);
            realMultiply(c(n+1), c(n+1), &a, realContext);
            realDivide(&a, mu(n+1), &r, realContext);
            realAdd(&r, nu(n), &a, realContext);
            realAdd(&r, mu(n), &b, realContext);
            realDivide(&a, &b, d(n), realContext);
          }
          complexMagnitude(const_1, c(0), &f, realContext);
          if(realIsNegative(&e)) {
            realSubtract(const_1, m, &a, realContext);
            realSquareRoot(&a, &g, realContext);
            realDivide(&g, d(0), dn, realContext);

            realDivide(dn, &f, cn, realContext);
            if(realIsNegative(&cos_umu)) {
              realChangeSign(cn);
            }

            realDivide(c(0), &g, &a, realContext);
            realMultiply(cn, &a, sn, realContext);
          }
          else {
            realCopy(d(0), dn);

            realDivide(const_1, &f, sn, realContext);
            if(realIsNegative(&sin_umu)) {
              realChangeSign(sn);
            }
            realMultiply(c(0), sn, cn, realContext);
          }

          #undef mu
          #undef nu
          #undef c
          #undef d

          freeC47Blocks(D, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
        }
        else {
          displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
          realCopy(const_NaN, sn); realCopy(const_NaN, cn); realCopy(const_NaN, dn);
        }
        freeC47Blocks(C, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
      }
      else {
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
        realCopy(const_NaN, sn); realCopy(const_NaN, cn); realCopy(const_NaN, dn);
      }
      freeC47Blocks(NU, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      realCopy(const_NaN, sn); realCopy(const_NaN, cn); realCopy(const_NaN, dn);
    }
    freeC47Blocks(MU, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
    realCopy(const_NaN, sn); realCopy(const_NaN, cn); realCopy(const_NaN, dn);
  }
}

static void calc_real_elliptic(real_t *sn, real_t *cn, real_t *dn, const real_t *u, const real_t *m, realContext_t *realContext) {
  real_t s_n, c_n, d_n;

  if(sn == NULL) {
    sn = &s_n;
  }
  if(cn == NULL) {
    cn = &c_n;
  }
  if(dn == NULL) {
    dn = &d_n;
  }

  if(realCompareLessThan(const_1, m)) {
    real_t k, uk, m_1;
    realSquareRoot(m, &k, realContext);
    realMultiply(&k, u, &uk, realContext);
    realDivide(const_1, m, &m_1, realContext);
    _calc_real_elliptic(sn, dn, cn, &uk, &m_1, realContext);
    realDivide(sn, &k, sn, realContext);
  }
  else if(realIsNegative(m)) {
    real_t mu, mu1, v, s, c, d;
    realSubtract(const_1, m, &mu1, realContext);
    realDivide(const_1, &mu1, &mu1, realContext);
    realMultiply(&mu1, m, &mu, realContext);
    realChangeSign(&mu);
    realSquareRoot(&mu1, &mu1, realContext);
    realDivide(u, &mu1, &v, realContext);
    _calc_real_elliptic(&s, &c, &d, &v, &mu, realContext);
    realDivide(&s, &d, sn, realContext);
    realDivide(&c, &d, cn, realContext);
    realDivide(const_1, &d, dn, realContext);
    realMultiply(sn, &mu1, sn, realContext);
  }
  else {
    _calc_real_elliptic(sn, cn, dn, u, m, realContext);
  }
}

static void elliptic_setup_cpx_real(real_t *r,
                                    real_t *snuk, real_t *cnuk, real_t *dnuk,
                                    real_t *snvki, real_t *cnvki, real_t *dnvki,
                                    const real_t *u, const real_t *v,
                                    const real_t *m, realContext_t *realContext) {
  real_t m_1;
  realSubtract(const_1, m, &m_1, realContext);

  calc_real_elliptic(snuk, cnuk, dnuk, u, m, realContext);
  calc_real_elliptic(snvki, cnvki, dnvki, v, &m_1, realContext);

  realMultiply(m, snuk, r, realContext);
  realMultiply(r, snuk, r, realContext);
  realMultiply(r, snvki, r, realContext);
  realMultiply(r, snvki, r, realContext);
  realFMA(cnvki, cnvki, r, r, realContext);
}

static int jacobi_check_inputs(real_t *m, real_t *uReal, real_t *uImag, bool_t *realInput) {
  bool_t cmplx = false;

  if (!getRegisterAsComplexOrReal(REGISTER_X, uReal, uImag, &cmplx)
          || !getRegisterAsReal(REGISTER_Y, m)
          || !saveLastX())
      return 0;
  *realInput = !cmplx;
  return 1;
}


void jacobiElliptic(const real_t *u, const real_t *m, real_t *am, real_t *sn, real_t *cn, real_t *dn, realContext_t *realContext) {
  real_t _sn;
  calc_real_elliptic(sn ? sn : &_sn, cn, dn, u, m, realContext);
  if(am) {
    WP34S_Asin(sn ? sn : &_sn, am, realContext);
  }
}

void jacobiComplexAm(const real_t *ur, const real_t *ui, const real_t *m, real_t *rr, real_t *ri, realContext_t *realContext) {
  jacobiComplexSn(ur, ui, m, rr, ri, realContext);
  ArcsinComplex(rr, ri, rr, ri, realContext);
}

void jacobiComplexSn(const real_t *ur, const real_t *ui, const real_t *m, real_t *rr, real_t *ri, realContext_t *realContext) {
  // SN(u + i v, m) = sn(u, m) . dn(v, 1-m) / denom
  //                  + i . cn(u, m) . dn(u, m) . sn(v, 1-m) . cn(v, 1-m) / denom
  // where
  //    denom = cn(v, 1-m)^2 + m * dn(u, m)^2 * sn(v, 1-m)^2

  real_t a, b, denom, snuk, cnuk, dnuk, snvki, cnvki, dnvki;
  elliptic_setup_cpx_real(&denom, &snuk, &cnuk, &dnuk, &snvki, &cnvki, &dnvki, ur, ui, m, realContext);

  realMultiply(&snuk, &dnvki, &a, realContext);
  realDivide(&a, &denom, rr, realContext);

  realMultiply(&cnuk, &dnuk, &a, realContext);
  realMultiply(&a, &snvki, &b, realContext);
  realMultiply(&b, &cnvki, &a, realContext);
  realDivide(&a, &denom, ri, realContext);
}

void jacobiComplexCn(const real_t *ur, const real_t *ui, const real_t *m, real_t *rr, real_t *ri, realContext_t *realContext) {
  // CN(u + i v, m) = cn(u, m) . cn(v, 1-m) / denom
  //                  - i . sn(u, m) . dn(u, m) . sn(v, 1-m) . dn(v, 1-m) / denom
  // where
  //    denom = cn(v, 1-m)^2 + m * dn(u, m)^2 * sn(v, 1-m)^2

  real_t a, b, denom, snuk, cnuk, dnuk, snvki, cnvki, dnvki;
  elliptic_setup_cpx_real(&denom, &snuk, &cnuk, &dnuk, &snvki, &cnvki, &dnvki, ur, ui, m, realContext);

  realMultiply(&cnuk, &cnvki, &a, realContext);
  realDivide(&a, &denom, rr, realContext);

  realMultiply(&snuk, &dnuk, &a, realContext);
  realMultiply(&a, &snvki, &b, realContext);
  realMultiply(&b, &dnvki, &a, realContext);
  realDivide(&a, &denom, &b, realContext);
  realMinus(&b, ri, realContext);
}

void jacobiComplexDn(const real_t *ur, const real_t *ui, const real_t *m, real_t *rr, real_t *ri, realContext_t *realContext) {
  // DN(u + i v, m) = dn(u, m) . cn(v, 1-m) . dn(v, 1-m) / denom
  //                  - i . m . sn(u, m) . cn(u, m) . sn(v, 1-m) / denom
  // where
  //    denom = cn(v, 1-m)^2 + m * dn(u, m)^2 * sn(v, 1-m)^2

  real_t a, b, denom, snuk, cnuk, dnuk, snvki, cnvki, dnvki;
  elliptic_setup_cpx_real(&denom, &snuk, &cnuk, &dnuk, &snvki, &cnvki, &dnvki, ur, ui, m, realContext);

  realMultiply(&dnuk, &cnvki, &a, realContext);
  realMultiply(&a, &dnvki, &b, realContext);
  realDivide(&b, &denom, rr, realContext);

  realMinus(m, &b, realContext);
  realMultiply(&b, &snuk, &a, realContext);
  realMultiply(&a, &cnuk, &b, realContext);
  realMultiply(&b, &snvki, &a, realContext);
  realDivide(&a, &denom, ri, realContext);
}

void ellipticKE(const real_t *m, real_t *k, real_t *ki, real_t *e, real_t *ei, realContext_t *realContext) {
  real_t a, b, cr, ci;

  if(k) {
    realCopy(const_NaN, k);
  }
  if(ki) {
    realZero(ki);
  }
  if(e) {
    realCopy(const_NaN, e);
  }
  if(ei) {
    realZero(ei);
  }

  if(realCompareLessEqual(m, const_1)) {
    if(realCompareEqual(m, const_1)) {
      if(k) {
        realCopy(const_plusInfinity, k);
      }
      if(e) {
        realCopy(const_1, e);
      }
    }
    else if(realCompareEqual(m, const_0)) {
      if(k) {
        realCopy(const_piOn2, k);
      }
      if(e) {
        realCopy(const_piOn2, e);
      }
    }
    else {
      realSubtract(const_1, m, &b, &ctxtReal39);
      realSquareRoot(&b, &b, &ctxtReal39);
      realCopy(m, &a);
      realAgmForE(const_1, &b, &a, &b, &ctxtReal39);
      realDivide(const_piOn2, &b, &b, &ctxtReal39);
      if(k) {
        realCopy(&b, k);
      }

      if(e) {
        realSubtract(&a, const_1, &a, &ctxtReal39);
        realChangeSign(&a);
        realMultiply(&a, &b, &b, &ctxtReal39);
        realCopy(&b, e);
      }
    }
  }
  else if(ki || ei) {
    if(k) {
      realCopy(const_NaN, k);
    }
    if(e) {
      realCopy(const_NaN, e);
    }

    realSubtract(m, const_1, &b, realContext);
    realSquareRoot(&b, &b, realContext);
    realCopy(m, &cr); realZero(&ci);
    complexAgmForE(const_1, const_0, const_0, &b, &cr, &ci, &a, &b, realContext);
    divRealComplex(const_piOn2, &a, &b, &a, &b, realContext);
    if(k && ki) {
      realCopy(&a, k); realCopy(&b, ki);
    }

    if(e && ei) {
      realSubtract(&cr, const_1, &cr, realContext);
      realChangeSign(&cr); realChangeSign(&ci);
      mulComplexComplex(&a, &b, &cr, &ci, &a, &b, realContext);
      realCopy(&a, e); realCopy(&b, ei);
    }
  }
}

static void rcSqrt(const real_t *x, real_t *r, real_t *i, realContext_t *realContext) {
  real_t xx;

  realCopy(x, &xx);
  if(realIsNegative(&xx)) {
    realSetPositiveSign(&xx);
    realSquareRoot(&xx, i, realContext);
    realZero(r);
  }
  else {
    realSquareRoot(&xx, r, realContext);
    realZero(i);
  }
}

static void _ellipticFE_lambda_mu(const real_t *phi, const real_t *psi, const real_t *m, real_t *lambda, real_t *lambdaI, real_t *mu, real_t *muI, realContext_t *realContext) {
  // Abramowitz & Stegun §17.4.11
  real_t m1, b, c, csc2Phi, tan2Phi, cot2Phi, sinh2Psi, cot2Lambda, cot2LambdaI;

  realSubtract(const_1, m, &m1, realContext);

  WP34S_Cvt2RadSinCosTan(phi, amRadian, &csc2Phi, &cot2Phi, &tan2Phi, realContext);
  realDivide(const_1, &csc2Phi, &csc2Phi, realContext);
  realMultiply(&csc2Phi, &csc2Phi, &csc2Phi, realContext);
  realDivide(const_1, &tan2Phi, &cot2Phi, realContext);
  realMultiply(&tan2Phi, &tan2Phi, &tan2Phi, realContext);
  realMultiply(&cot2Phi, &cot2Phi, &cot2Phi, realContext);
  WP34S_SinhCosh(psi, &sinh2Psi, NULL, realContext);
  realMultiply(&sinh2Psi, &sinh2Psi, &sinh2Psi, realContext);

  realMultiply(m, &sinh2Psi, &b, realContext);
  realFMA(&b, &csc2Phi, &cot2Phi, &b, realContext);
  realSubtract(&b, &m1, &b, realContext);
  realChangeSign(&b);
  realMultiply(&m1, &cot2Phi, &c, realContext);
  realChangeSign(&c);

  realMultiply(const__4, &c, &cot2Lambda, realContext);
  realFMA(&b, &b, &cot2Lambda, &cot2Lambda, realContext);
  rcSqrt(&cot2Lambda, &cot2Lambda, &cot2LambdaI, realContext);
  realSubtract(&cot2Lambda, &b, &cot2Lambda, realContext);
  realMultiply(&cot2Lambda, const_1on2, &cot2Lambda, realContext);
  realMultiply(&cot2LambdaI, const_1on2, &cot2LambdaI, realContext);

  if(realIsZero(&cot2LambdaI)) {
    realDivide(const_1, &cot2Lambda, lambda, realContext);
    rcSqrt(lambda, lambda, lambdaI, realContext);
    if(realIsZero(lambdaI)) {
      WP34S_Atan(lambda, lambda, realContext);
    }
    else {
      ArctanComplex(lambda, lambdaI, lambda, lambdaI, realContext);
    }

    if(realIsZero(&cot2Lambda) && realIsZero(&cot2LambdaI)) {
      realCopy(const__1, mu);
    }
    else {
      realFMA(&tan2Phi, &cot2Lambda, const__1, mu, realContext);
    }
    realDivide(mu, m, mu, realContext);
    rcSqrt(mu, mu, muI, realContext);
    if(realIsZero(muI)) {
      WP34S_Atan(mu, mu, realContext);
    }
    else {
      ArctanComplex(mu, muI, mu, muI, realContext);
    }
  }
  else {
    divRealComplex(const_1, &cot2Lambda, &cot2LambdaI, lambda, lambdaI, realContext);
    sqrtComplex(lambda, lambdaI, lambda, lambdaI, realContext);
    ArctanComplex(lambda, lambdaI, lambda, lambdaI, realContext);

    if(realIsZero(&cot2Lambda) && realIsZero(&cot2LambdaI)) {
      realCopy(const__1, mu); realCopy(const_0, muI);
    }
    else {
      realFMA(&tan2Phi, &cot2Lambda, const__1, mu, realContext); realMultiply(&tan2Phi, &cot2LambdaI, muI, realContext);
    }
    realDivide(mu, m, mu, realContext); realDivide(muI, m, muI, realContext);
    sqrtComplex(mu, muI, mu, muI, realContext);
    ArctanComplex(mu, muI, mu, muI, realContext);
  }
}

static void _ellipticF(const real_t *phi, const real_t *m, real_t *res, realContext_t *realContext) {
  // assumes 0 ≤ phi ≤ pi/2 and 0 ≤ m ≤ 1
  real_t v, k1, agm;
  int n;

  /* The formula used here is found at:
   * http://hp41programs.yolasite.com/ellipticf.php
   */
  realCopy(phi, &v);
  realSubtract(const_1, m, &k1, realContext);
  realSquareRoot(&k1, &k1, realContext);
  n = realAgmForF(const_1, &k1, &v, &agm, realContext);
  realDivide(&v, &agm, res, realContext);
  while(n-- > 0) {
    realMultiply(res, const_1on2, res, realContext);
  }
}

static void _ellipticF_1(const real_t *phi, const real_t *m, real_t *res, realContext_t *realContext) {
  if(realCompareLessEqual(phi, const_piOn4)) {
    _ellipticF(phi, m, res, realContext);
  }
  else {
    // Abramowitz & Stegun §17.4.13
    real_t psi, m1, k1, tanPhi, k;

    ellipticKE(m, &k, NULL, NULL, NULL, realContext);
    WP34S_Cvt2RadSinCosTan(phi, amRadian, &m1, &k1, &tanPhi, realContext);
    realSubtract(const_1, m, &m1, realContext);
    realSquareRoot(&m1, &k1, realContext);
    realDivide(const_1, &k1, &psi, realContext);
    realMultiply(&k1, &tanPhi, &psi, realContext);
    WP34S_Atan2(const_1, &psi, &psi, realContext);
    _ellipticF(&psi, &m1, res, realContext);
    realSubtract(&k, res, res, realContext);
  }
}

static void _ellipticF_2(const real_t *phi, const real_t *m, real_t *res, realContext_t *realContext) {
  // assumes phi is real and 0 ≤ m ≤ 1
  // Abramowitz & Stegun §17.4.1, §17.4.3
  real_t phi1, phiQuotient, phiRemainder;
  bool_t   remainderNegative = false;

  realCopyAbs(phi, &phi1);
  realDivide(&phi1, const_pi, &phiQuotient, realContext);
  realToIntegralValue(&phiQuotient, &phiQuotient, DEC_ROUND_DOWN, realContext);
  realDivideRemainder(&phi1, const_pi, &phiRemainder, realContext);
  if(realCompareGreaterThan(&phiRemainder, const_piOn2)) {
    realAdd(&phiQuotient, const_1, &phiQuotient, realContext);
    realSubtract(const_pi, &phiRemainder, &phiRemainder, realContext);
    remainderNegative = true;
  }

  if(realIsZero(&phiQuotient)) {
    _ellipticF_1(&phiRemainder, m, res, realContext);
  }
  else {
    realAdd(&phiQuotient, &phiQuotient, &phiQuotient, realContext);
    ellipticKE(m, &phi1, NULL, NULL, NULL, realContext);
    _ellipticF_1(&phiRemainder, m, res, realContext);
    if(remainderNegative) {
      realChangeSign(res);
    }
    realFMA(&phiQuotient, &phi1, res, res, realContext);
  }

  if(realIsNegative(phi)) {
    realChangeSign(res);
  }
}

static void _ellipticF_3(const real_t *phi, const real_t *m, real_t *res, real_t *resi, realContext_t *realContext) {
  if(realIsZero(m)) {
    // Abramowitz & Stegun §17.4.19-20
    realCopy(phi, res);
    realCopy(const_0, resi);
  }
  else if(realCompareGreaterThan(m, const_1)) {
    // Abramowitz & Stegun §17.4.15
    real_t k, m_1, theta, thetai, a;

    mod2Pi(phi, &theta, realContext);
    WP34S_Cvt2RadSinCosTan(&theta, amRadian, &a, NULL, NULL, realContext);
    realSquareRoot(m, &k, realContext);
    realDivide(const_1, m, &m_1, realContext);
    realMultiply(&k, &a, &a, realContext);
    ArcsinComplex(&a, const_0, &theta, &thetai, realContext);
    ellipticF(&theta, &thetai, &m_1, res, resi, realContext); // recurses here
    divComplexComplex(res, resi, &k, const_0, res, resi, realContext);
  }
  else if(realIsPositive(m)) {
    _ellipticF_2(phi, m, res, realContext);
    realCopy(const_0, resi);
  }
  else {
    // Abramowitz & Stegun §17.4.17
    real_t absm, mp1, rtmp1, k, ki, m2, phi1, f, fi;

    realCopyAbs(m, &absm);
    realAdd(&absm, const_1, &mp1, realContext);
    realSquareRoot(&mp1, &rtmp1, realContext);

    realDivide(&absm, &mp1, &m2, realContext);
    ellipticKE(&m2, &k, &ki, NULL, NULL, realContext);

    realSubtract(const_piOn2, phi, &phi1, realContext);
    ellipticF(&phi1, const_0, &m2, &f, &fi, realContext); // recurses here

    realSubtract(&k, &f, &k, realContext); realSubtract(&ki, &fi, &ki, realContext);
    divComplexComplex(&k, &ki, &rtmp1, const_0, res, resi, realContext);
  }
}

static void _ellipticF_4(const real_t *phi, const real_t *psi, const real_t *m, real_t *res, real_t *resi, realContext_t *realContext) {
  if(realIsZero(psi)) {
    _ellipticF_3(phi, m, res, resi, realContext);
  }
  else if(realIsZero(phi)) {
    // Abramowitz & Stegun §17.4.8
    real_t m1, theta;

    realSubtract(const_1, m, &m1, realContext);
    GudermannianReal(psi, &theta, realContext);
    _ellipticF_3(&theta, &m1, resi, res, realContext);
    realChangeSign(res);
  }
  else {
    // Abramowitz & Stegun §17.4.11
    real_t m1, lambda, lambdaI, mu, muI, b, c;

    realSubtract(const_1, m, &m1, realContext);
    _ellipticFE_lambda_mu(phi, psi, m, &lambda, &lambdaI, &mu, &muI, realContext);

    if(realIsZero(&lambdaI)) {
      _ellipticF_3(&lambda, m, &b, &c, realContext);
    }
    else {
      ellipticF(&lambda, &lambdaI, m, &b, &c, realContext); // recurses here
    }
    realCopy(&b, res); realCopy(&c, resi);
    if(realIsZero(&muI)) {
      _ellipticF_3(&mu, &m1, &b, &c, realContext);
    }
    else {
      _ellipticF_4(&mu, &muI, &m1, &b, &c, realContext); // recurses here
    }
    realAdd(resi, &b, resi, realContext);
    realSubtract(res, &c, res, realContext);
  }
}

static void _ellipticF_5(const real_t *phi, const real_t *psi, const real_t *m, real_t *res, real_t *resi, realContext_t *realContext) {
  // Abramowitz & Stegun §17.4.13
  if(realCompareAbsGreaterThan(phi, const_piOn4)) {
    real_t psir, psii, m1, k1, k1i, tanPhi, tanPhiI, k, ki;

    ellipticKE(m, &k, &ki, NULL, NULL, realContext);
    if(realIsNegative(psi)) {
      realChangeSign(&ki); // conjugate
    }
    TanComplex(phi, psi, &tanPhi, &tanPhiI, realContext);
    realSubtract(const_1, m, &m1, realContext);
    rcSqrt(&m1, &k1, &k1i, realContext);
    mulComplexComplex(&k1, &k1i, &tanPhi, &tanPhiI, &psir, &psii, realContext);
    divRealComplex(const_1, &psir, &psii, &psir, &psii, realContext);
    if(realIsZero(&psii)) {
      WP34S_Atan(&psir, &psir, realContext);
    }
    else {
      ArctanComplex(&psir, &psii, &psir, &psii, realContext);
    }
    _ellipticF_4(&psir, &psii, m, res, resi, realContext);

    if(!realIsZero(&ki)) {
      if(realIsPositive(&ki)) {
        realSetPositiveSign(resi);
      }
      else {
        realSetNegativeSign(resi);
      }
    }
    realSubtract(&k, res, res, realContext); realSubtract(&ki, resi, resi, realContext);

    if(realIsPositive(psi)) {
      realSetPositiveSign(resi);
    }
    else {
      realSetNegativeSign(resi);
    }
    if(!realIsZero(&ki) && realIsNegative(&ki)) {
      realChangeSign(resi);
    }
  }
  else {
    _ellipticF_4(phi, psi, m, res, resi, realContext);
  }
}

void ellipticF(const real_t *phi, const real_t *psi, const real_t *m, real_t *res, real_t *resi, realContext_t *realContext) {
  // Abramowitz & Stegun §17.4.1, §17.4.3
  real_t phi1, phiQuotient, phiRemainder, psi1;
  bool_t   remainderNegative = false;

  realCopy(phi, &phi1); realCopy(psi, &psi1);
  if(realIsNegative(phi)) {
    realChangeSign(&phi1);
    realChangeSign(&psi1);
  }
  realDivide(&phi1, const_pi, &phiQuotient, realContext);
  realToIntegralValue(&phiQuotient, &phiQuotient, DEC_ROUND_DOWN, realContext);
  realDivideRemainder(&phi1, const_pi, &phiRemainder, realContext);
  if(realCompareGreaterThan(&phiRemainder, const_piOn2)) {
    realAdd(&phiQuotient, const_1, &phiQuotient, realContext);
    realSubtract(const_pi, &phiRemainder, &phiRemainder, realContext);
    remainderNegative = true;
  }

  if(realIsZero(phi) && realIsZero(psi)) {
    realZero(res); realZero(resi);
    return;
  }
  else if(realIsZero(m)) {
    realCopy(phi, res); realCopy(psi, resi);
    return;
  }
  else if(realCompareEqual(m, const_1)) {
    real_t p, q;
    realFMA(phi, const_1on2, const_piOn4, &p, realContext); realMultiply(psi, const_1on2, &q, realContext);
    if(realIsZero(&q)) {
      WP34S_Cvt2RadSinCosTan(&p, amRadian, res, resi, &q, realContext);
      WP34S_Ln(&q, res, realContext);
      realZero(resi);
    }
    else {
      TanComplex(&p, &q, &p, &q, realContext);
      lnComplex(&p, &q, res, resi, realContext);
      if(realIsZero(&p)) {
        realZero(res);
      }
    }
    return;
  }
  else if(realIsZero(&phiQuotient)) {
    _ellipticF_5(&phiRemainder, &psi1, m, res, resi, realContext);
    if(remainderNegative) {
      realChangeSign(res);
      realChangeSign(resi);
    }
    if(realIsNegative(phi)) {
      realChangeSign(res);
      realChangeSign(resi);
    }
  }
  else {
    realAdd(&phiQuotient, &phiQuotient, &phiQuotient, realContext);
    _ellipticF_5(&phiRemainder, &psi1, m, res, resi, realContext);
    if(remainderNegative) {
      realChangeSign(res);
      realChangeSign(resi);
    }
    if(realIsNegative(phi)) {
      realChangeSign(res);
      realChangeSign(resi);
    }
    ellipticKE(m, &phi1, &psi1, NULL, NULL, realContext);
    realFMA(&phiQuotient, &phi1, res, res, realContext);
    realFMA(&phiQuotient, &psi1, resi, resi, realContext);
  }

  if(realCompareGreaterThan(m, const_1) && (realCompareAbsGreaterThan(phi, const_piOn2) || realIsZero(psi))) {
    if(realIsPositive(phi)) {
      realSetNegativeSign(resi);
    }
    else {
      realSetPositiveSign(resi);
    }
  }
  else {
    if(realIsPositive(psi)) {
      realSetPositiveSign(resi);
    }
    else if(realIsNegative(psi)) {
      realSetNegativeSign(resi);
    }
    else if(realIsPositive(psi)) {
      realSetPositiveSign(resi);
    }
    else if(realIsNegative(psi)) {
      realSetNegativeSign(resi);
    }
  }
}

static void _ellipticE_fromZeta(const real_t *phi, const real_t *psi, const real_t *m, real_t *res, real_t *resi, realContext_t *realContext) {
  // Compute from Jacobi Zeta
  real_t k, ki, e, ei, f, fi, z, zi;

  ellipticKE(m, &k, &ki, &e, &ei, realContext);
  ellipticF(phi, psi, m, &f, &fi, realContext);
  jacobiZeta(phi, psi, m, &z, &zi, realContext);
  divComplexComplex(&e, &ei, &k, &ki, res, resi, realContext);
  mulComplexComplex(res, resi, &f, &fi, res, resi, realContext);
  realAdd(res, &z, res, realContext); realAdd(resi, &zi, resi, realContext);
}
void ellipticE(const real_t *phi, const real_t *psi, const real_t *m, real_t *res, real_t *resi, realContext_t *realContext) {
  real_t phi1, phiQuotient, phiRemainder, psi1;

  realCopyAbs(phi, &phi1); realCopyAbs(psi, &psi1);
  realDivide(&phi1, const_pi, &phiQuotient, realContext);
  realToIntegralValue(&phiQuotient, &phiQuotient, DEC_ROUND_DOWN, realContext);
  realDivideRemainder(&phi1, const_pi, &phiRemainder, realContext);
  if(realCompareGreaterThan(&phiRemainder, const_piOn2)) {
    realAdd(&phiQuotient, const_1, &phiQuotient, realContext);
    realSubtract(&phiRemainder, const_pi, &phiRemainder, realContext);
  }
  if(realIsNegative(phi)) {
    realChangeSign(&phi1);
    realChangeSign(&phiQuotient);
    realChangeSign(&phiRemainder);
    realChangeSign(&psi1);
  }

  if(realIsZero(m)) {
    realCopy(phi, res); realCopy(psi, resi);
  }
  else if(realCompareEqual(m, const_1)) {
    if(realIsZero(psi)) {
      WP34S_Cvt2RadSinCosTan(&phiRemainder, amRadian, res, NULL, NULL, realContext); realZero(resi);
      realAdd(res, &phiQuotient, res, realContext);
      realAdd(res, &phiQuotient, res, realContext);
    }
    else {
      sinComplex(&phiRemainder, psi, res, resi, realContext);
      realAdd(res, &phiQuotient, res, realContext);
      realAdd(res, &phiQuotient, res, realContext);
    }
  }
  else if(realIsZero(psi) && realCompareGreaterThan(m, const_1)) {
    real_t k, ki, m_1, u, ui, a, b;

    // Check if the imaginary part is zero: am(F(φ+iψ|m)*sqrt(m)|1/m)
    ellipticF(phi, const_0, m, &u, &ui, realContext);
    rcSqrt(m, &k, &ki, realContext);
    realDivide(const_1, m, &m_1, realContext);
    mulComplexComplex(&u, &ui, &k, &ki, &a, &b, realContext);
    jacobiComplexAm(&a, &b, &m_1, &a, &b, realContext);

    _ellipticE_fromZeta(phi, const_0, m, res, resi, realContext);
    if(realIsZero(&b)) {
      realZero(resi);
    }
  }
  else if(realIsZero(psi)) {
    _ellipticE_fromZeta(phi, const_0, m, res, resi, realContext);
  }
  else if(realIsZero(phi)) {
    real_t theta, m1, a, b, c, d;

    GudermannianReal(psi, &theta, realContext);
    realSubtract(const_1, m, &m1, realContext);

    WP34S_Cvt2RadSinCosTan(&theta, amRadian, &c, &d, &a, realContext);

    realMultiply(&c, &c, &c, realContext);
    realMultiply(&c, &m1, &c, realContext);
    realSubtract(const_1, &c, &c, realContext);
    rcSqrt(&c, &c, &d, realContext);
    mulComplexComplex(&a, const_0, &c, &d, &a, &b, realContext);

    ellipticF(&theta, const_0, &m1, &c, &d, realContext);
    realAdd(&a, &c, &a, realContext); realAdd(&b, &d, &b, realContext);
    ellipticE(&theta, const_0, &m1, &c, &d, realContext);
    realSubtract(&a, &c, &a, realContext); realSubtract(&b, &d, &b, realContext);

    realCopy(&a, resi); realCopy(&b, res); realChangeSign(res);
  }
  else {
    real_t *tmpVal;

    #define LAMBDA        (tmpVal)
    #define LAMBDA_I      (tmpVal + 1)
    #define MU            (tmpVal + 2)
    #define MU_I          (tmpVal + 3)
    #define B1            (tmpVal + 4)
    #define B1_I          (tmpVal + 5)
    #define B2            (tmpVal + 6)
    #define B2_I          (tmpVal + 7)
    #define B3            (tmpVal + 8)
    #define B3_I          (tmpVal + 9)
    #define SIN_LAMBDA    (tmpVal + 10)
    #define SIN_LAMBDA_I  (tmpVal + 11)
    #define SIN2_LAMBDA   (tmpVal + 12)
    #define SIN2_LAMBDA_I (tmpVal + 13)
    #define COS_LAMBDA    (tmpVal + 14)
    #define COS_LAMBDA_I  (tmpVal + 15)
    #define SIN_MU        (tmpVal + 16)
    #define SIN_MU_I      (tmpVal + 17)
    #define SIN2_MU       (tmpVal + 18)
    #define SIN2_MU_I     (tmpVal + 19)
    #define COS_MU        (tmpVal + 20)
    #define COS_MU_I      (tmpVal + 21)
    #define COS2_MU       (tmpVal + 22)
    #define COS2_MU_I     (tmpVal + 23)
    #define M1            (tmpVal + 24)

    if((tmpVal = allocC47Blocks(25 * REAL_SIZE_IN_BLOCKS))) {
      bool_t           remainderNegative = realIsNegative(&phiRemainder);
      realContext_t *realContext2 = &ctxtReal51;
      realContext_t *realContext3 = &ctxtReal75;

      if(remainderNegative) {
        realChangeSign(&phiRemainder);
      }
      realSubtract(const_1, m, M1, realContext2);
      _ellipticFE_lambda_mu(&phiRemainder, &psi1, m, LAMBDA, LAMBDA_I, MU, MU_I, realContext2);
      sinComplex(LAMBDA, LAMBDA_I, SIN_LAMBDA, SIN_LAMBDA_I, realContext2);
      cosComplex(LAMBDA, LAMBDA_I, COS_LAMBDA, COS_LAMBDA_I, realContext2);
      sinComplex(MU, MU_I, SIN_MU, SIN_MU_I, realContext2);
      cosComplex(MU, MU_I, COS_MU, COS_MU_I, realContext2);
      mulComplexComplex(SIN_LAMBDA, SIN_LAMBDA_I, SIN_LAMBDA, SIN_LAMBDA_I, SIN2_LAMBDA, SIN2_LAMBDA_I, realContext2);
      mulComplexComplex(SIN_MU, SIN_MU_I, SIN_MU, SIN_MU_I, SIN2_MU, SIN2_MU_I, realContext2);
      mulComplexComplex(COS_MU, COS_MU_I, COS_MU, COS_MU_I, COS2_MU, COS2_MU_I, realContext2);

      realMultiply(m, SIN2_LAMBDA, B1, realContext2); realMultiply(m, SIN2_LAMBDA_I, B1_I, realContext2);
      realSubtract(const_1, B1, B2, realContext2); realSubtract(const_0, B1_I, B2_I, realContext2);
      sqrtComplex(B2, B2_I, B1, B1_I, realContext2);
      mulComplexComplex(SIN2_MU, SIN2_MU_I, B1, B1_I, B1, B1_I, realContext2);
      mulComplexComplex(COS_LAMBDA, COS_LAMBDA_I, B1, B1_I, B1, B1_I, realContext2);
      mulComplexComplex(SIN_LAMBDA, SIN_LAMBDA_I, B1, B1_I, B1, B1_I, realContext2);
      realMultiply(m, B1, B1, realContext2); realMultiply(m, B1_I, B1_I, realContext2);

      realMultiply(M1, SIN2_MU, B3, realContext2); realMultiply(M1, SIN2_MU_I, B3_I, realContext2);
      realSubtract(const_1, B3, B3, realContext2); realSubtract(const_0, B3_I, B3_I, realContext2);
      sqrtComplex(B3, B3_I, B3, B3_I, realContext2);
      mulComplexComplex(B2, B2_I, B3, B3_I, B2, B2_I, realContext2);
      mulComplexComplex(B2, B2_I, SIN_MU, SIN_MU_I, B2, B2_I, realContext2);
      mulComplexComplex(B2, B2_I, COS_MU, COS_MU_I, B2, B2_I, realContext2);

      realMultiply(m, SIN2_LAMBDA, B3, realContext2); realMultiply(m, SIN2_LAMBDA_I, B3_I, realContext2);
      mulComplexComplex(B3, B3_I, SIN2_MU, SIN2_MU_I, B3, B3_I, realContext2);
      realAdd(COS2_MU, B3, B3, realContext2); realAdd(COS2_MU_I, B3_I, B3_I, realContext2);

      realSubtract(B1, B2_I, res, realContext2); realAdd(B1_I, B2, resi, realContext2);
      divComplexComplex(res, resi, B3, B3_I, res, resi, realContext2);

      ellipticE(LAMBDA, LAMBDA_I, m, B1, B1_I, realContext3);
      ellipticE(MU, MU_I, M1, B2, B2_I, realContext3);
      ellipticF(MU, MU_I, M1, B3, B3_I, realContext3);

      realSubtract(res, B3_I, res, realContext); realAdd(resi, B3, resi, realContext);
      realAdd(res, B2_I, res, realContext); realSubtract(resi, B2, resi, realContext);
      realAdd(res, B1, res, realContext); realAdd(resi, B1_I, resi, realContext);

      if(remainderNegative) {
        realChangeSign(res);
      }

      if(!realIsZero(&phiQuotient)) {
        ellipticKE(m, NULL, NULL, B1, B1_I, realContext);
        realSetPositiveSign(&phiQuotient);
        realAdd(&phiQuotient, &phiQuotient, &phiQuotient, realContext);
        realMultiply(&phiQuotient, B1, B1, realContext); realMultiply(&phiQuotient, B1_I, B1_I, realContext);
        realAdd(res, B1, res, realContext); realAdd(resi, B1_I, resi, realContext);
      }

      if(realIsNegative(psi)) {
        realChangeSign(resi);
      }

      freeC47Blocks(tmpVal, 25 * REAL_SIZE_IN_BLOCKS);
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      realCopy(const_NaN, res); realCopy(const_NaN, resi);
    }

    #undef LAMBDA
    #undef LAMBDA_I
    #undef MU
    #undef MU_I
    #undef B1
    #undef B1_I
    #undef B2
    #undef B2_I
    #undef B3
    #undef B3_I
    #undef SIN_LAMBDA
    #undef SIN_LAMBDA_I
    #undef SIN2_LAMBDA
    #undef SIN2_LAMBDA_I
    #undef COS_LAMBDA
    #undef COS_LAMBDA_I
    #undef SIN_MU
    #undef SIN_MU_I
    #undef SIN2_MU
    #undef SIN2_MU_I
    #undef COS_MU
    #undef COS_MU_I
    #undef COS2_MU
    #undef COS2_MU_I
    #undef M1
  }
}

static void _jacobiZeta_Agm(const real_t *phi, const real_t *psi, const real_t *m, real_t *res, real_t *resi, realContext_t *realContext) {
  real_t *a, *ai, *b, *bi;
  int n;

  if(realIsZero(m)) {
    realZero(res); realZero(resi);
    return;
  }
  if(realCompareEqual(m, const_1)) {
    real_t k, ki;
    ellipticF(phi, psi, m, &k, &ki, realContext);
    TanhComplex(&k, &ki, res, resi, realContext);
    return;
  }
  if((a = allocC47Blocks(ELLIPTIC_N * REAL_SIZE_IN_BLOCKS))) {
    if((ai = allocC47Blocks(ELLIPTIC_N * REAL_SIZE_IN_BLOCKS))) {
      if((b = allocC47Blocks(ELLIPTIC_N * REAL_SIZE_IN_BLOCKS))) {
        if((bi = allocC47Blocks(ELLIPTIC_N * REAL_SIZE_IN_BLOCKS))) {
          real_t k, ki, c, ci, s, si, q;

          realZero(res); realZero(resi);
          realSubtract(const_1, m, &k, realContext);
          rcSqrt(&k, &k, &ki, realContext);
          n = complexAgmStep(const_1, const_0, &k, &ki, &c, &ci, a, ai, b, bi, ELLIPTIC_N, realContext);
          ellipticF(phi, psi, m, &k, &ki, realContext);
          mulComplexComplex(&k, &ki, a + n, ai + n, &k, &ki, realContext);
          for(int i = 0; i < n; ++i) {
            realAdd(&k, &k, &k, realContext); realAdd(&ki, &ki, &ki, realContext);
          }
          for(int i = n; i > 0; --i) {
            realCopy(&k, &q);
            mod2Pi(&q, &k, realContext);
            if(realCompareGreaterThan(&k, const_pi)) {
              realSubtract(&k, const_2pi_75, &k, realContext);
            }
            realSubtract(&q, &k, &q, realContext);
            realSubtract(a + i - 1, b + i - 1, &c, realContext); realSubtract(ai + i - 1, bi + i - 1, &ci, realContext);
            realMultiply(&c, const_1on2, &c, realContext); realMultiply(&ci, const_1on2, &ci, realContext);
            sinComplex(&k, &ki, &s, &si, realContext);
            mulComplexComplex(&s, &si, &c, &ci, &s, &si, realContext);
            realAdd(&s, res, res, realContext); realAdd(&si, resi, resi, realContext);
            divComplexComplex(&s, &si, a + i, ai + i, &s, &si, realContext);
            if(realIsZero(&si) && !realCompareAbsLessThan(const_1, &s)) {
              WP34S_Asin(&s, &s, realContext);
            }
            else {
              ArcsinComplex(&s, &si, &s, &si, realContext);
            }
            realAdd(&s, &k, &s, realContext); realAdd(&si, &ki, &si, realContext);
            realAdd(&s, &q, &s, realContext);
            realMultiply(&s, const_1on2, &k, realContext); realMultiply(&si, const_1on2, &ki, realContext);
          }
          freeC47Blocks(bi, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
        }
        else {
          displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
          realCopy(const_NaN, res); realCopy(const_NaN, resi);
        }
        freeC47Blocks(b, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
      }
      else {
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
        realCopy(const_NaN, res); realCopy(const_NaN, resi);
      }
      freeC47Blocks(ai, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      realCopy(const_NaN, res); realCopy(const_NaN, resi);
    }
    freeC47Blocks(a, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS);
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
    realCopy(const_NaN, res); realCopy(const_NaN, resi);
  }
}

static void _jacobiZeta(const real_t *phi, const real_t *psi, const real_t *m, real_t *res, real_t *resi, realContext_t *realContext) {
  if(realCompareLessEqual(m, const_1)) {
    _jacobiZeta_Agm(phi, psi, m, res, resi, realContext);
  }
  else if(realCompareLessEqual(m, const_2)) {
    int n = 0;
    real_t phir, phii, ur, ui, pp;
    realCopy(phi, &phir); realCopy(psi, &phii);
    ellipticF(&phir, &phii, m, &ur, &ui, realContext);

    if(realCompareLessEqual(m, const_2)) {
      realCopy(const_1on4, &pp);
    }
    else {
      realDivide(const_1on2, m, &pp, realContext);
    }

    while(realCompareGreaterThan(&phir, &pp)) {
      ++n;
      realMultiply(&ur, const_1on2, &ur, realContext); realMultiply(&ui, const_1on2, &ui, realContext);
      jacobiComplexAm(&ur, &ui, m, &phir, &phii, realContext);
    }

    _jacobiZeta_Agm(&phir, &phii, m, res, resi, realContext);
    while(n-- > 0) {
      real_t a, b, c, d;
      realAdd(res, res, res, realContext); realAdd(resi, resi, resi, realContext);
      jacobiComplexSn(&ur, &ui, m, &a, &b, realContext);
      mulComplexComplex(&a, &b, &a, &b, &a, &b, realContext);
      realMultiply(&a, m, &a, realContext); realMultiply(&b, m, &b, realContext);
      realAdd(&ur, &ur, &ur, realContext); realAdd(&ui, &ui, &ui, realContext);
      jacobiComplexSn(&ur, &ui, m, &c, &d, realContext);
      mulComplexComplex(&a, &b, &c, &d, &a, &b, realContext);
      realSubtract(res, &a, res, realContext); realSubtract(resi, &b, resi, realContext);
    }
  }
}

void jacobiZeta(const real_t *phi, const real_t *psi, const real_t *m, real_t *res, real_t *resi, realContext_t *realContext) {
  real_t v, vi;
  bool_t   negative = realIsNegative(phi);

  if(realIsZero(m)) {
    realZero(res); realZero(resi);
    return;
  }

  realCopy(phi, &v); realCopy(psi, &vi);
  if(negative) {
    realChangeSign(&v);
    realChangeSign(&vi);
  }
  realCopy(const_NaN, res); realCopy(const_NaN, resi);

  WP34S_Mod(&v, const_pi, &v, realContext);
  if(realCompareGreaterThan(&v, const_piOn2)) {
    realSubtract(const_pi, &v, &v, realContext);
    realChangeSign(&vi);
    negative = !negative;
  }
  if(realIsZero(&vi)) {
    _jacobiZeta(&v, &vi, m, res, resi, realContext);
  }
  else {
    real_t a, b, c, d;
    ellipticKE(m, &a, &b, &c, &d, realContext);
    divComplexComplex(&c, &d, &a, &b, &a, &b, realContext);
    ellipticF(&v, &vi, m, &c, &d, realContext);
    mulComplexComplex(&a, &b, &c, &d, &a, &b, realContext);
    ellipticE(&v, &vi, m, &c, &d, realContext);
    realSubtract(&c, &a, res, realContext); realSubtract(&d, &b, resi, realContext);
  }

  if(negative) {
    realChangeSign(res);
    realChangeSign(resi);
  }
}

static void heumanLambda(const real_t *phi, const real_t *psi, const real_t *m, real_t *res, real_t *resi, realContext_t *realContext) {
  // Abramowitz & Stegun §17.4.39-40
  real_t k, ki, k1, k1i, z, zi, f, fi, m1;

  realSubtract(const_1, m, &m1, realContext);
  ellipticKE(m, &k, &ki, NULL, NULL, realContext);
  ellipticKE(&m1, &k1, &k1i, NULL, NULL, realContext);
  ellipticF(phi, psi, &m1, &f, &fi, realContext);
  jacobiZeta(phi, psi, &m1, &z, &zi, realContext);

  divComplexComplex(&f, &fi, &k1, &k1i, res, resi, realContext);
  mulComplexComplex(&k, &ki, &z, &zi, &z, &zi, realContext);
  divComplexComplex(&z, &zi, const_piOn2, const_0, &z, &zi, realContext);
  realAdd(res, &z, res, realContext); realAdd(resi, &zi, resi, realContext);
}

static void _ellipticPi_1(const real_t *n, const real_t *m, real_t *res, real_t *resi, realContext_t *realContext) {
  // Abramowitz & Stegun §17.7.6
  real_t epsilon, epsiloni, delta1, delta1i, k, ki, z, zi, p;

  realDivide(n, m, &epsilon, realContext);
  rcSqrt(&epsilon, &epsilon, &epsiloni, realContext);
  ArcsinComplex(&epsilon, &epsiloni, &epsilon, &epsiloni, realContext);

  realSubtract(const_1, n, &delta1, realContext);
  realDivide(n, &delta1, &delta1, realContext);
  realSubtract(m, n, &p, realContext);
  realDivide(&delta1, &p, &delta1, realContext);
  rcSqrt(&delta1, &delta1, &delta1i, realContext);

  ellipticKE(m, &k, &ki, NULL, NULL, realContext);
  jacobiZeta(&epsilon, &epsiloni, m, &z, &zi, realContext);
  mulComplexComplex(&k, &ki, &z, &zi, res, resi, realContext);
  mulComplexComplex(res, resi, &delta1, &delta1i, res, resi, realContext);
  realAdd(res, &k, res, realContext); realAdd(resi, &ki, resi, realContext);
}
static void _ellipticPi_2(const real_t *n, const real_t *m, real_t *res, real_t *resi, realContext_t *realContext) {
  // Abramowitz & Stegun §17.7.7, §17.7.9
  real_t nn, k, ki;

  realDivide(m, n, &nn, realContext);
  ellipticKE(m, &k, &ki, NULL, NULL, realContext);
  _ellipticPi_1(&nn, m, res, resi, realContext);
  realSubtract(&k, res, res, realContext); realSubtract(&ki, resi, resi, realContext);
}
static void _ellipticPi_3(const real_t *n, const real_t *m, real_t *res, real_t *resi, realContext_t *realContext) {
  // Abramowitz & Stegun §17.7.14
  real_t epsilon, epsiloni, delta2, delta2i, k, ki, n1, m1, p;

  realSubtract(const_1, n, &n1, realContext);
  realSubtract(const_1, m, &m1, realContext);

  realDivide(&n1, &m1, &epsilon, realContext);
  rcSqrt(&epsilon, &epsilon, &epsiloni, realContext);
  ArcsinComplex(&epsilon, &epsiloni, &epsilon, &epsiloni, realContext);

  realDivide(n, &n1, &delta2, realContext);
  realSubtract(n, m, &p, realContext);
  realDivide(&delta2, &p, &delta2, realContext);
  rcSqrt(&delta2, &delta2, &delta2i, realContext);

  ellipticKE(m, &k, &ki, NULL, NULL, realContext);

  heumanLambda(&epsilon, &epsiloni, m, res, resi, realContext);
  realSubtract(const_1, res, res, realContext); realChangeSign(resi);
  mulComplexComplex(&delta2, &delta2i, res, resi, res, resi, realContext);
  realFMA(const_piOn2, res, &k, res, realContext); realFMA(const_piOn2, resi, &ki, resi, realContext);
}
static void _ellipticPi_4(const real_t *n, const real_t *m, real_t *res, real_t *resi, realContext_t *realContext) {
  // Abramowitz & Stegun §17.7.15, §17.7.17
  real_t nn, k, ki, mmn, n1;

  realSubtract(m, n, &mmn, realContext);
  realSubtract(const_1, n, &n1, realContext);
  realDivide(&mmn, &n1, &nn, realContext);
  ellipticKE(m, &k, &ki, NULL, NULL, realContext);

  _ellipticPi_3(&nn, m, res, resi, realContext);
  divComplexComplex(res, resi, &mmn, const_0, res, resi, realContext);
  divComplexComplex(res, resi, &n1, const_0, res, resi, realContext);
  realMultiply(m, res, res, realContext); realMultiply(m, resi, resi, realContext);
  realMultiply(n, res, res, realContext); realMultiply(n, resi, resi, realContext);

  divComplexComplex(&k, &ki, &mmn, const_0, &k, &ki, realContext);
  realMultiply(m, &k, &k, realContext); realMultiply(m, &ki, &ki, realContext);

  realSubtract(&k, res, res, realContext); realSubtract(&ki, resi, resi, realContext);
}

void ellipticPi(const real_t *n, const real_t *m, real_t *res, real_t *resi, realContext_t *realContext) {
  if(realIsZero(n)) {
    ellipticKE(m, res, resi, NULL, NULL, realContext);
  }
  else if(realIsZero(m)) {
    // Abramowitz & Stegun §17.7.20
    if(realCompareLessThan(n, const_1)) {
      //      lim       (Arctan(sqrt(1-n) tan(phi)) / sqrt(1-n))
      //  phi->(pi/2)-0                                          = (pi/2) sqrt(1-n) / n
      realSubtract(const_1, n, res, realContext);
      rcSqrt(res, res, resi, realContext);
      divComplexComplex(res, resi, n, const_0, res, resi, realContext);
      realMultiply(const_piOn2, res, res, realContext); realMultiply(const_piOn2, resi, resi, realContext);
    }
    else if(realCompareGreaterThan(n, const_1)) {
      //      lim       (Arctanh(sqrt(n-1) tan(phi)) / sqrt(n-1))
      //  phi->(pi/2)-0                                          = -(pi/2) i / sqrt(n-1)
      realSubtract(n, const_1, res, realContext);
      rcSqrt(res, res, resi, realContext);
      divComplexComplex(const_0, const_1, res, resi, res, resi, realContext);
      realMultiply(const_piOn2, res, res, realContext); realMultiply(const_piOn2, resi, resi, realContext);
      realChangeSign(res); realChangeSign(resi);
    }
    else { // n = 1
      realCopy(const_NaN, res); realCopy(const_0, resi); // unsigned infinity
    }
  }
  else if(realCompareEqual(m, const_1)) {
    if(realCompareLessThan(n, const_1)) {
      realCopy(const_plusInfinity, res); realCopy(const_0, resi);
    }
    else if(realCompareGreaterThan(n, const_1)) {
      realCopy(const_minusInfinity, res); realCopy(const_0, resi);
    }
    else { // n = 1
      realCopy(const_NaN, res); realCopy(const_0, resi); // unsigned infinity
    }
  }
  else if(realCompareEqual(m, n)) {
    // Abramowitz & Stegun §17.7.24
    real_t cos2Alpha, e, ei;

    realSubtract(const_1, m, &cos2Alpha, realContext);
    ellipticKE(m, NULL, NULL, &e, &ei, realContext);
    divComplexComplex(&e, &ei, &cos2Alpha, const_0, res, resi, realContext);
  }
  else if(realCompareEqual(m, const_1)) {
    realCopy(const_NaN, res); realCopy(const_0, resi); // unsigned infinity
  }
  else if(realCompareGreaterThan(n, const_1)) {
    _ellipticPi_2(n, m, res, resi, realContext);
  }
  else if(realIsNegative(n)) {
    _ellipticPi_4(n, m, res, resi, realContext);
  }
  else if(realCompareLessThan(n, m)) {
    _ellipticPi_1(n, m, res, resi, realContext);
  }
  else {
    _ellipticPi_3(n, m, res, resi, realContext);
  }
}


void fnJacobiSn(uint16_t unusedButMandatoryParameter) {
  bool_t   realInput;
  real_t m, uReal, uImag;
  real_t rReal, rImag;

  if(!jacobi_check_inputs(&m, &uReal, &uImag, &realInput)) {
    return;
  }

  if(realInput) {
    jacobiElliptic(&uReal, &m, NULL, &rReal, NULL, NULL, &ctxtReal39);
    convertRealToResultRegister(&rReal, REGISTER_X, amNone);
  }
  else {
    jacobiComplexSn(&uReal, &uImag, &m, &rReal, &rImag, &ctxtReal39);
    convertComplexToResultRegister(&rReal, &rImag, REGISTER_X);
  }

  adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
}

void fnJacobiCn(uint16_t unusedButMandatoryParameter) {
  bool_t   realInput;
  real_t m, uReal, uImag;
  real_t rReal, rImag;

  if(!jacobi_check_inputs(&m, &uReal, &uImag, &realInput)) {
    return;
  }

  if(realInput) {
    jacobiElliptic(&uReal, &m, NULL, NULL, &rReal, NULL, &ctxtReal39);
    convertRealToResultRegister(&rReal, REGISTER_X, amNone);
  }
  else {
    jacobiComplexCn(&uReal, &uImag, &m, &rReal, &rImag, &ctxtReal39);
    convertComplexToResultRegister(&rReal, &rImag, REGISTER_X);
  }

  adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
}

void fnJacobiDn(uint16_t unusedButMandatoryParameter) {
  bool_t   realInput;
  real_t m, uReal, uImag;
  real_t rReal, rImag;

  if(!jacobi_check_inputs(&m, &uReal, &uImag, &realInput)) {
    return;
  }

  if(realInput) {
    jacobiElliptic(&uReal, &m, NULL, NULL, NULL, &rReal, &ctxtReal39);
    convertRealToResultRegister(&rReal, REGISTER_X, amNone);
  }
  else {
    jacobiComplexDn(&uReal, &uImag, &m, &rReal, &rImag, &ctxtReal39);
    convertComplexToResultRegister(&rReal, &rImag, REGISTER_X);
  }

  adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
}

void fnJacobiAmplitude(uint16_t unusedButMandatoryParameter) {
  bool_t   realInput;
  real_t m, uReal, uImag;
  real_t rReal, rImag;

  if(!jacobi_check_inputs(&m, &uReal, &uImag, &realInput)) {
    return;
  }

  if(realInput) {
    jacobiElliptic(&uReal, &m, &rReal, NULL, NULL, NULL, &ctxtReal39);
    convertAngleFromTo(&rReal, amRadian, currentAngularMode, &ctxtReal39);
    convertRealToResultRegister(&rReal, REGISTER_X, currentAngularMode);
  }
  else {
    jacobiComplexAm(&uReal, &uImag, &m, &rReal, &rImag, &ctxtReal39);
    convertComplexToResultRegister(&rReal, &rImag, REGISTER_X);
  }

  adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
}

void fnEllipticK(uint16_t unusedButMandatoryParameter) {
  real_t m, a, b;

  if(!saveLastX()) {
    return;
  }

  if (!getRegisterAsReal(REGISTER_X, &m))
    return;

  if(realCompareLessEqual(&m, const_1)) {
    ellipticKE(&m, &b, NULL, NULL, NULL, &ctxtReal39);
    convertRealToResultRegister(&b, REGISTER_X, amNone);
  }
  else if(getFlag(FLAG_CPXRES)) {
    ellipticKE(&m, &a, &b, NULL, NULL, &ctxtReal39);
    convertComplexToResultRegister(&a, &b, REGISTER_X);
  }
  else {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Cannot calculate K(m) for m > 1 if CPXRES is not set");
      moreInfoOnError("In function fnEllipticK:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}

void fnEllipticE(uint16_t unusedButMandatoryParameter) {
  real_t m, a, b;

  if(!saveLastX()) {
    return;
  }

  if (!getRegisterAsReal(REGISTER_X, &m))
    return;

  if(realCompareLessEqual(&m, const_1)) {
    ellipticKE(&m, NULL, NULL, &b, NULL, &ctxtReal39);
    convertRealToResultRegister(&b, REGISTER_X, amNone);
  }
  else if(getFlag(FLAG_CPXRES)) {
    ellipticKE(&m, NULL, NULL, &a, &b, &ctxtReal39);
    convertComplexToResultRegister(&a, &b, REGISTER_X);
  }
  else {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Cannot calculate K(m) for m > 1 if CPXRES is not set");
      moreInfoOnError("In function fnEllipticE:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}

void fnEllipticPi(uint16_t unusedButMandatoryParameter) {
  real_t m, ur, ui, rr, ri;
  bool_t   realInput;

  if(!jacobi_check_inputs(&m, &ur, &ui, &realInput)) {
    return;
  }

  if(realIsNegative(&m) || realCompareGreaterEqual(&m, const_1)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "m is out of range (must in 0 ≤ m < 1)");
      moreInfoOnError("In function fnEllipticPi:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else if(realInput) {
    ellipticPi(&ur, &m, &rr, &ri, &ctxtReal39);
    if(realIsZero(&ri)) {
      convertRealToResultRegister(&rr, REGISTER_X, amNone);
    }
    else if(getFlag(FLAG_CPXRES)) {
      convertComplexToResultRegister(&rr, &ri, REGISTER_X);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Π(n|m) cannot return complex result without CPXRES set");
        moreInfoOnError("In function fnEllipticPi:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot calculate elliptic integral with %s in X", getRegisterDataTypeName(REGISTER_X, true, false));
      moreInfoOnError("In function fnEllipticPi:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
}

void fnEllipticFphi(uint16_t unusedButMandatoryParameter) {
  bool_t   realInput;
  real_t m, uReal, uImag;
  real_t rReal, rImag;

  if(!jacobi_check_inputs(&m, &uReal, &uImag, &realInput)) {
    return;
  }

  ellipticF(&uReal, &uImag, &m, &rReal, &rImag, &ctxtReal39);
  if(realInput) {
    if(realIsZero(&rImag)) {
      convertRealToResultRegister(&rReal, REGISTER_X, amNone);
    }
    else if(getFlag(FLAG_CPXRES)) {
      convertComplexToResultRegister(&rReal, &rImag, REGISTER_X);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "F(φ|m) cannot return complex result without CPXRES set");
        moreInfoOnError("In function fnEllipticFphi:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
  else {
    convertComplexToResultRegister(&rReal, &rImag, REGISTER_X);
  }

  adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
}

void fnEllipticEphi(uint16_t unusedButMandatoryParameter) {
  bool_t   realInput;
  real_t m, uReal, uImag;
  real_t rReal, rImag;

  if(!jacobi_check_inputs(&m, &uReal, &uImag, &realInput)) {
    return;
  }

  ellipticE(&uReal, &uImag, &m, &rReal, &rImag, &ctxtReal39);
  if(realInput) {
    if(realIsZero(&rImag)) {
      convertRealToResultRegister(&rReal, REGISTER_X, amNone);
    }
    else if(getFlag(FLAG_CPXRES)) {
      convertComplexToResultRegister(&rReal, &rImag, REGISTER_X);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "E(φ|m) cannot return complex result without CPXRES set");
        moreInfoOnError("In function fnEllipticEphi:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
  else {
    convertComplexToResultRegister(&rReal, &rImag, REGISTER_X);
  }

  adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
}

void fnJacobiZeta(uint16_t unusedButMandatoryParameter) {
  bool_t   realInput;
  real_t m, uReal, uImag;
  real_t rReal, rImag;

  if(!jacobi_check_inputs(&m, &uReal, &uImag, &realInput)) {
    return;
  }

  jacobiZeta(&uReal, &uImag, &m, &rReal, &rImag, &ctxtReal39);
  if(realInput) {
    if(realIsZero(&rImag)) {
      convertRealToResultRegister(&rReal, REGISTER_X, amNone);
    }
    else if(getFlag(FLAG_CPXRES)) {
      convertComplexToResultRegister(&rReal, &rImag, REGISTER_X);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Ζ(φ|m) cannot return complex result without CPXRES set");
        moreInfoOnError("In function fnJacobiZeta:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
  else {
    convertComplexToResultRegister(&rReal, &rImag, REGISTER_X);
  }

  adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
}
