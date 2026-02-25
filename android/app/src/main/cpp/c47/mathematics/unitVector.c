// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file unitVector.c
 ***********************************************/

#include "c47.h"

TO_QSPI void (* const unitVector[NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS])(void) = {
// regX ==> 1                2                3               4                5                6                7               8               9                10
//          Long integer     Real34           complex34       Time             Date             String           Real mat        Complex mat     Short integer    Config data
            unitVectorError, unitVectorError, unitVectorCplx, unitVectorError, unitVectorError, unitVectorError, unitVectorRema, unitVectorCxma, unitVectorError, unitVectorError
};



/********************************************//**
 * \brief Data type error in unitVector
 *
 * \param void
 * \return void
 ***********************************************/
#if (EXTRA_INFO_ON_CALC_ERROR == 1)
  void unitVectorError(void) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
    sprintf(errorMessage, "cannot calculate the unit vector of %s", getRegisterDataTypeName(REGISTER_X, true, false));
    moreInfoOnError("In function fnUnitVector:", errorMessage, NULL, NULL);
  }
#endif // (EXTRA_INFO_ON_CALC_ERROR == 1)



/********************************************//**
 * \brief regX ==> regL and unitVector(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnUnitVector(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }

  unitVector[getRegisterDataType(REGISTER_X)]();

  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}



void unitVectorCplx(void) {
  real_t a, b, norm;

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &a);
  real34ToReal(REGISTER_IMAG34_DATA(REGISTER_X), &b);

  realMultiply(&a, &a, &norm, &ctxtReal39);
  realFMA(&b, &b, &norm, &norm, &ctxtReal39);
  realSquareRoot(&norm, &norm, &ctxtReal39);
  realDivide(&a, &norm, &a, &ctxtReal39);
  realDivide(&b, &norm, &b, &ctxtReal39);

  convertComplexToResultRegister(&a, &b, REGISTER_X);
}



void unitVectorRema(void) {
  real34Matrix_t matrix;
  real_t elem, sum;

  linkToRealMatrixRegister(REGISTER_X, &matrix);

  realZero(&sum);
  for(int i = 0; i < matrix.header.matrixRows * matrix.header.matrixColumns; ++i) {
    real34ToReal(&matrix.matrixElements[i], &elem);
    realMultiply(&elem, &elem, &elem, &ctxtReal39);
    realAdd(&sum, &elem, &sum, &ctxtReal39);
  }
  realSquareRoot(&sum, &sum, &ctxtReal39);
  for(int i = 0; i < matrix.header.matrixRows * matrix.header.matrixColumns; ++i) {
    real34ToReal(&matrix.matrixElements[i], &elem);
    realDivide(&elem, &sum, &elem, &ctxtReal39);
    realToReal34(&elem, &matrix.matrixElements[i]);
  }
}



void unitVectorCxma(void) {
  complex34Matrix_t matrix;
  real_t real, imag, sum;

  linkToComplexMatrixRegister(REGISTER_X, &matrix);

  realZero(&sum);
  for(int i = 0; i < matrix.header.matrixRows * matrix.header.matrixColumns; ++i) {
    real34ToReal(VARIABLE_REAL34_DATA(&matrix.matrixElements[i]), &real);
    realMultiply(&real, &real, &real, &ctxtReal39);
    realAdd(&sum, &real, &sum, &ctxtReal39);
    real34ToReal(VARIABLE_IMAG34_DATA(&matrix.matrixElements[i]), &imag);
    realMultiply(&imag, &imag, &imag, &ctxtReal39);
    realAdd(&sum, &imag, &sum, &ctxtReal39);
  }
  realSquareRoot(&sum, &sum, &ctxtReal39);
  for(int i = 0; i < matrix.header.matrixRows * matrix.header.matrixColumns; ++i) {
    real34ToReal(VARIABLE_REAL34_DATA(&matrix.matrixElements[i]), &real);
    real34ToReal(VARIABLE_IMAG34_DATA(&matrix.matrixElements[i]), &imag);
    divComplexComplex(&real, &imag, &sum, const_0, &real, &imag, &ctxtReal39);
    realToReal34(&real, VARIABLE_REAL34_DATA(&matrix.matrixElements[i]));
    realToReal34(&imag, VARIABLE_IMAG34_DATA(&matrix.matrixElements[i]));
  }
}
