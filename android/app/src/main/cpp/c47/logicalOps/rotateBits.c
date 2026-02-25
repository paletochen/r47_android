// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file rotateBits.c
 ***********************************************/

#include "c47.h"

static bool_t getShiftInput(uint64_t *w, uint32_t *base) {
  if (!getRegisterAsRawShortInt(REGISTER_X, w, base)) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot shift/rotate %s", getRegisterDataTypeName(REGISTER_X, true, false));
      moreInfoOnError("In function fnAsr:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return false;
  }
  if (!saveLastX())
    return false;
  return true;
}

static void setShiftResult(uint64_t w, uint32_t base) {
  reallocateRegister(REGISTER_X, dtShortInteger, SHORT_INTEGER_SIZE_IN_BLOCKS, base);
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = w & shortIntegerMask;
  //adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}

/********************************************//**
 * \brief regX ==> regL and ASR(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] numberOfShifts uint16_t
 * \return void
 ***********************************************/
void fnAsr(uint16_t numberOfShifts) {
  int32_t i;
  uint32_t base;
  uint64_t w, sign;

  if (!getShiftInput(&w, &base))
    return;

  sign = w & shortIntegerSignBit;
  for(i=1; i<=numberOfShifts; i++) {
    if(i == numberOfShifts) {
      forceSystemFlag(FLAG_CARRY, w & 1);
    }

    w = (w >> 1) | sign;
  }
  setShiftResult(w, base);
}



/********************************************//**
 * \brief regX ==> regL and SL(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] numberOfShifts uint16_t
 * \return void
 ***********************************************/
void fnSl(uint16_t numberOfShifts) {
  uint64_t w;
  uint32_t base;
  int32_t i;

  if (!getShiftInput(&w, &base))
    return;

  for(i=1; i<=numberOfShifts; i++) {
    if(i == numberOfShifts) {
      forceSystemFlag(FLAG_CARRY, (w & shortIntegerSignBit) != 0);
    }
    w <<= 1;
  }
  setShiftResult(w, base);
}



/********************************************//**
 * \brief regX ==> regL and SR(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] numberOfShifts uint16_t
 * \return void
 ***********************************************/
void fnSr(uint16_t numberOfShifts) {
  uint64_t w;
  uint32_t base;
  int32_t i;

  if (!getShiftInput(&w, &base))
    return;

  for(i=1; i<=numberOfShifts; i++) {
    if(i == numberOfShifts) {
      forceSystemFlag(FLAG_CARRY, w & 1);
    }

    w >>= 1;
  }
  setShiftResult(w, base);
}



/********************************************//**
 * \brief regX ==> regL and RL(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] numberOfShifts uint16_t
 * \return void
 ***********************************************/
void fnRl(uint16_t numberOfShifts) {
  int32_t i;
  unsigned int sign;
  uint64_t w;
  uint32_t base;

  if (!getShiftInput(&w, &base))
    return;

  for(i=1; i<=numberOfShifts; i++) {
    sign = (w & shortIntegerSignBit) != 0;
    if(i == numberOfShifts) {
      forceSystemFlag(FLAG_CARRY, sign);
    }

    w = (w << 1) | sign;
  }
  setShiftResult(w, base);
}



/********************************************//**
 * \brief regX ==> regL and RR(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] numberOfShifts uint16_t
 * \return void
 ***********************************************/
void fnRr(uint16_t numberOfShifts) {
  int32_t i;
  uint64_t w;
  uint32_t base;

  if (!getShiftInput(&w, &base))
    return;

  for(i=1; i<=numberOfShifts; i++) {
    if(i == numberOfShifts) {
      forceSystemFlag(FLAG_CARRY, w & 1);
    }

    w = (w >> 1) | ((w & 1) << (shortIntegerWordSize - 1));
  }
  setShiftResult(w, base);
}



/********************************************//**
 * \brief regX ==> regL and RLC(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] numberOfShifts uint16_t
 * \return void
 ***********************************************/
void fnRlc(uint16_t numberOfShifts) {
  int32_t i;
  unsigned int sign, carry;
  uint64_t w;
  uint32_t base;

  if (!getShiftInput(&w, &base))
    return;

  carry = getSystemFlag(FLAG_CARRY);
  for(i=1; i<=numberOfShifts; i++) {
    sign = (w & shortIntegerSignBit) != 0;
    w = (w << 1) | carry;
    carry = sign;
  }

  forceSystemFlag(FLAG_CARRY, carry);
  setShiftResult(w, base);
}



/********************************************//**
 * \brief regX ==> regL and RRC(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] numberOfShifts uint16_t
 * \return void
 ***********************************************/
void fnRrc(uint16_t numberOfShifts) {
  int32_t i;
  unsigned int lsb, carry;
  uint64_t w;
  uint32_t base;

  if (!getShiftInput(&w, &base))
    return;

  carry = getSystemFlag(FLAG_CARRY);
  for(i=1; i<=numberOfShifts; i++) {
    lsb = w & 1;
    w = (w >> 1) | ((uint64_t)carry << (shortIntegerWordSize - 1));
    carry = lsb;
  }

  forceSystemFlag(FLAG_CARRY, carry);
  setShiftResult(w, base);
}



static void justifyResultToRegisters(uint32_t count, uint32_t base, uint64_t val) {
  longInteger_t ireg;

  reallocateRegister(REGISTER_X, dtShortInteger, SHORT_INTEGER_SIZE_IN_BLOCKS, base);
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = val & shortIntegerMask;
  setSystemFlag(FLAG_ASLIFT);
  liftStack();

  longIntegerInit(ireg);
  uInt32ToLongInteger(count, ireg);
  convertLongIntegerToLongIntegerRegister(ireg, REGISTER_X);
  longIntegerFree(ireg);
}

/********************************************//**
 * \brief regX ==> regL and LJ(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnLj(uint16_t unusedButMandatoryParameter) {
  uint32_t count, base;
  uint64_t w;

  if (!getShiftInput(&w, &base))
    return;

  if (w == 0)
    count = shortIntegerWordSize;
  else {
    count = __builtin_clzll(w) - (64 - shortIntegerWordSize);
    w <<= count;
  }
  justifyResultToRegisters(count, base, w);
}



/********************************************//**
 * \brief regX ==> regL and RJ(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnRj(uint16_t unusedButMandatoryParameter) {
  uint32_t count, base;
  uint64_t w;

  if (!getShiftInput(&w, &base))
    return;

  if (w == 0)
    count = shortIntegerWordSize;
  else {
    count = __builtin_ctzll(w | ~shortIntegerMask);
    w >>= count;
  }
  justifyResultToRegisters(count, base, w);
}



void fnMirror(uint16_t unusedButMandatoryParameter) {
  uint64_t x, r=0;
  uint32_t base;

  if (!getShiftInput(&x, &base))
    return;

  for(uint32_t i=0; i<shortIntegerWordSize; i++) {
    if(x & (1LL << i)) {
      r |= 1LL << (shortIntegerWordSize-i-1);
    }
  }

  setShiftResult(r, base);
}

/********************************************//**                    //JM vv
 * \brief regX ==> regL and Change Endianness(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSwapEndian(uint16_t bitWidth) {
  uint64_t b7,b6,b5,b4,b3,b2,b1,b0;
  uint64_t x;
  uint32_t base;

  if (!getShiftInput(&x, &base))
    return;

  //printf("### %d %d",(shortIntegerWordSize & (bitWidth-1)), (shortIntegerWordSize |  (bitWidth-1) ) + 1 );

  b7 = (x & 0xFF00000000000000) >> (64- 8);
  b6 = (x & 0x00FF000000000000) >> (64-16);
  b5 = (x & 0x0000FF0000000000) >> (64-24);
  b4 = (x & 0x000000FF00000000) >> (64-32);
  b3 = (x & 0x00000000FF000000) >> (64-40);
  b2 = (x & 0x0000000000FF0000) >> (64-48);
  b1 = (x & 0x000000000000FF00) >> (64-56);
  b0 = (x & 0x00000000000000FF) >> (64-64);

  if(bitWidth == 8) {
    if(shortIntegerWordSize<16) {fnSetWordSize(16);}
    else
      if( (shortIntegerWordSize & (bitWidth-1)) != 0 ) {fnSetWordSize((shortIntegerWordSize |  (bitWidth-1) ) + 1);}
    switch(shortIntegerWordSize) {
      case 16: x =                                                                               (b0 << 8 ) | b1; break;
      case 24: x =                                                                  (b0 << 16) | (b1 << 8 ) | b2; break;
      case 32: x =                                                     (b0 << 24) | (b1 << 16) | (b2 << 8 ) | b3; break;
      case 40: x =                                        (b0 << 32) | (b1 << 24) | (b2 << 16) | (b3 << 8 ) | b4; break;
      case 48: x =                           (b0 << 40) | (b1 << 32) | (b2 << 24) | (b3 << 16) | (b4 << 8 ) | b5; break;
      case 56: x =              (b0 << 48) | (b1 << 40) | (b2 << 32) | (b3 << 24) | (b4 << 16) | (b5 << 8 ) | b6; break;
      case 64: x = (b0 << 56) | (b1 << 48) | (b2 << 40) | (b3 << 32) | (b4 << 24) | (b5 << 16) | (b6 << 8 ) | b7; break;
      default:break;
    }
  }
  else
  if(bitWidth == 16) {
    if(shortIntegerWordSize<32) {fnSetWordSize(32);}
    else
      if( (shortIntegerWordSize & (bitWidth-1)) != 0 ) {fnSetWordSize((shortIntegerWordSize |  (bitWidth-1) ) + 1);}
    switch(shortIntegerWordSize) {
      case 32: x =                                                     (b1 << 24) | (b0 << 16) | (b3 << 8 ) | b2; break;
      case 48: x =                           (b1 << 40) | (b0 << 32) | (b3 << 24) | (b2 << 16) | (b5 << 8 ) | b4; break;
      case 64: x = (b1 << 56) | (b0 << 48) | (b3 << 40) | (b2 << 32) | (b5 << 24) | (b4 << 16) | (b7 << 8 ) | b6; break;
      default:break;
    }
  }
  setShiftResult(x, base);
}                                                              //JM ^^


void fnZip(uint16_t unusedButMandatoryParameter) {
  uint64_t x, y, r = 0, mask = 1;
  uint32_t base;
  unsigned int i, j;

  if (!getRegisterAsRawShortInt(REGISTER_Y, &y, &base)) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_Y);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot shift/rotate %s", getRegisterDataTypeName(REGISTER_Y, true, false));
      moreInfoOnError("In function fnZip:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }
  if (!getShiftInput(&x, &base))
    return;

  for (i = j = 0; i < shortIntegerWordSize / 2; i++) {
      r |= (x & mask) << j++;
      r |= (y & mask) << j;
      mask += mask;
  }
  setShiftResult(r, base);
  adjustResult(REGISTER_X, true, true, REGISTER_X, REGISTER_Y, -1);
}

void fnUnzip(uint16_t unusedButMandatoryParameter) {
  uint64_t a, x = 0, y = 0, mask = 1;
  uint32_t base;
  unsigned int i, j;

  if (!getShiftInput(&a, &base))
    return;
  setSystemFlag(FLAG_ASLIFT);
  liftStack();
  for (j = i = 0; i < shortIntegerWordSize / 2; i++) {
      x |= (a & mask) >> j++;
      mask += mask;
      y |= (a & mask) >> j;
      mask += mask;
  }
  reallocateRegister(REGISTER_Y, dtShortInteger, SHORT_INTEGER_SIZE_IN_BLOCKS, base);
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_Y)) = y & shortIntegerMask;
  setShiftResult(x, base);
}

