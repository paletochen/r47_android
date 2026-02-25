// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file parallel.c
 ***********************************************/

#include "c47.h"


/******************************************************************************************************************************************************************************************/
/* real34 || ...                                                                                                                                                                          */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(real34) || X(real34) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
static void doParallelReal(void) {
  real_t y, x, product;

  if(!getRegisterAsReal(REGISTER_X, &x)
          || !getRegisterAsReal(REGISTER_Y, &y))
    return;

  // y || x = xy / (x+y)
  if(!realIsZero(&x)) {
    realMultiply(&y, &x, &product, &ctxtReal75);
    realAdd(&y, &x, &y, &ctxtReal75);
    realDivide(&product, &y, &x, &ctxtReal75);
  }
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}



/******************************************************************************************************************************************************************************************/
/* complex34 || ...                                                                                                                                                                       */
/******************************************************************************************************************************************************************************************/

/********************************************//**
 * \brief Y(complex34) || X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
static void doParallelComplex(void) {
  real_t yReal, xReal, productReal, sumReal;
  real_t yImag, xImag, productImag, sumImag;

  // y || x = xy / (x + y)
  if (!getRegisterAsComplex(REGISTER_X, &xReal, &xImag)
          || !getRegisterAsComplex(REGISTER_Y, &yReal, &yImag))
      return;

  if(!realIsZero(&xReal) || !realIsZero(&xImag)) {
    mulComplexComplex(&yReal, &yImag, &xReal, &xImag, &productReal, &productImag, &ctxtReal75);
    realAdd(&yReal, &xReal, &sumReal, &ctxtReal75);
    realAdd(&yImag, &xImag, &sumImag, &ctxtReal75);
    divComplexComplex(&productReal, &productImag, &sumReal, &sumImag, &xReal, &xImag, &ctxtReal75);
  }
  convertComplexToResultRegister(&xReal, &xImag, REGISTER_X);
}

/********************************************//**
 * \brief regX ==> regL and regY || regX ==> regX
 * Drops Y, enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter
 * \return void
 ***********************************************/
void fnParallel(uint16_t unusedButMandatoryParameter) {
  processRealComplexDyadicFunction(doParallelReal, doParallelComplex);
}

