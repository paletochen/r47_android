// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file xthRoot.c
 ***********************************************/
// Coded by JM, based on power.c, with reference to cuberoot.c

#include "c47.h"

/********************************************//**
 * \brief Replaces an approximation with an exact Real34 root when one exists
 *
 * A nearest 34-digit candidate is reduced before verification. Each product is
 * formed only while its exact coefficient has at most 34 digits, so a 39-digit
 * multiplication cannot round it. A non-unit coefficient needs at most 112
 * factors because 2^113 has 35 digits.
 *
 * \param[in] operand root operand
 * \param[in] order positive integral root order
 * \param[in,out] root approximate root, replaced when exact
 * \return void
 ***********************************************/
static void correctExactReal34Root(const real_t *operand, const real_t *order, real_t *root) {
  realContext_t nearestContext = ctxtReal34;
  real_t nearest, power, reducedOperand;
  bool_t conversionError, negativePower;
  int32_t n;

  if(realIsSpecial(operand) || realIsZero(operand) || realIsSpecial(order) || !realIsAnInteger(order) || !realIsPositive(order)) {
    return;
  }
  n = realToInt32C47(order, &conversionError);
  if(conversionError) {
    return;
  }

  nearestContext.round = DEC_ROUND_HALF_EVEN;
  realPlus(root, &nearest, &nearestContext);
  realReduce(&nearest, &nearest, &ctxtReal39);
  if(n > 112) {
    realReduce(operand, &reducedOperand, &ctxtReal39);
    negativePower = realIsNegative(&nearest) && (n & 1);
    if(nearest.digits == 1 && nearest.lsu[0] == 1 && reducedOperand.digits == 1 && reducedOperand.lsu[0] == 1
       && negativePower == realIsNegative(&reducedOperand) && (int64_t)nearest.exponent * n == reducedOperand.exponent) {
      realCopy(&nearest, root);
    }
    return;
  }

  realSetOne(&power);
  for(int32_t factor = 0; factor < n; factor++) {
    if(power.digits + nearest.digits - 1 > 34) {
      return;
    }
    realMultiply(&power, &nearest, &power, &ctxtReal39);
  }
  if(realCompareEqual(&power, operand)) {
    realCopy(&nearest, root);
  }
}

/********************************************//**
 * \brief Computes Log Y with phase constants matching the working precision
 *
 * The shared polar conversion uses 39-digit constants for axes and infinities.
 * This function supplies the 75-digit constants needed by the Real34 complex
 * root path and retains the side of a finite nonzero imaginary component.
 *
 * \param[in] y Y
 * \param[out] ln logarithm of the magnitude in the real part, phase in radians in the imaginary part
 * \param[in] realContext working context
 * \return void
 ***********************************************/
static void lnXthRootComplex(const cplx_t *y, cplx_t *ln, realContext_t *realContext) {
  const real_t *pi     = realContext->digits > 51 ? const75_pi     : const39_pi;
  const real_t *piOn2  = realContext->digits > 51 ? const75_piOn2  : const39_piOn2;
  const real_t *piOn4  = realContext->digits > 51 ? const75_piOn4  : const39_piOn4;
  const real_t *pi3On4 = realContext->digits > 51 ? const75_3piOn4 : const39_3piOn4;

  lnComplex(y, ln, realContext);
  if(realIsNaN(&y->Real) || realIsNaN(&y->Imag)) {
    return;
  }

  if(realIsInfinite(&y->Real)) {
    if(realIsInfinite(&y->Imag)) {
      realCopy(realIsNegative(&y->Real) ? pi3On4 : piOn4, &ln->Imag);
      if(realIsNegative(&y->Imag)) {
        realSetNegativeSign(&ln->Imag);
      }
    }
    else if(realIsPositive(&y->Real)) {
      realSetZero(&ln->Imag);
      if(realIsNegative(&y->Imag)) {
        realSetNegativeSign(&ln->Imag);
      }
    }
    else {
      realCopy(pi, &ln->Imag);
      if(realIsNegative(&y->Imag) && !realIsZero(&y->Imag)) {
        realSetNegativeSign(&ln->Imag);
      }
    }
  }
  else if(realIsInfinite(&y->Imag) || realIsZero(&y->Real)) {
    realCopy(piOn2, &ln->Imag);
    if(realIsNegative(&y->Imag)) {
      realSetNegativeSign(&ln->Imag);
    }
  }
  else if(realIsZero(&y->Imag)) {
    if(realIsNegative(&y->Real)) {
      realCopy(pi, &ln->Imag);
    }
    else {
      realSetZero(&ln->Imag);
      if(realIsNegative(&y->Imag)) {
        realSetNegativeSign(&ln->Imag);
      }
    }
  }
}

/********************************************//**
 * \brief Tests whether a phase is pi or pi/2 to the rounding of the working digits
 *
 * A phase formed from pi/4 or pi/2 and X can miss the 75-digit pi or pi/2 in its last digits: twice pi/2 is one unit below pi. The bound is that of the
 * test for -pi in cpxXthRoot.
 *
 * \param[in] phase absolute phase in radians
 * \param[in] axis pi or pi/2 at the working digits
 * \param[in] realContext working context
 * \return true when phase and axis differ by less than 1E(3 - digits)
 ***********************************************/
static bool_t phaseOnAxis(const real_t *phase, const real_t *axis, realContext_t *realContext) {
  REAL_T_PTR(difference, 75);

  realSubtract(phase, axis, difference, realContext);
  return realIsZero(difference) || difference->exponent + difference->digits - 1 <= 2 - realContext->digits;
}

/********************************************//**
 * \brief find R such that R^X == Y
 *
 * Cases with no solution
 * 1) X is pure imaginary and |yImag|>e^|xImag| i.e. ⁱ√25i
 * 2) xReal² + xImag² < |xReal| also depending on Y being typically 
 *    around negative real axis. i.e. ⁰·⁵√(-2)
 *
 * exp(Log Y / X) is a root only when its imaginary
 * part lands in [-pi, pi]. An r whose argument falls outside 
 * does NOT satisfy r^X = Y. Adding 2·pi·k to Arg Y before the division
 * shifts Im into the strip when a suitable integer k exists -- and
 * when no k does, Y has no X-th root.
 *
 * Im is linear in k, so the valid branches are exactly the integers in
 * an interval. k = 0 is picked whenever it is valid (~99% of calls)
 * otherwise the valid k closest to 0 is picked.
 *
 * The strip is tested CLOSED instead of the mathematically correct
 * half-open ]-pi, pi]. Technically wrong but in it lets the X = -1 case
 * through.
 *
 * On the border of the roots existence it may produce a wrong answer.
 * Did some attempts to be more precise but they ended up too 
 * complex and heavy. (Jolbas)
 *
 * If the result of the above change which cleans up the residue, wants to be a negative real, i.e. -𝝅 rad,
 * it is not a possible root, e.g. xrooty(-5i, 0.5) = .5th root of (5∠-𝝅/2) = (5∠-𝝅/2)(5∠-𝝅/2) = 25∠-𝝅 with
 * -𝝅 out of the range (-𝝅, 𝝅], so add 2𝝅 which gives 25∠+𝝅 =  -25 and -25^0.5 = 5i ≠ -5i; so not a root.
 * 
 * \return true with the root in r; false when no such r exists
 ***********************************************/
static bool_t cpxXthRoot(const cplx_t *y, const cplx_t *x, cplx_t *r, realContext_t *realContext) {
  real_t x2, k, step;
  cplx_t lnY, lnR;
  const real_t *pi    = realContext->digits > 51 ? const75_pi    : const39_pi;
  const real_t *piOn2 = realContext->digits > 51 ? const75_piOn2 : const39_piOn2;
  const real_t *pi2   = realContext->digits > 51 ? const75_2pi   : const39_2pi;
  bool_t imWasPositive;

  if(realIsZero(&x->Real) && realIsZero(&x->Imag)) {             // r^0 = 1 whatever r is
    if(realCompareEqual(&y->Real, const_1) && realIsZero(&y->Imag)) {
      realSetOne(&r->Real);
      realSetZero(&r->Imag);
      return true;
    }
    return false;
  }

  if(realCompareEqual(&x->Real, const_1) && realIsZero(&x->Imag) && !realIsNaN(&y->Real) && !realIsNaN(&y->Imag)) {
    realCopy(&y->Real, &r->Real);
    realCopy(&y->Imag, &r->Imag);
    return true;
  }

  if(realIsZero(&y->Real) && realIsZero(&y->Imag)) {             // 0^X = 0 needs Re X > 0
    if(realCompareGreaterThan(&x->Real, const_0)) {
      realSetZero(&r->Real);
      realSetZero(&r->Imag);
      return true;
    }
    return false;
  }

  if(realCompareEqual(&x->Real, const__1) && realIsZero(&x->Imag) && !realIsNaN(&y->Real) && !realIsNaN(&y->Imag)) {
    divRealComplex(const_1, y, r, realContext);
    if(realIsZero(&y->Imag)) {
      realSetZero(&r->Imag);
    }
    return true;
  }

  if(realCompareAbsEqual(&x->Real, const_1on2) && realIsZero(&x->Imag) && !realIsSpecial(&y->Real) && !realIsSpecial(&y->Imag)) {
    const bool_t positiveHalf = realIsPositive(&x->Real);

    if(!realCompareGreaterThan(&y->Real, const_0)
       && !(realIsZero(&y->Real) && (positiveHalf ? realCompareGreaterThan(&y->Imag, const_0) : realCompareLessThan(&y->Imag, const_0)))) {
      return false;
    }
    mulComplexComplex(y, y, r, realContext);
    if(!positiveHalf) {
      divRealComplex(const_1, r, r, realContext);
    }
    if(realIsZero(&r->Real)) {
      realSetZero(&r->Real);
    }
    if(realIsZero(&r->Imag)) {
      realSetZero(&r->Imag);
    }
    return true;
  }

  lnXthRootComplex(y, &lnY, realContext);
  realMultiply(&x->Real, &x->Real, &x2, realContext);
  realFMA(&x->Imag, &x->Imag, &x2, &x2, realContext);            // |X|²

  realMultiply(&x->Real, &lnY.Imag, &lnR.Imag, realContext);
  if(!realIsZero(&x->Imag)) {                                    // a real X would make this 0*inf = NaN for an infinite Y
    realMultiply(&x->Imag, &lnY.Real, &lnR.Real, realContext);
    realSubtract(&lnR.Imag, &lnR.Real, &lnR.Imag, realContext);
  }
  realDivide(&lnR.Imag, &x2, &lnR.Imag, realContext);            // Im(Log Y / X), branch 0

  if(realIsSpecial(&lnR.Imag)) {
    return false;                                                // Arg r runs off: an infinite Y with a complex X, or NaN in
  }
  realSetZero(&k);

  if(realCompareAbsGreaterThan(&lnR.Imag, pi)) {                 // branch 0 is out
    if(realIsZero(&x->Real)) {
      return false;                                              // Im does not depend on k
    }

    realCopyAbs(&x->Real, &step);
    realMultiply(&step, pi2, &step, realContext);
    realDivide(&step, &x2, &step, realContext);                  // k steps for Im

    // Steps to bring |Im| back to at most pi.
    realCopyAbs(&lnR.Imag, &k);
    realSubtract(&k, pi, &k, realContext);
    realDivide(&k, &step, &k, realContext);
    realToIntegralValue(&k, &k, DEC_ROUND_CEILING, realContext); // at least 1

    imWasPositive = realIsPositive(&lnR.Imag);
    if(imWasPositive) {
      realChangeSign(&step);
    }
    realFMA(&k, &step, &lnR.Imag, &lnR.Imag, realContext);
    if(realCompareAbsGreaterThan(&lnR.Imag, pi)) {
      return false;                                              // no k gives a valid root
    }

    if(imWasPositive == realIsPositive(&x->Real)) {
      realChangeSign(&k);
    }
    realMultiply(&k, pi2, &k, realContext);                      // k·2·pi
  }

  if(realIsNegative(&lnR.Imag) && realIsZero(&x->Imag) && !realIsAnInteger(&x->Real)) {
    realCopyAbs(&lnR.Imag, &step);
    realSubtract(pi, &step, &step, realContext);
    realSetPositiveSign(&step);
    if(realIsZero(&step) || step.exponent + step.digits - 1 <= 2 - realContext->digits) {
      return false;
    }
  }

  realAdd(&lnY.Imag, &k, &lnR.Real, realContext);
  realMultiply(&x->Imag, &lnR.Real, &lnR.Real, realContext);
  realFMA(&x->Real, &lnY.Real, &lnR.Real, &lnR.Real, realContext);
  realDivide(&lnR.Real, &x2, &lnR.Real, realContext);            // Re(Log r)

  if(realIsInfinite(&lnR.Real)) {
    realPolarToRectangular(realIsPositive(&lnR.Real) ? const_plusInfinity : const_0, &lnR.Imag, r, realContext);
  }
  else {
    expComplex(&lnR, r, realContext);
  }

  if(realIsZero(&lnR.Imag) && realIsZero(&r->Imag)) {
    realSetZero(&r->Imag);
    if(realIsNegative(&lnR.Imag)) {
      realSetNegativeSign(&r->Imag);
    }
  }
  if(realIsPositive(&y->Real) && realIsZero(&y->Imag) && realIsZero(&x->Imag) && realIsZero(&r->Imag)) {
    realSetZero(&r->Imag);
  }

  // Remove residue due to going through polar form and back
  realCopyAbs(&lnR.Imag, &k);
  if(phaseOnAxis(&k, pi, realContext)) {
    if(realIsNegative(&lnR.Imag) && !(realIsZero(&x->Imag) && realIsAnInteger(&x->Real))) {
      return false;                                              // exp gives Arg r = +pi, so this r is not a root
    }
    realSetZero(&r->Imag);
  }
  else if(phaseOnAxis(&k, piOn2, realContext)) {
    realSetZero(&r->Real);
  }

  return true;
}

/********************************************//**
 * \brief (a+ib) ^ (1/(c+id))
 *
 * \param[in] Expecting a,b,c,d:   Y = a +ib;   X = c +id
 * \return REGISTER Y unchanged. REGISTER X with result of (a+ib) ^ (1/(c+id))
 ***********************************************/
static void xthRootComplex(const cplx_t *y, const cplx_t *x, realContext_t *realContext) {
  cplx_t r;

  // X = 0 is not tested here: cpxXthRoot does that.
  if(!getSystemFlag(FLAG_SPCRES) && (realIsNaN(&y->Real) || realIsNaN(&y->Imag) || realIsNaN(&x->Real) || realIsNaN(&x->Imag))) {
    convertComplexPartsToResultRegister(const_NaN, const_NaN, REGISTER_X);
    return;
  }

  if(cpxXthRoot(y, x, &r, realContext)) {
    if(realIsPositive(&y->Real) && realIsZero(&y->Imag) && realIsZero(&x->Imag) && realIsZero(&r.Imag)) {
      correctExactReal34Root(&y->Real, &x->Real, &r.Real);
    }
    convertComplexToResultRegister(&r, REGISTER_X);
    return;
  }

  // Y has no X-th root at all -- not "none on the branch we looked at". The
  // branches searched are branches of Log Y, and every one of them was ruled
  // out. This is a real answer, not a failure to compute: Y = (-8) with X = 0.5
  // asks for an R whose square root is -8, and no complex number has that.
  if(getSystemFlag(FLAG_SPCRES)) {
    if(getRegisterDataType(REGISTER_X) == dtComplex34 || getRegisterDataType(REGISTER_Y) == dtComplex34) {
      convertComplexPartsToResultRegister(const_NaN, const_NaN, REGISTER_X);
    }
    else {
      convertRealToResultRegister(const_NaN, REGISTER_X, amNone);   // real in, real NaN out
    }
  }
  else {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function xthRootComplex:", "Y has no X-th root: no R satisfies R" STD_SUP_x " = Y", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


/********************************************//**
 * \brief y^(1/x)
 *
 * \param[in] Expecting x,y
 * \return REGISTER Y unchanged. REGISTER X with result of y^x
 ***********************************************/
static void xthRootRealWithParity(const real_t *yy, const real_t *xx, realContext_t *realContext, int32_t integerParity) {
  real_t r, o, order, x, y;
  bool_t haveResult = false;
  bool_t orderIsInteger;

  realCopy(xx, &x);
  realCopy(xx, &order);
  realCopy(yy, &y);
  if(integerParity >= 0) {
    orderIsInteger = true;
    int32ToReal(integerParity, &r);
  }
  else {
    orderIsInteger = realIsAnInteger(&x);
    realDivideRemainder(&x, const_2, &r, realContext);
  }

  if(getSystemFlag(FLAG_SPCRES)) {
    //0
    if(   ((realIsZero(&y)                            && (realCompareGreaterEqual(&x, const_0) || (realIsInfinite(&x) && realIsPositive(&x)))))
       || ((realIsInfinite(&y) && realIsPositive(&y)) && (realCompareLessThan(&x, const_0) && (!realIsInfinite(&x))))
      ) {
      haveResult = true;
      realSetZero(&o);
    }

    //1
    if(((realCompareGreaterEqual(&y, const_0) || (realIsInfinite(&y) && realIsPositive(&y))) && realIsInfinite(&x))) {
      haveResult = true;
      realSetOne(&o);
    }

    //inf
    if(   (!realIsInfinite(&x))                                                                                                   // x finite, common to both cases
       && (   (realIsZero(&y)                             && realCompareLessThan(&x, const_0))                                    // (y=0.)    AND (-inf < x < 0)
           || ((realIsInfinite(&y) && realIsPositive(&y)) && realCompareGreaterEqual(&x, const_0))                                 // (y=+inf)  AND (0 <= x < inf)
          )) {
      haveResult = true;
      realSetPlusInfinity(&o);
    }

    //NaN
    if(    (realIsNaN(&x) || realIsNaN(&y))
       || ((realCompareLessThan(&y, const_0) || (realIsInfinite(&y) && realIsNegative(&y))) && (realIsInfinite(&x)   ))                  // (-inf <= y < 0)  AND (x =(inf or -inf))
       || ((realCompareLessThan(&y, const_0) && (!realIsInfinite(&y)                      ) && (!orderIsInteger) && (!getFlag(FLAG_CPXRES)))) // (-inf < y < 0)  AND (x is non-integer)  AND no complex results allowed
                                                                                                                                        // with CPXRES set this falls through to xthRootComplex instead: a
                                                                                                                                        // negative base with a non-integer root order has a perfectly good
                                                                                                                                        // complex root whenever |x| >= 1, e.g. (-8) with x = 1.5
       || ((realIsInfinite(&y) && realIsNegative(&y)) && (realIsZero(&r) && realCompareGreaterThan(&x, const_0)) && !getFlag(FLAG_CPXRES)) // (y=-inf) AND (x is even > 0) [zero r means x is a multiple of 2, so an even integer], which has a complex root when CPXRES is set
      ) {
      haveResult = true;
      realSetNaN(&o);
    }

    //-inf
    if((realIsInfinite(&y) && realIsNegative(&y)) && (realCompareAbsEqual(&r, const_1) && realCompareGreaterThan(&x, const_0))) { // (y=-inf) AND (x is odd > 0) [r still holds x mod 2; only an odd integer leaves +-1]
      haveResult = true;
      realSetMinusInfinity(&o);
    }
  }
  else { // not DANGER
    if(realIsZero(&x)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function xthRootReal: 0th Root is not defined!", NULL, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
    if(realIsNaN(&x) || realIsNaN(&y)) {
      haveResult = true;
      realSetNaN(&o);
    }
  }

  if(!haveResult) {
    realContext_t *powerContext = realContext->digits < 51 ? &ctxtReal51 : realContext;   // y^(1/x) multiplies the error of 1/x by |ln y|, up to 14200

    if(realCompareEqual(&x, const_1)) {
      realCopy(&y, &o);
    }
    else if(realCompareEqual(&x, const__1)) {
      realDivide(const_1, &y, &o, realContext);
    }
    else if(realIsPositive(&y)) {                                  //positive base, no problem, get the power function y^(1/x)
      realDivide(const_1, &x, &x, powerContext);
      PowerReal(&y, &x, &o, powerContext);
    }
    else {
      // negative base and odd exp: the root is real. 
      if(orderIsInteger && realCompareAbsEqual(&r, const_1)) {
        realDivide(const_1, &x, &x, powerContext);

        realSetPositiveSign(&y);
        PowerReal(&y, &x, &o, powerContext);
        realSetNegativeSign(&o);
      }
      else {
        // even exp, or neither odd nor even i.e. not integer: complex either way
        if(!getFlag(FLAG_CPXRES)) {
          displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            moreInfoOnError("In function xthRootReal:", "cannot do complex xthRoots when CPXRES is not set", NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          return;
        }
        cplx_t yc, xc;

        realCopy(&y, &yc.Real);
        realSetZero(&yc.Imag);
        realCopy(&x, &xc.Real);
        realSetZero(&xc.Imag);
        xthRootComplex(&yc, &xc, &ctxtReal75);
        return;
      }
    }
  }

  if(realContext->digits == 39) {
    correctExactReal34Root(yy, &order, &o);
  }

  convertRealToResultRegister(&o, REGISTER_X, amNone);
}


void xthRootReal(const real_t *yy, const real_t *xx, realContext_t *realContext) {
  xthRootRealWithParity(yy, xx, realContext, -1);
}


static bool_t exactNegativeLongIntegerSquareRoot(const longInteger_t operand, bool_t reciprocal, real34_t *result) {
  longInteger_t magnitude;

  if(!longIntegerIsNegative(operand) || !getFlag(FLAG_CPXRES)) {
    return false;
  }
  longIntegerInit(magnitude);
  longIntegerCopy(operand, magnitude);
  longIntegerSetPositiveSign(magnitude);
  const bool_t ok = reciprocal ? reciprocalRootLongIntegerToReal34(magnitude, 2u, result)
                               : rootLongIntegerToReal34(magnitude, 2u, result);
  longIntegerFree(magnitude);
  if(ok && reciprocal) {
    real34SetNegativeSign(result);
  }
  return ok;
}


/********************************************//**
 * \brief Y(long integer) ^ 1/X(long integer) ==> X(long integer)
 *
 * \param void
 * \return void
 ***********************************************/
static void doXthRootLonI(void) {
  real_t x, y;
  real34_t exactRoot, reciprocal;
  longInteger_t base, exponent, l;
  int32_t exp;

  if(!getRegisterAsLongInt(REGISTER_Y, base, NULL)) {
    goto end1;
  }
  if(!getRegisterAsLongInt(REGISTER_X, exponent, NULL)) {
    goto end2;
  }

  if(longIntegerIsZero(exponent) && !getSystemFlag(FLAG_SPCRES)) {    // 1/0 is not possible
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function doXthRootLonI: Cannot divide by 0!", NULL, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    goto end2;
  }

  if(longIntegerIsZero(base) && longIntegerIsPositive(exponent)) {          //base=0 -->  0
    uInt32ToLongInteger(0u, base);
    convertLongIntegerToLongIntegerRegister(base, REGISTER_X);
    goto end2;
  }

  if(longIntegerCompareInt(exponent, -1) == 0 && reciprocalLongIntegerRegisterToReal34(REGISTER_Y, &reciprocal)) {
    convertReal34ToReal34Register(&reciprocal, REGISTER_X);
    goto end2;
  }

  if((longIntegerCompareInt(exponent, 2) == 0 || longIntegerCompareInt(exponent, -2) == 0)
     && exactNegativeLongIntegerSquareRoot(base, longIntegerIsNegative(exponent), &exactRoot)) {
    convertReal34ToComplex34Register(const34_0, &exactRoot, REGISTER_X);
    goto end2;
  }

  if((longIntegerCompareInt(exponent, -2) == 0 || longIntegerCompareInt(exponent, -3) == 0 || longIntegerCompareInt(exponent, -4) == 0)
     && reciprocalRootLongIntegerToReal34(base, longIntegerCompareInt(exponent, -2) == 0 ? 2u : longIntegerCompareInt(exponent, -3) == 0 ? 3u : 4u, &exactRoot)) {
    convertReal34ToReal34Register(&exactRoot, REGISTER_X);
    goto end2;
  }

  if(longIntegerIsPositive(exponent) && longIntegerCompareUInt(exponent, 2147483640u) <= 0) {
    longIntegerToInt32(exponent, exp);
    if(longIntegerIsPositive(base)) {                                 // pos base
      longIntegerInit(l);
      if(longIntegerRoot(base, exp, l)) {                             // if integer xthRoot found, return
        convertLongIntegerToLongIntegerRegister(l, REGISTER_X);
        longIntegerFree(l);
        goto end2;
      }
      longIntegerFree(l);
    }
    else {
      if(longIntegerIsNegative(base)) {                                 // neg base and even exponent
        if(longIntegerIsOdd(exponent)) {
          longIntegerChangeSign(base);
          longIntegerInit(l);
          if(longIntegerRoot(base, exp, l)) {                           // if negative integer xthRoot found, return
            longIntegerChangeSign(l);
            convertLongIntegerToLongIntegerRegister(l, REGISTER_X);
            longIntegerFree(l);
            goto end2;
          }
          longIntegerFree(l);
          longIntegerChangeSign(base);
        }
      }
    }
    if(exp >= 2 && exp <= 4 && rootLongIntegerToReal34(base, exp, &exactRoot)) {
      convertReal34ToReal34Register(&exactRoot, REGISTER_X);
      goto end2;
    }
  }

  if(!getRegisterAsReal(REGISTER_X, &x) || !getRegisterAsReal(REGISTER_Y, &y)) {
    goto end2;
  }

  xthRootRealWithParity(&y, &x, &ctxtReal75, getRegisterIntegerParity(REGISTER_X));

end2:
  longIntegerFree(exponent);
end1:
  longIntegerFree(base);
}

/********************************************//**
 * \brief Y(short integer) ^ 1/X(short integer) ==> X(short integer)
 *
 * \param void
 * \return void
 ***********************************************/
static void doXthRootShoI(void) {
  const uint32_t base = getRegisterShortIntegerBase(REGISTER_Y);

  convertShortIntegerRegisterToLongIntegerRegister(REGISTER_X, REGISTER_X);
  convertShortIntegerRegisterToLongIntegerRegister(REGISTER_Y, REGISTER_Y);

  doXthRootLonI();

  if(getRegisterDataType(REGISTER_X) == dtLongInteger) {
    convertLongIntegerRegisterToShortIntegerRegister(REGISTER_X, REGISTER_X);
    setRegisterShortIntegerBase(REGISTER_X, base);
  }
}

/******************************************************************************************************************************************************************************************/
/* real34 ^ ...                                                                                                                                                                           */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(real34) ^ 1/X(real34) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
static void doXthRootReal(void) {
  real_t x, y;
  real34_t exactResult;

  if(getRegisterDataType(REGISTER_Y) == dtLongInteger && getRegisterDataType(REGISTER_X) == dtReal34
     && (real34CompareAbsEqual(REGISTER_REAL34_DATA(REGISTER_X), const34_2)
         || real34CompareAbsEqual(REGISTER_REAL34_DATA(REGISTER_X), const34_3)
         || real34CompareAbsEqual(REGISTER_REAL34_DATA(REGISTER_X), const34_4))) {
    longInteger_t operand;
    const uint32_t order = real34CompareAbsEqual(REGISTER_REAL34_DATA(REGISTER_X), const34_2) ? 2u
                           : real34CompareAbsEqual(REGISTER_REAL34_DATA(REGISTER_X), const34_3) ? 3u : 4u;

    convertLongIntegerRegisterToLongInteger(REGISTER_Y, operand);
    const bool_t reciprocal = real34IsNegative(REGISTER_REAL34_DATA(REGISTER_X));
    const bool_t complexSquareRoot = order == 2u && exactNegativeLongIntegerSquareRoot(operand, reciprocal, &exactResult);
    const bool_t exactRoot = complexSquareRoot
                             || (reciprocal ? reciprocalRootLongIntegerToReal34(operand, order, &exactResult)
                                            : rootLongIntegerToReal34(operand, order, &exactResult));
    longIntegerFree(operand);
    if(exactRoot) {
      if(complexSquareRoot) {
        convertReal34ToComplex34Register(const34_0, &exactResult, REGISTER_X);
      }
      else {
        convertReal34ToReal34Register(&exactResult, REGISTER_X);
      }
      return;
    }
  }

  if(!(getRegisterDataType(REGISTER_Y) == dtReal34 && real34IsInfinite(REGISTER_REAL34_DATA(REGISTER_Y)))
     && registerIdentityExponentToReal34(REGISTER_Y, REGISTER_X, &exactResult)) {
    convertReal34ToReal34Register(&exactResult, REGISTER_X);
    return;
  }

  if(getRegisterDataType(REGISTER_Y) == dtReal34 && real34IsZero(REGISTER_REAL34_DATA(REGISTER_Y))
     && real34IsNegative(REGISTER_REAL34_DATA(REGISTER_Y))) {
    real34_t signedZero;

    if(!getRegisterAsReal(REGISTER_X, &x)) {
      return;
    }
    if(!realIsSpecial(&x) && realCompareGreaterThan(&x, const_0)) {
      real34Copy(REGISTER_REAL34_DATA(REGISTER_Y), &signedZero);
      convertReal34ToReal34Register(&signedZero, REGISTER_X);
      return;
    }
  }

  if(!getRegisterAsReal(REGISTER_X, &x) || !getRegisterAsReal(REGISTER_Y, &y)) {
    return;
  }

  if((realIsInfinite(&x) || realIsInfinite(&y)) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function doXthRootReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X or Y input of xthRoot when flag SPCRES is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  xthRootRealWithParity(&y, &x, &ctxtReal39, getRegisterIntegerParity(REGISTER_X));
}

/********************************************//**
 * \brief Y(complex34) ^ 1/X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
static void doXthRootCplx(void) {                       //checked
  cplx_t y, x;

  if(!getRegisterAsComplex(REGISTER_Y, &y) || !getRegisterAsComplex(REGISTER_X, &x)) {
    return;
  }

  // An infinite Y used to answer inf + inf i whatever the direction. That is
  // only right for Arg Y = pi/4; crootComplex now derives the direction from
  // Arg Y like sqrt does. The 0th root of an infinity keeps its own answer.
  if((realIsInfinite(&y.Real) || realIsInfinite(&y.Imag)) && realIsZero(&x.Real) && realIsZero(&x.Imag)) {
    convertComplexPartsToResultRegister(const_NaN, const_NaN, REGISTER_X);
    return;
  }

  xthRootComplex(&y, &x, &ctxtReal75);
}

/********************************************//**
 * \brief regX ==> regL and regY ^ (1/regX) ==> regX
 * Drops Y, enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter
 * \return void
 ***********************************************/
void fnXthRoot(uint16_t unusedButMandatoryParameter) {
  real_t order;
  bool_t preserveNegativeZero = getRegisterDataType(REGISTER_Y) == dtReal34
                                && real34IsZero(REGISTER_REAL34_DATA(REGISTER_Y))
                                && real34IsNegative(REGISTER_REAL34_DATA(REGISTER_Y))
                                && getRegisterAsRealQuiet(REGISTER_X, &order)
                                && !realIsSpecial(&order)
                                && realCompareGreaterThan(&order, const_0);

  processIntRealComplexDyadicFunction(&doXthRootReal, &doXthRootCplx, &doXthRootShoI, &doXthRootLonI);
  if(preserveNegativeZero && lastErrorCode == ERROR_NONE && getRegisterDataType(REGISTER_X) == dtReal34
     && real34IsZero(REGISTER_REAL34_DATA(REGISTER_X))) {
    real34SetNegativeSign(REGISTER_REAL34_DATA(REGISTER_X));
  }
}
