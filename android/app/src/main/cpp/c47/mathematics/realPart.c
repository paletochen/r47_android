// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file realPart.c
 ***********************************************/

#include "c47.h"

static void realPartCxma(void) {
  complex34Matrix_t cMat;
  real34Matrix_t rMat;

  linkToComplexMatrixRegister(REGISTER_X, &cMat);
  if(realMatrixInit(&rMat, cMat.header.matrixRows, cMat.header.matrixColumns)) {
    for(uint16_t i = 0; i < cMat.header.matrixRows * cMat.header.matrixColumns; ++i) {
      real34Copy(VARIABLE_REAL34_DATA(&cMat.matrixElements[i]), &rMat.matrixElements[i]);
    }

    convertReal34MatrixToReal34MatrixRegister(&rMat, REGISTER_X); // cMat invalidates here
    realMatrixFree(&rMat);
  }
  else {
   displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
  }
}


static void realPartCplx(void) {
  real34_t real;

  real34Copy(REGISTER_REAL34_DATA(REGISTER_X), &real);
  convertReal34ToReal34Register(&real, REGISTER_X);
}


static void realPartReal(void) {
  real_t x;

  if(getRegisterDataType(REGISTER_X) == dtReal34) {
    setRegisterAngularMode(REGISTER_X, amNone);
    return;
  }
  if(getRegisterDataType(REGISTER_X) == dtLongInteger && (significantDigits == 0 || significantDigits == 34)) {
    convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
    return;
  }
  if(getRegisterAsReal(REGISTER_X, &x)) {
    convertRealToResultRegister(&x, REGISTER_X, amNone);
  }
}


/***********************************************venus
 * \brief regX ==> regL and Re(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnRealPart(uint16_t unusedButMandatoryParameter) {
  if(getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
    if(saveLastX()) {
      realPartCxma();
      adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
    }
  }
  else {
    processRealComplexMonadicFunction(&realPartReal, &realPartCplx);
  }
}
