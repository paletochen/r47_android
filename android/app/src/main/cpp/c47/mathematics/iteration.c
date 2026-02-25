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
  real34_t tmpval;

  if(getRegisterDataType(regist) == dtReal34) {
    real34ToIntegralValue(REGISTER_REAL34_DATA(regist), fp, DEC_ROUND_DOWN);
    real34Subtract(REGISTER_REAL34_DATA(regist), fp, fp);
    real34SetPositiveSign(fp);
    int32ToReal34(1000, &tmpval);
    real34Multiply(fp, &tmpval, target);
    real34Copy(target, step);
    real34ToIntegralValue(target, target, DEC_ROUND_DOWN);
    real34Subtract(step, target, step);
    int32ToReal34(100, &tmpval);
    real34Multiply(step, &tmpval, step);
    real34ToIntegralValue(step, step, DEC_ROUND_DOWN);
    if(real34IsZero(step)) {
      real34Copy(const34_1, step);
    }
  }
  else {
    real34Copy(const34_0, fp);
    real34Copy(const34_0, target);
    real34Copy(const34_1, step);
  }
}


static void incDecAndCompare(uint16_t regist, uint16_t mode) {
  real34_t fp, step;
  int8_t compared;

  reallocateRegister(TEMP_REGISTER_1, dtReal34, 0, amNone);
  getIterParam(regist, &fp, REGISTER_REAL34_DATA(TEMP_REGISTER_1), &step);
  switch(getRegisterDataType(regist)) {
    case dtLongInteger: {
      incDecLonI(regist, mode >> 2);
      registerCmp(regist, TEMP_REGISTER_1, &compared);
      break;
    }
    case dtShortInteger: {
      incDecShoI(regist, mode >> 2);
      registerCmp(regist, TEMP_REGISTER_1, &compared);
      break;
    }
    case dtReal34: {
      if((mode & 2) == 2) {
        real34Copy(const34_1, &step);
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
    /* fallthrough */
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
  displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, regist);
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
