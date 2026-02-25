// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file cross.c
 ***********************************************/

#include "c47.h"


/********************************************//**
 * \brief cross(Y(real34), X(real34)) ==> X(real34) = zero
 *
 * \param void
 * \return void
 ***********************************************/
static void crossReal(void) {
  convertRealToResultRegister(const_0, REGISTER_X, amNone);
}

/********************************************//**
 * \brief cross(Y(complex34), X(complex34)) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void crossCplx(void) {
  real_t xReal, xImag, yReal, yImag;
  real_t rReal, t;

  if (!getRegisterAsComplex(REGISTER_X, &xReal, &xImag)
          || !getRegisterAsComplex(REGISTER_Y, &yReal, &yImag))
      return;

  realMultiply(&xReal, &yImag, &t, &ctxtReal75);        // t = xReal * yImag
  realChangeSign(&t);
  realFMA(&yReal, &xImag, &t, &rReal, &ctxtReal39);     // r = yReal * xImag - t
  convertRealToResultRegister(&rReal, REGISTER_X, amNone);
}

//=============================================================================
// Matrix cross calculation function
//-----------------------------------------------------------------------------

/********************************************//**
 * \brief cross(Y(real34 matrix), X(real34 matrix)) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
static void crossRemaRema(void) {
  real34Matrix_t y, x, res;

  linkToRealMatrixRegister(REGISTER_Y, &y);
  linkToRealMatrixRegister(REGISTER_X, &x);

  if((realVectorSize(&y) == 0) || (realVectorSize(&x) == 0) || (realVectorSize(&y) > 3) || (realVectorSize(&x) > 3)) {
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "invalid numbers of elements of %d" STD_CROSS "%d-matrix to %d" STD_CROSS "%d-matrix",
              x.header.matrixRows, x.header.matrixColumns,
              y.header.matrixRows, y.header.matrixColumns);
      moreInfoOnError("In function crossRemaRema:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    crossRealVectors(&y, &x, &res);
    convertReal34MatrixToReal34MatrixRegister(&res, REGISTER_X);
  }
}

/********************************************//**
 * \brief cross(Y(complex34 matrix), X(complex34 matrix)) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
static void crossCpmaCpma(void) {
  complex34Matrix_t y, x, res;

  linkToComplexMatrixRegister(REGISTER_Y, &y);
  linkToComplexMatrixRegister(REGISTER_X, &x);

  if((complexVectorSize(&y) == 0) || (complexVectorSize(&x) == 0) || (complexVectorSize(&y) > 3) || (complexVectorSize(&x) > 3)) {
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "invalid numbers of elements of %d" STD_CROSS "%d-matrix to %d" STD_CROSS "%d-matrix",
              x.header.matrixRows, x.header.matrixColumns,
              y.header.matrixRows, y.header.matrixColumns);
      moreInfoOnError("In function crossCpmaCpma:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    crossComplexVectors(&y, &x, &res);
    convertComplex34MatrixToComplex34MatrixRegister(&res, REGISTER_X);
  }
}

/********************************************//**
 * \brief cross(Y(complex34 matrix), X(real34 matrix)) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
static void crossCpmaRema(void) {
  convertReal34MatrixRegisterToComplex34MatrixRegister(REGISTER_X, REGISTER_X);
  crossCpmaCpma();
}

/********************************************//**
 * \brief cross(Y(real34 matrix), X(complex34 matrix)) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
static void crossRemaCpma(void) {
  convertReal34MatrixRegisterToComplex34MatrixRegister(REGISTER_Y, REGISTER_Y);
  crossCpmaCpma();
}

/********************************************//**
 * \brief regX ==> regL and CROSS(regX, RegY) ==> regX
 * enables stack lift and refreshes the stack.
 * Calculate the cross (or vector) product between complex and matrix
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnCross(uint16_t unusedButMandatoryParameter) {
  uint32_t tx = getRegisterDataType(REGISTER_X), ty = getRegisterDataType(REGISTER_Y);

  if (tx == dtComplex34Matrix) {
    if (ty == dtComplex34Matrix) {
      if (saveLastX()) {
        crossCpmaCpma();
        adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
      }
    } else if (ty == dtReal34Matrix) {
      if (saveLastX()) {
        crossRemaCpma();
        adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
      }
    } else
      goto type_err;
  }

  else if (tx == dtReal34Matrix) {
    if (ty == dtComplex34Matrix) {
      if (saveLastX()) {
        crossCpmaRema();
        adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
      }
    } else if (ty == dtReal34Matrix) {
      if (saveLastX()) {
        crossRemaRema();
        adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
      }
    } else
      goto type_err;
  } else if (ty == dtComplex34Matrix || ty == dtReal34Matrix) {
type_err:
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);

    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot raise %s", getRegisterDataTypeName(REGISTER_Y, true, false));
      sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "to %s", getRegisterDataTypeName(REGISTER_X, true, false));
      moreInfoOnError("In function fnCross:", errorMessage, errorMessage + ERROR_MESSAGE_LENGTH/2, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  } else
    processRealComplexDyadicFunction(&crossReal, &crossCplx);
}
