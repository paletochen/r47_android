// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file multiplication.c
 ***********************************************/

#include "c47.h"

TO_QSPI void (* const multiplication[NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS][NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS])(void) = {
// regX |    regY ==>   1            2            3            4            5         6         7            8            9             10
//      V               Long integer Real34       Complex34    Time         Date      String    Real34 mat   Complex34 m  Short integer Config data
/*  1 Long integer  */ {mulLonILonI, mulRealLonI, mulCplxLonI, mulTimeLonI, mulError, mulError, mulRemaLonI, mulCxmaLonI, mulShoILonI,  mulError},
/*  2 Real34        */ {mulLonIReal, mulRealReal, mulCplxReal, mulTimeReal, mulError, mulError, mulRemaReal, mulCxmaReal, mulShoIReal,  mulError},
/*  3 Complex34     */ {mulLonICplx, mulRealCplx, mulCplxCplx, mulError,    mulError, mulError, mulRemaCplx, mulCxmaCplx, mulShoICplx,  mulError},
/*  4 Time          */ {mulLonITime, mulRealTime, mulError,    mulError,    mulError, mulError, mulError,    mulError,    mulShoITime,  mulError},
/*  5 Date          */ {mulError,    mulError,    mulError,    mulError,    mulError, mulError, mulError,    mulError,    mulError,     mulError},
/*  6 String        */ {mulError,    mulError,    mulError,    mulError,    mulError, mulError, mulError,    mulError,    mulError,     mulError},
/*  7 Real34 mat    */ {mulLonIRema, mulRealRema, mulCplxRema, mulError,    mulError, mulError, mulRemaRema, mulCxmaRema, mulShoIRema,  mulError},
/*  8 Complex34 mat */ {mulLonICxma, mulRealCxma, mulCplxCxma, mulError,    mulError, mulError, mulRemaCxma, mulCxmaCxma, mulShoICxma,  mulError},
/*  9 Short integer */ {mulLonIShoI, mulRealShoI, mulCplxShoI, mulTimeShoI, mulError, mulError, mulRemaShoI, mulCxmaShoI, mulShoIShoI,  mulError},
/* 10 Config data   */ {mulError,    mulError,    mulError,    mulError,    mulError, mulError, mulError,    mulError,    mulError,     mulError}
};


static void multiplyLongIntegerMatrixElement(calcRegister_t integerRegister, const real_t *integer, real34_t *element) {
  if(!multiplyLongIntegerReal34(integerRegister, element, element)) {
    real_t value;

    real34ToReal(element, &value);
    realMultiply(integer, &value, &value, &ctxtReal39);
    roundToSignificantDigits(&value, &value, significantDigits == 0 ? 34 : significantDigits, &ctxtReal39);
    realToReal34(&value, element);
  }
}



/********************************************//**
 * \brief Data type error in multiplication
 *
 * \param[in] unusedButMandatoryParameter
 * \return void
 ***********************************************/
#if (EXTRA_INFO_ON_CALC_ERROR == 1)
  void mulError(void) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    sprintf(errorMessage, "cannot multiply %s", getRegisterDataTypeName(REGISTER_Y, true, false));
    sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "by %s", getRegisterDataTypeName(REGISTER_X, true, false));
    moreInfoOnError("In function fnMultiply:", errorMessage, errorMessage + ERROR_MESSAGE_LENGTH/2, NULL);
  }
#endif // (EXTRA_INFO_ON_CALC_ERROR == 1)



/********************************************//**
 * \brief regX ==> regL and regY × regX ==> regX
 * Drops Y, enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter
 * \return void
 ***********************************************/
void fnMultiply(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }

  multiplication[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)]();

  adjustResult(REGISTER_X, true, true, REGISTER_X, REGISTER_Y, -1);
}



void mulComplexi(const cplx_t *in, cplx_t *product) {
  real_t tmpI;

  realCopy(&in->Imag, &tmpI);
  realChangeSign(&tmpI);
  realCopy(&in->Real, &product->Imag);
  realCopy(&tmpI, &product->Real);
}


void mulComplexComplex(const cplx_t *factor1, const cplx_t *factor2, cplx_t *product, realContext_t *realContext) {
  real_t a, b, c, d, p, t;
  bool_t aIsZero, bIsZero, cIsZero, dIsZero, aIsInfinite, bIsInfinite, cIsInfinite, dIsInfinite;
  bool_t infiniteResult = false;

  realCopy(&factor1->Real, &a);
  aIsZero = realIsZero(&a);
  aIsInfinite = realIsInfinite(&a);

  realCopy(&factor1->Imag, &b);
  bIsZero = realIsZero(&b);
  bIsInfinite = realIsInfinite(&b);

  realCopy(&factor2->Real, &c);
  cIsZero = realIsZero(&c);
  cIsInfinite = realIsInfinite(&c);

  realCopy(&factor2->Imag, &d);
  dIsZero = realIsZero(&d);
  dIsInfinite = realIsInfinite(&d);

  if(   realIsNaN(&a) || realIsNaN(&b) || realIsNaN(&c) || realIsNaN(&d)
     || (aIsZero && bIsZero && (cIsInfinite || dIsInfinite))
     || (cIsZero && dIsZero && (aIsInfinite || bIsInfinite))) {
    realSetNaN(&product->Real);
    realSetNaN(&product->Imag);
    return;
  }

  if((aIsInfinite || bIsInfinite) && (cIsInfinite || dIsInfinite)) { // perform multiplication in polar form
    setInfiniteComplexAngle((getInfiniteComplexAngle(factor1) + getInfiniteComplexAngle(factor2)) % 8, product);
  }
  // Not sure the following lines are needed
  //else if(aIsInfinite || bIsInfinite || cIsInfinite || dIsInfinite) {
  //  realRectangularToPolar(&a, &b, &a, &b, realContext);
  //  realRectangularToPolar(&c, &d, &c, &d, realContext);
  //  realMultiply(&a, &c, &a, realContext);
  //  realAdd(&b, &d, &b, realContext);
  //  realPolarToRectangular(&a, &b, productReal, productImag, realContext);
  //}
  else { // perform multiplication in rectangular form using numerically stable approach
    if(aIsInfinite || bIsInfinite) {
      if(aIsInfinite) {
        int32ToReal(realIsNegative(&a) ? -1 : 1, &a);
      }
      else {
        realSetZero(&a);
      }
      if(bIsInfinite) {
        int32ToReal(realIsNegative(&b) ? -1 : 1, &b);
      }
      else {
        realSetZero(&b);
      }
      infiniteResult = true;
    }
    else if(cIsInfinite || dIsInfinite) {
      if(cIsInfinite) {
        int32ToReal(realIsNegative(&c) ? -1 : 1, &c);
      }
      else {
        realSetZero(&c);
      }
      if(dIsInfinite) {
        int32ToReal(realIsNegative(&d) ? -1 : 1, &d);
      }
      else {
        realSetZero(&d);
      }
      infiniteResult = true;
    }

    // real part
    realMultiply(&a, &c, &p, realContext);                  // RN(ac)
    realChangeSign(&b);
    realFMA(&b, &d, &p, &t, realContext);                   // RN(p - bd)
    realChangeSign(&b);
    realChangeSign(&p);
    realFMA(&a, &c, &p, &product->Real, realContext);       // RN(ac - p)
    realAdd(&product->Real, &t, &product->Real, realContext);  // RN(RN(p - bd) + RN(ac - p))

    // imaginary part
    realMultiply(&a, &d, &p, realContext);                  // RN(ad)
    realFMA(&b, &c, &p, &t, realContext);                   // RN(bc + p)
    realChangeSign(&p);
    realFMA(&a, &d, &p, &product->Imag, realContext);       // RN(ad - p)
    realAdd(&product->Imag, &t, &product->Imag, realContext);  // RN(RN(bc + p) + RN(ad - p))

    if(infiniteResult) {
      if(!realIsZero(&product->Real)) {
        if(realIsNegative(&product->Real)) {
          realSetMinusInfinity(&product->Real);
        }
        else {
          realSetPlusInfinity(&product->Real);
        }
      }
      if(!realIsZero(&product->Imag)) {
        if(realIsNegative(&product->Imag)) {
          realSetMinusInfinity(&product->Imag);
        }
        else {
          realSetPlusInfinity(&product->Imag);
        }
      }
    }
  }
}


#if defined(OPTION_CUBIC_159) || defined(OPTION_SQUARE_159) || defined(OPTION_EIGEN_159)
// This re-write is needed as the mixing of decNumber types cannot deal with the real_t temporary variable withinn mulComplexComplex().
// The 159 series is needed for 39 digit precision in cuberoots and the cubic formula solver
void mulComplexComplex159(const cplx159_t *factor1, const cplx159_t *factor2, cplx159_t *product, realContext_t *realContext) {
  REAL_T_PTR(a, 159);
  REAL_T_PTR(b, 159);
  REAL_T_PTR(c, 159);
  REAL_T_PTR(d, 159);
  REAL_T_PTR(p, 159);
  REAL_T_PTR(t, 159);

  realSetZero(a);
  realSetZero(b);
  realSetZero(c);
  realSetZero(d);
  realSetZero(p);
  realSetZero(t);
  realCopy(&factor1->Real, a);
  realCopy(&factor1->Imag, b);
  realCopy(&factor2->Real, c);
  realCopy(&factor2->Imag, d);

  // Real part: ac - bd
  realMultiply(a, c, p, realContext);
  realChangeSign(b);
  realFMA(b, d, p, &product->Real, realContext);
  realChangeSign(b);

  // Imaginary part: ad + bc
  realMultiply(a, d, p, realContext);
  realFMA(b, c, p, &product->Imag, realContext);
}


// mulComplexReal for 159 digit values in a cplx159_t
void mulComplexReal159(const cplx159_t *factor1, const real_t *factor2, cplx159_t *product, realContext_t *realContext) {
  realMultiply(&factor1->Real, factor2, &product->Real, realContext);
  realMultiply(&factor1->Imag, factor2, &product->Imag, realContext);
}
#endif //OPTION_CUBIC_159 || OPTION_SQUARE_159 || OPTION_EIGEN_159




// Compensated for precision. This is tested and it works, but removed as I don't know the negative impact it may have.

// void mulComplexComplex159(const real_t *factor1Real, const real_t *factor1Imag, const real_t *factor2Real, const real_t *factor2Imag, real_t *productReal, real_t *productImag, realContext_t *realContext) {
//   printf("DEBUG MUL159: realContext->digits = %d\n", realContext->digits);
//   REAL_T_PTR(a, 159);
//   REAL_T_PTR(b, 159);
//   REAL_T_PTR(c, 159);
//   REAL_T_PTR(d, 159);
//   REAL_T_PTR(p, 159);
//   REAL_T_PTR(t, 159);
//   printf("DEBUG: mulComplexComplex159 called with compensated algorithm\n");
//
//   realSetZero(a);
//   realSetZero(b);
//   realSetZero(c);
//   realSetZero(d);
//   realSetZero(p);
//   realSetZero(t);
//   realPlus(factor1Real, a, realContext);
//   realPlus(factor1Imag, b, realContext);
//   realPlus(factor2Real, c, realContext);
//   realPlus(factor2Imag, d, realContext);
//
//
// char dbg[2000];
// printf("DEBUG MUL159: a.digits = %3d = %s\n", a.digits, realToString(a, dbg));
// printf("DEBUG MUL159: b.digits = %3d = %s\n", b.digits, realToString(b, dbg));
// printf("DEBUG MUL159: c.digits = %3d = %s\n", c.digits, realToString(c, dbg));
// printf("DEBUG MUL159: d.digits = %3d = %s\n", d.digits, realToString(d, dbg));
//
//
//
//   // Real part: ac - bd (numerically stable compensated algorithm)
// realMultiply(a, c, p, realContext);
// printf("DEBUG MUL159: p = a*c = %s\n", realToString(p, dbg));
// realChangeSign(b);
// realFMA(b, d, p, t, realContext);
// printf("DEBUG MUL159: t = p - b*d = %s\n", realToString(t, dbg));
//
// //  realMultiply(a, c, p, realContext);                // p = ac
// //  realChangeSign(b);
// //  realFMA(b, d, p, t, realContext);                  // t = p - bd
//   realChangeSign(b);
//   realChangeSign(p);
//   realFMA(a, c, p, productReal, realContext);          // productReal = ac - p
//   realAdd(productReal, t, productReal, realContext);   // productReal = (ac - p) + (p - bd)
//
//   // Imaginary part: ad + bc (numerically stable compensated algorithm)
//   realMultiply(a, d, p, realContext);                  // p = ad
//   realFMA(b, c, p, t, realContext);                    // t = bc + p
//   realChangeSign(p);
//   realFMA(a, d, p, productImag, realContext);          // productImag = ad - p
//   realAdd(productImag, t, productImag, realContext);   // productImag = (ad - p) + (bc + p)
// }




void mulComplexReal(const cplx_t *factor1, const real_t *factor2, cplx_t *product, realContext_t *realContext) {
  realMultiply(&factor1->Real, factor2, &product->Real, realContext);
  realMultiply(&factor1->Imag, factor2, &product->Imag, realContext);
}

/******************************************************************************************************************************************************************************************/
/* long integer × ...                                                                                                                                                                     */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(long integer) × X(long integer) ==> X(long integer)
 *
 * \param void
 * \return void
 ***********************************************/
void mulLonILonI(void) {
  longInteger_t y, x;

  convertLongIntegerRegisterToLongInteger(REGISTER_Y, y);
  convertLongIntegerRegisterToLongInteger(REGISTER_X, x);

  longIntegerMultiply(y, x, x);

  convertLongIntegerToLongIntegerRegister(x, REGISTER_X);

  longIntegerFree(y);
  longIntegerFree(x);
}



/********************************************//**
 * \brief Y(long integer) × X(time) ==> X(time)
 *
 * \param void
 * \return void
 ***********************************************/
void mulLonITime(void) {
  real_t y, x;

  convertLongIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x);

  realMultiply(&y, &x, &x, &ctxtReal39);
  realToReal34(&x, REGISTER_REAL34_DATA(REGISTER_X));
}



/********************************************//**
 * \brief Y(time) × X(long integer) ==> X(time)
 *
 * \param void
 * \return void
 ***********************************************/
void mulTimeLonI(void) {
  real_t y, x;

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y);
  convertLongIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  reallocateRegister(REGISTER_X, dtTime, 0, amNone);

  realMultiply(&y, &x, &x, &ctxtReal39);
  realToReal34(&x, REGISTER_REAL34_DATA(REGISTER_X));
}



/********************************************//**
 * \brief Y(long integer) × X(real34 matrix) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void mulLonIRema(void) {
  real34Matrix_t matrix;
  real_t y;

  convertLongIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  linkToRealMatrixRegister(REGISTER_X, &matrix);
  for(int32_t i = 0; i < matrix.header.matrixColumns * matrix.header.matrixRows; ++i) {
    multiplyLongIntegerMatrixElement(REGISTER_Y, &y, &matrix.matrixElements[i]);
  }
}



/********************************************//**
 * \brief Y(real34 matrix) × X(long integer) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void mulRemaLonI(void) {
  real34Matrix_t matrix;
  real_t x;

  convertLongIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  linkToRealMatrixRegister(REGISTER_Y, &matrix);
  for(int32_t i = 0; i < matrix.header.matrixColumns * matrix.header.matrixRows; ++i) {
    multiplyLongIntegerMatrixElement(REGISTER_X, &x, &matrix.matrixElements[i]);
  }
  fnSwapXY(NOPARAM);
}



/********************************************//**
 * \brief Y(long integer) × X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void mulLonICxma(void) {
  complex34Matrix_t matrix;
  real_t y;

  convertLongIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  linkToComplexMatrixRegister(REGISTER_X, &matrix);
  for(int32_t i = 0; i < matrix.header.matrixColumns * matrix.header.matrixRows; ++i) {
    multiplyLongIntegerMatrixElement(REGISTER_Y, &y, VARIABLE_REAL34_DATA(&matrix.matrixElements[i]));
    multiplyLongIntegerMatrixElement(REGISTER_Y, &y, VARIABLE_IMAG34_DATA(&matrix.matrixElements[i]));
  }
}



/********************************************//**
 * \brief Y(complex34 matrix) × X(long integer) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void mulCxmaLonI(void) {
  complex34Matrix_t matrix;
  real_t x;

  convertLongIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  linkToComplexMatrixRegister(REGISTER_Y, &matrix);
  for(int32_t i = 0; i < matrix.header.matrixColumns * matrix.header.matrixRows; ++i) {
    multiplyLongIntegerMatrixElement(REGISTER_X, &x, VARIABLE_REAL34_DATA(&matrix.matrixElements[i]));
    multiplyLongIntegerMatrixElement(REGISTER_X, &x, VARIABLE_IMAG34_DATA(&matrix.matrixElements[i]));
  }
  fnSwapXY(NOPARAM);
}



/********************************************//**
 * \brief Y(long integer) × X(short integer) ==> X(long integer)
 *
 * \param void
 * \return void
 ***********************************************/
void mulLonIShoI(void) {
  longInteger_t y, x;

  convertShortIntegerRegisterToLongIntegerRegister(REGISTER_X, REGISTER_X);

  convertLongIntegerRegisterToLongInteger(REGISTER_Y, y);
  convertLongIntegerRegisterToLongInteger(REGISTER_X, x);

  longIntegerMultiply(y, x, x);

  convertLongIntegerToLongIntegerRegister(x, REGISTER_X);

  longIntegerFree(y);
  longIntegerFree(x);
}



/********************************************//**
 * \brief Y(short integer) × X(long integer) ==> X(long integer)
 *
 * \param void
 * \return void
 ***********************************************/
void mulShoILonI(void) {
  longInteger_t y, x;

  convertShortIntegerRegisterToLongInteger(REGISTER_Y, y);
  convertLongIntegerRegisterToLongInteger(REGISTER_X, x);

  longIntegerMultiply(y, x, x);

  convertLongIntegerToLongIntegerRegister(x, REGISTER_X);

  longIntegerFree(y);
  longIntegerFree(x);
}



/********************************************//**
 * \brief Y(long integer) × X(real34) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void mulLonIReal(void) {
  lonIRealOp(mulFastLonIReal, realMultiplyOp);
}



/********************************************//**
 * \brief Y(real34) × X(long integer) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void mulRealLonI(void) {
  realLonIOp(mulFastLonIReal, realMultiplyOp);
}



/********************************************//**
 * \brief Y(long integer) × X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void mulLonICplx(void) {
  real_t a;
  cplx_t x;
  real34_t real, imag;

  if(multiplyLongIntegerReal34(REGISTER_Y, REGISTER_REAL34_DATA(REGISTER_X), &real) &&
     multiplyLongIntegerReal34(REGISTER_Y, REGISTER_IMAG34_DATA(REGISTER_X), &imag)) {
    real34Copy(&real, REGISTER_REAL34_DATA(REGISTER_X));
    real34Copy(&imag, REGISTER_IMAG34_DATA(REGISTER_X));
  }
  else {
    convertLongIntegerRegisterToReal(REGISTER_Y, &a, &ctxtReal39);
    real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x.Real);
    real34ToReal(REGISTER_IMAG34_DATA(REGISTER_X), &x.Imag);
    realMultiply(&x.Real, &a, &x.Real, &ctxtReal39);
    realMultiply(&x.Imag, &a, &x.Imag, &ctxtReal39);
    convertComplexToResultRegister(&x, REGISTER_X);
  }
}



/********************************************//**
 * \brief Y(complex34) × X(long integer) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void mulCplxLonI(void) {
  real_t c;
  cplx_t y;
  real34_t real, imag;

  if(multiplyLongIntegerReal34(REGISTER_X, REGISTER_REAL34_DATA(REGISTER_Y), &real) &&
     multiplyLongIntegerReal34(REGISTER_X, REGISTER_IMAG34_DATA(REGISTER_Y), &imag)) {
    convertReal34ToComplex34Register(&real, &imag, REGISTER_X);
  }
  else {
    real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y.Real);
    real34ToReal(REGISTER_IMAG34_DATA(REGISTER_Y), &y.Imag);
    convertLongIntegerRegisterToReal(REGISTER_X, &c, &ctxtReal39);
    realMultiply(&y.Real, &c, &y.Real, &ctxtReal39);
    realMultiply(&y.Imag, &c, &y.Imag, &ctxtReal39);
    reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
    convertComplexToResultRegister(&y, REGISTER_X);
  }
}



/******************************************************************************************************************************************************************************************/
/* time × ...                                                                                                                                                                             */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(time) × X(short integer) ==> X(time)
 *
 * \param void
 * \return void
 ***********************************************/
void mulTimeShoI(void) {
  real_t y, x;

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y);
  convertShortIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  reallocateRegister(REGISTER_X, dtTime, 0, amNone);

  realMultiply(&y, &x, &x, &ctxtReal39);
  realToReal34(&x, REGISTER_REAL34_DATA(REGISTER_X));
}



/********************************************//**
 * \brief Y(short integer) × X(time) ==> X(time)
 *
 * \param void
 * \return void
 ***********************************************/
void mulShoITime(void) {
  real_t y, x;

  convertShortIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x);

  realMultiply(&y, &x, &x, &ctxtReal39);
  realToReal34(&x, REGISTER_REAL34_DATA(REGISTER_X));
}



/********************************************//**
 * \brief Y(time) × X(real34) ==> X(time)
 *
 * \param void
 * \return void
 ***********************************************/
void mulTimeReal(void) {
  real34_t b;
  angularMode_t xAngularMode;

  xAngularMode = getRegisterAngularMode(REGISTER_X);

  if(xAngularMode == amNone) {
    real34Copy(REGISTER_REAL34_DATA(REGISTER_X), &b);
    reallocateRegister(REGISTER_X, dtTime, 0, amNone);
    real34Multiply(REGISTER_REAL34_DATA(REGISTER_Y), &b, REGISTER_REAL34_DATA(REGISTER_X));
  }
  else {
    mulError();
  }
}



/********************************************//**
 * \brief Y(real34) × X(time) ==> X(time)
 *
 * \param void
 * \return void
 ***********************************************/
void mulRealTime(void) {
  angularMode_t yAngularMode;

  yAngularMode = getRegisterAngularMode(REGISTER_Y);

  if(yAngularMode == amNone) {
    real34Multiply(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
  }
  else {
    mulError();
  }
}



/******************************************************************************************************************************************************************************************/
/* time × ...                                                                                                                                                                             */
/******************************************************************************************************************************************************************************************/



/******************************************************************************************************************************************************************************************/
/* string × ...                                                                                                                                                                           */
/******************************************************************************************************************************************************************************************/



/******************************************************************************************************************************************************************************************/
/* real34 matrix × ...                                                                                                                                                                    */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(real34 matrix) × X(real34 matrix) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void mulRemaRema(void) {
  real34Matrix_t y, x, res;

  linkToRealMatrixRegister(REGISTER_Y, &y);
  linkToRealMatrixRegister(REGISTER_X, &x);

  multiplyRealMatrices(&y, &x, &res);
  if(res.matrixElements) {
    convertReal34MatrixToReal34MatrixRegister(&res, REGISTER_X);
    realMatrixFree(&res);
  }
  else {
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot multiply %d" STD_CROSS "%d-matrix and %d" STD_CROSS "%d-matrix",
              y.header.matrixRows, y.header.matrixColumns,
              x.header.matrixRows, x.header.matrixColumns);
      moreInfoOnError("In function mulRemaRema:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}



/********************************************//**
 * \brief Y(real34 matrix) × X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void mulRemaCxma(void) {
  convertReal34MatrixRegisterToComplex34MatrixRegister(REGISTER_Y, REGISTER_Y);
  if(lastErrorCode != 0) {
    return;
  }
  mulCxmaCxma();
}



/********************************************//**
 * \brief Y(complex34 matrix) × X(real34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void mulCxmaRema(void) {
  convertReal34MatrixRegisterToComplex34MatrixRegister(REGISTER_X, REGISTER_X);
  if(lastErrorCode != 0) {
    return;
  }
  mulCxmaCxma();
}



/********************************************//**
 * \brief Y(real34 matrix) × X(short integer) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void mulRemaShoI(void) {
  convertShortIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
  mulRemaReal();
}



/********************************************//**
 * \brief Y(short integer) × X(real34 matrix) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void mulShoIRema(void) {
  convertShortIntegerRegisterToReal34Register(REGISTER_Y, REGISTER_Y);
  mulRealRema();
}



/********************************************//**
 * \brief Y(real34 matrix) × X(real34) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void mulRemaReal(void) {
  real34Matrix_t matrix;
  if(getRegisterAngularMode(REGISTER_X) == amNone) {
    linkToRealMatrixRegister(REGISTER_Y, &matrix);
    multiplyRealMatrix(&matrix, REGISTER_REAL34_DATA(REGISTER_X), &matrix);
    convertReal34MatrixToReal34MatrixRegister(&matrix, REGISTER_X);
  }
  else {
    elementwiseRemaReal(mulRealReal);
  }
}



/********************************************//**
 * \brief Y(real34) × X(real34 matrix) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void mulRealRema(void) {
  real34Matrix_t matrix;
  if(getRegisterAngularMode(REGISTER_Y) == amNone) {
    linkToRealMatrixRegister(REGISTER_X, &matrix);
    multiplyRealMatrix(&matrix, REGISTER_REAL34_DATA(REGISTER_Y), &matrix);
  }
  else {
    elementwiseRealRema(mulRealReal);
  }
}



/********************************************//**
 * \brief Y(real34 matrix) × X(complex34) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void mulRemaCplx(void) {
  convertReal34MatrixRegisterToComplex34MatrixRegister(REGISTER_Y, REGISTER_Y);
  if(lastErrorCode != 0) {
    return;
  }
  mulCxmaCplx();
}



/********************************************//**
 * \brief Y(complex34) × X(real34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void mulCplxRema(void) {
  convertReal34MatrixRegisterToComplex34MatrixRegister(REGISTER_X, REGISTER_X);
  if(lastErrorCode != 0) {
    return;
  }
  mulCplxCxma();
}



/******************************************************************************************************************************************************************************************/
/* complex34 matrix × ...                                                                                                                                                                 */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(complex34 matrix) × X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void mulCxmaCxma(void) {
  complex34Matrix_t y, x, res;

  linkToComplexMatrixRegister(REGISTER_Y, &y);
  linkToComplexMatrixRegister(REGISTER_X, &x);

  multiplyComplexMatrices(&y, &x, &res);
  if(res.matrixElements) {
    convertComplex34MatrixToComplex34MatrixRegister(&res, REGISTER_X);
    complexMatrixFree(&res);
  }
  else {
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot multiply %d" STD_CROSS "%d-matrix and %d" STD_CROSS "%d-matrix",
              y.header.matrixRows, y.header.matrixColumns,
              x.header.matrixRows, x.header.matrixColumns);
      moreInfoOnError("In function mulCxmaCxma:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}



/********************************************//**
 * \brief Y(complex34 matrix) × X(short integer) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void mulCxmaShoI(void) {
  convertShortIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
  mulCxmaReal();
}



/********************************************//**
 * \brief Y(short integer) × X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void mulShoICxma(void) {
  convertShortIntegerRegisterToReal34Register(REGISTER_Y, REGISTER_Y);
  mulRealCxma();
}



/********************************************//**
 * \brief Y(complex34 matrix) × X(real34) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void mulCxmaReal(void) {
  complex34Matrix_t matrix;
  if(getRegisterAngularMode(REGISTER_X) == amNone) {
    linkToComplexMatrixRegister(REGISTER_Y, &matrix);
    multiplyComplexMatrix(&matrix, REGISTER_REAL34_DATA(REGISTER_X), const34_0, &matrix);
    convertComplex34MatrixToComplex34MatrixRegister(&matrix, REGISTER_X);
  }
  else {
    elementwiseCxmaReal(mulCplxReal);
  }
}



/********************************************//**
 * \brief Y(real34) × X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void mulRealCxma(void) {
  complex34Matrix_t matrix;
  if(getRegisterAngularMode(REGISTER_Y) == amNone) {
    linkToComplexMatrixRegister(REGISTER_X, &matrix);
    multiplyComplexMatrix(&matrix, REGISTER_REAL34_DATA(REGISTER_Y), const34_0, &matrix);
  }
  else {
    elementwiseRealCxma(mulRealCplx);
  }
}



/********************************************//**
 * \brief Y(complex34 matrix) × X(complex34) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void mulCxmaCplx(void) {
  complex34Matrix_t matrix;
  linkToComplexMatrixRegister(REGISTER_Y, &matrix);
  multiplyComplexMatrix(&matrix, REGISTER_REAL34_DATA(REGISTER_X), REGISTER_IMAG34_DATA(REGISTER_X), &matrix);
  convertComplex34MatrixToComplex34MatrixRegister(&matrix, REGISTER_X);
}



/********************************************//**
 * \brief Y(complex34) × X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void mulCplxCxma(void) {
  complex34Matrix_t matrix;
  linkToComplexMatrixRegister(REGISTER_X, &matrix);
  multiplyComplexMatrix(&matrix, REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_IMAG34_DATA(REGISTER_Y), &matrix);
}



/******************************************************************************************************************************************************************************************/
/* short integer × ...                                                                                                                                                                    */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(short integer) × X(short integer) ==> X(short integer)
 *
 * \param void
 * \return void
 ***********************************************/
void mulShoIShoI(void) {
  setRegisterTag(REGISTER_X, getRegisterTag(REGISTER_Y));
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_intMultiply(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_Y)), *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)));
}



/********************************************//**
 * \brief Y(short integer) × X(real34) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void mulShoIReal(void) {
  angularMode_t xAngularMode;

  xAngularMode = getRegisterAngularMode(REGISTER_X);

  if(xAngularMode == amNone) {
    convertShortIntegerRegisterToReal34Register(REGISTER_Y, REGISTER_Y);
    real34Multiply(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
  }
  else {
    real_t y, x;

    convertShortIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
    real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x);
    convertAngleFromTo(&x, xAngularMode, currentAngularMode, &ctxtReal39);
    realMultiply(&y, &x, &x, &ctxtReal39);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
    setRegisterAngularMode(REGISTER_X, currentAngularMode);
  }
}



/********************************************//**
 * \brief Y(real34) × X(short integer) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void mulRealShoI(void) {
  angularMode_t yAngularMode;

  yAngularMode = getRegisterAngularMode(REGISTER_Y);

  if(yAngularMode == amNone) {
    convertShortIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
    real34Multiply(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
  }
  else {
    real_t y, x;

    real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y);
    convertShortIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    convertAngleFromTo(&y, yAngularMode, currentAngularMode, &ctxtReal39);
    realMultiply(&y, &x, &x, &ctxtReal39);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
    setRegisterAngularMode(REGISTER_X, currentAngularMode);
  }
}



/********************************************//**
 * \brief Y(short integer) × X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void mulShoICplx(void) {
  convertShortIntegerRegisterToReal34Register(REGISTER_Y, REGISTER_Y);
  real34Multiply(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X)); // real part
  real34Multiply(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_IMAG34_DATA(REGISTER_X), REGISTER_IMAG34_DATA(REGISTER_X)); // imaginary part
}



/********************************************//**
 * \brief Y(complex34) × X(short integer) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void mulCplxShoI(void) {
  convertShortIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
  real34Multiply(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_Y)); // real part
  real34Multiply(REGISTER_IMAG34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_IMAG34_DATA(REGISTER_Y)); // imaginary part
  convertReal34ToComplex34Register(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_IMAG34_DATA(REGISTER_Y), REGISTER_X);
}



/******************************************************************************************************************************************************************************************/
/* real34 × ...                                                                                                                                                                           */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(real34) × X(real34) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void mulRealReal(void) {
  angularMode_t yAngularMode, xAngularMode;

  yAngularMode = getRegisterAngularMode(REGISTER_Y);
  xAngularMode = getRegisterAngularMode(REGISTER_X);

  if(yAngularMode == amNone && xAngularMode == amNone) { // Neither is an angle
    real34Multiply(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
  }
  else if(yAngularMode != amNone && xAngularMode != amNone) { // Both are angles
    real34Multiply(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
    setRegisterAngularMode(REGISTER_X, amNone);
  }
  else { // One and only one is an angle
    real_t y, x;

    real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y);
    real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x);
    realMultiply(&y, &x, &x, &ctxtReal39);

    convertAngleFromTo(&x, yAngularMode != amNone ? yAngularMode : xAngularMode, currentAngularMode, &ctxtReal39);

    convertRealToReal34ResultRegister(&x, REGISTER_X);
    setRegisterAngularMode(REGISTER_X, currentAngularMode);
  }
}



/********************************************//**
 * \brief Y(real34) × X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void mulRealCplx(void) {
  real34Multiply(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X)); // real part
  real34Multiply(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_IMAG34_DATA(REGISTER_X), REGISTER_IMAG34_DATA(REGISTER_X)); // imaginary part
}



/********************************************//**
 * \brief Y(complex34) × X(real34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void mulCplxReal(void) {
  real34Multiply(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_Y)); // real part
  real34Multiply(REGISTER_IMAG34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_IMAG34_DATA(REGISTER_Y)); // imaginary part
  convertReal34ToComplex34Register(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_IMAG34_DATA(REGISTER_Y), REGISTER_X); // imaginary part
}



/******************************************************************************************************************************************************************************************/
/* complex34 × ...                                                                                                                                                                        */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(complex34) × X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void mulCplxCplx(void) {
  cplx_t y, x;

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y.Real);
  real34ToReal(REGISTER_IMAG34_DATA(REGISTER_Y), &y.Imag);
  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x.Real);
  real34ToReal(REGISTER_IMAG34_DATA(REGISTER_X), &x.Imag);

  mulComplexComplex(&y, &x, &x, &ctxtReal39);

  convertComplexToResultRegister(&x, REGISTER_X);
}
