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
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
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
  real_t norm;
  cplx_t x;

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x.Real);
  real34ToReal(REGISTER_IMAG34_DATA(REGISTER_X), &x.Imag);

  realMultiply(&x.Real, &x.Real, &norm, &ctxtReal39);
  realFMA(&x.Imag, &x.Imag, &norm, &norm, &ctxtReal39);
  realSquareRoot(&norm, &norm, &ctxtReal39);
  realDivide(&x.Real, &norm, &x.Real, &ctxtReal39);
  realDivide(&x.Imag, &norm, &x.Imag, &ctxtReal39);

  convertComplexToResultRegister(&x, REGISTER_X);
}



void unitVectorRema(void) {
  real34Matrix_t matrix;
  real_t elem, sum;

  linkToRealMatrixRegister(REGISTER_X, &matrix);

  realSetZero(&sum);
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
  cplx_t elem, sum;

  linkToComplexMatrixRegister(REGISTER_X, &matrix);

  realSetZero(&sum.Real);
  for(int i = 0; i < matrix.header.matrixRows * matrix.header.matrixColumns; ++i) {
    real34ToReal(VARIABLE_REAL34_DATA(&matrix.matrixElements[i]), &elem.Real);
    realMultiply(&elem.Real, &elem.Real, &elem.Real, &ctxtReal39);
    realAdd(&sum.Real, &elem.Real, &sum.Real, &ctxtReal39);
    real34ToReal(VARIABLE_IMAG34_DATA(&matrix.matrixElements[i]), &elem.Imag);
    realMultiply(&elem.Imag, &elem.Imag, &elem.Imag, &ctxtReal39);
    realAdd(&sum.Real, &elem.Imag, &sum.Real, &ctxtReal39);
  }
  realSquareRoot(&sum.Real, &sum.Real, &ctxtReal39);
  realSetZero(&sum.Imag);
  for(int i = 0; i < matrix.header.matrixRows * matrix.header.matrixColumns; ++i) {
    real34ToReal(VARIABLE_REAL34_DATA(&matrix.matrixElements[i]), &elem.Real);
    real34ToReal(VARIABLE_IMAG34_DATA(&matrix.matrixElements[i]), &elem.Imag);
    if(realIsSpecial(&elem.Real) || realIsSpecial(&elem.Imag) || realIsSpecial(&sum.Real)) {
      divComplexComplex(&elem, &sum, &elem, &ctxtReal39);
    }
    else {
      realDivide(&elem.Real, &sum.Real, &elem.Real, &ctxtReal39);
      realDivide(&elem.Imag, &sum.Real, &elem.Imag, &ctxtReal39);
    }
    realToReal34(&elem.Real, VARIABLE_REAL34_DATA(&matrix.matrixElements[i]));
    realToReal34(&elem.Imag, VARIABLE_IMAG34_DATA(&matrix.matrixElements[i]));
  }
}
