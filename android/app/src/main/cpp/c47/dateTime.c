// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file dateTime.c
 ***********************************************/

#include "c47.h"

void fnSetDateFormat(uint16_t dateFormat) {
  switch(dateFormat) {
    case ITM_DMY: {
      clearSystemFlag(FLAG_MDY);
      clearSystemFlag(FLAG_YMD);
      setSystemFlag(FLAG_DMY);
      break;
    }

    case ITM_MDY: {
      clearSystemFlag(FLAG_DMY);
      clearSystemFlag(FLAG_YMD);
      setSystemFlag(FLAG_MDY);
      break;
    }

    case ITM_YMD: {
      clearSystemFlag(FLAG_MDY);
      clearSystemFlag(FLAG_DMY);
      setSystemFlag(FLAG_YMD);
      break;
    }

    default: {
    }
  }
}

void internalDateToJulianDay(const real34_t *source, real34_t *destination) {
  real34Subtract(source, const34_43200, destination);
  real34Divide(destination, const34_86400, destination), real34ToIntegralValue(destination, destination, DEC_ROUND_FLOOR);
}

void julianDayToInternalDate(const real34_t *source, real34_t *destination) {
  real34ToIntegralValue(source, destination, DEC_ROUND_FLOOR);
  real34Multiply(destination, const34_86400, destination);
  real34Add(destination, const34_43200, destination);
}

bool_t checkDateArgument(calcRegister_t regist, real34_t *jd) {
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
  switch(getRegisterDataType(regist)) {
    case dtDate: {
      internalDateToJulianDay(REGISTER_REAL34_DATA(regist), jd);
      return true;
    }

    case dtReal34: {
      if(getRegisterAngularMode(regist) == amNone) {
        reallocateRegister(TEMP_REGISTER_1, dtReal34, 0, amNone); // make sure TEMP_REGISTER_1 is not of dtDate type here
        convertReal34RegisterToDateRegister(regist, TEMP_REGISTER_1, !YYSystem);
        if(getRegisterDataType(TEMP_REGISTER_1) != dtDate) {
          return false; // invalid date
        }
        internalDateToJulianDay(REGISTER_REAL34_DATA(TEMP_REGISTER_1), jd);
        return true;
      }
      /* fallthrough */
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "data type %s cannot be converted to date!", getRegisterDataTypeName(REGISTER_X, false, false));
        moreInfoOnError("In function checkDateArgument:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return false;
    }
  }
  #pragma GCC diagnostic pop
}

bool_t isLeapYear(const real34_t *year) {
  real34_t val, val2;
  int32_t y400; // year mod 400
  bool_t isGregorian;

  composeJulianDay(year, const34_2, const34_28, &val);
  uInt32ToReal34(firstGregorianDay, &val2);
  isGregorian = real34CompareGreaterEqual(&val, &val2);

  real34Divide(year, const34_400, &val), real34ToIntegralValue(&val, &val, DEC_ROUND_FLOOR);
  real34Multiply(&val, const34_400, &val);
  real34Subtract(year, &val, &val);
  y400 = real34ToInt32(&val);

  if(y400 == 0) {
    return true;
  }
  else if(isGregorian && (y400 % 100 == 0)) {
    return false;
  }

  return (y400 % 4 == 0);
}


bool_t isValidDay(const real34_t *year, const real34_t *month, const real34_t *day) {
  real34_t val;

  // Year (this rejects year -4713 and earlier)
  real34ToIntegralValue(year, &val, DEC_ROUND_FLOOR), real34Subtract(year, &val, &val);
  if(!real34IsZero(&val)) {
    return false;
  }
  real34Compare(year, const34__4712, &val);
  if(real34ToInt32(&val) < 0) {
    return false;
  }

  // Day
  real34ToIntegralValue(day, &val, DEC_ROUND_FLOOR), real34Subtract(day, &val, &val);
  if(!real34IsZero(&val)) {
    return false;
  }
  real34Compare(day, const34_1, &val);
  if(real34ToInt32(&val) < 0) {
    return false;
  }
  real34Compare(day, const34_31, &val);
  if(real34ToInt32(&val) > 0) {
    return false;
  }

  // Month
  real34ToIntegralValue(month, &val, DEC_ROUND_FLOOR), real34Subtract(month, &val, &val);
  if(!real34IsZero(&val)) {
    return false;
  }
  real34Compare(month, const34_1, &val);
  if(real34ToInt32(&val) < 0) {
    return false;
  }
  real34Compare(month, const34_12, &val);
  if(real34ToInt32(&val) > 0) {
    return false;
  }

  // Thirty days hath September...
  if(real34ToInt32(day) == 31) {
    switch(real34ToInt32(month)) {
      case 2:
      case 4:
      case 6:
      case 9:
      case 11: {
        return false;
      }
      default: {
        return true;
    }
  }
  }

  // February
  if(real34ToInt32(month) == 2) {
    if(real34ToInt32(day) == 30) {
      return false;
    }
    else if((real34ToInt32(day) == 29) && (!isLeapYear(year))) {
      return false;
    }
  }

  // Check for Julian-Gregorian gap
  if(firstGregorianDay > 0u) { // Gregorian
    real34_t y, m, d;
    composeJulianDay(year, month, day, &val);
    decomposeJulianDay(&val, &y, &m, &d);
    if(!real34CompareEqual(year, &y) || !real34CompareEqual(month, &m) || !real34CompareEqual(day, &d)) {
      return false;
    }
  }

  // Valid date
  return true;
}


static void divInt(const real34_t *p, const real34_t *q, real34_t *r) {
  real34_t tmp;
  real34Divide(p, q, &tmp), real34ToIntegralValue(&tmp, r, DEC_ROUND_DOWN);
}
static void divInt2(const real34_t *p, const real34_t *q, real34_t *r) {
  real34_t tmp;
  real34Divide(p, q, &tmp), real34ToIntegralValue(&tmp, r, DEC_ROUND_FLOOR);
}
static void modInt(const real34_t *p, const real34_t *q, real34_t *r) {
  real34_t tmp;
  divInt2(p, q, &tmp), real34Multiply(&tmp, q, &tmp), real34Subtract(p, &tmp, r);
}

void composeJulianDay(const real34_t *year, const real34_t *month, const real34_t *day, real34_t *jd) {
  real34_t fg, y, m, d;

  uInt32ToReal34(firstGregorianDay, &fg);
  real34Copy(year, &y); real34Copy(month, &m); real34Copy(day, &d);
  composeJulianDay_g(&y, &m, &d, jd);
  if((firstGregorianDay > 0u) && real34CompareLessThan(jd, &fg)) {
    composeJulianDay_j(&y, &m, &d, jd);
  }
}

// Gregorian
void composeJulianDay_g(const real34_t *year, const real34_t *month, const real34_t *day, real34_t *jd) {
  real34_t m_14_12; // round_down((month - 14) / 12)
  real34_t a;

  // round_down((month - 14) / 12)
  real34Subtract(month, const34_14, &m_14_12);
  divInt(&m_14_12, const34_12, &m_14_12);

  real34Add(year, const34_4800, &a);
  real34Add(&a, &m_14_12, &a);
  real34Multiply(&a, const34_1461, &a);
  divInt(&a, const34_4, jd);

  real34Multiply(&m_14_12, const34_12, &a);
  real34Subtract(month, &a, &a);
  real34Subtract(&a, const34_2, &a);
  real34Multiply(&a, const34_367, &a);
  divInt(&a, const34_12, &a);
  real34Add(jd, &a, jd);

  real34Add(year, const34_4900, &a);
  real34Add(&a, &m_14_12, &a);
  divInt(&a, const34_100, &a);
  real34Multiply(&a, const34_3, &a);
  divInt(&a, const34_4, &a);
  real34Subtract(jd, &a, jd);

  real34Add(jd, day, jd);
  real34Subtract(jd, const34_32075, jd);
}

// Julian
void composeJulianDay_j(const real34_t *year, const real34_t *month, const real34_t *day, real34_t *jd) {
  real34_t a;

  real34Multiply(year, const34_367, jd);

  real34Subtract(month, const34_9, &a);
  divInt(&a, const34_7, &a);
  real34Add(&a, year, &a);
  real34Add(&a, const34_5001, &a);
  real34Multiply(&a, const34_7, &a);
  divInt(&a, const34_4, &a);
  real34Subtract(jd, &a, jd);

  real34Multiply(month, const34_275, &a);
  divInt(&a, const34_9, &a);
  real34Add(jd, &a, jd);

  real34Add(jd, day, jd);
  real34Add(jd, const34_1729777, jd);
}

void decomposeJulianDay(const real34_t *jd, real34_t *year, real34_t *month, real34_t *day) {
  real34_t e, h, tmp1;

  real34Add(jd, const34_1401, &e);
  uInt32ToReal34(firstGregorianDay, &tmp1);
  // proleptic Gregorian calendar is used if firstGregorianDay == 0: for special purpose only!
  if((firstGregorianDay == 0u) || real34CompareGreaterEqual(jd, &tmp1)) { // Gregorian
    real34Multiply(jd, const34_4, &tmp1);
    real34Add(&tmp1, const34_274277, &tmp1);
    divInt2(&tmp1, const34_146097, &tmp1);
    real34Multiply(&tmp1, const34_3, &tmp1);
    divInt2(&tmp1, const34_4, &tmp1);
    real34Subtract(&tmp1, const34_38, &tmp1);
    real34Add(&e, &tmp1, &e);
  }

  real34Multiply(&e, const34_4, &e);
  real34Add(&e, const34_3, &e);

  modInt(&e, const34_1461, &h);
  divInt2(&h, const34_4, &h);
  real34Multiply(&h, const34_5, &h);
  real34Add(&h, const34_2, &h);

  modInt(&h, const34_153, day);
  divInt2(day, const34_5, day);
  real34Add(day, const34_1, day);

  divInt2(&h, const34_153, month);
  real34Add(month, const34_2, month);
  modInt(month, const34_12, month);
  real34Add(month, const34_1, month);

  divInt2(&e, const34_1461, year);
  real34Subtract(year, const34_4716, year);
  real34Subtract(const34_14, month, &tmp1);
  divInt2(&tmp1, const34_12, &tmp1);
  real34Add(year, &tmp1, year);
}

uint32_t julianDayToDayOfWeek(real34_t *jd) {
  real34_t dow;
  modInt(jd, const34_7, &dow);
  real34Add(&dow, const34_1, &dow);
  return real34ToUInt32(&dow);
}

uint32_t getJulianDayOfWeek(calcRegister_t regist) {
  real34_t date34;
  if(checkDateArgument(regist, &date34)) {
    return julianDayToDayOfWeek(&date34);
  }
  else {
    return 0u;
  }
}

void checkDateRange(const real34_t *date34) {
  if(real34CompareGreaterEqual(date34, const34_maxDate) || real34IsNegative(date34)) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "value of date type is too large");
      moreInfoOnError("In function checkDateRange:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }
}


void hmmssToSeconds(const real34_t *src, real34_t *dest) {
  real34_t time34, real34, value34;
  int32_t sign;

  sign = real34IsNegative(src);

  // Hours
  real34CopyAbs(src, &time34);
  real34ToIntegralValue(&time34, &real34, DEC_ROUND_DOWN);
  real34Multiply(&real34, const34_3600, dest);

  // Minutes
  real34Subtract(&time34, &real34, &time34);
  real34Multiply(&time34, const34_100, &time34);
  real34ToIntegralValue(&time34, &real34, DEC_ROUND_DOWN);
  real34Multiply(const34_60, &real34, &value34);
  real34Add(dest, &value34, dest);

  // Seconds
  real34Subtract(&time34, &real34, &time34);
  real34Multiply(&time34, const34_100, &time34);
  real34Add(dest, &time34, dest);

  // Sign
  if(sign) {
    real34SetNegativeSign(dest);
  }
}

void hmmssInRegisterToSeconds(calcRegister_t regist) {
  real34_t real34;

  real34Copy(REGISTER_REAL34_DATA(regist), &real34);
  reallocateRegister(regist, dtTime, 0, amNone);
  hmmssToSeconds(&real34, REGISTER_REAL34_DATA(regist));
  checkTimeRange(REGISTER_REAL34_DATA(regist));
}

void checkTimeRange(const real34_t *time34) {
  real34_t t;

  real34CopyAbs(time34, &t);
  if(real34CompareGreaterEqual(&t, const34_maxTime)) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "value of time type is too large");
      moreInfoOnError("In function checkTimeRange:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }
}

uint32_t getWeekOfYear(real34_t *jd) {
  real34_t dsow, sow, rdow;

  uInt32ToReal34(modulo(((int32_t)julianDayToDayOfWeek(jd)) - firstDayOfWeek, 7), &dsow);
  real34Subtract(jd, &dsow, &sow);  // 1st day of the week containing the date

  uInt32ToReal34(modulo((int32_t)firstWeekOfYearDay - firstDayOfWeek, 7), &dsow);
  real34Add(&sow, &dsow, &rdow);    // reference day of the week containing the date

  real34_t y, m, d, j1;
  decomposeJulianDay(&rdow, &y, &m, &d);  // Get the correct year (may be different from the year of jd)
  composeJulianDay(&y, const34_1, const34_1, &j1);  // 1st of january of the correct year

  int32_t dow = modulo((int32_t)julianDayToDayOfWeek(&j1), 7);
  if (firstWeekOfYearDay < dow) {    // if the reference day of the week containing the 1st of january is part of previous year…
    real34Add(&j1, const34_7, &j1);  // … skip to next week
  }

  uInt32ToReal34(modulo((int32_t)(dow - firstDayOfWeek), 7), &dsow);
  real34Subtract(&j1, &dsow, &j1);   // 1st day of the 1st week of the year

  real34_t woy;
  real34Subtract(&sow, &j1, &woy);      // number of days between the 2 dates
  real34Divide(&woy, const34_7, &woy);  // number of weeks
  real34Add(&woy, const34_1, &woy);     // first week has number 1
  return real34ToUInt32(&woy);
}



void fnJulianToDateTime(uint16_t unusedButMandatoryParameter) {
  real34_t date;

  if(!saveLastX()) {
    return;
  }

  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      liftStack();
      convertLongIntegerRegisterToReal34Register(REGISTER_Y, REGISTER_Y);
      julianDayToInternalDate(REGISTER_REAL34_DATA(REGISTER_Y), &date);
      reallocateRegister(REGISTER_Y, dtDate, 0, amNone);
      real34Copy(&date, REGISTER_REAL34_DATA(REGISTER_Y));
      reallocateRegister(REGISTER_X, dtTime, 0, amNone);
      real34Copy(const34_1on2, REGISTER_REAL34_DATA(REGISTER_X)); //Added offset so 0.00 -> 12:00:00
      real34Multiply(REGISTER_REAL34_DATA(REGISTER_X), const34_3600, REGISTER_REAL34_DATA(REGISTER_X));
      real34Multiply(REGISTER_REAL34_DATA(REGISTER_X), const34_24, REGISTER_REAL34_DATA(REGISTER_X));
      break;
    }
    case dtReal34: {
      liftStack();
      real34Add(REGISTER_REAL34_DATA(REGISTER_Y), const34_1on2, REGISTER_REAL34_DATA(REGISTER_Y));       //handle 0.5 offset
      copySourceRegisterToDestRegister(REGISTER_Y, REGISTER_X);

      //Get IP in Y
      reallocateRegister(REGISTER_Y, dtDate, 0, amNone);
      real34ToIntegralValue(REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_Y), DEC_ROUND_DOWN);

      //Get FP in x
      real_t y, x,cc;
      int32ToReal(3600*100*24*10, &cc);                              // 3600*100*24*10 rounds time up or down to 0.001 seconds
      real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y);            // IP
      real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x);            // Org
      realSubtract(&x, &y, &x, &ctxtReal39);                         // FP = Org - IP
      realMultiply(&x, &cc, &x, &ctxtReal39);                        // (3600*100*24*10*(FP))          //round seconds to 0.001s
      realToIntegralValue(&x, &x, DEC_ROUND_HALF_UP, &ctxtReal39);   // round (3600*100*24*10*(FP))
      realDivide  (&x, &cc, &x, &ctxtReal39);                        // (round (3600*100*24*10*(FP))) / (3600*100*24*10)

      //Convert date to Y
      julianDayToInternalDate(REGISTER_REAL34_DATA(REGISTER_Y), &date);
      reallocateRegister(REGISTER_Y, dtDate, 0, amNone);
      real34Copy(&date, REGISTER_REAL34_DATA(REGISTER_Y));

      //Convert x to time to X
      real_t tmp;
      int32ToReal(24*3600, &tmp);
      realMultiply(&tmp, &x, &tmp, &ctxtReal39);                      // tmp is now seconds
      reallocateRegister(REGISTER_X, dtTime, 0, amNone);    // this must be in front of the next line, otherwise accuracy is gone
      convertRealToReal34ResultRegister(&tmp, REGISTER_X);
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "data type %s cannot be converted to date!", getRegisterDataTypeName(REGISTER_X, false, false));
        moreInfoOnError("In function fnJulianToDateTime:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }
  }

  // check range
  checkDateRange(REGISTER_REAL34_DATA(REGISTER_Y));
  if(lastErrorCode != 0) {
    undo();
  }
}



void fnDateToJulian(uint16_t unusedButMandatoryParameter) {
  real34_t jd34;

  if(!saveLastX()) {
    return;
  }

  if(checkDateArgument(REGISTER_X, &jd34)) {
    convertReal34ToLongIntegerRegister(&jd34, REGISTER_X, DEC_ROUND_FLOOR);
  }
}



void fnDateTimeToJulian(uint16_t unusedButMandatoryParameter) {
  real34_t jd34;

  if(!saveLastX()) {
    return;
  }

  if(getRegisterDataType(REGISTER_X) == dtDate && getRegisterDataType(REGISTER_Y) == dtTime) {
    if(checkDateArgument(REGISTER_X, &jd34)) {
      fnSwapXY(0);
    }
  }

  if(checkDateArgument(REGISTER_Y, &jd34) && getRegisterDataType(REGISTER_X) == dtTime) {
    {
      convertReal34ToLongIntegerRegister(&jd34, REGISTER_Y, DEC_ROUND_DOWN);                      //Y is truncated date in real
      convertLongIntegerRegisterToReal34Register(REGISTER_Y, REGISTER_Y);
      timeToReal34(3);                                                                            //X is now Real seconds
      real34_t tmp;
      int32ToReal34 (3600*24,&tmp);
      real34Divide  (REGISTER_REAL34_DATA(REGISTER_X), &tmp, REGISTER_REAL34_DATA(REGISTER_X));   //X is now in days
      real34Add     (REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(REGISTER_Y), REGISTER_REAL34_DATA(REGISTER_X));
      real34Subtract(REGISTER_REAL34_DATA(REGISTER_X), const34_1on2, REGISTER_REAL34_DATA(REGISTER_X));                      //handle 0.5 offset
      adjustResult(REGISTER_X, true, true, REGISTER_X, REGISTER_Y, -1);
    }
  }
}



void fnIsLeap(uint16_t unusedButMandatoryParameter) {
  real34_t y, m, d, j;

  if(checkDateArgument(REGISTER_X, &j)) {
    decomposeJulianDay(&j, &y, &m, &d);
    SET_TI_TRUE_FALSE(isLeapYear(&y));
  }
}

void fnSetFirstGregorianDay(uint16_t param) {
  real34_t jd34;
  const uint32_t fgd = firstGregorianDay;

  if(param != NOPARAM) {
    switch(param) {
      case ITM_JUL_GREG_1752: firstGregorianDay = 2361222; break;    /* 14 Sept 1752 */
      case ITM_JUL_GREG_1949: firstGregorianDay = 2433191; break;    /* 1 Oct   1949 */
      case ITM_JUL_GREG_1582: firstGregorianDay = 2299161; break;    /* 15 Oct  1582 */
      case ITM_JUL_GREG_1873: firstGregorianDay = 2405160; break;    /* 1 Jan   1873 */
      default: break;
    }
    temporaryInformation = TI_DISP_JULIAN;
    return;
  }

  if((getRegisterDataType(REGISTER_X) == dtReal34) && (getRegisterAngularMode(REGISTER_X) == amNone)) {
    firstGregorianDay = 0u; // proleptic Gregorian mode
    if(checkDateArgument(REGISTER_X, &jd34)) {
      firstGregorianDay = real34ToUInt32(&jd34);
    }
    else {
      firstGregorianDay = fgd;
    }
    temporaryInformation = TI_DISP_JULIAN;
  }
  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      if(getRegisterDataType(REGISTER_X) == dtDate) {
        sprintf(errorMessage, "data type %s is disabled as input because of complicated Julian-Gregorian issue!", getRegisterDataTypeName(REGISTER_X, false, false));
      }
      else {
        sprintf(errorMessage, "data type %s cannot be interpreted as a date!", getRegisterDataTypeName(REGISTER_X, false, false));
      }
      moreInfoOnError("In function fnSetFirstGregorianDay:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}

void fnGetFirstGregorianDay(uint16_t unusedButMandatoryParameter) {
  real34_t j;

  uInt32ToReal34(firstGregorianDay, &j);
  liftStack();
  reallocateRegister(REGISTER_X, dtDate, 0, amNone);
  julianDayToInternalDate(&j, REGISTER_REAL34_DATA(REGISTER_X));
}


//return true if bit 14 (16384 0x4000) is set, meaning that the YY default is updated from the last used full YYYY used.
bool_t followYY(void) {
  return lastCenturyHighUsed & 0x4000;
}

void fnYYDflt(uint16_t tmp) {
  if(tmp == YY_TRACKING) {
    lastCenturyHighUsed = 0x4000;
  }
  else if(tmp == YY_OFF) {
    lastCenturyHighUsed = 0x8000;
  }
  else if(tmp < 100) {                                //allow lowest range 0100 -> 0199
    lastCenturyHighUsed = 0;
  }
  else {
    lastCenturyHighUsed = tmp;
  }
}


void fnXToDate(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }

  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
  switch(getRegisterDataType(REGISTER_X)) {
    case dtDate: {
      /* already in date: do nothing */
      break;
    }

    case dtReal34: {
      if(getRegisterAngularMode(REGISTER_X) == amNone) {
        convertReal34RegisterToDateRegister(REGISTER_X, REGISTER_X, !YYSystem);
        checkDateRange(REGISTER_REAL34_DATA(REGISTER_X));
        temporaryInformation = TI_DAY_OF_WEEK;
        if(lastErrorCode != 0) {
          undo();
        }
        break;
      }
      /* fallthrough */
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "data type %s cannot be converted to date!", getRegisterDataTypeName(REGISTER_X, false, false));
        moreInfoOnError("In function fnXToDate:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
  }
}
  #pragma GCC diagnostic pop
}


void fnYear(uint16_t unusedButMandatoryParameter) {
  real34_t y, m, d, j;

  if(!saveLastX()) {
    return;
  }

  if(checkDateArgument(REGISTER_X, &j)) {
    decomposeJulianDay(&j, &y, &m, &d);
    convertReal34ToLongIntegerRegister(&y, REGISTER_X, DEC_ROUND_FLOOR);
  }
}

void fnMonth(uint16_t unusedButMandatoryParameter) {
  real34_t y, m, d, j;

  if(!saveLastX()) {
    return;
  }

  if(checkDateArgument(REGISTER_X, &j)) {
    decomposeJulianDay(&j, &y, &m, &d);
    convertReal34ToLongIntegerRegister(&m, REGISTER_X, DEC_ROUND_FLOOR);
  }
}

void fnDay(uint16_t unusedButMandatoryParameter) {
  real34_t y, m, d, j;

  if(!saveLastX()) {
    return;
  }

  if(checkDateArgument(REGISTER_X, &j)) {
    decomposeJulianDay(&j, &y, &m, &d);
    convertReal34ToLongIntegerRegister(&d, REGISTER_X, DEC_ROUND_FLOOR);
  }
}

void fnWday(uint16_t unusedButMandatoryParameter) {
  const uint32_t dayOfWeek = getJulianDayOfWeek(REGISTER_X);
  longInteger_t result;

  if(!saveLastX()) {
    return;
  }

  if(dayOfWeek != 0) {
    longIntegerInit(result);
    uInt32ToLongInteger(dayOfWeek, result);
    convertLongIntegerToLongIntegerRegister(result, REGISTER_X);
    longIntegerFree(result);
    temporaryInformation = TI_DAY_OF_WEEK;
  }
}

void fnDateTo(uint16_t unusedButMandatoryParameter) {
  real34_t y, m, d, j;

  if(!saveLastX()) {
    return;
  }

  if(checkDateArgument(REGISTER_X, &j)) {
    liftStack();
    liftStack();
    decomposeJulianDay(&j, &y, &m, &d);
    convertReal34ToLongIntegerRegister(&y, getSystemFlag(FLAG_YMD) ? REGISTER_Z :                                        REGISTER_X, DEC_ROUND_FLOOR);
    convertReal34ToLongIntegerRegister(&m, getSystemFlag(FLAG_MDY) ? REGISTER_Z :                                        REGISTER_Y, DEC_ROUND_FLOOR);
    convertReal34ToLongIntegerRegister(&d, getSystemFlag(FLAG_DMY) ? REGISTER_Z : getSystemFlag(FLAG_MDY) ? REGISTER_Y : REGISTER_X, DEC_ROUND_FLOOR);
  }
}

void fnToDate(uint16_t unusedButMandatoryParameter) {
  real34_t y, m, d, j;
  real34_t *part[3];
  calcRegister_t r[3] = {REGISTER_Z, REGISTER_Y, REGISTER_X};
  int32_t i;

  if(!saveLastX()) {
    return;
  }

  if(getSystemFlag(FLAG_DMY)) {
    part[0] = &d;
    part[1] = &m;
    part[2] = &y;
  }
  else if(getSystemFlag(FLAG_MDY)) {
    part[0] = &m;
    part[1] = &d;
    part[2] = &y;
  }
  else {
    part[0] = &y;
    part[1] = &m;
    part[2] = &d;
  }

  for(i = 0; i < 3; ++i) {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
    switch(getRegisterDataType(r[i])) {
      case dtLongInteger: {
        convertLongIntegerRegisterToReal34(r[i], part[i]);
        break;
      }

      case dtReal34: {
        if(getRegisterAngularMode(r[i]) == amNone) {
          real34ToIntegralValue(REGISTER_REAL34_DATA(r[i]), part[i], DEC_ROUND_DOWN);
          break;
        }
        /* fallthrough */
      }

      default: {
        displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "data type %s cannot be converted to a real34!", getRegisterDataTypeName(r[i], false, false));
          moreInfoOnError("In function fnToDate:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return;
    }
  }
    #pragma GCC diagnostic pop
  }

  if(!isValidDay(&y, &m, &d)) {
      displayCalcErrorMessage(ERROR_BAD_TIME_OR_DATE_INPUT, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function fnToDate:", "Invalid date input like 30 Feb.", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
  }

  // valid date
  fnDropY(NOPARAM);
  if(lastErrorCode == ERROR_NONE) {
    fnDropY(NOPARAM);
    if(lastErrorCode == ERROR_NONE) {
      composeJulianDay(&y, &m, &d, &j);
      reallocateRegister(REGISTER_X, dtDate, 0, amNone);
      julianDayToInternalDate(&j, REGISTER_REAL34_DATA(REGISTER_X));

      // check range
      checkDateRange(REGISTER_REAL34_DATA(REGISTER_X));
      temporaryInformation = TI_DAY_OF_WEEK;
    }
  }
  if(lastErrorCode != 0) {
    undo();
  }
}


void fnToHr(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
    return;
  }

  switch(getRegisterDataType(REGISTER_X)) {
    case dtTime: {
      convertTimeRegisterToReal34Register(REGISTER_X, REGISTER_X);
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "data type %s cannot be converted to a real34!", getRegisterDataTypeName(REGISTER_X, false, false));
        moreInfoOnError("In function fnToHr:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
  }
}
}


void fnHMStoTM(uint16_t unusedButMandatoryParameter) {
  if(!saveLastX()) {
      return;
  }

  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
      switch(getRegisterDataType(REGISTER_X)) {
        case dtLongInteger: {
          convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
          break;
        }

        case dtTime: {
          /* already in hours: do nothing */
            return;
        }

        case dtReal34: {
          if(getRegisterAngularMode(REGISTER_X) == amNone) {
            break;
          }
          /* fallthrough */
        }

        default: {
          displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(errorMessage, "data type %s cannot be converted to time!", getRegisterDataTypeName(REGISTER_X, false, false));
            moreInfoOnError("In function fnHMStoTM:", errorMessage, NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          return;
          }
      }
  #pragma GCC diagnostic pop
  hmmssInRegisterToSeconds(REGISTER_X);
  if(lastErrorCode != 0) {
    undo();
  }
}


void fnHRtoTM(uint16_t unusedButMandatoryParameter) {
  switch(calcMode) {                     //JM vv
    case CM_NIM:
      #if !defined(TESTSUITE_BUILD)
        addItemToNimBuffer(ITM_HRtoTM);
      #endif // !TESTSUITE_BUILD
      break;

    default:                             //JM ^^
      if(!saveLastX()) {
          return;
      }
    }

  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
      switch(getRegisterDataType(REGISTER_X)) {
        case dtLongInteger: {
          convertLongIntegerRegisterToTimeRegister(REGISTER_X, REGISTER_X);
          break;
        }

        case dtTime: {
          /* already in hours: do nothing */
            break;
        }

        case dtReal34: {
          if(getRegisterAngularMode(REGISTER_X) == amNone) {
            convertReal34RegisterToTimeRegister(REGISTER_X, REGISTER_X);
            break;
          }
          /* fallthrough */
        }

        default: {
          displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            sprintf(errorMessage, "data type %s cannot be converted to time!", getRegisterDataTypeName(REGISTER_X, false, false));
            moreInfoOnError("In function fnHRtoTM:", errorMessage, NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          return;
          }
      }
  #pragma GCC diagnostic pop
  checkTimeRange(REGISTER_REAL34_DATA(REGISTER_X));
  if(lastErrorCode != 0) {
    undo();
  }
}


void fnDate(uint16_t unusedButMandatoryParameter) {
  real34_t y, m, d, j;

  #if defined(DMCP_BUILD)
    tm_t timeInfo;
    dt_t dateInfo;

    rtc_read(&timeInfo, &dateInfo);
    uInt32ToReal34(dateInfo.year,  &y);
    uInt32ToReal34(dateInfo.month, &m);
    uInt32ToReal34(dateInfo.day,   &d);
  #else // !DMCP_BUILD
    time_t epoch = time(NULL);
    struct tm *timeInfo = localtime(&epoch);

    int32ToReal34(timeInfo->tm_year + 1900, &y);
    int32ToReal34(timeInfo->tm_mon  + 1,    &m);
    int32ToReal34(timeInfo->tm_mday,        &d);
  #endif // DMCP_BUILD

  composeJulianDay(&y, &m, &d, &j);
  liftStack();
  reallocateRegister(REGISTER_X, dtDate, 0, amNone);
  julianDayToInternalDate(&j, REGISTER_REAL34_DATA(REGISTER_X));
  temporaryInformation = TI_DAY_OF_WEEK;
}

void fnTime(uint16_t unusedButMandatoryParameter) {
  real34_t time34;

  #if defined(DMCP_BUILD)
    tm_t timeInfo;
    dt_t dateInfo;

    rtc_read(&timeInfo, &dateInfo);
    uInt32ToReal34((uint32_t)timeInfo.hour * 3600u + (uint32_t)timeInfo.min * 60u + (uint32_t)timeInfo.sec, &time34);
  #else // !DMCP_BUILD
    time_t epoch = time(NULL);
    struct tm *timeInfo = localtime(&epoch);

    uInt32ToReal34((uint32_t)timeInfo->tm_hour * 3600u + (uint32_t)timeInfo->tm_min * 60u + (uint32_t)timeInfo->tm_sec, &time34);
  #endif // DMCP_BUILD

  liftStack();
  reallocateRegister(REGISTER_X, dtTime, 0, amNone);
  real34Copy(&time34, REGISTER_REAL34_DATA(REGISTER_X));
}


void fnSetDate(uint16_t unusedButMandatoryParameter) {
  #ifdef DMCP_BUILD
    cancelFilename = true;
      tm_t timeInfo;
      dt_t dateInfo;
      real34_t j, y, m, d;

      if(checkDateArgument(REGISTER_X, &j)) {
        rtc_read(&timeInfo, &dateInfo);
        decomposeJulianDay(&j, &y, &m, &d);
        dateInfo.year  = real34ToUInt32(&y);
        dateInfo.month = real34ToUInt32(&m);
        dateInfo.day   = real34ToUInt32(&d);
        rtc_write(&timeInfo, &dateInfo);
      }
  #endif //!PC_BUILD
}

void fnSetTime(uint16_t unusedButMandatoryParameter) {
  #ifdef DMCP_BUILD
    cancelFilename = true;
    tm_t timeInfo;
    dt_t dateInfo;
    real34_t time34;
    int32_t timeVal;

    if(getRegisterDataType(REGISTER_X) == dtTime) {
      if(real34IsNegative(REGISTER_REAL34_DATA(REGISTER_X)) || real34IsNaN(REGISTER_REAL34_DATA(REGISTER_X)) || real34CompareGreaterEqual(REGISTER_REAL34_DATA(REGISTER_X), const34_86400)) {
        displayCalcErrorMessage(ERROR_BAD_TIME_OR_DATE_INPUT, ERR_REGISTER_LINE, REGISTER_X);
      }
      else {
        rtc_read(&timeInfo, &dateInfo);
        real34Multiply(REGISTER_REAL34_DATA(REGISTER_X), const34_100, &time34);
        real34ToIntegralValue(&time34, &time34, DEC_ROUND_DOWN);
        timeVal = real34ToInt32(&time34);
        timeInfo.csec =  timeVal         % 100;
        timeInfo.sec  = (timeVal /= 100) %  60;
        timeInfo.min  = (timeVal /=  60) %  60;
        timeInfo.hour = (timeVal /=  60);
        rtc_write(&timeInfo, &dateInfo);
      }
    }
    else if(getRegisterDataType(REGISTER_X) == dtReal34) {
      if(real34IsNegative(REGISTER_REAL34_DATA(REGISTER_X)) || real34IsNaN(REGISTER_REAL34_DATA(REGISTER_X)) || real34CompareGreaterEqual(REGISTER_REAL34_DATA(REGISTER_X), const34_24)) {
        displayCalcErrorMessage(ERROR_BAD_TIME_OR_DATE_INPUT, ERR_REGISTER_LINE, REGISTER_X);
      }
      else {
        rtc_read(&timeInfo, &dateInfo);
        real34Multiply(REGISTER_REAL34_DATA(REGISTER_X), const34_1e6, &time34);
        real34ToIntegralValue(&time34, &time34, DEC_ROUND_DOWN);
        timeVal = real34ToInt32(&time34);
        timeInfo.csec =  timeVal         % 100;
        timeInfo.sec  = (timeVal /= 100) % 100;
        timeInfo.min  = (timeVal /= 100) % 100;
        timeInfo.hour = (timeVal /= 100);
        if(timeInfo.sec <= 59 && timeInfo.min <= 59 && timeInfo.hour <= 23) {
          rtc_write(&timeInfo, &dateInfo);
        }
        else {
          displayCalcErrorMessage(ERROR_BAD_TIME_OR_DATE_INPUT, ERR_REGISTER_LINE, REGISTER_X);
        }
      }
    }
    else {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
    }
  #endif //!PC_BUILD
}


void fnWeekOfYear(uint16_t unusedButMandatoryParameter) {
  real34_t jd34;

  if(!saveLastX()) {
    return;
  }

  if(checkDateArgument(REGISTER_X, &jd34)) {
    uint32_t woy = getWeekOfYear(&jd34);
    longInteger_t li;
    longIntegerInit(li);
    uInt32ToLongInteger(woy, li);
    convertLongIntegerToLongIntegerRegister(li, REGISTER_X);
    temporaryInformation = TI_WOY;
    longIntegerFree(li);
  }
}


void fnSetWeekOfYearRule(uint16_t param) {
  if(param != NOPARAM) {
    switch(param) {
      case ITM_WOY_ISO: firstDayOfWeek = 1; firstWeekOfYearDay = 4; break;
      case ITM_WOY_US:  firstDayOfWeek = 7; firstWeekOfYearDay = 6; break;
      case ITM_WOY_ME:  firstDayOfWeek = 6; firstWeekOfYearDay = 5; break;
      default: break;
    }
    temporaryInformation = TI_DISP_WOY;
  }
}


void fnGetWeekOfYearRule(uint16_t unusedButMandatoryParameter) {
  real34_t fdow, fwoyd;

  uInt32ToReal34(firstDayOfWeek, &fdow);
  uInt32ToReal34(firstWeekOfYearDay, &fwoyd);
  real34Multiply(&fwoyd, const34_1on10, &fwoyd);
  liftStack();
  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
  real34Add(&fdow, &fwoyd, REGISTER_REAL34_DATA(REGISTER_X));
  temporaryInformation = TI_WOY_RULE;
}


void getDateString(char *dateString) {
  #if defined(DMCP_BUILD)
    tm_t timeInfo;
    dt_t dateInfo;

    rtc_read(&timeInfo, &dateInfo);
    if(!getSystemFlag(FLAG_TDM24)) { // 2 digit year
      if(getSystemFlag(FLAG_DMY)) {
        sprintf(dateString, "%02d.%02d.%02d", dateInfo.day, dateInfo.month, dateInfo.year % 100);
      }
      else if(getSystemFlag(FLAG_YMD)) {
        sprintf(dateString, "%02d-%02d-%02d", dateInfo.year % 100, dateInfo.month, dateInfo.day);
      }
      else if(getSystemFlag(FLAG_MDY)) {
        sprintf(dateString, "%02d/%02d/%02d", dateInfo.month, dateInfo.day, dateInfo.year % 100);
      }
      else {
        strcpy(dateString, "?? ?? ????");
      }
    }
    else {// 4 digit year
      if(getSystemFlag(FLAG_DMY)) {
        sprintf(dateString, "%02d.%02d.%04d", dateInfo.day, dateInfo.month, dateInfo.year);
      }
      else if(getSystemFlag(FLAG_YMD)) {
        sprintf(dateString, "%04d-%02d-%02d", dateInfo.year, dateInfo.month, dateInfo.day);
      }
      else if(getSystemFlag(FLAG_MDY)) {
        sprintf(dateString, "%02d/%02d/%04d", dateInfo.month, dateInfo.day, dateInfo.year);
      }
      else {
        strcpy(dateString, "?? ?? ????");
      }
    }
  #else // !DMCP_BUILD
    time_t epoch = time(NULL);
    struct tm *timeInfo = localtime(&epoch);

    // For the format string : man strftime
    if(!getSystemFlag(FLAG_TDM24)) { // time format = 12H ==> 2 digit year
      if(getSystemFlag(FLAG_DMY)) {
        strftime(dateString, 11, "%d.%m.%y", timeInfo);
      }
      else if(getSystemFlag(FLAG_YMD)) {
        strftime(dateString, 11, "%y-%m-%d", timeInfo);
      }
      else if(getSystemFlag(FLAG_MDY)) {
        strftime(dateString, 11, "%m/%d/%y", timeInfo);
      }
      else {
        strcpy(dateString, "?? ?? ????");
      }
    }
    else {// 4 digit year
      if(getSystemFlag(FLAG_DMY)) {
        strftime(dateString, 11, "%d.%m.%Y", timeInfo);
      }
      else if(getSystemFlag(FLAG_YMD)) {
        strftime(dateString, 11, "%Y-%m-%d", timeInfo);
      }
      else if(getSystemFlag(FLAG_MDY)) {
        strftime(dateString, 11, "%m/%d/%Y", timeInfo);
      }
      else {
        strcpy(dateString, "?? ?? ????");
      }
    }
  #endif // DMCP_BUILD
}



void getTimeString(char *timeString) {
  #if defined(DMCP_BUILD)
    tm_t timeInfo;
    dt_t dateInfo;

    rtc_read(&timeInfo, &dateInfo);
    if(!getSystemFlag(FLAG_TDM24)) {
      sprintf(timeString, "%02d:%02d", (timeInfo.hour % 12) == 0 ? 12 : timeInfo.hour % 12, timeInfo.min);
      if(timeInfo.hour >= 12) {
        strcat(timeString, "pm");
      }
      else {
        strcat(timeString, "am");
      }
    }
    else {
      sprintf(timeString, "%02d:%02d", timeInfo.hour, timeInfo.min);
    }
  #else // !DMCP_BUILD
    time_t epoch = time(NULL);
    struct tm *timeInfo = localtime(&epoch);

    // For the format string : man strftime
    if(getSystemFlag(FLAG_TDM24)) { // time format = 24H
      strftime(timeString, 8, "%H:%M", timeInfo); // %R don't work on Windows
    }
    else {
      strftime(timeString, 8, "%I:%M", timeInfo); // I could use %p but in many locals the AM and PM string are empty
      if(timeInfo->tm_hour >= 12) {
        strcat(timeString, "pm");
      }
      else {
        strcat(timeString, "am");
      }
    }
  #endif // DMCP_BUILD
}

void getWeekOfYearString(char *weekOfYearString) {
  real34_t jd, y, m, d;

  #if defined(DMCP_BUILD)
    tm_t timeInfo;
    dt_t dateInfo;

    rtc_read(&timeInfo, &dateInfo);
    uInt32ToReal34(dateInfo.year, &y);
    uInt32ToReal34(dateInfo.month, &m);
    uInt32ToReal34(dateInfo.day, &d);
  #else // !DMCP_BUILD
    time_t epoch = time(NULL);
    struct tm *timeInfo = localtime(&epoch);

    uInt32ToReal34(timeInfo->tm_year + 1900, &y);
    uInt32ToReal34(timeInfo->tm_mon + 1, &m);
    uInt32ToReal34(timeInfo->tm_mday, &d);
  #endif // DMCP_BUILD

  composeJulianDay(&y, &m, &d, &jd);
  uint32_t woy = getWeekOfYear(&jd);
  int32_t dow = julianDayToDayOfWeek(&jd);
  dow = modulo((dow + 7) - firstDayOfWeek, 7) + 1;
  sprintf(weekOfYearString, "W%02d-%d", (int)woy, (int)dow);
}
