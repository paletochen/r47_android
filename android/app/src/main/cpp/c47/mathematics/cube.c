// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file cube.c
 ***********************************************/

#include "c47.h"

static void cubeLonI(void) {
  longInteger_t x, c;

  if(!getRegisterAsLongInt(REGISTER_X, x, NULL)) {
    goto end;
  }

  longIntegerInit(c);
  longIntegerMultiply(x, x, c);
  longIntegerMultiply(c, x, c);
  convertLongIntegerToLongIntegerRegister(c, REGISTER_X);
  longIntegerFree(c);
end:
  longIntegerFree(x);
}

static void cubeShoI(void) {
  uint64_t square;

  // TODO: set the overflow/carry flags properly
  square = WP34S_intMultiply(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)), *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)));
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_intMultiply(square, *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)));
}

static void cubeReal(void) {
  real_t x, xSquared;

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }

  if(realIsInfinite(&x) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function cubeReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of curt when flag SPCRES is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  realMultiply(&x, &x, &xSquared, &ctxtReal39);
  realMultiply(&xSquared, &x, &x, &ctxtReal39);
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}

/********************************************//**
 * \brief Cubes a complex number with compensated component factors
 *
 * \param[in]  z          complex number
 * \param[out] cubed      cube of z
 * \param[in]  realContext work context
 * \return void
 ***********************************************/
static void cubeComplex(const cplx_t *z, cplx_t *cubed, realContext_t *realContext) {
  real_t factor, product, term, correction;
  cplx_t result;

  if(realIsNaN(&z->Real) || realIsNaN(&z->Imag)) {
    realSetNaN(&cubed->Real);
    realSetNaN(&cubed->Imag);
    return;
  }

  if(realIsInfinite(&z->Real) || realIsInfinite(&z->Imag)) {
    setInfiniteComplexAngle((3 * getInfiniteComplexAngle(z)) % 8, cubed);
    return;
  }

  // Real part: a(a^2 - 3b^2)
  realMultiply(const_3, &z->Imag, &factor, realContext);
  realMultiply(&z->Real, &z->Real, &product, realContext);
  realChangeSign(&factor);
  realFMA(&factor, &z->Imag, &product, &term, realContext);
  realChangeSign(&product);
  realFMA(&z->Real, &z->Real, &product, &correction, realContext);
  realAdd(&correction, &term, &correction, realContext);
  realMultiply(&z->Real, &correction, &result.Real, realContext);

  // Imaginary part: b(3a^2 - b^2)
  realMultiply(const_3, &z->Real, &factor, realContext);
  realMultiply(&factor, &z->Real, &product, realContext);
  realCopy(&z->Imag, &correction);
  realChangeSign(&correction);
  realFMA(&correction, &z->Imag, &product, &term, realContext);
  realChangeSign(&product);
  realFMA(&factor, &z->Real, &product, &correction, realContext);
  realAdd(&correction, &term, &correction, realContext);
  realMultiply(&z->Imag, &correction, &result.Imag, realContext);

  if(realIsZero(&result.Real)) {
    realSetZero(&result.Real);
  }
  if(realIsZero(&result.Imag)) {
    realSetZero(&result.Imag);
  }
  realCopy(&result.Real, &cubed->Real);
  realCopy(&result.Imag, &cubed->Imag);
}

static void cubeCplx(void) {
  cplx_t a;

  if(getRegisterAsComplex(REGISTER_X, &a)) {
    cubeComplex(&a, &a, &ctxtReal39);
    convertComplexToResultRegister(&a, REGISTER_X);
  }
}

/********************************************//**
 * \brief regX ==> regL and regX × regX × regX ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter
 * \return void
 ***********************************************/
void fnCube(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexMonadicFunction(&cubeReal, &cubeCplx, &cubeShoI, &cubeLonI);
}

