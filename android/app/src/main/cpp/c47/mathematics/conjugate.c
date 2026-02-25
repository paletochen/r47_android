// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file conjugate.c
 ***********************************************/

#include "c47.h"

static void conjRema(void) {
  complex34Matrix_t cMat;

  convertReal34MatrixRegisterToComplex34Matrix(REGISTER_X, &cMat);
  if(getSystemFlag(FLAG_SPCRES))
    for (uint16_t i = 0; i < cMat.header.matrixRows * cMat.header.matrixColumns; ++i)
      real34ChangeSign(VARIABLE_IMAG34_DATA(&cMat.matrixElements[i]));
  convertComplex34MatrixToComplex34MatrixRegister(&cMat, REGISTER_X);
}

static void conjCxma(void) {
  complex34Matrix_t cMat;

  linkToComplexMatrixRegister(REGISTER_X, &cMat);

  for(uint16_t i = 0; i < cMat.header.matrixRows * cMat.header.matrixColumns; ++i) {
    real34ChangeSign(VARIABLE_IMAG34_DATA(&cMat.matrixElements[i]));
    if(real34IsZero(VARIABLE_IMAG34_DATA(&cMat.matrixElements[i])) && !getSystemFlag(FLAG_SPCRES)) {
      real34SetPositiveSign(VARIABLE_IMAG34_DATA(&cMat.matrixElements[i]));
    }
  }
}

void conjCplx(void) {
  real_t r, i;

  if(!getRegisterAsComplex(REGISTER_X, &r, &i))
      return;

  realChangeSign(&i);
  if(realIsZero(&i) && !getSystemFlag(FLAG_SPCRES)) {
    realSetPositiveSign(&i);
  }
  reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
  convertComplexToResultRegister(&r, &i, REGISTER_X);
}

/********************************************//**
 * \brief regX ==> regL and conj(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnConjugate(uint16_t unusedButMandatoryParameter) {
  uint32_t typex = getRegisterDataType(REGISTER_X);

  if(!saveLastX())
    return;

  if(typex == dtComplex34Matrix)
    conjCxma();
  else if(typex == dtReal34Matrix)
    conjRema();
  else
    conjCplx();
}

