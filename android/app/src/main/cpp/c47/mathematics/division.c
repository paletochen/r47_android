// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file division.c
 ***********************************************/

#include "c47.h"

TO_QSPI void (* const division[NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS][NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS])(void) = {
// regX |    regY ==>   1            2            3            4            5         6         7            8            9             10
//      V               Long integer Real34       Complex34    Time         Date      String    Real34 mat   Complex34 m  Short integer Config data
/*  1 Long integer  */ {divLonILonI, divRealLonI, divCplxLonI, divTimeLonI, divError, divError, divRemaLonI, divCxmaLonI, divShoILonI,  divError},
/*  2 Real34        */ {divLonIReal, divRealReal, divCplxReal, divTimeReal, divError, divError, divRemaReal, divCxmaReal, divShoIReal,  divError},
/*  3 Complex34     */ {divLonICplx, divRealCplx, divCplxCplx, divError,    divError, divError, divRemaCplx, divCxmaCplx, divShoICplx,  divError},
/*  4 Time          */ {divLonITime, divRealTime, divError,    divTimeTime, divError, divError, divError,    divError,    divShoITime,  divError},
/*  5 Date          */ {divError,    divError,    divError,    divError,    divError, divError, divError,    divError,    divError,     divError},
/*  6 String        */ {divError,    divError,    divError,    divError,    divError, divError, divError,    divError,    divError,     divError},
/*  7 Real34 mat    */ {divLonIRema, divRealRema, divCplxRema, divError,    divError, divError, divRemaRema, divCxmaRema, divShoIRema,  divError},
/*  8 Complex34 mat */ {divLonICxma, divRealCxma, divCplxCxma, divError,    divError, divError, divRemaCxma, divCxmaCxma, divShoICxma,  divError},
/*  9 Short integer */ {divLonIShoI, divRealShoI, divCplxShoI, divTimeShoI, divError, divError, divRemaShoI, divCxmaShoI, divShoIShoI,  divError},
/* 10 Config data   */ {divError,    divError,    divError,    divError,    divError, divError, divError,    divError,    divError,     divError}
};



/********************************************//**
 * \brief Data type error in division
 *
 * \param[in] unusedButMandatoryParameter
 * \return void
 ***********************************************/
#if (EXTRA_INFO_ON_CALC_ERROR == 1)
  void divError(void) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
    sprintf(errorMessage, "cannot divide %s", getRegisterDataTypeName(REGISTER_Y, true, false));
    sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "by %s", getRegisterDataTypeName(REGISTER_X, true, false));
    moreInfoOnError("In function fnDivide:", errorMessage, errorMessage + ERROR_MESSAGE_LENGTH/2, NULL);
  }
#endif // (EXTRA_INFO_ON_CALC_ERROR == 1)



/********************************************//**
 * \brief regX ==> regL and regY ÷ regX ==> regX
 * Drops Y, enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter
 * \return void
 ***********************************************/
void fnDivide(uint16_t unusedButMandatoryParameter) {
  copySourceRegisterToDestRegister(REGISTER_X, REGISTER_L);

  division[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)]();

  adjustResult(REGISTER_X, true, true, REGISTER_X, REGISTER_Y, -1);
}



void divComplexComplex75(const real_t *numerReal, const real_t *numerImag, const real_t *denomReal, const real_t *denomImag, real_t *quotientReal, real_t *quotientImag, realContext_t *realContext) {
  real_t realNumer, realDenom, a, b, c, d;

  realCopy(numerReal, &a);
  realCopy(numerImag, &b);
  realCopy(denomReal, &c);
  realCopy(denomImag, &d);

  if(realIsNaN(&a) || realIsNaN(&b) || realIsNaN(&c) || realIsNaN(&d)) {
    realCopy(const_NaN, quotientReal);
    realCopy(const_NaN, quotientImag);
    return;
  }

  if(realIsInfinite(&c) || realIsInfinite(&d)) {
    if(realIsInfinite(&a) || realIsInfinite(&b)) {
      realCopy(const_NaN, quotientReal);
      realCopy(const_NaN, quotientImag);
    }
    else {
      realZero(quotientReal);
      realZero(quotientImag);
    }
    return;
  }

  if(realIsInfinite(&a) && !realIsInfinite(&b)) {
    realZero(&b);
  }

  if(realIsInfinite(&b) && !realIsInfinite(&a)) {
    realZero(&a);
  }


  // Denominator
  realMultiply(&c, &c, &realDenom, realContext);                 // realDenom = c²
  realFMA(&d, &d, &realDenom, &realDenom, realContext);          // realDenom = c² + d²

  // real part
  realMultiply(&a, &c, &realNumer, realContext);                 // realNumer = a*c
  realFMA(&b, &d, &realNumer, &realNumer, realContext);          // realNumer = a*c + b*d
  realDivide(&realNumer, &realDenom, quotientReal, realContext); // realPart = (a*c + b*d) / (c² + d²) = realNumer / realDenom

  // imaginary part
  realMultiply(&b, &c, &realNumer, realContext);                 // realNumer = b*c
  realChangeSign(&a);                                            // a = -a
  realFMA(&a, &d, &realNumer, &realNumer, realContext);          // realNumer = b*c - a*d
  realDivide(&realNumer, &realDenom, quotientImag, realContext); // imagPart = (b*c - a*d) / (c² + d²) = realNumer / realDenom

}

#if defined(OPTION_CUBIC_159) || defined(OPTION_SQUARE_159) || defined(OPTION_EIGEN_159)
void divComplexComplex159(const real_t *numerReal, const real_t *numerImag, const real_t *denomReal, const real_t *denomImag, real_t *quotientReal, real_t *quotientImag, realContext_t *realContext) {
  real159_t realNumer, realDenom, a, b, c, d;

  realZero((real_t *)&realNumer); realZero((real_t *)&realDenom);
  realZero((real_t *)&a); realZero((real_t *)&b);
  realZero((real_t *)&c); realZero((real_t *)&d);

  realCopy(numerReal, (real_t *)&a);
  realCopy(numerImag, (real_t *)&b);
  realCopy(denomReal, (real_t *)&c);
  realCopy(denomImag, (real_t *)&d);

  if(realIsNaN((real_t *)&a) || realIsNaN((real_t *)&b) || realIsNaN((real_t *)&c) || realIsNaN((real_t *)&d)) {
    realCopy(const_NaN, quotientReal);
    realCopy(const_NaN, quotientImag);
    return;
  }

  if(realIsInfinite((real_t *)&c) || realIsInfinite((real_t *)&d)) {
    if(realIsInfinite((real_t *)&a) || realIsInfinite((real_t *)&b)) {
      realCopy(const_NaN, quotientReal);
      realCopy(const_NaN, quotientImag);
    }
    else {
      realZero(quotientReal);
      realZero(quotientImag);
    }
    return;
  }

  if(realIsInfinite((real_t *)&a) && !realIsInfinite((real_t *)&b)) {
    realZero((real_t *)&b);
  }
  if(realIsInfinite((real_t *)&b) && !realIsInfinite((real_t *)&a)) {
    realZero((real_t *)&a);
  }

  // Denominator: c² + d²
  realMultiply((real_t *)&c, (real_t *)&c, (real_t *)&realDenom, realContext);
  realFMA((real_t *)&d, (real_t *)&d, (real_t *)&realDenom, (real_t *)&realDenom, realContext);

  // Real part: (a*c + b*d) / (c² + d²)
  realMultiply((real_t *)&a, (real_t *)&c, (real_t *)&realNumer, realContext);
  realFMA((real_t *)&b, (real_t *)&d, (real_t *)&realNumer, (real_t *)&realNumer, realContext);
  realDivide((real_t *)&realNumer, (real_t *)&realDenom, quotientReal, realContext);

  // Imaginary part: (b*c - a*d) / (c² + d²)
  realMultiply((real_t *)&b, (real_t *)&c, (real_t *)&realNumer, realContext);
  realChangeSign((real_t *)&a);
  realFMA((real_t *)&a, (real_t *)&d, (real_t *)&realNumer, (real_t *)&realNumer, realContext);
  realDivide((real_t *)&realNumer, (real_t *)&realDenom, quotientImag, realContext);
}
#endif //OPTION_CUBIC_159 || OPTION_SQUARE_159 || OPTION_EIGEN_159


void divComplexComplex(const real_t *numerReal, const real_t *numerImag, const real_t *denomReal, const real_t *denomImag, real_t *quotientReal, real_t *quotientImag, realContext_t *realContext) {
  if(realContext->digits <= 75) {
    divComplexComplex75(numerReal, numerImag, denomReal, denomImag, quotientReal, quotientImag, realContext);
#if defined(OPTION_CUBIC_159) || defined(OPTION_SQUARE_159) || defined(OPTION_EIGEN_159)
  } else
  if(realContext->digits <= 159) {
    divComplexComplex159(numerReal, numerImag, denomReal, denomImag, quotientReal, quotientImag, realContext);
#endif //OPTION_CUBIC_159 || OPTION_SQUARE_159 || OPTION_EIGEN_159
  } else {
    sprintf(errorMessage, "Exceed digits :divComplexComplex: %d", (int)realContext->digits);
    displayBugScreen(errorMessage);
  }
}




void divRealComplex(const real_t *numerReal, const real_t *denomReal, const real_t *denomImag, real_t *quotientReal, real_t *quotientImag, realContext_t *realContext) {
  real_t a, c, d, denom;

  realCopy(numerReal, &a);
  realCopy(denomReal, &c);
  realCopy(denomImag, &d);

  if(realIsNaN(&a) || realIsNaN(&c) || realIsNaN(&d)) {
    realCopy(const_NaN, quotientReal);
    realCopy(const_NaN, quotientImag);
    return;
  }

  if(realIsInfinite(&c) || realIsInfinite(&d)) {
    realZero(quotientReal);
    realZero(quotientImag);
    return;
  }

  realMultiply(&c, &c, &denom, realContext);    // c²
  realFMA(&d, &d, &denom, &denom, realContext); // c² + d²

  // real part
  realMultiply(&a, &c, quotientReal, realContext);             // numer = a*c
  realDivide(quotientReal, &denom, quotientReal, realContext); // realPart  = (a*c) / (c² + d²) = numer / denom

  // imaginary part
  realChangeSign(&a);                                          // a = -a
  realMultiply(&a, &d, quotientImag, realContext);             // numer = -a*d
  realDivide(quotientImag, &denom, quotientImag, realContext); // imagPart  = -(a*d) / (c² + d²) = numer / denom
}

void divComplexReal(const real_t *numerReal, const real_t *numerImag, const real_t *denom, real_t *quotientReal, real_t *quotientImag, realContext_t *realContext) {
  realDivide(numerReal, denom, quotientReal, realContext);
  realDivide(numerImag, denom, quotientImag, realContext);
}



/******************************************************************************************************************************************************************************************/
/* long integer / ...                                                                                                                                                                     */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(long integer) ÷ X(long integer) ==> X(long integer or real34)
 *
 * \param void
 * \return void
 ***********************************************/
void divLonILonI(void) {
  longInteger_t x,y;

  convertLongIntegerRegisterToLongInteger(REGISTER_X, x);
  convertLongIntegerRegisterToLongInteger(REGISTER_Y, y);

  if(longIntegerIsZero(x)) {
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    convertLongIntegerToReal34(x,REGISTER_REAL34_DATA(REGISTER_X));
    reallocateRegister(REGISTER_Y, dtReal34, 0, amNone);
    convertLongIntegerToReal34(y,REGISTER_REAL34_DATA(REGISTER_Y));
    divRealReal();
  }
  else {
    longInteger_t quotient, remainder;

    longIntegerInit(quotient);
    longIntegerInit(remainder);
    longIntegerDivideQuotientRemainder(y, x, quotient, remainder);

    if(longIntegerIsZero(remainder)) {
      convertLongIntegerToLongIntegerRegister(quotient, REGISTER_X);
    }
    else {
      real_t xIc, yIc;

      convertLongIntegerRegisterToReal(REGISTER_Y, &yIc, &ctxtReal39);
      convertLongIntegerRegisterToReal(REGISTER_X, &xIc, &ctxtReal39);
      reallocateRegister(REGISTER_X, dtReal34, 0, amNone);

      realDivide(&yIc, &xIc, &xIc, &ctxtReal39);
      convertRealToReal34ResultRegister(&xIc, REGISTER_X);
    }

    longIntegerFree(quotient);
    longIntegerFree(remainder);
  }

  longIntegerFree(y);
  longIntegerFree(x);
}



/********************************************//**
 * \brief Y(long integer) ÷ X(short integer) ==> X(long integer)
 *
 * \param void
 * \return void
 ***********************************************/
void divLonIShoI(void) {
  longInteger_t a, c;

  convertLongIntegerRegisterToLongInteger(REGISTER_Y, a);
  convertShortIntegerRegisterToLongIntegerRegister(REGISTER_X, REGISTER_X);
  convertLongIntegerRegisterToLongInteger(REGISTER_X, c);

  if(longIntegerIsZero(c)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function divLonIShoI:", "cannot divide a long integer by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    longIntegerDivideQuotientRemainder(a, c, a, c);
    convertLongIntegerToLongIntegerRegister(a, REGISTER_X);
  }

  longIntegerFree(a);
  longIntegerFree(c);
}



/********************************************//**
 * \brief Y(short integer) ÷ X(long integer) ==> X(long integer)
 *
 * \param void
 * \return void
 ***********************************************/
void divShoILonI(void) {
  longInteger_t a, c;

  convertShortIntegerRegisterToLongIntegerRegister(REGISTER_Y, REGISTER_Y);
  convertLongIntegerRegisterToLongInteger(REGISTER_Y, a);
  convertLongIntegerRegisterToLongInteger(REGISTER_X, c);

  if(longIntegerIsZero(c)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function divShoILonI:", "cannot divide a short integer by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    longIntegerDivideQuotientRemainder(a, c, a, c);
    convertLongIntegerToLongIntegerRegister(a, REGISTER_X);
  }

  longIntegerFree(a);
  longIntegerFree(c);
}



/********************************************//**
 * \brief Y(long integer) ÷ X(real34) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void divLonIReal(void) {
  real_t y, x;

  setRegisterAngularMode(REGISTER_X, amNone);
  convertLongIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);

  if(real34IsZero(REGISTER_REAL34_DATA(REGISTER_X))) {
    if(realIsZero(&y)) {
      if(getSystemFlag(FLAG_SPCRES)) {
        convertRealToReal34ResultRegister(const_NaN, REGISTER_X);
      }
      else {
        displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function divLonIReal:", "cannot divide 0 by 0", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
    }
    else {
      if(getSystemFlag(FLAG_SPCRES)) {
        realToReal34((realIsPositive(&y) ? const_plusInfinity : const_minusInfinity), REGISTER_REAL34_DATA(REGISTER_X));
      }
      else {
        displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function divLonIReal:", "cannot divide a long integer by 0", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
    }
  }

  else {
    real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x);
    realDivide(&y, &x, &x, &ctxtReal39);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
  }
}



/********************************************//**
 * \brief Y(real34) ÷ X(long integer) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void divRealLonI(void) {
  real_t y, x;
  angularMode_t yAngularMode;

  convertLongIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);

  if(realIsZero(&x)) {
    if(real34IsZero(REGISTER_REAL34_DATA(REGISTER_Y))) {
      if(getSystemFlag(FLAG_SPCRES)) {
        convertRealToReal34ResultRegister(const_NaN, REGISTER_X);
      }
      else {
        displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function divRealLonI:", "cannot divide 0 by 0", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
    }
    else {
      if(getSystemFlag(FLAG_SPCRES)) {
        realToReal34((real34IsPositive(REGISTER_REAL34_DATA(REGISTER_Y)) ? const_plusInfinity : const_minusInfinity), REGISTER_REAL34_DATA(REGISTER_X));
      }
      else {
        displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function divRealLonI:", "cannot divide a real34 by 0", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
    }
  }

  else {
    real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y);
    yAngularMode = getRegisterAngularMode(REGISTER_Y);

    if(yAngularMode == amNone) {
      realDivide(&y, &x, &x, &ctxtReal39);
      convertRealToReal34ResultRegister(&x, REGISTER_X);
    }
    else {
      convertAngleFromTo(&y, yAngularMode, currentAngularMode, &ctxtReal39);
      realDivide(&y, &x, &x, &ctxtReal39);
      convertRealToReal34ResultRegister(&x, REGISTER_X);
      setRegisterAngularMode(REGISTER_X, currentAngularMode);
    }
  }
}



/********************************************//**
 * \brief Y(long integer) ÷ X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void divLonICplx(void) {
  real_t y, xReal, xImag;

  convertLongIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &xReal);
  real34ToReal(REGISTER_IMAG34_DATA(REGISTER_X), &xImag);

  divRealComplex(&y, &xReal, &xImag, &xReal, &xImag, &ctxtReal39);

  convertComplexToResultRegister(&xReal, &xImag, REGISTER_X);
}



/********************************************//**
 * \brief Y(complex34) ÷ X(long integer) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void divCplxLonI(void) {
  real_t a, b, c;

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &a);
  real34ToReal(REGISTER_IMAG34_DATA(REGISTER_Y), &b);
  convertLongIntegerRegisterToReal(REGISTER_X, &c, &ctxtReal39);

  realDivide(&a, &c, &a, &ctxtReal39);
  realDivide(&b, &c, &b, &ctxtReal39);

  reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
  convertComplexToResultRegister(&a, &b, REGISTER_X);
}



/******************************************************************************************************************************************************************************************/
/* time / ...                                                                                                                                                                             */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(time) ÷ X(long integer) ==> X(time)
 *
 * \param void
 * \return void
 ***********************************************/
void divTimeLonI(void) {
  real_t y, x;

  convertLongIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  reallocateRegister(REGISTER_X, dtTime, 0, amNone);

  if(realIsZero(&x)) {
    if(real34IsZero(REGISTER_REAL34_DATA(REGISTER_Y))) {
      if(getSystemFlag(FLAG_SPCRES)) {
        realToReal34(const_NaN, REGISTER_REAL34_DATA(REGISTER_X));
      }
      else {
        displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function divTimeLonI:", "cannot divide 0 by 0", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
    }
    else {
      if(getSystemFlag(FLAG_SPCRES)) {
        realToReal34((real34IsPositive(REGISTER_REAL34_DATA(REGISTER_Y)) ? const_plusInfinity : const_minusInfinity), REGISTER_REAL34_DATA(REGISTER_X));
      }
      else {
        displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function divTimeLonI:", "cannot divide time by 0", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
    }
  }

  else {
    real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y);

    realDivide(&y, &x, &x, &ctxtReal39);
    realToReal34(&x, REGISTER_REAL34_DATA(REGISTER_X));
  }
}



/********************************************//**
 * \brief Y(long integer) ÷ X(time) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void divLonITime(void) {
  convertTimeRegisterToReal34Register(REGISTER_X, REGISTER_X);
  divLonIReal();
}



/********************************************//**
 * \brief Y(time) ÷ X(short integer) ==> X(time)
 *
 * \param void
 * \return void
 ***********************************************/
void divTimeShoI(void) {
  real_t y, x;

  convertShortIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  reallocateRegister(REGISTER_X, dtTime, 0, amNone);

  if(realIsZero(&x)) {
    if(real34IsZero(REGISTER_REAL34_DATA(REGISTER_Y))) {
      if(getSystemFlag(FLAG_SPCRES)) {
        convertRealToReal34ResultRegister(const_NaN, REGISTER_X);
      }
      else {
        displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function divTimeShoI:", "cannot divide 0 by 0", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
    }
    else {
      if(getSystemFlag(FLAG_SPCRES)) {
        realToReal34((real34IsPositive(REGISTER_REAL34_DATA(REGISTER_Y)) ? const_plusInfinity : const_minusInfinity), REGISTER_REAL34_DATA(REGISTER_X));
      }
      else {
        displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function divTimeShoI:", "cannot divide time by 0", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
    }
  }

  else {
    real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y);

    realDivide(&y, &x, &x, &ctxtReal39);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
  }
}



/********************************************//**
 * \brief Y(short integer) ÷ X(time) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void divShoITime(void) {
  convertTimeRegisterToReal34Register(REGISTER_X, REGISTER_X);
  divShoIReal();
}



/********************************************//**
 * \brief Y(time) ÷ X(real34) ==> X(time)
 *
 * \param void
 * \return void
 ***********************************************/
void divTimeReal(void) {
  if(real34IsZero(REGISTER_REAL34_DATA(REGISTER_Y)) && real34IsZero(REGISTER_REAL34_DATA(REGISTER_X))) {
    if(getSystemFlag(FLAG_SPCRES)) {
      reallocateRegister(REGISTER_X, dtTime, 0, amNone);
      convertRealToReal34ResultRegister(const_NaN, REGISTER_X);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function divTimeReal:", "cannot divide 0 by 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }

  else if(real34IsZero(REGISTER_REAL34_DATA(REGISTER_X))) {
    if(getSystemFlag(FLAG_SPCRES)) {
      reallocateRegister(REGISTER_X, dtTime, 0, amNone);
      realToReal34((real34IsPositive(REGISTER_REAL34_DATA(REGISTER_Y)) ? const_plusInfinity : const_minusInfinity), REGISTER_REAL34_DATA(REGISTER_X));
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function divTimeReal:", "cannot divide a real34 by 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }

  else {
    real34_t x;
    angularMode_t xAngularMode;

    real34Copy(REGISTER_REAL34_DATA(REGISTER_X), &x);
    xAngularMode = getRegisterAngularMode(REGISTER_X);

    if(xAngularMode == amNone) { // time / real
      reallocateRegister(REGISTER_X, dtTime, 0, amNone);
      real34Divide(REGISTER_REAL34_DATA(REGISTER_Y), &x, REGISTER_REAL34_DATA(REGISTER_X));
    }
    else { // time / angle
      divError();
    }
  }
}



/********************************************//**
 * \brief Y(real34) ÷ X(time) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void divRealTime(void) {
  convertTimeRegisterToReal34Register(REGISTER_X, REGISTER_X);
  divRealReal();
}



/********************************************//**
 * \brief Y(time) ÷ X(time) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void divTimeTime(void) {
  if(real34IsZero(REGISTER_REAL34_DATA(REGISTER_Y)) && real34IsZero(REGISTER_REAL34_DATA(REGISTER_X))) {
    if(getSystemFlag(FLAG_SPCRES)) {
      convertRealToReal34ResultRegister(const_NaN, REGISTER_X);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function divTimeTime:", "cannot divide 0 by 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }

  else if(real34IsZero(REGISTER_REAL34_DATA(REGISTER_X))) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realToReal34((real34IsPositive(REGISTER_REAL34_DATA(REGISTER_Y)) ? const_plusInfinity : const_minusInfinity), REGISTER_REAL34_DATA(REGISTER_X));
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function divTimeTime:", "cannot divide time by 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }

  else {
    real34_t b;

    real34Copy(REGISTER_REAL34_DATA(REGISTER_X), &b);
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    real34Divide(REGISTER_REAL34_DATA(REGISTER_Y), &b, REGISTER_REAL34_DATA(REGISTER_X));
    setRegisterAngularMode(REGISTER_X, amNone);
  }
}



/******************************************************************************************************************************************************************************************/
/* date / ...                                                                                                                                                                             */
/******************************************************************************************************************************************************************************************/

/******************************************************************************************************************************************************************************************/
/* string / ...                                                                                                                                                                           */
/******************************************************************************************************************************************************************************************/

/******************************************************************************************************************************************************************************************/
/* real34 matrix / ...                                                                                                                                                                    */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(real34 matrix) ÷ X(long integer) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void divRemaLonI(void) {
  real34Matrix_t matrix, res;
  real_t x;

  convertLongIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  if(!getSystemFlag(FLAG_SPCRES) && realIsZero(&x)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function divRemaLonI:", "cannot divide by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  else {
    linkToRealMatrixRegister(REGISTER_Y, &matrix);
    _divideRealMatrix(&matrix, &x, &res, &ctxtReal39);
    convertReal34MatrixToReal34MatrixRegister(&res, REGISTER_X);
    realMatrixFree(&res);
  }
}



/********************************************//**
 * \brief Y(long integer) ÷ X(real34 matrix) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void divLonIRema(void) {
  real34Matrix_t matrix, res;
  real_t y;
  bool_t divZeroOccurs = false;

  convertLongIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  linkToRealMatrixRegister(REGISTER_X, &matrix);
  if(!getSystemFlag(FLAG_SPCRES)) {
    const uint16_t rows = matrix.header.matrixRows;
    const uint16_t cols = matrix.header.matrixColumns;
    int32_t i;

    for(i = 0; i < cols * rows; ++i) {
      if(real34IsZero(&matrix.matrixElements[i])) {
        divZeroOccurs = true;
      }
    }
  }

  if(divZeroOccurs) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function divLonIRema:", "cannot divide by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  else {
    _divideByRealMatrix(&y, &matrix, &res, &ctxtReal39);
    convertReal34MatrixToReal34MatrixRegister(&res, REGISTER_X);
    realMatrixFree(&res);
  }
}



/********************************************//**
 * \brief Y(real34 matrix) ÷ X(real34 matrix) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void divRemaRema(void) {
  real34Matrix_t y, x, res;

  linkToRealMatrixRegister(REGISTER_Y, &y);
  linkToRealMatrixRegister(REGISTER_X, &x);

  if(y.header.matrixColumns != x.header.matrixRows || y.header.matrixColumns != x.header.matrixColumns || x.header.matrixRows != x.header.matrixColumns) {
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot divide %d" STD_CROSS "%d-matrix and %d" STD_CROSS "%d-matrix",
              y.header.matrixRows, y.header.matrixColumns,
              x.header.matrixRows, x.header.matrixColumns);
      moreInfoOnError("In function divRemaRema:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    divideRealMatrices(&y, &x, &res);
    if(res.matrixElements) {
      convertReal34MatrixToReal34MatrixRegister(&res, REGISTER_X);
      realMatrixFree(&res);
    }
    else {
      displayCalcErrorMessage(ERROR_SINGULAR_MATRIX, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "cannot divide by a singular matrix");
        moreInfoOnError("In function divRemaRema:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
}



/********************************************//**
 * \brief Y(real34 matrix) ÷ X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void divRemaCxma(void) {
  convertReal34MatrixRegisterToComplex34MatrixRegister(REGISTER_Y, REGISTER_Y);
  divCxmaCxma();
}



/********************************************//**
 * \brief Y(real34 matrix) ÷ X(short integer) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void divRemaShoI(void) {
  real34Matrix_t matrix, res;
  real_t x;

  convertShortIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  if(!getSystemFlag(FLAG_SPCRES) && realIsZero(&x)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function divRemaShoI:", "cannot divide by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  else {
    linkToRealMatrixRegister(REGISTER_Y, &matrix);
    _divideRealMatrix(&matrix, &x, &res, &ctxtReal39);
    convertReal34MatrixToReal34MatrixRegister(&res, REGISTER_X);
    realMatrixFree(&res);
  }
}



/********************************************//**
 * \brief Y(short integer) ÷ X(real34 matrix) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void divShoIRema(void) {
  real34Matrix_t matrix, res;
  real_t y;
  bool_t divZeroOccurs = false;

  convertShortIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  linkToRealMatrixRegister(REGISTER_X, &matrix);
  if(!getSystemFlag(FLAG_SPCRES)) {
    const uint16_t rows = matrix.header.matrixRows;
    const uint16_t cols = matrix.header.matrixColumns;
    int32_t i;

    for(i = 0; i < cols * rows; ++i) {
      if(real34IsZero(&matrix.matrixElements[i])) {
        divZeroOccurs = true;
      }
    }
  }

  if(divZeroOccurs) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function divShoIRema:", "cannot divide by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  else {
    _divideByRealMatrix(&y, &matrix, &res, &ctxtReal39);
    convertReal34MatrixToReal34MatrixRegister(&res, REGISTER_X);
    realMatrixFree(&res);
  }
}



/********************************************//**
 * \brief Y(real34 matrix) ÷ X(real34) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void divRemaReal(void) {
  real34Matrix_t matrix;
  if(!getSystemFlag(FLAG_SPCRES) && real34IsZero(REGISTER_REAL34_DATA(REGISTER_X))) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function divRemaReal:", "cannot divide by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else if(getRegisterAngularMode(REGISTER_X) == amNone) {
    linkToRealMatrixRegister(REGISTER_Y, &matrix);
    divideRealMatrix(&matrix, REGISTER_REAL34_DATA(REGISTER_X), &matrix);
    convertReal34MatrixToReal34MatrixRegister(&matrix, REGISTER_X);
  }
  else {
    elementwiseRemaReal(divRealReal);
  }
}



/********************************************//**
 * \brief Y(real34) ÷ X(real34 matrix) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void divRealRema(void) {
  real34Matrix_t matrix, res;
  bool_t divZeroOccurs = false;

  linkToRealMatrixRegister(REGISTER_X, &matrix);
  if(!getSystemFlag(FLAG_SPCRES)) {
    const uint16_t rows = matrix.header.matrixRows;
    const uint16_t cols = matrix.header.matrixColumns;
    int32_t i;

    for(i = 0; i < cols * rows; ++i) {
      if(real34IsZero(&matrix.matrixElements[i])) {
        divZeroOccurs = true;
      }
    }
  }

  if(divZeroOccurs) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function divRealRema:", "cannot divide by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  else if(getRegisterAngularMode(REGISTER_Y) == amNone) {
    divideByRealMatrix(REGISTER_REAL34_DATA(REGISTER_Y), &matrix, &res);
    convertReal34MatrixToReal34MatrixRegister(&res, REGISTER_X);
    realMatrixFree(&res);
  }

  else {
    elementwiseRealRema(divRealReal);
  }
}



/********************************************//**
 * \brief Y(real34 matrix) ÷ X(complex34) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void divRemaCplx(void) {
  convertReal34MatrixRegisterToComplex34MatrixRegister(REGISTER_Y, REGISTER_Y);
  divCxmaCplx();
}



/********************************************//**
 * \brief Y(complex34) ÷ X(real34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void divCplxRema(void) {
  convertReal34MatrixRegisterToComplex34MatrixRegister(REGISTER_X, REGISTER_X);
  divCplxCxma();
}



/******************************************************************************************************************************************************************************************/
/* complex34 matrix / ...                                                                                                                                                                 */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(complex34 matrix) ÷ X(long integer) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void divCxmaLonI(void) {
  complex34Matrix_t matrix, res;
  real_t x;

  convertLongIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  linkToComplexMatrixRegister(REGISTER_Y, &matrix);
  _divideComplexMatrix(&matrix, &x, const_0, &res, &ctxtReal39);
  convertComplex34MatrixToComplex34MatrixRegister(&res, REGISTER_X);
  complexMatrixFree(&res);
}



/********************************************//**
 * \brief Y(long integer) ÷ X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void divLonICxma(void) {
  complex34Matrix_t matrix, res;
  real_t y;

  convertLongIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  linkToComplexMatrixRegister(REGISTER_X, &matrix);
  _divideByComplexMatrix(&y, const_0, &matrix, &res, &ctxtReal39);
  convertComplex34MatrixToComplex34MatrixRegister(&res, REGISTER_X);
  complexMatrixFree(&res);
}



/********************************************//**
 * \brief Y(complex34 matrix) ÷ X(real34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void divCxmaRema(void) {
  convertReal34MatrixRegisterToComplex34MatrixRegister(REGISTER_X, REGISTER_X);
  divCxmaCxma();
}



/********************************************//**
 * \brief Y(complex34 matrix) ÷ X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void divCxmaCxma(void) {
  complex34Matrix_t y, x, res;

  linkToComplexMatrixRegister(REGISTER_Y, &y);
  linkToComplexMatrixRegister(REGISTER_X, &x);

  if(y.header.matrixColumns != x.header.matrixRows || y.header.matrixColumns != x.header.matrixColumns || x.header.matrixRows != x.header.matrixColumns) {
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot divide %d" STD_CROSS "%d-matrix and %d" STD_CROSS "%d-matrix",
              y.header.matrixRows, y.header.matrixColumns,
              x.header.matrixRows, x.header.matrixColumns);
      moreInfoOnError("In function divCxmaCxma:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    divideComplexMatrices(&y, &x, &res);
    if(res.matrixElements) {
      convertComplex34MatrixToComplex34MatrixRegister(&res, REGISTER_X);
      complexMatrixFree(&res);
    }
    else {
      displayCalcErrorMessage(ERROR_SINGULAR_MATRIX, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "cannot divide by a singular matrix");
        moreInfoOnError("In function divCxmaCxma:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
}



/********************************************//**
 * \brief Y(complex34 matrix) ÷ X(short integer) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void divCxmaShoI(void) {
  convertShortIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
  divCxmaReal();
}



/********************************************//**
 * \brief Y(short integer) ÷ X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void divShoICxma(void) {
  convertShortIntegerRegisterToReal34Register(REGISTER_Y, REGISTER_Y);
  divRealCxma();
}



/********************************************//**
 * \brief Y(complex34 matrix) ÷ X(real34) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void divCxmaReal(void) {
  complex34Matrix_t matrix;
  if(getRegisterAngularMode(REGISTER_X) == amNone) {
    linkToComplexMatrixRegister(REGISTER_Y, &matrix);
    divideComplexMatrix(&matrix, REGISTER_REAL34_DATA(REGISTER_X), const34_0, &matrix);
    convertComplex34MatrixToComplex34MatrixRegister(&matrix, REGISTER_X);
  }
  else {
    elementwiseCxmaReal(divCplxReal);
  }
}



/********************************************//**
 * \brief Y(real34) ÷ X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void divRealCxma(void) {
  complex34Matrix_t matrix;
  if(getRegisterAngularMode(REGISTER_Y) == amNone) {
    linkToComplexMatrixRegister(REGISTER_X, &matrix);
    divideByComplexMatrix(REGISTER_REAL34_DATA(REGISTER_Y), const34_0, &matrix, &matrix);
  }
  else {
    elementwiseRealCxma(divRealCplx);
  }
}



/********************************************//**
 * \brief Y(complex34 matrix) ÷ X(complex34) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void divCxmaCplx(void) {
  complex34Matrix_t matrix;
  linkToComplexMatrixRegister(REGISTER_Y, &matrix);
  divideComplexMatrix(&matrix, REGISTER_REAL34_DATA(REGISTER_X), REGISTER_IMAG34_DATA(REGISTER_X), &matrix);
  convertComplex34MatrixToComplex34MatrixRegister(&matrix, REGISTER_X);
}



/********************************************//**
 * \brief Y(complex34) ÷ X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void divCplxCxma(void) {
  complex34Matrix_t matrix;
  linkToComplexMatrixRegister(REGISTER_X, &matrix);
  divideByComplexMatrix(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_IMAG34_DATA(REGISTER_Y), &matrix, &matrix);
}



/******************************************************************************************************************************************************************************************/
/* short integer / ...                                                                                                                                                                    */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(short integer) ÷ X(short integer) ==> X(short integer)
 *
 * \param void
 * \return void
 ***********************************************/
void divShoIShoI(void) {
  int16_t sign;
  uint64_t value;

  convertShortIntegerRegisterToUInt64(REGISTER_X, &sign, &value);
  if(value == 0) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function divShoIShoI:", "cannot divide a short integer by 0", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_intDivide(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_Y)), *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)));
    setRegisterShortIntegerBase(REGISTER_X, getRegisterShortIntegerBase(REGISTER_Y));
  }
}



/********************************************//**
 * \brief Y(short integer) ÷ X(real34) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void divShoIReal(void) {
  real_t y, x;

  setRegisterAngularMode(REGISTER_X, amNone);
  convertShortIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);

  if(real34IsZero(REGISTER_REAL34_DATA(REGISTER_X))) {
    if(realIsZero(&y)) {
      if(getSystemFlag(FLAG_SPCRES)) {
        convertRealToReal34ResultRegister(const_NaN, REGISTER_X);
      }
      else {
        displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function divShoIReal:", "cannot divide 0 by 0", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
    }
    else {
      if(getSystemFlag(FLAG_SPCRES)) {
        realToReal34((realIsPositive(&y) ? const_plusInfinity : const_minusInfinity), REGISTER_REAL34_DATA(REGISTER_X));
      }
      else {
        displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function divShoIReal:", "cannot divide a short integer by 0", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
    }
  }

  else {
    real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x);
    realDivide(&y, &x, &x, &ctxtReal39);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
  }
}



/********************************************//**
 * \brief Y(real34) ÷ X(short integer) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void divRealShoI(void) {
  real_t y, x;
  angularMode_t yAngularMode;

  convertShortIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);

  if(realIsZero(&x)) {
    if(real34IsZero(REGISTER_REAL34_DATA(REGISTER_Y))) {
      if(getSystemFlag(FLAG_SPCRES)) {
        convertRealToReal34ResultRegister(const_NaN, REGISTER_X);
      }
      else {
        displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function divRealShoI:", "cannot divide 0 by 0", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
    }
    else {
      if(getSystemFlag(FLAG_SPCRES)) {
        realToReal34((real34IsPositive(REGISTER_REAL34_DATA(REGISTER_Y)) ? const_plusInfinity : const_minusInfinity), REGISTER_REAL34_DATA(REGISTER_X));
      }
      else {
        displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function divRealShoI:", "cannot divide a real34 by 0", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
    }
  }

  else {
    real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y);
    yAngularMode = getRegisterAngularMode(REGISTER_Y);

    if(yAngularMode == amNone) {
      realDivide(&y, &x, &x, &ctxtReal39);
      convertRealToReal34ResultRegister(&x, REGISTER_X);
    }
    else {
      convertAngleFromTo(&y, yAngularMode, currentAngularMode, &ctxtReal39);
      realDivide(&y, &x, &x, &ctxtReal39);
      convertRealToReal34ResultRegister(&x, REGISTER_X);
      setRegisterAngularMode(REGISTER_X, currentAngularMode);
    }
  }
}



/********************************************//**
 * \brief Y(short integer) ÷ X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void divShoICplx(void) {
  real_t y, xReal, xImag;

  convertShortIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &xReal);
  real34ToReal(REGISTER_IMAG34_DATA(REGISTER_X), &xImag);

  divRealComplex(&y, &xReal, &xImag, &xReal, &xImag, &ctxtReal39);

  convertComplexToResultRegister(&xReal, &xImag, REGISTER_X);
}



/********************************************//**
 * \brief Y(complex34) ÷ X(short integer) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void divCplxShoI(void) {
  convertShortIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
  real34Divide(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_Y)); // real part
  real34Divide(REGISTER_IMAG34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_IMAG34_DATA(REGISTER_Y)); // imaginary part
  reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
  complex34Copy(REGISTER_COMPLEX34_DATA(REGISTER_Y), REGISTER_COMPLEX34_DATA(REGISTER_X));
}



/******************************************************************************************************************************************************************************************/
/* real34 / ...                                                                                                                                                                           */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(real34) ÷ X(real34) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void divRealReal(void) {
  if(real34IsZero(REGISTER_REAL34_DATA(REGISTER_Y)) && real34IsZero(REGISTER_REAL34_DATA(REGISTER_X))) {
    if(getSystemFlag(FLAG_SPCRES)) {
      convertRealToReal34ResultRegister(const_NaN, REGISTER_X);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function divRealReal:", "cannot divide 0 by 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }

  else if(real34IsZero(REGISTER_REAL34_DATA(REGISTER_X))) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realToReal34((real34IsPositive(REGISTER_REAL34_DATA(REGISTER_Y)) ? const_plusInfinity : const_minusInfinity), REGISTER_REAL34_DATA(REGISTER_X));
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function divRealReal:", "cannot divide a real34 by 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }

  else {


    real_t y, x;
    angularMode_t yAngularMode, xAngularMode;
    yAngularMode = getRegisterAngularMode(REGISTER_Y);
    xAngularMode = getRegisterAngularMode(REGISTER_X);

    if(yAngularMode == amNone) { // real / (real or angle)
      real34Divide(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
      setRegisterAngularMode(REGISTER_X, amNone);
    }
    else {
      real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y);
      real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x);

      if(yAngularMode != amNone && xAngularMode != amNone) { // angle / angle
        convertAngleFromTo(&x, xAngularMode, yAngularMode, &ctxtReal39);
        realDivide(&y, &x, &x, &ctxtReal39);
        convertRealToReal34ResultRegister(&x, REGISTER_X);
        setRegisterAngularMode(REGISTER_X, amNone);
      }
      else { // angle / real
        realDivide(&y, &x, &x, &ctxtReal39);
        convertAngleFromTo(&x, yAngularMode, currentAngularMode, &ctxtReal39);
        convertRealToReal34ResultRegister(&x, REGISTER_X);
        setRegisterAngularMode(REGISTER_X, currentAngularMode);
      }
    }
  }
}



/********************************************//**
 * \brief Y(real34) ÷ X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void divRealCplx(void) {
  real_t y, xReal, xImag;

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y);
  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &xReal);
  real34ToReal(REGISTER_IMAG34_DATA(REGISTER_X), &xImag);

  divRealComplex(&y, &xReal, &xImag, &xReal, &xImag, &ctxtReal39);

  convertComplexToResultRegister(&xReal, &xImag, REGISTER_X);
}



/********************************************//**
 * \brief Y(complex34) ÷ X(real34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void divCplxReal(void) {
  real34Divide(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_Y)); // real part
  real34Divide(REGISTER_IMAG34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_IMAG34_DATA(REGISTER_Y)); // imaginary part
  reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
  complex34Copy(REGISTER_COMPLEX34_DATA(REGISTER_Y), REGISTER_COMPLEX34_DATA(REGISTER_X));
}



/******************************************************************************************************************************************************************************************/
/* complex34 / ...                                                                                                                                                                        */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(complex34) ÷ X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void divCplxCplx(void) {
  real_t yReal, yImag, xReal, xImag;

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &yReal);
  real34ToReal(REGISTER_IMAG34_DATA(REGISTER_Y), &yImag);
  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &xReal);
  real34ToReal(REGISTER_IMAG34_DATA(REGISTER_X), &xImag);

  divComplexComplex(&yReal, &yImag, &xReal, &xImag, &xReal, &xImag, &ctxtReal39);

  convertComplexToResultRegister(&xReal, &xImag, REGISTER_X);
}
