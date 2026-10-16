// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file round.c
 ***********************************************/

#include "c47.h"

TO_QSPI void (* const Round[NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS])(void) = {
// regX ==> 1            2          3          4          5          6           7          8           9             10
//          Long integer Real34     Complex34  Time       Date       String      Real34 mat Complex34 m Short integer Config data
            doNothing,   roundReal, roundCplx, roundTime, roundDate, roundError, roundRema, roundCxma,  doNothing,    roundError
};

static bool_t roundSpecial(const real34_t *x) {
  uint16_t errorCode;

  if(!real34IsSpecial(x)) {
    return false;
  }

  if(!getSystemFlag(FLAG_SPCRES)) {
    if(real34IsNaN(x)) {
      errorCode = ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN;
    }
    else {
      errorCode = real34IsNegative(x) ? ERROR_OVERFLOW_MINUS_INF : ERROR_OVERFLOW_PLUS_INF;
    }
    displayCalcErrorMessage(errorCode, ERR_REGISTER_LINE);
  }
  return true;
}



/********************************************//**
 * \brief Data type error in round
 *
 * \param void
 * \return void
 ***********************************************/
#if (EXTRA_INFO_ON_CALC_ERROR == 1)
  void roundError(void) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
    sprintf(errorMessage, "cannot calculate ROUND for %s", getRegisterDataTypeName(REGISTER_X, true, false));
    moreInfoOnError("In function roundError:", errorMessage, NULL, NULL);
  }
#endif // (EXTRA_INFO_ON_CALC_ERROR == 1)



/********************************************//**
 * \brief regX ==> regL and round(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnRound(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }

  Round[getRegisterDataType(REGISTER_X)]();

  adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
}



void roundTime(void) {
  real_t real;

  if(roundSpecial(REGISTER_REAL34_DATA(REGISTER_X))) {
    return;
  }

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &real);

  switch(timeDisplayFormatDigits) {
    case 0: { // no rounding
      return;
    }
    case 1:
    case 2: { // round to minutes
      real_t sixty;
      int32ToReal(60, &sixty);                                     // exponent 0: const_60 is stored as 6E+1, which gives the seconds exponent 1
      realDivide(&real, const_60, &real, &ctxtReal39);
      realToIntegralValue(&real, &real, DEC_ROUND_DOWN, &ctxtReal39);
      realMultiply(&real, &sixty, &real, &ctxtReal39);
      break;
    }
    default: { // round to seconds, milliseconds, microseconds, ...
      // round to timeDisplayFormatDigits decimal places via scaleB (exponent shift)
      int32_t n = timeDisplayFormatDigits - 3;
      real.exponent += n;
      realToIntegralValue(&real, &real, roundingModeTable[displayRoundingMode], &ctxtReal39);   // the last unit is rounded by DRM, as ROUND rounds a number
      real.exponent -= n;
    }
  }

  realToReal34(&real, REGISTER_REAL34_DATA(REGISTER_X));
}



void roundDate(void) {
  // For the case accidentally added fractions of a day. It should not occur.
  real_t real, offset;

  if(roundSpecial(REGISTER_REAL34_DATA(REGISTER_X))) {
    return;
  }

  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &real);
  int32ToReal(43200, &offset);

  realSubtract(&real, &offset, &real, &ctxtReal39);
  realDivide(&real, const_86400, &real, &ctxtReal39);

  realToIntegralValue(&real, &real, roundingModeTable[roundingMode], &ctxtReal39);

  realMultiply(&real, const_86400, &real, &ctxtReal39);
  realAdd(&real, &offset, &real, &ctxtReal39);

  realToReal34(&real, REGISTER_REAL34_DATA(REGISTER_X));
}



void roundRema(void) {
  if(!getSystemFlag(FLAG_SPCRES)) {
    real34Matrix_t x;
    uint32_t elements;

    linkToRealMatrixRegister(REGISTER_X, &x);
    elements = x.header.matrixRows * x.header.matrixColumns;
    for(uint32_t i = 0; i < elements; i++) {
      if(roundSpecial(x.matrixElements + i)) {
        return;
      }
    }
  }
  elementwiseRema(roundReal);
}



void roundCxma(void) {
  if(!getSystemFlag(FLAG_SPCRES)) {
    complex34Matrix_t x;
    uint32_t elements;

    linkToComplexMatrixRegister(REGISTER_X, &x);
    elements = x.header.matrixRows * x.header.matrixColumns;
    for(uint32_t i = 0; i < elements; i++) {
      if(roundSpecial(VARIABLE_REAL34_DATA(x.matrixElements + i)) || roundSpecial(VARIABLE_IMAG34_DATA(x.matrixElements + i))) {
        return;
      }
    }
  }
  elementwiseCxma(roundCplx);
}



void roundReal(void) {
  if(roundSpecial(REGISTER_REAL34_DATA(REGISTER_X))) {
    return;
  }

  updateDisplayValueX = true;
  displayValueX[0] = 0;
  refreshRegisterLine(REGISTER_X);
  updateDisplayValueX = false;

  if(getSystemFlag(FLAG_FRACT)) {
    int16_t endOfIntegerPart, slashPos;
    real_t value, numerator, denominator;

    endOfIntegerPart = -1;
    if(getSystemFlag(FLAG_PROPFR) && strchr(displayValueX, ' ') != NULL) { // a b/c; below 1 the line shows d/c
      while(displayValueX[++endOfIntegerPart] != ' ') {
      }
      displayValueX[endOfIntegerPart] = 0;
      stringToReal(displayValueX, &value, &ctxtReal39);
    }
    else { // FT_IMPROPER d/c
      realSetZero(&value);
      if(displayValueX[0] == '-') {
        endOfIntegerPart = 0;                                            // the numerator starts after the minus sign
      }
    }

    slashPos = endOfIntegerPart++;
    while(displayValueX[++slashPos] != '/') {
    }
    displayValueX[slashPos++] = 0;
    int32ToReal(stringToInt32(displayValueX + endOfIntegerPart), &numerator);
    int32ToReal(stringToInt32(displayValueX + slashPos), &denominator);
    realMultiply(&value, &denominator, &value, &ctxtReal39);           // a b/c is (a c + b)/c: exact integers, divided once
    if(displayValueX[0] == '-') {
      realSubtract(&value, &numerator, &value, &ctxtReal39);
    }
    else {
      realAdd(&value, &numerator, &value, &ctxtReal39);
    }
    realDivide(&value, &denominator, &value, &ctxtReal34);
    realToReal34(&value, REGISTER_REAL34_DATA(REGISTER_X));
  }
  else {
    stringToReal34(displayValueX, REGISTER_REAL34_DATA(REGISTER_X));
  }
}



void roundCplx(void) {
  int32_t pos, posI;
  bool_t polar = false;

  if(roundSpecial(REGISTER_REAL34_DATA(REGISTER_X)) || roundSpecial(REGISTER_IMAG34_DATA(REGISTER_X))) {
    return;
  }

  updateDisplayValueX = true;
  displayValueX[0] = 0;
  refreshRegisterLine(REGISTER_X);
  updateDisplayValueX = false;

  posI = DISPLAY_VALUE_LEN - 1;
  pos = 0;
  while(displayValueX[pos] != 0) {
    if(displayValueX[pos] == 'i') {
      posI = pos;
      break;
    }
    pos++;
  }

  if(posI == DISPLAY_VALUE_LEN - 1) {
    pos = 0;
    while(displayValueX[pos] != 0) {
      if(displayValueX[pos] == 'j') {
        posI = pos;
        polar = true;
        break;
      }
      pos++;
    }
  }

  displayValueX[posI++] = 0;
  if(polar) {
    cplx_t z;

    stringToReal(displayValueX,        &z.Mag, &ctxtReal39);
    stringToReal(displayValueX + posI, &z.Arg, &ctxtReal39);
    convertAngleFromTo(&z.Arg, getComplexRegisterAngularMode(REGISTER_X), amRadian, &ctxtReal39);   // the angle is shown in the unit of the register
    realPolarToRectangular(&z.Mag, &z.Arg, &z, &ctxtReal39);
    realToReal34(&z.Real, REGISTER_REAL34_DATA(REGISTER_X));
    realToReal34(&z.Imag, REGISTER_IMAG34_DATA(REGISTER_X));
  }
  else {
    stringToReal34(displayValueX,        REGISTER_REAL34_DATA(REGISTER_X));
    stringToReal34(displayValueX + posI, REGISTER_IMAG34_DATA(REGISTER_X));
  }
}
