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
    roundReal34ResultToSignificantDigits(VARIABLE_REAL34_DATA(&cMat.matrixElements[i]));
    roundReal34ResultToSignificantDigits(VARIABLE_IMAG34_DATA(&cMat.matrixElements[i]));
  }
  setRegisterTag(REGISTER_X, amNone);
}

static void swapReImRema(void) {
  complex34Matrix_t c;
  real34Matrix_t r;

  linkToRealMatrixRegister(REGISTER_X, &r);
  convertReal34MatrixToComplex34Matrix(&r, &c);
  if(c.matrixElements == NULL) {
    return;
  }

  for(uint16_t i = 0; i < c.header.matrixRows * c.header.matrixColumns; ++i) {
    real34Copy(VARIABLE_REAL34_DATA(&c.matrixElements[i]), VARIABLE_IMAG34_DATA(&c.matrixElements[i]));
    real34SetZero(VARIABLE_REAL34_DATA(&c.matrixElements[i]));
    roundReal34ResultToSignificantDigits(VARIABLE_IMAG34_DATA(&c.matrixElements[i]));
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
  cplx_t x;
  const uint32_t type = getRegisterDataType(REGISTER_X);

  if(!saveLastX()) {
    return;
  }

  if(type == dtReal34Matrix) {
    swapReImRema();
  }
  else if(type == dtComplex34Matrix) {
    swapReImCxma();
  }
  else if(type == dtComplex34) {
    real34_t tmp;
    real34Copy(REGISTER_REAL34_DATA(REGISTER_X), &tmp);
    real34Copy(REGISTER_IMAG34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
    real34Copy(&tmp, REGISTER_IMAG34_DATA(REGISTER_X));
    roundReal34ResultToSignificantDigits(REGISTER_REAL34_DATA(REGISTER_X));
    roundReal34ResultToSignificantDigits(REGISTER_IMAG34_DATA(REGISTER_X));
    setRegisterTag(REGISTER_X, amNone);
  }
  else if(type == dtReal34) {
    real34_t real;
    real34Copy(REGISTER_REAL34_DATA(REGISTER_X), &real);
    roundReal34ResultToSignificantDigits(&real);
    reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
    if(lastErrorCode == ERROR_RAM_FULL) {
      return;
    }
    real34SetZero(REGISTER_REAL34_DATA(REGISTER_X));
    real34Copy(&real, REGISTER_IMAG34_DATA(REGISTER_X));
  }
  else if(type == dtLongInteger && (significantDigits == 0 || significantDigits == 34)) {
    real34_t real;
    convertLongIntegerRegisterToReal34(REGISTER_X, &real);
    reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
    if(lastErrorCode == ERROR_RAM_FULL) {
      return;
    }
    real34SetZero(REGISTER_REAL34_DATA(REGISTER_X));
    real34Copy(&real, REGISTER_IMAG34_DATA(REGISTER_X));
  }
  else {
    if(!getRegisterAsComplex(REGISTER_X, &x)) {
      return;
    }
    convertComplexPartsToResultRegister(&x.Imag, &x.Real, REGISTER_X);
  }
}
