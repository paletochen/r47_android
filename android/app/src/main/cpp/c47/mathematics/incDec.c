// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file incDec.c
 ***********************************************/

#include "c47.h"



TO_QSPI void (* const incDec[NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS])(uint16_t, uint8_t) = {
// reg ==>   1            2           3           4            5            6            7            8            9              10
//           Long integer Real34      Complex34   Time         Date         String       Real34 mat   Complex34 m  Short integer  Config data
             incDecLonI,  incDecReal, incDecCplx, incDecError, incDecError, incDecError, incDecError, incDecError, incDecShoI,    incDecError
};



void incDecError(uint16_t regist, uint8_t flag) {
  displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, regist);

  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    sprintf(errorMessage, "Cannot increment/decrement, incompatible type.");
    moreInfoOnError("In function incDecError:", errorMessage, NULL, NULL);
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
}



/********************************************//**
 * \brief Decrement a register
 *
 * \param[in] registerNumber uint16_t
 * \return void
 ***********************************************/
void fnDec(uint16_t regist) {
  if(regist < FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters) {
    incDec[getRegisterDataType(regist)](regist, DEC_FLAG);
  }

  #if defined(PC_BUILD)
    else {
      sprintf(errorMessage, "local register .%02d", regist - FIRST_LOCAL_REGISTER);
      moreInfoOnError("In function fnDec:", errorMessage, "is not defined!", NULL);
    }
  #endif // PC_BUILD
}



/********************************************//**
 * \brief Increment a register
 *
 * \param[in] registerNumber uint16_t
 * \return void
 ***********************************************/
void fnInc(uint16_t regist) {
  if(regist < FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters) {
    incDec[getRegisterDataType(regist)](regist, INC_FLAG);
  }

  #if defined(PC_BUILD)
    else {
      sprintf(errorMessage, "local register .%02d", regist - FIRST_LOCAL_REGISTER);
      moreInfoOnError("In function fnInc:", errorMessage, "is not defined!", NULL);
    }
  #endif // PC_BUILD
}



void incDecLonI(uint16_t regist, uint8_t flag) {
  longInteger_t r;

  convertLongIntegerRegisterToLongInteger(regist, r);

  (flag == INC_FLAG) ? longIntegerAddUInt(r, 1, r) : longIntegerSubtractUInt(r, 1, r);

  convertLongIntegerToLongIntegerRegister(r, regist);

  longIntegerFree(r);
}



void incDecReal(uint16_t regist, uint8_t flag) {
  real_t r;

  real34ToReal(REGISTER_REAL34_DATA(regist), &r);
  (flag == INC_FLAG) ? realAdd(&r, const_1, &r, &ctxtReal39) : realSubtract(&r, const_1, &r, &ctxtReal39);

  realToReal34(&r, REGISTER_REAL34_DATA(regist));
}



void incDecCplx(uint16_t regist, uint8_t flag) {
  real_t r_real;

  real34ToReal(REGISTER_REAL34_DATA(regist), &r_real);

  (flag == INC_FLAG) ? realAdd(&r_real, const_1, &r_real, &ctxtReal39) : realSubtract(&r_real, const_1, &r_real, &ctxtReal39);

  realToReal34(&r_real, REGISTER_REAL34_DATA(regist));
}



void incDecShoI(uint16_t regist, uint8_t flag) {
  int16_t r_sign;
  uint64_t r_value;

  convertShortIntegerRegisterToUInt64(regist, &r_sign, &r_value);

  if(r_sign) { // regiter regist negative
    (flag != INC_FLAG) ? r_value++ : r_value--;
  }
  else { // register regist positive
    (flag == INC_FLAG) ? r_value++ : r_value--;
  }

  convertUInt64ToShortIntegerRegister(r_sign, r_value, getRegisterTag(regist), regist);
}
