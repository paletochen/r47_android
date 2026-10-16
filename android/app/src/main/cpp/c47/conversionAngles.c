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
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
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
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
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
        displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnCvtDegToRad:", "cannot use an angle34 not tagged degree as an input of fnCvtDegToRad", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return;
      }
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
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
        displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnCvtRadToDeg:", "cannot use an angle34 not tagged degree as an input of fnCvtRadToDeg", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return;
      }
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
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
        displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnCvtMultPiToRad:", "cannot use an angle34 not tagged mult" STD_pi " as an input of fnCvtMultPiToRad", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
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
        displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnCvtRadToMultPi:", "cannot use an angle34 not tagged radian as an input of fnCvtRadToMultPi", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return;
      }
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
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
        displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnCvtDegToDms:", "cannot use an angle34 not tagged degree as an input of fnCvtDegToDms", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return;
      }
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
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
        displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnCvtDmsToDeg:", "cannot use an angle34 not tagged d.ms as an input of fnCvtDmsToDeg", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
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
        displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnCvtDmsToCurrentAngularMode:", "cannot use an angle34 not tagged d.ms as an input of fnCvtDmsToDeg", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
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



/********************************************//**
 * \brief Replaces an angle in radians with the right angle less that angle, both in the current angular mode
 *
 * \param[in,out] angle angle in radians, returned as ±90° less the angle in the current angular mode
 * \param[in] negative true for -90°
 * \return void
 ***********************************************/
void rightAngleLessRadians(real_t *angle, bool_t negative) {
  real_t rightAngle;

  realCopy(const_90, &rightAngle);
  if(negative) {
    realSetNegativeSign(&rightAngle);
  }
  convertAngleFromTo(&rightAngle, amDegree, currentAngularMode, &ctxtReal39);
  convertAngleFromTo(angle, amRadian, currentAngularMode, &ctxtReal39);
  realSubtract(&rightAngle, angle, angle, &ctxtReal39);
}



void convertAngleFromTo(real_t *angle, angularMode_t fromAngularMode, angularMode_t toAngularMode, realContext_t *realContext) {
  if(fromAngularMode == amNone) {
    fromAngularMode = currentAngularMode;
  }
  if(toAngularMode == amNone) {
    toAngularMode = currentAngularMode;
  }
  bool_t lp  = (realContext->digits > 39);
  bool_t vlp = (realContext->digits > 75);
  switch(fromAngularMode) {
    case amRadian: {
      switch(toAngularMode) {
        case amMultPi: {
          realDivide(angle, vlp ? const1071_pi : (lp ? const75_pi : const39_pi), angle, realContext);
          break;
        }
        case amGrad: {
          if(vlp) {
            realMultiply(angle, const_100, angle, realContext);
            realDivide(  angle, const1071_piOn2, angle, realContext);
          }
          else if(lp) {
            realMultiply(angle, const_100, angle, realContext);
            realDivide(  angle, const75_piOn2, angle, realContext);
          }
          else {
            realMultiply(angle, const39_200onPi, angle, realContext);
          }
          break;
        }
        case amDegree:
        case amDMS: {
          if(vlp) {
            realMultiply(angle, const_90, angle, realContext);
            realDivide(  angle, const1071_piOn2, angle, realContext);
          }
          else if(lp) {
            realMultiply(angle, const_90, angle, realContext);
            realDivide(  angle, const75_piOn2, angle, realContext);
          }
          else {
            realMultiply(angle, const39_180onPi, angle, realContext);
          }
          break;
        }
        default: ;
      }
      break;
    }

    case amMultPi: {
      switch(toAngularMode) {
        case amRadian: {
          realMultiply(angle, vlp ? const1071_pi : (lp ? const75_pi : const39_pi), angle, realContext);
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
          if(vlp) {
            realMultiply(angle, const1071_piOn2, angle, realContext);
            realDivide(  angle, const_100, angle, realContext);
          }
          else if(lp) {
            realMultiply(angle, const75_piOn2, angle, realContext);
            realDivide(  angle, const_100, angle, realContext);
          }
          else {
            realDivide(angle, const39_200onPi, angle, realContext);
          }
          break;
        }
        case amMultPi: {
          realDivide(angle, const_200, angle, realContext);
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
          if(vlp) {
            realMultiply(angle, const1071_piOn2, angle, realContext);
            realDivide(  angle, const_90, angle, realContext);
          }
          else if(lp) {
            realMultiply(angle, const75_piOn2, angle, realContext);
            realDivide(  angle, const_90, angle, realContext);
          }
          else {
            realDivide(angle, const39_180onPi, angle, realContext);
          }
          break;
        }
        case amMultPi: {
          realDivide(angle, const_180, angle, realContext);
          break;
        }
        case amGrad: {
          realDivide(angle, const_9on10, angle, realContext);
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



uint32_t getInfiniteComplexAngle(const cplx_t *z) {
  if(!realIsInfinite(&z->Real)) {
    return 2 + 4*realIsNegative(&z->Imag); // 2 -> 90°    6 -> -90° or 270°
  }

  if(!realIsInfinite(&z->Imag)) {
    return 4*realIsNegative(&z->Real);  // 0 -> 0°     4 -> -180° or 180°
  }

  // At this point, x and y are infinite
  if(realIsPositive(&z->Real)) {
    return 1 + 6*realIsNegative(&z->Imag); // 1 -> 45°    7 -> -45° or 315°
  }

  // At this point, x is negative
  return 3 + 2*realIsNegative(&z->Imag);  // 3 -> 135°   5 -> -135° or 225°
}



void setInfiniteComplexAngle(uint32_t angle, cplx_t *z) {
  switch(angle) {
    case 3:
    case 4:
    case 5: realSetMinusInfinity(&z->Real); break;

    case 2:
    case 6: realSetZero(&z->Real);          break;

    default:realSetPlusInfinity(&z->Real);
  }

  switch(angle) {
    case 5:
    case 6:
    case 7: realSetMinusInfinity(&z->Imag); break;

    case 0:
    case 4: realSetZero(&z->Imag);          break;

    default:realSetPlusInfinity(&z->Imag);
  }
}



/********************************************//**
 * \brief Converts a real from DMS to DEG
 *
 * \param[in] angleDms Angle in ddd.mm.ss format
 * \param[out] angleDec Angle in decimal degrees
 * \param[in] realContext Arithmetic context
 * \return void
 ***********************************************/
void realFromDmsToDeg(const real_t *angleDms, real_t *angleDec, realContext_t *realContext) {
  real_t angle, degrees, minutes, seconds;
  bool_t negative = realIsNegative(angleDms);

  if(realIsSpecial(angleDms)) {
    realCopy(angleDms, angleDec);
    return;
  }

  realCopy(angleDms, &angle);
  realSetPositiveSign(&angle);

  realToIntegralValue(&angle, &degrees, DEC_ROUND_DOWN, realContext);

  realSubtract(&angle, &degrees, &angle, realContext);
  angle.exponent += 2; // angle = angle * 100

  realToIntegralValue(&angle, &minutes, DEC_ROUND_DOWN, realContext);

  realSubtract(&angle, &minutes, &angle, realContext);
  realMultiply(&angle, const_100, &seconds, realContext);

  if(realCompareGreaterEqual(&seconds, const_60)) {
    realSubtract(&seconds, const_60, &seconds, realContext);
    realAdd(&minutes, const_1, &minutes, realContext);
  }

  if(realCompareGreaterEqual(&minutes, const_60)) {
    realSubtract(&minutes, const_60, &minutes, realContext);
    realAdd(&degrees, const_1, &degrees, realContext);
  }

  realDivide(&minutes, const_60,   &minutes, realContext);
  realDivide(&seconds, const_3600, &seconds, realContext);

  realAdd(&degrees, &minutes, &angle, realContext);
  realAdd(&angle,   &seconds, &angle, realContext);

  if(negative) {
    realSetNegativeSign(&angle);
  }

  realCopy(&angle, angleDec);
}



/********************************************//**
 * \brief Converts a real from DEG to DMS
 *
 * \param[in] angleDec Angle in decimal degrees
 * \param[out] angleDms Angle in ddd.mm.ss format
 * \param[in] realContext Arithmetic context
 * \return void
 ***********************************************/
void realFromDegToDms(const real_t *angleDec, real_t *angleDms, realContext_t *realContext) {
  real_t angle, degrees, minutes, seconds;
  bool_t negative = realIsNegative(angleDec);

  if(realIsSpecial(angleDec)) {
    realCopy(angleDec, angleDms);
    return;
  }

  realCopy(angleDec, &angle);
  realSetPositiveSign(&angle);

  realToIntegralValue(&angle, &degrees, DEC_ROUND_DOWN, realContext);

  realSubtract(&angle, &degrees, &angle, realContext);
  realMultiply(&angle, const_60, &angle, realContext);

  realToIntegralValue(&angle, &minutes, DEC_ROUND_DOWN, realContext);

  realSubtract(&angle, &minutes, &angle, realContext);
  realMultiply(&angle, const_60, &seconds, realContext);

  minutes.exponent -= 2; // minutes = minutes / 100
  seconds.exponent -= 4; // seconds = seconds / 10000

  realAdd(&degrees, &minutes, &angle, realContext);
  realAdd(&angle,   &seconds, &angle, realContext);

  if(negative) {
    realSetNegativeSign(&angle);
  }

  realCopy(&angle, angleDms);
}



/********************************************//**
 * \brief Converts a real34 from DMS to DEG
 *
 * \param[in]  angleDms real34_t* Real to be conveted to DEG, the format of the real must be ddd.mm.ssfffffff e.g. 12.345678 = 12°34'56.78"
 * \param[out] angleDec real34_t* Converted real, from the example above the result is 12.58243888888888888888888888888889°
 * \return void
 ***********************************************/
void real34FromDmsToDeg(const real34_t *angleDms, real34_t *angleDec) {
  real_t angle;

  real34ToReal(angleDms, &angle);
  decContextClearStatus(&ctxtReal34, DEC_Invalid_operation);
  realFromDmsToDeg(&angle, &angle, &ctxtReal39);
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
  real_t angle;

  real34ToReal(angleDec, &angle);
  realFromDegToDms(&angle, &angle, &ctxtReal39);
  realToReal34(&angle, angleDms);

  checkDms34(angleDms);
}
