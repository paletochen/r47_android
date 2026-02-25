// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file addition.c
 ***********************************************/

#include "c47.h"

TO_QSPI void (* const addition[NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS][NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS])(void) = {
// regX |    regY ==>   1            2            3            4            5            6            7            8            9             10
//      V               Long integer Real34       Complex34    Time         Date         String       Real34 mat   Complex34 m  Short integer Config data
/*  1 Long integer  */ {addLonILonI, addRealLonI, addCplxLonI, addTimeLonI, addDateLonI, addStriLonI, addRemaLonI, addCxmaLonI, addShoILonI,  addError},
/*  2 Real34        */ {addLonIReal, addRealReal, addCplxReal, addTimeReal, addDateReal, addStriReal, addRemaReal, addCxmaReal, addShoIReal,  addError},
/*  3 Complex34     */ {addLonICplx, addRealCplx, addCplxCplx, addError,    addError,    addStriCplx, addRemaCplx, addCxmaCplx, addShoICplx,  addError},
/*  4 Time          */ {addLonITime, addRealTime, addError,    addTimeTime, addError,    addStriTime, addError,    addError,    addError,     addError},
/*  5 Date          */ {addLonIDate, addRealDate, addError,    addError,    addError,    addStriDate, addError,    addError,    addError,     addError},
/*  6 String        */ {addRegYStri, addRegYStri, addRegYStri, addRegYStri, addRegYStri, addStriStri, addRegYStri, addRegYStri, addRegYStri,  addError},
/*  7 Real34 mat    */ {addLonIRema, addRealRema, addCplxRema, addError,    addError,    addStriRema, addRemaRema, addCxmaRema, addShoIRema,  addError},
/*  8 Complex34 mat */ {addLonICxma, addRealCxma, addCplxCxma, addError,    addError,    addStriCxma, addRemaCxma, addCxmaCxma, addShoICxma,  addError},
/*  9 Short integer */ {addLonIShoI, addRealShoI, addCplxShoI, addError,    addError,    addStriShoI, addRemaShoI, addCxmaShoI, addShoIShoI,  addError},
/* 10 Config data   */ {addError,    addError,    addError,    addError,    addError,    addError,    addError,    addError,    addError,     addError}
};



/********************************************//**
 * \brief Data type error in addition
 *
 * \param void
 * \return void
 ***********************************************/
#if (EXTRA_INFO_ON_CALC_ERROR == 1)
  void addError(void) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
    sprintf(errorMessage, "cannot add %s", getRegisterDataTypeName(REGISTER_X, true, false));
    sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "to %s", getRegisterDataTypeName(REGISTER_Y, true, false));
    moreInfoOnError("In function fnAdd:", errorMessage, errorMessage + ERROR_MESSAGE_LENGTH/2, NULL);
  }
#endif // (EXTRA_INFO_ON_CALC_ERROR == 1)



/********************************************//**
 * \brief regX ==> regL and regY + regX ==> regX
 * Drops Y, enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnAdd(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }

  addition[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)]();

  adjustResult(REGISTER_X, true, true, REGISTER_X, REGISTER_Y, -1);
}





void addRegYStri(void) {
  copySourceRegisterToDestRegister(REGISTER_X, TEMP_REGISTER_1);
  copySourceRegisterToDestRegister(REGISTER_Y, REGISTER_X);

  char tmp[2];
  tmp[0]=0;
  int16_t len = stringByteLength(tmp) + 1;
  reallocateRegister(REGISTER_Y, dtString, TO_BLOCKS(len), amNone);
  xcopy(REGISTER_STRING_DATA(REGISTER_Y), tmp, len);

  addition[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)]();

  copySourceRegisterToDestRegister(REGISTER_X, REGISTER_Y);
  copySourceRegisterToDestRegister(TEMP_REGISTER_1, REGISTER_X);

  addition[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)]();

}




/******************************************************************************************************************************************************************************************/
/* long integer + ...                                                                                                                                                                     */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(long integer) + X(long integer) ==> X(long integer)
 *
 * \param void
 * \return void
 ***********************************************/
void addLonILonI(void) {
  longInteger_t y, x;

  convertLongIntegerRegisterToLongInteger(REGISTER_Y, y);
  convertLongIntegerRegisterToLongInteger(REGISTER_X, x);

  longIntegerAdd(y, x, x);

  convertLongIntegerToLongIntegerRegister(x, REGISTER_X);

  longIntegerFree(y);
  longIntegerFree(x);
}



/********************************************//**
 * \brief Y(long integer) + X(time) ==> X(time)
 *
 * \param void
 * \return void
 ***********************************************/
void addLonITime(void) {
  convertLongIntegerRegisterToTimeRegister(REGISTER_Y, REGISTER_Y);
  real34Add(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
}



/********************************************//**
 * \brief Y(time) + X(long integer) ==> X(time)
 *
 * \param void
 * \return void
 ***********************************************/
void addTimeLonI(void) {
  convertLongIntegerRegisterToTimeRegister(REGISTER_X, REGISTER_X);
  real34Add(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
}



/********************************************//**
 * \brief Y(long integer) + X(date) ==> X(date)
 *
 * \param void
 * \return void
 ***********************************************/
void addLonIDate(void) {
  real34_t val;
  convertLongIntegerRegisterToReal34Register(REGISTER_Y, REGISTER_Y);
  int32ToReal34(86400, &val);
  real34Multiply(REGISTER_REAL34_DATA(REGISTER_Y), &val, &val);
  real34Add(&val, REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
}



/********************************************//**
 * \brief Y(date) + X(long integer) ==> X(date)
 *
 * \param void
 * \return void
 ***********************************************/
void addDateLonI(void) {
  real34_t val;
  convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
  int32ToReal34(86400, &val);
  real34Multiply(REGISTER_REAL34_DATA(REGISTER_X), &val, &val);
  reallocateRegister(REGISTER_X, dtDate, 0, amNone);
  real34Add(REGISTER_REAL34_DATA(REGISTER_Y), &val, REGISTER_REAL34_DATA(REGISTER_X));
}



/********************************************//**
 * \brief Y(long integer) + X(short integer) ==> X(long integer)
 *
 * \param void
 * \return void
 ***********************************************/
void addLonIShoI(void) {
  longInteger_t y, x;

  convertLongIntegerRegisterToLongInteger(REGISTER_Y, y);
  convertShortIntegerRegisterToLongInteger(REGISTER_X, x);

  longIntegerAdd(y, x, x);

  convertLongIntegerToLongIntegerRegister(x, REGISTER_X);

  longIntegerFree(y);
  longIntegerFree(x);
}



/********************************************//**
 * \brief Y(short integer) + X(long integer) ==> X(long integer)
 *
 * \param void
 * \return void
 ***********************************************/
void addShoILonI(void) {
  longInteger_t y, x;

  convertShortIntegerRegisterToLongInteger(REGISTER_Y, y);
  convertLongIntegerRegisterToLongInteger(REGISTER_X, x);

  longIntegerAdd(y, x, x);

  convertLongIntegerToLongIntegerRegister(x, REGISTER_X);

  longIntegerFree(y);
  longIntegerFree(x);
}



/********************************************//**
 * \brief Y(long integer) + X(real34) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void addLonIReal(void) {
  real_t y, x;
  angularMode_t xAngularMode;

  convertLongIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x);
  xAngularMode = getRegisterAngularMode(REGISTER_X);

  if(xAngularMode == amNone) {
    realAdd(&y, &x, &x, &ctxtReal39);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
  }
  else {
    convertAngleFromTo(&x, xAngularMode, currentAngularMode, &ctxtReal39);
    realAdd(&y, &x, &x, &ctxtReal39);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
    setRegisterAngularMode(REGISTER_X, currentAngularMode);
  }
}



/********************************************//**
 * \brief Y(real34) + X(long integer) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void addRealLonI(void) {
  real_t y, x;
  angularMode_t yAngularMode;

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y);
  yAngularMode = getRegisterAngularMode(REGISTER_Y);
  convertLongIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);

  if(yAngularMode == amNone) {
    realAdd(&y, &x, &x, &ctxtReal39);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
  }
  else {
    convertAngleFromTo(&y, yAngularMode, currentAngularMode, &ctxtReal39);
    realAdd(&y, &x, &x, &ctxtReal39);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
    setRegisterAngularMode(REGISTER_X, currentAngularMode);
  }
}



/********************************************//**
 * \brief Y(long integer) + X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void addLonICplx(void) {
  real_t a, c;

  convertLongIntegerRegisterToReal(REGISTER_Y, &a, &ctxtReal39);
  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &c);

  realAdd(&a, &c, &c, &ctxtReal39);

  convertRealToReal34ResultRegister(&c, REGISTER_X);
}



/********************************************//**
 * \brief Y(complex34) + X(long integer) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void addCplxLonI(void) {
  real_t a, c;
  real34_t b;

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &a);
  real34Copy(REGISTER_IMAG34_DATA(REGISTER_Y), &b);
  convertLongIntegerRegisterToReal(REGISTER_X, &c, &ctxtReal39);

  realAdd(&a, &c, &c, &ctxtReal39);

  reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
  convertRealToReal34ResultRegister(&c, REGISTER_X);
  real34Copy(&b, REGISTER_IMAG34_DATA(REGISTER_X));
}



/******************************************************************************************************************************************************************************************/
/* time + ...                                                                                                                                                                             */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(time) + X(time) ==> X(time)
 *
 * \param void
 * \return void
 ***********************************************/
void addTimeTime(void) {
  real34Add(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
}



/********************************************//**
 * \brief Y(time) + X(real34) ==> X(time)
 *
 * \param void
 * \return void
 ***********************************************/
void addTimeReal(void) {
  angularMode_t xAngularMode;

  xAngularMode = getRegisterAngularMode(REGISTER_X);

  if(xAngularMode == amNone) {
    convertReal34RegisterToTimeRegister(REGISTER_X, REGISTER_X);
    real34Add(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
  }
  else {
    addError();
  }
}



/********************************************//**
 * \brief Y(real34) + X(time) ==> X(time)
 *
 * \param void
 * \return void
 ***********************************************/
void addRealTime(void) {
  angularMode_t yAngularMode;

  yAngularMode = getRegisterAngularMode(REGISTER_Y);

  if(yAngularMode == amNone) {
    convertReal34RegisterToTimeRegister(REGISTER_Y, REGISTER_Y);
    real34Add(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
  }
  else {
    addError();
  }
}



/******************************************************************************************************************************************************************************************/
/* date + ...                                                                                                                                                                             */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(date) + X(real34) ==> X(date)
 *
 * \param void
 * \return void
 ***********************************************/
void addDateReal(void) {
  angularMode_t xAngularMode;
  real34_t val;

  xAngularMode = getRegisterAngularMode(REGISTER_X);

  if(xAngularMode == amNone) {
    int32ToReal34(86400, &val);
    real34ToIntegralValue(REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X), roundingMode);
    real34Multiply(REGISTER_REAL34_DATA(REGISTER_X), &val, &val);
    reallocateRegister(REGISTER_X, dtDate, 0, amNone);
    real34Add(REGISTER_REAL34_DATA(REGISTER_Y), &val, REGISTER_REAL34_DATA(REGISTER_X));
  }
  else {
    addError();
  }
}



/********************************************//**
 * \brief Y(real34) + X(date) ==> X(date)
 *
 * \param void
 * \return void
 ***********************************************/
void addRealDate(void) {
  angularMode_t yAngularMode;
  real34_t val;

  yAngularMode = getRegisterAngularMode(REGISTER_Y);

  if(yAngularMode == amNone) {
    int32ToReal34(86400, &val);
    real34ToIntegralValue(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_Y), roundingMode);
    real34Multiply(REGISTER_REAL34_DATA(REGISTER_Y), &val, &val);
    real34Add(&val, REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
  }
  else {
    addError();
  }
}



/******************************************************************************************************************************************************************************************/
/* string + ...                                                                                                                                                                           */
/******************************************************************************************************************************************************************************************/

static void _addString(const char *stringToAppend) {
  int16_t len1, len2;

  if(stringGlyphLength(REGISTER_STRING_DATA(REGISTER_Y)) + stringGlyphLength(stringToAppend) > MAX_NUMBER_OF_GLYPHS_IN_STRING) {
    displayCalcErrorMessage(ERROR_STRING_WOULD_BE_TOO_LONG, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "the resulting string would be %d (Y %d + X %d) characters long. Maximum is %d",
                                                           stringGlyphLength(REGISTER_STRING_DATA(REGISTER_Y)) + stringGlyphLength(stringToAppend),
                                                                 stringGlyphLength(REGISTER_STRING_DATA(REGISTER_Y)),
                                                                        stringGlyphLength(stringToAppend),  MAX_NUMBER_OF_GLYPHS_IN_STRING);
      moreInfoOnError("In function _addString:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    len1 = stringByteLength(REGISTER_STRING_DATA(REGISTER_Y));
    len2 = stringByteLength(stringToAppend) + 1;

    reallocateRegister(REGISTER_X, dtString, TO_BLOCKS(len1 + len2), amNone);

    xcopy(REGISTER_STRING_DATA(REGISTER_X),        REGISTER_STRING_DATA(REGISTER_Y), len1);
    xcopy(REGISTER_STRING_DATA(REGISTER_X) + len1, stringToAppend,                        len2);
  }
}

/********************************************//**
 * \brief Y(string) + X(long integer) ==> X(string)
 *
 * \param void
 * \return void
 ***********************************************/
void addStriLonI(void) {
  longIntegerRegisterToDisplayString(REGISTER_X, tmpString, TMP_STR_LENGTH, SCREEN_WIDTH, 50, false);
  _addString(tmpString);
}



/********************************************//**
 * \brief Y(string) + X(time) ==> X(string)
 *
 * \param void
 * \return void
 ***********************************************/
void addStriTime(void) {
  timeToDisplayString(REGISTER_X, tmpString, false);
  _addString(tmpString);
}



/********************************************//**
 * \brief Y(string) + X(date) ==> X(string)
 *
 * \param void
 * \return void
 ***********************************************/
void addStriDate(void) {
  dateToDisplayString(REGISTER_X, tmpString);
  _addString(tmpString);
}



/********************************************//**
 * \brief Y(string) + X(string) ==> X(string)
 *
 * \param void
 * \return void
 ***********************************************/
void addStriStri(void) {
  xcopy( tmpString, REGISTER_STRING_DATA(REGISTER_X),
    (REGISTER_STRING_DATA(REGISTER_X) && REGISTER_STRING_DATA(REGISTER_X)[0] != '\0') ? stringByteLength(REGISTER_STRING_DATA(REGISTER_X)) + 1 : 1
  );
  _addString(tmpString);
}



/********************************************//**
 * \brief Y(string) + X(real34 matrix) ==> X(string)
 *
 * \param void
 * \return void
 ***********************************************/
void addStriRema(void) {
  real34MatrixToDisplayString(REGISTER_X, tmpString);
  _addString(tmpString);
}



/********************************************//**
 * \brief Y(string) + X(complex34 matrix) ==> X(string)
 *
 * \param void
 * \return void
 ***********************************************/
void addStriCxma(void) {
  complex34MatrixToDisplayString(REGISTER_X, tmpString);
  _addString(tmpString);
}



/********************************************//**
 * \brief Y(string) + X(short integer) ==> X(string)
 *
 * \param void
 * \return void
 ***********************************************/
void addStriShoI(void) {
  shortIntegerToDisplayString(REGISTER_X, tmpString, false, noBaseOverride);
  _addString(tmpString);
}



/********************************************//**
 * \brief Y(string) + X(real34) ==> X(string)
 *
 * \param void
 * \return void
 ***********************************************/
void addStriReal(void) {
  if(getSystemFlag(FLAG_FRACT)) {
    fractionToDisplayString(REGISTER_X, tmpString);
  }
  else {
    real34ToDisplayString(REGISTER_REAL34_DATA(REGISTER_X), getRegisterAngularMode(REGISTER_X), tmpString, &standardFont, SCREEN_WIDTH, NUMBER_OF_DISPLAY_DIGITS, !LIMITEXP, FRONTSPACE, FULLIRFRAC);
  }
  _addString(tmpString);
}



/********************************************//**
 * \brief Y(string) + X(complex34) ==> X(string)
 *
 * \param void
 * \return void
 ***********************************************/
void addStriCplx(void) {
  complex34ToDisplayString(REGISTER_COMPLEX34_DATA(REGISTER_X), tmpString, &numericFont, SCREEN_WIDTH, NUMBER_OF_DISPLAY_DIGITS, !LIMITEXP, FRONTSPACE, FULLIRFRAC, currentAngularMode, getSystemFlag(FLAG_POLAR));
  _addString(tmpString);
}



/******************************************************************************************************************************************************************************************/
/* real34 matrix + ...                                                                                                                                                                    */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(real34 matrix) + X(long integer) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void addRemaLonI(void) {
  real34Matrix_t ym;
  real_t y, x;

  linkToRealMatrixRegister(REGISTER_Y, &ym);

  const uint16_t rows = ym.header.matrixRows;
  const uint16_t cols = ym.header.matrixColumns;
  int32_t i;

  convertLongIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  for(i = 0; i < cols * rows; ++i) {
    real34ToReal(&ym.matrixElements[i], &y);
    realAdd(&y, &x, &y, &ctxtReal39);
    roundToSignificantDigits(&y, &y, significantDigits == 0 ? 34 : significantDigits, &ctxtReal39);
    realToReal34(&y, &ym.matrixElements[i]);
  }
  fnSwapXY(NOPARAM);
}



/********************************************//**
 * \brief Y(long integer) + X(real34 matrix) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void addLonIRema(void) {
  real34Matrix_t xm;
  real_t y, x;

  linkToRealMatrixRegister(REGISTER_X, &xm);

  const uint16_t rows = xm.header.matrixRows;
  const uint16_t cols = xm.header.matrixColumns;
  int32_t i;

  convertLongIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  for(i = 0; i < cols * rows; ++i) {
    real34ToReal(&xm.matrixElements[i], &x);
    realAdd(&y, &x, &x, &ctxtReal39);
    roundToSignificantDigits(&x, &x, significantDigits == 0 ? 34 : significantDigits, &ctxtReal39);
    realToReal34(&x, &xm.matrixElements[i]);
  }
}



/********************************************//**
 * \brief Y(real34 matrix) + X(real34 matrix) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void addRemaRema(void) {
  real34Matrix_t y, x;

  linkToRealMatrixRegister(REGISTER_Y, &y);
  convertReal34MatrixRegisterToReal34Matrix(REGISTER_X, &x);

  addRealMatrices(&y, &x, &x);
  if(x.matrixElements) {
    convertReal34MatrixToReal34MatrixRegister(&x, REGISTER_X);
  }
  else {
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot add %d" STD_CROSS "%d-matrix to %d" STD_CROSS "%d-matrix",
              x.header.matrixRows, x.header.matrixColumns,
              y.header.matrixRows, y.header.matrixColumns);
      moreInfoOnError("In function addRemaRema:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  realMatrixFree(&x);
}



/********************************************//**
 * \brief Y(real34 matrix) + X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void addRemaCxma(void) {
  convertReal34MatrixRegisterToComplex34MatrixRegister(REGISTER_Y, REGISTER_Y);
  addCxmaCxma();
}



/********************************************//**
 * \brief Y(complex34 matrix) + X(real34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void addCxmaRema(void) {
  convertReal34MatrixRegisterToComplex34MatrixRegister(REGISTER_X, REGISTER_X);
  addCxmaCxma();
}



/********************************************//**
 * \brief Y(real34 matrix) + X(short integer) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void addRemaShoI(void) {
  real34Matrix_t ym;
  real_t y, x;

  linkToRealMatrixRegister(REGISTER_Y, &ym);

  const uint16_t rows = ym.header.matrixRows;
  const uint16_t cols = ym.header.matrixColumns;
  int32_t i;

  convertShortIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  for(i = 0; i < cols * rows; ++i) {
    real34ToReal(&ym.matrixElements[i], &y);
    realAdd(&y, &x, &y, &ctxtReal39);
    roundToSignificantDigits(&y, &y, significantDigits == 0 ? 34 : significantDigits, &ctxtReal39);
    realToReal34(&y, &ym.matrixElements[i]);
  }
  fnSwapXY(NOPARAM);
}



/********************************************//**
 * \brief Y(short integer) + X(real34 matrix) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void addShoIRema(void) {
  real34Matrix_t xm;
  real_t y, x;

  linkToRealMatrixRegister(REGISTER_X, &xm);

  const uint16_t rows = xm.header.matrixRows;
  const uint16_t cols = xm.header.matrixColumns;
  int32_t i;

  convertShortIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  for(i = 0; i < cols * rows; ++i) {
    real34ToReal(&xm.matrixElements[i], &x);
    realAdd(&y, &x, &x, &ctxtReal39);
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
void addRemaReal(void) {
  real34Matrix_t y;
  angularMode_t xAngularMode;

  linkToRealMatrixRegister(REGISTER_Y, &y);
  xAngularMode = getRegisterAngularMode(REGISTER_X);

  if(xAngularMode == amNone) {
    const uint16_t rows = y.header.matrixRows;
    const uint16_t cols = y.header.matrixColumns;
    int32_t i;

    for(i = 0; i < cols * rows; ++i) {
      real34Add(&y.matrixElements[i], REGISTER_REAL34_DATA(REGISTER_X), &y.matrixElements[i]);
    }
    fnSwapXY(NOPARAM);
  }
  else {
    elementwiseRemaReal(addRealReal);
  }
}



/********************************************//**
 * \brief Y(real34) + X(real34 matrix) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void addRealRema(void) {
  real34Matrix_t x;
  angularMode_t yAngularMode;

  linkToRealMatrixRegister(REGISTER_X, &x);
  yAngularMode = getRegisterAngularMode(REGISTER_Y);

  if(yAngularMode == amNone) {
    const uint16_t rows = x.header.matrixRows;
    const uint16_t cols = x.header.matrixColumns;
    int32_t i;

    for(i = 0; i < cols * rows; ++i) {
      real34Add(REGISTER_REAL34_DATA(REGISTER_Y), &x.matrixElements[i], &x.matrixElements[i]);
    }
  }
  else {
    elementwiseRealRema(addRealReal);
  }
}



/********************************************//**
 * \brief Y(real34 matrix) + X(complex34) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void addRemaCplx(void) {
  convertReal34MatrixRegisterToComplex34MatrixRegister(REGISTER_Y, REGISTER_Y);
  addCxmaCplx();
}



/********************************************//**
 * \brief Y(complex34) + X(real34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void addCplxRema(void) {
  convertReal34MatrixRegisterToComplex34MatrixRegister(REGISTER_X, REGISTER_X);
  addCplxCxma();
}



/******************************************************************************************************************************************************************************************/
/* complex34 matrix + ...                                                                                                                                                                 */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(complex34 matrix) + X(long integer) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void addCxmaLonI(void) {
  complex34Matrix_t ym;
  real_t y, x;

  linkToComplexMatrixRegister(REGISTER_Y, &ym);

  const uint16_t rows = ym.header.matrixRows;
  const uint16_t cols = ym.header.matrixColumns;
  int32_t i;

  convertLongIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  for(i = 0; i < cols * rows; ++i) {
    real34ToReal(VARIABLE_REAL34_DATA(&ym.matrixElements[i]), &y);
    realAdd(&y, &x, &y, &ctxtReal39);
    roundToSignificantDigits(&y, &y, significantDigits == 0 ? 34 : significantDigits, &ctxtReal39);
    realToReal34(&y, VARIABLE_REAL34_DATA(&ym.matrixElements[i]));
  }
  fnSwapXY(NOPARAM);
}



/********************************************//**
 * \brief Y(long integer) + X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void addLonICxma(void) {
  complex34Matrix_t xm;
  real_t y, x;

  linkToComplexMatrixRegister(REGISTER_X, &xm);

  const uint16_t rows = xm.header.matrixRows;
  const uint16_t cols = xm.header.matrixColumns;
  int32_t i;

  convertLongIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  for(i = 0; i < cols * rows; ++i) {
    real34ToReal(VARIABLE_REAL34_DATA(&xm.matrixElements[i]), &x);
    realAdd(&y, &x, &x, &ctxtReal39);
    roundToSignificantDigits(&x, &x, significantDigits == 0 ? 34 : significantDigits, &ctxtReal39);
    realToReal34(&x, VARIABLE_REAL34_DATA(&xm.matrixElements[i]));
  }
}



/********************************************//**
 * \brief Y(complex34 matrix) + X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void addCxmaCxma(void) {
  complex34Matrix_t y, x;

  linkToComplexMatrixRegister(REGISTER_Y, &y);
  convertComplex34MatrixRegisterToComplex34Matrix(REGISTER_X, &x);

  addComplexMatrices(&y, &x, &x);
  if(x.matrixElements) {
    convertComplex34MatrixToComplex34MatrixRegister(&x, REGISTER_X);
  }
  else {
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot add %d" STD_CROSS "%d-matrix to %d" STD_CROSS "%d-matrix",
              x.header.matrixRows, x.header.matrixColumns,
              y.header.matrixRows, y.header.matrixColumns);
      moreInfoOnError("In function addCxmaCxma:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  complexMatrixFree(&x);
}



/********************************************//**
 * \brief Y(complex34 matrix) + X(short integer) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void addCxmaShoI(void) {
  complex34Matrix_t ym;
  real_t y, x;

  linkToComplexMatrixRegister(REGISTER_Y, &ym);

  const uint16_t rows = ym.header.matrixRows;
  const uint16_t cols = ym.header.matrixColumns;
  int32_t i;

  convertShortIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  for(i = 0; i < cols * rows; ++i) {
    real34ToReal(VARIABLE_REAL34_DATA(&ym.matrixElements[i]), &y);
    realAdd(&y, &x, &y, &ctxtReal39);
    roundToSignificantDigits(&y, &y, significantDigits == 0 ? 34 : significantDigits, &ctxtReal39);
    realToReal34(&y, VARIABLE_REAL34_DATA(&ym.matrixElements[i]));
  }
  fnSwapXY(NOPARAM);
}



/********************************************//**
 * \brief Y(short integer) + X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void addShoICxma(void) {
  complex34Matrix_t xm;
  real_t y, x;

  linkToComplexMatrixRegister(REGISTER_X, &xm);

  const uint16_t rows = xm.header.matrixRows;
  const uint16_t cols = xm.header.matrixColumns;
  int32_t i;

  convertShortIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  for(i = 0; i < cols * rows; ++i) {
    real34ToReal(VARIABLE_REAL34_DATA(&xm.matrixElements[i]), &x);
    realAdd(&y, &x, &x, &ctxtReal39);
    roundToSignificantDigits(&x, &x, significantDigits == 0 ? 34 : significantDigits, &ctxtReal39);
    realToReal34(&x, VARIABLE_REAL34_DATA(&xm.matrixElements[i]));
  }
}



/********************************************//**
 * \brief Y(complex34 matrix) + X(real34) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void addCxmaReal(void) {
  complex34Matrix_t y;
  angularMode_t xAngularMode;

  linkToComplexMatrixRegister(REGISTER_Y, &y);
  xAngularMode = getRegisterAngularMode(REGISTER_X);

  if(xAngularMode == amNone) {
    const uint16_t rows = y.header.matrixRows;
    const uint16_t cols = y.header.matrixColumns;
    int32_t i;

    for(i = 0; i < cols * rows; ++i) {
      real34Add(VARIABLE_REAL34_DATA(&y.matrixElements[i]), REGISTER_REAL34_DATA(REGISTER_X), VARIABLE_REAL34_DATA(&y.matrixElements[i]));
    }
    fnSwapXY(NOPARAM);
  }
  else {
    elementwiseCxmaReal(addCplxReal);
  }
}



/********************************************//**
 * \brief Y(real34) + X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void addRealCxma(void) {
  complex34Matrix_t x;
  angularMode_t yAngularMode;

  linkToComplexMatrixRegister(REGISTER_X, &x);
  yAngularMode = getRegisterAngularMode(REGISTER_Y);

  if(yAngularMode == amNone) {
    const uint16_t rows = x.header.matrixRows;
    const uint16_t cols = x.header.matrixColumns;
    int32_t i;

    for(i = 0; i < cols * rows; ++i) {
      real34Add(REGISTER_REAL34_DATA(REGISTER_Y), VARIABLE_REAL34_DATA(&x.matrixElements[i]), VARIABLE_REAL34_DATA(&x.matrixElements[i]));
    }
  }
  else {
    elementwiseRealCxma(addRealCplx);
  }
}



/********************************************//**
 * \brief Y(complex34 matrix) + X(complex34) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void addCxmaCplx(void) {
  complex34Matrix_t y;

  linkToComplexMatrixRegister(REGISTER_Y, &y);

  const uint16_t rows = y.header.matrixRows;
  const uint16_t cols = y.header.matrixColumns;
  int32_t i;

  for(i = 0; i < cols * rows; ++i) {
    real34Add(VARIABLE_REAL34_DATA(&y.matrixElements[i]), REGISTER_REAL34_DATA(REGISTER_X), VARIABLE_REAL34_DATA(&y.matrixElements[i]));
    real34Add(VARIABLE_IMAG34_DATA(&y.matrixElements[i]), REGISTER_IMAG34_DATA(REGISTER_X), VARIABLE_IMAG34_DATA(&y.matrixElements[i]));
  }
  fnSwapXY(NOPARAM);
}



/********************************************//**
 * \brief Y(complex34) + X(complex34 matrix) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
void addCplxCxma(void) {
  complex34Matrix_t x;

  linkToComplexMatrixRegister(REGISTER_X, &x);

  const uint16_t rows = x.header.matrixRows;
  const uint16_t cols = x.header.matrixColumns;
  int32_t i;

  for(i = 0; i < cols * rows; ++i) {
    real34Add(REGISTER_REAL34_DATA(REGISTER_Y), VARIABLE_REAL34_DATA(&x.matrixElements[i]), VARIABLE_REAL34_DATA(&x.matrixElements[i]));
    real34Add(REGISTER_IMAG34_DATA(REGISTER_Y), VARIABLE_IMAG34_DATA(&x.matrixElements[i]), VARIABLE_IMAG34_DATA(&x.matrixElements[i]));
  }
}



/******************************************************************************************************************************************************************************************/
/* short integer + ...                                                                                                                                                                    */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(short integer) + X(short integer) ==> X(short integer)
 *
 * \param void
 * \return void
 ***********************************************/
void addShoIShoI(void) {
  setRegisterTag(REGISTER_X, getRegisterTag(REGISTER_Y));
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_intAdd(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_Y)), *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)));
}



/********************************************//**
 * \brief Y(short integer) + X(real34) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void addShoIReal(void) {
  real_t y, x;
  angularMode_t xAngularMode;

  convertShortIntegerRegisterToReal(REGISTER_Y, &y, &ctxtReal39);
  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x);
  xAngularMode = getRegisterAngularMode(REGISTER_X);

  if(xAngularMode == amNone) {
    realAdd(&y, &x, &x, &ctxtReal39);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
  }
  else {
    convertAngleFromTo(&x, xAngularMode, currentAngularMode, &ctxtReal39);
    realAdd(&y, &x, &x, &ctxtReal39);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
    setRegisterAngularMode(REGISTER_X, currentAngularMode);
  }
}



/********************************************//**
 * \brief Y(real34) + X(short integer) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void addRealShoI(void) {
  real_t y, x;
  angularMode_t yAngularMode;

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y);
  yAngularMode = getRegisterAngularMode(REGISTER_Y);
  convertShortIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);

  if(yAngularMode == amNone) {
    realAdd(&y, &x, &x, &ctxtReal39);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
  }
  else {
    convertAngleFromTo(&y, yAngularMode, currentAngularMode, &ctxtReal39);
    realAdd(&y, &x, &x, &ctxtReal39);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
    setRegisterAngularMode(REGISTER_X, currentAngularMode);
  }
}



/********************************************//**
 * \brief Y(short integer) + X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void addShoICplx(void) {
  convertShortIntegerRegisterToReal34Register(REGISTER_Y, REGISTER_Y);
  real34Add(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X)); // real part
}



/********************************************//**
 * \brief Y(complex34) + X(short integer) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void addCplxShoI(void) {
  convertShortIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
  real34Add(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_Y)); // real part
  reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
  complex34Copy(REGISTER_COMPLEX34_DATA(REGISTER_Y), REGISTER_COMPLEX34_DATA(REGISTER_X));
}



/******************************************************************************************************************************************************************************************/
/* real34 + ...                                                                                                                                                                           */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(real34) + X(real34) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void addRealReal(void) {
  angularMode_t yAngularMode, xAngularMode;

  yAngularMode = getRegisterAngularMode(REGISTER_Y);
  xAngularMode = getRegisterAngularMode(REGISTER_X);

  if(yAngularMode == amNone && xAngularMode == amNone) {
    real34Add(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
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

    realAdd(&y, &x, &x, &ctxtReal39);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
    setRegisterAngularMode(REGISTER_X, currentAngularMode);
  }
}



/********************************************//**
 * \brief Y(real34) + X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void addRealCplx(void) {
  real34Add(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X)); // real part
}



/********************************************//**
 * \brief Y(complex34) + X(real34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void addCplxReal(void) {
  real34Add(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_Y)); // real part
  reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
  complex34Copy(REGISTER_COMPLEX34_DATA(REGISTER_Y), REGISTER_COMPLEX34_DATA(REGISTER_X)); // imaginary part
}



/******************************************************************************************************************************************************************************************/
/* complex34 + ...                                                                                                                                                                        */
/******************************************************************************************************************************************************************************************/
void addComplex(const real_t *aReal, const real_t *aImag, const real_t *bReal, const real_t *bImag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  realAdd(aReal, bReal, resReal, realContext);
  realAdd(aImag, bImag, resImag, realContext);
}

/********************************************//**
 * \brief Y(complex34) + X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
void addCplxCplx(void) {
  real34Add(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X)); // real part
  real34Add(REGISTER_IMAG34_DATA(REGISTER_Y), REGISTER_IMAG34_DATA(REGISTER_X), REGISTER_IMAG34_DATA(REGISTER_X)); // imaginary part
}
