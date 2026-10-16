// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file eulersFormula.c
 ***********************************************/

#include "c47.h"

void eulersFormula(const cplx_t *in, cplx_t *out, realContext_t *ctxt) {
  cplx_t z;

  if(realIsZero(&in->Imag)) {
    realCopy(&in->Real, &z.Real);
    C47_WP34S_Cvt2RadSinCosTan(&z.Real, amRadian, &out->Imag, &out->Real, NULL, ctxt);
    return;
  }

  mulComplexi(in, &z);
  expComplex(&z, out, ctxt);
}

static void eulersFormulaCplx(void) {
  cplx_t z;

  if(!getRegisterAsComplex(REGISTER_X, &z)) {
    return;
  }

  if((realIsInfinite(&z.Real) || realIsInfinite(&z.Imag)) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function eulersFormulaCplx:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as real or imag X input when flag SPCRES is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(realIsInfinite(&z.Real)) {
    convertComplexPartsToResultRegister(const_NaN, const_NaN, REGISTER_X);
    goto end;
  }

  if(realIsZero(&z.Imag)) {
    eulersFormula(&z, &z, &ctxtReal34);
  }
  else {
    eulersFormula(&z, &z, &ctxtReal39);
  }
  convertComplexToResultRegister(&z, REGISTER_X);

end:
  fnSetFlag(FLAG_CPXRES);
  fnRefreshState();
}

static void eulersFormulaReal(void) {
  real_t angle;
  cplx_t c;
  angularMode_t xAngularMode = amNone;
  const uint32_t type = getRegisterDataType(REGISTER_X);

  if(type == dtLongInteger) {
    if(!reduceLongIntegerRegisterAngle(REGISTER_X, amRadian, &c.Real, true)) {
      return;
    }
  }
  else if(!getRegisterAsReal(REGISTER_X, &c.Real)) {
    return;
  }

  if(realIsInfinite(&c.Real) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function eulersFormulaReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input when flag SPCRES is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(type == dtReal34) {
    xAngularMode = getRegisterAngularMode(REGISTER_X);
    if(xAngularMode != amNone) {
      realCopy(&c.Real, &angle);
      C47_WP34S_Cvt2RadSinCosTan(&angle, xAngularMode, &c.Imag, &c.Real, NULL, &ctxtReal34);
    }
  }

  if(type != dtReal34 || xAngularMode == amNone) {
    realSetZero(&c.Imag);
    eulersFormula(&c, &c, &ctxtReal34);
  }

  convertComplexToResultRegister(&c, REGISTER_X);
  fnSetFlag(FLAG_CPXRES);
  fnRefreshState();
}

void fnEulersFormula(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&eulersFormulaReal, &eulersFormulaCplx);
}
