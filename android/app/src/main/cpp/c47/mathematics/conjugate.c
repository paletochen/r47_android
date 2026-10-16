// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file conjugate.c
 ***********************************************/

#include "c47.h"

static void conjugateComponent(real34_t *component) {
  real34ChangeSign(component);
  if(real34IsZero(component) && !getSystemFlag(FLAG_SPCRES)) {
    real34SetPositiveSign(component);
  }
  roundReal34ResultToSignificantDigits(component);
}

static void conjRema(void) {
  complex34Matrix_t cMat;

  convertReal34MatrixRegisterToComplex34Matrix(REGISTER_X, &cMat);
  if(lastErrorCode != 0) {
    return;
  }
  for(uint16_t i = 0; i < cMat.header.matrixRows * cMat.header.matrixColumns; ++i) {
    roundReal34ResultToSignificantDigits(VARIABLE_REAL34_DATA(&cMat.matrixElements[i]));
    conjugateComponent(VARIABLE_IMAG34_DATA(&cMat.matrixElements[i]));
  }
  convertComplex34MatrixToComplex34MatrixRegister(&cMat, REGISTER_X);
  complexMatrixFree(&cMat);
}

static void conjCxma(void) {
  complex34Matrix_t cMat;

  linkToComplexMatrixRegister(REGISTER_X, &cMat);

  for(uint16_t i = 0; i < cMat.header.matrixRows * cMat.header.matrixColumns; ++i) {
    roundReal34ResultToSignificantDigits(VARIABLE_REAL34_DATA(&cMat.matrixElements[i]));
    conjugateComponent(VARIABLE_IMAG34_DATA(&cMat.matrixElements[i]));
  }
  setRegisterTag(REGISTER_X, amNone);
}

void conjCplx(void) {
  const uint32_t type = getRegisterDataType(REGISTER_X);
  cplx_t x;

  if(type == dtComplex34) {
    roundReal34ResultToSignificantDigits(REGISTER_REAL34_DATA(REGISTER_X));
    conjugateComponent(REGISTER_IMAG34_DATA(REGISTER_X));
    setRegisterTag(REGISTER_X, amNone);
    return;
  }
  if(type == dtReal34) {
    real34_t r34;
    real34Copy(REGISTER_REAL34_DATA(REGISTER_X), &r34);
    roundReal34ResultToSignificantDigits(&r34);
    reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
    if(lastErrorCode == ERROR_RAM_FULL) {
      return;
    }
    real34Copy(&r34, REGISTER_REAL34_DATA(REGISTER_X));
    real34SetZero(REGISTER_IMAG34_DATA(REGISTER_X));
    conjugateComponent(REGISTER_IMAG34_DATA(REGISTER_X));
    return;
  }
  if(type == dtLongInteger && (significantDigits == 0 || significantDigits == 34)) {
    real34_t r34;
    convertLongIntegerRegisterToReal34(REGISTER_X, &r34);
    reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
    if(lastErrorCode == ERROR_RAM_FULL) {
      return;
    }
    real34Copy(&r34, REGISTER_REAL34_DATA(REGISTER_X));
    real34SetZero(REGISTER_IMAG34_DATA(REGISTER_X));
    conjugateComponent(REGISTER_IMAG34_DATA(REGISTER_X));
    return;
  }
  if(!getRegisterAsComplex(REGISTER_X, &x)) {
    return;
  }

  realChangeSign(&x.Imag);
  if(realIsZero(&x.Imag) && !getSystemFlag(FLAG_SPCRES)) {
    realSetPositiveSign(&x.Imag);
  }
  reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
  convertComplexToResultRegister(&x, REGISTER_X);
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

  if(!saveLastX()) {
    return;
  }

  if(typex == dtComplex34Matrix) {
    conjCxma();
  }
  else if(typex == dtReal34Matrix) {
    conjRema();
  }
  else {
    conjCplx();
  }
}
