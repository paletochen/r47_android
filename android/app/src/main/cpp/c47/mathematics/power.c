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
    realDivideRemainder(x, const_2, &lny, realContext);
    bool_t isOdd = !realIsZero(&lny);
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
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
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

  if (!getRegisterAsReal(REGISTER_X, &x) || !getRegisterAsReal(REGISTER_Y, &y))
    return;

  if(realIsInfinite(&y)) {
    if(realIsZero(&x)) {
      realCopy(const_NaN, &res);
    }
    else {
      if(realIsPositive(&x) && realIsAnInteger(&x)) {
        WP34S_Mod(&x, const_2, &res, &ctxtReal39);
        realCopy(realIsZero(&res) ? const_plusInfinity : const_minusInfinity, &res);
      }
      else {
        realCopy(const_plusInfinity, &res);
      }
    }
    goto finish;
  }

  PowerReal(&y, &x, &res, &ctxtReal39);

  if(realIsNaN(&res) && realIsNegative(&y) && !realIsAnInteger(&x)) {
    if(getFlag(FLAG_CPXRES)) {
      powCplx();
      return;
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
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
 * Calculate y^x for complex numbers.
 */
uint8_t PowerComplex(const real_t *yReal, const real_t *yImag, const real_t *xReal, const real_t *xImag, real_t *rReal, real_t *rImag, realContext_t *realContext) {
  uint8_t errorCode = ERROR_NONE;

  if(realIsInfinite(yReal) || realIsInfinite(yImag)) {
      if(realIsZero(xReal) && realIsZero(xImag)) {
          realCopy(const_NaN, rReal);
          realCopy(const_NaN, rImag);
      }
      else {
          realCopy(const_plusInfinity, rReal);
          realCopy(const_plusInfinity, rImag);
      }
  }
  else if(realIsZero(yReal) && realIsZero(yImag)) {
      if(realIsZero(xReal)) {
          realCopy(const_NaN, rReal);
          realCopy(const_NaN, rImag);
      }
      else {
          realCopy(const_0, rReal);
          realCopy(const_0, rImag);
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
      if (realCompareAbsEqual(yReal, yImag)) {
        realDivideRemainder(xReal, const_8, &xR, realContext);        // {See [5], if yR=yI then we have theta = pi/4 exact, then we can do a Xr remainder by 8}
        if (realIsZero(xImag) && realIsAnInteger(&xR)) {              // only compute md when exponent is integer
          md = realToInt32C47(&xR);
          if (md % 4 == 0) {
            doZeroingImag = true;
          } else if ((md-2) % 4 == 0) {
            doZeroingReal = true;
          }
        }
      } else if (realIsZero(yReal)) {
        realDivideRemainder(xReal, const_4, &xR, realContext);        // {See [5], if yR=0 then we have theta = pi/2 exact, then we can do a Xr remainder by 4}
        if (realIsZero(xImag) && realIsAnInteger(&xR)) {              // only compute md when exponent is integer
          md = realToInt32C47(&xR);
          if (md % 2 == 0) {
            doZeroingImag = true;
          } else {
            doZeroingReal = true;
          }
        }
      } else {
        realCopy(xReal, &xR);
      }

      realFMA(&theta, &xR, rImag, rImag, realContext);                //rImag = Xi.LN r  +  theta . Xr  ===> this theta.Xt is the coefficient of r.e^i.COEF, hence the angle and therefore we can get the remainder after dividing by number of revolutions.
      realChangeSign(&theta);

      realMultiply(rReal, xReal, rReal, realContext);                 //rReal = Xr.LN r
      realFMA(&theta, xImag, rReal, rReal, realContext);              //rReal = Xr.LN r  *  -theta . Xi

      realExp(rReal, &tmp, realContext);
      realPolarToRectangular(const_1, rImag, rReal, rImag, realContext);
      if (doZeroingImag) {
        realCopy(const_0, rImag);
      } else {
        realMultiply(&tmp, rImag, rImag, realContext);
      }
      if (doZeroingReal) {
        realCopy(const_0, rReal);
      } else {
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

  if (!getRegisterAsComplex(REGISTER_Y, &yReal, &yImag) || !getRegisterAsComplex(REGISTER_X, &xReal, &xImag))
    return;

  uint8_t errorCode = PowerComplex(&yReal, &yImag, &xReal, &xImag, &rReal, &rImag, &ctxtReal39);

  if(errorCode != ERROR_NONE) {
    displayCalcErrorMessage(errorCode, ERR_REGISTER_LINE, REGISTER_X);
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
