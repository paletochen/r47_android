// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file logxy.c
 ***********************************************/

#include "c47.h"


static void logXYComplex(const real_t *xReal, const real_t *xImag, const real_t *yReal, const real_t *yImag, real_t *rReal, real_t *rImag, realContext_t *realContext) {
  real_t lnxReal, lnxImag;

  // lnComplex handle the case where y = 0+i0.
  lnComplex(yReal, yImag, rReal, rImag, realContext);                             //   r = Ln(y)
  lnComplex(xReal, xImag, &lnxReal, &lnxImag, realContext);                       // lnx = Ln(x)
  divComplexComplex(rReal, rImag, &lnxReal, &lnxImag, rReal, rImag, realContext); // r = Ln(y) / Ln(x)
}

#define COMPLEX_IS_ZERO(real, imag) (realIsZero(real) && (imag==NULL || realIsZero(imag)))

static bool_t checkArgs(const real_t *xReal, const real_t *xImag, const real_t *yReal, const real_t *yImag) {
  /*
   * Log(0, 0) = +Inf
   * Log(x, 0) = -Inf x!=0
   * Log(0, y) = NaN  y!=0
   */
  if(COMPLEX_IS_ZERO(xReal, xImag) && COMPLEX_IS_ZERO(yReal, yImag)) {
    if (getFlag(FLAG_SPCRES))
      convertRealToResultRegister(const_plusInfinity, REGISTER_X, amNone);
    else {
      displayCalcErrorMessage(ERROR_OVERFLOW_PLUS_INF, ERR_REGISTER_LINE, REGISTER_X);
      EXTRA_INFO_MESSAGE("checkArgs", "cannot calculate LogXY with x=0 and y=0");
    }
  }
  else if(!COMPLEX_IS_ZERO(xReal, xImag) && COMPLEX_IS_ZERO(yReal, yImag)) {
    if (getFlag(FLAG_SPCRES))
      convertRealToResultRegister(const_minusInfinity, REGISTER_X, amNone);
    else {
      displayCalcErrorMessage(ERROR_OVERFLOW_MINUS_INF, ERR_REGISTER_LINE, REGISTER_X);
      EXTRA_INFO_MESSAGE("checkArgs", "cannot calculate LogXY with x=0 and y!=0");
    }
  }
  else if(COMPLEX_IS_ZERO(xReal, xImag) && !COMPLEX_IS_ZERO(yReal, yImag)) {
    if (getFlag(FLAG_SPCRES))
      convertRealToResultRegister(const_NaN, REGISTER_X, amNone);
    else
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
  }
  else {
    return true;
  }

  return false;
}

static void logxy(const real_t *xReal, const real_t *yReal, realContext_t *realContext) {
  real_t rReal, rImag;

  /*
   * Log(0, 0) = +Inf
   * Log(x, 0) = -Inf x!=0
   * Log(0, y) = NaN  y!=0
   */
  if(checkArgs(xReal, NULL, yReal, NULL)) {
    if(realIsNegative(xReal) || realIsNegative(yReal)) {
      logXYComplex(xReal, const_0, yReal, const_0, &rReal, &rImag, realContext);

      if(!realIsZero(&rImag)) {
        if(getFlag(FLAG_CPXRES)) {
          convertComplexToResultRegister(&rReal, &rImag, REGISTER_X);
          return;
        }
        else if(getFlag(FLAG_SPCRES)) {
          realCopy(const_NaN, &rReal);
        }
        else {
          displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
          EXTRA_INFO_MESSAGE("logxy", "cannot calculate LogXY with x<0 or y<0 when flag I is not set");
          return;
        }
      }
    } else
      WP34S_Logxy(yReal, xReal, &rReal, realContext);
    convertRealToResultRegister(&rReal, REGISTER_X, amNone);
  }
}


static void logXYLongInt(void) {
  real_t x, y;

  if (!getRegisterAsReal(REGISTER_X, &x) || !getRegisterAsReal(REGISTER_Y, &y))
    return;

  logxy(&x, &y, &ctxtReal39);

  if(getRegisterDataType(REGISTER_X) == dtReal34 && real34IsAnInteger(REGISTER_REAL34_DATA(REGISTER_X)))
    convertReal34ToLongIntegerRegister(REGISTER_REAL34_DATA(REGISTER_X), REGISTER_X, DEC_ROUND_DOWN);
}


static void logXYShortInt(void) {
  real_t x, y;
  int32_t base = getRegisterShortIntegerBase(REGISTER_Y);

  if (!getRegisterAsReal(REGISTER_Y, &y) || !getRegisterAsReal(REGISTER_X, &x))
    return;

  logxy(&x, &y, &ctxtReal39);

  if(getRegisterDataType(REGISTER_X) == dtReal34) {
    if(real34IsNaN(REGISTER_REAL34_DATA(REGISTER_X)) && !getSystemFlag(FLAG_SPCRES)) {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      EXTRA_INFO_MESSAGE("logxy", "cannot calculate LogXY with x=0");
      return;
    }
    else if(real34IsAnInteger(REGISTER_REAL34_DATA(REGISTER_X))) {
      clearSystemFlag(FLAG_CARRY);
    }
    else {
      setSystemFlag(FLAG_CARRY);
    }
    fnChangeBase((uint16_t)base);
  }
}


static void logXYReal(void) {
  real_t x, y;

  if (!getRegisterAsReal(REGISTER_Y, &y) || !getRegisterAsReal(REGISTER_X, &x))
    return;
  logxy(&x, &y, &ctxtReal39);
}

static void logXYCplx(void) {
  real_t xReal, xImag, yReal, yImag, rReal, rImag;

  if (!getRegisterAsComplex(REGISTER_Y, &yReal, &yImag)
      || !getRegisterAsComplex(REGISTER_X, &xReal, &xImag))
    return;
  if(checkArgs(&xReal, &xImag, &yReal, &yImag)) {
    logXYComplex(&xReal, &xImag, &yReal, &yImag, &rReal, &rImag, &ctxtReal39);
    convertComplexToResultRegister(&rReal, &rImag, REGISTER_X);
  }
}

/********************************************//**convertComplexToResultRegister
 * \brief regX ==> regL and log(regX)(RegY) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnLogXY(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexDyadicFunction(&logXYReal, &logXYCplx, &logXYShortInt, &logXYLongInt);
}
