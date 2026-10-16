// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file imaginaryPart.c
 ***********************************************/

#include "c47.h"


static void imagPartCxma(void) {
  complex34Matrix_t cMat;
  real34Matrix_t rMat;

  linkToComplexMatrixRegister(REGISTER_X, &cMat);
  if(realMatrixInit(&rMat, cMat.header.matrixRows, cMat.header.matrixColumns)) {
    for(uint16_t i = 0; i < cMat.header.matrixRows * cMat.header.matrixColumns; ++i) {
      real34Copy(VARIABLE_IMAG34_DATA(&cMat.matrixElements[i]), &rMat.matrixElements[i]);
    }

    convertReal34MatrixToReal34MatrixRegister(&rMat, REGISTER_X); // cMat invalidates here
    realMatrixFree(&rMat);
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
  }
}

static void imagPartCplx(void) {
  real34_t imaginary;

  real34Copy(REGISTER_IMAG34_DATA(REGISTER_X), &imaginary);
  convertReal34ToReal34Register(&imaginary, REGISTER_X);
}


static void imagPartReal(void) {
  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
  real34SetZero(REGISTER_REAL34_DATA(REGISTER_X));
}

/********************************************//**
 * \brief regX ==> regL and Im(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnImaginaryPart(uint16_t unusedButMandatoryParameter) {
  if(getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
    if(saveLastX()) {
      imagPartCxma();
      adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
    }
  }
  else {
    processRealComplexMonadicFunction(&imagPartReal, &imagPartCplx);
  }
}
