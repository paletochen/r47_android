// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file iteration.c
 ***********************************************/

#include "c47.h"

#define ITER_ISE ((INC_FLAG << 2) | 0)
#define ITER_ISG ((INC_FLAG << 2) | 1)
#define ITER_ISZ ((INC_FLAG << 2) | 2)
#define ITER_DSE ((DEC_FLAG << 2) | 0)
#define ITER_DSL ((DEC_FLAG << 2) | 1)
#define ITER_DSZ ((DEC_FLAG << 2) | 2)

static void getIterParam(uint16_t regist, real34_t *fp, real34_t *target, real34_t *step) {
  if(getRegisterDataType(regist) == dtReal34) {
    real34ToIntegralValue(REGISTER_REAL34_DATA(regist), fp, DEC_ROUND_DOWN);
    real34Subtract(REGISTER_REAL34_DATA(regist), fp, fp);
    real34SetPositiveSign(fp);
    real34Multiply(fp, const34_1000, target);
    real34Copy(target, step);
    real34ToIntegralValue(target, target, DEC_ROUND_DOWN);
    real34Subtract(step, target, step);
    real34Multiply(step, const34_100, step);
    real34ToIntegralValue(step, step, DEC_ROUND_DOWN);
    if(real34IsZero(step)) {
      real34SetOne(step);
    }
  }
  else {
    real34SetZero(fp);
    real34SetZero(target);
    real34SetOne(step);
  }
}


static void incDecAndCompare(uint16_t regist, uint16_t mode) {
  real34_t fp, step;
  int8_t compared;
  const uint32_t dataType = getRegisterDataType(regist);

  // The decoded ccccccc.fffii parameters and the TEMP_REGISTER_1 comparand are
  // not needed on every path, and this runs once per loop iteration of every
  // counted program loop. A long-integer counter always compares against 0
  // with step 1 (getIterParam's non-real34 branch returns exactly that), and
  // ISZ and DSZ on a real34 or time counter set their own step and compare
  // against a constant below. Skip the reallocation and the decode for those.
  if(dataType != dtLongInteger && !(((mode & 2) == 2) && (dataType == dtReal34 || dataType == dtTime))) {
    reallocateRegister(TEMP_REGISTER_1, dtReal34, 0, amNone);
    getIterParam(regist, &fp, REGISTER_REAL34_DATA(TEMP_REGISTER_1), &step);
  }
  switch(dataType) {
    case dtLongInteger: {
      // registerCmp against a real34 zero reduces to the sign of the counter,
      // which the long-integer register tag already holds.
      incDecLonI(regist, mode >> 2);
      const uint32_t sign = getRegisterLongIntegerSign(regist);
      compared = (sign == LI_ZERO) ? 0 : ((sign == LI_POSITIVE) ? 1 : -1);
      break;
    }
    case dtShortInteger: {
      incDecShoI(regist, mode >> 2);
      registerCmp(regist, TEMP_REGISTER_1, &compared);
      break;
    }
    case dtReal34: {
      if((mode & 2) == 2) {
        real34SetOne(&step);
        incDecReal(regist, mode >> 2);
        compared = real34CompareAbsLessThan(REGISTER_REAL34_DATA(regist), const34_1) ? 0 : 1;
        break;
      }
      else if(getRegisterAngularMode(regist) == amNone) {
        real34ToIntegralValue(REGISTER_REAL34_DATA(regist), REGISTER_REAL34_DATA(regist), DEC_ROUND_DOWN);
        if((mode >> 2) == DEC_FLAG) {
          real34SetNegativeSign(&step);
        }
        real34Add(REGISTER_REAL34_DATA(regist), &step, REGISTER_REAL34_DATA(regist));
        registerCmp(regist, TEMP_REGISTER_1, &compared);
        if(real34IsNegative(REGISTER_REAL34_DATA(regist))) {
          real34SetNegativeSign(&fp);
        }
        real34Add(REGISTER_REAL34_DATA(regist), &fp, REGISTER_REAL34_DATA(regist));
        break;
      }
      __attribute__((fallthrough));
    }
    case dtTime: {
      if((mode & 2) == 2) {  // DSZ / ISZ : count by whole hours (similar /3600 used by MAX/MIN and compares =?)
        real_t v;
        real34ToReal(REGISTER_REAL34_DATA(regist), &v);                                                                     // seconds
        (mode >> 2) == DEC_FLAG ? realSubtract(&v, const_3600, &v, &ctxtReal39) : realAdd(&v, const_3600, &v, &ctxtReal39); // step = 1 hour = 3600 s
        realToReal34(&v, REGISTER_REAL34_DATA(regist));
        compared = realCompareAbsLessThan(&v, const_3600) ? 0 : 1;                                                          // |time| < 1h -> treat as zero
        break;
      }
      goto invalidType;      // ISG/DSE/ISE/DSL ccccccc.fffii counter format has no meaning for Time
    }
    default: {
      goto invalidType;
    }
  }
  if(compared > 0) {
    SET_TI_TRUE_FALSE(!((mode & 6) == (INC_FLAG << 2)));
  }
  else if(compared < 0) {
    SET_TI_TRUE_FALSE(!((mode & 6) == (DEC_FLAG << 2)));
  }
  else { // compared==0
    SET_TI_TRUE_FALSE( ((mode & 1) !=              0 ));
  }
  return;

invalidType:
  displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    sprintf(errorMessage, "incompatible type for iterator.");
    moreInfoOnError("In function incDecAndCompare:", errorMessage, NULL, NULL);
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  return;
}



void fnIse(uint16_t regist) {
  incDecAndCompare(regist, ITER_ISE);
}

void fnIsg(uint16_t regist) {
  incDecAndCompare(regist, ITER_ISG);
}

void fnIsz(uint16_t regist) {
  incDecAndCompare(regist, ITER_ISZ);
}

void fnDse(uint16_t regist) {
  incDecAndCompare(regist, ITER_DSE);
}

void fnDsl(uint16_t regist) {
  incDecAndCompare(regist, ITER_DSL);
}

void fnDsz(uint16_t regist) {
  incDecAndCompare(regist, ITER_DSZ);
}


#undef ITER_ISE
#undef ITER_ISG
#undef ITER_ISZ
#undef ITER_DSE
#undef ITER_DSL
#undef ITER_DSZ
