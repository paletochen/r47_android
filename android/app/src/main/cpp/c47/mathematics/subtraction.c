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



/********************************************//**
 * \brief Data type error in subtraction
 *
 * \param void
 * \return void
 ***********************************************/
#if (EXTRA_INFO_ON_CALC_ERROR == 1)
  void subError(void) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
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
  longInteger_t y, x;

  convertLongIntegerRegisterToLongInteger(REGISTER_Y, y);
  convertLongIntegerRegisterToLongInteger(REGISTER_X, x);

  longIntegerSubtract(y, x, x);

  convertLongIntegerToLongIntegerRegister(x, REGISTER_X);

  longIntegerFree(y);
  longIntegerFree(x);
}



/********************************************//**
 * \brief Y(long integer) - X(time) ==> X(time)
 *
 * \param void
 * \return void
 ***********************************************/
void subLonITime(void) {
  convertLongIntegerRegisterToTimeRegister(REGISTER_Y, REGISTER_Y);
  real34Subtract(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
}



/********************************************//**
 * \brief Y(time) - X(long integer) ==> X(time)
 *
 * \param void
 * \return void
 ***********************************************/
void subTimeLonI(void) {
  convertLongIntegerRegisterToTimeRegister(REGISTER_X, REGISTER_X);
  real34Subtract(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
}



/********************************************//**
 * \brief Y(date) - X(long integer) ==> X(date)
 *
 * \param void
 * \return void
 ***********************************************/
void subDateLonI(void) {
  real34_t val;
  convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
  int32ToReal34(86400, &val);
  real34Multiply(REGISTER_REAL34_DATA(REGISTER_X), &val, &val);
  reallocateRegister(REGISTER_X, dtDate, 0, amNone);
  real34Subtract(REGISTER_REAL34_DATA(REGISTER_Y), &val, REGISTER_REAL34_DATA(REGISTER_X));
}



/********************************************//**
 * \brief Y(long integer) - X(short integer) ==> X(long integer)
 *
 * \param void
 * \return void
 ***********************************************/
void subLonIShoI(void) {
  longInteger_t y, x;

  convertLongIntegerRegisterToLongInteger(REGISTER_Y, y);
  convertShortIntegerRegisterToLongInteger(REGISTER_X, x);

  longIntegerSubtract(y, x, x);

  convertLongIntegerToLongIntegerRegister(x, REGISTER_X);

  longIntegerFree(y);
  longIntegerFree(x);
}



/********************************************//**
 * \brief Y(short integer) - X(long integer) ==> X(long integer)
 *
 * \param void
 * \return void
 ***********************************************/
void subShoILonI(void) {
  longInteger_t y, x;

  convertShortIntegerRegisterToLongInteger(REGISTER_Y, y);
  convertLongIntegerRegisterToLongInteger(REGISTER_X, x);

  longIntegerSubtract(y, x, x);

  convertLongIntegerToLongIntegerRegister(x, REGISTER_X);

  longIntegerFree(y);
  longIntegerFree(x);
}



/********************************************//**
 * \brief Y(long integer) - X(real34) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void subLonIReal(void) {
  real_t y, x;
  angularMode_t xAngularMode;

  convertLongIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x);
  xAngularMode = getRegisterAngularMode(REGISTER_X);

  if(xAngularMode == amNone) {
    realSubtract(&y, &x, &x, &ctxtReal39);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
  }
  else {
    convertAngleFromTo(&x, xAngularMode, currentAngularMode, &ctxtReal39);
    realSubtract(&y, &x, &x, &ctxtReal39);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
    setRegisterAngularMode(REGISTER_X, currentAngularMode);
  }
}



/********************************************//**
 * \brief Y(real34) - X(long integer) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void subRealLonI(void) {
  real_t y, x;
  angularMode_t yAngularMode;

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y);
  yAngularMode = getRegisterAngularMode(REGISTER_Y);
  convertLongIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);

  if(yAngularMode == amNone) {
    realSubtract(&y, &x, &x, &ctxtReal39);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
  }
  else {
    convertAngleFromTo(&y, yAngularMode, currentAngularMode, &ctxtReal39);
    realSubtract(&y, &x, &x, &ctxtReal39);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
    setRegisterAngularMode(REGISTER_X, currentAngularMode);
  }
}



/********************************************//**
 * \brief Y(long integer) - X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void subLonICplx(void) {
  real_t a, c;

  convertLongIntegerRegisterToReal(REGISTER_Y, &a, &ctxtReal39);
  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &c);

  realSubtract(&a, &c, &c, &ctxtReal39);

  convertRealToReal34ResultRegister(&c, REGISTER_X);
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
  real34_t b;

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &a);
  real34Copy(REGISTER_IMAG34_DATA(REGISTER_Y), &b);
  convertLongIntegerRegisterToReal(REGISTER_X, &c, &ctxtReal39);

  realSubtract(&a, &c, &c, &ctxtReal39);

  reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
  convertRealToReal34ResultRegister(&c, REGISTER_X);
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
  int32ToReal34(86400, &val);
  real34Divide(REGISTER_REAL34_DATA(REGISTER_Y), &val, &val);
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
    int32ToReal34(86400, &val);
    real34ToIntegralValue(REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X), roundingMode);
    real34Multiply(REGISTER_REAL34_DATA(REGISTER_X), &val, &val);
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
  real_t y, x;

  linkToRealMatrixRegister(REGISTER_Y, &ym);

  const uint16_t rows = ym.header.matrixRows;
  const uint16_t cols = ym.header.matrixColumns;
  int32_t i;

  convertLongIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  for(i = 0; i < cols * rows; ++i) {
    real34ToReal(&ym.matrixElements[i], &y);
    realSubtract(&y, &x, &y, &ctxtReal39);
    roundToSignificantDigits(&y, &y, significantDigits == 0 ? 34 : significantDigits, &ctxtReal39);
    realToReal34(&y, &ym.matrixElements[i]);
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
  real_t y, x;

  linkToRealMatrixRegister(REGISTER_X, &xm);

  const uint16_t rows = xm.header.matrixRows;
  const uint16_t cols = xm.header.matrixColumns;
  int32_t i;

  convertLongIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  for(i = 0; i < cols * rows; ++i) {
    real34ToReal(&xm.matrixElements[i], &x);
    realSubtract(&y, &x, &x, &ctxtReal39);
    roundToSignificantDigits(&x, &x, significantDigits == 0 ? 34 : significantDigits, &ctxtReal39);
    realToReal34(&x, &xm.matrixElements[i]);
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

  subtractRealMatrices(&y, &x, &x);
  if(x.matrixElements) {
    convertReal34MatrixToReal34MatrixRegister(&x, REGISTER_X);
  }
  else {
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE, REGISTER_X);
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
  subCxmaCxma();
}



/********************************************//**
 * \brief Y(real34 matrix) - X(short integer) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subRemaShoI(void) {
  real34Matrix_t ym;
  real_t y, x;

  linkToRealMatrixRegister(REGISTER_Y, &ym);

  const uint16_t rows = ym.header.matrixRows;
  const uint16_t cols = ym.header.matrixColumns;
  int32_t i;

  convertShortIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  for(i = 0; i < cols * rows; ++i) {
    real34ToReal(&ym.matrixElements[i], &y);
    realSubtract(&y, &x, &y, &ctxtReal39);
    roundToSignificantDigits(&y, &y, significantDigits == 0 ? 34 : significantDigits, &ctxtReal39);
    realToReal34(&y, &ym.matrixElements[i]);
  }
  fnSwapXY(NOPARAM);
}



/********************************************//**
 * \brief Y(short integer) - X(real34 matrix) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subShoIRema(void) {
  real34Matrix_t xm;
  real_t y, x;

  linkToRealMatrixRegister(REGISTER_X, &xm);

  const uint16_t rows = xm.header.matrixRows;
  const uint16_t cols = xm.header.matrixColumns;
  int32_t i;

  convertShortIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  for(i = 0; i < cols * rows; ++i) {
    real34ToReal(&xm.matrixElements[i], &x);
    realSubtract(&y, &x, &x, &ctxtReal39);
    roundToSignificantDigits(&x, &x, significantDigits == 0 ? 34 : significantDigits, &ctxtReal39);
    realToReal34(&x, &xm.matrixElements[i]);
  }
}



/********************************************//**
 * \brief Y(real34 matrix) + X(real34) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subRemaReal(void) {
  real34Matrix_t y;
  angularMode_t xAngularMode;

  linkToRealMatrixRegister(REGISTER_Y, &y);
  xAngularMode = getRegisterAngularMode(REGISTER_X);

  if(xAngularMode == amNone) {
    const uint16_t rows = y.header.matrixRows;
    const uint16_t cols = y.header.matrixColumns;
    int32_t i;

    for(i = 0; i < cols * rows; ++i) {
      real34Subtract(&y.matrixElements[i], REGISTER_REAL34_DATA(REGISTER_X), &y.matrixElements[i]);
    }
    fnSwapXY(NOPARAM);
  }
  else {
    elementwiseRemaReal(subRealReal);
  }
}



/********************************************//**
 * \brief Y(real34) - X(real34 matrix) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subRealRema(void) {
  real34Matrix_t x;
  angularMode_t yAngularMode;

  linkToRealMatrixRegister(REGISTER_X, &x);
  yAngularMode = getRegisterAngularMode(REGISTER_Y);

  if(yAngularMode == amNone) {
    const uint16_t rows = x.header.matrixRows;
    const uint16_t cols = x.header.matrixColumns;
    int32_t i;

    for(i = 0; i < cols * rows; ++i) {
      real34Subtract(REGISTER_REAL34_DATA(REGISTER_Y), &x.matrixElements[i], &x.matrixElements[i]);
    }
  }
  else {
    elementwiseRealRema(subRealReal);
  }
}



/********************************************//**
 * \brief Y(real34 matrix) - X(complex34) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subRemaCplx(void) {
  convertReal34MatrixRegisterToComplex34MatrixRegister(REGISTER_Y, REGISTER_Y);
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
  real_t y, x;

  linkToComplexMatrixRegister(REGISTER_Y, &ym);

  const uint16_t rows = ym.header.matrixRows;
  const uint16_t cols = ym.header.matrixColumns;
  int32_t i;

  convertLongIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  for(i = 0; i < cols * rows; ++i) {
    real34ToReal(VARIABLE_REAL34_DATA(&ym.matrixElements[i]), &y);
    realSubtract(&y, &x, &y, &ctxtReal39);
    roundToSignificantDigits(&y, &y, significantDigits == 0 ? 34 : significantDigits, &ctxtReal39);
    realToReal34(&y, VARIABLE_REAL34_DATA(&ym.matrixElements[i]));
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
  real_t y, x;

  linkToComplexMatrixRegister(REGISTER_X, &xm);

  const uint16_t rows = xm.header.matrixRows;
  const uint16_t cols = xm.header.matrixColumns;
  int32_t i;

  convertLongIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  for(i = 0; i < cols * rows; ++i) {
    real34ToReal(VARIABLE_REAL34_DATA(&xm.matrixElements[i]), &x);
    realSubtract(&y, &x, &x, &ctxtReal39);
    roundToSignificantDigits(&x, &x, significantDigits == 0 ? 34 : significantDigits, &ctxtReal39);
    realToReal34(&x, VARIABLE_REAL34_DATA(&xm.matrixElements[i]));
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

  subtractComplexMatrices(&y, &x, &x);
  if(x.matrixElements) {
    convertComplex34MatrixToComplex34MatrixRegister(&x, REGISTER_X);
  }
  else {
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE, REGISTER_X);
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
  complex34Matrix_t ym;
  real_t y, x;

  linkToComplexMatrixRegister(REGISTER_Y, &ym);

  const uint16_t rows = ym.header.matrixRows;
  const uint16_t cols = ym.header.matrixColumns;
  int32_t i;

  convertShortIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  for(i = 0; i < cols * rows; ++i) {
    real34ToReal(VARIABLE_REAL34_DATA(&ym.matrixElements[i]), &y);
    realSubtract(&y, &x, &y, &ctxtReal39);
    roundToSignificantDigits(&y, &y, significantDigits == 0 ? 34 : significantDigits, &ctxtReal39);
    realToReal34(&y, VARIABLE_REAL34_DATA(&ym.matrixElements[i]));
  }
  fnSwapXY(NOPARAM);
}



/********************************************//**
 * \brief Y(short integer) - X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subShoICxma(void) {
  complex34Matrix_t xm;
  real_t y, x;

  linkToComplexMatrixRegister(REGISTER_X, &xm);

  const uint16_t rows = xm.header.matrixRows;
  const uint16_t cols = xm.header.matrixColumns;
  int32_t i;

  convertShortIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  for(i = 0; i < cols * rows; ++i) {
    real34ToReal(VARIABLE_REAL34_DATA(&xm.matrixElements[i]), &x);
    realSubtract(&y, &x, &x, &ctxtReal39);
    roundToSignificantDigits(&x, &x, significantDigits == 0 ? 34 : significantDigits, &ctxtReal39);
    realToReal34(&x, VARIABLE_REAL34_DATA(&xm.matrixElements[i]));
    real34ChangeSign(VARIABLE_IMAG34_DATA(&xm.matrixElements[i]));
  }
}



/********************************************//**
 * \brief Y(complex34 matrix) - X(real34) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subCxmaReal(void) {
  complex34Matrix_t y;
  angularMode_t xAngularMode;

  linkToComplexMatrixRegister(REGISTER_Y, &y);
  xAngularMode = getRegisterAngularMode(REGISTER_X);

  if(xAngularMode == amNone) {
    const uint16_t rows = y.header.matrixRows;
    const uint16_t cols = y.header.matrixColumns;
    int32_t i;

    for(i = 0; i < cols * rows; ++i) {
      real34Subtract(VARIABLE_REAL34_DATA(&y.matrixElements[i]), REGISTER_REAL34_DATA(REGISTER_X), VARIABLE_REAL34_DATA(&y.matrixElements[i]));
    }
    fnSwapXY(NOPARAM);
  }
  else {
    elementwiseCxmaReal(subCplxReal);
  }
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
  complex34Matrix_t y;

  linkToComplexMatrixRegister(REGISTER_Y, &y);

  const uint16_t rows = y.header.matrixRows;
  const uint16_t cols = y.header.matrixColumns;
  int32_t i;

  for(i = 0; i < cols * rows; ++i) {
    real34Subtract(VARIABLE_REAL34_DATA(&y.matrixElements[i]), REGISTER_REAL34_DATA(REGISTER_X), VARIABLE_REAL34_DATA(&y.matrixElements[i]));
    real34Subtract(VARIABLE_IMAG34_DATA(&y.matrixElements[i]), REGISTER_IMAG34_DATA(REGISTER_X), VARIABLE_IMAG34_DATA(&y.matrixElements[i]));
  }
  fnSwapXY(NOPARAM);
}



/********************************************//**
 * \brief Y(complex34) - X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void subCplxCxma(void) {
  complex34Matrix_t x;

  linkToComplexMatrixRegister(REGISTER_X, &x);

  const uint16_t rows = x.header.matrixRows;
  const uint16_t cols = x.header.matrixColumns;
  int32_t i;

  for(i = 0; i < cols * rows; ++i) {
    real34Subtract(REGISTER_REAL34_DATA(REGISTER_Y), VARIABLE_REAL34_DATA(&x.matrixElements[i]), VARIABLE_REAL34_DATA(&x.matrixElements[i]));
    real34Subtract(REGISTER_IMAG34_DATA(REGISTER_Y), VARIABLE_IMAG34_DATA(&x.matrixElements[i]), VARIABLE_IMAG34_DATA(&x.matrixElements[i]));
  }
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
  real_t y, x;
  angularMode_t xAngularMode = getRegisterAngularMode(REGISTER_X);

  convertShortIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x);

  if(xAngularMode == amNone) {
    realSubtract(&y, &x, &x, &ctxtReal39);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
  }
  else {
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
  real_t y, x;
  angularMode_t yAngularMode = getRegisterAngularMode(REGISTER_Y);

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y);
  convertShortIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);

  if(yAngularMode == amNone) {
    realSubtract(&y, &x, &x, &ctxtReal39);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
  }
  else {
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
  convertShortIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
  real34Subtract(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_Y)); // real part
  reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
  complex34Copy(REGISTER_COMPLEX34_DATA(REGISTER_Y), REGISTER_COMPLEX34_DATA(REGISTER_X));
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
  reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
  complex34Copy(REGISTER_COMPLEX34_DATA(REGISTER_Y), REGISTER_COMPLEX34_DATA(REGISTER_X));
}



/******************************************************************************************************************************************************************************************/
/* complex34 - ...                                                                                                                                                                        */
/******************************************************************************************************************************************************************************************/
void subComplex(const real_t *aReal, const real_t *aImag, const real_t *bReal, const real_t *bImag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  realSubtract(aReal, bReal, resReal, realContext);
  realSubtract(aImag, bImag, resImag, realContext);
}

/********************************************//**
 * \brief Y(complex34) - X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void subCplxCplx(void) {
  real34Subtract(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X)); // real part
  real34Subtract(REGISTER_IMAG34_DATA(REGISTER_Y), REGISTER_IMAG34_DATA(REGISTER_X), REGISTER_IMAG34_DATA(REGISTER_X)); // imaginary part
}
