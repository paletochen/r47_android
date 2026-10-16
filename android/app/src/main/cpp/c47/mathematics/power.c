// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file power.c
 ***********************************************/

#include "c47.h"

static void powReal(void);
static void powCplx(void);


static void powerRealWithParity(const real_t *y, const real_t *x, real_t *res, realContext_t *realContext, int32_t integerParity) {
  real_t lny;

  realCopyAbs(x, &lny);
  if(realIsAnInteger(x) && realCompareLessEqual(&lny, const_10p9__1)) {
    realPower(y, x, res, realContext);
  }
  else if(realIsNegative(y) && (integerParity >= 0 || realIsAnInteger(x))) {
    bool_t isOdd = false;

    if(integerParity >= 0) {
      isOdd = integerParity == 1;
    }
    else if(!realIsInfinite(x)) {                        // realIsAnInteger() is true for an infinity, which is neither odd nor even
      realDivideRemainder(x, const_2, &lny, realContext);
      isOdd = !realIsZero(&lny);
    }
    realCopyAbs(y, &lny);
    WP34S_Ln(&lny, &lny, realContext);
    realMultiply(x, &lny, res, realContext);
    realExp(res, res, realContext);
    if(isOdd) {
      realChangeSign(res);
    }
  }
  else {
    WP34S_Ln(y, &lny, realContext);
    realMultiply(x, &lny, res, realContext);
    realExp(res, res, realContext);
  }
}


void PowerReal(const real_t *y, const real_t *x, real_t *res, realContext_t *realContext) {
  powerRealWithParity(y, x, res, realContext, -1);
}


/********************************************//**
 * \brief x to the integer power n, when the power is exact
 *
 * The power is taken at 75 digits half even, by repeated multiplication. It is exact when no digit is dropped, which the Inexact status of the context
 * reports.
 *
 * \param[in]  x   const real_t* a real
 * \param[in]  n   const real_t* the exponent
 * \param[out] res real_t*       x^n, when the result is true
 * \return bool_t true when n is an integer below 10000 in magnitude and res is x^n exactly
 ***********************************************/
bool_t realIntegerPowerExact(const real_t *x, const real_t *n, real_t *res) {
  realContext_t c = ctxtReal75;

  if(realIsSpecial(x) || realIsSpecial(n) || !realIsAnInteger(n) || (!realIsZero(n) && realGetExponent(n) > 3)) {
    return false;
  }
  c.round = DEC_ROUND_HALF_EVEN;
  c.status = 0;
  realPower(x, n, res, &c);
  return (c.status & (DEC_Inexact | DEC_Overflow | DEC_Underflow | DEC_Invalid_operation | DEC_Division_by_zero)) == 0 && !realIsSpecial(res);
}


/********************************************//**
 * \brief The n-th root of x, exact where x is the n-th power of a number of 34 digits
 *
 * root rounded half even to 34 digits is the exact root when one of 34 digits exists. The test is that its n-th power is x exactly.
 *
 * \param[in]     x    const real_t* a positive real
 * \param[in]     n    const real_t* an integer of 2 or more
 * \param[in,out] root real_t*       the root at working precision, replaced by the exact root when there is one
 * \return bool_t true when root is replaced
 ***********************************************/
bool_t realExactRoot(const real_t *x, const real_t *n, real_t *root) {
  real_t c, p;
  realContext_t c34 = ctxtReal34;

  c34.round = DEC_ROUND_HALF_EVEN;
  realPlus(root, &c, &c34);
  if(!realIntegerPowerExact(&c, n, &p) || !realCompareEqual(&p, x)) {
    return false;
  }
  realCopy(&c, root);
  return true;
}


/******************************************************************************************************************************************************************************************/
/* long integer ^ ...                                                                                                                                                                     */
/******************************************************************************************************************************************************************************************/

void longIntegerPower(longInteger_t base, longInteger_t exponent, longInteger_t result) {
  if(longIntegerIsZero(exponent)) {
    uInt32ToLongInteger(1u, result);
  }
  else if(longIntegerIsZero(base)) {
    uInt32ToLongInteger(0u, result);
  }
  else if((longIntegerCompareInt(base, 1) == 0 || longIntegerCompareInt(base, -1) == 0) && longIntegerCompareInt(exponent, -1) == 0) {
    longIntegerCopy(base, result);
  }
  else if(longIntegerIsNegative(exponent)) {
    uInt32ToLongInteger(0u, result);
  }
  else {
    uInt32ToLongInteger(1u, result);

    while(!longIntegerIsZero(exponent)) {
      if(longIntegerIsOdd(exponent)) {
       longIntegerMultiply(result, base, result);
      }

      longIntegerDivide2(exponent, exponent);

      if(!longIntegerIsZero(exponent)) {
        longIntegerSquare(base, base);
      }
    }
  }
}

/********************************************//**
 * \brief Y(short integer) ^ X(short integer) ==> X(short integer)
 *
 * \param void
 * \return void
 ***********************************************/
static void powShoI(void) {
  int32_t exponentSign, baseSign;

  uint64_t exponent = WP34S_extract_value(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)), &exponentSign);
  uint64_t base = WP34S_extract_value(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_Y)), &baseSign);

  if(base == 1 && exponent == 1 && exponentSign) {
    setRegisterShortIntegerBase(REGISTER_X, getRegisterShortIntegerBase(REGISTER_Y));
    *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = *(REGISTER_SHORT_INTEGER_DATA(REGISTER_Y));
  }
  else if(exponentSign) { // exponent is negative
    powReal();
  }
  else {
    setRegisterShortIntegerBase(REGISTER_X, getRegisterShortIntegerBase(REGISTER_Y));
    *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_intPower(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_Y)), *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)));
  }
}


/********************************************//**
 * \brief Y(long integer) ^ X(long integer) ==> X(long integer)
 *
 * \param void
 * \return void
 ***********************************************/
static void powLonI(void) {
  longInteger_t base, exponent, result;
  real34_t exactPower, reciprocal;

  if(!getRegisterAsLongInt(REGISTER_Y, base, NULL)) {
    goto finish1;
  }
  if(!getRegisterAsLongInt(REGISTER_X, exponent, NULL)) {
    goto finish2;
  }

  if(longIntegerIsZero(exponent) && longIntegerIsZero(base)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function powLonI: Cannot calculate 0^0!", NULL, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    goto finish2;
  }

  if((longIntegerCompareInt(base, 1) == 0 || longIntegerCompareInt(base, -1) == 0) && longIntegerCompareInt(exponent, -1) == 0) {
    convertLongIntegerToLongIntegerRegister(base, REGISTER_X);
    goto finish2;
  }
  else if(longIntegerCompareInt(exponent, -1) == 0 && reciprocalLongIntegerRegisterToReal34(REGISTER_Y, &reciprocal)) {
    convertReal34ToReal34Register(&reciprocal, REGISTER_X);
    goto finish2;
  }
  else if((longIntegerCompareInt(exponent, -2) == 0 || longIntegerCompareInt(exponent, -3) == 0 || longIntegerCompareInt(exponent, -4) == 0)
          && powerLongIntegerToReal34(base, longIntegerCompareInt(exponent, -2) == 0 ? -2 : longIntegerCompareInt(exponent, -3) == 0 ? -3 : -4, &exactPower)) {
    convertReal34ToReal34Register(&exactPower, REGISTER_X);
    goto finish2;
  }
  else if(longIntegerIsNegative(exponent)) {
    powReal();
    goto finish2;
  }

  longIntegerInit(result);
  longIntegerPower(base, exponent, result);

  convertLongIntegerToLongIntegerRegister(result, REGISTER_X);

  longIntegerFree(result);
finish2:
  longIntegerFree(exponent);
finish1:
  longIntegerFree(base);
}


/******************************************************************************************************************************************************************************************/
/* real34 ^ ...                                                                                                                                                                           */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(real34) ^ X(real34) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
static void powReal(void) {
  real_t y, x, res;
  real34_t exactResult;
  const int32_t integerParity = getRegisterIntegerParity(REGISTER_X);

  if(getRegisterDataType(REGISTER_Y) == dtLongInteger && getRegisterDataType(REGISTER_X) == dtReal34 && real34IsAnInteger(REGISTER_REAL34_DATA(REGISTER_X))) {
    longInteger_t base;
    const int32_t exponent = real34ToInt32(REGISTER_REAL34_DATA(REGISTER_X));   // 0 out of range; powerLongIntegerToReal34 takes ±2, ±3 and ±4 only

    convertLongIntegerRegisterToLongInteger(REGISTER_Y, base);
    const bool_t exactPower = powerLongIntegerToReal34(base, exponent, &exactResult);
    longIntegerFree(base);
    if(exactPower) {
      convertReal34ToReal34Register(&exactResult, REGISTER_X);
      return;
    }
  }

  if(registerIdentityExponentToReal34(REGISTER_Y, REGISTER_X, &exactResult)) {
    convertReal34ToReal34Register(&exactResult, REGISTER_X);
    return;
  }

  if(!getRegisterAsReal(REGISTER_X, &x) || !getRegisterAsReal(REGISTER_Y, &y)) {
    return;
  }

  if((realCompareEqual(&x, const_1on2) || realCompareEqual(&x, const_1on4))
     && rootRegisterToReal34(REGISTER_Y, realCompareEqual(&x, const_1on2) ? 2u : 4u, false, &exactResult)) {   // y^(1/n) is the n-th root of y, rounded once
    convertReal34ToReal34Register(&exactResult, REGISTER_X);
    return;
  }

  if(realCompareEqual(&x, const_1on2) && realIsNegative(&y) && !realIsZero(&y) && !realIsInfinite(&y) && getFlag(FLAG_CPXRES)) {   // (-y)^(1/2) is 0 + i√y, as √ gives it
    realSetPositiveSign(&y);
    realSquareRoot(&y, &y, &ctxtReal39);
    convertComplexPartsToResultRegister(const_0, &y, REGISTER_X);
    return;
  }

  if(realIsInfinite(&y)) {                             // y (the base) is +/- infinity
    if(realIsNaN(&x)) {                                // inf ^ NaN is NaN; must precede the tests below, realIsPositive() is true for a NaN
      realSetNaN(&res);
    }
    else if(realIsZero(&x)) {                          // inf ^ 0 is indeterminate
      realSetNaN(&res);
    }
    else {
      bool_t oddIntegerExponent = false;
      if(!realIsInfinite(&x)) {                        // ReM p.202: y^(+/-inf) follows the exponent sign alone; realIsAnInteger() is true for an infinity
        if(integerParity >= 0) {
          oddIntegerExponent = integerParity == 1;
        }
        else if(realIsAnInteger(&x)) {
          WP34S_Mod(&x, const_2, &res, &ctxtReal39);
          oddIntegerExponent = !realIsZero(&res);
        }
        else if(realIsNegative(&y)) {                  // (-inf) ^ non-integer is not a real number
          if(getFlag(FLAG_CPXRES)) {
            powCplx();
            return;
          }
          displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            moreInfoOnError("In function powReal:", "cannot do complex results if CPXRES is not set", NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          return;
        }
      }
      if(realIsPositive(&x)) {                         // exponent > 0  ==> +/- infinity
        realSetPlusInfinity(&res);
      }
      else {                                           // exponent < 0  ==> +/- zero
        realSetZero(&res);
      }
      if(realIsNegative(&y) && oddIntegerExponent) {   // (-inf) ^ odd keeps the negative sign
        realSetNegativeSign(&res);
      }
    }
    goto finish;
  }

  if(realIsZero(&y) && !realIsAnInteger(&x)) {        // (-0) ^ non-integer is (+0) ^ non-integer, a real
    realSetPositiveSign(&y);
  }

  if(!realIsZero(&y) && realIntegerPowerExact(&y, &x, &res)) {   // an integer exponent with an exact power: the power itself, rounded once by RM
    goto finish;
  }

  powerRealWithParity(&y, &x, &res, realIsAnInteger(&x) ? &ctxtReal39 : &ctxtReal51, integerParity);

  if(realIsPositive(&y) && !realIsZero(&x) && !realIsSpecial(&res)) {   // an exponent 1/n with an exact n-th root of y gives that root
    real_t n;
    realDivide(const_1, &x, &n, &ctxtReal39);
    if(realIsAnInteger(&n) && realCompareGreaterThan(&n, const_1)) {
      realExactRoot(&y, &n, &res);
    }
  }

  if(realIsNaN(&res) && realIsNegative(&y) && !realIsAnInteger(&x)) {
    if(getFlag(FLAG_CPXRES)) {
      powCplx();
      return;
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function powReal:", "cannot do complex results if CPXRES is not set", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }

finish:
  convertRealToResultRegister(&res, REGISTER_X, amNone);
}


/******************************************************************************************************************************************************************************************/
/* complex34 + ...                                                                                                                                                                        */
/******************************************************************************************************************************************************************************************/

/*
 * ReM p.B-18: a power of (r ; phi) is (r^x ; phi.x). An infinite base sits on one of the 8 sector boundaries that
 * getInfiniteComplexAngle() reports, so phi.x is representable only when it lands on one too. Returns false when it does
 * not, the exponent being infinite or the product fractional, leaving the caller to return an infinity of no angle.
 */
static bool_t setInfiniteComplexPower(const cplx_t *y, const real_t *xReal, int32_t exponentModulo8, cplx_t *r, realContext_t *realContext) {
  real_t sector, residue;
  const real_t *exponent = xReal;

  if(realIsInfinite(xReal)) {                            // phi.x is then unbounded, and WP34S_Mod() of it is a NaN
    return false;
  }

  int32ToReal(getInfiniteComplexAngle(y), &sector);
  if(exponentModulo8 >= 0) {                             // a long integer exponent above 75 digits comes rounded in xReal
    int32ToReal(exponentModulo8, &residue);
    exponent = &residue;
  }
  realMultiply(&sector, exponent, &sector, realContext);
  WP34S_Mod(&sector, const_8, &sector, realContext);     // remainder is truncated, so lift it into [0, 8) = one revolution
  if(realIsNegative(&sector)) {
    realAdd(&sector, const_8, &sector, realContext);
  }

  if(!realIsAnInteger(&sector)) {
    return false;
  }
  setInfiniteComplexAngle(realToInt32C47(&sector, NULL), r);
  return true;
}


/********************************************//**
 * \brief Computes the principal logarithm of a finite nonzero complex base
 *
 * \param[in] z the base
 * \param[out] lnReal logarithm of the magnitude
 * \param[out] lnImag phase in radians
 * \param[in] realContext working context
 * \return void
 ***********************************************/
static void lnPowerComplex(const cplx_t *z, real_t *lnReal, real_t *lnImag, realContext_t *realContext) {
  REAL_T_PTR(delta, 75);
  REAL_T_PTR(sum, 75);

  realMultiply(&z->Real, &z->Real, lnReal, realContext);
  realFMA(&z->Imag, &z->Imag, lnReal, lnReal, realContext);
  realSquareRoot(lnReal, lnReal, realContext);
  C47_WP34S_Atan2(&z->Imag, &z->Real, lnImag, realContext);

  if(realContext->digits == 75 && realCompareGreaterThan(lnReal, const_1on2) && realCompareLessThan(lnReal, const_2)) {
    if(realCompareAbsLessThan(&z->Real, &z->Imag)) {
      realSubtract(&z->Imag, const_1, delta, realContext);
      realAdd(&z->Imag, const_1, sum, realContext);
      realMultiply(delta, sum, delta, realContext);
      realFMA(&z->Real, &z->Real, delta, delta, realContext);
    } else {
      realSubtract(&z->Real, const_1, delta, realContext);
      realAdd(&z->Real, const_1, sum, realContext);
      realMultiply(delta, sum, delta, realContext);
      realFMA(&z->Imag, &z->Imag, delta, delta, realContext);
    }
    WP34S_Ln1P(delta, lnReal, realContext);
    realMultiply(lnReal, const_1on2, lnReal, realContext);
  } else {
    WP34S_Ln(lnReal, lnReal, realContext);
  }
}


/********************************************//**
 * \brief Reduces the real exponent for an exact-axis phase
 *
 * \param[in] exponent real component of the exponent
 * \param[in] period phase period in exponent units
 * \param[in] exponentModulo8 a long integer exponent modulo 8, a multiple of every period, which the real component rounded above 75 digits no longer gives, or -1
 * \param[out] remainder reduced exponent
 * \param[in] realContext working context
 * \return void
 ***********************************************/
static void reducePowerExponent(const real_t *exponent, const real_t *period, int32_t exponentModulo8, real_t *remainder, realContext_t *realContext) {
  if(exponentModulo8 >= 0) {
    int32ToReal(exponentModulo8, remainder);
  }
  else if(exponent->digits <= 34 && period->digits <= 34) {
    WP34S_Real34Remainder(exponent, period, remainder);
  } else {
    WP34S_Mod(exponent, period, remainder, realContext);
  }
}


/*
 * Calculate y^x for complex numbers.
 */
static uint8_t powerComplexWithResidue(const cplx_t *y, const cplx_t *x, cplx_t *r, realContext_t *realContext, int32_t exponentModulo8) {
  uint8_t errorCode = ERROR_NONE;

  if(realIsInfinite(&y->Real) || realIsInfinite(&y->Imag)) {  // the base has an infinite component
    if(realIsNaN(&x->Real) || realIsNaN(&x->Imag)) {     // inf ^ NaN is NaN
      realSetNaN(&r->Real);
      realSetNaN(&r->Imag);
    }
    else if(realIsZero(&x->Real) && realIsZero(&x->Imag)) {  // inf ^ 0 is indeterminate
      realSetNaN(&r->Real);
      realSetNaN(&r->Imag);
    }
    else if(realIsZero(&x->Real)) {                      // Re(exponent) == 0 (pure imaginary) is indeterminate
      realSetNaN(&r->Real);
      realSetNaN(&r->Imag);
    }
    else if(realIsNegative(&x->Real)) {                  // Re(exponent) < 0 ==> modulus collapses to 0, where the angle no longer counts
      realSetZero(&r->Real);
      realSetZero(&r->Imag);
    }
    else {                                               // Re(exponent) > 0 ==> infinite modulus
      bool_t onSector = false;

      if(realIsZero(&x->Imag)) {                         // Im(exponent) != 0 adds Im(x).ln r to the argument, leaving it unbounded
        onSector = setInfiniteComplexPower(y, &x->Real, exponentModulo8, r, realContext);
      }
      if(!onSector) {
        realSetPlusInfinity(&r->Real);
        realSetPlusInfinity(&r->Imag);
      }
    }
  }
  else if(realIsZero(&y->Real) && realIsZero(&y->Imag)) {
    if(realIsNaN(&x->Real) || realIsNaN(&x->Imag) || realIsZero(&x->Real)) {
      realSetNaN(&r->Real);
      realSetNaN(&r->Imag);
    }
    else {
      realSetZero(&r->Real);
      realSetZero(&r->Imag);
    }
  }
  else if(realCompareEqual(&x->Real, const_1) && realIsZero(&x->Imag) && !realIsNaN(&y->Real) && !realIsNaN(&y->Imag)) {
    realCopy(&y->Real, &r->Real);
    realCopy(&y->Imag, &r->Imag);
  }
  else {
    real_t theta;
    real_t tmp;

    // (Yr+iYi) ^ (Xr+iXi)
    // EXP [  (Xr+iXi) LN (Yr+iYi)  ]
    // EXP [  (Xr+iXi) LN (r angle theta)  ]
    // EXP [  Xr.LN (r angle theta)   +   i.(Xi.LN (r angle theta)) ]
    // EXP [ (Xr.LN r) * (-theta Xi)  +   i.(Xi.LN r + (theta . Xr)) ]  [5]


    lnPowerComplex(y, &r->Real, &theta, realContext);

    realMultiply(&r->Real, &x->Imag, &r->Imag, realContext);        //rImag = Xi.LN r

    real_t xR;
    int8_t md;
    bool_t doZeroingReal = false;
    bool_t doZeroingImag = false;
    if(realCompareAbsEqual(&y->Real, &y->Imag)) {
      reducePowerExponent(&x->Real, const_8, exponentModulo8, &xR, realContext);     // {See [5], if yR=yI then we have theta = pi/4 exact, then we can do a Xr remainder by 8}
      if(realIsZero(&x->Imag) && realIsAnInteger(&xR)) {           // only compute md when exponent is integer
        md = realToInt32C47(&xR, NULL);
        if(md % 4 == 0) {
          doZeroingImag = true;
        }
        else if((md-2) % 4 == 0) {
          doZeroingReal = true;
        }
      }
    }
    else if(realIsZero(&y->Real)) {
      reducePowerExponent(&x->Real, const_4, exponentModulo8, &xR, realContext);     // {See [5], if yR=0 then we have theta = pi/2 exact, then we can do a Xr remainder by 4}
      if(realIsZero(&x->Imag) && realIsAnInteger(&xR)) {           // only compute md when exponent is integer
        md = realToInt32C47(&xR, NULL);
        if(md % 2 == 0) {
          doZeroingImag = true;
        }
        else {
          doZeroingReal = true;
        }
      }
    }
    else if(realIsZero(&y->Imag) && realIsNegative(&y->Real)) {
      reducePowerExponent(&x->Real, const_2, exponentModulo8, &xR, realContext);
      if(realIsZero(&x->Imag) && realIsAnInteger(&xR)) {
        doZeroingImag = true;
      }
    }
    else {
      realCopy(&x->Real, &xR);
    }

    realFMA(&theta, &xR, &r->Imag, &r->Imag, realContext);          //rImag = Xi.LN r  +  theta . Xr  ===> this theta.Xt is the coefficient of r.e^i.COEF, hence the angle and therefore we can get the remainder after dividing by number of revolutions.
    realChangeSign(&theta);

    realMultiply(&r->Real, &x->Real, &r->Real, realContext);        //rReal = Xr.LN r
    realFMA(&theta, &x->Imag, &r->Real, &r->Real, realContext);     //rReal = Xr.LN r  *  -theta . Xi

    realExp(&r->Real, &tmp, realContext);
    realPolarToRectangular(const_1, &r->Imag, r, realContext);
    if(doZeroingImag) {
      realSetZero(&r->Imag);
    }
    else {
      realMultiply(&tmp, &r->Imag, &r->Imag, realContext);
    }
    if(doZeroingReal) {
      realSetZero(&r->Real);
    }
    else {
      realMultiply(&tmp, &r->Real, &r->Real, realContext);
    }
  }

  return errorCode;
}


uint8_t PowerComplex(const cplx_t *y, const cplx_t *x, cplx_t *r, realContext_t *realContext) {
  return powerComplexWithResidue(y, x, r, realContext, -1);
}


/********************************************//**
 * \brief Returns a long integer register value modulo 8
 *
 * \param[in] regist source register
 * \return the residue from 0 to 7, or -1 for any other data type
 ***********************************************/
static int32_t registerLongIntegerModulo8(calcRegister_t regist) {
  longInteger_t integer;
  int32_t residue;

  if(getRegisterDataType(regist) != dtLongInteger) {
    return -1;
  }
  convertLongIntegerRegisterToLongInteger(regist, integer);
  residue = longIntegerModuloUInt(integer, 8);
  longIntegerFree(integer);
  return residue;
}

/********************************************//**
 * \brief Y(complex34) ^ X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
static void powCplx(void) {
  cplx_t y, x, r;

  if(!getRegisterAsComplex(REGISTER_Y, &y) || !getRegisterAsComplex(REGISTER_X, &x)) {
    return;
  }

  uint8_t errorCode = powerComplexWithResidue(&y, &x, &r, &ctxtReal75, registerLongIntegerModulo8(REGISTER_X));

  if(errorCode != ERROR_NONE) {
    displayCalcErrorMessage(errorCode, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot raise %s", getRegisterDataTypeName(REGISTER_Y, true, false));
      sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "to %s", getRegisterDataTypeName(REGISTER_X, true, false));
      moreInfoOnError("In function powCplx:", errorMessage, errorMessage + ERROR_MESSAGE_LENGTH/2, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    convertComplexToResultRegister(&r, REGISTER_X);
  }
}


/********************************************//**
 * \brief regX ==> regL and regY ^ regX ==> regX
 * Drops Y, enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter
 * \return void
 ***********************************************/
void fnPower(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexDyadicFunction(&powReal, &powCplx, &powShoI, &powLonI);
}
