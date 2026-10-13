// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file xthRoot.c
 ***********************************************/
// Coded by JM, based on power.c, with reference to cuberoot.c

#include "c47.h"

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
 * \return true and r in rReal, rImag; false when no such r exists
 ***********************************************/
static bool_t cpxXthRoot(const real_t *yReal, const real_t *yImag, const real_t *xReal, const real_t *xImag,
                           real_t *rReal, real_t *rImag, realContext_t *realContext) {
  real_t x2, lnYReal, lnYImag, lnRReal, lnRImag, k, step;
  bool_t imWasPositive;

  if(realIsZero(xReal) && realIsZero(xImag)) {                   // r^0 = 1 whatever r is
    if(realCompareEqual(yReal, const_1) && realIsZero(yImag)) {
      realSetOne(rReal);
      realSetZero(rImag);
      return true;
    }
    return false;
  }

  if(realIsZero(yReal) && realIsZero(yImag)) {                   // 0^X = 0 needs Re X > 0
    if(realCompareGreaterThan(xReal, const_0)) {
      realSetZero(rReal);
      realSetZero(rImag);
      return true;
    }
    return false;
  }

  lnComplex(yReal, yImag, &lnYReal, &lnYImag, realContext);
  realMultiply(xReal, xReal, &x2, realContext);
  realFMA(xImag, xImag, &x2, &x2, realContext);                  // |X|²

  realMultiply(xReal, &lnYImag, &lnRImag, realContext);
  if(!realIsZero(xImag)) {                                       // a real X would make this 0*inf = NaN for an infinite Y
    realMultiply(xImag, &lnYReal, &lnRReal, realContext);
    realSubtract(&lnRImag, &lnRReal, &lnRImag, realContext);
  }
  realDivide(&lnRImag, &x2, &lnRImag, realContext);              // Im(Log Y / X), branch 0

  if(realIsSpecial(&lnRImag)) {
    return false;                                                // Arg r runs off: an infinite Y with a complex X, or NaN in
  }
  realSetZero(&k);

  if(realCompareAbsGreaterThan(&lnRImag, const39_pi)) {          // branch 0 is out
    if(realIsZero(xReal)) {
      return false;                                              // Im does not depend on k
    }

    realCopyAbs(xReal, &step);
    realMultiply(&step, const39_2pi, &step, realContext);
    realDivide(&step, &x2, &step, realContext);                  // k steps for Im

    // Steps to bring |Im| back to at most pi.
    realCopyAbs(&lnRImag, &k);
    realSubtract(&k, const39_pi, &k, realContext);
    realDivide(&k, &step, &k, realContext);
    realToIntegralValue(&k, &k, DEC_ROUND_CEILING, realContext); // at least 1

    imWasPositive = realIsPositive(&lnRImag);
    if(imWasPositive) {
      realChangeSign(&step);
    }
    realFMA(&k, &step, &lnRImag, &lnRImag, realContext);
    if(realCompareAbsGreaterThan(&lnRImag, const39_pi)) {
      return false;                                              // no k gives a valid root
    }

    if(imWasPositive == realIsPositive(xReal)) {
      realChangeSign(&k);
    }
    realMultiply(&k, const39_2pi, &k, realContext);              // k·2·pi
  }

  realAdd(&lnYImag, &k, &lnRReal, realContext);
  realMultiply(xImag, &lnRReal, &lnRReal, realContext);
  realFMA(xReal, &lnYReal, &lnRReal, &lnRReal, realContext);
  realDivide(&lnRReal, &x2, &lnRReal, realContext);              // Re(Log r)

  if(realIsInfinite(&lnRReal)) {
    realPolarToRectangular(realIsPositive(&lnRReal) ? const_plusInfinity : const_0, &lnRImag, rReal, rImag, realContext);
  }
  else {
    expComplex(&lnRReal, &lnRImag, rReal, rImag, realContext);
  }

  // Remove residue due to going through polar form and back
  realCopyAbs(&lnRImag, &k);
  if(realCompareEqual(&k, const39_pi)) {
    if(realIsNegative(&lnRImag) && !(realIsZero(xImag) && realIsAnInteger(xReal))) {
      return false;                                              // exp gives Arg r = +pi, so this r is not a root
    }
    realSetZero(rImag);
  }
  else if(realCompareEqual(&k, const39_piOn2)) {
    realSetZero(rReal);
  }

  return true;
}

/********************************************//**
 * \brief (a+ib) ^ (1/(c+id))
 *
 * \param[in] Expecting a,b,c,d:   Y = a +ib;   X = c +id
 * \return REGISTER Y unchanged. REGISTER X with result of (a+ib) ^ (1/(c+id))
 ***********************************************/
static void xthRootComplex(const real_t *aa, const real_t *bb, const real_t *cc, const real_t *dd, realContext_t *realContext) {
  real_t rReal, rImag;

  // X = 0 is not tested here: cpxXthRoot does that.
  if(!getSystemFlag(FLAG_SPCRES) && (realIsNaN(aa) || realIsNaN(bb) || realIsNaN(cc) || realIsNaN(dd))) {
    convertComplexToResultRegister(const_NaN, const_NaN, REGISTER_X);
    return;
  }

  if(cpxXthRoot(aa, bb, cc, dd, &rReal, &rImag, realContext)) {
    convertComplexToResultRegister(&rReal, &rImag, REGISTER_X);
    return;
  }

  // Y has no X-th root at all -- not "none on the branch we looked at". The
  // branches searched are branches of Log Y, and every one of them was ruled
  // out. This is a real answer, not a failure to compute: Y = (-8) with X = 0.5
  // asks for an R whose square root is -8, and no complex number has that.
  if(getSystemFlag(FLAG_SPCRES)) {
    if(getRegisterDataType(REGISTER_X) == dtComplex34 || getRegisterDataType(REGISTER_Y) == dtComplex34) {
      convertComplexToResultRegister(const_NaN, const_NaN, REGISTER_X);
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
void xthRootReal(const real_t *yy, const real_t *xx, realContext_t *realContext) {
  real_t r, o, x, y;
  bool_t haveResult = false;

  realCopy(xx, &x);
  realCopy(yy, &y);

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
    realDivideRemainder(&x, const_2, &r, realContext);
    if(    (realIsNaN(&x) || realIsNaN(&y))
       || ((realCompareLessThan(&y, const_0) || (realIsInfinite(&y) && realIsNegative(&y))) && (realIsInfinite(&x)   ))                  // (-inf <= y < 0)  AND (x =(inf or -inf))
       || ((realCompareLessThan(&y, const_0) && (!realIsInfinite(&y)                      ) && (!realIsAnInteger(&x)) && (!getFlag(FLAG_CPXRES)))) // (-inf < y < 0)  AND (x is non-integer)  AND no complex results allowed
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
    if(realIsPositive(&y)) {                                       //positive base, no problem, get the power function y^(1/x)
      realDivide(const_1, &x, &x, realContext);
      PowerReal(&y, &x, &o, realContext);
      if(realIsAnInteger(xx) && realCompareGreaterThan(xx, const_1) && !realIsZero(&y)) {   // an n-th power of a number of 34 digits gives that number
        realExactRoot(&y, xx, &o);
      }
    }
    else {
      // negative base and odd exp: the root is real. 
      realDivideRemainder(&x, const_2, &r, realContext);
      if(realCompareAbsEqual(&r, const_1)) {
        realDivide(const_1, &x, &x, realContext);

        realSetPositiveSign(&y);
        PowerReal(&y, &x, &o, realContext);
        if(realCompareGreaterThan(xx, const_1)) {
          realExactRoot(&y, xx, &o);
        }
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
        xthRootComplex(&y, const_0, &x, const_0, realContext);
        return;
      }
    }
  }

  convertRealToResultRegister(&o, REGISTER_X, amNone);
}


/********************************************//**
 * \brief Y(long integer) ^ 1/X(long integer) ==> X(long integer)
 *
 * \param void
 * \return void
 ***********************************************/
static void doXthRootLonI(void) {
  real_t x, y;
  longInteger_t base, exponent, l;
  int32_t exp;

  if(!getRegisterAsLongInt(REGISTER_Y, base, NULL)) {
    goto end1;
  }
  if(!getRegisterAsLongInt(REGISTER_X, exponent, NULL)) {
    goto end2;
  }

  if(longIntegerIsZero(exponent)) {    // 1/0 is not possible
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function doXthRootLonI: Cannot divide by 0!", NULL, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    goto end2;
  }

  if(longIntegerIsZero(base)) {          //base=0 -->  0
    uInt32ToLongInteger(0u, base);
    convertLongIntegerToLongIntegerRegister(base, REGISTER_X);
    goto end2;
  }

  if(longIntegerCompareUInt(base, 2147483640) == -1) {
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
        }
      }
    }
  }

  if(!getRegisterAsReal(REGISTER_X, &x) || !getRegisterAsReal(REGISTER_Y, &y)) {
    goto end2;
  }

  xthRootReal(&y, &x, &ctxtReal75);

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

  xthRootReal(&y, &x, &ctxtReal39);
}

/********************************************//**
 * \brief Y(complex34) ^ 1/X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
static void doXthRootCplx(void) {                       //checked
  real_t a, b, c, d;

  if(!getRegisterAsComplex(REGISTER_Y, &a, &b) || !getRegisterAsComplex(REGISTER_X, &c, &d)) {
    return;
  }

  // An infinite Y used to answer inf + inf i whatever the direction. That is
  // only right for Arg Y = pi/4; crootComplex now derives the direction from
  // Arg Y like sqrt does. The 0th root of an infinity keeps its own answer.
  if((realIsInfinite(&a) || realIsInfinite(&b)) && realIsZero(&c) && realIsZero(&d)) {
    convertComplexToResultRegister(const_NaN, const_NaN, REGISTER_X);
    return;
  }

  xthRootComplex(&a, &b, &c, &d, &ctxtReal39);
}

/********************************************//**
 * \brief regX ==> regL and regY ^ (1/regX) ==> regX
 * Drops Y, enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter
 * \return void
 ***********************************************/
void fnXthRoot(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexDyadicFunction(&doXthRootReal, &doXthRootCplx, &doXthRootShoI, &doXthRootLonI);
}
