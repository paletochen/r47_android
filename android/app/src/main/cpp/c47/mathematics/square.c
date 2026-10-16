// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file square.c
 ***********************************************/

#include "c47.h"

static void squareLonI(void) {
  longInteger_t lgInt;

  if(!getRegisterAsLongInt(REGISTER_X, lgInt, NULL)) {
    goto end;
  }

  longIntegerMultiply(lgInt, lgInt, lgInt);
  convertLongIntegerToLongIntegerRegister(lgInt, REGISTER_X);
end:
  longIntegerFree(lgInt);
}

static void squareShoI(void) {
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_intMultiply(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)), *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)));
}

static void squareReal(void) {
  real_t x;

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }

  if(realIsInfinite(&x) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function squareReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of curt when flag SPCRES is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  realMultiply(&x, &x, &x, &ctxtReal34);
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}

/********************************************//**
 * \brief Squares a complex number with a compensated real component
 *
 * \param[in]  z           complex number
 * \param[out] squared     square of z
 * \param[in]  realContext work context
 * \return void
 ***********************************************/
static void squareComplex(const cplx_t *z, cplx_t *squared, realContext_t *realContext) {
  real_t product, tmp, term;
  cplx_t result;

  if(realIsSpecial(&z->Real) || realIsSpecial(&z->Imag)) {
    mulComplexComplex(z, z, squared, realContext);
    return;
  }

  // Real part: compensated a^2 - b^2
  realMultiply(&z->Real, &z->Real, &product, realContext);
  realCopy(&z->Imag, &tmp);
  realChangeSign(&tmp);
  realFMA(&tmp, &z->Imag, &product, &term, realContext);
  realChangeSign(&product);
  realFMA(&z->Real, &z->Real, &product, &result.Real, realContext);
  realAdd(&result.Real, &term, &result.Real, realContext);

  // Imaginary part: 2ab
  realAdd(&z->Real, &z->Real, &tmp, realContext);
  realMultiply(&tmp, &z->Imag, &result.Imag, realContext);

  if(realIsZero(&result.Real)) {
    realSetZero(&result.Real);
  }
  if(realIsZero(&result.Imag)) {
    realSetZero(&result.Imag);
  }
  realCopy(&result.Real, &squared->Real);
  realCopy(&result.Imag, &squared->Imag);
}

static void squareCplx(void) {
  cplx_t a;

  if(getRegisterAsComplex(REGISTER_X, &a)) {
    squareComplex(&a, &a, &ctxtReal39);
    convertComplexToResultRegister(&a, REGISTER_X);
  }
}

/********************************************//**
 * \brief regX ==> regL and regX × regX ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter
 * \return void
 ***********************************************/
void fnSquare(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexMonadicFunction(&squareReal, &squareCplx, &squareShoI, &squareLonI);
}
