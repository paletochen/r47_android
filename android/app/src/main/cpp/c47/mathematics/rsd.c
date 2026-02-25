// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file rsd.c
 ***********************************************/

#include "c47.h"

TO_QSPI void (* const Rsd[NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS])(uint16_t) = {
// regX ==> 1            2        3         4        5         6         7          8           9             10
//          Long integer Real34   Complex34 Time     Date      String    Real34 mat Complex34 m Short integer Config data
            rsdError,    rsdReal, rsdCplx,  rsdTime, rsdError, rsdError, rsdRema,   rsdCxma,    rsdError,     rsdError
};



/********************************************//**
 * \brief calculate RSD
 *
 * \param[in] source real_t*
 * \param[out] destination real_t*
 * \param[in] realContext realContext_t*
 * \param[in] digits uint16_t
 * \return void
 ***********************************************/
//Modified this function to not use temporary variables, in order to transparantly work with 159 and 1071 digits as well

void roundToSignificantDigits(const real_t *source, real_t *destination, uint16_t digits, realContext_t *realContext) {
  real_t sign, output;
  int32_t exponent;
  if(realIsZero(source) || realIsSpecial(source)) {
    if(source != destination) {
      realCopy(source, destination);
    }
    return;
  }
  if(source != destination) {
    realCopy(source, destination);
  }
  exponent = destination->digits + destination->exponent - 1;
  destination->exponent -= exponent + 1;
  while(1) { // in case of subnormal
    realCompare(destination, const_0, &sign, &ctxtReal4);
    if(realIsNegative(&sign)) {
      realCompare(destination, const_1on10, &output, realContext);
      if(realIsPositive(&output)) {
        ++destination->exponent;
        --exponent;
      }
      else {
        break;
      }
    }
    else {
      realCompare(destination, const_1on10, &output, realContext);
      if(realIsNegative(&output)) {
        ++destination->exponent;
        --exponent;
      }
      else {
        break;
      }
    }
  }
  destination->exponent += digits;
  realToIntegralValue(destination, destination, roundingModeTable[roundingMode], realContext);
  destination->exponent -= digits;
  destination->exponent += exponent + 1;
}



/********************************************//**
 * \brief Data type error in rsd
 *
 * \param unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void rsdError(uint16_t unusedButMandatoryParameter) {
  displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    sprintf(errorMessage, "cannot calculate RSD for %s", getRegisterDataTypeName(REGISTER_X, true, false));
    moreInfoOnError("In function rsdError:", errorMessage, NULL, NULL);
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
}



/********************************************//**
 * \brief round to given significant digits
 *
 * \param[in] digits uint16_t
 * \return void
 ***********************************************/
void fnRsd(uint16_t digits) {
  if(!saveLastX()) {
    return;
  }

  Rsd[getRegisterDataType(REGISTER_X)](digits);

  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
}



void senaryDigitToDecimal(bool_t pre_grouped, real_t *val, realContext_t *realContext) {
  real_t sixtieths, st;
  realDivide(const_100, const_60, &st, realContext);

  if(pre_grouped) {
    realDivideRemainder(val, const_10, &sixtieths, realContext);
  }
  else {
    realCopy(val, &sixtieths);
  }

  realToIntegralValue(&sixtieths, &sixtieths, DEC_ROUND_DOWN, realContext);
  realSubtract(val, &sixtieths, val, realContext);
  realMultiply(&sixtieths, &st, &sixtieths, realContext);
  realToIntegralValue(&sixtieths, &sixtieths, DEC_ROUND_DOWN, realContext);
  realAdd(val, &sixtieths, val, realContext);
}

void decimalDigitToSenary(bool_t pre_grouped, real_t *val, realContext_t *realContext) {
  real_t sixtieths, st;
  realDivide(const_60, const_100, &st, realContext);

  if(pre_grouped) {
    realDivideRemainder(val, const_10, &sixtieths, realContext);
  }
  else {
    realCopy(val, &sixtieths);
  }

  realToIntegralValue(&sixtieths, &sixtieths, DEC_ROUND_DOWN, realContext);
  realSubtract(val, &sixtieths, val, realContext);
  realMultiply(&sixtieths, &st, &sixtieths, realContext);
  realToIntegralValue(&sixtieths, &sixtieths, DEC_ROUND_UP, realContext);
  realAdd(val, &sixtieths, val, realContext);
}



void rsdTime(uint16_t digits) {
  real_t val;
  int32_t i;

  updateDisplayValueX = true;
  displayValueX[0] = 0;
  refreshRegisterLine(REGISTER_X);
  updateDisplayValueX = false;

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &val);

  for(i = 0; i < 2; ++i){
    val.exponent -= 1;
    senaryDigitToDecimal(false, &val, &ctxtReal39);
    val.exponent -= 1;
  }
  roundToSignificantDigits(&val, &val, digits, &ctxtReal39);
  for(i = 0; i < 2; ++i){
    val.exponent += 1;
    decimalDigitToSenary(false, &val, &ctxtReal39);
    val.exponent += 1;
  }

  realToReal34(&val, REGISTER_REAL34_DATA(REGISTER_X));
}



void rsdRema(uint16_t digits) {
  elementwiseRema_UInt16(rsdReal, digits);
}



void rsdCxma(uint16_t digits) {
  elementwiseCxma_UInt16(rsdCplx, digits);
}



void rsdReal(uint16_t digits) {
  real_t val;

  if(getRegisterAngularMode(REGISTER_X) == amDMS) {
    real34FromDegToDms(REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
    checkDms34(REGISTER_REAL34_DATA(REGISTER_X));
  }

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &val);
  roundToSignificantDigits(&val, &val, digits, &ctxtReal39);
  convertRealToReal34ResultRegister(&val, REGISTER_X);

  if(getRegisterAngularMode(REGISTER_X) == amDMS) {
    real34FromDmsToDeg(REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
  }
}
/*void rsdReal(uint16_t digits) {
  real_t val;
  int32_t i;

  updateDisplayValueX = true;
  displayValueX[0] = 0;
  refreshRegisterLine(REGISTER_X);
  updateDisplayValueX = false;

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &val);
  if(getRegisterAngularMode(REGISTER_X) == amDMS) {
    for(i = 0; i < 2; ++i){
      val.exponent += 1;
      senaryDigitToDecimal(true, &val, &ctxtReal39);
      val.exponent += 1;
    }
    val.exponent -= 4;
  }
  roundToSignificantDigits(&val, &val, digits, &ctxtReal39);
  if(getRegisterAngularMode(REGISTER_X) == amDMS) {
    for(i = 0; i < 2; ++i){
      val.exponent += 1;
      decimalDigitToSenary(true, &val, &ctxtReal39);
      val.exponent += 1;
    }
    val.exponent -= 4;
    convertAngleFromTo(&val, amDMS, amDMS, &ctxtReal39);
  }
  convertRealToReal34ResultRegister(&val, REGISTER_X);
}*/



void rsdCplx(uint16_t digits) {
  if(getSystemFlag(FLAG_POLAR)) {
    real_t magnitude, theta;
    real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &magnitude);
    real34ToReal(REGISTER_IMAG34_DATA(REGISTER_X), &theta);
    realRectangularToPolar(&magnitude, &theta, &magnitude, &theta, &ctxtReal39);
    roundToSignificantDigits(&magnitude, &magnitude, digits, &ctxtReal39);
    roundToSignificantDigits(&theta,     &theta,     digits, &ctxtReal39);
    realPolarToRectangular(&magnitude, &theta, &magnitude, &theta, &ctxtReal39);
    convertComplexToResultRegister(&magnitude, &theta, REGISTER_X);
  }
  else {
    real_t real, imaginary;
    real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &real);
    real34ToReal(REGISTER_IMAG34_DATA(REGISTER_X), &imaginary);
    roundToSignificantDigits(&real,      &real,      digits, &ctxtReal39);
    roundToSignificantDigits(&imaginary, &imaginary, digits, &ctxtReal39);
    convertComplexToResultRegister(&real, &imaginary, REGISTER_X);
  }
}
