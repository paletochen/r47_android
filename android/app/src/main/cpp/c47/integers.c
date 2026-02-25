// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

void fnChangeBase(uint16_t base) {
  bool_t sign;
  uint64_t val;

  if (base < 2 || base > 16) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_T);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "base = %" PRIu16 "! The base must be fron 2 to 16.", base);
      moreInfoOnError("In function fnChangeBase:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }

  if (!getRegisterAsShortInt(REGISTER_X, &sign, &val, NULL, NULL))
    return;
  convertUInt64ToShortIntegerRegister(sign, val, base, REGISTER_X);
  lastIntegerBase = base;                //JMNIM
  fnRefreshState();             //JM
}



void longIntegerMultiply(longInteger_t opY, longInteger_t opX, longInteger_t result) {
  if(longIntegerBits(opY) + longIntegerBits(opX) <= MAX_LONG_INTEGER_SIZE_IN_BITS) {
    mpz_mul(result, opY, opX);
  }
  else {
    displayCalcErrorMessage(longIntegerSign(opY) == longIntegerSign(opX) ? ERROR_OVERFLOW_PLUS_INF : ERROR_OVERFLOW_MINUS_INF, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Multiplying this 2 values (%" PRIu64 " bits " STD_CROSS " %" PRIu64 " bits) would result in a value exceeding %" PRId16 " bits!", (uint64_t)longIntegerBits(opY), (uint64_t)longIntegerBits(opX), (uint16_t)MAX_LONG_INTEGER_SIZE_IN_BITS);
      longIntegerToAllocatedString(opY, tmpString, TMP_STR_LENGTH / 2);
      longIntegerToAllocatedString(opX, tmpString + TMP_STR_LENGTH / 2, TMP_STR_LENGTH / 2);
      moreInfoOnError("In function longIntegerMultiply:", errorMessage, tmpString, tmpString + TMP_STR_LENGTH / 2);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}



void longIntegerSquare(longInteger_t op, longInteger_t result) {
  if(longIntegerBits(op) * 2 <= MAX_LONG_INTEGER_SIZE_IN_BITS) {
    mpz_mul(result, op, op);
  }
  else {
    displayCalcErrorMessage(ERROR_OVERFLOW_PLUS_INF, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Squaring this value (%" PRIu64 " bits) would result in a value exceeding %" PRId16 " bits!", (uint64_t)longIntegerBits(op), (uint16_t)MAX_LONG_INTEGER_SIZE_IN_BITS);
      longIntegerToAllocatedString(op, tmpString, TMP_STR_LENGTH);
      moreInfoOnError("In function longIntegerSquare:", errorMessage, tmpString, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}



void longIntegerAdd(longInteger_t opY, longInteger_t opX, longInteger_t result) {
  if(longIntegerSign(opY) != longIntegerSign(opX) || max(longIntegerBits(opY), longIntegerBits(opX)) <= MAX_LONG_INTEGER_SIZE_IN_BITS - 1) {
    mpz_add(result, opY, opX);
  }
  else {
    displayCalcErrorMessage(longIntegerSign(opY) == 0 ? ERROR_OVERFLOW_PLUS_INF : ERROR_OVERFLOW_MINUS_INF, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Adding this 2 values (%" PRIu64 " bits " STD_CROSS " %" PRIu64 " bits) would result in a value exceeding %" PRId16 " bits!", (uint64_t)longIntegerBits(opY), (uint64_t)longIntegerBits(opX), (uint16_t)MAX_LONG_INTEGER_SIZE_IN_BITS);
      longIntegerToAllocatedString(opY, tmpString, TMP_STR_LENGTH / 2);
      longIntegerToAllocatedString(opX, tmpString + TMP_STR_LENGTH / 2, TMP_STR_LENGTH / 2);
      moreInfoOnError("In function longIntegerAdd:", errorMessage, tmpString, tmpString + TMP_STR_LENGTH / 2);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}



void longIntegerSubtract(longInteger_t opY, longInteger_t opX, longInteger_t result) {
  if(longIntegerSign(opY) == longIntegerSign(opX) || max(longIntegerBits(opY), longIntegerBits(opX)) <= MAX_LONG_INTEGER_SIZE_IN_BITS - 1) {
    mpz_sub(result, opY, opX);
  }
  else {
    displayCalcErrorMessage(longIntegerSign(opY) == 0 ? ERROR_OVERFLOW_PLUS_INF : ERROR_OVERFLOW_MINUS_INF, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Subtracting this 2 values (%" PRIu64 " bits " STD_CROSS " %" PRIu64 " bits) would result in a value exceeding %" PRId16 " bits!", (uint64_t)longIntegerBits(opY), (uint64_t)longIntegerBits(opX), (uint16_t)MAX_LONG_INTEGER_SIZE_IN_BITS);
      longIntegerToAllocatedString(opY, tmpString, TMP_STR_LENGTH / 2);
      longIntegerToAllocatedString(opX, tmpString + TMP_STR_LENGTH / 2, TMP_STR_LENGTH / 2);
      moreInfoOnError("In function longIntegerSubtract:", errorMessage, tmpString, tmpString + TMP_STR_LENGTH / 2);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}






/*
 * The functions below are borrowed
 * from the WP34S project and sligtly
 * modified and adapted
 */

/* Helper routine for addition and subtraction that detemines the proper
 * setting for the overflow bit.  This routine should only be called when
 * the signs of the operands are the same for addition and different
 * for subtraction.  Overflow isn't possible if the signs are opposite.
 * The arguments of the operator should be passed in after conversion
 * to positive unsigned quantities nominally in two's complement.
 */
static int32_t WP34S_calc_overflow(uint64_t xv, uint64_t yv, int32_t neg) {
  uint64_t u;
  int32_t i;

  switch(shortIntegerMode) {
    case SIM_UNSIGN: {
      // C doesn't expose the processor's status bits to us so we
      // break the addition down so we don't lose the overflow.
      u = (yv & (shortIntegerSignBit-1)) + (xv & (shortIntegerSignBit-1));
      i = ((u & shortIntegerSignBit)?1:0) + ((xv & shortIntegerSignBit)?1:0) + ((yv & shortIntegerSignBit)?1:0);
      if(i > 1) {
        break;
      }
      return 0;
    }

    case SIM_2COMPL: {
      u = xv + yv;
      if(neg && u == shortIntegerSignBit) {
        return 0;
      }

      if(shortIntegerSignBit & u) {
        break;
      }

      if((xv == shortIntegerSignBit && yv !=0) || (yv == shortIntegerSignBit && xv != 0)) {
        break;
      }
      return 0;
    }

    case SIM_SIGNMT:
    case SIM_1COMPL: {
      if(shortIntegerSignBit & (xv + yv)) {
        break;
      }
      return 0;
    }

    default: {
      sprintf(errorMessage, commonBugScreenMessages[bugMsgValueFor], "WP34S_calc_overflow", shortIntegerMode, "shortIntegerMode");
      displayBugScreen(errorMessage);
      return 0;
    }
  }

  setSystemFlag(FLAG_OVERFLOW);

  return 1;
}


/* Utility routine to convert a binary integer into separate sign and
 * value components.  The sign returned is 1 for negative and 0 for positive.
 */
uint64_t WP34S_extract_value(const uint64_t val, int32_t *const sign) {
  uint64_t value = val & shortIntegerMask;

  if(shortIntegerMode == SIM_UNSIGN) {
    *sign = 0;
    return value;
  }

  if(value & shortIntegerSignBit) {
    *sign = 1;
    if(shortIntegerMode == SIM_2COMPL) {
      value = -value;
    }
    else if(shortIntegerMode == SIM_1COMPL) {
      value = ~value;
    }
    else { // if(shortIntegerMode == SIM_SIGNMT)
      value ^= shortIntegerSignBit;
    }
  }
  else {
    *sign = 0;
  }

  return value & shortIntegerMask;
}


/* Helper routine to construct a value from the magnitude and sign
 */
int64_t WP34S_build_value(const uint64_t x, const int32_t sign) {
  int64_t value = x & shortIntegerMask;

  if(sign == 0 || shortIntegerMode == SIM_UNSIGN) {
    return value;
  }

  if(shortIntegerMode == SIM_2COMPL) {
    return (-(int64_t)value) & shortIntegerMask;
  }

  if(shortIntegerMode == SIM_1COMPL) {
    return (~value) & shortIntegerMask;
  }

  return value | shortIntegerSignBit;
}


static uint64_t WP34S_multiply_with_overflow(uint64_t multiplier, uint64_t multiplicand, int32_t *overflow) {
  const uint64_t product = (multiplier * multiplicand) & shortIntegerMask;

  if(!*overflow && multiplicand != 0) {
    const uint64_t tbm = (shortIntegerMode == SIM_UNSIGN) ? 0 : shortIntegerSignBit;

    if((product & tbm) != 0 || product / multiplicand != multiplier) {
      *overflow = 1;
    }
  }
  return product;
}

uint64_t WP34S_intAdd(uint64_t y, uint64_t x) {
  int32_t termXSign, termYSign;
  uint64_t termX = WP34S_extract_value(x, &termXSign);
  uint64_t termY = WP34S_extract_value(y, &termYSign);
  uint64_t sum;
  int32_t overflow;

  clearSystemFlag(FLAG_OVERFLOW);
  if(termXSign == termYSign) {
    overflow = WP34S_calc_overflow(termX, termY, termXSign);
  }
  else {
    overflow = 0;
  }

  if(shortIntegerMode == SIM_SIGNMT) {
    const uint64_t x2 = (x & shortIntegerSignBit)?-(x ^ shortIntegerSignBit):x;
    const uint64_t y2 = (y & shortIntegerSignBit)?-(y ^ shortIntegerSignBit):y;

    if(overflow) {
      setSystemFlag(FLAG_CARRY);
    }
    else {
      clearSystemFlag(FLAG_CARRY);
    }

    sum = y2 + x2;
    if(sum & shortIntegerSignBit) {
      sum = -sum | shortIntegerSignBit;
    }
  }
  else {
    const uint64_t maskedSum = (y + x) & shortIntegerMask;

    sum = y + x;

    if(maskedSum < (y & shortIntegerMask)) {
      setSystemFlag(FLAG_CARRY);

      if(shortIntegerMode == SIM_1COMPL) {
        sum++;
      }
    }
    else {
      clearSystemFlag(FLAG_CARRY);
    }
  }
  return sum & shortIntegerMask;
}


uint64_t WP34S_intSubtract(uint64_t y, uint64_t x) {
  int32_t termXSign, termYSign;
  uint64_t termX = WP34S_extract_value(x, &termXSign);
  uint64_t termY = WP34S_extract_value(y, &termYSign);
  uint64_t difference;

  clearSystemFlag(FLAG_OVERFLOW);
  if(termXSign != termYSign) {
    WP34S_calc_overflow(termX, termY, termYSign);
  }

  if(shortIntegerMode == SIM_SIGNMT) {
    int64_t x2, y2;

    if((termXSign == 0 && termYSign == 0 && termX > termY) || (termXSign != 0 && termYSign != 0 && termX < termY)) {
      setSystemFlag(FLAG_CARRY);
    }
    else {
      clearSystemFlag(FLAG_CARRY);
    }

    x2 = (x & shortIntegerSignBit) ? -(x ^ shortIntegerSignBit) : x;
    y2 = (y & shortIntegerSignBit) ? -(y ^ shortIntegerSignBit) : y;

    difference = y2 - x2;
    if(difference & shortIntegerSignBit) {
      difference = -difference | shortIntegerSignBit;
    }
  }
  else {
    difference = y - x;

    if((uint64_t)y < (uint64_t)x) {
      setSystemFlag(FLAG_CARRY);
      if(shortIntegerMode == SIM_UNSIGN) {
        setSystemFlag(FLAG_OVERFLOW);
      }

      if(shortIntegerMode == SIM_1COMPL) {
        difference--;
      }
    }
    else {
      clearSystemFlag(FLAG_CARRY);
    }
  }
  return difference & shortIntegerMask;
}


uint64_t WP34S_intMultiply(uint64_t y, uint64_t x) {
  uint64_t product;
  int32_t multiplicandSign, multiplierSign;
  uint64_t multiplicand = WP34S_extract_value(x, &multiplicandSign);
  uint64_t multiplier   = WP34S_extract_value(y, &multiplierSign);
  int32_t overflow = 0;

  product = WP34S_multiply_with_overflow(multiplier, multiplicand, &overflow);

  if(overflow) {
    setSystemFlag(FLAG_OVERFLOW);
  }
  else {
    clearSystemFlag(FLAG_OVERFLOW);
  }

  if(shortIntegerMode == SIM_UNSIGN) {
    return product;
  }
  return WP34S_build_value(product & ~shortIntegerSignBit, multiplicandSign ^ multiplierSign);
}


uint64_t WP34S_intDivide(uint64_t y, uint64_t x) {
  int32_t divisorSign, dividendSign;
  uint64_t divisor = WP34S_extract_value(x, &divisorSign);
  uint64_t dividend   = WP34S_extract_value(y, &dividendSign);
  uint64_t quotient;

  if(divisor == 0) {
    if(dividend == 0) {
      displayCalcErrorMessage(ERROR_BAD_TIME_OR_DATE_INPUT, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function WP34S_intDivide: cannot divide 0 by 0!", NULL, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else if(dividendSign) {
      displayCalcErrorMessage(ERROR_BAD_TIME_OR_DATE_INPUT, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function WP34S_intDivide: cannot divide a negative short integer by 0!", NULL, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      displayCalcErrorMessage(ERROR_BAD_TIME_OR_DATE_INPUT, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function WP34S_intDivide: cannot divide a positive short integer by 0!", NULL, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    return 0;
  }

  clearSystemFlag(FLAG_OVERFLOW);
  quotient = (dividend / divisor) & shortIntegerMask;
  // Set carry if there is a remainder
  if(quotient * divisor != dividend) {
    setSystemFlag(FLAG_CARRY);
  }
  else {
    clearSystemFlag(FLAG_CARRY);
  }

  if(shortIntegerMode != SIM_UNSIGN) {
    if(quotient & shortIntegerSignBit) {
      setSystemFlag(FLAG_CARRY);
    }
    // Special case for 0x8000...00 / -1 in 2's complement
    if(shortIntegerMode == SIM_2COMPL && divisorSign && divisor == 1 && y == shortIntegerSignBit) {
      setSystemFlag(FLAG_OVERFLOW);
    }
  }
  return WP34S_build_value(quotient, divisorSign ^ dividendSign);
}

/* never used
uint64_t WP34S_intSqr(uint64_t x) {
  return WP34S_intMultiply(x, x);
}
*/
/* never used
uint64_t WP34S_intCube(uint64_t x) {
  int64_t y = WP34S_intMultiply(x, x);
  int32_t overflow = (getSystemFlag(FLAG_OVERFLOW) == ON ? 1 : 0);

  y = WP34S_intMultiply(x, y);
  if(overflow) {
    setSystemFlag(FLAG_OVERFLOW);
  }
  return y;
}
*/

uint64_t WP34S_int_gcd(uint64_t a, uint64_t b) {
  while(b != 0) {
    uint64_t t = b;
    b = a % b;
    a = t;
  }
  return a;
}


uint64_t WP34S_intGCD(uint64_t y, uint64_t x) {
  int32_t s;
  uint64_t xv = WP34S_extract_value(x, &s);
  uint64_t yv = WP34S_extract_value(y, &s);
  uint64_t gcd;

  if(xv == 0) {
    gcd = yv;
  }
  else if(yv == 0) {
    gcd = xv;
  }
  else {
    gcd = WP34S_int_gcd(xv, yv);
  }
  return WP34S_build_value(gcd, 0);
}


uint64_t WP34S_intLCM(uint64_t y, uint64_t x) {
  int32_t s;
  uint64_t xv = WP34S_extract_value(x, &s);
  uint64_t yv = WP34S_extract_value(y, &s);
  uint64_t gcd;

  if(xv == 0 || yv == 0) {
    return 0;
  }

  gcd = WP34S_int_gcd(xv, yv);
  return WP34S_intMultiply((xv / gcd) & shortIntegerMask, WP34S_build_value(yv, 0));
}


uint64_t WP34S_intChs(uint64_t x) {
  int32_t signValue;
  uint64_t value = WP34S_extract_value(x, &signValue);

  if(shortIntegerMode == SIM_UNSIGN || (shortIntegerMode == SIM_2COMPL && x == shortIntegerSignBit)) {
    temporaryInformation = TI_DATA_NEG_OVRFL;
    setSystemFlag(FLAG_OVERFLOW);
    return (-(int64_t)value) & shortIntegerMask;
  }

  clearSystemFlag(FLAG_OVERFLOW);
  return WP34S_build_value(value, !signValue);
}


/* Integer floor(sqrt())
 */
uint64_t WP34S_intSqrt(uint64_t x) {
  int32_t signValue;
  uint64_t value = WP34S_extract_value(x, &signValue);
  uint64_t nn0, nn1;

  if(signValue) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function WP34S_intSqrt: Cannot extract the square root of a negative short integer!", NULL, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return 0;
  }
  if(value == 0) {
    nn1 = 0;
  }
  else {
    nn0 = value / 2 + 1;
    nn1 = value / nn0 + nn0 / 2;
    while(nn1 < nn0) {
      nn0 = nn1;
      nn1 = (nn0 + value / nn0) / 2;
    }
    nn0 = nn1 * nn1;
    if(nn0 > value) {
      nn1--;
    }

    if(nn0 != value) {
      setSystemFlag(FLAG_CARRY);
    }
    else {
      clearSystemFlag(FLAG_CARRY);
    }
  }
  return WP34S_build_value(nn1, signValue);
}


uint64_t WP34S_intAbs(uint64_t x) {
  int32_t signValue;
  uint64_t value = WP34S_extract_value(x, &signValue);

  clearSystemFlag(FLAG_OVERFLOW);
  if(shortIntegerMode == SIM_2COMPL && x == shortIntegerSignBit) {
    setSystemFlag(FLAG_OVERFLOW);
    return x;
  }
  return WP34S_build_value(value, 0);
}


//uint64_t WP34S_intNot(uint64_t x) {
//  return (~x) & shortIntegerMask;
//}


/* Fraction and integer parts are very easy for integers.
 */
//uint64_t WP34S_intFP(uint64_t x) {
//  return 0;
//}


//uint64_t WP34S_intIP(uint64_t x) {
//  return x;
//}


uint64_t WP34S_intSign(uint64_t x) {
  int32_t signValue;
  uint64_t value = WP34S_extract_value(x, &signValue);

  if(value == 0) {
    signValue = 0;
  }
  else {
    value = 1;
  }
  return WP34S_build_value(value, signValue);
}


static uint64_t WP34S_int_power_helper(uint64_t base, uint64_t exponent, int32_t overflow) {
  uint64_t power = 1;
  uint32_t i;
  int32_t overflow_next = 0;

  for(i=0; i<shortIntegerWordSize && exponent != 0; i++) {
    if(exponent & 1) {
      if(overflow_next) {
        overflow = 1;
      }
      power = WP34S_multiply_with_overflow(power, base, &overflow);
    }
    exponent >>= 1;

    if(exponent != 0) {
      base = WP34S_multiply_with_overflow(base, base, &overflow_next);
    }
  }

  if(overflow) {
    setSystemFlag(FLAG_OVERFLOW);
  }
  else {
    clearSystemFlag(FLAG_OVERFLOW);
  }

  return power;
}


/* Integer power y^x
 */
uint64_t WP34S_intPower(uint64_t b, uint64_t e) {
  int32_t exponentSign, baseSign, powerSign;
  uint64_t exponent = WP34S_extract_value(e, &exponentSign);
  uint64_t base     = WP34S_extract_value(b, &baseSign);

  if(exponent == 0 && base == 0) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function WP34S_intPower: Cannot calculate 0^0!", NULL, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    setSystemFlag(FLAG_OVERFLOW);
    return 0;
  }
  clearSystemFlag(FLAG_CARRY);
  clearSystemFlag(FLAG_OVERFLOW);

  if(exponent == 0) {
    return 1;
  }
  else if(base == 0) {
    return 0;
  }

  //if(exponentSign) { when WP34S_intPower is called, exponent cannot be negative
  //  setSystemFlag(FLAG_CARRY);
  //  return 0;
  //}

  powerSign = (baseSign && (exponent & 1))?1:0; // Determine the sign of the result
  return WP34S_build_value(WP34S_int_power_helper(base, exponent, 0), powerSign);
}


/* 2^x
 */
uint64_t WP34S_int2pow(uint64_t x) {
  int32_t signExponent;
  uint64_t exponent = WP34S_extract_value(x, &signExponent);
  uint32_t wordSize = shortIntegerWordSize;

  clearSystemFlag(FLAG_OVERFLOW);
  if(signExponent && exponent == 1) {
    setSystemFlag(FLAG_CARRY);
  }
  else {
    clearSystemFlag(FLAG_CARRY);
  }

  if(signExponent && exponent != 0) {
    return 0;
  }

  if(shortIntegerMode != SIM_UNSIGN) {
    wordSize--;
  }
  if(exponent >= wordSize) {
    if(exponent == wordSize) {
      setSystemFlag(FLAG_CARRY);
    }
    else {
      clearSystemFlag(FLAG_CARRY);
    }

    setSystemFlag(FLAG_OVERFLOW);
    return 0;
  }
  return 1LL << (uint32_t)(exponent & 0xff);
}


/* 10^x
 */
uint64_t WP34S_int10pow(uint64_t x) {
  int32_t sx;
  uint64_t exponent = WP34S_extract_value(x, &sx);
  int32_t overflow = 0;

  clearSystemFlag(FLAG_CARRY);
  if(exponent == 0) {
    clearSystemFlag(FLAG_OVERFLOW);
    return 1;
  }

  if(sx) {
    setSystemFlag(FLAG_CARRY);
    return 0;
  }

  if(shortIntegerWordSize <= 3 || (shortIntegerMode != SIM_UNSIGN && shortIntegerWordSize == 4)) {
    overflow = 1;
  }
  return WP34S_build_value(WP34S_int_power_helper(10, x, overflow), 0);
}


/* Integer floor(log2())
 */
uint64_t WP34S_intLog2(uint64_t x) {
  int32_t signValue;
  uint64_t value = WP34S_extract_value(x, &signValue);
  uint32_t log2 = 0;

  if(value == 0 || signValue) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function WP34S_intLog2: Cannot calculate the log" STD_SUB_2 " of a number " STD_LESS_EQUAL " 0!", NULL, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return 0;
  }

  if(value & (value - 1)) {
    setSystemFlag(FLAG_CARRY);
  }
  else {
    clearSystemFlag(FLAG_CARRY);
  }

  while(value >>= 1) {
    log2++;
  }

  return WP34S_build_value(log2, signValue);
}


/* Integer floor(log10())
 */
uint64_t WP34S_intLog10(uint64_t x) {
  int32_t signValue;
  uint64_t value = WP34S_extract_value(x, &signValue);
  int32_t r = 0;
  int32_t c = 0;

  if(value == 0 || signValue) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function WP34S_intLog10: Cannot calculate the log" STD_SUB_10 " of a number " STD_LESS_EQUAL " 0!", NULL, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return 0;
  }

  while(value >= 10) {
    r++;
    if(value % 10) {
      c = 1;
    }
    value /= 10;
  }

  if(c || value != 1) {
    setSystemFlag(FLAG_CARRY);
  }
  else {
    clearSystemFlag(FLAG_CARRY);
  }

  return WP34S_build_value(r, signValue);
}

/* Calculate (a . b) mod c taking care to avoid overflow */
uint64_t WP34S_mulmod(const uint64_t a, uint64_t b, const uint64_t c) {
#ifdef __SIZEOF_INT128__
  __uint128_t x = a * (__uint128_t)b;
#else
  uint64_t x = 0, y = a % c;
  while(b > 0) {
    if((b & 1)) {
      x = (x + y) % c;
    }
    y = (y + y) % c;
    b /= 2;
  }
#endif
  return x % c;
}



/* Calculate (a ^ b) mod c */
uint64_t WP34S_expmod(const uint64_t a, uint64_t b, const uint64_t c) {
  uint64_t x = 1, y = a;
  while(b > 0) {
    if((b & 1)) {
      x = WP34S_mulmod(x, y, c);
    }
    y = WP34S_mulmod(y, y, c);
    b /= 2;
  }
  return (x % c);
}

