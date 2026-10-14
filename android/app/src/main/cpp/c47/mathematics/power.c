// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file power.c
 ***********************************************/

#include "c47.h"

static void powReal(void);
static void powCplx(void);


void PowerReal(const real_t *y, const real_t *x, real_t *res, realContext_t *realContext) {
  real_t lny;

  if(realIsNegative(y) && realIsAnInteger(x)){
    bool_t isOdd = false;

    if(!realIsInfinite(x)) {                             // realIsAnInteger() is true for an infinity, which is neither odd nor even
      realDivideRemainder(x, const_2, &lny, realContext);
      isOdd = !realIsZero(&lny);
    }
    realCopyAbs(y, &lny);
    WP34S_Ln(&lny, &lny, realContext);
    realMultiply(x, &lny, res, realContext);
    realExp(res, res, realContext);
    #if defined(PC_BUILD)
      fflush(stdout);
    #endif //PC_BUILD
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

  if(!getRegisterAsReal(REGISTER_X, &x) || !getRegisterAsReal(REGISTER_Y, &y)) {
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
        if(realIsAnInteger(&x)) {
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

  PowerReal(&y, &x, &res, &ctxtReal39);

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
static bool_t setInfiniteComplexPower(const real_t *yReal, const real_t *yImag, const real_t *xReal, real_t *rReal, real_t *rImag, realContext_t *realContext) {
  real_t re, im, sector;

  if(realIsInfinite(xReal)) {                            // phi.x is then unbounded, and WP34S_Mod() of it is a NaN
    return false;
  }

  realCopy(yReal, &re);                                  // getInfiniteComplexAngle() takes non-const operands
  realCopy(yImag, &im);
  int32ToReal(getInfiniteComplexAngle(&re, &im), &sector);
  realMultiply(&sector, xReal, &sector, realContext);
  WP34S_Mod(&sector, const_8, &sector, realContext);     // remainder is truncated, so lift it into [0, 8) = one revolution
  if(realIsNegative(&sector)) {
    realAdd(&sector, const_8, &sector, realContext);
  }

  if(!realIsAnInteger(&sector)) {
    return false;
  }
  setInfiniteComplexAngle(realToInt32C47(&sector, NULL), rReal, rImag);
  return true;
}


/*
 * Calculate y^x for complex numbers.
 */
uint8_t PowerComplex(const real_t *yReal, const real_t *yImag, const real_t *xReal, const real_t *xImag, real_t *rReal, real_t *rImag, realContext_t *realContext) {
  uint8_t errorCode = ERROR_NONE;

  if(realIsInfinite(yReal) || realIsInfinite(yImag)) {   // the base has an infinite component
    if(realIsNaN(xReal) || realIsNaN(xImag)) {           // inf ^ NaN is NaN
      realSetNaN(rReal);
      realSetNaN(rImag);
    }
    else if(realIsZero(xReal) && realIsZero(xImag)) {    // inf ^ 0 is indeterminate
      realSetNaN(rReal);
      realSetNaN(rImag);
    }
    else if(realIsZero(xReal)) {                         // Re(exponent) == 0 (pure imaginary) is indeterminate
      realSetNaN(rReal);
      realSetNaN(rImag);
    }
    else if(realIsNegative(xReal)) {                     // Re(exponent) < 0 ==> modulus collapses to 0, where the angle no longer counts
      realSetZero(rReal);
      realSetZero(rImag);
    }
    else {                                               // Re(exponent) > 0 ==> infinite modulus
      bool_t onSector = false;

      if(realIsZero(xImag)) {                            // Im(exponent) != 0 adds Im(x).ln r to the argument, leaving it unbounded
        onSector = setInfiniteComplexPower(yReal, yImag, xReal, rReal, rImag, realContext);
      }
      if(!onSector) {
        realSetPlusInfinity(rReal);
        realSetPlusInfinity(rImag);
      }
    }
  }
  else if(realIsZero(yReal) && realIsZero(yImag)) {
    if(realIsZero(xReal)) {
      realSetNaN(rReal);
      realSetNaN(rImag);
    }
    else {
      realSetZero(rReal);
      realSetZero(rImag);
    }
  }
  else {
    real_t theta;
    real_t tmp;

    // (Yr+iYi) ^ (Xr+iXi)
    // EXP [  (Xr+iXi) LN (Yr+iYi)  ]
    // EXP [  (Xr+iXi) LN (r angle theta)  ]
    // EXP [  Xr.LN (r angle theta)   +   i.(Xi.LN (r angle theta)) ]
    // EXP [ (Xr.LN r) * (-theta Xi)  +   i.(Xi.LN r + (theta . Xr)) ]  [5]


    realRectangularToPolar(yReal, yImag, rReal, &theta, realContext);
    WP34S_Ln(rReal, rReal, realContext);

    realMultiply(rReal, xImag, rImag, realContext);                 //rImag = Xi.LN r

    real_t xR;
    int8_t md;
    bool_t doZeroingReal = false;
    bool_t doZeroingImag = false;
    if(realCompareAbsEqual(yReal, yImag)) {
      realDivideRemainder(xReal, const_8, &xR, realContext);        // {See [5], if yR=yI then we have theta = pi/4 exact, then we can do a Xr remainder by 8}
      if(realIsZero(xImag) && realIsAnInteger(&xR)) {              // only compute md when exponent is integer
        md = realToInt32C47(&xR, NULL);
        if(md % 4 == 0) {
          doZeroingImag = true;
        }
        else if((md-2) % 4 == 0) {
          doZeroingReal = true;
        }
      }
    }
    else if(realIsZero(yReal)) {
      realDivideRemainder(xReal, const_4, &xR, realContext);        // {See [5], if yR=0 then we have theta = pi/2 exact, then we can do a Xr remainder by 4}
      if(realIsZero(xImag) && realIsAnInteger(&xR)) {              // only compute md when exponent is integer
        md = realToInt32C47(&xR, NULL);
        if(md % 2 == 0) {
          doZeroingImag = true;
        }
        else {
          doZeroingReal = true;
        }
      }
    }
    else {
      realCopy(xReal, &xR);
    }

    realFMA(&theta, &xR, rImag, rImag, realContext);                //rImag = Xi.LN r  +  theta . Xr  ===> this theta.Xt is the coefficient of r.e^i.COEF, hence the angle and therefore we can get the remainder after dividing by number of revolutions.
    realChangeSign(&theta);

    realMultiply(rReal, xReal, rReal, realContext);                 //rReal = Xr.LN r
    realFMA(&theta, xImag, rReal, rReal, realContext);              //rReal = Xr.LN r  *  -theta . Xi

    realExp(rReal, &tmp, realContext);
    realPolarToRectangular(const_1, rImag, rReal, rImag, realContext);
    if(doZeroingImag) {
      realSetZero(rImag);
    }
    else {
      realMultiply(&tmp, rImag, rImag, realContext);
    }
    if(doZeroingReal) {
      realSetZero(rReal);
    }
    else {
      realMultiply(&tmp, rReal, rReal, realContext);
    }
  }

  return errorCode;
}

/********************************************//**
 * \brief Y(complex34) ^ X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
static void powCplx(void) {
  real_t yReal, yImag, xReal, xImag, rReal, rImag;

  if(!getRegisterAsComplex(REGISTER_Y, &yReal, &yImag) || !getRegisterAsComplex(REGISTER_X, &xReal, &xImag)) {
    return;
  }

  uint8_t errorCode = PowerComplex(&yReal, &yImag, &xReal, &xImag, &rReal, &rImag, &ctxtReal39);

  if(errorCode != ERROR_NONE) {
    displayCalcErrorMessage(errorCode, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot raise %s", getRegisterDataTypeName(REGISTER_Y, true, false));
      sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "to %s", getRegisterDataTypeName(REGISTER_X, true, false));
      moreInfoOnError("In function powCplx:", errorMessage, errorMessage + ERROR_MESSAGE_LENGTH/2, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    convertComplexToResultRegister(&rReal, &rImag, REGISTER_X);
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
