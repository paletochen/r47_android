// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file swapRealImaginary.c
 ***********************************************/

#include "c47.h"

static void swapReImCxma(void) {
  complex34Matrix_t cMat;
  real34_t tmp;

  linkToComplexMatrixRegister(REGISTER_X, &cMat);

  for(uint16_t i = 0; i < cMat.header.matrixRows * cMat.header.matrixColumns; ++i) {
    real34Copy(VARIABLE_REAL34_DATA(&cMat.matrixElements[i]), &tmp);
    real34Copy(VARIABLE_IMAG34_DATA(&cMat.matrixElements[i]), VARIABLE_REAL34_DATA(&cMat.matrixElements[i]));
    real34Copy(&tmp                                         , VARIABLE_IMAG34_DATA(&cMat.matrixElements[i]));
  }
}

static void swapReImRema(void) {
  complex34Matrix_t c;
  real34Matrix_t r;

  linkToRealMatrixRegister(REGISTER_X, &r);
  convertReal34MatrixToComplex34Matrix(&r, &c);

  for(uint16_t i = 0; i < c.header.matrixRows * c.header.matrixColumns; ++i) {
    real34Copy(VARIABLE_REAL34_DATA(&c.matrixElements[i]), VARIABLE_IMAG34_DATA(&c.matrixElements[i]));
    real34Zero(VARIABLE_REAL34_DATA(&c.matrixElements[i]));
  }

  convertComplex34MatrixToComplex34MatrixRegister(&c, REGISTER_X);
  complexMatrixFree(&c);
}

/********************************************//**
 * \brief regX ==> regL and Re<>IM(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSwapRealImaginary(uint16_t unusedButMandatoryParameter) {
  real_t a, b;
  const uint32_t type = getRegisterDataType(REGISTER_X);

  if(!saveLastX())
    return;

  if(type == dtReal34Matrix)
    swapReImRema();
  else if(type == dtComplex34Matrix)
    swapReImCxma();
  else {
    if(!getRegisterAsComplex(REGISTER_X, &a, &b))
      return;
    convertComplexToResultRegister(&b, &a, REGISTER_X);
  }
}

