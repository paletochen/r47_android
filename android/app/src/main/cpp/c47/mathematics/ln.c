// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file ln.c
 ***********************************************/

#include "c47.h"




void lnComplex(const cplx_t *z, cplx_t *ln, realContext_t *realContext) {
  if(realIsZero(&z->Real) && realIsZero(&z->Imag)) {
    realSetMinusInfinity(&ln->Real);
    realSetZero(&ln->Imag);
  }
  else {
    realRectangularToPolar(z, &ln->Mag, &ln->Arg, realContext);
    WP34S_Ln(&ln->Mag, &ln->Real, realContext);
  }
}


void lnComplexDirect(const cplx_t *z, cplx_t *ln) {
  cplx_t w;

  realCopy(&z->Real, &w.Real);
  realCopy(&z->Imag, &w.Imag);
  realRectangularToPolar(&w, &ln->Mag, &ln->Arg, &ctxtReal39);
  if(realCompareGreaterThan(&ln->Mag, const_1on2) && realCompareLessThan(&ln->Mag, const_2)) {
    realContext_t nearUnitContext = ctxtReal75;
    if(realCompareAbsLessThan(&w.Real, &w.Imag)) {
      realSubtract(&w.Imag, const_1, &ln->Real, &nearUnitContext);
      realAdd(&w.Imag, const_1, &w.Imag, &nearUnitContext);
      realMultiply(&ln->Real, &w.Imag, &ln->Real, &nearUnitContext);
      realFMA(&w.Real, &w.Real, &ln->Real, &ln->Real, &nearUnitContext);
    }
    else {
      realSubtract(&w.Real, const_1, &ln->Real, &nearUnitContext);
      realAdd(&w.Real, const_1, &w.Real, &nearUnitContext);
      realMultiply(&ln->Real, &w.Real, &ln->Real, &nearUnitContext);
      realFMA(&w.Imag, &w.Imag, &ln->Real, &ln->Real, &nearUnitContext);
    }
    WP34S_Ln1P(&ln->Real, &ln->Real, &nearUnitContext);
    realMultiply(&ln->Real, const_1on2, &ln->Real, &nearUnitContext);
  }
  else {
    WP34S_Ln(&ln->Mag, &ln->Real, &ctxtReal39);
  }
}



/**********************************************************************
 * In all the functions below:
 * if X is a number then X = a + ib
 * The variables a and b are used for intermediate calculations
 ***********************************************************************/

void lnReal(void) {
  real_t x;

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }

  if(realIsZero(&x)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      realSetMinusInfinity(&x);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function lnReal:", "cannot calculate Ln(0)", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  else if(realIsInfinite(&x)) {
    if(!getSystemFlag(FLAG_SPCRES)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function lnReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of ln when flag SPCRES is not set", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
    else if(realIsPositive(&x)) {
      realSetPlusInfinity(&x);
    }
    else if(getFlag(FLAG_CPXRES)) {
      convertComplexPartsToResultRegister(const_plusInfinity, const39_pi, REGISTER_X);
      return;
    }
    else {
      realSetNaN(&x);
    }
  }
  else {
    if(realIsPositive(&x)) {
      WP34S_Ln(&x, &x, &ctxtReal39);
     }
    else if(getFlag(FLAG_CPXRES)) {
      realSetPositiveSign(&x);
      WP34S_Ln(&x, &x, &ctxtReal39);
      convertComplexPartsToResultRegister(&x, const39_pi, REGISTER_X);
      return;
    }
    else if(getSystemFlag(FLAG_SPCRES)) {
      realSetNaN(&x);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function lnReal:", "cannot calculate Ln of a negative number when CPXRES is not set!", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}


void lnCplx(void) {
  cplx_t x;
  cplx_t result;

  if(!getRegisterAsComplex(REGISTER_X, &x)) {
    return;
  }

  if(realIsZero(&x.Real) && realIsZero(&x.Imag) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function lnCplx:", "cannot calculate Ln(0)", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  lnComplexDirect(&x, &result);
  convertComplexToResultRegister(&result, REGISTER_X);
}


/********************************************//**
 * \brief regX ==> regL and ln(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnLn(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&lnReal, &lnCplx);
}
