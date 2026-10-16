// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file changeSign.c
 ***********************************************/

#include "c47.h"

static void chsLonI(void) {
  longInteger_t x;

  if(!getRegisterAsLongInt(REGISTER_X, x, NULL)) {
    goto end;
  }

  longIntegerChangeSign(x);
  convertLongIntegerToLongIntegerRegister(x, REGISTER_X);
end:
  longIntegerFree(x);
}

void chsShoI(void) {
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_intChs(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)));
}

static void chsZeroCheck(real34_t *a) {
  real34ChangeSign(a);
  if(real34IsZero(a) && !getSystemFlag(FLAG_SPCRES)) {
    real34SetPositiveSign(a);
  }
}

void chsReal(void) {
  chsZeroCheck(REGISTER_REAL34_DATA(REGISTER_X));
}

void chsCplx(void) {
  chsZeroCheck(REGISTER_REAL34_DATA(REGISTER_X));
  chsZeroCheck(REGISTER_IMAG34_DATA(REGISTER_X));
  setRegisterTag(REGISTER_X, amNone);
}

static void chsRema(void) {
  real34Matrix_t matrix;

  linkToRealMatrixRegister(REGISTER_X, &matrix);
  for(uint16_t i = 0; i < matrix.header.matrixRows * matrix.header.matrixColumns; ++i) {
    chsZeroCheck(&matrix.matrixElements[i]);
  }
  setRegisterTag(REGISTER_X, amNone);
}

static void chsCxma(void) {
  complex34Matrix_t matrix;

  linkToComplexMatrixRegister(REGISTER_X, &matrix);
  for(uint16_t i = 0; i < matrix.header.matrixRows * matrix.header.matrixColumns; ++i) {
    chsZeroCheck(VARIABLE_REAL34_DATA(&matrix.matrixElements[i]));
    chsZeroCheck(VARIABLE_IMAG34_DATA(&matrix.matrixElements[i]));
  }
  setRegisterTag(REGISTER_X, amNone);
}

/********************************************//**
 * \brief rexX ==> regL and -regX ==> regX
 * Drops Y, enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnChangeSign(uint16_t unusedButMandatoryParameter) {
  if(getRegisterDataType(REGISTER_X) == dtTime) {
    chsZeroCheck(REGISTER_REAL34_DATA(REGISTER_X));
    return;
  }
  if(getRegisterDataType(REGISTER_X) == dtReal34Matrix) {
    chsRema();
    adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
    return;
  }
  if(getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
    chsCxma();
    adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
    return;
  }

  processIntRealComplexMonadicFunction(&chsReal, &chsCplx, &chsShoI, &chsLonI);
}
