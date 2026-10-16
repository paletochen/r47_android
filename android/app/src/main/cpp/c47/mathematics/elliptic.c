// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file elliptic.c
 ***********************************************/

#include "c47.h"

#define ELLIPTIC_N 24
#define ELLIPTIC_PI(c)      ((c)->digits > 39 ? const75_pi : const39_pi)
#define ELLIPTIC_PI_ON_2(c) ((c)->digits > 39 ? const75_piOn2 : const39_piOn2)
#define ELLIPTIC_PI_ON_4(c) ((c)->digits > 39 ? const75_piOn4 : const39_piOn4)
#define ELLIPTIC_2_PI(c)    ((c)->digits > 39 ? const75_2pi : const39_2pi)

#if defined(OPTION_ELLIPTIC)


void _ellipseE(void) {
  real_t x, y;
  if(!getRegisterAsReal(REGISTER_Y, &y) || !getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }
  if(!saveLastX()) {
    return;
  }
  real_t reX, a, b, sum;
  realCopyAbs(&x, &x);
  realCopyAbs(&y, &y);
  // ensure a <= b: a is smaller, b is larger
  if(realCompareGreaterThan(&x, &y)) {
    realCopy(&y, &a);
    realCopy(&x, &b);
  }
  else {
    realCopy(&x, &a);
    realCopy(&y, &b);
  }
  realSubtract(&b, &a, &reX, &ctxtReal39);
  realDivide(&reX, &b, &reX, &ctxtReal39);
  realDivide(&a, &b, &sum, &ctxtReal39);
  realAdd(const_1, &sum, &sum, &ctxtReal39);
  realMultiply(&reX, &sum, &reX, &ctxtReal39);
  convertRealToResultRegister(&reX, REGISTER_X, amNone);
  adjustResult(REGISTER_X, false, false, -1, -1, -1);
  temporaryInformation = TI_ELLIPSE_M;
}

void fnEllipse(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexDyadicFunction(&_ellipseE, NULL, &_ellipseE, &_ellipseE);
}

void fnKtoM(uint16_t unusedButMandatoryParameter) {
  fnSquare(NOPARAM);
  temporaryInformation = TI_ELLIPSE_M;
}

void fnMtoK(uint16_t unusedButMandatoryParameter) {
  fnSquareRoot(NOPARAM);
  temporaryInformation = TI_ELLIPSE_K;
}

void fnMtoTheta(uint16_t unusedButMandatoryParameter) {
  fnSquareRoot(NOPARAM);
  fnArcsin(NOPARAM);
  temporaryInformation = TI_ELLIPSE_Theta;
}

void fnThetatoM(uint16_t unusedButMandatoryParameter) {
  fnSin(NOPARAM);
  fnSquare(NOPARAM);
  temporaryInformation = TI_ELLIPSE_M;
}

static void _calc_real_elliptic(real_t *sn, real_t *cn, real_t *dn, const real_t *u, const real_t *m, realContext_t *realContext) {
  real_t a, b, e, f, g;
  real_t *MU, *NU, *C, *D;
  real_t sin_umu, cos_umu, t, r;
  int n = 0;

  if((MU = allocC47Blocks(ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75)))) {
    if((NU = allocC47Blocks(ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75)))) {
      if((C = allocC47Blocks(ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75)))) {
        if((D = allocC47Blocks(ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75)))) {
          #define mu(n) (MU + (n))
          #define nu(n) (NU + (n))
          #define c(n)  (C + (n))
          #define d(n)  (D + (n))

          if(realIsNegative(m) || realCompareLessThan(const_1, m)) {
            realSetNaN(sn);
            realSetNaN(cn);
            realSetNaN(dn);
            freeC47Blocks(MU, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75));
            freeC47Blocks(NU, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75));
            freeC47Blocks(C, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75));
            freeC47Blocks(D, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75));
            return;
          }
          if(realIsZero(m)) {
            C47_WP34S_Cvt2RadSinCosTan(u, amRadian, sn, cn, NULL, realContext);
            realSetOne(dn);
            freeC47Blocks(MU, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75));
            freeC47Blocks(NU, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75));
            freeC47Blocks(C, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75));
            freeC47Blocks(D, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75));
            return;
          }
          realSubtract(m, const_1, &a, realContext);
          if(realIsZero(&a)) {
            WP34S_SinhCosh(u, &a, &b, realContext);
            realDivide(const_1, &b, cn, realContext);
            realMultiply(&a, cn, sn, realContext);
            realCopy(cn, dn);
            freeC47Blocks(MU, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75));
            freeC47Blocks(NU, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75));
            freeC47Blocks(C, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75));
            freeC47Blocks(D, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75));
            return;
          }

          realSubtract(const_1, m, &a, realContext);
          realSquareRoot(&a, &a, realContext);
          n = realAgmStep(const_1, &a, &b, MU, NU, ELLIPTIC_N, realContext);
          if(n == 0) {
            realSetZero(&e);  // No AGM step for |m| below about 1e-71: e = 0 selects the branch below that gives sin u, cos u and 1, the m = 0 limit to 75 digits.
          }
          else {
            realSubtract(mu(n-1), nu(n-1), &e, realContext);
          }

          realMultiply(u, mu(n), &a, realContext);
          C47_WP34S_Cvt2RadSinCosTan(&a, amRadian, &sin_umu, &cos_umu, NULL, realContext);
          realCopyAbs(&cos_umu, &b);
          //if(realCompareAbsLessThan(&sin_umu, &b))
          //  realDivide(&sin_umu, &cos_umu, &t, realContext);
          //else
            realDivide(&cos_umu, &sin_umu, &t, realContext);
          if(realIsZero(&sin_umu)) {
            realSetZero(sn);
            realSetOne(cn);
            realSetOne(dn);
            freeC47Blocks(MU, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75));
            freeC47Blocks(NU, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75));
            freeC47Blocks(C, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75));
            freeC47Blocks(D, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75));
            return;
          }

          realMultiply(mu(n), &t, c(n), realContext);
          realSetOne(d(n));

          while(n > 0) {
            n--;
            realMultiply(d(n+1), c(n+1), c(n), realContext);
            realMultiply(c(n+1), c(n+1), &a, realContext);
            realDivide(&a, mu(n+1), &r, realContext);
            realAdd(&r, nu(n), &a, realContext);
            realAdd(&r, mu(n), &b, realContext);
            realDivide(&a, &b, d(n), realContext);
          }
          realFMA(c(0), c(0), const_1, &f, realContext);   // f = |1 + i c(0)|
          realSquareRoot(&f, &f, realContext);
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

          freeC47Blocks(D, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75));
        }
        else {
          displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
          realSetNaN(sn);
          realSetNaN(cn);
          realSetNaN(dn);
        }
        freeC47Blocks(C, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75));
      }
      else {
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
        realSetNaN(sn);
        realSetNaN(cn);
        realSetNaN(dn);
      }
      freeC47Blocks(NU, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75));
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      realSetNaN(sn);
      realSetNaN(cn);
      realSetNaN(dn);
    }
    freeC47Blocks(MU, ELLIPTIC_N * REAL_SIZE_IN_BLOCKS(75));
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    realSetNaN(sn);
    realSetNaN(cn);
    realSetNaN(dn);
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

static int jacobi_check_inputs(real_t *m, cplx_t *u, bool_t *realInput) {
  bool_t cmplx = false;

  if(!getRegisterAsComplexOrReal(REGISTER_X, u, &cmplx) || !getRegisterAsReal(REGISTER_Y, m) || !saveLastX()) {
    return 0;
  }
  *realInput = !cmplx;
  return 1;
}



static int jacobi_check_inputs_phi(real_t *m, cplx_t *phi, bool_t *realInput) {
  if(getRegisterDataType(REGISTER_X) == dtComplex34) {
    return jacobi_check_inputs(m, phi, realInput);
  }
  angularMode_t xAngularMode;
  if(!getRegisterAsRealAngle(REGISTER_X, &phi->Real, &xAngularMode, !ifLongIntegerDoAngleReduction) || !getRegisterAsReal(REGISTER_Y, m) || !saveLastX()) {
    return 0;
  }

     // const real_t *angle45, *angle90, *angle180;
     // angle45  = const_0;
     // angle90  = const_0;
     // angle180 = const_0;
     // reduceAngleToRange(phiReal, &angle45, &angle90, &angle180, &xAngularMode, ctxtReal75.digits, &ctxtReal75);
     // Removed the angle reduction as the reduction is a lot more complcated than simplistic modulo 2𝝅

  convertAngleFromTo(&phi->Real, xAngularMode, amRadian, &ctxtReal75);
  realSetZero(&phi->Imag);
  *realInput = true;
  return 1;
}

/**
 * Adds 2 pi n to am, the angle of cn + i sn in (-pi, pi], with n = round(u / (4K) - am / (2 pi)) the winding count. For m < 1 am rises by pi each 2K; subtracting the
 * wrapped angle before rounding keeps n exact at an odd multiple of 2K, where u / (4K) alone lands on a half-integer and the round tie disagrees with the atan2 branch.
 */
static void _jacobiAmUnfold(const real_t *u, const real_t *m, real_t *am, realContext_t *realContext) {
  real_t k, t;

  ellipticKE(m, &k, NULL, realContext);
  realDivide(u, &k, &k, realContext);
  realMultiply(&k, const_1on4, &k, realContext);
  realDivide(am, const75_2pi, &t, realContext);
  realSubtract(&k, &t, &k, realContext);
  realToIntegralValue(&k, &k, DEC_ROUND_HALF_EVEN, realContext);
  realFMA(&k, const75_2pi, am, am, realContext);
}

void jacobiElliptic(const real_t *u, const real_t *m, real_t *am, real_t *sn, real_t *cn, real_t *dn, realContext_t *realContext) {
  real_t _sn, _cn;

  if(sn == NULL) {
    sn = &_sn;
  }
  if(cn == NULL) {
    cn = &_cn;
  }
  calc_real_elliptic(sn, cn, dn, u, m, realContext);
  if(am) {
    // For m < 1 am is atan2(sn, cn) unfolded, which keeps its digits next to u = K where asin(sn) folds into [-pi/2, pi/2]. For m >= 1, am stays within
    // asin(1 / sqrt m) of 0 and is asin(sn).
    if(realCompareLessThan(m, const_1)) {
      C47_WP34S_Atan2(sn, cn, am, realContext);
      _jacobiAmUnfold(u, m, am, realContext);
    }
    else {
      C47_WP34S_Asin(sn, am, realContext);
    }
  }
}

void jacobiComplexAm(const cplx_t *u, const real_t *m, cplx_t *r, realContext_t *realContext) {
  if(realCompareLessThan(m, const_1)) {
    // am = -i ln(cn + i sn), its real part unfolded as for a real u
    cplx_t s, c;

    jacobiComplexSn(u, m, &s, realContext);
    jacobiComplexCn(u, m, &c, realContext);
    realSubtract(&c.Real, &s.Imag, &c.Real, realContext);
    realAdd(&c.Imag, &s.Real, &c.Imag, realContext);
    lnComplex(&c, &s, realContext);
    realCopy(&s.Imag, &r->Real);
    realCopy(&s.Real, &r->Imag);
    realChangeSign(&r->Imag);
    _jacobiAmUnfold(&u->Real, m, &r->Real, realContext);
  }
  else {
    jacobiComplexSn(u, m, r, realContext);
    ArcsinComplex(r, r, realContext);
  }
}

void jacobiComplexSn(const cplx_t *u, const real_t *m, cplx_t *r, realContext_t *realContext) {
  // SN(u + i v, m) = sn(u, m) . dn(v, 1-m) / denom
  //                  + i . cn(u, m) . dn(u, m) . sn(v, 1-m) . cn(v, 1-m) / denom
  // where
  //    denom = cn(v, 1-m)^2 + m * dn(u, m)^2 * sn(v, 1-m)^2

  real_t a, b, denom, snuk, cnuk, dnuk, snvki, cnvki, dnvki;
  elliptic_setup_cpx_real(&denom, &snuk, &cnuk, &dnuk, &snvki, &cnvki, &dnvki, &u->Real, &u->Imag, m, realContext);

  realMultiply(&snuk, &dnvki, &a, realContext);
  realDivide(&a, &denom, &r->Real, realContext);

  realMultiply(&cnuk, &dnuk, &a, realContext);
  realMultiply(&a, &snvki, &b, realContext);
  realMultiply(&b, &cnvki, &a, realContext);
  realDivide(&a, &denom, &r->Imag, realContext);
}

void jacobiComplexCn(const cplx_t *u, const real_t *m, cplx_t *r, realContext_t *realContext) {
  // CN(u + i v, m) = cn(u, m) . cn(v, 1-m) / denom
  //                  - i . sn(u, m) . dn(u, m) . sn(v, 1-m) . dn(v, 1-m) / denom
  // where
  //    denom = cn(v, 1-m)^2 + m * dn(u, m)^2 * sn(v, 1-m)^2

  real_t a, b, denom, snuk, cnuk, dnuk, snvki, cnvki, dnvki;
  elliptic_setup_cpx_real(&denom, &snuk, &cnuk, &dnuk, &snvki, &cnvki, &dnvki, &u->Real, &u->Imag, m, realContext);

  realMultiply(&cnuk, &cnvki, &a, realContext);
  realDivide(&a, &denom, &r->Real, realContext);

  realMultiply(&snuk, &dnuk, &a, realContext);
  realMultiply(&a, &snvki, &b, realContext);
  realMultiply(&b, &dnvki, &a, realContext);
  realDivide(&a, &denom, &b, realContext);
  realMinus(&b, &r->Imag, realContext);
}

void jacobiComplexDn(const cplx_t *u, const real_t *m, cplx_t *r, realContext_t *realContext) {
  // DN(u + i v, m) = dn(u, m) . cn(v, 1-m) . dn(v, 1-m) / denom
  //                  - i . m . sn(u, m) . cn(u, m) . sn(v, 1-m) / denom
  // where
  //    denom = cn(v, 1-m)^2 + m * dn(u, m)^2 * sn(v, 1-m)^2

  real_t a, b, denom, snuk, cnuk, dnuk, snvki, cnvki, dnvki;
  elliptic_setup_cpx_real(&denom, &snuk, &cnuk, &dnuk, &snvki, &cnvki, &dnvki, &u->Real, &u->Imag, m, realContext);

  realMultiply(&dnuk, &cnvki, &a, realContext);
  realMultiply(&a, &dnvki, &b, realContext);
  realDivide(&b, &denom, &r->Real, realContext);

  realMinus(m, &b, realContext);
  realMultiply(&b, &snuk, &a, realContext);
  realMultiply(&a, &cnuk, &b, realContext);
  realMultiply(&b, &snvki, &a, realContext);
  realDivide(&a, &denom, &r->Imag, realContext);
}

void ellipticKE(const real_t *m, real_t *k, real_t *e, realContext_t *realContext) {
  if(k) {
    realSetNaN(k);
  }
  if(e) {
    realSetNaN(e);
  }

  if(realCompareLessEqual(m, const_1)) {
    if(realCompareEqual(m, const_1)) {
      if(k) {
        realSetPlusInfinity(k);
      }
      if(e) {
        realSetOne(e);
      }
    }
    else if(realCompareEqual(m, const_0)) {
      if(k) {
        realCopy(ELLIPTIC_PI_ON_2(realContext), k);
      }
      if(e) {
        realCopy(ELLIPTIC_PI_ON_2(realContext), e);
      }
    }
    else {
      real_t a, b;

      realSubtract(const_1, m, &b, realContext);
      realSquareRoot(&b, &b, realContext);
      realCopy(m, &a);
      realAgmForE(const_1, &b, &a, &b, realContext);
      realDivide(ELLIPTIC_PI_ON_2(realContext), &b, &b, realContext);
      if(k) {
        realCopy(&b, k);
      }

      if(e) {
        realSubtract(&a, const_1, &a, realContext);
        realChangeSign(&a);
        realMultiply(&a, &b, &b, realContext);
        realCopy(&b, e);
      }
    }
  }
}

void ellipticComplexKE(const real_t *m, cplx_t *k, cplx_t *e, realContext_t *realContext) {
  if(realCompareLessEqual(m, const_1)) {
    ellipticKE(m, k ? &k->Real : NULL, e ? &e->Real : NULL, realContext);
    if(k) {
      realSetZero(&k->Imag);
    }
    if(e) {
      realSetZero(&e->Imag);
    }
  }
  else if(k && !e) {
    // K(m) = (K(1/m) - i K(1 - 1/m)) / sqrt(m) and K(x) = pi/2 / AGM(1, sqrt(1 - x)) give K(m) = pi/2 / AGM(sqrt(m), sqrt(m - 1)) - i pi/2 / AGM(sqrt(m), 1),
    // as the AGM scales with its arguments. No 1 - 1/m is rounded on the way, which at p39 keeps only 39 - log10(m) digits of 1/m. An infinite m gives the limit 0
    // and a NaN gives NaN, here without the AGM.
    real_t a, b;

    if(realIsSpecial(m)) {
      realDivide(const_1, m, &k->Real, realContext);
      realDivide(const__1, m, &k->Imag, realContext);
    }
    else {
      realSquareRoot(m, &a, realContext);
      realSubtract(m, const_1, &b, realContext);
      realSquareRoot(&b, &b, realContext);
      realAgm(&a, &b, &b, realContext);
      realAgm(&a, const_1, &a, realContext);
      realDivide(ELLIPTIC_PI_ON_2(realContext), &b, &k->Real, realContext);
      realDivide(ELLIPTIC_PI_ON_2(realContext), &a, &k->Imag, realContext);
      realChangeSign(&k->Imag);
    }
  }
  else {
    cplx_t a, c, one;

    realSubtract(m, const_1, &a.Imag, realContext);
    realSquareRoot(&a.Imag, &a.Imag, realContext);
    realSetZero(&a.Real);
    realCopy(m, &c.Real);
    realSetZero(&c.Imag);
    realSetOne(&one.Real);
    realSetZero(&one.Imag);
    complexAgmForE(&one, &a, &c, &a, realContext);
    divRealComplex(ELLIPTIC_PI_ON_2(realContext), &a, &a, realContext);
    if(k) {
      realCopy(&a.Real, &k->Real);
      realCopy(&a.Imag, &k->Imag);
    }

    if(e) {
      realSubtract(&c.Real, const_1, &c.Real, realContext);
      realChangeSign(&c.Real);
      realChangeSign(&c.Imag);
      mulComplexComplex(&a, &c, e, realContext);
    }
  }
}

static void rcSqrt(const real_t *x, cplx_t *res, realContext_t *realContext) {
  real_t xx;

  realCopy(x, &xx);
  if(realIsNegative(&xx)) {
    realSetPositiveSign(&xx);
    realSquareRoot(&xx, &res->Imag, realContext);
    realSetZero(&res->Real);
  }
  else {
    realSquareRoot(&xx, &res->Real, realContext);
    realSetZero(&res->Imag);
  }
}

static void _ellipticFE_lambda_mu(const cplx_t *amplitude, const real_t *m, cplx_t *lambda, cplx_t *mu, realContext_t *realContext) {
  const real_t *phi = &amplitude->Real, *psi = &amplitude->Imag;
  // Abramowitz & Stegun §17.4.11
  real_t m1, b, c, csc2Phi, tan2Phi, cot2Phi, sinh2Psi;
  cplx_t cot2Lambda;

  realSubtract(const_1, m, &m1, realContext);

  C47_WP34S_Cvt2RadSinCosTan(phi, amRadian, &csc2Phi, &cot2Phi, &tan2Phi, realContext);
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

  realMultiply(const__4, &c, &cot2Lambda.Real, realContext);
  realFMA(&b, &b, &cot2Lambda.Real, &cot2Lambda.Real, realContext);
  rcSqrt(&cot2Lambda.Real, &cot2Lambda, realContext);
  realSubtract(&cot2Lambda.Real, &b, &cot2Lambda.Real, realContext);
  realMultiply(&cot2Lambda.Real, const_1on2, &cot2Lambda.Real, realContext);
  realMultiply(&cot2Lambda.Imag, const_1on2, &cot2Lambda.Imag, realContext);

  if(realIsZero(&cot2Lambda.Imag)) {
    realDivide(const_1, &cot2Lambda.Real, &lambda->Real, realContext);
    rcSqrt(&lambda->Real, lambda, realContext);
    if(realIsZero(&lambda->Imag)) {
      C47_WP34S_Atan(&lambda->Real, &lambda->Real, realContext);
    }
    else {
      ArctanComplex(lambda, lambda, realContext);
    }

    if(realIsZero(&cot2Lambda.Real) && realIsZero(&cot2Lambda.Imag)) {
      realSetNegativeOne(&mu->Real);
    }
    else {
      realFMA(&tan2Phi, &cot2Lambda.Real, const__1, &mu->Real, realContext);
    }
    realDivide(&mu->Real, m, &mu->Real, realContext);
    rcSqrt(&mu->Real, mu, realContext);
    if(realIsZero(&mu->Imag)) {
      C47_WP34S_Atan(&mu->Real, &mu->Real, realContext);
    }
    else {
      ArctanComplex(mu, mu, realContext);
    }
  }
  else {
    divRealComplex(const_1, &cot2Lambda, lambda, realContext);
    sqrtComplex(lambda, lambda, realContext);
    ArctanComplex(lambda, lambda, realContext);

    if(realIsZero(&cot2Lambda.Real) && realIsZero(&cot2Lambda.Imag)) {
      realSetNegativeOne(&mu->Real);
      realSetZero(&mu->Imag);
    }
    else {
      realFMA(&tan2Phi, &cot2Lambda.Real, const__1, &mu->Real, realContext);
      realMultiply(&tan2Phi, &cot2Lambda.Imag, &mu->Imag, realContext);
    }
    realDivide(&mu->Real, m, &mu->Real, realContext);
    realDivide(&mu->Imag, m, &mu->Imag, realContext);
    sqrtComplex(mu, mu, realContext);
    ArctanComplex(mu, mu, realContext);
  }
}

static void carlsonRF(const real_t *x0, const real_t *y0, const real_t *z0, real_t *res, realContext_t *realContext) {
  // Carlson RF via duplication, DLMF 19.26.17: RF(x,y,z) = 1/sqrt(A) at convergence
  real_t x, y, z, sx, sy, sz, lam, lam4;
  const real_t *tol = (realContext->digits <= 39) ? const_1e_37 : const_1e_49;
  realCopy(x0, &x);
  realCopy(y0, &y);
  realCopy(z0, &z);
  do {
    realSquareRoot(&x, &sx, realContext);
    realSquareRoot(&y, &sy, realContext);
    realSquareRoot(&z, &sz, realContext);
    realMultiply(&sx, &sy, &lam, realContext);
    realFMA(&sy, &sz, &lam, &lam, realContext);             // lam = sqrt(xy)+sqrt(yz)+sqrt(zx)
    realFMA(&sz, &sx, &lam, &lam, realContext);
    realMultiply(&lam, const_1on4, &lam4, realContext);     // lam4 = lam/4
    realFMA(&x, const_1on4, &lam4, &x, realContext);        // x = x/4 + lam/4
    realFMA(&y, const_1on4, &lam4, &y, realContext);        // y = y/4 + lam/4
    realFMA(&z, const_1on4, &lam4, &z, realContext);        // z = z/4 + lam/4
  } while(!WP34S_RelativeError(&x, &y, tol, realContext) || !WP34S_RelativeError(&x, &z, tol, realContext));
  realSquareRoot(&x, res, realContext);
  realDivide(const_1, res, res, realContext);
}

static void carlsonRD(const real_t *x0, const real_t *y0, const real_t *z0, real_t *res, realContext_t *realContext) {
  // Carlson RD via duplication algorithm, Carlson 1995 eq. 2.7
  // RD(x,y,z) = 3*sum_{k} fac_k/(sz_k*(z_k+lam_k)) + fac_n/A^(3/2)
  real_t x, y, z, sx, sy, sz, lam, lam4, sum, fac, t;
  const real_t *tol = (realContext->digits <= 39) ? const_1e_37 : const_1e_49;
  realCopy(x0, &x);
  realCopy(y0, &y);
  realCopy(z0, &z);
  realSetZero(&sum);
  realSetOne(&fac);
  do {
    realSquareRoot(&x, &sx, realContext);
    realSquareRoot(&y, &sy, realContext);
    realSquareRoot(&z, &sz, realContext);
    realMultiply(&sx, &sy, &lam, realContext);
    realFMA(&sy, &sz, &lam, &lam, realContext);             // lam = sqrt(xy)+sqrt(yz)+sqrt(zx)
    realFMA(&sz, &sx, &lam, &lam, realContext);
    realAdd(&z, &lam, &t, realContext);                     // t = z + lam
    realMultiply(&t, &sz, &t, realContext);                 // t = sz*(z+lam)
    realDivide(&fac, &t, &t, realContext);                  // t = fac/(sz*(z+lam))
    realAdd(&sum, &t, &sum, realContext);                   // sum += fac/(sz*(z+lam))
    realMultiply(&lam, const_1on4, &lam4, realContext);     // lam4 = lam/4
    realFMA(&x, const_1on4, &lam4, &x, realContext);        // x = x/4 + lam/4
    realFMA(&y, const_1on4, &lam4, &y, realContext);        // y = y/4 + lam/4
    realFMA(&z, const_1on4, &lam4, &z, realContext);        // z = z/4 + lam/4
    realMultiply(&fac, const_1on4, &fac, realContext);      // fac /= 4
  } while(!WP34S_RelativeError(&x, &y, tol, realContext) || !WP34S_RelativeError(&x, &z, tol, realContext));
  realSquareRoot(&x, &sx, realContext);                     // sx = sqrt(A)
  realMultiply(&x, &sx, res, realContext);                  // res = A^(3/2)
  realDivide(&fac, res, res, realContext);                  // res = fac/A^(3/2)
  realFMA(const_3, &sum, res, res, realContext);            // RD = 3*sum + fac/A^(3/2)
}

/**
 * Calculates Carlson RC(1, 1+x) for x greater than -1.
 */
static void carlsonRC1p(const real_t *x, real_t *res, realContext_t *realContext) {
  real_t root;

  if(realIsZero(x)) {
    realSetOne(res);
  }
  else if(realIsPositive(x)) {
    realSquareRoot(x, &root, realContext);
    C47_WP34S_Atan(&root, res, realContext);
    realDivide(res, &root, res, realContext);
  }
  else {
    realCopyAbs(x, &root);
    realSquareRoot(&root, &root, realContext);
    WP34S_ArcTanh(&root, res, realContext);
    realDivide(res, &root, res, realContext);
  }
}

/**
 * Calculates Carlson RJ for nonnegative x, y, z and positive p.
 */
static void carlsonRJ(const real_t *x0, const real_t *y0, const real_t *z0, const real_t *p0, real_t *res, realContext_t *realContext) {
  real_t x, y, z, p, sx, sy, sz, sp, lam, dm, em, rc, sum, fac, a, b, delta;
  const real_t *tol = realContext->digits <= 39 ? const_1e_37 : const_1e_49;

  realCopy(x0, &x);
  realCopy(y0, &y);
  realCopy(z0, &z);
  realCopy(p0, &p);
  realSubtract(p0, x0, &delta, realContext);
  realSubtract(p0, y0, &a, realContext);
  realMultiply(&delta, &a, &delta, realContext);
  realSubtract(p0, z0, &a, realContext);
  realMultiply(&delta, &a, &delta, realContext);
  realSetZero(&sum);
  realSetOne(&fac);

  do {
    realSquareRoot(&x, &sx, realContext);
    realSquareRoot(&y, &sy, realContext);
    realSquareRoot(&z, &sz, realContext);
    realSquareRoot(&p, &sp, realContext);
    realMultiply(&sx, &sy, &lam, realContext);
    realFMA(&sx, &sz, &lam, &lam, realContext);
    realFMA(&sy, &sz, &lam, &lam, realContext);

    realAdd(&sp, &sx, &dm, realContext);
    realAdd(&sp, &sy, &a, realContext);
    realMultiply(&dm, &a, &dm, realContext);
    realAdd(&sp, &sz, &a, realContext);
    realMultiply(&dm, &a, &dm, realContext);
    realMultiply(&fac, &fac, &em, realContext);
    realMultiply(&em, &fac, &em, realContext);
    realMultiply(&em, &delta, &em, realContext);
    realMultiply(&dm, &dm, &a, realContext);
    realDivide(&em, &a, &em, realContext);
    carlsonRC1p(&em, &rc, realContext);
    realMultiply(&fac, &rc, &a, realContext);
    realDivide(&a, &dm, &a, realContext);
    realAdd(&sum, &a, &sum, realContext);

    realMultiply(&lam, const_1on4, &a, realContext);
    realFMA(&x, const_1on4, &a, &x, realContext);
    realFMA(&y, const_1on4, &a, &y, realContext);
    realFMA(&z, const_1on4, &a, &z, realContext);
    realFMA(&p, const_1on4, &a, &p, realContext);
    realMultiply(&fac, const_1on4, &fac, realContext);
  } while(!WP34S_RelativeError(&x, &y, tol, realContext) ||
          !WP34S_RelativeError(&x, &z, tol, realContext) ||
          !WP34S_RelativeError(&x, &p, tol, realContext));

  realAdd(&x, &y, &a, realContext);
  realAdd(&a, &z, &a, realContext);
  realFMA(&p, const_2, &a, &a, realContext);
  realDivide(&a, const_5, &a, realContext);
  realSquareRoot(&a, &b, realContext);
  realMultiply(&a, &b, &a, realContext);
  realDivide(&fac, &a, res, realContext);
  realFMA(const_6, &sum, res, res, realContext);
}

static void _ellipticF(const real_t *phi, const real_t *m, real_t *res, realContext_t *realContext) {
  // F(phi, m) = sin(phi) * RF(cos^2(phi), 1 - m*sin^2(phi), 1)
  real_t s, c, s2, y;
  C47_WP34S_Cvt2RadSinCosTan(phi, amRadian, &s, &c, NULL, realContext);
  realMultiply(&s, &s, &s2, realContext);                    // s2 = sin²phi
  realMultiply(&c, &c, &c, realContext);                     // c  = cos²phi
  realMultiply(m, &s2, &y, realContext);
  realSubtract(const_1, &y, &y, realContext);                // y = 1 - m*sin²phi
  carlsonRF(&c, &y, const_1, res, realContext);              // RF(cos²,1-m*sin²,1)
  realMultiply(&s, res, res, realContext);                   // F = sin*RF
}

/**
 * Calculates the real incomplete elliptic integral of the second kind on the reduced interval.
 */
static void _ellipticE(const real_t *phi, const real_t *m, real_t *res, realContext_t *realContext) {
  real_t s, c, s2, y, rf, rd, a;

  C47_WP34S_Cvt2RadSinCosTan(phi, amRadian, &s, &c, NULL, realContext);
  realMultiply(&s, &s, &s2, realContext);
  realMultiply(&c, &c, &c, realContext);
  realMultiply(m, &s2, &y, realContext);
  realSubtract(const_1, &y, &y, realContext);
  carlsonRF(&c, &y, const_1, &rf, realContext);
  carlsonRD(&c, &y, const_1, &rd, realContext);
  realMultiply(&s, &rf, res, realContext);
  realMultiply(&s2, &s, &a, realContext);
  realMultiply(&a, m, &a, realContext);
  realDivide(&a, const_3, &a, realContext);
  realChangeSign(&a);
  realFMA(&a, &rd, res, res, realContext);
}

/**
 * Returns true when m sin^2(phi) is at most 1, where the Carlson forms of F(phi|m) and E(phi|m) are real. The product is rounded as in _ellipticF and _ellipticE,
 * so their 1 - m sin^2(phi) is not negative when this returns true.
 */
static bool_t _ellipticCarlsonReal(const real_t *phi, const real_t *m, realContext_t *realContext) {
  REAL_T_PTR(s, 75);

  C47_WP34S_Cvt2RadSinCosTan(phi, amRadian, s, NULL, NULL, realContext);
  realMultiply(s, s, s, realContext);
  realMultiply(m, s, s, realContext);
  return !realCompareGreaterThan(s, const_1);
}

static void _ellipticF_1(const real_t *phi, const real_t *m, real_t *res, realContext_t *realContext) {
  // Carlson RF handles full [0,pi/2] range without cancellation; no transformation needed.
  _ellipticF(phi, m, res, realContext);
}

static void _ellipticF_2(const real_t *phi, const real_t *m, real_t *res, realContext_t *realContext) {
  // assumes phi is real and 0 ≤ m ≤ 1
  // Abramowitz & Stegun §17.4.1, §17.4.3
  real_t phi1, phiQuotient, phiRemainder;
  bool_t   remainderNegative = false;

  realCopyAbs(phi, &phi1);
  realDivide(&phi1, ELLIPTIC_PI(realContext), &phiQuotient, realContext);
  realToIntegralValue(&phiQuotient, &phiQuotient, DEC_ROUND_DOWN, realContext);
  realDivideRemainder(&phi1, ELLIPTIC_PI(realContext), &phiRemainder, realContext);
  if(realCompareGreaterThan(&phiRemainder, ELLIPTIC_PI_ON_2(realContext))) {
    realAdd(&phiQuotient, const_1, &phiQuotient, realContext);
    realSubtract(ELLIPTIC_PI(realContext), &phiRemainder, &phiRemainder, realContext);
    remainderNegative = true;
  }

  if(realIsZero(&phiQuotient)) {
    _ellipticF_1(&phiRemainder, m, res, realContext);
  }
  else {
    realAdd(&phiQuotient, &phiQuotient, &phiQuotient, realContext);
    ellipticKE(m, &phi1, NULL, realContext);
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

static void _ellipticF_3(const real_t *phi, const real_t *m, cplx_t *res, realContext_t *realContext) {
  if(realIsZero(m)) {
    // Abramowitz & Stegun §17.4.19-20
    realCopy(phi, &res->Real);
    realSetZero(&res->Imag);
  }
else if(realCompareGreaterThan(m, const_1)) {
    // Abramowitz & Stegun §17.4.15
    // Use one precision level above caller to absorb cancellation in ArcsinComplex for |sqrt(m)*sin(phi)| > 1, which calculates complex theta with precision loss.
    real_t m_1;
    cplx_t k, a, theta;
    realContext_t *hCtx = &ctxtReal75;
    mod2Pi(phi, &theta.Real, hCtx);
    C47_WP34S_Cvt2RadSinCosTan(&theta.Real, amRadian, &a.Real, NULL, NULL, hCtx);
    realSquareRoot(m, &k.Real, hCtx);
    realSetZero(&k.Imag);
    realDivide(const_1, m, &m_1, hCtx);
    realMultiply(&k.Real, &a.Real, &a.Real, hCtx);
    realSetZero(&a.Imag);
    ArcsinComplex(&a, &theta, hCtx);
    ellipticF(&theta, &m_1, res, hCtx);                          // recurses with m_1=1/m
    divComplexComplex(res, &k, res, hCtx);
    realPlus(&res->Real, &res->Real, realContext);               // round back to caller precision
    realPlus(&res->Imag, &res->Imag, realContext);
  }  else if(realIsPositive(m)) {
    _ellipticF_2(phi, m, &res->Real, realContext);
    realSetZero(&res->Imag);
  }
  else {
    _ellipticF(phi, m, &res->Real, realContext);
    realSetZero(&res->Imag);
  }
}

static void _ellipticF_4(const cplx_t *amplitude, const real_t *m, cplx_t *res, realContext_t *realContext) {
  const real_t *phi = &amplitude->Real, *psi = &amplitude->Imag;
  if(realIsZero(psi)) {
    _ellipticF_3(phi, m, res, realContext);
  }
  else if(realIsZero(phi)) {
    // Abramowitz & Stegun §17.4.8
    real_t m1, theta;

    realSubtract(const_1, m, &m1, realContext);
    GudermannianReal(psi, &theta, realContext);
    _ellipticF_3(&theta, &m1, res, realContext);
    mulComplexi(res, res);
  }
  else {
    // Abramowitz & Stegun §17.4.11
    real_t m1;
    cplx_t lambda, mu, f;

    realSubtract(const_1, m, &m1, realContext);
    _ellipticFE_lambda_mu(amplitude, m, &lambda, &mu, realContext);

    if(realIsZero(&lambda.Imag)) {
      _ellipticF_3(&lambda.Real, m, &f, realContext);
    }
    else {
      ellipticF(&lambda, m, &f, realContext);               // recurses here
    }
    realCopy(&f.Real, &res->Real);
    realCopy(&f.Imag, &res->Imag);
    if(realIsZero(&mu.Imag)) {
      _ellipticF_3(&mu.Real, &m1, &f, realContext);
    }
    else {
      _ellipticF_4(&mu, &m1, &f, realContext);           // recurses here
    }
    realAdd(&res->Imag, &f.Real, &res->Imag, realContext);
    realSubtract(&res->Real, &f.Imag, &res->Real, realContext);
  }
}

static void _ellipticF_5(const cplx_t *amplitude, const real_t *m, cplx_t *res, realContext_t *realContext) {
  const real_t *phi = &amplitude->Real, *psi = &amplitude->Imag;
  if(realIsInfinite(psi) && !realIsZero(phi)) {
    // F(phi + i inf | m) = i K(1 - m) for 0 < phi <= pi/2, the limit of Abramowitz & Stegun §17.4.8 at phi = 0 from the right: far above the real axis the
    // integrand falls as e^-Im, so the limit does not depend on phi. phi = 0 takes §17.4.8 in _ellipticF_4, as for m < 0 the imaginary axis is a cut where F
    // is odd. ellipticF sets the sign of the imaginary part and adds the half turns as for a finite psi.
    real_t m1;
    cplx_t k;

    realSubtract(const_1, m, &m1, realContext);
    ellipticComplexKE(&m1, &k, NULL, realContext);
    realMinus(&k.Imag, &res->Real, realContext);
    realCopy(&k.Real, &res->Imag);
    return;
  }
  // Abramowitz & Stegun §17.4.13
  if(realCompareAbsGreaterThan(phi, ELLIPTIC_PI_ON_4(realContext))) {
    real_t m1;
    cplx_t psiZ, k1, tanPhi, k;
    if(realIsZero(psi) && realCompareAbsLessThan(phi, ELLIPTIC_PI_ON_2(realContext)) && (!realCompareGreaterThan(m, const_1) || _ellipticCarlsonReal(phi, m, realContext))) {
      // Real phi in (pi/4, pi/2), m sin^2(phi) <= 1: Carlson RF avoids K-F cancellation.
      _ellipticF(phi, m, &res->Real, realContext);
      realSetZero(&res->Imag);
      return;
    }
    ellipticComplexKE(m, &k, NULL, realContext);
    if(realIsNegative(psi)) {
      realChangeSign(&k.Imag); // conjugate
    }
    realCopy(phi, &tanPhi.Real);
    realCopy(psi, &tanPhi.Imag);
    TanComplex(&tanPhi, &tanPhi, realContext);
    realSubtract(const_1, m, &m1, realContext);
    rcSqrt(&m1, &k1, realContext);
    mulComplexComplex(&k1, &tanPhi, &psiZ, realContext);
    divRealComplex(const_1, &psiZ, &psiZ, realContext);
    if(realIsZero(&psiZ.Imag)) {
      C47_WP34S_Atan(&psiZ.Real, &psiZ.Real, realContext);
    }
    else {
      ArctanComplex(&psiZ, &psiZ, realContext);
    }
    _ellipticF_4(&psiZ, m, res, realContext);

    if(!realIsZero(&k.Imag)) {
      if(realIsPositive(&k.Imag)) {
        realSetPositiveSign(&res->Imag);
      }
      else {
        realSetNegativeSign(&res->Imag);
      }
    }
    realSubtract(&k.Real, &res->Real, &res->Real, realContext);
    realSubtract(&k.Imag, &res->Imag, &res->Imag, realContext);

    if(realIsPositive(psi)) {
      realSetPositiveSign(&res->Imag);
    }
    else {
      realSetNegativeSign(&res->Imag);
    }
    if(!realIsZero(&k.Imag) && realIsNegative(&k.Imag)) {
      realChangeSign(&res->Imag);
    }
  }
  else {
    _ellipticF_4(amplitude, m, res, realContext);
  }
}

/**
 * Sets F(phi|m), E(phi|m) or Z(phi|m) for a NaN or infinite amplitude or parameter: NaN, with a zero imaginary part when the amplitude is real. With grows set,
 * a real amplitude of +-inf and a finite m of at most 1 give +-inf, since F and E rise by 2K(m) or 2E(m) each half turn there.
 */
static void _ellipticSpecialArguments(const cplx_t *amplitude, const real_t *m, bool_t grows, cplx_t *res) {
  const real_t *phi = &amplitude->Real, *psi = &amplitude->Imag;

  if(grows && realIsInfinite(phi) && realIsZero(psi) && !realIsSpecial(m) && realCompareLessEqual(m, const_1)) {
    realCopy(phi, &res->Real);
  }
  else {
    realSetNaN(&res->Real);
  }
  if(realIsZero(psi)) {
    realSetZero(&res->Imag);
  }
  else {
    realSetNaN(&res->Imag);
  }
}

void ellipticF(const cplx_t *amplitude, const real_t *m, cplx_t *res, realContext_t *realContext) {
  const real_t *phi = &amplitude->Real, *psi = &amplitude->Imag;
  // Abramowitz & Stegun §17.4.1, §17.4.3
  real_t phi1, phiQuotient;
  cplx_t reduced;
  bool_t   remainderNegative = false;

  // _ellipticF_5 and _ellipticF_4 hand a NaN amplitude back to ellipticF, which would repeat without end. An infinite psi with a finite phi and m takes the
  // route below, where _ellipticF_5 returns its limit.
  if(realIsSpecial(phi) || realIsNaN(psi) || realIsSpecial(m)) {
    _ellipticSpecialArguments(amplitude, m, true, res);
    return;
  }
  realCopy(phi, &phi1);
  realCopy(psi, &reduced.Imag);
  if(realIsNegative(phi)) {
    realChangeSign(&phi1);
    realChangeSign(&reduced.Imag);
  }
  realDivide(&phi1, ELLIPTIC_PI(realContext), &phiQuotient, realContext);
  realToIntegralValue(&phiQuotient, &phiQuotient, DEC_ROUND_DOWN, realContext);
  realDivideRemainder(&phi1, ELLIPTIC_PI(realContext), &reduced.Real, realContext);
  if(realCompareGreaterThan(&reduced.Real, ELLIPTIC_PI_ON_2(realContext))) {
    realAdd(&phiQuotient, const_1, &phiQuotient, realContext);
    realSubtract(ELLIPTIC_PI(realContext), &reduced.Real, &reduced.Real, realContext);
    remainderNegative = true;
  }

  if(realIsZero(phi) && realIsZero(psi)) {
    realSetZero(&res->Real);
    realSetZero(&res->Imag);
    return;
  }
  else if(realIsZero(m)) {
    realCopy(phi, &res->Real);
    realCopy(psi, &res->Imag);
    return;
  }
  else if(realCompareEqual(m, const_1)) {
    cplx_t p;
    realFMA(phi, const_1on2, ELLIPTIC_PI_ON_4(realContext), &p.Real, realContext);
    realMultiply(psi, const_1on2, &p.Imag, realContext);
    if(realIsZero(&p.Imag)) {
      C47_WP34S_Cvt2RadSinCosTan(&p.Real, amRadian, &res->Real, &res->Imag, &p.Imag, realContext);
      WP34S_Ln(&p.Imag, &res->Real, realContext);
      realSetZero(&res->Imag);
    }
    else {
      TanComplex(&p, &p, realContext);
      lnComplex(&p, res, realContext);
      if(realIsZero(&p.Real)) {
        realSetZero(&res->Real);
      }
    }
    return;
  }
  else if(realIsZero(&phiQuotient)) {
    _ellipticF_5(&reduced, m, res, realContext);
    if(remainderNegative) {
      realChangeSign(&res->Real);
      realChangeSign(&res->Imag);
    }
    if(realIsNegative(phi)) {
      realChangeSign(&res->Real);
      realChangeSign(&res->Imag);
    }
  }
  else {
    cplx_t k;

    realAdd(&phiQuotient, &phiQuotient, &phiQuotient, realContext);
    _ellipticF_5(&reduced, m, res, realContext);
    if(remainderNegative) {
      realChangeSign(&res->Real);
      realChangeSign(&res->Imag);
    }
    if(!realIsZero(psi)) {
      realSetSignFrom(&reduced.Imag, &res->Imag);
    }
    ellipticComplexKE(m, &k, NULL, realContext);
    realFMA(&phiQuotient, &k.Real, &res->Real, &res->Real, realContext);
    realFMA(&phiQuotient, &k.Imag, &res->Imag, &res->Imag, realContext);
    if(realIsNegative(phi)) {
      realChangeSign(&res->Real);
      realChangeSign(&res->Imag);
    }
  }

  if(realCompareGreaterThan(m, const_1) && (realCompareAbsGreaterThan(phi, ELLIPTIC_PI_ON_2(realContext)) || realIsZero(psi))) {
    if(realIsPositive(phi)) {
      realSetNegativeSign(&res->Imag);
    }
    else {
      realSetPositiveSign(&res->Imag);
    }
  }
  else {
    if(realIsPositive(psi)) {
      realSetPositiveSign(&res->Imag);
    }
    else if(realIsNegative(psi)) {
      realSetNegativeSign(&res->Imag);
    }
    else if(realIsPositive(psi)) {
      realSetPositiveSign(&res->Imag);
    }
    else if(realIsNegative(psi)) {
      realSetNegativeSign(&res->Imag);
    }
  }
}

static void _ellipticE_fromZeta(const cplx_t *amplitude, const real_t *m, cplx_t *res, realContext_t *realContext) {
  // Compute from Jacobi Zeta
  cplx_t k, e, f, z;

  ellipticComplexKE(m, &k, &e, realContext);
  ellipticF(amplitude, m, &f, realContext);
  jacobiZeta(amplitude, m, &z, realContext);
  divComplexComplex(&e, &k, res, realContext);
  mulComplexComplex(res, &f, res, realContext);
  realAdd(&res->Real, &z.Real, &res->Real, realContext);
  realAdd(&res->Imag, &z.Imag, &res->Imag, realContext);
}

void ellipticE(const cplx_t *amplitude, const real_t *m, cplx_t *res, realContext_t *realContext) {
  const real_t *phi = &amplitude->Real, *psi = &amplitude->Imag;
  real_t phi1, phiQuotient;
  cplx_t reduced;

  if(realIsSpecial(phi) || realIsSpecial(psi) || realIsSpecial(m)) {
    _ellipticSpecialArguments(amplitude, m, true, res);
    return;
  }
  realCopyAbs(phi, &phi1);
  realCopyAbs(psi, &reduced.Imag);
  realDivide(&phi1, ELLIPTIC_PI(realContext), &phiQuotient, realContext);
  realToIntegralValue(&phiQuotient, &phiQuotient, DEC_ROUND_DOWN, realContext);
  realDivideRemainder(&phi1, ELLIPTIC_PI(realContext), &reduced.Real, realContext);
  if(realCompareGreaterThan(&reduced.Real, ELLIPTIC_PI_ON_2(realContext))) {
    realAdd(&phiQuotient, const_1, &phiQuotient, realContext);
    realSubtract(&reduced.Real, ELLIPTIC_PI(realContext), &reduced.Real, realContext);
  }
  if(realIsNegative(phi)) {
    realChangeSign(&phi1);
    realChangeSign(&phiQuotient);
    realChangeSign(&reduced.Real);
    realChangeSign(&reduced.Imag);
  }

  if(realIsZero(m)) {
    realCopy(phi, &res->Real);
    realCopy(psi, &res->Imag);
  }
  else if(realCompareEqual(m, const_1)) {
    if(realIsZero(psi)) {
      C47_WP34S_Cvt2RadSinCosTan(&reduced.Real, amRadian, &res->Real, NULL, NULL, realContext);
      realSetZero(&res->Imag);
      realAdd(&res->Real, &phiQuotient, &res->Real, realContext);
      realAdd(&res->Real, &phiQuotient, &res->Real, realContext);
    }
    else {
      realCopy(&reduced.Real, &res->Real);
      realCopy(psi, &res->Imag);
      sinComplex(res, res, realContext);
      realAdd(&res->Real, &phiQuotient, &res->Real, realContext);
      realAdd(&res->Real, &phiQuotient, &res->Real, realContext);
    }
  }
  else if(realIsZero(psi) && realCompareGreaterThan(m, const_1) && (!realIsZero(&phiQuotient) || !_ellipticCarlsonReal(phi, m, realContext))) {
    // For |phi| <= pi/2 and m sin^2(phi) <= 1 E is real, and the Carlson form below returns it where E from Z leaves an imaginary part. Beyond that E is complex.
    cplx_t realPhi;

    realCopy(phi, &realPhi.Real);
    realSetZero(&realPhi.Imag);
    _ellipticE_fromZeta(&realPhi, m, res, realContext);
  }
  else if(realIsZero(psi)) {
    real_t e;
    _ellipticE(&reduced.Real, m, &res->Real, realContext);
    if(!realIsZero(&phiQuotient)) {
      ellipticKE(m, NULL, &e, realContext);
      realAdd(&phiQuotient, &phiQuotient, &phiQuotient, realContext);
      realFMA(&phiQuotient, &e, &res->Real, &res->Real, realContext);
    }
    realSetZero(&res->Imag);
  }
  else if(realIsZero(phi)) {
    real_t m1;
    cplx_t a, c, theta;

    GudermannianReal(psi, &theta.Real, realContext);
    realSetZero(&theta.Imag);
    realSubtract(const_1, m, &m1, realContext);

    C47_WP34S_Cvt2RadSinCosTan(&theta.Real, amRadian, &c.Real, &c.Imag, &a.Real, realContext);

    realMultiply(&c.Real, &c.Real, &c.Real, realContext);
    realMultiply(&c.Real, &m1, &c.Real, realContext);
    realSubtract(const_1, &c.Real, &c.Real, realContext);
    rcSqrt(&c.Real, &c, realContext);
    realSetZero(&a.Imag);
    mulComplexComplex(&a, &c, &a, realContext);

    ellipticF(&theta, &m1, &c, realContext);
    realAdd(&a.Real, &c.Real, &a.Real, realContext);
    realAdd(&a.Imag, &c.Imag, &a.Imag, realContext);
    ellipticE(&theta, &m1, &c, realContext);
    realSubtract(&a.Real, &c.Real, &a.Real, realContext);
    realSubtract(&a.Imag, &c.Imag, &a.Imag, realContext);

    mulComplexi(&a, res);
  }
  else {
    cplx_t *tmpVal;

    #define LAMBDA        (tmpVal + 0)
    #define MU            (tmpVal + 1)
    #define B1            (tmpVal + 2)
    #define B2            (tmpVal + 3)
    #define B3            (tmpVal + 4)
    #define SIN_LAMBDA    (tmpVal + 5)
    #define SIN2_LAMBDA   (tmpVal + 6)
    #define COS_LAMBDA    (tmpVal + 7)
    #define SIN_MU        (tmpVal + 8)
    #define SIN2_MU       (tmpVal + 9)
    #define COS_MU        (tmpVal + 10)
    #define COS2_MU       (tmpVal + 11)
    #define M1            (&tmpVal[12].Real)  // the real half of slot 12, the last real of the block

    if((tmpVal = allocC47Blocks(25 * REAL_SIZE_IN_BLOCKS(75)))) {
      bool_t           remainderNegative = realIsNegative(&reduced.Real);
      realContext_t *realContext2 = &ctxtReal75;
      realContext_t *realContext3 = &ctxtReal75;

      if(remainderNegative) {
        realChangeSign(&reduced.Real);
      }
      realSubtract(const_1, m, M1, realContext2);
      _ellipticFE_lambda_mu(&reduced, m, LAMBDA, MU, realContext2);
      sinComplex(LAMBDA, SIN_LAMBDA, realContext2);
      cosComplex(LAMBDA, COS_LAMBDA, realContext2);
      sinComplex(MU, SIN_MU, realContext2);
      cosComplex(MU, COS_MU, realContext2);
      mulComplexComplex(SIN_LAMBDA, SIN_LAMBDA, SIN2_LAMBDA, realContext2);
      mulComplexComplex(SIN_MU, SIN_MU, SIN2_MU, realContext2);
      mulComplexComplex(COS_MU, COS_MU, COS2_MU, realContext2);

      realMultiply(m, &SIN2_LAMBDA->Real, &B1->Real, realContext2);
      realMultiply(m, &SIN2_LAMBDA->Imag, &B1->Imag, realContext2);
      realSubtract(const_1, &B1->Real, &B2->Real, realContext2);
      realSubtract(const_0, &B1->Imag, &B2->Imag, realContext2);
      sqrtComplex(B2, B1, realContext2);
      mulComplexComplex(SIN2_MU, B1, B1, realContext2);
      mulComplexComplex(COS_LAMBDA, B1, B1, realContext2);
      mulComplexComplex(SIN_LAMBDA, B1, B1, realContext2);
      realMultiply(m, &B1->Real, &B1->Real, realContext2);
      realMultiply(m, &B1->Imag, &B1->Imag, realContext2);

      realMultiply(M1, &SIN2_MU->Real, &B3->Real, realContext2);
      realMultiply(M1, &SIN2_MU->Imag, &B3->Imag, realContext2);
      realSubtract(const_1, &B3->Real, &B3->Real, realContext2);
      realSubtract(const_0, &B3->Imag, &B3->Imag, realContext2);
      sqrtComplex(B3, B3, realContext2);
      mulComplexComplex(B2, B3, B2, realContext2);
      mulComplexComplex(B2, SIN_MU, B2, realContext2);
      mulComplexComplex(B2, COS_MU, B2, realContext2);

      realMultiply(m, &SIN2_LAMBDA->Real, &B3->Real, realContext2);
      realMultiply(m, &SIN2_LAMBDA->Imag, &B3->Imag, realContext2);
      mulComplexComplex(B3, SIN2_MU, B3, realContext2);
      realAdd(&COS2_MU->Real, &B3->Real, &B3->Real, realContext2);
      realAdd(&COS2_MU->Imag, &B3->Imag, &B3->Imag, realContext2);

      realSubtract(&B1->Real, &B2->Imag, &res->Real, realContext2);
      realAdd(&B1->Imag, &B2->Real, &res->Imag, realContext2);
      divComplexComplex(res, B3, res, realContext2);

      ellipticE(LAMBDA, m, B1, realContext3);
      ellipticE(MU, M1, B2, realContext3);
      ellipticF(MU, M1, B3, realContext3);

      realSubtract(&res->Real, &B3->Imag, &res->Real, realContext);
      realAdd(&res->Imag, &B3->Real, &res->Imag, realContext);
      realAdd(&res->Real, &B2->Imag, &res->Real, realContext);
      realSubtract(&res->Imag, &B2->Real, &res->Imag, realContext);
      realAdd(&res->Real, &B1->Real, &res->Real, realContext);
      realAdd(&res->Imag, &B1->Imag, &res->Imag, realContext);

      if(remainderNegative) {
        realChangeSign(&res->Real);
      }

      if(realIsNegative(psi)) {
        realChangeSign(&res->Imag);
      }

      if(!realIsZero(&phiQuotient)) {
        ellipticComplexKE(m, NULL, B1, realContext);
        realAdd(&phiQuotient, &phiQuotient, &phiQuotient, realContext);
        realMultiply(&phiQuotient, &B1->Real, &B1->Real, realContext);
        realMultiply(&phiQuotient, &B1->Imag, &B1->Imag, realContext);
        realAdd(&res->Real, &B1->Real, &res->Real, realContext);
        realAdd(&res->Imag, &B1->Imag, &res->Imag, realContext);
      }

      freeC47Blocks(tmpVal, 25 * REAL_SIZE_IN_BLOCKS(75));
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      realSetNaN(&res->Real);
      realSetNaN(&res->Imag);
    }

    #undef LAMBDA
    #undef MU
    #undef B1
    #undef B2
    #undef B3
    #undef SIN_LAMBDA
    #undef SIN2_LAMBDA
    #undef COS_LAMBDA
    #undef SIN_MU
    #undef SIN2_MU
    #undef COS_MU
    #undef COS2_MU
    #undef M1
  }
}

static void _jacobiZeta_Agm(const cplx_t *amplitude, const real_t *m, cplx_t *res, realContext_t *realContext) {
  cplx_t *a, *b;
  int n;

  if(realIsZero(m)) {
    realSetZero(&res->Real);
    realSetZero(&res->Imag);
    return;
  }
  if(realCompareEqual(m, const_1)) {
    cplx_t k;
    ellipticF(amplitude, m, &k, realContext);
    TanhComplex(&k, res, realContext);
    return;
  }
  if((a = allocC47Blocks(ELLIPTIC_N * 2 * REAL_SIZE_IN_BLOCKS(75)))) {
    if((b = allocC47Blocks(ELLIPTIC_N * 2 * REAL_SIZE_IN_BLOCKS(75)))) {
      cplx_t k, c, s;
      real_t q;

      realSetZero(&res->Real);
      realSetZero(&res->Imag);
      realSubtract(const_1, m, &k.Real, realContext);
      rcSqrt(&k.Real, &k, realContext);
      realSetOne(&s.Real);  // s takes 1 + 0i, the first AGM argument, until sinComplex() below
      realSetZero(&s.Imag);
      n = complexAgmStep(&s, &k, &c, a, b, ELLIPTIC_N, realContext);
      ellipticF(amplitude, m, &k, realContext);
      mulComplexComplex(&k, a + n, &k, realContext);
      for(int i = 0; i < n; ++i) {
        realAdd(&k.Real, &k.Real, &k.Real, realContext);
        realAdd(&k.Imag, &k.Imag, &k.Imag, realContext);
      }
      for(int i = n; i > 0; --i) {
        realCopy(&k.Real, &q);
        mod2Pi(&q, &k.Real, realContext);
        if(realCompareGreaterThan(&k.Real, ELLIPTIC_PI(realContext))) {
          realSubtract(&k.Real, ELLIPTIC_2_PI(realContext), &k.Real, realContext); // shift k to (-pi, pi]
        }
        realSubtract(&q, &k.Real, &q, realContext);
        realSubtract(&a[i - 1].Real, &b[i - 1].Real, &c.Real, realContext);
        realSubtract(&a[i - 1].Imag, &b[i - 1].Imag, &c.Imag, realContext);
        realMultiply(&c.Real, const_1on2, &c.Real, realContext);
        realMultiply(&c.Imag, const_1on2, &c.Imag, realContext);
        sinComplex(&k, &s, realContext);
        mulComplexComplex(&s, &c, &s, realContext);
        realAdd(&s.Real, &res->Real, &res->Real, realContext);
        realAdd(&s.Imag, &res->Imag, &res->Imag, realContext);
        divComplexComplex(&s, a + i, &s, realContext);
        if(realIsZero(&s.Imag) && !realCompareAbsLessThan(const_1, &s.Real)) {
          C47_WP34S_Asin(&s.Real, &s.Real, realContext);
        }
        else {
          ArcsinComplex(&s, &s, realContext);
        }
        realAdd(&s.Real, &k.Real, &s.Real, realContext);
        realAdd(&s.Imag, &k.Imag, &s.Imag, realContext);
        realAdd(&s.Real, &q, &s.Real, realContext);
        realMultiply(&s.Real, const_1on2, &k.Real, realContext);
        realMultiply(&s.Imag, const_1on2, &k.Imag, realContext);
      }
      freeC47Blocks(b, ELLIPTIC_N * 2 * REAL_SIZE_IN_BLOCKS(75));
    }
    else {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      realSetNaN(&res->Real);
      realSetNaN(&res->Imag);
    }
    freeC47Blocks(a, ELLIPTIC_N * 2 * REAL_SIZE_IN_BLOCKS(75));
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    realSetNaN(&res->Real);
    realSetNaN(&res->Imag);
  }
}

static void _jacobiZeta_carlson(const real_t *phi, const real_t *m, cplx_t *res, realContext_t *realContext) {
  // Z(phi|m) = E(phi|m) - (E(m)/K(m))*F(phi|m), for real phi, 0<=m<=1 only.
  // Uses Carlson RF/RD to avoid mutual recursion with ellipticE.
  // E(phi|m) = sin(phi)*RF(cos²phi, 1-m*sin²phi, 1) - (m/3)*sin³(phi)*RD(cos²phi, 1-m*sin²phi, 1)
  // F(phi|m) = sin(phi)*RF(cos²phi, 1-m*sin²phi, 1)
  real_t s, c, s2, c2, y, rf, rd, e_phi, f_phi, ek, em, delta, delta2, limit, root;
  if(realIsZero(m)) {
    realSetZero(&res->Real);
    realSetZero(&res->Imag);
    return;
  }
  if(realCompareEqual(m, const_1)) {
    cplx_t k, realPhi;
    realCopy(phi, &realPhi.Real);
    realSetZero(&realPhi.Imag);
    ellipticF(&realPhi, m, &k, realContext);
    TanhComplex(&k, res, realContext);
    return;
  }
  if(realCompareLessThan(m, const_1e_37)) {
    C47_WP34S_Cvt2RadSinCosTan(phi, amRadian, &s, &c, NULL, realContext);
    realMultiply(m, &s, &res->Real, realContext);
    realMultiply(&res->Real, &c, &res->Real, realContext);
    realMultiply(&res->Real, const_1on2, &res->Real, realContext);
    realSetZero(&res->Imag);
    return;
  }
  realSubtract(phi, ELLIPTIC_PI_ON_2(realContext), &delta, realContext);
  realMultiply(&delta, &delta, &delta2, realContext);
  realSubtract(const_1, m, &y, realContext);
  realMultiply(&y, const_1e_37, &limit, realContext);
  if(realCompareLessThan(&delta2, &limit)) {
    ellipticKE(m, &ek, &em, realContext);
    realDivide(&em, &ek, &em, realContext);
    realSubtract(&y, &em, &em, realContext);
    realSquareRoot(&y, &root, realContext);
    realDivide(&em, &root, &em, realContext);
    realMultiply(&delta, &em, &res->Real, realContext);
    realSetZero(&res->Imag);
    return;
  }
  C47_WP34S_Cvt2RadSinCosTan(phi, amRadian, &s, &c, NULL, realContext);
  realMultiply(&s, &s, &s2, realContext);            // s2 = sin²phi
  realMultiply(&c, &c, &c2, realContext);            // c2 = cos²phi
  realFMA(m, &s2, const__1, &y, realContext);
  realChangeSign(&y);                                // y = 1 - m*sin²phi
  carlsonRF(&c2, &y, const_1, &rf, realContext);     // RF(cos²,1-m*sin²,1)
  carlsonRD(&c2, &y, const_1, &rd, realContext);     // RD(cos²,1-m*sin²,1)
  realMultiply(&s, &rf, &f_phi, realContext);        // F(phi|m) = sin*RF
  realMultiply(&s2, &s, &s2, realContext);           // sin³
  realMultiply(m, &s2, &s2, realContext);
  realDivide(&s2, const_3, &s2, realContext);         // (m/3)*sin³
  realChangeSign(&s2);                               // -(m/3)*sin³
  realFMA(&s2, &rd, &f_phi, &e_phi, realContext);    // E(phi|m) = f_phi - (m/3)*sin³*RD
  ellipticKE(m, &ek, &em, realContext);              // ek=K(m), em=E(m), real for 0<=m<=1
  realDivide(&em, &ek, &em, realContext);            // E(m)/K(m)
  realChangeSign(&em);                               // -E(m)/K(m)
  realFMA(&em, &f_phi, &e_phi, &res->Real, realContext);  // Z = e_phi - E(m)/K(m)*F(phi|m)
  realSetZero(&res->Imag);
}

static void _jacobiZeta(const cplx_t *amplitude, const real_t *m, cplx_t *res, realContext_t *realContext) {
  const real_t *phi = &amplitude->Real, *psi = &amplitude->Imag;
  if(realCompareLessEqual(m, const_1)) {
    if(realIsZero(psi) && !realIsNegative(m)) {
      // Real phi, 0<=m<=1: use Carlson RF/RD; _jacobiZeta_Agm gives wrong results here.
      _jacobiZeta_carlson(phi, m, res, realContext);
    }
    else {
      // Complex phi or negative m: AGM back-substitution handles these correctly.
      _jacobiZeta_Agm(amplitude, m, res, realContext);
    }
  }
  else {
    int n = 0;
    real_t pp;
    cplx_t phiZ, u;
    realCopy(phi, &phiZ.Real);
    realCopy(psi, &phiZ.Imag);
    ellipticF(&phiZ, m, &u, realContext);

    if(realCompareLessEqual(m, const_2)) {
      realCopy(const_1on4, &pp);
    }
    else {
      realDivide(const_1on2, m, &pp, realContext);
    }

    while(realCompareGreaterThan(&phiZ.Real, &pp)) {
      ++n;
      realMultiply(&u.Real, const_1on2, &u.Real, realContext);
      realMultiply(&u.Imag, const_1on2, &u.Imag, realContext);
      jacobiComplexAm(&u, m, &phiZ, realContext);
    }

    _jacobiZeta_Agm(&phiZ, m, res, realContext);
    while(n-- > 0) {
      cplx_t a, c;
      realAdd(&res->Real, &res->Real, &res->Real, realContext);
      realAdd(&res->Imag, &res->Imag, &res->Imag, realContext);
      jacobiComplexSn(&u, m, &a, realContext);
      mulComplexComplex(&a, &a, &a, realContext);
      realMultiply(&a.Real, m, &a.Real, realContext);
      realMultiply(&a.Imag, m, &a.Imag, realContext);
      realAdd(&u.Real, &u.Real, &u.Real, realContext);
      realAdd(&u.Imag, &u.Imag, &u.Imag, realContext);
      jacobiComplexSn(&u, m, &c, realContext);
      mulComplexComplex(&a, &c, &a, realContext);
      realSubtract(&res->Real, &a.Real, &res->Real, realContext);
      realSubtract(&res->Imag, &a.Imag, &res->Imag, realContext);
    }
  }
}

void jacobiZeta(const cplx_t *amplitude, const real_t *m, cplx_t *res, realContext_t *realContext) {
  const real_t *phi = &amplitude->Real, *psi = &amplitude->Imag;
  cplx_t reduced;
  bool_t   negative = realIsNegative(phi);

  if(realIsSpecial(phi) || realIsSpecial(psi) || realIsSpecial(m)) {  // Z repeats with period pi in phi
    _ellipticSpecialArguments(amplitude, m, false, res);
    return;
  }
  if(realIsZero(m)) {
    realSetZero(&res->Real);
    realSetZero(&res->Imag);
    return;
  }

  realCopy(phi, &reduced.Real);
  realCopy(psi, &reduced.Imag);
  if(negative) {
    realChangeSign(&reduced.Real);
    realChangeSign(&reduced.Imag);
  }
  realSetNaN(&res->Real);
  realSetNaN(&res->Imag);

  WP34S_Mod(&reduced.Real, ELLIPTIC_PI(realContext), &reduced.Real, realContext);
  if(realCompareGreaterThan(&reduced.Real, ELLIPTIC_PI_ON_2(realContext))) {
    realSubtract(ELLIPTIC_PI(realContext), &reduced.Real, &reduced.Real, realContext);
    realChangeSign(&reduced.Imag);
    negative = !negative;
  }
  if(realIsZero(&reduced.Imag)) {
    _jacobiZeta(&reduced, m, res, realContext);
  }
  else {
    cplx_t a, c;
    ellipticComplexKE(m, &a, &c, realContext);
    divComplexComplex(&c, &a, &a, realContext);
    ellipticF(&reduced, m, &c, realContext);
    mulComplexComplex(&a, &c, &a, realContext);
    ellipticE(&reduced, m, &c, realContext);
    realSubtract(&c.Real, &a.Real, &res->Real, realContext);
    realSubtract(&c.Imag, &a.Imag, &res->Imag, realContext);
  }

  if(negative) {
    realChangeSign(&res->Real);
    realChangeSign(&res->Imag);
  }
}

/**
 * Calculates the Cauchy principal value of complete Pi(n,m) for n greater than one.
 */
static void ellipticPiPrincipalValue(const real_t *n, const real_t *m, cplx_t *res, realContext_t *realContext) {
  real_t nn, m1, p, rj;

  realDivide(m, n, &nn, realContext);
  realSubtract(const_1, m, &m1, realContext);
  realSubtract(const_1, &nn, &p, realContext);
  carlsonRJ(const_0, &m1, const_1, &p, &rj, realContext);
  realMultiply(&nn, &rj, &res->Real, realContext);
  realDivide(&res->Real, const_3, &res->Real, realContext);
  realChangeSign(&res->Real);
  realSetZero(&res->Imag);
}

/**
 * Calculates complete Pi(n,m) directly from Carlson RF and RJ for n less than one.
 */
static void ellipticPiCarlson(const real_t *n, const real_t *m, cplx_t *res, realContext_t *realContext) {
  real_t m1, p, rf, rj, a;

  realSubtract(const_1, m, &m1, realContext);
  realSubtract(const_1, n, &p, realContext);
  carlsonRF(const_0, &m1, const_1, &rf, realContext);
  carlsonRJ(const_0, &m1, const_1, &p, &rj, realContext);
  realMultiply(n, &rj, &a, realContext);
  realDivide(&a, const_3, &a, realContext);
  realAdd(&rf, &a, &res->Real, realContext);
  realSetZero(&res->Imag);
}

/**
 * Calculates complete Pi(n,m) after mapping n less than -1 into the bounded interval (-1,0).
 */
static void ellipticPiNegative(const real_t *n, const real_t *m, cplx_t *res, realContext_t *realContext) {
  real_t q, m1, p, rj, h, a, b;

  realDivide(m, n, &q, realContext);
  realSubtract(const_1, m, &m1, realContext);
  realSubtract(const_1, &q, &p, realContext);
  carlsonRJ(const_0, &m1, const_1, &p, &rj, realContext);
  realMultiply(&q, &rj, &res->Real, realContext);
  realDivide(&res->Real, const_3, &res->Real, realContext);
  realChangeSign(&res->Real);

  realSubtract(const_1, n, &a, realContext);
  realSubtract(n, m, &b, realContext);
  realMultiply(n, &b, &h, realContext);
  realMultiply(&h, &a, &h, realContext);
  realSquareRoot(&h, &h, realContext);
  realMultiply(n, ELLIPTIC_PI_ON_2(realContext), &a, realContext);
  realDivide(&a, &h, &a, realContext);
  realSubtract(&res->Real, &a, &res->Real, realContext);
  realSetZero(&res->Imag);
}

void ellipticPi(const real_t *n, const real_t *m, cplx_t *res, realContext_t *realContext) {
  if(realIsNaN(n) || realIsNaN(m)) {
    realSetNaN(&res->Real);
    realSetZero(&res->Imag);
  }
  else if(realIsZero(n)) {
    ellipticComplexKE(m, res, NULL, realContext);
  }
  else if(realIsZero(m)) {
    // Abramowitz & Stegun §17.7.20
    if(realCompareLessThan(n, const_1)) {
      //      lim       (Arctan(sqrt(1-n) tan(phi)) / sqrt(1-n))
      //  phi->(pi/2)-0
      // fixed correct formula  correct formula (A&S §17.7.20) is: π / (2√(1−n))
      cplx_t piOn2;

      realSubtract(const_1, n, &res->Real, realContext);
      rcSqrt(&res->Real, res, realContext);
      realCopy(ELLIPTIC_PI_ON_2(realContext), &piOn2.Real);
      realSetZero(&piOn2.Imag);
      divComplexComplex(&piOn2, res, res, realContext);
    }
    else if(realCompareGreaterThan(n, const_1)) {
      //      lim       (Arctanh(sqrt(n-1) tan(phi)) / sqrt(n-1))
      //  phi->(pi/2)-0                                          = -(pi/2) i / sqrt(n-1)
      cplx_t i;

      realSubtract(n, const_1, &res->Real, realContext);
      rcSqrt(&res->Real, res, realContext);
      realSetZero(&i.Real);
      realSetOne(&i.Imag);
      divComplexComplex(&i, res, res, realContext);
      realMultiply(ELLIPTIC_PI_ON_2(realContext), &res->Real, &res->Real, realContext);
      realMultiply(ELLIPTIC_PI_ON_2(realContext), &res->Imag, &res->Imag, realContext);
      realChangeSign(&res->Real);
      realChangeSign(&res->Imag);
    }
    else { // n = 1
      realSetNaN(&res->Real);
      realSetZero(&res->Imag); // unsigned infinity
    }
  }
  else if(realCompareEqual(m, const_1)) {
    if(realCompareLessThan(n, const_1)) {
      realSetPlusInfinity(&res->Real);
      realSetZero(&res->Imag);
    }
    else if(realCompareGreaterThan(n, const_1)) {
      realSetMinusInfinity(&res->Real);
      realSetZero(&res->Imag);
    }
    else { // n = 1
      realSetNaN(&res->Real);
      realSetZero(&res->Imag); // unsigned infinity
    }
  }
  else if(realCompareGreaterThan(n, const_1)) {
    ellipticPiPrincipalValue(n, m, res, realContext);
  }
  else if(realIsInfinite(n)) {
    realSetZero(&res->Real);
    realSetZero(&res->Imag);
  }
  else if(realCompareLessThan(n, const__1)) {
    ellipticPiNegative(n, m, res, realContext);
  }
  else {
    ellipticPiCarlson(n, m, res, realContext);
  }
}

void fnJacobiSn(uint16_t unusedButMandatoryParameter) {
  bool_t realInput;
  real_t m;
  cplx_t u;
  cplx_t r;

  if(!jacobi_check_inputs(&m, &u, &realInput)) {
    return;
  }

  if(realInput) {
    jacobiElliptic(&u.Real, &m, NULL, &r.Real, NULL, NULL, &ctxtReal75);
    convertRealToResultRegister(&r.Real, REGISTER_X, amNone);
  }
  else {
    jacobiComplexSn(&u, &m, &r, &ctxtReal75);
    convertComplexToResultRegister(&r, REGISTER_X);
  }

  adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
}

void fnJacobiCn(uint16_t unusedButMandatoryParameter) {
  bool_t realInput;
  real_t m;
  cplx_t u;
  cplx_t r;

  if(!jacobi_check_inputs(&m, &u, &realInput)) {
    return;
  }

  if(realInput) {
    jacobiElliptic(&u.Real, &m, NULL, NULL, &r.Real, NULL, &ctxtReal75);
    convertRealToResultRegister(&r.Real, REGISTER_X, amNone);
  }
  else {
    jacobiComplexCn(&u, &m, &r, &ctxtReal75);
    convertComplexToResultRegister(&r, REGISTER_X);
  }

  adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
}

void fnJacobiDn(uint16_t unusedButMandatoryParameter) {
  bool_t realInput;
  real_t m;
  cplx_t u;
  cplx_t r;

  if(!jacobi_check_inputs(&m, &u, &realInput)) {
    return;
  }

  if(realInput) {
    jacobiElliptic(&u.Real, &m, NULL, NULL, NULL, &r.Real, &ctxtReal75);
    convertRealToResultRegister(&r.Real, REGISTER_X, amNone);
  }
  else {
    jacobiComplexDn(&u, &m, &r, &ctxtReal75);
    convertComplexToResultRegister(&r, REGISTER_X);
  }

  adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
}

void fnJacobiAmplitude(uint16_t unusedButMandatoryParameter) {
  bool_t realInput;
  real_t m;
  cplx_t u;
  cplx_t r;

  if(!jacobi_check_inputs(&m, &u, &realInput)) {
    return;
  }

  if(realInput) {
    jacobiElliptic(&u.Real, &m, &r.Real, NULL, NULL, NULL, &ctxtReal75);
    convertAngleFromTo(&r.Real, amRadian, currentAngularMode, &ctxtReal39);
    convertRealToResultRegister(&r.Real, REGISTER_X, currentAngularMode);
  }
  else {
    jacobiComplexAm(&u, &m, &r, &ctxtReal75);
    convertComplexToResultRegister(&r, REGISTER_X);
  }

  adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
}

void fnEllipticK(uint16_t unusedButMandatoryParameter) {
  real_t m;
  cplx_t k;

  if(!saveLastX()) {
    return;
  }

  if(!getRegisterAsReal(REGISTER_X, &m)) {
    return;
  }

  if(realCompareLessEqual(&m, const_1)) {
    ellipticKE(&m, &k.Real, NULL, &ctxtReal39);
    convertRealToResultRegister(&k.Real, REGISTER_X, amNone);
  }
  else if(getFlag(FLAG_CPXRES)) {
    ellipticComplexKE(&m, &k, NULL, &ctxtReal39);
    convertComplexToResultRegister(&k, REGISTER_X);
  }
  else {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Cannot calculate K(m) for m > 1 if CPXRES is not set");
      moreInfoOnError("In function fnEllipticK:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}

void fnEllipticE(uint16_t unusedButMandatoryParameter) {
  real_t m;
  cplx_t e;

  if(!saveLastX()) {
    return;
  }

  if(!getRegisterAsReal(REGISTER_X, &m)) {
    return;
  }

  if(realCompareLessEqual(&m, const_1)) {
    ellipticKE(&m, NULL, &e.Real, &ctxtReal39);
    convertRealToResultRegister(&e.Real, REGISTER_X, amNone);
  }
  else if(getFlag(FLAG_CPXRES)) {
    ellipticComplexKE(&m, NULL, &e, &ctxtReal75);
    convertComplexToResultRegister(&e, REGISTER_X);
  }
  else {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Cannot calculate K(m) for m > 1 if CPXRES is not set");
      moreInfoOnError("In function fnEllipticE:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}

void fnEllipticPi(uint16_t unusedButMandatoryParameter) {
  real_t m;
  cplx_t u;
  cplx_t r;
  bool_t realInput;

  if(!jacobi_check_inputs(&m, &u, &realInput)) {
    return;
  }

  if(realIsNegative(&m) || realCompareGreaterEqual(&m, const_1)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "m is out of range (must in 0 ≤ m < 1)");
      moreInfoOnError("In function fnEllipticPi:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else if(realInput) {
    ellipticPi(&u.Real, &m, &r, &ctxtReal39);
    if(realIsZero(&r.Imag)) {
      convertRealToResultRegister(&r.Real, REGISTER_X, amNone);
    }
    else if(getFlag(FLAG_CPXRES)) {
      convertComplexToResultRegister(&r, REGISTER_X);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Π(n|m) cannot return complex result without CPXRES set");
        moreInfoOnError("In function fnEllipticPi:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot calculate elliptic integral with %s in X", getRegisterDataTypeName(REGISTER_X, true, false));
      moreInfoOnError("In function fnEllipticPi:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
}

void fnEllipticFphi(uint16_t unusedButMandatoryParameter) {
  bool_t realInput;
  real_t m;
  cplx_t u;
  cplx_t r;

  if(!jacobi_check_inputs_phi(&m, &u, &realInput)) {
    return;
  }

  realContext_t *realContext = realInput && realCompareLessEqual(&m, const_1) ? &ctxtReal39 : &ctxtReal75;
  ellipticF(&u, &m, &r, realContext);
  if(realInput) {
    if(realIsZero(&r.Imag)) {
      convertRealToResultRegister(&r.Real, REGISTER_X, amNone);
    }
    else if(getFlag(FLAG_CPXRES)) {
      convertComplexToResultRegister(&r, REGISTER_X);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "F(φ|m) cannot return complex result without CPXRES set");
        moreInfoOnError("In function fnEllipticFphi:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
  else {
    convertComplexToResultRegister(&r, REGISTER_X);
  }

  adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
}

void fnEllipticEphi(uint16_t unusedButMandatoryParameter) {
  bool_t realInput;
  real_t m;
  cplx_t u;
  cplx_t r;

  if(!jacobi_check_inputs_phi(&m, &u, &realInput)) {
    return;
  }

  realContext_t *realContext = realInput && realCompareLessEqual(&m, const_1) ? &ctxtReal39 : &ctxtReal75;
  ellipticE(&u, &m, &r, realContext);
  if(realInput) {
    if(realIsZero(&r.Imag)) {
      convertRealToResultRegister(&r.Real, REGISTER_X, amNone);
    }
    else if(getFlag(FLAG_CPXRES)) {
      convertComplexToResultRegister(&r, REGISTER_X);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "E(φ|m) cannot return complex result without CPXRES set");
        moreInfoOnError("In function fnEllipticEphi:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
  else {
    convertComplexToResultRegister(&r, REGISTER_X);
  }

  adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
}

void fnJacobiZeta(uint16_t unusedButMandatoryParameter) {
  bool_t realInput;
  real_t m;
  cplx_t u;
  cplx_t r;

  if(!jacobi_check_inputs_phi(&m, &u, &realInput)) {
    return;
  }

  jacobiZeta(&u, &m, &r, &ctxtReal75);
  if(realInput) {
    if(realIsZero(&r.Imag)) {
      convertRealToResultRegister(&r.Real, REGISTER_X, amNone);
    }
    else if(getFlag(FLAG_CPXRES)) {
      convertComplexToResultRegister(&r, REGISTER_X);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Ζ(φ|m) cannot return complex result without CPXRES set");
        moreInfoOnError("In function fnJacobiZeta:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
  else {
    convertComplexToResultRegister(&r, REGISTER_X);
  }

  adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
}

#else //OPTION_ELLIPTIC
  void fnEllipse(uint16_t unusedButMandatoryParameter) {;}
  void fnKtoM(uint16_t unusedButMandatoryParameter) {;}
  void fnMtoK(uint16_t unusedButMandatoryParameter) {;}
  void fnMtoTheta(uint16_t unusedButMandatoryParameter) {;}
  void fnThetatoM(uint16_t unusedButMandatoryParameter) {;}
  void fnJacobiSn(uint16_t unusedButMandatoryParameter) {;}
  void fnJacobiCn(uint16_t unusedButMandatoryParameter) {;}
  void fnJacobiDn(uint16_t unusedButMandatoryParameter) {;}
  void fnJacobiAmplitude(uint16_t unusedButMandatoryParameter) {;}
  void fnEllipticK(uint16_t unusedButMandatoryParameter) {;}
  void fnEllipticE(uint16_t unusedButMandatoryParameter) {;}
  void fnEllipticPi(uint16_t unusedButMandatoryParameter) {;}
  void fnEllipticFphi(uint16_t unusedButMandatoryParameter) {;}
  void fnEllipticEphi(uint16_t unusedButMandatoryParameter) {;}
  void fnJacobiZeta(uint16_t unusedButMandatoryParameter) {;}
#endif //OPTION_ELLIPTIC
