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
   displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
  }
}


static void realPartCplx(void) {
  real_t a, b;

  if (getRegisterAsComplex(REGISTER_X, &a, &b))
    convertRealToResultRegister(&a, REGISTER_X, amNone);
}


static void realPartReal(void) {
  real_t x;

  if (getRegisterAsReal(REGISTER_X, &x))
    convertRealToResultRegister(&x, REGISTER_X, amNone);
}


/***********************************************venus
 * \brief regX ==> regL and Re(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnRealPart(uint16_t unusedButMandatoryParameter) {
  if (getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
    if (saveLastX())
      realPartCxma();
  } else
    processRealComplexMonadicFunction(&realPartReal, &realPartCplx);
}
