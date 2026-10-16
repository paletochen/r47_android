// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file lnbeta.c
 ***********************************************/

#include "c47.h"


/*
 * This function checks argument for LnGamma function.
 * Returns true if function can be calculated, false otherwise. In case of error the function sets the appropriate value
 * into REGISTER_X and display the error message.
 */
#define RESULT_TYPE_UNKNOWN 0
#define RESULT_TYPE_REAL    1
#define RESULT_TYPE_COMPLEX 2


static void lanczosSumRatioReal(const real_t *a, const real_t *b, const real_t *sumAB, real_t *res, realContext_t *realContext) {
  real_t coefficientA, denominatorB, denominatorSum, product, sumLanczos, term, twentyEight;
  int k;

  WP34S_LanczosSum(sumAB, const_28, &sumLanczos, realContext);
  realSetZero(res);
  int32ToReal(28, &twentyEight);
  realAdd(b, &twentyEight, &denominatorB, realContext);
  for(k = 28; k >= 0; k--) {
    realAdd(&denominatorB, a, &denominatorSum, realContext);
    realMultiply(&denominatorB, &denominatorSum, &product, realContext);
    realMultiply((real_t *)(((uint8_t *)const75_gammaC01) + k * REAL_SIZE_IN_BYTES(75)), a, &coefficientA, realContext);
    realDivide(&coefficientA, &product, &term, realContext);
    realAdd(res, &term, res, realContext);
    realSubtract(&denominatorB, const_1, &denominatorB, realContext);
  }
  realDivide(res, &sumLanczos, res, realContext);
  WP34S_Ln1P(res, res, realContext);
}


/*
 * Positive real log-Beta evaluated as one Lanczos expression. Labelling the
 * smaller argument a and the larger one b changes the three central logarithms
 * to terms containing log1p(a/(b+h)). The Lanczos-sum quotient is formed from
 * its termwise difference, so neither part depends on retaining a in a+b.
 */
void C47_LnBetaPositiveReal(const real_t *x, const real_t *y, real_t *res) {
  real_t a, b, logA, logRa, logRb, logRatio, ra, rb, sumAB, t, u;
  realContext_t *realContext = &ctxtReal75;

  if(realIsAnInteger(x) && realCompareLessEqual(x, const_205)) {
    bool_t error;
    int n = realToInt32C47(x, &error);

    WP34S_Gamma(x, &t, realContext);
    WP34S_Ln(&t, res, realContext);
    WP34S_Ln(y, &t, realContext);
    realMultiply(x, &t, &t, realContext);
    realSubtract(res, &t, res, realContext);
    for(int k = 1; k < n; k++) {
      int32ToReal(k, &t);
      realDivide(&t, y, &t, realContext);
      WP34S_Ln1P(&t, &t, realContext);
      realSubtract(res, &t, res, realContext);
    }
    return;
  }
  if(realIsAnInteger(y) && realCompareLessEqual(y, const_205)) {
    C47_LnBetaPositiveReal(y, x, res);
    return;
  }

  if(realCompareLessEqual(x, y)) {
    realCopy(x, &a);
    realCopy(y, &b);
  }
  else {
    realCopy(y, &a);
    realCopy(x, &b);
  }

  if(realCompareEqual(&a, const_1)) {
    WP34S_Ln(&b, res, realContext);
    realChangeSign(res);
    return;
  }

  realAdd(&a, &b, &sumAB, realContext);

  WP34S_LanczosSum(&a, const_28, &logA, realContext);
  WP34S_Ln(&logA, &logA, realContext);
  lanczosSumRatioReal(&a, &b, &sumAB, &logRatio, realContext);

  realAdd(&a, const_gammaRm1, &ra, realContext);
  realAdd(&b, const_gammaRm1, &rb, realContext);
  WP34S_Ln(&ra, &logRa, realContext);
  WP34S_Ln(&rb, &logRb, realContext);

  realSubtract(&a, const_1on2, &t, realContext);
  realMultiply(&t, &logRa, &t, realContext);
  realAdd(&logA, &logRatio, res, realContext);
  realAdd(res, &t, res, realContext);

  realMultiply(&a, &logRb, &t, realContext);
  realSubtract(res, &t, res, realContext);

  realDivide(&a, &rb, &u, realContext);
  WP34S_Ln1P(&u, &u, realContext);
  realSubtract(&sumAB, const_1on2, &t, realContext);
  realMultiply(&t, &u, &t, realContext);
  realSubtract(res, &t, res, realContext);
  realSubtract(res, const_gammaRm1, res, realContext);
}


static void lanczosSumComplex(const cplx_t *z, cplx_t *sum, realContext_t *realContext) {
  real_t twentyEight;
  cplx_t denominator, term;
  int k;

  realSetZero(&sum->Real);
  realSetZero(&sum->Imag);
  int32ToReal(28, &twentyEight);
  realAdd(&z->Real, &twentyEight, &denominator.Real, realContext);
  realCopy(&z->Imag, &denominator.Imag);
  for(k = 28; k >= 0; k--) {
    divRealComplex((real_t *)(((uint8_t *)const75_gammaC01) + k * REAL_SIZE_IN_BYTES(75)), &denominator, &term, realContext);
    realAdd(&sum->Real, &term.Real, &sum->Real, realContext);
    realAdd(&sum->Imag, &term.Imag, &sum->Imag, realContext);
    realSubtract(&denominator.Real, const_1, &denominator.Real, realContext);
  }
  realAdd(&sum->Real, const75_gammaC00, &sum->Real, realContext);
}


static void lanczosSumRatioComplex(const cplx_t *a, const cplx_t *b,
                                   const cplx_t *sum, cplx_t *res,
                                   realContext_t *realContext) {
  cplx_t coefficientA, denominatorB, denominatorSum;
  real_t twentyEight;
  cplx_t product, sumLanczos, term;
  int k;

  lanczosSumComplex(sum, &sumLanczos, realContext);
  realSetZero(&res->Real);
  realSetZero(&res->Imag);
  int32ToReal(28, &twentyEight);
  realAdd(&b->Real, &twentyEight, &denominatorB.Real, realContext);
  realCopy(&b->Imag, &denominatorB.Imag);
  for(k = 28; k >= 0; k--) {
    realAdd(&denominatorB.Real, &a->Real, &denominatorSum.Real, realContext);
    realAdd(&denominatorB.Imag, &a->Imag, &denominatorSum.Imag, realContext);
    mulComplexComplex(&denominatorB, &denominatorSum, &product, realContext);
    realMultiply((real_t *)(((uint8_t *)const75_gammaC01) + k * REAL_SIZE_IN_BYTES(75)), &a->Real, &coefficientA.Real, realContext);
    realMultiply((real_t *)(((uint8_t *)const75_gammaC01) + k * REAL_SIZE_IN_BYTES(75)), &a->Imag, &coefficientA.Imag, realContext);
    divComplexComplex(&coefficientA, &product, &term, realContext);
    realAdd(&res->Real, &term.Real, &res->Real, realContext);
    realAdd(&res->Imag, &term.Imag, &res->Imag, realContext);
    realSubtract(&denominatorB.Real, const_1, &denominatorB.Real, realContext);
  }
  divComplexComplex(res, &sumLanczos, res, realContext);
  lnP1Complex(res, res, realContext);
}


void C47_LnBetaPositiveComplex(const cplx_t *x, const cplx_t *y, cplx_t *res) {
  real_t magnitudeX, magnitudeY;
  cplx_t a, b, logA, logRatio;
  cplx_t ra, rb;
  cplx_t logRa, logRb, sum, t, u;
  realContext_t *realContext = &ctxtReal75;

  if(realIsZero(&x->Imag) && realIsAnInteger(&x->Real) && realCompareLessEqual(&x->Real, const_205)) {
    bool_t error;
    int n = realToInt32C47(&x->Real, &error);

    WP34S_Gamma(&x->Real, &t.Real, realContext);
    WP34S_Ln(&t.Real, &res->Real, realContext);
    realSetZero(&res->Imag);
    lnComplex(y, &t, realContext);
    realMultiply(&x->Real, &t.Real, &t.Real, realContext);
    realMultiply(&x->Real, &t.Imag, &t.Imag, realContext);
    realSubtract(&res->Real, &t.Real, &res->Real, realContext);
    realSubtract(&res->Imag, &t.Imag, &res->Imag, realContext);
    for(int k = 1; k < n; k++) {
      int32ToReal(k, &t.Real);
      divRealComplex(&t.Real, y, &u, realContext);
      lnP1Complex(&u, &u, realContext);
      realSubtract(&res->Real, &u.Real, &res->Real, realContext);
      realSubtract(&res->Imag, &u.Imag, &res->Imag, realContext);
    }
    return;
  }
  if(realIsZero(&y->Imag) && realIsAnInteger(&y->Real) && realCompareLessEqual(&y->Real, const_205)) {
    C47_LnBetaPositiveComplex(y, x, res);
    return;
  }

  complexMagnitude(x, &magnitudeX, realContext);
  complexMagnitude(y, &magnitudeY, realContext);
  if(realCompareLessEqual(&magnitudeX, &magnitudeY)) {
    realCopy(&x->Real, &a.Real);
    realCopy(&x->Imag, &a.Imag);
    realCopy(&y->Real, &b.Real);
    realCopy(&y->Imag, &b.Imag);
  }
  else {
    realCopy(&y->Real, &a.Real);
    realCopy(&y->Imag, &a.Imag);
    realCopy(&x->Real, &b.Real);
    realCopy(&x->Imag, &b.Imag);
  }

  if(realCompareEqual(&a.Real, const_1) && realIsZero(&a.Imag)) {
    lnComplex(&b, res, realContext);
    realChangeSign(&res->Real);
    realChangeSign(&res->Imag);
    return;
  }

  realAdd(&a.Real, &b.Real, &sum.Real, realContext);
  realAdd(&a.Imag, &b.Imag, &sum.Imag, realContext);

  lanczosSumComplex(&a, &logA, realContext);
  lnComplex(&logA, &logA, realContext);
  lanczosSumRatioComplex(&a, &b, &sum, &logRatio, realContext);

  realAdd(&a.Real, const_gammaRm1, &ra.Real, realContext);
  realCopy(&a.Imag, &ra.Imag);
  realAdd(&b.Real, const_gammaRm1, &rb.Real, realContext);
  realCopy(&b.Imag, &rb.Imag);
  lnComplex(&ra, &logRa, realContext);
  lnComplex(&rb, &logRb, realContext);

  realSubtract(&a.Real, const_1on2, &t.Real, realContext);
  realCopy(&a.Imag, &t.Imag);
  mulComplexComplex(&t, &logRa, res, realContext);
  realAdd(&res->Real, &logA.Real, &res->Real, realContext);
  realAdd(&res->Imag, &logA.Imag, &res->Imag, realContext);
  realAdd(&res->Real, &logRatio.Real, &res->Real, realContext);
  realAdd(&res->Imag, &logRatio.Imag, &res->Imag, realContext);

  mulComplexComplex(&a, &logRb, &t, realContext);
  realSubtract(&res->Real, &t.Real, &res->Real, realContext);
  realSubtract(&res->Imag, &t.Imag, &res->Imag, realContext);

  divComplexComplex(&a, &rb, &u, realContext);
  lnP1Complex(&u, &u, realContext);
  realSubtract(&sum.Real, const_1on2, &t.Real, realContext);
  realCopy(&sum.Imag, &t.Imag);
  mulComplexComplex(&t, &u, &t, realContext);
  realSubtract(&res->Real, &t.Real, &res->Real, realContext);
  realSubtract(&res->Imag, &t.Imag, &res->Imag, realContext);
  realSubtract(&res->Real, const_gammaRm1, &res->Real, realContext);
}

static bool_t _checkLnGammaArgs(int8_t *resultType, real_t *xReal, realContext_t *realContext) {
  bool_t result = true;
  *resultType = RESULT_TYPE_UNKNOWN;

  if(realIsInfinite(xReal)) {
    if(!getSystemFlag(FLAG_SPCRES)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      EXTRA_INFO_MESSAGE("_checkLnGammaArgs", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of lnbeta when flag SPCRES is not set");
    }
    else {
      realToReal34((realIsPositive(xReal) ? const_plusInfinity : const_NaN), REGISTER_REAL34_DATA(REGISTER_X));
    }

    result = false;
  }
  else if(realCompareLessEqual(xReal, const_0)) { // x <= 0
    if(realIsAnInteger(xReal)) {
      if(!getSystemFlag(FLAG_SPCRES)) {
        displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
        EXTRA_INFO_MESSAGE("_checkLnGammaArgs", "cannot use a negative integer as X input of lnbeta when flag SPCRES is not set");
      }
      else {
        convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
      }

      result = false;
    }
    else { // x is negative and not an integer
      real_t tmp;

      realMinus(xReal, &tmp, realContext); // tmp = -x
      WP34S_Mod(&tmp, const_2, &tmp, realContext); // tmp = ?

      if(realCompareGreaterThan(&tmp, const_1)) { // the result is a real
          *resultType = RESULT_TYPE_REAL;
      }
      else { // the result is a complex
        if(getFlag(FLAG_CPXRES)) {
          *resultType = RESULT_TYPE_COMPLEX;
        }
        else { // Domain error
          displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
          EXTRA_INFO_MESSAGE("_checkLnGammaArgs", "cannot use a as X input of lnbeta if gamma(X)<0 when flag I is not set");
          result = false;
        }
      }
    }
  }
  else {
      *resultType = RESULT_TYPE_REAL;
  }

  return result;
}



static void _lnGammaReal(real_t *xReal, real_t *rReal, realContext_t *realContext) {
  WP34S_LnGamma(xReal, rReal, realContext);
}



static void _lnGammaComplex(real_t *xReal, cplx_t *r, realContext_t *realContext) {
  realCopy(xReal, &r->Imag);
  WP34S_Gamma(xReal, xReal, realContext);
  realSetPositiveSign(xReal);
  WP34S_Ln(xReal, xReal, realContext);
  realCopy(xReal, &r->Real);
  realToIntegralValue(&r->Imag, &r->Imag, DEC_ROUND_FLOOR, realContext);
  realMultiply(&r->Imag, const39_pi, &r->Imag, realContext);
}



#if defined(OPTION_ZETA_BETA)
static void lnGammaComplexWide(const cplx_t *x, cplx_t *r, realContext_t *realContext) {
  if(realContext->digits <= 39 || realIsZero(&x->Imag)) {
    complexLnGamma(x, r, realContext);
  }
  else {
    real_t turns;
    cplx_t reference;

    complexLnGamma(x, &reference, &ctxtReal39);
    WP34S_ComplexLnGamma(x, r, realContext);
    realSubtract(&reference.Imag, &r->Imag, &turns, realContext);
    realDivide(&turns, const75_2pi, &turns, realContext);
    realToIntegralValue(&turns, &turns, DEC_ROUND_HALF_EVEN, realContext);
    realFMA(const75_2pi, &turns, &r->Imag, &r->Imag, realContext);
  }
}


static void lnBetaComplexDirect(cplx_t *x, cplx_t *y, cplx_t *r, realContext_t *realContext) {
  cplx_t t;

  lnGammaComplexWide(x, &t, realContext);                        // t = LnGamma(x)
  lnGammaComplexWide(y, r, realContext);                         // r = LnGamma(y)

  realAdd(&r->Real, &t.Real, &r->Real, realContext); // r = LnGamma(x) + LnGamma(y)
  realAdd(&r->Imag, &t.Imag, &r->Imag, realContext);

  realAdd(&x->Real, &y->Real, &t.Real, realContext); // t = x + y
  realAdd(&x->Imag, &y->Imag, &t.Imag, realContext);

  lnGammaComplexWide(&t, &t, realContext);                         // t = LnGamma(x + y);

  realSubtract(&r->Real, &t.Real, &r->Real, realContext); // r = LnGamma(x) + LnGamma(y) - LnGamma(x + y);
  realSubtract(&r->Imag, &t.Imag, &r->Imag, realContext);
}


static void _lnBetaComplex(cplx_t *x, cplx_t *y, cplx_t *r, realContext_t *realContext) {
  if(getSystemFlag(FLAG_SPCRES) && realIsZero(&x->Imag) && realIsZero(&y->Imag) &&
     ((realIsInfinite(&x->Real) && realIsPositive(&x->Real) && realCompareGreaterThan(&y->Real, const_0)) ||
      (realIsInfinite(&y->Real) && realIsPositive(&y->Real) && realCompareGreaterThan(&x->Real, const_0)))) {
    realSetMinusInfinity(&r->Real);
    realSetZero(&r->Imag);
  }
  else if(realCompareGreaterThan(&x->Real, const_0) && realCompareGreaterThan(&y->Real, const_0) &&
     !realIsSpecial(&x->Real) && !realIsSpecial(&x->Imag) && !realIsSpecial(&y->Real) && !realIsSpecial(&y->Imag)) {
    real_t turns;
    cplx_t reference;

    C47_LnBetaPositiveComplex(x, y, r);
    lnBetaComplexDirect(x, y, &reference, &ctxtReal39);
    realSubtract(&reference.Imag, &r->Imag, &turns, realContext);
    realDivide(&turns, const75_2pi, &turns, realContext);
    realToIntegralValue(&turns, &turns, DEC_ROUND_HALF_EVEN, realContext);
    realFMA(const75_2pi, &turns, &r->Imag, &r->Imag, realContext);
  }
  else {
    lnBetaComplexDirect(x, y, r, realContext);
  }
}
#endif // OPTION_ZETA_BETA



static bool_t _lnBetaReal(real_t *xReal, real_t *yReal, cplx_t *r, realContext_t *realContext) {
  int8_t xflag, yflag, sflag;

  if((realIsInfinite(xReal) && realIsPositive(xReal) && realCompareGreaterThan(yReal, const_0)) ||
     (realIsInfinite(yReal) && realIsPositive(yReal) && realCompareGreaterThan(xReal, const_0))) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realSetMinusInfinity(&r->Real);
      realSetZero(&r->Imag);
      return true;
    }
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    EXTRA_INFO_MESSAGE("_lnBetaReal", "cannot use positive infinity in lnbeta when flag SPCRES is not set");
    return false;
  }

  if(realCompareGreaterThan(xReal, const_0) && realCompareGreaterThan(yReal, const_0) && !realIsSpecial(xReal) && !realIsSpecial(yReal)) {
    C47_LnBetaPositiveReal(xReal, yReal, &r->Real);
    realSetZero(&r->Imag);
    return true;
  }

  realAdd(xReal, yReal, &r->Real, realContext);  // r = x+y

  if(_checkLnGammaArgs(&xflag, xReal, realContext) && _checkLnGammaArgs(&yflag, yReal, realContext) && _checkLnGammaArgs(&sflag, &r->Real, realContext)) {
    cplx_t gx;              // LnGamma(x)
    cplx_t gy;              // LnGamma(y)
    cplx_t gs;              // LnGamma(x+y)

    if(xflag==RESULT_TYPE_REAL) {
      _lnGammaReal(xReal, &gx.Real, realContext);
      realSetZero(&gx.Imag);
    }
    else {
      _lnGammaComplex(xReal, &gx, realContext);
    }

    if(yflag==RESULT_TYPE_REAL) {
      _lnGammaReal(yReal, &gy.Real, realContext);
      realSetZero(&gy.Imag);
    }
    else {
      _lnGammaComplex(yReal, &gy, realContext);
    }

    if(sflag==RESULT_TYPE_REAL) {
      _lnGammaReal(&r->Real, &gs.Real, realContext);
      realSetZero(&gs.Imag);
    }
    else {
      _lnGammaComplex(&r->Real, &gs, realContext);
    }

    realAdd(&gx.Real, &gy.Real, &r->Real, realContext); // r = LnGamma(x) + LnGamma(y)
    realAdd(&gx.Imag, &gy.Imag, &r->Imag, realContext);

    realSubtract(&r->Real, &gs.Real, &r->Real, realContext); // r = LnGamma(x) + LnGamma(y) - LnGamma(x+y)
    realSubtract(&r->Imag, &gs.Imag, &r->Imag, realContext);

    return true;
  }

  return false;
}


// Deliberately outside OPTION_ZETA_BETA, which gates only the LN-beta key: the F distribution calls this from f.c, and WP34S_betai in wp34s.c calls it too.
// Gating it would leave both calling an empty function, the way erfc did before it went with OPTION_DIST_NORMAL. _lnBetaReal and the LnGamma helpers it
// reaches stay outside for the same reason. Only _lnBetaComplex, which nothing but the key uses, sits inside the option.
void LnBeta(const real_t *x, const real_t *y, real_t *res, realContext_t *realContext) {
  cplx_t r;
  real_t xx, yy;

  realCopy(x, &xx);
  realCopy(y, &yy);
  if(_lnBetaReal(&xx, &yy, &r, realContext) && realIsZero(&r.Imag)) {
    realCopy(&r.Real, res);
  }
  else {
    realSetNaN(res);
  }
}


#if defined(OPTION_ZETA_BETA)
static void lnbetaReal(void) {
  real_t x, y;
  cplx_t r;

  if(getRegisterAsReal(REGISTER_X, &x) && getRegisterAsReal(REGISTER_Y, &y)) {
    if(_lnBetaReal(&x, &y, &r, &ctxtReal75)) {
      if(realIsZero(&r.Imag)) {
        convertRealToResultRegister(&r.Real, REGISTER_X, amNone);
      }
      else {
        convertComplexToResultRegister(&r, REGISTER_X);
      }
    }
  }
}

static void lnbetaCplx(void)  {
  cplx_t x, y, r;

  if(getRegisterAsComplex(REGISTER_X, &x) && getRegisterAsComplex(REGISTER_Y, &y)) {
    _lnBetaComplex(&x, &y, &r, &ctxtReal75);
    convertComplexToResultRegister(&r, REGISTER_X);
  }
}
#endif // OPTION_ZETA_BETA

/********************************************//**
 * \brief regX ==> regL and lnBeta(regX, RegY) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnLnBeta(uint16_t unusedButMandatoryParameter) {
#if defined(OPTION_ZETA_BETA)
    processRealComplexDyadicFunction(&lnbetaReal, &lnbetaCplx);
#endif // OPTION_ZETA_BETA
}
