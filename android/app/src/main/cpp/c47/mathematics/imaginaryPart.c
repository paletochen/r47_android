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
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
  }
}

static void imagPartCplx(void) {
  real_t zReal, zImag;

  if(getRegisterAsComplex(REGISTER_X, &zReal, &zImag))
    convertRealToResultRegister(&zImag, REGISTER_X, amNone);
}


static void imagPartReal(void) {
  convertRealToResultRegister(const_0, REGISTER_X, amNone);
}

/********************************************//**
 * \brief regX ==> regL and Im(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnImaginaryPart(uint16_t unusedButMandatoryParameter) {
  if (getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
    if (saveLastX())
      imagPartCxma();
  } else
    processRealComplexMonadicFunction(&imagPartReal, &imagPartCplx);
}
