// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file dot.c
 ***********************************************/

#include "c47.h"

//=============================================================================
// Complex dot product calculation functionS
//-----------------------------------------------------------------------------

// Numerically stable dot product funtion.  Slower but accurate.
void dotCplx(const real_t *xReal, const real_t *xImag, const real_t *yReal, const real_t *yImag, real_t *rReal, realContext_t *realContext) {
  real_t p, t;

  realMultiply(xReal, yReal, &p, realContext);
  realFMA(xImag, yImag, &p, &t, realContext);
  if(realIsSpecial(xReal) || realIsSpecial(xImag) || realIsSpecial(yReal) || realIsSpecial(yImag)) {
    realCopy(&t, rReal);
    return;
  }
  realChangeSign(&p);
  realFMA(xReal, yReal, &p, rReal, realContext);
  realAdd(rReal, &t, rReal, realContext);
}


/********************************************//**
 * \brief Dot(Y(real), X(real)) ==> X(real34) = x * y
 *
 * \param void
 * \return void
 ***********************************************/
void doDotReal(void) {
  real_t x, y, r;
  real34_t exactResult;
  const uint32_t typeX = getRegisterDataType(REGISTER_X);
  const uint32_t typeY = getRegisterDataType(REGISTER_Y);
  realContext_t *realContext = typeX == dtReal34 && typeY == dtReal34 ? &ctxtReal34 : &ctxtReal39;

  if((typeX == dtLongInteger || typeY == dtLongInteger) && multiplyRegisterValuesToReal34Scaled(REGISTER_X, REGISTER_Y, 0, &exactResult)) {
    convertReal34ToReal34Register(&exactResult, REGISTER_X);
  }
  else if(getRegisterAsReal(REGISTER_X, &x) && getRegisterAsReal(REGISTER_Y, &y)) {
    realMultiply(&x, &y, &r, realContext);
    convertRealToResultRegister(&r, REGISTER_X, amNone);
  }
}

/********************************************//**
 * \brief Dot(Y(complex34), X(complex34)) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
void doDotCplx(void) {
  cplx_t x, y;
  real_t rReal;

  if(getRegisterAsComplex(REGISTER_X, &x) && getRegisterAsComplex(REGISTER_Y, &y)) {
    dotCplx(&x.Real, &x.Imag, &y.Real, &y.Imag, &rReal, &ctxtReal39);
    convertRealToResultRegister(&rReal, REGISTER_X, amNone);
  }
}

//=============================================================================
// Matrix dot calculation function
//-----------------------------------------------------------------------------

/********************************************//**
 * \brief Dot(Y(real34 matrix), X(real34 matrix)) ==> X(real34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
static void dotRemaRema(void) {
  real34Matrix_t y, x;
  real34_t res;

  linkToRealMatrixRegister(REGISTER_Y, &y);
  linkToRealMatrixRegister(REGISTER_X, &x);

  if((realVectorSize(&y) == 0) || (realVectorSize(&x) == 0) || (realVectorSize(&y) != realVectorSize(&x))) {
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "numbers of elements of %d" STD_CROSS "%d-matrix to %d" STD_CROSS "%d-matrix mismatch",
              x.header.matrixRows, x.header.matrixColumns,
              y.header.matrixRows, y.header.matrixColumns);
      moreInfoOnError("In function dotRemaRema:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    dotRealVectors(&y, &x, &res);
    convertReal34ToReal34Register(&res, REGISTER_X);
  }
}

/********************************************//**
 * \brief Dot(Y(complex34 matrix), X(complex34 matrix)) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
static void dotCpmaCpma(void) {
  complex34Matrix_t y, x;
  real34_t res_r, res_i;

  linkToComplexMatrixRegister(REGISTER_Y, &y);
  linkToComplexMatrixRegister(REGISTER_X, &x);

  if((complexVectorSize(&y) == 0) || (complexVectorSize(&x) == 0) || (complexVectorSize(&y) != complexVectorSize(&x))) {
    displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "numbers of elements of %d" STD_CROSS "%d-matrix to %d" STD_CROSS "%d-matrix mismatch",
              x.header.matrixRows, x.header.matrixColumns,
              y.header.matrixRows, y.header.matrixColumns);
      moreInfoOnError("In function dotCpmaCpma:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    dotComplexVectors(&y, &x, &res_r, &res_i);
    convertReal34ToComplex34Register(&res_r, &res_i, REGISTER_X);
  }
}

/********************************************//**
 * \brief Dot(Y(complex34 matrix), X(real34 matrix)) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
static void dotCpmaRema(void) {
  convertReal34MatrixRegisterToComplex34MatrixRegister(REGISTER_X, REGISTER_X);
  if(lastErrorCode != 0) {
    return;
  }
  dotCpmaCpma();
}

/********************************************//**
 * \brief Dot(Y(real34 matrix), X(complex34 matrix)) ==> X(complex34 matrix)
 *
 * \param void
 * \return void
 ***********************************************/
static void dotRemaCpma(void) {
  convertReal34MatrixRegisterToComplex34MatrixRegister(REGISTER_Y, REGISTER_Y);
  if(lastErrorCode != 0) {
    return;
  }
  dotCpmaCpma();
}


/********************************************//**
 * \brief regX ==> regL and Dot(regX, RegY) ==> regX
 * enables stack lift and refreshes the stack.
 * Calculate the dot (or scalar) product between complex and matrix
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnDot(uint16_t unusedButMandatoryParameter) {
  uint32_t tx = getRegisterDataType(REGISTER_X), ty = getRegisterDataType(REGISTER_Y);

  if(tx == dtComplex34Matrix) {
    if(ty == dtComplex34Matrix) {
      if(saveLastX()) {
        dotCpmaCpma();
        adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
      }
    }
    else if(ty == dtReal34Matrix) {
      if(saveLastX()) {
        dotRemaCpma();
        adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
      }
    }
    else {
      goto type_err;
    }
  }
  else if(tx == dtReal34Matrix) {
    if(ty == dtComplex34Matrix) {
      if(saveLastX()) {
        dotCpmaRema();
        adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
      }
    }
    else if(ty == dtReal34Matrix) {
      if(saveLastX()) {
        dotRemaRema();
        adjustResult(REGISTER_X, true, true, REGISTER_X, -1, -1);
      }
    }
    else {
      goto type_err;
    }
  }
  else if(ty == dtComplex34Matrix || ty == dtReal34Matrix) {
type_err:
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);

    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot raise %s", getRegisterDataTypeName(REGISTER_Y, true, false));
      sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "to %s", getRegisterDataTypeName(REGISTER_X, true, false));
      moreInfoOnError("In function fnDot:", errorMessage, errorMessage + ERROR_MESSAGE_LENGTH/2, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    processRealComplexDyadicFunction(&doDotReal, &doDotCplx);
  }
}
