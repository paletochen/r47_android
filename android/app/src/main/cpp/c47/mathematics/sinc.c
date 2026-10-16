// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file sinc.c
 ***********************************************/
// Coded by JM, based on sin.c

#include "c47.h"

/********************************************//**
 * \brief Evaluate complex sinc with its power series.
 *
 * \param[in] z Complex argument
 * \param[out] res Result
 * \param[in] realContext Working context
 * \return void
 ***********************************************/
static void sincComplexSeries(const cplx_t *z, cplx_t *res, realContext_t *realContext) {
  cplx_t z2;
  cplx_t term;
  cplx_t next;
  cplx_t previous;
  REAL_T_PTR(divisor, 75);

  mulComplexComplex(z, z, &z2, realContext);
  realSetOne(&term.Real);
  realSetZero(&term.Imag);
  realSetZero(&res->Real);
  realSetZero(&res->Imag);

  for(int32_t n = 1; n < 1000; n++) {
    mulComplexComplex(&term, &z2, &next, realContext);
    realChangeSign(&next.Real);
    realChangeSign(&next.Imag);
    int32ToReal((2 * n) * (2 * n + 1), divisor);
    realDivide(&next.Real, divisor, &term.Real, realContext);
    realDivide(&next.Imag, divisor, &term.Imag, realContext);
    realCopy(&res->Real, &previous.Real);
    realCopy(&res->Imag, &previous.Imag);
    realAdd(&res->Real, &term.Real, &res->Real, realContext);
    realAdd(&res->Imag, &term.Imag, &res->Imag, realContext);
    if(realCompareEqual(&res->Real, &previous.Real) && realCompareEqual(&res->Imag, &previous.Imag)) {
      break;
    }
  }

  realCopy(&res->Real, &previous.Real);
  realAdd(const_1, &previous.Real, &res->Real, realContext);
  if(!realIsZero(&previous.Real) && realCompareEqual(&res->Real, const_1)) {
    realNextToward(&res->Real, realIsNegative(&previous.Real) ? const_0 : const_2, &res->Real, realContext);
  }
}

void sincComplex(const cplx_t *z, cplx_t *res, realContext_t *realContext) {
  // sin(a + ib) = sin(a)*cosh(b) + i*cos(a)*sinh(b)
  // sinc (a + ib) = sin(a + ib) / (a + ib), for the allowable conditions
  cplx_t x;

  //fnCvtFromCurrentAngularMode(amRadian);

  realCopy(&z->Real, &x.Real);
  realCopy(&z->Imag, &x.Imag);

  if(realIsZero(&x.Real) && realIsZero(&x.Imag)) {
    realSetOne(&res->Real);
    realSetZero(&res->Imag);
  }
  else if(realCompareAbsLessThan(&x.Real, const_1on10) && realCompareAbsLessThan(&x.Imag, const_1on10)) {
    sincComplexSeries(&x, res, realContext);
  }
  else if(realIsZero(&x.Real) && !realIsSpecial(&x.Imag)) {
    WP34S_SinhCosh(&x.Imag, &res->Real, NULL, realContext);
    realDivide(&res->Real, &x.Imag, &res->Real, realContext);
    realSetZero(&res->Imag);
  }
  else {
    real_t sina, cosa, sinhb, coshb;
    cplx_t sinZ;

    C47_WP34S_Cvt2RadSinCosTan(&z->Real, amRadian, &sina, &cosa, NULL, realContext);
    WP34S_SinhCosh(&z->Imag, &sinhb, &coshb, realContext);

    if(!realIsSpecial(&x.Real) && !realIsSpecial(&x.Imag) && (realIsInfinite(&sinhb) || realIsInfinite(&coshb))) {
      realCopyAbs(&x.Imag, &sinZ.Real);
      realMultiply(&x.Real, &sina, &res->Real, realContext);
      realFMA(&sinZ.Real, &cosa, &res->Real, &res->Real, realContext);
      realMultiply(&x.Real, &cosa, &res->Imag, realContext);
      realChangeSign(&sina);
      realFMA(&sinZ.Real, &sina, &res->Imag, &res->Imag, realContext);
      if(realIsNegative(&x.Imag)) {
        realChangeSign(&res->Imag);
      }
      if(!realIsZero(&res->Real)) {
        bool_t negative = realIsNegative(&res->Real);
        realSetPlusInfinity(&res->Real);
        if(negative) {
          realSetNegativeSign(&res->Real);
        }
      }
      if(!realIsZero(&res->Imag)) {
        bool_t negative = realIsNegative(&res->Imag);
        realSetPlusInfinity(&res->Imag);
        if(negative) {
          realSetNegativeSign(&res->Imag);
        }
      }
    }
    else {
      multiplyTrigByHyperbolic(&sina, &coshb, &res->Real, realContext);
      multiplyTrigByHyperbolic(&cosa, &sinhb, &res->Imag, realContext);

      realCopy(&res->Real, &sinZ.Real);
      realCopy(&res->Imag, &sinZ.Imag);
      divComplexComplex(&sinZ, &x, res, realContext);
    }
  }
}


static void sincReal(void) {
  real_t x, sine, phase;
  const real_t *r = &x;
  angularMode_t xAngularMode;
  const uint32_t type = getRegisterDataType(REGISTER_X);

  if(type == dtLongInteger) {
    convertLongIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal75);
    if(!reduceLongIntegerRegisterAngle(REGISTER_X, amRadian, &phase, true)) {
      return;
    }
  }
  else {
    if(!getRegisterAsReal(REGISTER_X, &x)) {
      return;
    }
    realCopy(&x, &phase);
  }

  if(realIsInfinite(&x)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      r = const_0;
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function sincReal:", "cannot divide a real34 by " STD_PLUS_MINUS STD_INFINITY " when flag SPCRES is not set", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
  else {
    if(realIsZero(&x)) {
      r = const_1;
    }
    else {
      xAngularMode = amRadian;
      if(type == dtReal34) {
        xAngularMode = getRegisterAngularMode(REGISTER_X);
        if(xAngularMode == amNone) {
          xAngularMode = amRadian;
        }
      }
      C47_WP34S_Cvt2RadSinCosTan(&phase, xAngularMode, &sine, NULL, NULL, &ctxtReal39);
      if(xAngularMode != amRadian) {
        convertAngleFromTo(&x, xAngularMode, amRadian, &ctxtReal39);
      }
      realDivide(&sine, &x, &x, &ctxtReal39);
    }
  }
  convertRealToResultRegister(r, REGISTER_X, amNone);
}



static void sincCplx(void) {
  cplx_t z;

  if(!getRegisterAsComplex(REGISTER_X, &z)) {
    return;
  }

  sincComplex(&z, &z, &ctxtReal75);

  convertComplexToResultRegister(&z, REGISTER_X);
}



/********************************************//**
 * \brief regX ==> regL and sinc(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSinc(uint16_t unusedButMandatoryParameter) {
  processRealComplexMonadicFunction(&sincReal, &sincCplx);
}
