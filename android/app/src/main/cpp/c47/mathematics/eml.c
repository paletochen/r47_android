// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The C47 Authors

/********************************************//**
 * \file eml.c
 ***********************************************/

#include "c47.h"

#if defined(OPTION_EML)

static void emlCplx(void);

/********************************************//**
 * \brief e^Y(real34) - Ln X(real34) ==> X(real34)
 *
 * \param void
 * \return void
 ***********************************************/
static void emlReal(void) {
  real_t x, y, exp, ln;

  if(!getRegisterAsReal(REGISTER_X, &x) || !getRegisterAsReal(REGISTER_Y, &y)) {
    return;
  }

  if(realIsNegative(&x) && getFlag(FLAG_CPXRES)) {
    emlCplx();
    return;
  }

  /* e^Y domain and special value checks */
  if(realIsInfinite(&y) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function emlReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as Y input of eml when flag SPCRES is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  /* Ln X domain and special value checks */
  if(realIsZero(&x)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realSetPlusInfinity(&x);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function emlReal:", "cannot calculate Ln(0)", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  else if(realIsInfinite(&x)) {
    if(!getSystemFlag(FLAG_SPCRES)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function emlReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of eml when flag SPCRES is not set", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
    else if(realIsPositive(&x)) {
      realExp(&y, &exp, &ctxtReal39);
      if(realIsInfinite(&exp)) {
        realSetNaN(&x);
      }
      else {
        realSetMinusInfinity(&x);
      }
    }
    else if(getFlag(FLAG_CPXRES)) {
      emlCplx();
      return;
    }
  }
  else {
    realExp(&y, &exp, &ctxtReal75);
    WP34S_Ln(&x, &ln, &ctxtReal75);
    realSubtract(&exp, &ln, &x, &ctxtReal75);
  }
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}

/********************************************//**
 * \brief e^Y(complex34) - Ln X(complex34) ==> X(complex34)
 *
 * \param void
 * \return void
 ***********************************************/
static void emlCplx(void) {
  cplx_t x, y, exp, ln;

  if(!getRegisterAsComplex(REGISTER_Y, &y) || !getRegisterAsComplex(REGISTER_X, &x)) {
    return;
  }

  if(realIsZero(&x.Real) && realIsZero(&x.Imag) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function emlCplx:", "cannot calculate eml(0)", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  expComplex(&y, &exp, &ctxtReal75);
  lnComplex(&x, &ln, &ctxtReal75);
  subComplex(&exp, &ln, &x, &ctxtReal75);
  convertComplexToResultRegister(&x, REGISTER_X);
}

/********************************************//**
 * \brief regX ==> regL and e^regY - Ln(regX) ==> regX
 * Drops Y, enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter
 * \return void
 ***********************************************/
void fnEML(uint16_t unusedButMandatoryParameter) {
  processRealComplexDyadicFunction(&emlReal, &emlCplx);
}
#endif // OPTION_EML
