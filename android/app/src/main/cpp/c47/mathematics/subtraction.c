// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file subtraction.c
 ***********************************************/

#include "c47.h"



TO_QSPI void (* const subtraction[NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS][NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS])(void) = {
// regX |    regY ==>   1            2            3            4            5            6         7            8            9             10
//      V               Long integer Real34       Complex34    Time         Date         String    Real34 mat   Complex34 m  Short integer Config data
/*  1 Long integer  */ {subLonILonI, subRealLonI, subCplxLonI, subTimeLonI, subDateLonI, subError, subRemaLonI, subCxmaLonI, subShoILonI,  subError},
/*  2 Real34        */ {subLonIReal, subRealReal, subCplxReal, subTimeReal, subDateReal, subError, subRemaReal, subCxmaReal, subShoIReal,  subError},
/*  3 Complex34     */ {subLonICplx, subRealCplx, subCplxCplx, subError,    subError,    subError, subRemaCplx, subCxmaCplx, subShoICplx,  subError},
/*  4 Time          */ {subLonITime, subRealTime, subError,    subTimeTime, subError,    subError, subError,    subError,    subError,     subError},
/*  5 Date          */ {subError,    subError,    subError,    subError,    subDateDate, subError, subError,    subError,    subError,     subError},
/*  6 String        */ {subError,    subError,    subError,    subError,    subError,    subError, subError,    subError,    subError,     subError},
/*  7 Real34 mat    */ {subLonIRema, subRealRema, subCplxRema, subError,    subError,    subError, subRemaRema, subCxmaRema, subShoIRema,  subError},
/*  8 Complex34 mat */ {subLonICxma, subRealCxma, subCplxCxma, subError,    subError,    subError, subRemaCxma, subCxmaCxma, subShoICxma,  subError},
/*  9 Short integer */ {subLonIShoI, subRealShoI, subCplxShoI, subError,    subError,    subError, subRemaShoI, subCxmaShoI, subShoIShoI,  subError},
/* 10 Config data   */ {subError,    subError,    subError,    subError,    subError,    subError, subError,    subError,    subError,     subError}
};


static void subtractLongIntegerMatrixElement(calcRegister_t integerRegister, const real_t *integer, bool_t integerFirst, real34_t *element) {
  if(!addSubtractLongIntegerReal34(integerRegister, element, integerFirst, true, element)) {
    real_t value;

    real34ToReal(element, &value);
    if(integerFirst) {
      realSubtract(integer, &value, &value, &ctxtReal39);
    }
    else {
      realSubtract(&value, integer, &value, &ctxtReal39);
    }
    roundToSignificantDigits(&value, &value, significantDigits == 0 ? 34 : significantDigits, &ctxtReal39);
    realToReal34(&value, element);
  }
}



/********************************************//**
 * \brief Data type error in subtraction
 *
 * \param void
 * \return void
 ***********************************************/
#if (EXTRA_INFO_ON_CALC_ERROR == 1)
  void subError(void) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    sprintf(errorMessage, "cannot subtract %s", getRegisterDataTypeName(REGISTER_X, true, false));
    sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "from %s", getRegisterDataTypeName(REGISTER_Y, true, false));
    moreInfoOnError("In function fnSubtract:", errorMessage, errorMessage + ERROR_MESSAGE_LENGTH/2, NULL);
  }
#endif // (EXTRA_INFO_ON_CALC_ERROR == 1)



/********************************************//**
 * \brief rexX ==> regL and regY - regX ==> regX
 * Drops Y, enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSubtract(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }

  subtraction[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)]();

  adjustResult(REGISTER_X, true, true, REGISTER_X, REGISTER_Y, -1);
}



/******************************************************************************************************************************************************************************************/
/* long integer - ...                                                                                                                                                                     */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(long integer) - X(long integer) ==> X(long integer)
 *
 * \param void
 * \return void
 ***********************************************/
void subLonILonI(void) {
  addSubIntInt(longIntegerSubtract, convertLongIntegerRegisterToLongInteger, convertLongIntegerRegisterToLongInteger);
}



/********************************************//**
 * \brief Y(long integer) - X(time) ==> X(time)
 *
 * \param void
 * \return void
 ***********************************************/
void subLonITime(void) {
  addSubTimeLonI(real34SubtractOp, REGISTER_Y);
}



/********************************************//**
 * \brief Y(time) - X(long integer) ==> X(time)
 *
 * \param void
 * \return void
 ***********************************************/
void subTimeLonI(void) {
  addSubTimeLonI(real34SubtractOp, REGISTER_X);
}



/********************************************//**
 * \brief Y(date) - X(long integer) ==> X(date)
 *
 * \param void
 * \return void
 ***********************************************/
void subDateLonI(void) {
  addSubDateLonI(real34SubtractOp);
}



/********************************************//**
 * \brief Y(long integer) - X(short integer) ==> X(long integer)
 *
 * \param void
 * \return void
 ***********************************************/
void subLonIShoI(void) {
  addSubIntInt(longIntegerSubtract, convertLongIntegerRegisterToLongInteger, convertShortIntegerRegisterToLongInteger);
}



/********************************************//**
 * \brief Y(short integer) - X(long integer) ==> X(long integer)
 *
 * \param void
 * \return void
 ***********************************************/
void subShoILonI(void) {
  addSubIntInt(longIntegerSubtract, convertShortIntegerRegisterToLongInteger, convertLongIntegerRegisterToLongInteger);
}



/********************************************//**
 * \brief Y(long integer) - X(real34) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void subLonIReal(void) {
  lonIRealOp(subFastLonIReal, realSubtractOp);
}



/********************************************//**
 * \brief Y(real34) - X(long integer) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void subRealLonI(void) {
  realLonIOp(subFastLonIReal, realSubtractOp);
}



/********************************************//**
 * \brief Y(long integer) - X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void subLonICplx(void) {
  real_t a, c;

  if(!addSubtractLongIntegerReal34(REGISTER_Y, REGISTER_REAL34_DATA(REGISTER_X), true, true, REGISTER_REAL34_DATA(REGISTER_X))) {
    convertLongIntegerRegisterToReal(REGISTER_Y, &a, &ctxtReal39);
    real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &c);
    realSubtract(&a, &c, &c, &ctxtReal39);
    convertRealToReal34ResultRegister(&c, REGISTER_X);
  }
  real34ChangeSign(REGISTER_IMAG34_DATA(REGISTER_X));

}



/********************************************//**
 * \brief Y(complex34) - X(long integer) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void subCplxLonI(void) {
  real_t a, c;
  real34_t b, result;

  real34Copy(REGISTER_IMAG34_DATA(REGISTER_Y), &b);
  if(addSubtractLongIntegerReal34(REGISTER_X, REGISTER_REAL34_DATA(REGISTER_Y), false, true, &result)) {
    reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
    real34Copy(&result, REGISTER_REAL34_DATA(REGISTER_X));
  }
  else {
    real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &a);
    convertLongIntegerRegisterToReal(REGISTER_X, &c, &ctxtReal39);
    realSubtract(&a, &c, &c, &ctxtReal39);
    reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
    convertRealToReal34ResultRegister(&c, REGISTER_X);
  }
  real34Copy(&b, REGISTER_IMAG34_DATA(REGISTER_X));

}



/******************************************************************************************************************************************************************************************/
/* time - ...                                                                                                                                                                             */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(time) - X(time) ==> X(time)
 *
 * \param void
 * \return void
 ***********************************************/
void subTimeTime(void) {
  real34Subtract(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
}



/********************************************//**
 * \brief Y(time) - X(real34) ==> X(time)
 *
 * \param void
 * \return void
 ***********************************************/
void subTimeReal(void) {
  angularMode_t xAngularMode = getRegisterAngularMode(REGISTER_X);

  if(xAngularMode == amNone) {
    convertReal34RegisterToTimeRegister(REGISTER_X, REGISTER_X);
    real34Subtract(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
  }
  else {
    subError();
  }
}



/********************************************//**
 * \brief Y(real34) - X(time) ==> X(time)
 *
 * \param void
 * \return void
 ***********************************************/
void subRealTime(void) {
  angularMode_t yAngularMode = getRegisterAngularMode(REGISTER_Y);

  if(yAngularMode == amNone) {
    convertReal34RegisterToTimeRegister(REGISTER_Y, REGISTER_Y);
    real34Subtract(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
  }
  else {
    subError();
  }
}



/******************************************************************************************************************************************************************************************/
/* date - ...                                                                                                                                                                             */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(date) - X(date) ==> X(long integer)
 *
 * \param void
 * \return void
 ***********************************************/
void subDateDate(void) {
  real34_t val;

  real34Subtract(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_Y));
  real34Divide(REGISTER_REAL34_DATA(REGISTER_Y), const34_86400, &val);
  convertReal34ToLongIntegerRegister(&val, REGISTER_X, DEC_ROUND_DOWN);
}



/********************************************//**
 * \brief Y(date) - X(real34) ==> X(date)
 *
 * \param void
 * \return void
 ***********************************************/
void subDateReal(void) {
  angularMode_t xAngularMode = getRegisterAngularMode(REGISTER_X);
  real34_t val;

  if(xAngularMode == amNone) {
    real34ToIntegralValue(REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X), roundingModeTable[roundingMode]);
    real34Multiply(REGISTER_REAL34_DATA(REGISTER_X), const34_86400, &val);
    reallocateRegister(REGISTER_X, dtDate, 0, amNone);
    real34Subtract(REGISTER_REAL34_DATA(REGISTER_Y), &val, REGISTER_REAL34_DATA(REGISTER_X));
  }
  else {
    subError();
  }
}



/******************************************************************************************************************************************************************************************/
/* string - ...                                                                                                                                                                           */
/******************************************************************************************************************************************************************************************/

/******************************************************************************************************************************************************************************************/
/* real34 matrix - ...                                                                                                                                                                    */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(real34 matrix) - X(long integer) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subRemaLonI(void) {
  real34Matrix_t ym;
  real_t x;

  linkToRealMatrixRegister(REGISTER_Y, &ym);

  const uint16_t rows = ym.header.matrixRows;
  const uint16_t cols = ym.header.matrixColumns;
  int32_t i;

  convertLongIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  for(i = 0; i < cols * rows; ++i) {
    subtractLongIntegerMatrixElement(REGISTER_X, &x, false, &ym.matrixElements[i]);
  }
  fnSwapXY(NOPARAM);
}



/********************************************//**
 * \brief Y(long integer) - X(real34 matrix) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subLonIRema(void) {
  real34Matrix_t xm;
  real_t y;

  linkToRealMatrixRegister(REGISTER_X, &xm);

  const uint16_t rows = xm.header.matrixRows;
  const uint16_t cols = xm.header.matrixColumns;
  int32_t i;

  convertLongIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  for(i = 0; i < cols * rows; ++i) {
    subtractLongIntegerMatrixElement(REGISTER_Y, &y, true, &xm.matrixElements[i]);
  }
}



/********************************************//**
 * \brief Y(real34 matrix) - X(real34 matrix) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subRemaRema(void) {
  real34Matrix_t y, x;

  linkToRealMatrixRegister(REGISTER_Y, &y);
  convertReal34MatrixRegisterToReal34Matrix(REGISTER_X, &x);
  if(lastErrorCode != 0) {
    return;
  }

  subtractRealMatrices(&y, &x, &x);
  if(x.matrixElements) {
    convertReal34MatrixToReal34MatrixRegister(&x, REGISTER_X);
  }
  else {
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot subtract %d" STD_CROSS "%d-matrix from %d" STD_CROSS "%d-matrix",
              x.header.matrixRows, x.header.matrixColumns,
              y.header.matrixRows, y.header.matrixColumns);
      moreInfoOnError("In function subRemaRema:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  realMatrixFree(&x);
}



/********************************************//**
 * \brief Y(real34 matrix) - X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subRemaCxma(void) {
  convertReal34MatrixRegisterToComplex34MatrixRegister(REGISTER_Y, REGISTER_Y);
  if(lastErrorCode != 0) {
    return;
  }
  subCxmaCxma();
}



/********************************************//**
 * \brief Y(complex34 matrix) - X(real34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subCxmaRema(void) {
  convertReal34MatrixRegisterToComplex34MatrixRegister(REGISTER_X, REGISTER_X);
  if(lastErrorCode != 0) {
    return;
  }
  subCxmaCxma();
}



/********************************************//**
 * \brief Y(real34 matrix) - X(short integer) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subRemaShoI(void) {
  convertShortIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
  subRemaReal();
}



/********************************************//**
 * \brief Y(short integer) - X(real34 matrix) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subShoIRema(void) {
  convertShortIntegerRegisterToReal34Register(REGISTER_Y, REGISTER_Y);
  subRealRema();
}



/********************************************//**
 * \brief Y(real34 matrix) + X(real34) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subRemaReal(void) {
  addSubRemaReal(real34SubtractOp, subRealReal);
}



/********************************************//**
 * \brief Y(real34) - X(real34 matrix) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subRealRema(void) {
  addSubRealRema(real34SubtractOp, subRealReal);
}



/********************************************//**
 * \brief Y(real34 matrix) - X(complex34) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subRemaCplx(void) {
  convertReal34MatrixRegisterToComplex34MatrixRegister(REGISTER_Y, REGISTER_Y);
  if(lastErrorCode != 0) {
    return;
  }
  subCxmaCplx();
}



/********************************************//**
 * \brief Y(complex34) - X(real34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subCplxRema(void) {
  convertReal34MatrixRegisterToComplex34MatrixRegister(REGISTER_X, REGISTER_X);
  if(lastErrorCode != 0) {
    return;
  }
  subCplxCxma();
}



/******************************************************************************************************************************************************************************************/
/* complex34 matrix - ...                                                                                                                                                                 */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(complex34 matrix) - X(long integer) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subCxmaLonI(void) {
  complex34Matrix_t ym;
  real_t x;

  linkToComplexMatrixRegister(REGISTER_Y, &ym);

  const uint16_t rows = ym.header.matrixRows;
  const uint16_t cols = ym.header.matrixColumns;
  int32_t i;

  convertLongIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  for(i = 0; i < cols * rows; ++i) {
    subtractLongIntegerMatrixElement(REGISTER_X, &x, false, VARIABLE_REAL34_DATA(&ym.matrixElements[i]));
  }
  fnSwapXY(NOPARAM);
}



/********************************************//**
 * \brief Y(long integer) - X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subLonICxma(void) {
  complex34Matrix_t xm;
  real_t y;

  linkToComplexMatrixRegister(REGISTER_X, &xm);

  const uint16_t rows = xm.header.matrixRows;
  const uint16_t cols = xm.header.matrixColumns;
  int32_t i;

  convertLongIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  for(i = 0; i < cols * rows; ++i) {
    subtractLongIntegerMatrixElement(REGISTER_Y, &y, true, VARIABLE_REAL34_DATA(&xm.matrixElements[i]));
    real34ChangeSign(VARIABLE_IMAG34_DATA(&xm.matrixElements[i]));
  }
}



/********************************************//**
 * \brief Y(complex34 matrix) - X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subCxmaCxma(void) {
  complex34Matrix_t y, x;

  linkToComplexMatrixRegister(REGISTER_Y, &y);
  convertComplex34MatrixRegisterToComplex34Matrix(REGISTER_X, &x);
  if(lastErrorCode != 0) {
    return;
  }

  subtractComplexMatrices(&y, &x, &x);
  if(x.matrixElements) {
    convertComplex34MatrixToComplex34MatrixRegister(&x, REGISTER_X);
  }
  else {
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot add %d" STD_CROSS "%d-matrix to %d" STD_CROSS "%d-matrix",
              x.header.matrixRows, x.header.matrixColumns,
              y.header.matrixRows, y.header.matrixColumns);
      moreInfoOnError("In function subCxmaCxma:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  complexMatrixFree(&x);
}



/********************************************//**
 * \brief Y(complex34 matrix) - X(short integer) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subCxmaShoI(void) {
  convertShortIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
  subCxmaReal();
}



/********************************************//**
 * \brief Y(short integer) - X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subShoICxma(void) {
  convertShortIntegerRegisterToReal34Register(REGISTER_Y, REGISTER_Y);
  subRealCxma();
}



/********************************************//**
 * \brief Y(complex34 matrix) - X(real34) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subCxmaReal(void) {
  addSubCxmaReal(real34SubtractOp, subCplxReal);
}



/********************************************//**
 * \brief Y(real34) - X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subRealCxma(void) {
  complex34Matrix_t x;
  angularMode_t yAngularMode;

  linkToComplexMatrixRegister(REGISTER_X, &x);
  yAngularMode = getRegisterAngularMode(REGISTER_Y);

  if(yAngularMode == amNone) {
    const uint16_t rows = x.header.matrixRows;
    const uint16_t cols = x.header.matrixColumns;
    int32_t i;

    for(i = 0; i < cols * rows; ++i) {
      real34Subtract(REGISTER_REAL34_DATA(REGISTER_Y), VARIABLE_REAL34_DATA(&x.matrixElements[i]), VARIABLE_REAL34_DATA(&x.matrixElements[i]));
      real34ChangeSign(VARIABLE_IMAG34_DATA(&x.matrixElements[i]));
    }
  }
  else {
    elementwiseRealCxma(subRealCplx);
  }
}



/********************************************//**
 * \brief Y(complex34 matrix) - X(complex34) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subCxmaCplx(void) {
  addSubCxmaCplx(real34SubtractOp);
}



/********************************************//**
 * \brief Y(complex34) - X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subCplxCxma(void) {
  addSubCplxCxma(real34SubtractOp);
}



/******************************************************************************************************************************************************************************************/
/* short integer + ...                                                                                                                                                                    */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(Short integer) - X(Short integer) ==> X(Short integer)
 *
 * \param void
 * \return void
 ***********************************************/
void subShoIShoI(void) {
  setRegisterTag(REGISTER_X, getRegisterTag(REGISTER_Y));
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_intSubtract(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_Y)), *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)));
}



/********************************************//**
 * \brief Y(short integer) - X(real34) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void subShoIReal(void) {
  angularMode_t xAngularMode = getRegisterAngularMode(REGISTER_X);

  if(xAngularMode == amNone) {
    convertShortIntegerRegisterToReal34Register(REGISTER_Y, REGISTER_Y);
    real34Subtract(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
  }
  else {
    real_t y, x;

    convertShortIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
    real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x);
    convertAngleFromTo(&x, xAngularMode, currentAngularMode, &ctxtReal39);
    realSubtract(&y, &x, &x, &ctxtReal39);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
    setRegisterAngularMode(REGISTER_X, currentAngularMode);
  }
}



/********************************************//**
 * \brief Y(real34) - X(short integer) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void subRealShoI(void) {
  angularMode_t yAngularMode = getRegisterAngularMode(REGISTER_Y);

  if(yAngularMode == amNone) {
    convertShortIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
    real34Subtract(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
  }
  else {
    real_t y, x;

    real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y);
    convertShortIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    convertAngleFromTo(&y, yAngularMode, currentAngularMode, &ctxtReal39);
    realSubtract(&y, &x, &x, &ctxtReal39);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
    setRegisterAngularMode(REGISTER_X, currentAngularMode);
  }
}



/********************************************//**
 * \brief Y(short integer) - X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void subShoICplx(void) {
  convertShortIntegerRegisterToReal34Register(REGISTER_Y, REGISTER_Y);
  real34Subtract(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X)); // real part
  real34ChangeSign(REGISTER_IMAG34_DATA(REGISTER_X));
}




/********************************************//**
 * \brief Y(complex34) - X(short integer) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void subCplxShoI(void) {
  addSubCplxShoI(real34SubtractOp);
}



/******************************************************************************************************************************************************************************************/
/* real34 - ...                                                                                                                                                                           */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(real34) - X(real34) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void subRealReal(void) {
  angularMode_t yAngularMode = getRegisterAngularMode(REGISTER_Y);
  angularMode_t xAngularMode = getRegisterAngularMode(REGISTER_X);

  if(yAngularMode == amNone && xAngularMode == amNone) {
    real34Subtract(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
  }
  else {
    real_t y, x;

    real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y);
    real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x);

    if(yAngularMode == amNone) {
      yAngularMode = currentAngularMode;
    }
    else if(xAngularMode == amNone) {
      xAngularMode = currentAngularMode;
    }

    convertAngleFromTo(&y, yAngularMode, currentAngularMode, &ctxtReal39);
    convertAngleFromTo(&x, xAngularMode, currentAngularMode, &ctxtReal39);

    realSubtract(&y, &x, &x, &ctxtReal39);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
    setRegisterAngularMode(REGISTER_X, currentAngularMode);
  }
}



/********************************************//**
 * \brief Y(real34) - X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void subRealCplx(void) {
  real34Subtract(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X)); // real part
  real34ChangeSign(REGISTER_IMAG34_DATA(REGISTER_X));
}



/********************************************//**
 * \brief Y(complex34) - X(real34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void subCplxReal(void) {
  real34Subtract(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_Y)); // real part
  convertReal34ToComplex34Register(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_IMAG34_DATA(REGISTER_Y), REGISTER_X);
}



/******************************************************************************************************************************************************************************************/
/* complex34 - ...                                                                                                                                                                        */
/******************************************************************************************************************************************************************************************/
void subComplex(const cplx_t *a, const cplx_t *b, cplx_t *res, realContext_t *realContext) {
  realSubtract(&a->Real, &b->Real, &res->Real, realContext);
  realSubtract(&a->Imag, &b->Imag, &res->Imag, realContext);
}


#if defined(OPTION_CUBIC_159) || defined(OPTION_SQUARE_159) || defined(OPTION_EIGEN_159)
// subComplex for 159 digit values in a cplx159_t
void subComplex159(const cplx159_t *a, const cplx159_t *b, cplx159_t *res, realContext_t *realContext) {
  realSubtract(&a->Real, &b->Real, &res->Real, realContext);
  realSubtract(&a->Imag, &b->Imag, &res->Imag, realContext);
}
#endif // OPTION_CUBIC_159 || OPTION_SQUARE_159 || OPTION_EIGEN_159

/********************************************//**
 * \brief Y(complex34) - X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void subCplxCplx(void) {
  addSubCplxCplx(real34SubtractOp);
}
