// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file arg.c
 ***********************************************/
// Coded by JM, based on arctan.c

#include "c47.h"

/********************************************//**
 * \brief Data type error in arctan
 *
 * \param void
 * \return void
 ***********************************************/
static void argError(void) {
  getRegisterAsReal(REGISTER_X, NULL);
}

static const real_t *realArg(const real_t *x) {
  if(realIsNaN(x))
    return x;
  if(realIsZero(x))
    return getSystemFlag(FLAG_SPCRES) ? x : const_0;
  return realIsPositive(x) ? const_0 : const_180;
}

/********************************************//**
 * \brief regX ==> regL and arg(regX) = arctan(Im(regX) / Re(regX)) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
static void argReal(void) {
  real_t x;
  const real_t *r;

  if(!getRegisterAsReal(REGISTER_X, &x))
    return;

  r = realArg(&x);

  convertRealToResultRegister(r, REGISTER_X, amNone);
  if(!realIsNaN(r)) {
    convertAngle34FromTo(REGISTER_REAL34_DATA(REGISTER_X), amDegree, currentAngularMode);
    setRegisterAngularMode(REGISTER_X, currentAngularMode);
  }
}


static void argCplx(void) {
  real_t real, imag;

  if(!getRegisterAsComplex(REGISTER_X, &real, &imag))
    return;

  realRectangularToPolar(&real, &imag, &real, &imag, &ctxtReal39);
  convertAngleFromTo(&imag, amRadian, currentAngularMode, &ctxtReal39);

  reallocateRegister(REGISTER_X, dtReal34, 0, currentAngularMode);
  convertRealToReal34ResultRegister(&imag, REGISTER_X);
}



static void argRema(void) {
  real34Matrix_t x;
  real_t r;

  convertReal34MatrixRegisterToReal34Matrix(REGISTER_X, &x);
  const int numOfElements = x.header.matrixRows * x.header.matrixColumns;

  for(int i = 0; i < numOfElements; ++i) {
    real34ToReal(&x.matrixElements[i], &r);
    r = *realArg(&r);
    convertAngleFromTo(&r, amDegree, currentAngularMode, &ctxtReal39);
    realToReal34(&r, &x.matrixElements[i]);
  }

  convertReal34MatrixToReal34MatrixRegister(&x, REGISTER_X);
  realMatrixFree(&x);
}


static void argCxma(void) {
  complex34Matrix_t cMat;
  real34Matrix_t rMat;
  real34_t dummy;

  linkToComplexMatrixRegister(REGISTER_X, &cMat);
  if(realMatrixInit(&rMat, cMat.header.matrixRows, cMat.header.matrixColumns)) {
    for(uint16_t i = 0; i < cMat.header.matrixRows * cMat.header.matrixColumns; ++i) {
      real34RectangularToPolar(VARIABLE_REAL34_DATA(&cMat.matrixElements[i]), VARIABLE_IMAG34_DATA(&cMat.matrixElements[i]), &dummy, &rMat.matrixElements[i]);
      convertAngle34FromTo(&rMat.matrixElements[i], amRadian, currentAngularMode);
    }

    convertReal34MatrixToReal34MatrixRegister(&rMat, REGISTER_X);
    realMatrixFree(&rMat);
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
  }
}


TO_QSPI void (* const arg[NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS])(void) = {
// regX ==> 1            2           3           4            5            6            7            8            9             10
//          Long integer Real34      Complex34   Time         Date         String       Real34 mat   Complex34 m  Short integer Config data
            argReal,     argReal,    argCplx,    argError,    argError,    argError,    argRema,     argCxma,     argReal,      argError
};

/********************************************//**
 * \brief regX ==> regL and arctan(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnArg(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }

  arg[getRegisterDataType(REGISTER_X)]();

  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
}
