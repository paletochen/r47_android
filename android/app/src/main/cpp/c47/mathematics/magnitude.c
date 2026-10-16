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

  linkToComplexMatrixRegister(REGISTER_X, &cMat);
  if(realMatrixInit(&rMat, cMat.header.matrixRows, cMat.header.matrixColumns)) {
    for(uint16_t i = 0; i < cMat.header.matrixRows * cMat.header.matrixColumns; ++i) {
      real_t magnitude;
      cplx_t z;
      real34ToReal(VARIABLE_REAL34_DATA(&cMat.matrixElements[i]), &z.Real);
      real34ToReal(VARIABLE_IMAG34_DATA(&cMat.matrixElements[i]), &z.Imag);
      complexMagnitude(&z, &magnitude, &ctxtReal39);
      roundToSignificantDigits(&magnitude, &magnitude, significantDigits == 0 ? 34 : significantDigits, &ctxtReal39);
      realToReal34(&magnitude, &rMat.matrixElements[i]);
    }

    convertReal34MatrixToReal34MatrixRegister(&rMat, REGISTER_X);
    realMatrixFree(&rMat);
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
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
  real_t c;
  cplx_t a;

  if(getRegisterAsComplex(REGISTER_X, &a)) {
    complexMagnitude(&a, &c, &ctxtReal39);
    convertRealToResultRegister(&c, REGISTER_X, amNone);
  }
}

// c may be the same variable as a->Real, not as a->Imag. The square is formed in c, which needs room for the digits of realContext.
void complexMagnitude2(const cplx_t *a, real_t *c, realContext_t *realContext) {
  realMultiply(&a->Real, &a->Real, c, realContext);
  realFMA(&a->Imag, &a->Imag, c, c, realContext);
}


void complexMagnitude(const cplx_t *a, real_t *c, realContext_t *realContext) {
  complexMagnitude2(a, c, realContext);
  realSquareRoot(c, c, realContext);
}

/********************************************//**
 * \brief Returns the absolute value of an integer or a real and the magnitude of a complex
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnMagnitude(uint16_t unusedButMandatoryParameter) {
  if(getRegisterDataType(REGISTER_X) == dtComplex34Matrix) {
    if(saveLastX()) {
      magnitudeCxma();
      adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
    }
  }
  else {
    processIntRealComplexMonadicFunction(&magnitudeReal, &magnitudeCplx, &magnitudeShoI, &magnitudeLonI);
  }
}
