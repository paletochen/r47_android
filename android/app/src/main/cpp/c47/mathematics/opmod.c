// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file opmod.c
 ***********************************************/

#include "c47.h"

TO_QSPI void (* const opModL[NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS][NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS])(uint16_t) = {
// regX |    regY ==>   1                  2                  3           4           5           6           7           8           9                  10
//      V               Long integer       Real34             Complex34   Time        Date        String      Real34 mat  Complex34 m Short integer      Config data
/*  1 Long integer  */ {opModLonILonILonI, opModLonIRealLonI, opModError, opModError, opModError, opModError, opModError, opModError, opModLonIShoILonI, opModError},
/*  2 Real34        */ {opModLonILonIReal, opModLonIRealReal, opModError, opModError, opModError, opModError, opModError, opModError, opModLonIShoIReal, opModError},
/*  3 Complex34     */ {opModError,        opModError,        opModError, opModError, opModError, opModError, opModError, opModError, opModError,        opModError},
/*  4 Time          */ {opModError,        opModError,        opModError, opModError, opModError, opModError, opModError, opModError, opModError,        opModError},
/*  5 Date          */ {opModError,        opModError,        opModError, opModError, opModError, opModError, opModError, opModError, opModError,        opModError},
/*  6 String        */ {opModError,        opModError,        opModError, opModError, opModError, opModError, opModError, opModError, opModError,        opModError},
/*  7 Real34 mat    */ {opModError,        opModError,        opModError, opModError, opModError, opModError, opModError, opModError, opModError,        opModError},
/*  8 Complex34 mat */ {opModError,        opModError,        opModError, opModError, opModError, opModError, opModError, opModError, opModError,        opModError},
/*  9 Short integer */ {opModLonILonIShoI, opModLonIRealShoI, opModError, opModError, opModError, opModError, opModError, opModError, opModLonIShoIShoI, opModError},
/* 10 Config data   */ {opModError,        opModError,        opModError, opModError, opModError, opModError, opModError, opModError, opModError,        opModError}
};
TO_QSPI void (* const opModS[NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS][NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS])(uint16_t) = {
// regX |    regY ==>   1                  2                  3           4           5           6           7           8           9                  10
//      V               Long integer       Real34             Complex34   Time        Date        String      Real34 mat  Complex34 m Short integer      Config data
/*  1 Long integer  */ {opModShoILonILonI, opModShoIRealLonI, opModError, opModError, opModError, opModError, opModError, opModError, opModShoIShoILonI, opModError},
/*  2 Real34        */ {opModShoILonIReal, opModShoIRealReal, opModError, opModError, opModError, opModError, opModError, opModError, opModShoIShoIReal, opModError},
/*  3 Complex34     */ {opModError,        opModError,        opModError, opModError, opModError, opModError, opModError, opModError, opModError,        opModError},
/*  4 Time          */ {opModError,        opModError,        opModError, opModError, opModError, opModError, opModError, opModError, opModError,        opModError},
/*  5 Date          */ {opModError,        opModError,        opModError, opModError, opModError, opModError, opModError, opModError, opModError,        opModError},
/*  6 String        */ {opModError,        opModError,        opModError, opModError, opModError, opModError, opModError, opModError, opModError,        opModError},
/*  7 Real34 mat    */ {opModError,        opModError,        opModError, opModError, opModError, opModError, opModError, opModError, opModError,        opModError},
/*  8 Complex34 mat */ {opModError,        opModError,        opModError, opModError, opModError, opModError, opModError, opModError, opModError,        opModError},
/*  9 Short integer */ {opModShoILonIShoI, opModShoIRealShoI, opModError, opModError, opModError, opModError, opModError, opModError, opModShoIShoIShoI, opModError},
/* 10 Config data   */ {opModError,        opModError,        opModError, opModError, opModError, opModError, opModError, opModError, opModError,        opModError}
};
TO_QSPI void (* const opModR[NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS][NUMBER_OF_DATA_TYPES_FOR_CALCULATIONS])(uint16_t) = {
// regX |    regY ==>   1                  2                  3           4           5           6           7           8           9                  10
//      V               Long integer       Real34             Complex34   Time        Date        String      Real34 mat  Complex34 m Short integer      Config data
/*  1 Long integer  */ {opModRealLonILonI, opModRealRealLonI, opModError, opModError, opModError, opModError, opModError, opModError, opModRealShoILonI, opModError},
/*  2 Real34        */ {opModRealLonIReal, opModRealRealReal, opModError, opModError, opModError, opModError, opModError, opModError, opModRealShoIReal, opModError},
/*  3 Complex34     */ {opModError,        opModError,        opModError, opModError, opModError, opModError, opModError, opModError, opModError,        opModError},
/*  4 Time          */ {opModError,        opModError,        opModError, opModError, opModError, opModError, opModError, opModError, opModError,        opModError},
/*  5 Date          */ {opModError,        opModError,        opModError, opModError, opModError, opModError, opModError, opModError, opModError,        opModError},
/*  6 String        */ {opModError,        opModError,        opModError, opModError, opModError, opModError, opModError, opModError, opModError,        opModError},
/*  7 Real34 mat    */ {opModError,        opModError,        opModError, opModError, opModError, opModError, opModError, opModError, opModError,        opModError},
/*  8 Complex34 mat */ {opModError,        opModError,        opModError, opModError, opModError, opModError, opModError, opModError, opModError,        opModError},
/*  9 Short integer */ {opModRealLonIShoI, opModRealRealShoI, opModError, opModError, opModError, opModError, opModError, opModError, opModRealShoIShoI, opModError},
/* 10 Config data   */ {opModError,        opModError,        opModError, opModError, opModError, opModError, opModError, opModError, opModError,        opModError}
};


void opModError(uint16_t mode) {
  displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    sprintf(errorMessage, "cannot MOD %s %s %s", getRegisterDataTypeName(REGISTER_Z, true, false), mode == OPMOD_POWER ? "^" : "*", getRegisterDataTypeName(REGISTER_Y, true, false));
    sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "by %s", getRegisterDataTypeName(REGISTER_X, true, false));
    moreInfoOnError("In function fnOpMod:", errorMessage, errorMessage + ERROR_MESSAGE_LENGTH/2, NULL);
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
}


void opModOutOfDomain(uint16_t mode) {
  displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    sprintf(errorMessage, "At least one of the arguments is out of the domain: Z > 0, Y > 0, X > 1");
    moreInfoOnError("In function fnOpMod:", errorMessage, NULL, NULL);
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
}


void fnMulMod(uint16_t unusedButMandatoryParameter) {
  fnOpMod(OPMOD_MULTIPLY);
}


void fnExpMod(uint16_t unusedButMandatoryParameter) {
  fnOpMod(OPMOD_POWER);
}


void fnOpMod(uint16_t mode) {
  if(!saveLastX()) {
    return;
  }


  switch(getRegisterDataType(REGISTER_Z)) {
    case dtLongInteger: {
      opModL[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)](mode);
      break;
    }
    case dtShortInteger: {
      opModS[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)](mode);
      break;
    }
    case dtReal34: {
      opModR[getRegisterDataType(REGISTER_X)][getRegisterDataType(REGISTER_Y)](mode);
      break;
    }
    default: {
      opModError(mode);
    }
  }
  adjustResult(REGISTER_X, true, false, REGISTER_X, REGISTER_Y, REGISTER_Z);
  if(lastErrorCode == ERROR_NONE) {
    fnDropY(NOPARAM);
  }
}


static void parseReal(calcRegister_t regist, longInteger_t longIntVal, int32_t *exponent) {
  real_t r;

  real34ToReal(REGISTER_REAL34_DATA(regist), &r);

  if(r.exponent < 0) {
    *exponent = -r.exponent;
    r.exponent = 0;
  }
  longIntegerInit(longIntVal);
  convertRealToLongInteger(&r, longIntVal, DEC_ROUND_FLOOR);
}


static void toRealResult(longInteger_t longIntVal, int32_t exponent) {
  real_t r;

  convertLongIntegerToReal(longIntVal, &r, &ctxtReal75);
  r.exponent -= exponent;
  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
  convertRealToReal34ResultRegister(&r, REGISTER_X);

}


// Adapted from WP34s integer-mode function to long integer
void longInteger_mulmod(const longInteger_t a, int32_t exp_a, const longInteger_t b, int32_t exp_b, const longInteger_t c, int32_t exp_c, longInteger_t res, int32_t *exp_res) {
  longInteger_t x, y, aa, bb, cc;
  int32_t i;

  if(exp_res) {
    *exp_res = exp_a + exp_b;
  }

  longIntegerInit(x);
  longIntegerInit(y);
  longIntegerInit(aa);
  longIntegerInit(bb);
  longIntegerInit(cc);

  longIntegerCopy(a, aa);
  longIntegerCopy(c, cc);
  for(i = 0; i < (exp_c - exp_b - exp_a); ++i) {
    longIntegerMultiplyUInt(aa, 10, aa);
  }
  for(i = 0; i < (exp_a + exp_b - exp_c); ++i) {
    longIntegerMultiplyUInt(cc, 10, cc);
  }

  longIntegerSetZero(x);
  longIntegerModulo(aa, cc, y);
  longIntegerCopy(b, bb);

  while(longIntegerCompareInt(bb, 0) > 0) {
    if(longIntegerIsOdd(bb)) {
      longIntegerAdd(x, y, x);
      longIntegerModulo(x, cc, x);
    }
    longIntegerAdd(y, y, y);
    longIntegerModulo(y, cc, y);
    longIntegerDivideUInt(bb, 2u, bb);
  }
  longIntegerModulo(x, cc, res);

  longIntegerFree(x);
  longIntegerFree(y);
  longIntegerFree(aa);
  longIntegerFree(bb);
  longIntegerFree(cc);
}


void longInteger_expmod(const longInteger_t a, const longInteger_t b, const longInteger_t c, longInteger_t res) {
  longInteger_t x, y, bb;

  longIntegerInit(x);
  longIntegerInit(y);
  longIntegerInit(bb);

  uInt32ToLongInteger(1u, x);
  longIntegerModulo(a, c, y);
  longIntegerCopy(b, bb);

  while(longIntegerCompareInt(bb, 0) > 0) {
    if(longIntegerIsOdd(bb)) {
      longInteger_mulmod(x, 0, y, 0, c, 0, x, NULL);
    }
    longInteger_mulmod(y, 0, y, 0, c, 0, y, NULL);
    longIntegerDivideUInt(bb, 2u, bb);
  }
  longIntegerModulo(x, c, res);

  longIntegerFree(x);
  longIntegerFree(y);
  longIntegerFree(bb);
}


void opModLonILonILonI(uint16_t mode) {
  longInteger_t x, y, z;

  convertLongIntegerRegisterToLongInteger(REGISTER_X, x);
  convertLongIntegerRegisterToLongInteger(REGISTER_Y, y);
  convertLongIntegerRegisterToLongInteger(REGISTER_Z, z);

  if((longIntegerCompareInt(x, 1) > 0) && longIntegerIsPositive(y) && longIntegerIsPositive(z)) {
    if(mode == OPMOD_POWER) {
      longInteger_expmod(z, y, x, x);
    }
    else {
      longInteger_mulmod(z, 0, y, 0, x, 0, x, NULL);
    }
    convertLongIntegerToLongIntegerRegister(x, REGISTER_X);
  }
  else {
    opModOutOfDomain(mode);
  }

  longIntegerFree(x);
  longIntegerFree(y);
  longIntegerFree(z);
}


void opModLonILonIShoI(uint16_t mode) {
  convertShortIntegerRegisterToLongIntegerRegister(REGISTER_X, REGISTER_X);
  opModLonILonILonI(mode);
}


void opModLonILonIReal(uint16_t mode) {
  longInteger_t x, y, z;
  real_t xx;
  int32_t exp_x = 0, exponent;

  parseReal(REGISTER_X, x, &exp_x);
  convertLongIntegerRegisterToLongInteger(REGISTER_Y, y);
  convertLongIntegerRegisterToLongInteger(REGISTER_Z, z);
  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &xx);

  if((mode == OPMOD_POWER) || (getRegisterAngularMode(REGISTER_X) != amNone)) {
    opModError(mode);
  }
  else if(realCompareGreaterThan(&xx, const_1) && longIntegerIsPositive(y) && longIntegerIsPositive(z)) {
    longInteger_mulmod(z, 0, y, 0, x, exp_x, x, &exponent);
    toRealResult(x, exponent);
  }
  else {
    opModOutOfDomain(mode);
  }

  longIntegerFree(x);
  longIntegerFree(y);
  longIntegerFree(z);
}

void opModLonIShoILonI(uint16_t mode) {
  convertShortIntegerRegisterToLongIntegerRegister(REGISTER_Y, REGISTER_Y);
  opModLonILonILonI(mode);
}


void opModLonIShoIShoI(uint16_t mode) {
  convertShortIntegerRegisterToLongIntegerRegister(REGISTER_X, REGISTER_X);
  convertShortIntegerRegisterToLongIntegerRegister(REGISTER_Y, REGISTER_Y);
  opModLonILonILonI(mode);
}


void opModLonIShoIReal(uint16_t mode) {
  convertShortIntegerRegisterToLongIntegerRegister(REGISTER_Y, REGISTER_Y);
  opModLonILonIReal(mode);
}


void opModLonIRealLonI(uint16_t mode) {
  longInteger_t x, y, z;
  int32_t exp_y = 0, exponent;

  convertLongIntegerRegisterToLongInteger(REGISTER_X, x);
  parseReal(REGISTER_Y, y, &exp_y);
  convertLongIntegerRegisterToLongInteger(REGISTER_Z, z);

  if((mode == OPMOD_POWER) || (getRegisterAngularMode(REGISTER_Y) != amNone)) {
    opModError(mode);
  }
  else if((longIntegerCompareInt(x, 1) > 0) && longIntegerIsPositive(y) && longIntegerIsPositive(z)) {
    longInteger_mulmod(z, 0, y, exp_y, x, 0, x, &exponent);
    toRealResult(x, exponent);
  }
  else {
    opModOutOfDomain(mode);
  }

  longIntegerFree(x);
  longIntegerFree(y);
  longIntegerFree(z);
}


void opModLonIRealShoI(uint16_t mode) {
  convertShortIntegerRegisterToLongIntegerRegister(REGISTER_X, REGISTER_X);
  opModLonIRealLonI(mode);
}


void opModLonIRealReal(uint16_t mode) {
  longInteger_t x, y, z;
  real_t xx;
  int32_t exp_x = 0, exp_y = 0, exponent;

  parseReal(REGISTER_X, x, &exp_x);
  parseReal(REGISTER_Y, y, &exp_y);
  convertLongIntegerRegisterToLongInteger(REGISTER_Z, z);
  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &xx);

  if((mode == OPMOD_POWER) || (getRegisterAngularMode(REGISTER_Y) != amNone) || (getRegisterAngularMode(REGISTER_X) != amNone)) {
    opModError(mode);
  }
  else if(realCompareGreaterThan(&xx, const_1) && longIntegerIsPositive(y) && longIntegerIsPositive(z)) {
    longInteger_mulmod(z, 0, y, exp_y, x, exp_x, x, &exponent);
    toRealResult(x, exponent);
  }
  else {
    opModOutOfDomain(mode);
  }

  longIntegerFree(x);
  longIntegerFree(y);
  longIntegerFree(z);
}


void opModShoILonILonI(uint16_t mode) {
  convertShortIntegerRegisterToLongIntegerRegister(REGISTER_Z, REGISTER_Z);
  opModLonILonILonI(mode);
}


void opModShoILonIShoI(uint16_t mode) {
  const uint32_t base = getRegisterShortIntegerBase(REGISTER_Z);
  convertShortIntegerRegisterToLongIntegerRegister(REGISTER_X, REGISTER_X);
  convertShortIntegerRegisterToLongIntegerRegister(REGISTER_Z, REGISTER_Z);
  opModLonILonILonI(mode);
  if(mode == OPMOD_MULTIPLY) {
    convertLongIntegerRegisterToShortIntegerRegister(REGISTER_X, REGISTER_X);
    setRegisterTag(REGISTER_X, base);
  }
}


void opModShoILonIReal(uint16_t mode) {
  convertShortIntegerRegisterToLongIntegerRegister(REGISTER_Z, REGISTER_Z);
  opModLonILonIReal(mode);
}


void opModShoIShoILonI(uint16_t mode) {
  convertShortIntegerRegisterToLongIntegerRegister(REGISTER_Y, REGISTER_Y);
  convertShortIntegerRegisterToLongIntegerRegister(REGISTER_Z, REGISTER_Z);
  opModLonILonILonI(mode);
}


void opModShoIShoIShoI(uint16_t mode) {
  uint64_t x, y, z;
  int16_t sx, sy, sz;

  convertShortIntegerRegisterToUInt64(REGISTER_X, &sx, &x);
  convertShortIntegerRegisterToUInt64(REGISTER_Y, &sy, &y);
  convertShortIntegerRegisterToUInt64(REGISTER_Z, &sz, &z);

  if((!sx) && (x > 1) && (!sy) && (!sz)) {
    if(mode == OPMOD_POWER) {
      x = WP34S_expmod(z, y, x);
    }
    else {
      x = WP34S_mulmod(z, y, x);
    }
    convertUInt64ToShortIntegerRegister(0, x, getRegisterShortIntegerBase(REGISTER_Z), REGISTER_X);
  }
  else {
    opModOutOfDomain(mode);
  }
}


void opModShoIShoIReal(uint16_t mode) {
  convertShortIntegerRegisterToReal34Register(REGISTER_Y, REGISTER_Y);
  convertShortIntegerRegisterToReal34Register(REGISTER_Z, REGISTER_Z);
  opModRealRealReal(mode);
}


void opModShoIRealLonI(uint16_t mode) {
  convertShortIntegerRegisterToLongIntegerRegister(REGISTER_Z, REGISTER_Z);
  opModLonIRealLonI(mode);
}


void opModShoIRealShoI(uint16_t mode) {
  convertShortIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
  convertShortIntegerRegisterToReal34Register(REGISTER_Z, REGISTER_Z);
  opModRealRealReal(mode);
}


void opModShoIRealReal(uint16_t mode) {
  convertShortIntegerRegisterToReal34Register(REGISTER_Z, REGISTER_Z);
  opModRealRealReal(mode);
}


void opModRealLonILonI(uint16_t mode) {
  longInteger_t x, y, z;
  int32_t exp_z = 0, exponent;

  convertLongIntegerRegisterToLongInteger(REGISTER_X, x);
  convertLongIntegerRegisterToLongInteger(REGISTER_Y, y);
  parseReal(REGISTER_Z, z, &exp_z);

  if((mode == OPMOD_POWER) || (getRegisterAngularMode(REGISTER_Z) != amNone)) {
    opModError(mode);
  }
  else if((longIntegerCompareInt(x, 1) > 0) && longIntegerIsPositive(y) && longIntegerIsPositive(z)) {
    longInteger_mulmod(z, exp_z, y, 0, x, 0, x, &exponent);
    toRealResult(x, exponent);
  }
  else {
    opModOutOfDomain(mode);
  }

  longIntegerFree(x);
  longIntegerFree(y);
  longIntegerFree(z);
}


void opModRealLonIShoI(uint16_t mode) {
  convertShortIntegerRegisterToLongIntegerRegister(REGISTER_X, REGISTER_X);
  opModRealLonILonI(mode);
}


void opModRealLonIReal(uint16_t mode) {
  longInteger_t x, y, z;
  real_t xx;
  int32_t exp_x = 0, exp_z = 0, exponent;

  parseReal(REGISTER_X, x, &exp_x);
  convertLongIntegerRegisterToLongInteger(REGISTER_Y, y);
  parseReal(REGISTER_Z, z, &exp_z);
  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &xx);

  if((mode == OPMOD_POWER) || (getRegisterAngularMode(REGISTER_Z) != amNone) || (getRegisterAngularMode(REGISTER_X) != amNone)) {
    opModError(mode);
  }
  else if(realCompareGreaterThan(&xx, const_1) && longIntegerIsPositive(y) && longIntegerIsPositive(z)) {
    longInteger_mulmod(z, exp_z, y, 0, x, exp_x, x, &exponent);
    toRealResult(x, exponent);
  }
  else {
    opModOutOfDomain(mode);
  }

  longIntegerFree(x);
  longIntegerFree(y);
  longIntegerFree(z);
}

void opModRealShoILonI(uint16_t mode) {
  convertShortIntegerRegisterToLongIntegerRegister(REGISTER_Y, REGISTER_Y);
  opModRealLonILonI(mode);
}


void opModRealShoIShoI(uint16_t mode) {
  convertShortIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
  convertShortIntegerRegisterToReal34Register(REGISTER_Y, REGISTER_Y);
  opModRealRealReal(mode);
}


void opModRealShoIReal(uint16_t mode) {
  convertShortIntegerRegisterToReal34Register(REGISTER_Y, REGISTER_Y);
  opModRealRealReal(mode);
}


void opModRealRealLonI(uint16_t mode) {
  longInteger_t x, y, z;
  int32_t exp_y = 0, exp_z = 0, exponent;

  convertLongIntegerRegisterToLongInteger(REGISTER_X, x);
  parseReal(REGISTER_Y, y, &exp_y);
  parseReal(REGISTER_Z, z, &exp_z);


  if((mode == OPMOD_POWER) || (getRegisterAngularMode(REGISTER_Z) != amNone) || (getRegisterAngularMode(REGISTER_Y) != amNone)) {
    opModError(mode);
  }
  else if((longIntegerCompareInt(x, 1) > 0) && longIntegerIsPositive(y) && longIntegerIsPositive(z)) {
    longInteger_mulmod(z, exp_z, y, exp_y, x, 0, x, &exponent);
    toRealResult(x, exponent);
  }
  else {
    opModOutOfDomain(mode);
  }

  longIntegerFree(x);
  longIntegerFree(y);
  longIntegerFree(z);
}


void opModRealRealShoI(uint16_t mode) {
  convertShortIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
  opModRealRealReal(mode);
}


void opModRealRealReal(uint16_t mode) {
  real_t x, y, z;
  real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x);
  real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y);
  real34ToReal(REGISTER_REAL34_DATA(REGISTER_Z), &z);
  if((mode == OPMOD_POWER) || (getRegisterAngularMode(REGISTER_Z) != amNone) || (getRegisterAngularMode(REGISTER_Y) != amNone) || (getRegisterAngularMode(REGISTER_X) != amNone)) {
    opModError(mode);
  }
  else if(realCompareGreaterThan(&x, const_1) && realCompareGreaterThan(&y, const_0) && realCompareGreaterThan(&z, const_0)) {
    realMultiply(&z, &y, &z, &ctxtReal75);
    WP34S_Mod(&z, &x, &x, &ctxtReal75);
    convertRealToReal34ResultRegister(&x, REGISTER_X);
  }
  else {
    opModOutOfDomain(mode);
  }
}
