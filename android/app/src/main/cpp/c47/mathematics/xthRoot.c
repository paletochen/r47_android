// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file xthRoot.c
 ***********************************************/
// Coded by JM, based on power.c, with reference to cuberoot.c

#include "c47.h"

/********************************************//**
 * \brief (a+ib) ^ (1/(c+id))
 *
 * \param[in] Expecting a,b,c,d:   Y = a +ib;   X = c +id
 * \return REGISTER Y unchanged. REGISTER X with result of (a+ib) ^ (1/(c+id))
 ***********************************************/
static void xthRootComplex(const real_t *aa, const real_t *bb, const real_t *cc, const real_t *dd, realContext_t *realContext) {
  real_t theta, a, b, c, d;

  realCopy(aa, &a);
  realCopy(bb, &b);
  realCopy(cc, &c);
  realCopy(dd, &d);

  if(!getSystemFlag(FLAG_SPCRES)) {
    if(realIsZero(&c)&&realIsZero(&d)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function xthRootComplex: 0th Root is not defined!", NULL, NULL, NULL);
      #endif //  (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
     }
    else {
      if(realIsNaN(&a)||realIsNaN(&b)||realIsNaN(&c)||realIsNaN(&d)) {
        convertComplexToResultRegister(const_NaN, const_NaN, REGISTER_X);
        return;
      }
    }
  }

  if(realIsZero(&a) && realIsZero(&b) && !realIsZero(&c)) {
    convertComplexToResultRegister(const_0, const_0, REGISTER_X);
    return;
  }

  divRealComplex(const_1, &c, &d, &c, &d, realContext);

  //From power.c
  realRectangularToPolar(&a, &b, &a, &theta, realContext);
  WP34S_Ln(&a, &a, realContext);
  realMultiply(&a, &d, &b, realContext);
  realFMA(&theta, &c, &b, &b, realContext);
  realChangeSign(&theta);
  realMultiply(&a, &c, &a, realContext);
  realFMA(&theta, &d, &a, &a, realContext);
  realExp(&a, &c, realContext);
  realPolarToRectangular(const_1, &b, &a, &b, realContext);
  realMultiply(&c, &b, &d, realContext);
  realMultiply(&c, &a, &c, realContext);

  convertComplexToResultRegister(&c, &d, REGISTER_X);
}


/********************************************//**
 * \brief y^(1/x)
 *
 * \param[in] Expecting x,y
 * \return REGISTER Y unchanged. REGISTER X with result of y^x
 ***********************************************/
void xthRootReal(real_t *yy, real_t *xx, realContext_t *realContext) {
  real_t r, o, x, y;
  uint8_t telltale;

  realCopy(xx, &x);
  realCopy(yy, &y);

  telltale = 0;
  if(getSystemFlag(FLAG_SPCRES)) {
    //0
    if(   ((realIsZero(&y)                            && (realCompareGreaterEqual(&x, const_0) || (realIsInfinite(&x) && realIsPositive(&x)))))
       || ((realIsInfinite(&y) && realIsPositive(&y)) && (realCompareLessThan(&x, const_0) && (!realIsInfinite(&x))))
      ) {
      telltale += 1;
      realCopy(const_0, &o);
    }

    //1
    if(((realCompareGreaterEqual(&y, const_0) || (realIsInfinite(&y) && realIsPositive(&y))) && realIsInfinite(&x))) {
      telltale += 2;
      realCopy(const_1, &o);
    }

    //inf
    if(   (realIsZero(&y) && (realCompareLessThan(&x, const_0) && (!realIsInfinite(&x)))) // (y=0.) AND (-inf < x < 0)
       || ((realIsInfinite(&y) && realIsPositive(&y))          && (realCompareGreaterEqual(&x, const_0) && (!realIsInfinite(&x)))) // (y=+inf)  AND (0>= x > inf)
      ) {
      telltale += 4;
      realCopy(const_plusInfinity, &o);
    }

    //NaN
    realDivideRemainder(&x, const_2, &r, realContext);
    if(    (realIsNaN(&x) || realIsNaN(&y))
       || ((realCompareLessThan(&y, const_0) || (realIsInfinite(&y) && realIsNegative(&y))) && (realIsInfinite(&x)   ))                  // (-inf <= y < 0)  AND (x =(inf or -inf))
       || ((realCompareLessThan(&y, const_0) && (!realIsInfinite(&y)                      ) && (!realIsAnInteger(&x))))                  // (-inf < y < 0)  AND (x in non-integer)
       || ((realIsInfinite(&y) && realIsNegative(&y)) && (realIsZero(&r) && realCompareGreaterThan(&x, const_0) && realIsAnInteger(&x))) // (y=-inf) AND (x is even > 0) [zero r means n/2 has no remainder, therefore even]
      ) {
      telltale += 8;
      realCopy(const_NaN, &o);
    }

    //-inf
    realAdd(&x, const_1, &r, realContext);
    realDivideRemainder(&r, const_2, &r, realContext);
    if((realIsInfinite(&y) && realIsNegative(&y)) && (realIsZero(&r) && realCompareGreaterThan(&x, const_0) && realIsAnInteger(&x))) { // (y=-inf) AND (x is odd > 0) [zero r means (n+1)/2 has no remainder, therefore even]
      telltale += 16;
      realCopy(const_minusInfinity, &o);
    }


    if(telltale != 0) {
      convertRealToResultRegister(&o, REGISTER_X, amNone);
      return;
    }
  }
  else { // not DANGER
    if(realIsZero(&x)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function xthRootReal: 0th Root is not defined!", NULL, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
    else {
      if(realIsNaN(&x)||realIsNaN(&y)) {
        convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
        return;
      }
    }
  }

  if(realIsPositive(&y)) {                                         //positive base, no problem, get the power function y^(1/x)
    realDivide(const_1, &x, &x, realContext);

    PowerReal(&y, &x, &x, realContext);
    convertRealToResultRegister(&x, REGISTER_X, amNone);
    return;
  }  //fall through, but returned
  else {
    if(realIsNegative(&y)) {
      realDivideRemainder(&x, const_2, &r, realContext);
      if(realIsZero(&r)) {                                          // negative base and even exp     (zero means no remainder means even)
        if(!getFlag(FLAG_CPXRES)) {
          displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            moreInfoOnError("In function xthRootReal:", "cannot do complex xthRoots when CPXRES is not set", NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          return;
        }
        else {
          xthRootComplex(&y, const_0, &x, const_0, realContext);
        }
      } //fall through after Root CC
      else {
        realAdd(&x, const_1, &r, realContext);
        realDivideRemainder(&r, const_2, &r, realContext);
        if(realIsZero(&r)) {                                        // negative base and odd exp
          realDivide(const_1,&x, &x, realContext);

          realSetPositiveSign(&y);
          PowerReal(&y, &x, &x, realContext);
          realSetNegativeSign(&x);

          convertRealToResultRegister(&x, REGISTER_X, amNone);
          return;
        } //fall though, but returned
        else {      //neither odd nor even, i.e. not integer
          if(!getFlag(FLAG_CPXRES)) {
            displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              moreInfoOnError("In function xthRootReal:", "cannot do complex xthRoots when CPXRES is not set", NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
            return;
          }
          else {
            xthRootComplex(&y, const_0, &x, const_0, realContext);
          }
        }
      } //fall through
    }
    else {
      if(realIsZero(&y)) {
        convertRealToResultRegister(realIsZero(&x) ? const_NaN : const_1, REGISTER_X, amNone);
      } //fall through, but returned
    }
  }
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
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
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

  if(!getRegisterAsReal(REGISTER_X, &x) || !getRegisterAsReal(REGISTER_Y, &y))
    return;

  if((realIsInfinite(&x) || realIsInfinite(&y)) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function doXthRootReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X or Y input of xthRoot when flag D is not set", NULL, NULL);
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

  if(!getRegisterAsComplex(REGISTER_Y, &a, &b)
      || !getRegisterAsComplex(REGISTER_X, &c, &d))
    return;

  if(realIsInfinite(&a) || realIsInfinite(&b)) {
    if(realIsZero(&c) && realIsZero(&d)) {
      convertComplexToResultRegister(const_NaN, const_NaN, REGISTER_X);
    }
    else {
      convertComplexToResultRegister(const_plusInfinity, const_plusInfinity, REGISTER_X);
    }
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
