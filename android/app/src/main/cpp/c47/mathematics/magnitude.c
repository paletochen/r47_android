// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file magnitude.c
 ***********************************************/

#include "c47.h"





static void magnitudeLonI(void) {
  setRegisterLongIntegerSign(REGISTER_X, LI_POSITIVE);
}

static void magnitudeCxma(void) {
  complex34Matrix_t cMat;
  real34Matrix_t rMat;
  real34_t dummy;

  linkToComplexMatrixRegister(REGISTER_X, &cMat);
  if(realMatrixInit(&rMat, cMat.header.matrixRows, cMat.header.matrixColumns)) {
    for(uint16_t i = 0; i < cMat.header.matrixRows * cMat.header.matrixColumns; ++i) {
      real34RectangularToPolar(VARIABLE_REAL34_DATA(&cMat.matrixElements[i]), VARIABLE_IMAG34_DATA(&cMat.matrixElements[i]), &rMat.matrixElements[i], &dummy);
    }

    convertReal34MatrixToReal34MatrixRegister(&rMat, REGISTER_X);
    realMatrixFree(&rMat);
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
  }
}

static void magnitudeShoI(void) {
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_intAbs(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)));
}

static void magnitudeReal(void) {
  real34SetPositiveSign(REGISTER_REAL34_DATA(REGISTER_X));
  setRegisterAngularMode(REGISTER_X, amNone);
}

static void magnitudeCplx(void) {
  real_t a, b, c;

  if(getRegisterAsComplex(REGISTER_X, &a, &b)) {
    complexMagnitude(&a, &b, &c, &ctxtReal39);
    convertRealToResultRegister(&c, REGISTER_X, amNone);
  }
}

void complexMagnitude2(const real_t *a, const real_t *b, real_t *c, realContext_t *realContext) {
  real_t t;

  realMultiply(a, a, &t, realContext);
  realFMA(b, b, &t, c, realContext);
}


void complexMagnitude(const real_t *a, const real_t *b, real_t *c, realContext_t *realContext) {
  real_t u;

  complexMagnitude2(a, b, &u, realContext);
  realSquareRoot(&u, c, realContext);
}

/********************************************//**
 * \brief Returns the absolute value of an integer or a real and the magnitude of a complex
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnMagnitude(uint16_t unusedButMandatoryParameter) {
  if (getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
    if (saveLastX())
      magnitudeCxma();
  } else
    processIntRealComplexMonadicFunction(&magnitudeReal, &magnitudeCplx, &magnitudeShoI, &magnitudeLonI);
}
