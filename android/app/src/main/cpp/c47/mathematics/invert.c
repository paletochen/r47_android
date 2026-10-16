// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file invert.c
 ***********************************************/

#include "c47.h"

/********************************************//**
 * \brief 1 ÷ X(real34) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
static void invertReal(void) {
  real_t x, *res = &x;
  const uint32_t type = getRegisterDataType(REGISTER_X);

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }

  if(realIsZero(&x)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      res = realIsNegative(&x) ? const_minusInfinity : const_plusInfinity;
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function invertReal:", "cannot divide a real by 0", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }

  else if(realIsInfinite(&x)) {
    const int setNeg = realIsNegative(&x) && getSystemFlag(FLAG_SPCRES);

    realSetZero(&x);
    if(setNeg) {
      realChangeSign(&x);
    }
  }

  else if(realCompareAbsEqual(&x, const_1)) {
    return;
  }
  else if(type == dtLongInteger) {
    real34_t reciprocal;

    if(reciprocalLongIntegerRegisterToReal34(REGISTER_X, &reciprocal)) {
      convertReal34ToReal34Register(&reciprocal, REGISTER_X);
    }
    return;
  }
  else {
    realDivide(const_1, &x, &x, &ctxtReal34);
  }
  convertRealToResultRegister(res, REGISTER_X, amNone);
}



/********************************************//**
 * \brief 1 ÷ X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
static void invertCplx(void) {
  cplx_t a;

  if(getRegisterAsComplex(REGISTER_X, &a)) {
    divRealComplex(const_1, &a, &a, &ctxtReal39);
    convertComplexToResultRegister(&a, REGISTER_X);
  }
}


/********************************************//**
 * \brief regX ==> regL and 1/regX ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnInvert(uint16_t unusedButMandatoryParameter) {
  const uint32_t type = getRegisterDataType(REGISTER_X);

  if(type == dtReal34Matrix || type == dtComplex34Matrix) {
    fnInvertMatrix(NOPARAM);
  }
  else {
    processRealComplexMonadicFunction(&invertReal, &invertCplx);
  }
}
