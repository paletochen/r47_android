// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file linpol.c
 ***********************************************/

#include "c47.h"

/*
 * Linear interpolation between a and b by an amount p.
 * p = 0 returns a, p = 1 returns b.
 *
 * There are two basic formulae for this but both have issues.
 *     linpol = a + p * (b - a) has cancellation issues but is monotonic
 *     linpol = (1 - p) * a + p * b is only monotonic if a * b < 0
 * we work in high precision and special case both
 */
void linpol(const real_t *a, const real_t *b, const real_t *p, real_t *res) {
  real_t x;

  if(realIsNaN(a) || realIsNaN(b) || realIsNaN(p) || realIsInfinite(p)) {
    realCopy(const_NaN, res);
  }
  else if(realIsInfinite(a)) {
    if(realIsInfinite(b)) {    // both infinite, either NaN or one of them
      if(realIsNegative(a) == realIsNegative(b)) {
        realCopy(a, res);
      }
      else {
        realCopy(const_NaN, res);
      }
    }
    else {
      realCopy(a, res);         // a only is infinite, return a
    }
  }
  else if(realIsInfinite(b)) {
    realCopy(b, res);           // b only is infinite, return b
  }
  else if(realCompareEqual(a, b)) {
    realCopy(a, res);           // both same, return one
  }
  else if(realIsNegative(a) != realIsNegative(b)) {
    /* a * b < 0, use linpol = (1 - p) * a + p * b */
    realCopy(p, &x);
    realChangeSign(&x);
    realFMA(&x, a, a, &x, &ctxtReal75);   // a - p * a
    realFMA(p, b, &x, res, &ctxtReal75);  // p * b + (a - p * a)
  }
  else {
    /* a * b > 0, use linpol = a + p * (b - a) */
    realSubtract(b, a, &x, &ctxtReal75);
    realFMA(&x, p, a, res, &ctxtReal75);
  }
}

/********************************************//**
 * \brief (p, b, a) ==> (r) p ==> regL
 * enables stack lift and refreshes the stack
 * Valid input: Errors if X is not Real/LInt
 * Valid input: Real/ShInt/LInt/Cplx, Real/ShInt/LInt/Cplx, Real/LInt (errors if Y & Z are not Real/ShInt/LInt/Cplx)
 * Valid input: Time, Time, Real/LInt (errors if both Y & Z are not Time)
 * Valid input: Angle, Angle, Real/LInt (errors if both Y & Z are not Angle: Y & Z can be different angle types. If angle types are the same, output is in that type,otherwise angle type is ADM)
 * Valid input: Dates not supported as it does not make sense in the granularity of one day
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnLINPOL(uint16_t unusedButMandatoryParameter) {
  real_t aReal, bReal, aImag, bImag, p, rReal, rImag;
  bool_t realCoefs = true;
  uint32_t dataTypeY, dataTypeZ;
  uint16_t dataTagY, dataTagZ;

  realZero(&aImag);
  realZero(&bImag);

  switch(getRegisterDataType(REGISTER_X)) {
    case dtReal34: {
      real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &p);
      break;
    }
    case dtLongInteger: {
      convertLongIntegerRegisterToReal(REGISTER_X, &p, &ctxtReal75);
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "cannot LINPOL with %s in X", getRegisterDataTypeName(REGISTER_X, true, false));
        moreInfoOnError("In function fnLINPOL:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }

  dataTypeY = getRegisterDataType(REGISTER_Y);
  dataTypeZ = getRegisterDataType(REGISTER_Z);
  bool_t isYangle = (dataTypeY == dtReal34) && (getRegisterAngularMode(REGISTER_Y) != amNone);
  bool_t isZangle = (dataTypeZ == dtReal34) && (getRegisterAngularMode(REGISTER_Z) != amNone);

  if((dataTypeY != dataTypeZ && (dataTypeY == dtTime || dataTypeZ == dtTime)) ||                         //if any one is time, both must be time
     (isYangle && !isZangle) || (isZangle && !isYangle)                                                  //if any one is an angle, both must be any angle
    ) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_Y);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot LINPOL with differing data types in Y (%s) and Z (%s)", getRegisterDataTypeName(REGISTER_Y, true, false), getRegisterDataTypeName(REGISTER_Z, true, false));
      moreInfoOnError("In function fnLINPOL:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  dataTagY = amNone;
  dataTagZ = amNone;

  switch(dataTypeY) {
    case dtLongInteger: {
      convertLongIntegerRegisterToReal(REGISTER_Y, &bReal, &ctxtReal75);
      break;
    }

    case dtShortInteger: {
      convertShortIntegerRegisterToReal(REGISTER_Y, &bReal, &ctxtReal39);
      break;
    }

    case dtReal34: {
      real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &bReal);
      dataTagY = getRegisterAngularMode(REGISTER_Y);
     break;
    }

    case dtTime: {
      convertTimeRegisterToReal34Register(REGISTER_Y, REGISTER_Y);
      real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &bReal);
      break;
    }

    case dtDate: {
      // convertDateRegisterToReal34Register is not appropriate here
      internalDateToJulianDay(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_Y));
      real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &bReal);
      break;
    }

    case dtComplex34: {
      real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &bReal);
      real34ToReal(REGISTER_IMAG34_DATA(REGISTER_Y), &bImag);
      realCoefs = false;
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_Y);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "cannot LINPOL with %s in Y", getRegisterDataTypeName(REGISTER_Y, true, false));
        moreInfoOnError("In function fnLINPOL:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }

  switch(dataTypeZ) {
    case dtLongInteger: {
      convertLongIntegerRegisterToReal(REGISTER_Z, &aReal, &ctxtReal75);
      break;
    }

    case dtShortInteger: {
      convertShortIntegerRegisterToReal(REGISTER_Z, &aReal, &ctxtReal39);
      break;
    }

    case dtReal34: {
      real34ToReal(REGISTER_REAL34_DATA(REGISTER_Z), &aReal);
      dataTagZ = getRegisterAngularMode(REGISTER_Z);
      break;
    }

    case dtTime: {
      convertTimeRegisterToReal34Register(REGISTER_Z, REGISTER_Z);
      real34ToReal(REGISTER_REAL34_DATA(REGISTER_Z), &aReal);
      break;
    }

    case dtDate: {
      // convertDateRegisterToReal34Register is not appropriate here
      internalDateToJulianDay(REGISTER_REAL34_DATA(REGISTER_Z), REGISTER_REAL34_DATA(REGISTER_Z));
      real34ToReal(REGISTER_REAL34_DATA(REGISTER_Z), &aReal);
      break;
    }

    case dtComplex34: {
      real34ToReal(REGISTER_REAL34_DATA(REGISTER_Z), &aReal);
      real34ToReal(REGISTER_IMAG34_DATA(REGISTER_Z), &aImag);
      realCoefs = false;
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_Z);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "cannot LINPOL with %s in Z", getRegisterDataTypeName(REGISTER_Z, true, false));
        moreInfoOnError("In function fnLINPOL:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }

  if(!saveLastX()) {
    return;
  }

  adjustResult(REGISTER_X, false, true, REGISTER_X, REGISTER_Y, REGISTER_Z);
  fnDrop(NOPARAM);
  fnDrop(NOPARAM);

  linpol(&aReal, &bReal, &p, &rReal);
  if(realCoefs) {
    if(dataTypeY == dtTime) {
      reallocateRegister(REGISTER_X, dtReal34, 0, amNone);      // use standard Real type for time, to change to Time below
      convertRealToReal34ResultRegister(&rReal, REGISTER_X);
      convertReal34RegisterToTimeRegister(REGISTER_X, REGISTER_X);
    }
    else if(dataTypeY == dtDate) {
      reallocateRegister(REGISTER_X, dtDate, 0, amNone);
      realToReal34(&rReal, REGISTER_REAL34_DATA(REGISTER_X));
      real34ToIntegralValue(REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X), DEC_ROUND_CEILING);
      julianDayToInternalDate(REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_X));
    }
    else {
      if(isYangle && isZangle) {                                          // arrives here if both are angles
        if(dataTagY != dataTagZ) {
          dataTagY = currentAngularMode;
        } else {
          //   dataTagY will be used                                      //   arrives here if both are angles but the angle tags are not the same
        }
      } else {                                                            // arrives here if both are not angles
        dataTagY = amNone;
      }
      reallocateRegister(REGISTER_X, dtReal34, 0, dataTagY);   // use the type and tag of b (Y)
      convertRealToReal34ResultRegister(&rReal, REGISTER_X);
    }
  }
  else {
    linpol(&aImag, &bImag, &p, &rImag);
    reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
    convertComplexToResultRegister(&rReal, &rImag, REGISTER_X);
  }
}
