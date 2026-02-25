// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file conversionAngles.c
 ***********************************************/

#include "c47.h"

void fnCvtToCurrentAngularMode(uint16_t fromAngularMode) {
  if(!saveLastX()) {
    return;
  }

  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
      convertAngle34FromTo(REGISTER_REAL34_DATA(REGISTER_X), fromAngularMode, currentAngularMode);
      setRegisterAngularMode(REGISTER_X, currentAngularMode);
      break;
    }

    case dtReal34: {
      convertAngle34FromTo(REGISTER_REAL34_DATA(REGISTER_X), fromAngularMode, currentAngularMode);
      setRegisterAngularMode(REGISTER_X, currentAngularMode);
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "%s cannot be converted to an angle!", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function fnCvtToCurrentAngularMode:", "the input value must be a long integer, a real34 or an angle34", errorMessage, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
  }
}
}



void fnCvtFromCurrentAngularModeRegister(calcRegister_t regist1, uint16_t toAngularMode) {
    if(!saveLastX()) {
      return;
    }

  switch(getRegisterDataType(regist1)) {
    case dtLongInteger: {
      convertLongIntegerRegisterToReal34Register(regist1, regist1);

     convertAngle34FromTo(REGISTER_REAL34_DATA(regist1), currentAngularMode, toAngularMode);
      setRegisterAngularMode(regist1, toAngularMode);
      break;
    }

    case dtReal34: {
      if(currentAngularMode == amDMS && getRegisterAngularMode(regist1) == amNone) {
        real34FromDmsToDeg(REGISTER_REAL34_DATA(regist1), REGISTER_REAL34_DATA(regist1));
        setRegisterAngularMode(regist1, amDegree);
      }

      convertAngle34FromTo(REGISTER_REAL34_DATA(regist1), getRegisterAngularMode(regist1) == amNone ? currentAngularMode : getRegisterAngularMode(regist1), toAngularMode);
      setRegisterAngularMode(regist1, toAngularMode);
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, regist1);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "%s cannot be converted to an angle!", getRegisterDataTypeName(regist1, true, false));
        moreInfoOnError("In function fnCvtFromCurrentAngularModeRegister:", "the input value must be a long integer, a real34 or an angle16 or an angle34", errorMessage, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
  }
}
}


void fnCvtFromCurrentAngularMode(uint16_t toAngularMode) {          //JM converted to used generic version including register
  fnCvtFromCurrentAngularModeRegister(REGISTER_X, toAngularMode);
}

/*
void fnCvtDegToRad(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }

  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
      convertAngle34FromTo(REGISTER_REAL34_DATA(REGISTER_X), amDegree, amRadian);
      setRegisterAngularMode(REGISTER_X, amRadian);
      break;
    }

    case dtReal34: {
      if(getRegisterAngularMode(REGISTER_X) == amDegree || getRegisterAngularMode(REGISTER_X) == amDMS || getRegisterAngularMode(REGISTER_X) == amNone) {
        convertAngle34FromTo(REGISTER_REAL34_DATA(REGISTER_X), amDegree, amRadian);
        setRegisterAngularMode(REGISTER_X, amRadian);
      }
      else {
        displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnCvtDegToRad:", "cannot use an angle34 not tagged degree as an input of fnCvtDegToRad", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return;
      }
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "%s cannot be converted to an angle!", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function fnCvtDegToRad:", "the input value must be a real34 or a long integer", errorMessage, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
}


void fnCvtRadToDeg(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }

  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
      convertAngle34FromTo(REGISTER_REAL34_DATA(REGISTER_X), amRadian, amDegree);
      setRegisterAngularMode(REGISTER_X, amDegree);
      break;
    }

    case dtReal34: {
      if(getRegisterAngularMode(REGISTER_X) == amRadian || getRegisterAngularMode(REGISTER_X) == amNone) {
        convertAngle34FromTo(REGISTER_REAL34_DATA(REGISTER_X), amRadian, amDegree);
        setRegisterAngularMode(REGISTER_X, amDegree);
      }
      else {
        displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnCvtRadToDeg:", "cannot use an angle34 not tagged degree as an input of fnCvtRadToDeg", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return;
      }
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "%s cannot be converted to an angle!", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function fnCvtRadToDeg:", "the input value must be a real34 or a long integer", errorMessage, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
}



void fnCvtMultPiToRad(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }

  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
      setRegisterAngularMode(REGISTER_X, amRadian);
      break;
    }

    case dtReal34: {
      if(getRegisterAngularMode(REGISTER_X) == amNone) {
        real34FromDmsToDeg(REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
        setRegisterAngularMode(REGISTER_X, amRadian);
      }
      else if(getRegisterAngularMode(REGISTER_X) == amMultPi) {
        setRegisterAngularMode(REGISTER_X, amRadian);
      }
      else {
        displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnCvtMultPiToRad:", "cannot use an angle34 not tagged mult" STD_pi " as an input of fnCvtMultPiToRad", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "%s cannot be converted to an angle!", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function fnCvtMultPiToRad:", "the input value must be a real34 or a long integer", errorMessage, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
}



void fnCvtRadToMultPi(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }

  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
      setRegisterAngularMode(REGISTER_X, amMultPi);
      break;
    }

    case dtReal34: {
      if(getRegisterAngularMode(REGISTER_X) == amRadian || getRegisterAngularMode(REGISTER_X) == amNone) {
        setRegisterAngularMode(REGISTER_X, amMultPi);
      }
      else {
        displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnCvtRadToMultPi:", "cannot use an angle34 not tagged radian as an input of fnCvtRadToMultPi", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return;
      }
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "%s cannot be converted to an angle!", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function fnCvtRadToMultPi:", "the input value must be a real34 or a long integer", errorMessage, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
}



void fnCvtDegToDms(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }

  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
      setRegisterAngularMode(REGISTER_X, amDMS);
      break;
    }

    case dtReal34: {
      if(getRegisterAngularMode(REGISTER_X) == amDegree || getRegisterAngularMode(REGISTER_X) == amNone) {
        setRegisterAngularMode(REGISTER_X, amDMS);
      }
      else {
        displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnCvtDegToDms:", "cannot use an angle34 not tagged degree as an input of fnCvtDegToDms", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return;
      }
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "%s cannot be converted to an angle!", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function fnCvtDegToDms:", "the input value must be a real34 or a long integer", errorMessage, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }
}



void fnCvtDmsToDeg(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }

  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
      setRegisterAngularMode(REGISTER_X, amDegree);
      break;
    }

    case dtReal34: {
      if(getRegisterAngularMode(REGISTER_X) == amNone) {
        real34FromDmsToDeg(REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
        setRegisterAngularMode(REGISTER_X, amDegree);
      }
      else if(getRegisterAngularMode(REGISTER_X) == amDMS) {
        setRegisterAngularMode(REGISTER_X, amDegree);
      }
      else {
        displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnCvtDmsToDeg:", "cannot use an angle34 not tagged d.ms as an input of fnCvtDmsToDeg", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "%s cannot be converted to an angle!", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function fnCvtDmsToDeg:", "the input value must be a real34 or a long integer", errorMessage, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
}



void fnCvtDmsToCurrentAngularMode(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }

  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
      convertAngle34FromTo(REGISTER_REAL34_DATA(REGISTER_X), amDMS, currentAngularMode);
      setRegisterAngularMode(REGISTER_X, currentAngularMode);
      break;
    }

    case dtReal34: {
      if(getRegisterAngularMode(REGISTER_X) == amNone) {
        real34FromDmsToDeg(REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
        convertAngle34FromTo(REGISTER_REAL34_DATA(REGISTER_X), amDMS, currentAngularMode);
        setRegisterAngularMode(REGISTER_X, currentAngularMode);
      }
      else if(getRegisterAngularMode(REGISTER_X) == amDMS) {
        convertAngle34FromTo(REGISTER_REAL34_DATA(REGISTER_X), amDMS, currentAngularMode);
        setRegisterAngularMode(REGISTER_X, currentAngularMode);
      }
      else {
        displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnCvtDmsToCurrentAngularMode:", "cannot use an angle34 not tagged d.ms as an input of fnCvtDmsToDeg", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "%s cannot be converted to an angle!", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function fnCvtDmsToCurrentAngularMode:", "the input value must be a real34 or a long integer", errorMessage, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
}

*/


void convertAngle34FromTo(real34_t *angle34, angularMode_t fromAngularMode, angularMode_t toAngularMode) {
  real_t angle;

  real34ToReal(angle34, &angle);
  convertAngleFromTo(&angle, fromAngularMode, toAngularMode, &ctxtReal39);
  realToReal34(&angle, angle34);
}



void convertAngleFromTo(real_t *angle, angularMode_t fromAngularMode, angularMode_t toAngularMode, realContext_t *realContext) {
  if(fromAngularMode == amNone)
    fromAngularMode = currentAngularMode;
  if(toAngularMode == amNone)
    toAngularMode = currentAngularMode;

  switch(fromAngularMode) {
    case amRadian: {
      switch(toAngularMode) {
        case amMultPi: {
          realDivide(angle, const_pi, angle, realContext);
          break;
        }
        case amGrad: {
          realMultiply(angle, const_200onPi, angle, realContext);
          break;
        }
        case amDegree:
        case amDMS: {
          realMultiply(angle, const_180onPi, angle, realContext);
          break;
        }
        default: ;
      }
      break;
    }

    case amMultPi: {
      switch(toAngularMode) {
        case amRadian: {
          realMultiply(angle, const_pi, angle, realContext);
          break;
        }
        case amGrad: {
          realMultiply(angle, const_200, angle, realContext);
          break;
        }
        case amDegree:
        case amDMS: {
          realMultiply(angle, const_180, angle, realContext);
          break;
        }
        default: ;
      }
      break;
    }

    case amGrad: {
      switch(toAngularMode) {
        case amRadian: {
          realDivide(  angle, const_200onPi, angle, realContext);
          break;
        }
        case amMultPi: {
          realDivide(  angle, const_200, angle, realContext);
          break;
        }
        case amDegree:
        case amDMS: {
          realMultiply(angle, const_9on10, angle, realContext);
          break;
        }
        default: ;
      }
      break;
    }

    case amDegree:
    case amDMS: {
      switch(toAngularMode) {
        case amRadian: {
          realDivide(  angle, const_180onPi, angle, realContext);
          break;
        }
        case amMultPi: {
          realDivide(  angle, const_180, angle, realContext);
          break;
        }
        case amGrad: {
          realDivide(  angle, const_9on10,   angle, realContext);
          break;
        }
        default: ;
      }
      break;
    }

    default: ;
  }
}



void checkDms34(real34_t *angle34Dms) {
  int16_t  sign;
  real_t angleDms, degrees, minutes, seconds;

  real34ToReal(angle34Dms, &angleDms);

  sign = realIsNegative(&angleDms) ? -1 : 1;
  realSetPositiveSign(&angleDms);

  realToIntegralValue(&angleDms, &degrees, DEC_ROUND_DOWN, &ctxtReal39);
  realSubtract(&angleDms, &degrees, &angleDms, &ctxtReal39);

  angleDms.exponent += 2; // angleDms = angleDms * 100
  realToIntegralValue(&angleDms, &minutes, DEC_ROUND_DOWN, &ctxtReal39);
  realSubtract(&angleDms, &minutes, &angleDms, &ctxtReal39);

  realMultiply(&angleDms, const_100, &seconds, &ctxtReal39);

  if(realCompareGreaterEqual(&seconds, const_60)) {
    realSubtract(&seconds, const_60, &seconds, &ctxtReal39);
    realAdd(&minutes, const_1, &minutes, &ctxtReal39);
  }

  if(realCompareGreaterEqual(&minutes, const_60)) {
    realSubtract(&minutes, const_60, &minutes, &ctxtReal39);
    realAdd(&degrees, const_1, &degrees, &ctxtReal39);
  }

  minutes.exponent -= 2; // minutes = minutes / 100
  realAdd(&degrees, &minutes, &angleDms, &ctxtReal39);
  seconds.exponent -= 4; // seconds = seconds / 10000
  realAdd(&angleDms, &seconds, &angleDms, &ctxtReal39);

  if(sign == -1) {
    realSetNegativeSign(&angleDms);
  }

  realToReal34(&angleDms, angle34Dms);
}



uint32_t getInfiniteComplexAngle(real_t *x, real_t *y) {
  if(!realIsInfinite(x)) {
    if(realIsPositive(y)) {
      return 2; // 90°
    }
    else {
      return 6; // -90° or 270°
    }
  }

  if(!realIsInfinite(y)) {
    if(realIsPositive(x)) {
      return 0; // 0°
    }
    else {
      return 4; // -180° or 180°
    }
  }

  // At this point, x and y are infinite
  if(realIsPositive(x)) {
    if(realIsPositive(y)) {
      return 1; // 45°
    }
    else {
      return 7; // -45° or 315°
    }
  }

  // At this point, x is negative
  if(realIsPositive(y)) {
    return 3; // 135°
  }
  else {
    return 5; // -135° or 225°
  }
}



void setInfiniteComplexAngle(uint32_t angle, real_t *x, real_t *y) {
  switch(angle) {
    case 3:
    case 4:
    case 5: {
      realCopy(const_minusInfinity, x);
             break;
    }

    case 2:
    case 6: {
      realZero(x);
             break;
    }

    default: {
      realCopy(const_plusInfinity, x);
    }
  }

  switch(angle) {
    case 5:
    case 6:
    case 7: {
      realCopy(const_minusInfinity, y);
             break;
    }

    case 0:
    case 4: {
      realZero(y);
             break;
    }

    default: {
      realCopy(const_plusInfinity, y);
    }
  }
}



/********************************************//**
 * \brief Converts a real34 from DMS to DEG
 *
 * \param[in]  angleDms real34_t* Real to be conveted to DEG, the format of the real must be ddd.mm.ssfffffff e.g. 12.345678 = 12°34'56.78"
 * \param[out] angleDec real34_t* Converted real, from the example above the result is 12.58243888888888888888888888888889°
 * \return void
 ***********************************************/
void real34FromDmsToDeg(const real34_t *angleDms, real34_t *angleDec) {
  real_t angle, degrees, minutes, seconds;

  real34ToReal(angleDms, &angle);
  realSetPositiveSign(&angle);

  decContextClearStatus(&ctxtReal34, DEC_Invalid_operation);
  realToIntegralValue(&angle, &degrees, DEC_ROUND_DOWN, &ctxtReal39);

  realSubtract(&angle, &degrees, &angle, &ctxtReal39);
  angle.exponent += 2; // angle = angle * 100

  realToIntegralValue(&angle, &minutes, DEC_ROUND_DOWN, &ctxtReal39);

  realSubtract(&angle, &minutes, &angle, &ctxtReal39);
  realMultiply(&angle, const_100, &seconds, &ctxtReal39);

  if(realCompareGreaterEqual(&seconds, const_60)) {
    realSubtract(&seconds, const_60, &seconds, &ctxtReal39);
    realAdd(&minutes, const_1, &minutes, &ctxtReal39);
  }

  if(realCompareGreaterEqual(&minutes, const_60)) {
    realSubtract(&minutes, const_60, &minutes, &ctxtReal39);
    realAdd(&degrees, const_1, &degrees, &ctxtReal39);
  }

  realDivide(&minutes, const_60,   &minutes, &ctxtReal39);
  realDivide(&seconds, const_3600, &seconds, &ctxtReal39);

  realAdd(&degrees, &minutes, &angle, &ctxtReal39);
  realAdd(&angle,   &seconds, &angle, &ctxtReal39);

  if(real34IsNegative(angleDms)) {
    realSetNegativeSign(&angle);
  }

  realToReal34(&angle, angleDec);
}



/********************************************//**
 * \brief Converts a real34 from DEG to DMS
 *
 * \param[in]  angleDec real34_t* Real to be conveted to DMS, 23.456789°
 * \param[out] angleDms real34_t* Converted real, from the example above the result 23.27244404 = 23°27'24.4404"
 * \return void
 ***********************************************/
void real34FromDegToDms(const real34_t *angleDec, real34_t *angleDms) {
  real_t angle, degrees, minutes, seconds;

  real34ToReal(angleDec, &angle);
  realSetPositiveSign(&angle);

  realToIntegralValue(&angle, &degrees, DEC_ROUND_DOWN, &ctxtReal39);

  realSubtract(&angle, &degrees, &angle, &ctxtReal39);
  realMultiply(&angle, const_60, &angle, &ctxtReal39);

  realToIntegralValue(&angle, &minutes, DEC_ROUND_DOWN, &ctxtReal39);

  realSubtract(&angle, &minutes, &angle, &ctxtReal39);
  realMultiply(&angle, const_60, &seconds, &ctxtReal39);

  minutes.exponent -= 2; // minutes = minutes / 100
  seconds.exponent -= 4; // seconds = seconds / 10000

  realAdd(&degrees, &minutes, &angle, &ctxtReal39);
  realAdd(&angle,   &seconds, &angle, &ctxtReal39);

  if(real34IsNegative(angleDec)) {
    realSetNegativeSign(&angle);
  }

  realToReal34(&angle, angleDms);

  checkDms34(angleDms);
}
