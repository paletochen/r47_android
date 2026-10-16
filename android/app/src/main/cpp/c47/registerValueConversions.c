// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"
#include <float.h>
#if defined(PC_BUILD)
  #include <locale.h>
#endif // PC_BUILD

static float fnRealToFloat(const real_t *r);

void convertLongIntegerToLongIntegerRegister(const longInteger_t lgInt, calcRegister_t regist) {
  uint16_t sizeInBytes = longIntegerSizeInBytes(lgInt);

  reallocateRegister(regist, dtLongInteger, TO_BLOCKS(sizeInBytes), longIntegerSignTag(lgInt));
  xcopy(REGISTER_LONG_INTEGER_DATA(regist), lgInt->_mp_d, sizeInBytes);
}



void convertLongIntegerRegisterToLongInteger(calcRegister_t regist, longInteger_t lgInt) {
  uint32_t sizeInBytes = TO_BYTES(getRegisterMaxDataLengthInBlocks(regist));

  longIntegerInitSizeInBits(lgInt, 8 * max(sizeInBytes, LIMB_SIZE));

  xcopy(lgInt->_mp_d, REGISTER_LONG_INTEGER_DATA(regist), sizeInBytes);

  // Trim trailing zero limbs: GMP requires _mp_size = 0 for value zero,
  // else mpz_sizeinbase returns 0 and callers underflow on (bits - 1).
  // GRAMOD = 0 was checked to have _mp_size = 1, not 0
  while(sizeInBytes >= LIMB_SIZE && lgInt->_mp_d[sizeInBytes/LIMB_SIZE - 1] == 0) {
    sizeInBytes -= LIMB_SIZE;
  }

  if(sizeInBytes > 0 && getRegisterLongIntegerSign(regist) == LI_NEGATIVE) {
    lgInt->_mp_size = -(sizeInBytes / LIMB_SIZE);
  }
  else {
    lgInt->_mp_size = sizeInBytes / LIMB_SIZE;
  }
}



void convertLongIntegerRegisterToReal34Register(calcRegister_t source, calcRegister_t destination) {
  longInteger_t lgInt;

  convertLongIntegerRegisterToLongInteger(source, lgInt);
  longIntegerToAllocatedString(lgInt, tmpString, TMP_STR_LENGTH);
  longIntegerFree(lgInt);
  reallocateRegister(destination, dtReal34, REAL34_SIZE_IN_BLOCKS, amNone);
  stringToReal34(tmpString, REGISTER_REAL34_DATA(destination));
}


void convertLongIntegerRegisterToReal34(calcRegister_t source, real34_t *destination) {
  longInteger_t lgInt;

  convertLongIntegerRegisterToLongInteger(source, lgInt);
  longIntegerToAllocatedString(lgInt, tmpString, TMP_STR_LENGTH);
  longIntegerFree(lgInt);
  stringToReal34(tmpString, destination);
}


void convertLongIntegerRegisterToReal(calcRegister_t source, real_t *destination, realContext_t *ctxt) {
  longInteger_t lgInt;

  convertLongIntegerRegisterToLongInteger(source, lgInt);
  convertLongIntegerToReal(lgInt, destination, ctxt);
  longIntegerFree(lgInt);
}



void convertLongIntegerToReal(longInteger_t source, real_t *destination, realContext_t *ctxt) {
  longIntegerToAllocatedString(source, tmpString, TMP_STR_LENGTH);
  stringToReal(tmpString, destination, ctxt);
}


void convertLongIntegerToReal34(longInteger_t source, real34_t *destination) {
  longIntegerToAllocatedString(source, tmpString, TMP_STR_LENGTH);
  stringToReal34(tmpString, destination);
}



void convertLongIntegerToShortIntegerRegister(longInteger_t lgInt, uint32_t base, calcRegister_t destination) {
  uint64_t u64;
  bool_t overflow;

  reallocateRegister(destination, dtShortInteger, SHORT_INTEGER_SIZE_IN_BLOCKS, base);
  if(longIntegerIsZero(lgInt)) {
    *(REGISTER_SHORT_INTEGER_DATA(destination)) = 0;
  }
  else {
    #if defined(OS32BIT) // 32 bit
      u64 = *(uint32_t *)(lgInt->_mp_d);
      if(abs(lgInt->_mp_size) > 1) {
        u64 |= (int64_t)(*(((uint32_t *)(lgInt->_mp_d)) + 1)) << 32;
      }
      overflow = abs(lgInt->_mp_size) > 2 || (u64 & shortIntegerMask) != u64;
      *(REGISTER_SHORT_INTEGER_DATA(destination)) = u64 & shortIntegerMask;
    #else // 64 bit
      u64 = *(uint64_t *)(lgInt->_mp_d);
      overflow = abs(lgInt->_mp_size) > 1 || (u64 & shortIntegerMask) != u64;
      *(REGISTER_SHORT_INTEGER_DATA(destination)) = u64 & shortIntegerMask;
    #endif // OS32BIT

    if(longIntegerIsNegative(lgInt)) {
      clearSystemFlag(FLAG_OVERFLOW);
      *(REGISTER_SHORT_INTEGER_DATA(destination)) = WP34S_intChs(*(REGISTER_SHORT_INTEGER_DATA(destination)));
    }

    if(overflow && !getSystemFlag(FLAG_OVERFLOW)) {
      setSystemFlag(FLAG_OVERFLOW);
    }
  }
}



void convertLongIntegerRegisterToShortIntegerRegister(calcRegister_t source, calcRegister_t destination) {
  longInteger_t lgInt;

  convertLongIntegerRegisterToLongInteger(source, lgInt);
  convertLongIntegerToShortIntegerRegister(lgInt, 10, destination);
  longIntegerFree(lgInt);
}



void convertShortIntegerRegisterToReal34Register(calcRegister_t source, calcRegister_t destination) {
  uint64_t value;
  int16_t sign;
  real34_t lowWord;

  convertShortIntegerRegisterToUInt64(source, &sign, &value);
  reallocateRegister(destination, dtReal34, REAL34_SIZE_IN_BLOCKS, amNone);

  uInt32ToReal34(value >> 32, REGISTER_REAL34_DATA(destination));
  uInt32ToReal34(value & 0x00000000ffffffff, &lowWord);
  real34FMA(REGISTER_REAL34_DATA(destination), const34_2p32, &lowWord, REGISTER_REAL34_DATA(destination));

  if(sign) {
    real34SetNegativeSign(REGISTER_REAL34_DATA(destination));
  }
}



void convertShortIntegerRegisterToReal(calcRegister_t source, real_t *destination, realContext_t *ctxt) {
  uint64_t value;
  int16_t sign;
  real_t lowWord;

  convertShortIntegerRegisterToUInt64(source, &sign, &value);

  uInt32ToReal(value >> 32, destination);
  uInt32ToReal(value & 0x00000000ffffffff, &lowWord);
  realFMA(destination, const_2p32, &lowWord, destination, ctxt);

  if(sign) {
    realSetNegativeSign(destination);
  }
}



void convertShortIntegerRegisterToUInt64(calcRegister_t regist, int16_t *sign, uint64_t *value) {
  *value = *(REGISTER_SHORT_INTEGER_DATA(regist)) & shortIntegerMask;

  if(shortIntegerMode == SIM_UNSIGN) {
    *sign = 0;
  }
  else {
    if(*value & shortIntegerSignBit) { // Negative value
      *sign = 1;

      if(shortIntegerMode == SIM_2COMPL) {
        *value = ((~*value) + 1) & shortIntegerMask;
      }
      else if(shortIntegerMode == SIM_1COMPL) {
        *value = (~*value) & shortIntegerMask;
      }
      else if(shortIntegerMode == SIM_SIGNMT) {
       *value -= shortIntegerSignBit;
      }
      else {
        sprintf(errorMessage, commonBugScreenMessages[bugMsgValueFor], "convertShortIntegerRegisterToUInt64", shortIntegerMode, "shortIntegerMode");
        displayBugScreen(errorMessage);
        *sign = 0;
        *value = 0;
      }
    }
    else { // Positive value
      *sign = 0;
    }
  }
}



void convertShortIntegerRegisterToLongInteger(calcRegister_t source, longInteger_t lgInt) {
  uint64_t value;
  int16_t sign;

  convertShortIntegerRegisterToUInt64(source, &sign, &value);

  longIntegerInit(lgInt);
  uInt32ToLongInteger((uint32_t)(value >> 32), lgInt);
  longIntegerLeftShift(lgInt, 32, lgInt);
  longIntegerAddUInt(lgInt, value & 0x00000000ffffffff, lgInt);

  if(sign) {
    longIntegerChangeSign(lgInt);
  }
}



void convertShortIntegerRegisterToLongIntegerRegister(calcRegister_t source, calcRegister_t destination) {
  longInteger_t lgInt;

  convertShortIntegerRegisterToLongInteger(source, lgInt);

  convertLongIntegerToLongIntegerRegister(lgInt, destination);
  longIntegerFree(lgInt);
}



void convertUInt64ToShortIntegerRegister(int16_t sign, uint64_t value, uint32_t base, calcRegister_t regist) {
  if(sign) { // Negative value
    if((shortIntegerMode == SIM_2COMPL) || (shortIntegerMode == SIM_UNSIGN)) {
      value = (~value) + 1;
    }
    else if(shortIntegerMode == SIM_1COMPL) {
      value = ~value;
    }
    else if(shortIntegerMode == SIM_SIGNMT) {
      value += shortIntegerSignBit;
    }
    else {
      sprintf(errorMessage, commonBugScreenMessages[bugMsgValueFor], "convertUInt64ToShortIntegerRegister", shortIntegerMode, "shortIntegerMode");
      displayBugScreen(errorMessage);
      value = 0;
    }
  }

  reallocateRegister(regist, dtShortInteger, SHORT_INTEGER_SIZE_IN_BLOCKS, base);
  *(REGISTER_SHORT_INTEGER_DATA(regist)) = value & shortIntegerMask;
}



void convertReal34ToLongInteger(const real34_t *re34, longInteger_t lgInt, enum rounding roundingMode) {
  int32_t exponent;
  real34_t real34;

  real34ToIntegralValue(re34, &real34, roundingMode);
  if(!convertReal34ToScaledLongInteger(&real34, lgInt, &exponent)) {
    longIntegerInit(lgInt);
    return;
  }

  while(exponent > 0) {
    longIntegerMultiplyUInt(lgInt, 10, lgInt);
    exponent--;
  }
}



/********************************************//**
 * \brief Converts an integer or finite Real34 register to an integer coefficient and decimal exponent
 *
 * \param[in] regist source register
 * \param[out] coefficient signed integer coefficient, initialised when conversion succeeds
 * \param[out] exponent decimal exponent
 * \return true when conversion succeeds, false for an unsupported or special value
 ***********************************************/
bool_t convertRegisterToScaledLongInteger(calcRegister_t regist, longInteger_t coefficient, int32_t *exponent) {
  *exponent = 0;

  switch(getRegisterDataType(regist)) {
    case dtLongInteger: {
      convertLongIntegerRegisterToLongInteger(regist, coefficient);
      return true;
    }
    case dtShortInteger: {
      convertShortIntegerRegisterToLongInteger(regist, coefficient);
      return true;
    }
    case dtReal34: {
      return convertReal34ToScaledLongInteger(REGISTER_REAL34_DATA(regist), coefficient, exponent);
    }
    default: {
      return false;
    }
  }
}


/********************************************//**
 * \brief Returns the exact parity of an integer register value
 *
 * \param[in] regist source register
 * \return one for odd, zero for even, or minus one for a noninteger or unsupported value
 ***********************************************/
int32_t getRegisterIntegerParity(calcRegister_t regist) {
  longInteger_t integer;

  switch(getRegisterDataType(regist)) {
    case dtLongInteger: {
      convertLongIntegerRegisterToLongInteger(regist, integer);
      break;
    }
    case dtShortInteger: {
      convertShortIntegerRegisterToLongInteger(regist, integer);
      break;
    }
    case dtReal34: {
      if(!real34IsAnInteger(REGISTER_REAL34_DATA(regist))) {
        return -1;
      }
      convertReal34ToLongInteger(REGISTER_REAL34_DATA(regist), integer, DEC_ROUND_DOWN);
      break;
    }
    default: {
      return -1;
    }
  }

  const int32_t parity = longIntegerIsOdd(integer) ? 1 : 0;
  longIntegerFree(integer);
  return parity;
}



/********************************************//**
 * \brief Converts a finite Real34 to its signed integer coefficient and decimal exponent
 *
 * \param[in] real34 source value
 * \param[out] coefficient signed integer coefficient, initialised when the source is finite
 * \param[out] exponent decimal exponent
 * \return true for a finite source, false for a special value
 ***********************************************/
bool_t convertReal34ToScaledLongInteger(const real34_t *real34, longInteger_t coefficient, int32_t *exponent) {
  uint8_t bcd[DECQUAD_Pmax];
  int32_t sign;

  if(real34IsSpecial(real34)) {
    return false;
  }

  sign = real34GetCoefficient(real34, bcd);
  *exponent = real34GetExponent(real34);

  longIntegerInit(coefficient);
  uInt32ToLongInteger(bcd[0], coefficient);

  for(int i=1; i<DECQUAD_Pmax; i++) {
    longIntegerMultiplyUInt(coefficient, 10, coefficient);
    longIntegerAddUInt(coefficient, bcd[i], coefficient);
  }

  if(sign) {
    longIntegerChangeSign(coefficient);
  }

  return true;
}



static uint32_t exactLongIntegerDecimalDigits(const longInteger_t value) {
  uint32_t digits;
  longInteger_t boundary;

  longIntegerInit(boundary);
  digits = longIntegerBase10Digits(value);
  longIntegerPowerUIntUInt(10u, digits - 1, boundary);
  if(longIntegerCompareAbs(value, boundary) < 0) {
    digits--;
  }
  longIntegerFree(boundary);
  return digits;
}



/********************************************//**
 * \brief Converts a signed integer coefficient and decimal exponent to a Real
 *
 * \param[in] coefficient signed integer coefficient
 * \param[in] exponent decimal exponent
 * \param[out] real result value
 * \param[in] realContext destination context
 * \return void
 ***********************************************/
void convertScaledLongIntegerToReal(const longInteger_t coefficient, int32_t exponent, real_t *real, realContext_t *realContext) {
  size_t length;
  uint32_t digits;

  if(longIntegerIsZero(coefficient)) {
    realSetZero(real);
    return;
  }

  digits = longIntegerBase10Digits(coefficient);

  if(digits > (uint32_t)realContext->digits + 2) {
    const uint32_t discarded = digits - ((uint32_t)realContext->digits + 2);
    longInteger_t leading, trailing, boundary;

    longIntegerInit(boundary);
    longIntegerPowerUIntUInt(10u, discarded, boundary);
    longIntegerInit(leading);
    longIntegerInit(trailing);
    longIntegerDivideQuotientRemainder(coefficient, boundary, leading, trailing);
    longIntegerToAllocatedString(leading, tmpString, TMP_STR_LENGTH);
    length = strlen(tmpString);
    if(!longIntegerIsZero(trailing)) {
      tmpString[length++] = '1';
      tmpString[length] = 0;
      exponent += (int32_t)discarded - 1;
    }
    else {
      exponent += (int32_t)discarded;
    }
    longIntegerFree(trailing);
    longIntegerFree(leading);
    longIntegerFree(boundary);
  }
  else {
    longIntegerToAllocatedString(coefficient, tmpString, TMP_STR_LENGTH);
  }

  length = strlen(tmpString);
  snprintf(tmpString + length, TMP_STR_LENGTH - length, "e%" PRId32, exponent);
  stringToReal(tmpString, real, realContext);
}



static void exactResultContext(realContext_t *realContext) {
  *realContext = ctxtReal34;
  if(significantDigits > 0 && significantDigits < 34) {
    realContext->digits = significantDigits;
    realContext->emin += significantDigits - 34;
  }
}



static void convertScaledLongIntegerToReal34Result(const longInteger_t coefficient, int32_t exponent, real34_t *real34) {
  real_t real;
  realContext_t realContext;

  exactResultContext(&realContext);
  convertScaledLongIntegerToReal(coefficient, exponent, &real, &realContext);
  realToReal34(&real, real34);
}



/********************************************//**
 * \brief Converts a signed integer coefficient and decimal exponent to Real34
 *
 * \param[in] coefficient signed integer coefficient
 * \param[in] exponent decimal exponent
 * \param[out] real34 result value
 * \return void
 ***********************************************/
void convertScaledLongIntegerToReal34(const longInteger_t coefficient, int32_t exponent, real34_t *real34) {
  real_t real;

  convertScaledLongIntegerToReal(coefficient, exponent, &real, &ctxtReal34);
  realToReal34(&real, real34);
}



/********************************************//**
 * \brief Adds or subtracts two scaled integer coefficients exactly
 *
 * \p result is initialised by this function and must be freed by the caller.
 *
 * \param[in] first first signed integer coefficient
 * \param[in] firstExponent decimal exponent of the first coefficient
 * \param[in] second second signed integer coefficient
 * \param[in] secondExponent decimal exponent of the second coefficient
 * \param[in] subtract true for subtraction, false for addition
 * \param[out] result exact signed integer coefficient
 * \param[out] resultExponent decimal exponent of the result coefficient
 * \return void
 ***********************************************/
void addSubtractScaledLongIntegers(const longInteger_t first, int32_t firstExponent, const longInteger_t second, int32_t secondExponent, bool_t subtract, longInteger_t result, int32_t *resultExponent) {
  mpz_srcptr firstScaled = first, secondScaled = second;
  longInteger_t scale;

  *resultExponent = min(firstExponent, secondExponent);
  longIntegerInit(scale);
  longIntegerInit(result);
  if(firstExponent > *resultExponent) {
    longIntegerPowerUIntUInt(10u, firstExponent - *resultExponent, result);
    longIntegerMultiplyUnbounded(first, result, scale);
    firstScaled = scale;
  }
  else if(secondExponent > *resultExponent) {
    longIntegerPowerUIntUInt(10u, secondExponent - *resultExponent, result);
    longIntegerMultiplyUnbounded(second, result, scale);
    secondScaled = scale;
  }

  if(subtract) {
    longIntegerSubtractUnbounded(firstScaled, secondScaled, result);
  }
  else {
    longIntegerAddUnbounded(firstScaled, secondScaled, result);
  }

  longIntegerFree(scale);
}



static bool_t rootLongIntegerToReal34Internal(const longInteger_t operand, uint32_t order, bool_t reciprocal, int32_t exponentShift, real34_t *result) {
  if(order < 2u || order > 4u || longIntegerIsZero(operand) || ((order & 1u) == 0u && longIntegerIsNegative(operand))) {
    return false;
  }

  const bool_t negative = longIntegerIsNegative(operand);
  const uint32_t targetDigits = significantDigits > 0 && significantDigits < 34 ? significantDigits : 34;
  int64_t decimalScale;
  longInteger_t magnitude, scale, scaled, discarded, coefficient, remainder;
  longInteger_t midpoint, midpointPower, comparison;

  longIntegerInit(magnitude);
  longIntegerCopy(operand, magnitude);
  longIntegerSetPositiveSign(magnitude);
  const uint32_t digits = exactLongIntegerDecimalDigits(magnitude);
  if(reciprocal) {
    const uint64_t adjustedExponent = digits - 1u;
    int64_t rootExponent = (int64_t)((adjustedExponent + order - 1u) / order);

    if(adjustedExponent % order == 0u) {
      longIntegerInit(scale);
      longIntegerPowerUIntUInt(10u, (uint32_t)adjustedExponent, scale);
      if(longIntegerCompare(magnitude, scale) > 0) {
        rootExponent++;
      }
      longIntegerFree(scale);
    }
    decimalScale = targetDigits - 1 + rootExponent;
  }
  else {
    decimalScale = targetDigits - 1 - (int64_t)((digits - 1u) / order);
  }
  const uint64_t scalePower = order * (uint64_t)(decimalScale < 0 ? -decimalScale : decimalScale);
  if(scalePower > UINT32_MAX || decimalScale < INT32_MIN || decimalScale > INT32_MAX) {
    longIntegerFree(magnitude);
    return false;
  }

  longIntegerInit(scale);
  longIntegerInit(scaled);
  longIntegerInit(discarded);
  longIntegerInit(coefficient);
  longIntegerInit(remainder);
  longIntegerPowerUIntUInt(10u, (uint32_t)scalePower, scale);
  if(reciprocal) {
    longIntegerDivideQuotientRemainder(scale, magnitude, scaled, discarded);
  }
  else if(decimalScale >= 0) {
    longIntegerMultiplyUnbounded(magnitude, scale, scaled);
  }
  else {
    longIntegerDivideQuotientRemainder(magnitude, scale, scaled, discarded);
  }
  mpz_rootrem(coefficient, remainder, scaled, order);
  const bool_t inexact = !longIntegerIsZero(remainder) || !longIntegerIsZero(discarded);

  bool_t increment = false;
  if(inexact) {
    const enum rounding mode = roundingModeTable[roundingMode];

    if(mode == DEC_ROUND_HALF_EVEN || mode == DEC_ROUND_HALF_UP || mode == DEC_ROUND_HALF_DOWN) {
      longIntegerInit(midpoint);
      longIntegerInit(midpointPower);
      longIntegerInit(comparison);
      longIntegerMultiplyUInt(coefficient, 2u, midpoint);
      longIntegerAddUInt(midpoint, 1u, midpoint);
      mpz_pow_ui(midpointPower, midpoint, order);
      if(reciprocal) {
        longIntegerMultiplyUInt(scale, 1u << order, comparison);
        longIntegerMultiplyUnbounded(midpointPower, magnitude, midpointPower);
      }
      else {
        longIntegerMultiplyUInt(magnitude, 1u << order, comparison);
        if(decimalScale >= 0) {
          longIntegerMultiplyUnbounded(comparison, scale, comparison);
        }
        else {
          longIntegerMultiplyUnbounded(midpointPower, scale, midpointPower);
        }
      }
      const int32_t midpointComparison = longIntegerCompare(comparison, midpointPower);
      increment = midpointComparison > 0
                  || (midpointComparison == 0
                      && (mode == DEC_ROUND_HALF_UP || (mode == DEC_ROUND_HALF_EVEN && longIntegerIsOdd(coefficient))));
      longIntegerFree(comparison);
      longIntegerFree(midpointPower);
      longIntegerFree(midpoint);
    }
    else if(mode == DEC_ROUND_UP) {
      increment = true;
    }
    else if(mode == DEC_ROUND_CEILING) {
      increment = !negative;
    }
    else if(mode == DEC_ROUND_FLOOR) {
      increment = negative;
    }
  }

  if(increment) {
    longIntegerAddUInt(coefficient, 1u, coefficient);
  }
  if(negative) {
    longIntegerChangeSign(coefficient);
  }
  convertScaledLongIntegerToReal34Result(coefficient, exponentShift - (int32_t)decimalScale, result);

  longIntegerFree(remainder);
  longIntegerFree(coefficient);
  longIntegerFree(discarded);
  longIntegerFree(scaled);
  longIntegerFree(scale);
  longIntegerFree(magnitude);
  return true;
}



/********************************************//**
 * \brief Rounds a positive square root or signed cube root of a long integer directly to Real34
 *
 * \param[in] operand integer radicand
 * \param[in] order root order from two through four
 * \param[out] result correctly rounded Real34 result
 * \return true for a supported radicand and order, otherwise false
 ***********************************************/
bool_t rootLongIntegerToReal34(const longInteger_t operand, uint32_t order, real34_t *result) {
  return rootLongIntegerToReal34Internal(operand, order, false, 0, result);
}



/********************************************//**
 * \brief Rounds a reciprocal square root or reciprocal signed cube root of a long integer directly to Real34
 *
 * \param[in] operand integer radicand
 * \param[in] order root order from two through four
 * \param[out] result correctly rounded Real34 result
 * \return true for a supported radicand and order, otherwise false
 ***********************************************/
bool_t reciprocalRootLongIntegerToReal34(const longInteger_t operand, uint32_t order, real34_t *result) {
  return rootLongIntegerToReal34Internal(operand, order, true, 0, result);
}



/********************************************//**
 * \brief Rounds the root of an integer or Real34 register once to Real34
 *
 * c·10ᵉ is scaled to an exponent that is a multiple of the order, so its root is the root of the integer c, scaled exactly.
 *
 * \param[in] regist register containing the radicand
 * \param[in] order root order from two through four
 * \param[in] exactExponent true gives an exact root the exponent e/order, false gives it 34 digits as an inexact root has
 * \param[out] result correctly rounded Real34 result
 * \return true for a supported radicand and order, otherwise false
 ***********************************************/
bool_t rootRegisterToReal34(calcRegister_t regist, uint32_t order, bool_t exactExponent, real34_t *result) {
  longInteger_t coefficient, root;
  int32_t exponent;
  bool_t rooted = true;

  if(!convertRegisterToScaledLongInteger(regist, coefficient, &exponent)) {
    return false;
  }
  while(exponent % (int32_t)order != 0) {
    longIntegerMultiplyUInt(coefficient, 10, coefficient);
    exponent--;
  }
  longIntegerInit(root);
  if(exactExponent && longIntegerIsPositive(coefficient) && longIntegerRoot(coefficient, order, root)) {   // an exact root takes the exponent e/order, as decNumber gives an exact square root
    convertScaledLongIntegerToReal34Result(root, exponent / (int32_t)order, result);
  }
  else {
    rooted = rootLongIntegerToReal34Internal(coefficient, order, false, exponent / (int32_t)order, result);
  }
  longIntegerFree(root);
  longIntegerFree(coefficient);
  return rooted;
}



/********************************************//**
 * \brief Evaluates a long integer to a small signed integral power with one Real34 rounding
 *
 * \param[in] base integer base
 * \param[in] exponent exponent from minus four through four, excluding minus one through one
 * \param[out] result correctly rounded Real34 result
 * \return true for a supported base and exponent, otherwise false
 ***********************************************/
bool_t powerLongIntegerToReal34(const longInteger_t base, int32_t exponent, real34_t *result) {
  uint32_t magnitude;
  longInteger_t power;

  if((exponent < -4 || exponent > 4 || (exponent >= -1 && exponent <= 1)) || (exponent < 0 && longIntegerIsZero(base))) {
    return false;
  }
  magnitude = exponent < 0 ? (uint32_t)-exponent : (uint32_t)exponent;

  longIntegerInit(power);
  mpz_pow_ui(power, base, magnitude);
  if(exponent > 0) {
    convertScaledLongIntegerToReal34Result(power, 0, result);
  }
  else {
    longInteger_t one;

    longIntegerInit(one);
    uInt32ToLongInteger(1u, one);
    divideScaledLongIntegersToReal34(one, 0, false, power, 0, result);
    longIntegerFree(one);
  }
  longIntegerFree(power);
  return true;
}



/********************************************//**
 * \brief Adds or subtracts a long integer and a finite Real34 exactly before the Real34 conversion
 *
 * \param[in] integerRegister long integer source register
 * \param[in] real34 Real34 source value
 * \param[in] integerFirst true when the integer is the left operand
 * \param[in] subtract true for subtraction, false for addition
 * \param[out] result Real34 result
 * \return true for a finite Real34 source, false for a special value
 ***********************************************/
bool_t addSubtractLongIntegerReal34(calcRegister_t integerRegister, const real34_t *real34, bool_t integerFirst, bool_t subtract, real34_t *result) {
  int32_t exponent;
  longInteger_t integer, coefficient, combined;
  const bool_t integerNegative = subtract && !integerFirst;
  const bool_t realNegative = real34IsNegative(real34) != (subtract && integerFirst);

  convertLongIntegerRegisterToLongInteger(integerRegister, integer);
  if(!convertReal34ToScaledLongInteger(real34, coefficient, &exponent)) {
    longIntegerFree(integer);
    return false;
  }

  const bool_t negativeZero = longIntegerIsZero(integer) && longIntegerIsZero(coefficient) && integerNegative == realNegative
                              ? integerNegative : ctxtReal34.round == DEC_ROUND_FLOOR;
  if(integerFirst) {
    addSubtractScaledLongIntegers(integer, 0, coefficient, exponent, subtract, combined, &exponent);
  }
  else {
    addSubtractScaledLongIntegers(coefficient, exponent, integer, 0, subtract, combined, &exponent);
  }

  convertScaledLongIntegerToReal34Result(combined, exponent, result);
  if(longIntegerIsZero(combined) && negativeZero) {
    real34SetNegativeSign(result);
  }

  longIntegerFree(combined);
  longIntegerFree(coefficient);
  longIntegerFree(integer);
  return true;
}



/********************************************//**
 * \brief Multiplies two scaled integer coefficients before one Real34 conversion
 *
 * \param[in] first first signed integer coefficient
 * \param[in] firstExponent decimal exponent of the first coefficient
 * \param[in] second second signed integer coefficient
 * \param[in] secondExponent decimal exponent of the second coefficient
 * \param[in] exponentAdjustment additional decimal exponent adjustment
 * \param[in] negativeZero sign to use when the product coefficient is zero
 * \param[out] result Real34 result
 * \return void
 ***********************************************/
static void multiplyScaledLongIntegersToReal34(const longInteger_t first, int32_t firstExponent, const longInteger_t second, int32_t secondExponent, int32_t exponentAdjustment, bool_t negativeZero, real34_t *result) {
  longInteger_t product;

  longIntegerInit(product);
  longIntegerMultiplyUnbounded(first, second, product);
  convertScaledLongIntegerToReal34Result(product, firstExponent + secondExponent + exponentAdjustment, result);
  if(longIntegerIsZero(product) && negativeZero) {
    real34SetNegativeSign(result);
  }
  longIntegerFree(product);
}



/********************************************//**
 * \brief Multiplies a long integer and a finite Real34 exactly before the Real34 conversion
 *
 * \param[in] integerRegister long integer source register
 * \param[in] real34 Real34 source value
 * \param[out] result Real34 result
 * \return true for a finite Real34 source, false for a special value
 ***********************************************/
bool_t multiplyLongIntegerReal34(calcRegister_t integerRegister, const real34_t *real34, real34_t *result) {
  int32_t exponent;
  longInteger_t integer, coefficient;
  const bool_t negative = (getRegisterLongIntegerSign(integerRegister) == LI_NEGATIVE) != real34IsNegative(real34);

  convertLongIntegerRegisterToLongInteger(integerRegister, integer);
  if(!convertReal34ToScaledLongInteger(real34, coefficient, &exponent)) {
    longIntegerFree(integer);
    return false;
  }

  multiplyScaledLongIntegersToReal34(integer, 0, coefficient, exponent, 0, negative, result);

  longIntegerFree(coefficient);
  longIntegerFree(integer);
  return true;
}



/********************************************//**
 * \brief Multiplies two finite register values with a decimal scale and one Real34 rounding
 *
 * \param[in] firstRegister first integer or Real34 source register
 * \param[in] secondRegister second integer or Real34 source register
 * \param[in] exponentAdjustment additional decimal exponent adjustment
 * \param[out] result Real34 result
 * \return true for supported finite operands, otherwise false
 ***********************************************/
bool_t multiplyRegisterValuesToReal34Scaled(calcRegister_t firstRegister, calcRegister_t secondRegister, int32_t exponentAdjustment, real34_t *result) {
  int32_t firstExponent, secondExponent;
  longInteger_t first, second;

  if(!convertRegisterToScaledLongInteger(firstRegister, first, &firstExponent)) {
    return false;
  }
  if(!convertRegisterToScaledLongInteger(secondRegister, second, &secondExponent)) {
    longIntegerFree(first);
    return false;
  }

  bool_t firstNegative = longIntegerIsNegative(first);
  bool_t secondNegative = longIntegerIsNegative(second);
  if(longIntegerIsZero(first) && getRegisterDataType(firstRegister) == dtReal34) {
    firstNegative = real34IsNegative(REGISTER_REAL34_DATA(firstRegister));
  }
  if(longIntegerIsZero(second) && getRegisterDataType(secondRegister) == dtReal34) {
    secondNegative = real34IsNegative(REGISTER_REAL34_DATA(secondRegister));
  }
  multiplyScaledLongIntegersToReal34(first, firstExponent, second, secondExponent, exponentAdjustment, firstNegative != secondNegative, result);

  longIntegerFree(second);
  longIntegerFree(first);
  return true;
}



/********************************************//**
 * \brief Divides two scaled integer coefficients in the supplied context
 *
 * \param[in] numerator signed numerator coefficient
 * \param[in] numeratorExponent decimal exponent of the numerator coefficient
 * \param[in] negativeZero sign to use when the numerator coefficient is zero
 * \param[in] denominator signed denominator coefficient
 * \param[in] denominatorExponent decimal exponent of the denominator coefficient
 * \param[out] result Real result
 * \param[in] realContext destination context
 * \return true for a nonzero denominator, otherwise false
 ***********************************************/
bool_t divideScaledLongIntegersToReal(const longInteger_t numerator, int32_t numeratorExponent, bool_t negativeZero, const longInteger_t denominator, int32_t denominatorExponent, real_t *result, realContext_t *realContext) {
  int32_t scale;
  longInteger_t scaledNumerator, scaledDenominator, power, quotient, remainder;

  if(longIntegerIsZero(denominator)) {
    return false;
  }

  if(longIntegerIsZero(numerator)) {
    realSetZero(result);
    if(negativeZero) {
      realSetNegativeSign(result);
    }
    return true;
  }

  const bool_t negative = longIntegerIsNegative(numerator) != longIntegerIsNegative(denominator);
  // Each base-10 digit count can exceed the exact count by one; two extra places guarantee a rounding digit.
  scale = realContext->digits + 2 + longIntegerBase10Digits(denominator) - longIntegerBase10Digits(numerator);

  longIntegerInit(scaledNumerator);
  longIntegerInit(scaledDenominator);
  longIntegerInit(power);
  longIntegerInit(quotient);
  longIntegerInit(remainder);
  longIntegerCopy(numerator, scaledNumerator);
  longIntegerCopy(denominator, scaledDenominator);
  longIntegerSetPositiveSign(scaledNumerator);
  longIntegerSetPositiveSign(scaledDenominator);
  if(scale >= 0) {
    longIntegerPowerUIntUInt(10u, scale, power);
    longIntegerMultiplyUnbounded(scaledNumerator, power, scaledNumerator);
  }
  else {
    longIntegerPowerUIntUInt(10u, -scale, power);
    longIntegerMultiplyUnbounded(scaledDenominator, power, scaledDenominator);
  }
  longIntegerDivideQuotientRemainder(scaledNumerator, scaledDenominator, quotient, remainder);

  if(!longIntegerIsZero(remainder)) {
    longIntegerMultiplyUInt(quotient, 10u, quotient);
    longIntegerAddUInt(quotient, 1u, quotient);
    scale++;
  }
  if(negative) {
    longIntegerChangeSign(quotient);
  }

  convertScaledLongIntegerToReal(quotient, numeratorExponent - denominatorExponent - scale, result, realContext);
  longIntegerFree(remainder);
  longIntegerFree(quotient);
  longIntegerFree(power);
  longIntegerFree(scaledDenominator);
  longIntegerFree(scaledNumerator);
  return true;
}



/********************************************//**
 * \brief Divides two scaled integer coefficients with one result rounding
 *
 * \param[in] numerator signed numerator coefficient
 * \param[in] numeratorExponent decimal exponent of the numerator coefficient
 * \param[in] negativeZero sign to use when the numerator coefficient is zero
 * \param[in] denominator signed denominator coefficient
 * \param[in] denominatorExponent decimal exponent of the denominator coefficient
 * \param[out] result Real34 result
 * \return true for a nonzero denominator, otherwise false
 ***********************************************/
bool_t divideScaledLongIntegersToReal34(const longInteger_t numerator, int32_t numeratorExponent, bool_t negativeZero, const longInteger_t denominator, int32_t denominatorExponent, real34_t *result) {
  real_t real;
  realContext_t realContext;

  exactResultContext(&realContext);
  const bool_t ok = divideScaledLongIntegersToReal(numerator, numeratorExponent, negativeZero, denominator, denominatorExponent, &real, &realContext);
  if(ok) {
    realToReal34(&real, result);
  }
  return ok;
}



/********************************************//**
 * \brief Divides a long integer and a finite Real34 with one result rounding
 *
 * \param[in] integerRegister long integer source register
 * \param[in] real34 Real34 source value
 * \param[in] integerFirst true when the integer is the numerator
 * \param[out] result Real34 result
 * \return true for finite operands and a nonzero denominator, otherwise false
 ***********************************************/
bool_t divideLongIntegerReal34(calcRegister_t integerRegister, const real34_t *real34, bool_t integerFirst, real34_t *result) {
  int32_t exponent;
  longInteger_t integer, coefficient;

  convertLongIntegerRegisterToLongInteger(integerRegister, integer);
  if(!convertReal34ToScaledLongInteger(real34, coefficient, &exponent)) {
    longIntegerFree(integer);
    return false;
  }

  bool_t ok;
  if(integerFirst) {
    ok = divideScaledLongIntegersToReal34(integer, 0, real34IsNegative(real34), coefficient, exponent, result);
  }
  else {
    const bool_t negativeZero = longIntegerIsZero(coefficient) && (real34IsNegative(real34) != longIntegerIsNegative(integer));
    ok = divideScaledLongIntegersToReal34(coefficient, exponent, negativeZero, integer, 0, result);
  }
  longIntegerFree(coefficient);
  longIntegerFree(integer);
  return ok;
}



/********************************************//**
 * \brief Divides two finite integer or Real34 register values with a decimal scale and one Real34 rounding
 *
 * \param[in] numeratorRegister numerator source register
 * \param[in] denominatorRegister denominator source register
 * \param[in] exponentAdjustment additional decimal exponent adjustment
 * \param[out] result Real34 result
 * \return true for finite operands and a nonzero denominator, otherwise false
 ***********************************************/
bool_t divideRegisterValuesToReal34Scaled(calcRegister_t numeratorRegister, calcRegister_t denominatorRegister, int32_t exponentAdjustment, real34_t *result) {
  int32_t numeratorExponent, denominatorExponent;
  longInteger_t numerator, denominator;

  if(!convertRegisterToScaledLongInteger(numeratorRegister, numerator, &numeratorExponent)) {
    return false;
  }
  if(!convertRegisterToScaledLongInteger(denominatorRegister, denominator, &denominatorExponent)) {
    longIntegerFree(numerator);
    return false;
  }

  bool_t negativeZero = false;
  if(longIntegerIsZero(numerator)) {
    const bool_t numeratorNegative = getRegisterDataType(numeratorRegister) == dtReal34 && real34IsNegative(REGISTER_REAL34_DATA(numeratorRegister));
    negativeZero = numeratorNegative != longIntegerIsNegative(denominator);
  }
  const bool_t ok = divideScaledLongIntegersToReal34(numerator, numeratorExponent + exponentAdjustment, negativeZero, denominator, denominatorExponent, result);
  longIntegerFree(denominator);
  longIntegerFree(numerator);
  return ok;
}



/********************************************//**
 * \brief Divides two finite integer or Real34 register values with one Real34 rounding
 *
 * \param[in] numeratorRegister numerator source register
 * \param[in] denominatorRegister denominator source register
 * \param[out] result Real34 result
 * \return true for finite operands and a nonzero denominator, otherwise false
 ***********************************************/
bool_t divideRegisterValuesToReal34(calcRegister_t numeratorRegister, calcRegister_t denominatorRegister, real34_t *result) {
  return divideRegisterValuesToReal34Scaled(numeratorRegister, denominatorRegister, 0, result);
}



/********************************************//**
 * \brief Divides one by a long integer with one Real34 rounding
 *
 * \param[in] denominatorRegister long or short integer denominator register
 * \param[out] result Real34 result
 * \return true for a nonzero denominator, otherwise false
 ***********************************************/
bool_t reciprocalLongIntegerRegisterToReal34(calcRegister_t denominatorRegister, real34_t *result) {
  longInteger_t numerator, denominator;

  longIntegerInit(numerator);
  uInt32ToLongInteger(1u, numerator);
  getRegisterAsLongIntQuiet(denominatorRegister, denominator, NULL);
  const bool_t ok = divideScaledLongIntegersToReal34(numerator, 0, false, denominator, 0, result);
  longIntegerFree(denominator);
  longIntegerFree(numerator);
  return ok;
}



/********************************************//**
 * \brief Evaluates a finite scalar base raised to an exponent of plus or minus one
 *
 * \param[in] baseRegister scalar base register
 * \param[in] exponentRegister Real34 exponent register
 * \param[out] result Real34 result
 * \return true when the identity was evaluated, otherwise false
 ***********************************************/
bool_t registerIdentityExponentToReal34(calcRegister_t baseRegister, calcRegister_t exponentRegister, real34_t *result) {
  if(getRegisterDataType(exponentRegister) != dtReal34 || !real34CompareAbsEqual(REGISTER_REAL34_DATA(exponentRegister), const34_1)) {
    return false;
  }

  const bool_t reciprocal = real34IsNegative(REGISTER_REAL34_DATA(exponentRegister));
  switch(getRegisterDataType(baseRegister)) {
    case dtLongInteger: {
      if(reciprocal) {
        return reciprocalLongIntegerRegisterToReal34(baseRegister, result);
      }
      longInteger_t base;

      convertLongIntegerRegisterToLongInteger(baseRegister, base);
      convertScaledLongIntegerToReal34Result(base, 0, result);
      longIntegerFree(base);
      return true;
    }
    case dtReal34: {
      if(reciprocal) {
        if(real34IsSpecial(REGISTER_REAL34_DATA(baseRegister)) || real34IsZero(REGISTER_REAL34_DATA(baseRegister))) {
          return false;
        }
        real34Divide(const34_1, REGISTER_REAL34_DATA(baseRegister), result);
      }
      else {
        real34Copy(REGISTER_REAL34_DATA(baseRegister), result);
      }
      return true;
    }
    default: {
      return false;
    }
  }
}



void convertReal34ToLongIntegerRegister(const real34_t *real34, calcRegister_t dest, enum rounding roundingMode) {
  longInteger_t lgInt;

  convertReal34ToLongInteger(real34, lgInt, roundingMode);
  convertLongIntegerToLongIntegerRegister(lgInt, dest);

  longIntegerFree(lgInt);
}



void convertReal34ToReal34Register(const real34_t *real34, calcRegister_t dest) {
  reallocateRegister(dest, dtReal34, REAL34_SIZE_IN_BLOCKS, amNone);
  real34Copy(real34, REGISTER_REAL34_DATA(dest));
}



void convertReal34ToComplex34Register(const real34_t *real34, const real34_t *imag34, calcRegister_t dest) {
  reallocateRegister(dest, dtComplex34, COMPLEX34_SIZE_IN_BLOCKS, amNone);
  real34Copy(real34, REGISTER_REAL34_DATA(dest));
  real34Copy(imag34, REGISTER_IMAG34_DATA(dest));
}



void convertRealToLongInteger(const real_t *re, longInteger_t lgInt, enum rounding roundingMode) {
  uint8_t bcd[75];
  int32_t sign, exponent;
  real_t real;

  realToIntegralValue(re, &real, roundingMode, &ctxtReal39);
  realGetCoefficient(&real, bcd);
  sign = (realIsNegative(&real) ? 1 : 0);
  exponent = real.exponent;

  uInt32ToLongInteger(bcd[0], lgInt);

  for(int i=1; i<real.digits; i++) {
    longIntegerMultiplyUInt(lgInt, 10, lgInt);
    longIntegerAddUInt(lgInt, bcd[i], lgInt);
  }

  while(exponent > 0) {
    longIntegerMultiplyUInt(lgInt, 10, lgInt);
    exponent--;
  }

  if(sign) {
    longIntegerChangeSign(lgInt);
  }
}



void convertRealToLongIntegerRegister(const real_t *real, calcRegister_t dest, enum rounding roundingMode) {
  longInteger_t lgInt;

  longIntegerInit(lgInt);

  convertRealToLongInteger(real, lgInt, roundingMode);
  convertLongIntegerToLongIntegerRegister(lgInt, dest);
  longIntegerFree(lgInt);
}



void convertComplexRegisterToRealIfZeroImag(calcRegister_t regist) {
  real_t b;
  if(real34IsZero(REGISTER_IMAG34_DATA(regist))) {
    real34ToReal(REGISTER_REAL34_DATA(regist), &b);
    convertRealToResultRegister(&b, regist, amNone);
  }
}



void realToIntegralValue(const real_t *source, real_t *destination, enum rounding mode, realContext_t *realContext) {
  enum rounding savedRoundingMode;

  savedRoundingMode = realContext->round;
  realContext->round = mode;
  realContext->status = 0;
  decNumberToIntegralValue(destination, source, realContext);
  realContext->round = savedRoundingMode;
}



void convertRealToReal34ResultRegister(const real_t *real, calcRegister_t dest) {
  if(significantDigits == 0 || significantDigits >= 34) {
    realToReal34(real, REGISTER_REAL34_DATA(dest));
  }
  else {
    real_t rounded;
    roundToSignificantDigits(real, &rounded, significantDigits, &ctxtReal34);
    realToReal34(&rounded, REGISTER_REAL34_DATA(dest));
  }
}

void convertRealToImag34ResultRegister(const real_t *real, calcRegister_t dest) {
  if(significantDigits == 0 || significantDigits >= 34) {
    realToReal34(real, REGISTER_IMAG34_DATA(dest));
  }
  else {
    real_t rounded;
    roundToSignificantDigits(real, &rounded, significantDigits, &ctxtReal34);
    realToReal34(&rounded, REGISTER_IMAG34_DATA(dest));
  }
}

void convertRealToResultRegister(const real_t *x, calcRegister_t dest, angularMode_t angle) {
  reallocateRegister(dest, dtReal34, REAL34_SIZE_IN_BLOCKS, angle);
  convertRealToReal34ResultRegister(x, dest);
}

void convertComplexToResultRegister(const cplx_t *z, calcRegister_t dest) {
  convertComplexPartsToResultRegister(&z->Real, &z->Imag, dest);
}

// convertComplexToResultRegister for a real and an imaginary part that are not one cplx_t, such as constants or parts computed apart
void convertComplexPartsToResultRegister(const real_t *real, const real_t *imag, calcRegister_t dest) {
  reallocateRegister(dest, dtComplex34, COMPLEX34_SIZE_IN_BLOCKS, amNone);
  convertRealToReal34ResultRegister(real, dest);
  convertRealToImag34ResultRegister(imag, dest);
}

void convertComplexToResultRegisterRPangle(const real_t *real, const real_t *imag, calcRegister_t dest, angularMode_t angl, uint8_t polarTag) {
  reallocateRegister(dest, dtComplex34, COMPLEX34_SIZE_IN_BLOCKS, angl);
  convertRealToReal34ResultRegister(real, dest);
  convertRealToImag34ResultRegister(imag, dest);
  setComplexRegisterPolarMode(dest, polarTag);
}

void convertTimeRegisterToReal34Register(calcRegister_t source, calcRegister_t destination) {
  real34_t real34;
  real34Copy(REGISTER_REAL34_DATA(source), &real34);
  reallocateRegister(destination, dtReal34, REAL34_SIZE_IN_BLOCKS, amNone);
  real34Divide(&real34, const34_3600, REGISTER_REAL34_DATA(destination));
}



void convertReal34RegisterToTimeRegister(calcRegister_t source, calcRegister_t destination) {
  real34_t real34;
  real34Copy(REGISTER_REAL34_DATA(source), &real34);
  reallocateRegister(destination, dtTime, REAL34_SIZE_IN_BLOCKS, amNone);
  real34Multiply(&real34, const34_3600, REGISTER_REAL34_DATA(destination));
}



void convertLongIntegerRegisterToTimeRegister(calcRegister_t source, calcRegister_t destination) {
  convertLongIntegerRegisterToReal34Register(source, destination);
  convertReal34RegisterToTimeRegister(destination, destination);
}



void convertDateRegisterToReal34Register(calcRegister_t source, calcRegister_t destination) {
  real34_t y, m, d, j;
  bool_t isNegative;

  internalDateToJulianDay(REGISTER_REAL34_DATA(source), &j);
  decomposeJulianDay(&j, &y, &m, &d);
  isNegative = real34IsNegative(&y);
  real34SetPositiveSign(&y);

  if(getSystemFlag(FLAG_YMD)) {
    real34Divide(&m, const34_100, &m);
    real34Multiply(&d, const34_1e_4, &d);
  }
  else if(getSystemFlag(FLAG_MDY)) {
    real34Divide(&d, const34_100, &d);
    real34Divide(&y, const34_1e6, &y);
  }
  else if(getSystemFlag(FLAG_DMY)) {
    real34Divide(&m, const34_100, &m);
    real34Divide(&y, const34_1e6, &y);
  }

  reallocateRegister(destination, dtReal34, REAL34_SIZE_IN_BLOCKS, amNone);
  real34Add(&y, &m, REGISTER_REAL34_DATA(destination));
  real34Add(REGISTER_REAL34_DATA(destination), &d, REGISTER_REAL34_DATA(destination));
  if(isNegative) {
    real34SetNegativeSign(REGISTER_REAL34_DATA(destination));
  }
}



void convertReal34RegisterToDateRegister(calcRegister_t source, calcRegister_t destination, bool_t handleYY) {
  real34_t part1, part2, part3, val;
  bool_t isNegative;

  isNegative = real34IsNegative(REGISTER_REAL34_DATA(source));
  real34CopyAbs(REGISTER_REAL34_DATA(source), &part2);
  real34ToIntegralValue(&part2, &part1, DEC_ROUND_DOWN);      // Y D or M

  real34Subtract(&part2, &part1, &part2);
  real34Multiply(&part2, const34_100, &part2);
  real34Copy(&part2, &part3);
  real34ToIntegralValue(&part2, &part2, DEC_ROUND_DOWN);      // M or D

  real34Subtract(&part3, &part2, &part3);
  int32ToReal34(getSystemFlag(FLAG_YMD) ? 100 : 10000, &val);
  real34Multiply(&part3, &val, &part3);
  real34ToIntegralValue(&part3, &part3, DEC_ROUND_DOWN);      // D or Y

  if(isNegative) {
    if(getSystemFlag(FLAG_YMD)) {
      real34SetNegativeSign(&part1);
    }
    else {
      real34SetNegativeSign(&part3);
    }
  }


  uint16_t lastCenturyHighUsedtmp;
  if(handleYY) {
    //get the actual active YYYY value, excluding the tracking flag
    lastCenturyHighUsedtmp = lastCenturyHighUsed & (YY_MASK_TRACKING - 1);

    // remember last used century if the century is not an abbreviation, i.e. if YYYY > 100, ignore neg value YYYY
    if(getSystemFlag(FLAG_YMD)) {
      if(real34CompareGreaterEqual(&part1, const34_100)) {
        lastCenturyHighUsedtmp = 100*(int16_t)(real34ToInt32(&part1) / 100) + 99;
      }
    }
    //FLAG_MDY //FLAG_DMY
    else if(real34CompareGreaterEqual(&part3, const34_100)) {
      lastCenturyHighUsedtmp = 100*(int16_t)(real34ToInt32(&part3) / 100) + 99;
    }



    //No YYYY digits, i.e. no year given at all, so we use the current year.
    if(!(lastCenturyHighUsed & 0x8000)) {
      #if defined(DMCP_BUILD)
        tm_t timeInfo;
        dt_t dateInfo;
        rtc_read(&timeInfo, &dateInfo);
      #elif defined(PC_BUILD) // PC_BUILD
        time_t epoch = time(NULL);
        struct tm *timeInfo = localtime(&epoch);
      #endif // PC_BUILD

      if(getSystemFlag(FLAG_YMD)) {
        if(real34IsZero(&part1)) {
          #if defined(DMCP_BUILD)
            uInt32ToReal34(dateInfo.year, &part1);
          #elif defined(PC_BUILD) // PC_BUILD
            uInt32ToReal34(timeInfo->tm_year + 1900, &part1);
          #endif // PC_BUILD
        }
      }
      //FLAG_MDY //FLAG_DMY
      else if(real34IsZero(&part3)) {
        #if defined(DMCP_BUILD)
          uInt32ToReal34(dateInfo.year, &part3);
        #elif defined(PC_BUILD) // PC_BUILD
          uInt32ToReal34(timeInfo->tm_year + 1900, &part3);
        #endif // PC_BUILD
      }
    }



    //Only YY digits
    // thresholdYYHigh = 1950 :     Automatic, 2024 ==> 2000-2099. 29 ==> 2029, 59 ==> 2059
    //                              1950-2049. 29 ==> 2029, 59 ==> 1959
    //                              if yy > 49, then yy += 1900 else yy += 2000
    int16_t thresholdYYHigh = max(0, (int16_t)(lastCenturyHighUsed & (YY_MASK_TRACKING - 1)) - 99);
    if(getSystemFlag(FLAG_YMD)) {
      if(!(lastCenturyHighUsed & YY_MASK_OFF) && real34CompareLessThan(&part1, const34_100)) {
        int16_t yy = (int16_t)(real34ToInt32(&part1));
        if(yy >= (thresholdYYHigh) % 100) {
          yy += (thresholdYYHigh - thresholdYYHigh % 100);
        }
        else {
          yy += (thresholdYYHigh - thresholdYYHigh % 100) + 100;
        }
        int32ToReal34((int32_t)yy, &part1);
      }
    }
    //FLAG_MDY //FLAG_DMY
    else if(!(lastCenturyHighUsed & YY_MASK_OFF) && real34CompareLessThan(&part3, const34_100)) {
      int16_t yy = (int16_t)(real34ToInt32(&part3));
      if(yy >= (thresholdYYHigh) % 100) {
        yy += (thresholdYYHigh - thresholdYYHigh % 100);
      }
      else {
        yy += (thresholdYYHigh - thresholdYYHigh % 100) + 100;
      }
      int32ToReal34((int32_t)yy, &part3);
    }
  }



  if((getSystemFlag(FLAG_YMD) && !isValidDay(&part1, &part2, &part3)) ||
    ( getSystemFlag(FLAG_MDY) && !isValidDay(&part3, &part1, &part2)) ||
    ( getSystemFlag(FLAG_DMY) && !isValidDay(&part3, &part2, &part1))) {
      displayCalcErrorMessage(ERROR_BAD_TIME_OR_DATE_INPUT, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function convertReal34RegisterToDateRegister:", "Invalid date input like 30 Feb.", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
  }


  //update stored YYYY and add the control bits again
  if(handleYY && !(lastCenturyHighUsed & YY_MASK_OFF) && (lastCenturyHighUsed & YY_MASK_TRACKING)) {
    lastCenturyHighUsed = (lastCenturyHighUsed & ~(YY_MASK_TRACKING - 1)) | (lastCenturyHighUsedtmp & (YY_MASK_TRACKING - 1));
  }

  reallocateRegister(destination, dtDate, REAL34_SIZE_IN_BLOCKS, amNone);
  if(getSystemFlag(FLAG_YMD)) {
    composeJulianDay(&part1, &part2, &part3, REGISTER_REAL34_DATA(destination));
  }
  else if(getSystemFlag(FLAG_MDY)) {
    composeJulianDay(&part3, &part1, &part2, REGISTER_REAL34_DATA(destination));
  }
  else if(getSystemFlag(FLAG_DMY)) {
    composeJulianDay(&part3, &part2, &part1, REGISTER_REAL34_DATA(destination));
  }

  real34Multiply(REGISTER_REAL34_DATA(destination), const34_86400, REGISTER_REAL34_DATA(destination));
  real34Add(REGISTER_REAL34_DATA(destination), const34_43200, REGISTER_REAL34_DATA(destination));
}



void convertReal34MatrixRegisterToReal34Matrix(calcRegister_t regist, real34Matrix_t *matrix) {
  matrixHeader_t *matrixHeader = REGISTER_MATRIX_HEADER(regist);

  if(realMatrixInit(matrix, matrixHeader->matrixRows, matrixHeader->matrixColumns)) {
    if(matrix->matrixElements) {
      xcopy(matrix->matrixElements, REGISTER_REAL34_MATRIX_ELEMENTS(regist), (matrix->header.matrixColumns * matrix->header.matrixRows) * REAL34_SIZE_IN_BYTES);
    }
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
  }
}

void convertReal34MatrixToReal34MatrixRegister(const real34Matrix_t *matrix, calcRegister_t regist) {
  const uint32_t neededSizeInBytes = (matrix->header.matrixColumns * matrix->header.matrixRows) * REAL34_SIZE_IN_BYTES;
  reallocateRegister(regist, dtReal34Matrix, TO_BLOCKS(neededSizeInBytes), amNone);
  if(lastErrorCode != ERROR_RAM_FULL) {
    xcopy(REGISTER_MATRIX_HEADER(regist), matrix, sizeof(matrixHeader_t));
    xcopy(REGISTER_REAL34_MATRIX_ELEMENTS(regist), matrix->matrixElements, neededSizeInBytes);
  }
}

void convertComplex34MatrixRegisterToComplex34Matrix(calcRegister_t regist, complex34Matrix_t *matrix) {
  matrixHeader_t *matrixHeader = REGISTER_MATRIX_HEADER(regist);

  if(complexMatrixInit(matrix, matrixHeader->matrixRows, matrixHeader->matrixColumns)) {
    if(matrix->matrixElements) {
      xcopy(matrix->matrixElements, REGISTER_COMPLEX34_MATRIX_ELEMENTS(regist), (matrix->header.matrixColumns * matrix->header.matrixRows) * COMPLEX34_SIZE_IN_BYTES);
    }
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
  }
}

void convertComplex34MatrixToComplex34MatrixRegister(const complex34Matrix_t *matrix, calcRegister_t regist) {
  const uint32_t neededSizeInBytes = (matrix->header.matrixColumns * matrix->header.matrixRows) * COMPLEX34_SIZE_IN_BYTES;
  reallocateRegister(regist, dtComplex34Matrix, TO_BLOCKS(neededSizeInBytes), amNone);
  if(lastErrorCode != ERROR_RAM_FULL) {
    xcopy(REGISTER_MATRIX_HEADER(regist), matrix, sizeof(matrixHeader_t));
    xcopy(REGISTER_COMPLEX34_MATRIX_ELEMENTS(regist), matrix->matrixElements, neededSizeInBytes);
  }
}

void convertReal34MatrixToComplex34Matrix(const real34Matrix_t *source, complex34Matrix_t *destination) {
  if(complexMatrixInit(destination, source->header.matrixRows, source->header.matrixColumns)) {
    if(destination->matrixElements) {
      for(uint16_t i = 0; i < source->header.matrixRows * source->header.matrixColumns; ++i) {
        real34Copy(&source->matrixElements[i], VARIABLE_REAL34_DATA(&destination->matrixElements[i]));
        real34SetZero(VARIABLE_IMAG34_DATA(&destination->matrixElements[i]));
      }
    }
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
  }
}

void convertReal34MatrixRegisterToComplex34Matrix(calcRegister_t source, complex34Matrix_t *destination) {
  real34Matrix_t matrix;
  linkToRealMatrixRegister(source, &matrix);
  convertReal34MatrixToComplex34Matrix(&matrix, destination);
}

void convertReal34MatrixRegisterToComplex34MatrixRegister(calcRegister_t source, calcRegister_t destination) {
  complex34Matrix_t matrix;
  convertReal34MatrixRegisterToComplex34Matrix(source, &matrix);
  convertComplex34MatrixToComplex34MatrixRegister(&matrix, destination);
  setComplexRegisterAngularMode(destination, currentAngularMode);
  setComplexRegisterPolarMode(destination, getSystemFlag(FLAG_POLAR) ? amPolar : 0);
  complexMatrixFree(&matrix);
}


const char *doubleSpecialLabel(double value) {                                 // Returns "NaN", "+Inf" or "-Inf" for a special value, or NULL for a normal finite value.
  if(value != value) {
    return "NaN";
  }
  if(value > DBL_MAX) {
    return "+Inf";
  }
  if(value < -DBL_MAX) {
    return "-Inf";
  }
  return NULL;
}



void sci_fmt(char *buf, int n, double x) {
/*
 * Usage:
 *   char buf[32];
 *   sci_fmt(buf, sizeof(buf), x);  // replaces snprintf(buf, sizeof(buf), "%.16e", x);
 *
 * Output format (if buffer allows):
 *   [-]d.dddddddddddddddde±dd\0 (up to 25–30 bytes depending on exponent digits)
 *   eg. 2.5 -> "2.50000000000000e+00", -0.05 -> "-5.00000007450580e-02", 0 -> "0.00000000000000e+00"
 *   NaN -> "nan", +Inf -> "inf", -Inf -> "-inf"; compatible with strtof
 */
    int exp = 0, i = 0;
    if(isnan(x)) {
      snprintf(buf, n, "nan");
      return;
    }
    if(isinf(x)) {
      snprintf(buf, n, x < 0 ? "-inf" : "inf");
      return;
    }
    if(x < 0) {
        buf[i++] = '-';
        x = -x;
    }

    while(x && x < 1.0) {
      x *= 10.0, exp--;
    }
    while(x >= 10.0) {
      x /= 10.0, exp++;
    }

    unsigned long long m = (unsigned long long)(x * 1e15 + 0.5);
    if(m >= 10000000000000000ULL) {
        m /= 10;
        exp++;
    }

    buf[i++] = '0' + (m / 1000000000000000ULL);
    buf[i++] = '.';

    static const unsigned long long divs[] = {
        1000000000000000ULL, 100000000000000ULL, 10000000000000ULL,
        1000000000000ULL,   100000000000ULL,    10000000000ULL,
        1000000000ULL,      100000000ULL,       10000000ULL,
        1000000ULL,         100000ULL,          10000ULL,
        1000ULL,            100ULL,             10ULL
    };

    for(int j = 1; j < 15 && i < n - 6; j++) {
        buf[i++] = '0' + (m / divs[j]) % 10;
    }

    i += snprintf(buf + i, n - i, "e%+03d", exp);
    buf[i] = 0;
}



  void convertDoubleToString(double x, int16_t n, char *buff) { //Reformatting real strings that are formatted according to different locale settings
    const char *special = doubleSpecialLabel(x);
    if(special != NULL) {
      snprintf(buff, n, "%s", special);  // "NaN", "+Inf" or "-Inf" for a special value
      return;
    }
    uint16_t i = 2;
    uint16_t j = 2;
    bool_t error = false;

//    snprintf(buff, n, "%.16e", x);
    sci_fmt(buff, n, x);

    if(buff[0] != '-') {
      i = 0;
      while(buff[i] != 0) {
        i++;
      }
      buff[i+1] = 0;
      while(i != 0) {
        buff[i] = buff[i-1];
        i--;
      }
      buff[0] = '+';
    }

    if(buff[0]!=0 && (buff[1]=='+' || buff[1]!='-') && (buff[2]=='.' || buff[2]==',')) {
      buff[2] = '.';
      i = 3;
      j = 3;
      while(buff[i] != 0) {
        if(buff[i]==',' || buff[i]=='.' || buff[i]==' ') {
          buff[j] = 0;
        }
        else {
          buff[j] = buff[i];
          j++;
        }
        i++;
      }
      buff[j] = 0;
    }
    else {
      error = true;
    }

    //debug code to check for locale error by forcing a real conversion
    //  stringToReal34(buff, REGISTER_REAL34_DATA(REGISTER_X));
    //  if(real34IsNaN(REGISTER_REAL34_DATA(REGISTER_X))) error = true;

      if(error) {
        #if defined(PC_BUILD)
          printf("ERROR in locale: doubleToString: attempt to correct:  §%s§\n", buff);
          snprintf(buff, 100, "%.16e", x);
          printf("                                 Original conversion: §%s§\n", buff);
        #endif //PC_BUILD
        strcpy(buff, "NaN");
      }
    }


  void convertDoubleToReal(double x, real_t *destination, realContext_t *ctxt) {
    char buff[100];

    buff[0]=0;
    convertDoubleToString(x, 100, buff);
    stringToReal(buff, destination, ctxt);
  }

  void convertDoubleToReal34Register(double x, calcRegister_t destination) {
    char buff[100];

    reallocateRegister(destination, dtReal34, REAL34_SIZE_IN_BLOCKS, amNone);
    convertDoubleToString(x, 100, buff);
    stringToReal34(buff, REGISTER_REAL34_DATA(destination));

    #if defined(PC_BUILD)
      if(real34IsNaN(REGISTER_REAL34_DATA(destination))) {
        snprintf(buff, 100, "%.16e", x);
        printf("ERROR in convertDoubleToReal34Register: %s\n", buff);
      }
    #endif // PC_BUILD
  }


  void convertDoubleToReal34RegisterPush(double x, calcRegister_t destination) {
    setSystemFlag(FLAG_ASLIFT);
    liftStack();
    convertDoubleToReal34Register(x, destination);
    setSystemFlag(FLAG_ASLIFT);
  }




double convertRegisterToDouble(calcRegister_t regist) {
  cplx_t regValue;

  return getRegisterAsComplex(regist, &regValue) ? fnRealToFloat(&regValue.Real) : DOUBLE_NOT_INIT;
}


//Pauli volunteered this fuction, rev 1 2021-10-10
#if DECDPUN != 3
  #error DECDPUN must be 3
#endif

static float fnRealToFloat(const real_t *r){
  int s = 0;
  int j, n, e;

  TO_QSPI static const float exps[] = {
    1.e-45, 1.e-44, 1.e-43, 1.e-42, 1.e-41, 1.e-40, 1.e-39, 1.e-38,
    1.e-37, 1.e-36, 1.e-35, 1.e-34, 1.e-33, 1.e-32, 1.e-31, 1.e-30,
    1.e-29, 1.e-28, 1.e-27, 1.e-26, 1.e-25, 1.e-24, 1.e-23, 1.e-22,
    1.e-21, 1.e-20, 1.e-19, 1.e-18, 1.e-17, 1.e-16, 1.e-15, 1.e-14,
    1.e-13, 1.e-12, 1.e-11, 1.e-10, 1.e-9,  1.e-8,  1.e-7,  1.e-6,
    1.e-5,  1.e-4,  1.e-3,  1.e-2,  1.e-1,  1.e0,   1.e1,   1.e2,
    1.e3,   1.e4,   1.e5,   1.e6,   1.e7,   1.e8,   1.e9,
    1.e10,  1.e11,  1.e12,  1.e13,  1.e14,  1.e15,  1.e16,  1.e17,
    1.e18,  1.e19,  1.e20,  1.e21,  1.e22,  1.e23,  1.e24,  1.e25,
    1.e26,  1.e27,  1.e28,  1.e29,  1.e30,  1.e31,  1.e32,  1.e33,
    1.e34,  1.e35,  1.e36,  1.e37,  1.e38
  };

  if(realIsSpecial(r)) {
    if(realIsNaN(r)) {
      return 0. / 0.;
    }
    goto infinite;
  }
  if(realIsZero(r)) {
    goto zero;
  }

  j = (r->digits + DECDPUN-1) / DECDPUN;
  while(j > 0) {
    if((s = r->lsu[--j]) != 0) {
      break;
    }
  }
  for(n = 0; --j >= 0 && n < 2; n++) {
    s = (s * 1000) + r->lsu[j];
  }
  if(realIsNegative(r)) {
    s = -s;
  }
  e = r->exponent + (j + 1) * DECDPUN;
  if(e < -45) {
zero:
    return realIsPositive(r) ? 0. : -0.;
  }
  if(e > 38) {
infinite:
    if(realIsPositive(r)) {
      return 1. / 0.;
    }
    return -1. / 0.;
  }
  return (float)s * exps[e + 45];
}

void realToFloat(const real_t *vv, float *v) {
  *v = fnRealToFloat(vv);
}


// d * 10^e without libm. 10^0..10^22 are exact in double, so one multiply or divide rounds once; larger
// exponents step by 10^22 and saturate to 0 or +Inf on the way, as strtod of the same decimal does.
// The clamp is exact rather than defensive: d is under 10^19, so |e| past 400 saturates whatever d holds.
static double scaleByPowerOfTen(double d, int32_t e) {
  TO_QSPI static const double pow10[] = {
    1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,  1e10,
    1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22
  };
  bool_t divide;
  uint32_t k;

  if(e > 400) {
    e = 400;
  }
  else if(e < -400) {
    e = -400;
  }
  divide = (e < 0);
  k = (uint32_t)(divide ? -e : e);
  while(k > 22) {
    d = divide ? d / 1e22 : d * 1e22;
    k -= 22;
    if(d == 0.0 || d > DBL_MAX) {
      return d;
    }
  }
  return divide ? d / pow10[k] : d * pow10[k];
}


#if defined(PC_BUILD)
  // Locale-free parse for the simulator's backup file: accepts '.' or ',' regardless of the locale, so a file written under one region setting loads
  // under another. Buffer sized for a full 75-digit real string with exponent. The device parses no floating-point text, so this stays off DMCP.
  double stringToDouble(const char *str) {
    char buf[120];
    const char radix = *localeconv()->decimal_point;
    uint32_t i = 0;
    while(str[i] != 0 && i < sizeof(buf) - 1) {
      buf[i] = (str[i] == '.' || str[i] == ',') ? radix : str[i];
      i++;
    }
    buf[i] = 0;
    return strtod(buf, NULL);
  }
#endif // PC_BUILD


// real_t to double without the string round trip: coefficient units into a uint64_t, one conversion, then scaleByPowerOfTen(). Units are taken
// while the mantissa is below 10^16, so it ends under 10^19: every digit of a coefficient up to 19 digits, the top 17 to 19 of a longer one; e counts
// the units left below it. The unit walk is fnRealToFloat()'s. Specials and zero pass their sign through.
static double fnRealToDouble(const real_t *r) {
  const uint64_t unitCap = 10000000000000000ULL;   // 10^16
  uint64_t mant = 0;
  int32_t j, e;
  double d;

  if(realIsSpecial(r)) {
    if(realIsNaN(r)) {
      return 0.0 / 0.0;
    }
    return realIsPositive(r) ? 1.0 / 0.0 : -1.0 / 0.0;
  }
  if(realIsZero(r)) {
    return realIsPositive(r) ? 0.0 : -0.0;
  }
  j = (r->digits + DECDPUN - 1) / DECDPUN;
  while(j > 0) {
    if((mant = r->lsu[--j]) != 0) {
      break;
    }
  }
  while(--j >= 0 && mant < unitCap) {
    mant = mant * 1000 + r->lsu[j];
  }
  e = r->exponent + (j + 1) * DECDPUN;
  d = scaleByPowerOfTen((double)mant, e);
  return realIsNegative(r) ? -d : d;
}



void realToDouble(const real_t *vv, double *v) {
  *v = fnRealToDouble(vv);
}

static bool_t typeIsNumber(uint32_t type, bool_t *cmplx) {
  switch(type) {
    case dtComplex34:
      if(cmplx != NULL) {
        *cmplx = true;
      }
      return true;

    case dtLongInteger:
    case dtShortInteger:
    case dtReal34:
      if(cmplx != NULL) {
        *cmplx = false;
      }
      return true;
  }
  return false;
}

void badTypeError(calcRegister_t reg) {
  displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
#if (EXTRA_INFO_ON_CALC_ERROR == 1)
  sprintf(errorMessage, "cannot convert Register %d from %s", reg, getRegisterDataTypeName(reg, true, false));
  moreInfoOnError("In function badTypeError:", errorMessage, NULL, NULL);
#endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
}

void badDomainError(calcRegister_t reg) {
  displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
#if (EXTRA_INFO_ON_CALC_ERROR == 1)
  sprintf(errorMessage, "The input value is outside of the domain.");
  moreInfoOnError("In function badDomainError:", errorMessage, NULL, NULL);
#endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
}

void badTypeErrorX(void) {
  badTypeError(REGISTER_X);
}

void badDomainErrorX(void) {
  badDomainError(REGISTER_X);
}

bool_t getRegisterAsComplex(calcRegister_t reg, cplx_t *z) {
  switch(getRegisterDataType(reg)) {
    case dtLongInteger:
      convertLongIntegerRegisterToReal(reg, &z->Real, &ctxtReal75);
      break;

    case dtShortInteger:
      convertShortIntegerRegisterToReal(reg, &z->Real, &ctxtReal34);
      break;

    case dtReal34:
      real34ToReal(REGISTER_REAL34_DATA(reg), &z->Real);
      break;

    case dtComplex34:
      real34ToReal(REGISTER_REAL34_DATA(reg), &z->Real);
      real34ToReal(REGISTER_IMAG34_DATA(reg), &z->Imag);
      return true;

    default: {
      badTypeError(reg);
      return false;
    }
  }
  realSetZero(&z->Imag);
  return true;
}

bool_t getRegisterAsComplexOrAnyRealQuiet(calcRegister_t reg, cplx_t *z, bool_t *cmplx) {
  switch(getRegisterDataType(reg)) {
    case dtLongInteger:
      convertLongIntegerRegisterToReal(reg, &z->Real, &ctxtReal75);
      break;

    case dtShortInteger:
      convertShortIntegerRegisterToReal(reg, &z->Real, &ctxtReal34);
      break;

    case dtTime:
    case dtDate:
    case dtReal34:
      real34ToReal(REGISTER_REAL34_DATA(reg), &z->Real);
      break;

    case dtComplex34:
      real34ToReal(REGISTER_REAL34_DATA(reg), &z->Real);
      real34ToReal(REGISTER_IMAG34_DATA(reg), &z->Imag);
      if(cmplx != NULL) {
        *cmplx = true;
      }
      return true;

    default:
      return false;
  }
  realSetZero(&z->Imag);
  return true;
}

bool_t getRegisterAsComplexOrAnyReal(calcRegister_t reg, cplx_t *z, bool_t *cmplx) {
  const bool_t ret = getRegisterAsComplexOrAnyRealQuiet(reg, z, cmplx);

  if(!ret) {
    badTypeError(reg);
  }

  return ret;
}

bool_t getRegisterAsComplexOrRealQuiet(calcRegister_t reg, cplx_t *z, bool_t *cmplx) {
  const uint32_t t = getRegisterDataType(reg);

  if(t == dtTime || t == dtDate) {
    return false;
  }

  return getRegisterAsComplexOrAnyRealQuiet(reg, z, cmplx);
}

bool_t getRegisterAsComplexOrReal(calcRegister_t reg, cplx_t *z, bool_t *cmplx) {
  const bool_t ret = getRegisterAsComplexOrRealQuiet(reg, z, cmplx);

  if(!ret) {
    badTypeError(reg);
  }

  return ret;
}

bool_t getRegisterAsAnyRealQuiet(calcRegister_t reg, real_t *val) {
  switch(getRegisterDataType(reg)) {
    case dtLongInteger:
      convertLongIntegerRegisterToReal(reg, val, &ctxtReal75);
      break;

    case dtShortInteger:
      convertShortIntegerRegisterToReal(reg, val, &ctxtReal34);
      break;

    case dtDate:
    case dtTime:
    case dtReal34:
      real34ToReal(REGISTER_REAL34_DATA(reg), val);
      break;

    case dtComplex34:
      if(real34IsZero(REGISTER_IMAG34_DATA(reg))) {
        real34ToReal(REGISTER_REAL34_DATA(reg), val);
        break;
      }
    /* fall through */

    default:
      return false;
  }
  return true;
}

bool_t getRegisterAsRealQuiet(calcRegister_t reg, real_t *val) {
  uint32_t t = getRegisterDataType(reg);

  if(t == dtDate || t ==dtTime) {
    return false;
  }

  return getRegisterAsAnyRealQuiet(reg, val);
}

bool_t getRegisterAsReal(calcRegister_t reg, real_t *val) {
  bool_t res = getRegisterAsRealQuiet(reg, val);

  if(!res) {
    badTypeError(reg);
  }

  return res;
}

bool_t getRegisterAsAnyReal(calcRegister_t reg, real_t *val) {
  bool_t res = getRegisterAsAnyRealQuiet(reg, val);

  if(!res) {
    badTypeError(reg);
  }

  return res;
}

bool_t getRegisterAsReal34Quiet(calcRegister_t reg, real34_t *val) {
  real_t realVal;
  if(getRegisterAsRealQuiet(reg, &realVal)) {
    realToReal34(&realVal, val);
    return true;
  }
  return false;
}

bool_t getRegisterAsShortInt(calcRegister_t reg, bool_t *sign, uint64_t *val, bool_t *overflow, bool_t *fractional) {
  real_t rval;
  longInteger_t ival;
  uint64_t u64;
  int16_t sign16;
  bool_t of = false, frac = false;

  switch(getRegisterDataType(reg)) {
    case dtShortInteger:
      convertShortIntegerRegisterToUInt64(reg, &sign16, val);
      *sign = sign16 != 0;
      break;

    case dtLongInteger:
      convertLongIntegerRegisterToLongInteger(reg, ival);
    #if defined(OS32BIT) // 32 bit
      u64 = (ival->_mp_size == 0 ? 0 : *(uint32_t *)(ival->_mp_d));
      if(abs(ival->_mp_size) > 1) {
        u64 |= (int64_t)(*(((uint32_t *)(ival->_mp_d)) + 1)) << 32;
      }
      of = abs(ival->_mp_size) > 2 || (u64 & shortIntegerMask) != u64;
      u64 &= shortIntegerMask;
    #else // 64 bit
      u64 = (ival->_mp_size == 0 ? 0 : *(uint64_t *)(ival->_mp_d)) & shortIntegerMask;
      of = abs(ival->_mp_size) > 1 || (u64 & shortIntegerMask) != u64;
    #endif // OS32BIT
      *sign = longIntegerIsNegative(ival);
      longIntegerFree(ival);
      *val = u64;
      break;

    case dtComplex34:
    case dtReal34:
      if(getRegisterAsReal(reg, &rval)) {
        if(realIsSpecial(&rval)) {
          badDomainError(reg);
          return false;
        }
        *sign = realIsNegative(&rval);
        realSetPositiveSign(&rval);
        frac = !realIsAnInteger(&rval);

        //u64 = realToUint64C47(&rval);
        real_t i;
        realToIntegralValue(&rval, &i, DEC_ROUND_DOWN, &ctxtReal39); // After this call, it's guaranteed that i is an integer and i->exponent >= 0
        decNumberQuantize(&i, &i, const_0, &ctxtReal39); // After this call, it's guaranteed that the value of i is not changed and i->exponent == 0 (const_0 is used only because it's exponent is 0)
        u64 = decNumberToUInt64(&i, &ctxtReal39);


        of = (u64 & shortIntegerMask) != u64;
        if(!of) {
          switch(shortIntegerMode) {
            case SIM_UNSIGN:
              of = realCompareGreaterEqual(&rval, const_2p64);
              break;
            case SIM_2COMPL:
              if(*sign) {
                of = realCompareGreaterThan(&rval, const_2p63);
              }
              else
                /* fall through */
            case SIM_1COMPL:
            case SIM_SIGNMT:
              of = realCompareGreaterEqual(&rval, const_2p63);
              break;
          }
        }
        *val = u64;
        break;
      }
      /* fall through */

    default:
      badTypeError(reg);
      return false;
  }

  if(overflow != NULL) {
    *overflow = of;
  }

  if(fractional != NULL) {
    *fractional = frac;
  }

  return true;
}

bool_t getRegisterAsRawShortInt(calcRegister_t reg, uint64_t *val, uint32_t *base) {
  bool_t sign, overflow, fractional;
  uint64_t v;
  uint32_t b;

  if(getRegisterDataType(reg) == dtShortInteger) {
    v = *REGISTER_SHORT_INTEGER_DATA(reg);
    b = getRegisterShortIntegerBase(reg);
    goto finish;
  }
  if(!getRegisterAsShortInt(reg, &sign, &v, &overflow, &fractional)) {
    return false;
  }
  if(overflow || fractional) {
    badDomainError(reg);
    return false;
  }
  v = (uint64_t)WP34S_build_value(v, sign);
  b = lastIntegerBase != 0 ? lastIntegerBase : 10;
finish:
  if(base != NULL) {
    *base = b;
  }
  *val = v;
  return true;
}

int getRegisterAsLongIntQuiet(calcRegister_t reg, longInteger_t val, bool_t *fractional) {
  real_t rval;
  bool_t frac = false;

  switch(getRegisterDataType(reg)) {
    case dtLongInteger:
      convertLongIntegerRegisterToLongInteger(reg, val);
      break;

    case dtShortInteger:
      convertShortIntegerRegisterToLongInteger(reg, val);
      break;

    case dtComplex34:
    case dtReal34:
      if(getRegisterAsReal(reg, &rval)) {
        longIntegerInit(val); // convertRealToLongInteger expects an initialised val
        if(realIsSpecial(&rval)) {
          return ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN;
        }
        if(!realIsAnInteger(&rval)) {
          realToIntegralValue(&rval, &rval, DEC_ROUND_DOWN, &ctxtReal39);
          frac = true;
        }
        convertRealToLongInteger(&rval, val, DEC_ROUND_DOWN);
        break;
      }
      /* fall through */

    default:
      longIntegerInit(val);
      return ERROR_INVALID_DATA_TYPE_FOR_OP;
  }
  if(fractional != NULL) {
    *fractional = frac;
  }

  return ERROR_NONE;
}

bool_t getRegisterAsLongInt(calcRegister_t reg, longInteger_t val, bool_t *fractional) {
  const int err = getRegisterAsLongIntQuiet(reg, val, fractional);

  if(err != ERROR_NONE) {
    displayCalcErrorMessage(err, ERR_REGISTER_LINE);
  }

  return err == ERROR_NONE;
}

bool_t getRegisterAsUint32Param(uint16_t regist, uint32_t *value) {   // For setters' non-negative long integer, returned as uint32
  longInteger_t lgInt;
  bool_t ok = getRegisterAsLongInt(regist, lgInt, NULL);
  if(ok) {
    ok = longIntegerCompareInt(lgInt, 0) >= 0 && longIntegerCompareUInt(lgInt, UINT32_MAX) <= 0;
    if(ok) {
      longIntegerToUInt32(lgInt, *value);
    }
  }
  longIntegerFree(lgInt);
  return ok;
}

bool_t getRegisterAsInt32Param(uint16_t regist, int32_t *value) {     // signed variant of getRegisterAsUint32Param
  longInteger_t lgInt;
  bool_t ok = getRegisterAsLongInt(regist, lgInt, NULL);
  if(ok) {
    ok = longIntegerCompareInt(lgInt, INT32_MIN) >= 0 && longIntegerCompareInt(lgInt, INT32_MAX) <= 0;
    if(ok) {
      longIntegerToInt32(lgInt, *value);
    }
  }
  longIntegerFree(lgInt);
  return ok;
}

// Integer radian reduction: one width for the context, the buffers and the 2pi divisor, so the divisor cannot drift from the context; 2139 covers the 1000 digit input cap.
#define LONGINT_RADIAN_REDUCTION_DIGITS 2139
#define LONGINT_RADIAN_REDUCTION_2PI    const2139_2pi

bool_t reduceLongIntegerRegisterAngle(calcRegister_t regist, angularMode_t angularMode, real_t *reducedAngle, bool_t reduceLongintegerAngle) {
  uint32_t oneTurn;
  longInteger_t angle;

  if(reduceLongintegerAngle) {
    switch(angularMode) {
      case amMultPi: {
        oneTurn = 2;
        break;
      }
      case amGrad: {
        oneTurn = 400;
        break;
      }
      case amDegree:
      case amDMS: {
        oneTurn = 360;
        break;
      }
      case amRadian: {
        //incoming longInteger, converted via tempString to real6147, modulus 2pi into real6147, convert to real75
        // The two 2139 digit buffers are 1436 bytes each, 2872 of this function's 2936 byte frame. From the heap the frame falls to 64 bytes.
        REAL_T_ALLOC(reducedAngleTmp,  LONGINT_RADIAN_REDUCTION_DIGITS); // This cannot be increased to 6147 further. 6147 overruns the stack even if we just have the type in here also when using 2139 digits below.
        REAL_T_ALLOC(reducedAngleTmp2, LONGINT_RADIAN_REDUCTION_DIGITS);
        if(reducedAngleTmp == NULL || reducedAngleTmp2 == NULL) {
          REAL_T_FREE(reducedAngleTmp,  LONGINT_RADIAN_REDUCTION_DIGITS);
          REAL_T_FREE(reducedAngleTmp2, LONGINT_RADIAN_REDUCTION_DIGITS);
          displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
          return false;
        }
        realContext_t c = ctxtReal75;
        c.digits = LONGINT_RADIAN_REDUCTION_DIGITS;    // Cannot be increased further. It works well on 1071, worked for a few tests already on 2139 but crashes if this goes to 6147 (together with the real_xxx above)
                                                       // The minimum required for 1000 digits input reduction is slightly less than double, so 1071 is maybe ok for 99.99% cases, but 2139 is preferred as theoretically you will not have a case where 2139 will not work.
        convertLongIntegerRegisterToLongInteger(regist, angle);

        if(longIntegerBase10Digits(angle) > 1001) {
          displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
          #if (EXTRA_INFO_ON_CALC_ERROR == 1)
            moreInfoOnError("In function reduceLongIntegerRegisterAngle:", "Invalid integer size for angle reduction in radians: exponent too large.", NULL, NULL);
          #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
          longIntegerFree(angle);
          REAL_T_FREE(reducedAngleTmp,  LONGINT_RADIAN_REDUCTION_DIGITS);
          REAL_T_FREE(reducedAngleTmp2, LONGINT_RADIAN_REDUCTION_DIGITS);
          return false;
        }

        longIntegerToString(angle, 10, tmpString);
        decNumberFromString(reducedAngleTmp, tmpString, &c);
        WP34S_Mod(reducedAngleTmp, LONGINT_RADIAN_REDUCTION_2PI, reducedAngleTmp2, &c);  // the 2139-digit 2pi reduces at the 2139 context; the 12321-digit 2pi NaNs on the DM42 decNumber here
        realPlus(reducedAngleTmp2, reducedAngle, &ctxtReal75);
        longIntegerFree(angle);
        REAL_T_FREE(reducedAngleTmp,  LONGINT_RADIAN_REDUCTION_DIGITS);
        REAL_T_FREE(reducedAngleTmp2, LONGINT_RADIAN_REDUCTION_DIGITS);
        return true;
      }
      default: { //amNone
        convertLongIntegerRegisterToReal(regist, reducedAngle, &ctxtReal75);
        return true;
      }
    }
  }

  if(reduceLongintegerAngle) {
    convertLongIntegerRegisterToLongInteger(regist, angle);
    uInt32ToReal(longIntegerModuloUInt(angle, oneTurn), reducedAngle);   // Reduction for DEG, GRAD, MULPI branch
    longIntegerFree(angle);
  } else {
    convertLongIntegerRegisterToReal(regist, reducedAngle, &ctxtReal39);  // No reduction for DEG, GRAD, MULPI branch
  }
  return true;
}



bool_t getRegisterAsRealAngle(calcRegister_t reg, real_t *val, angularMode_t *xAngularMode, bool_t reduceLongintegerAngle) {
  switch(getRegisterDataType(reg)) {
    case dtLongInteger:
      if(!reduceLongIntegerRegisterAngle(reg, currentAngularMode, val, reduceLongintegerAngle)) {
        return false;
      }
      // out of range error rolled into reduceLongIntegerRegisterAngle as the longintegr is not accessible here except for converting again
      *xAngularMode = currentAngularMode;
      break;

    case dtShortInteger:
      convertShortIntegerRegisterToReal(reg, val, &ctxtReal34);
      *xAngularMode = currentAngularMode;
      break;

    case dtReal34:
      real34ToReal(REGISTER_REAL34_DATA(reg), val);
      *xAngularMode = getRegisterAngularMode(reg);
      if(*xAngularMode == amNone) {
        *xAngularMode = currentAngularMode;
      }
      if(*xAngularMode == amRadian && realGetExponent(val) > 999) {
        displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function getRegisterAsRealAngle:", "Invalid real input size for angle reduction in radians: exponent too large.", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return false;
      }
      break;

    case dtComplex34:
      if(real34IsZero(REGISTER_IMAG34_DATA(reg))) {
        real34ToReal(REGISTER_REAL34_DATA(reg), val);
        *xAngularMode = amRadian;
        break;
      }
    /* fall through */

    default:
      badTypeError(reg);
      return false;
  }
  return true;
}

void saveRegisterSnapshot(calcRegister_t reg, snap_t *s) {
  s->mem = NULL;
  s->blocks = 0;
  switch(s->t = getRegisterDataType(reg)) {
    case dtComplex34:
      real34Copy(REGISTER_REAL34_DATA(reg), &s->r);
      real34Copy(REGISTER_IMAG34_DATA(reg), &s->i);
      break;
    case dtReal34:
    case dtTime:
    case dtDate:
      real34Copy(REGISTER_REAL34_DATA(reg), &s->r);
      real34SetZero(&s->i);
      break;
    case dtLongInteger:
      getRegisterAsLongInt(reg, s->li, NULL);
      break;
    case dtShortInteger:
      getRegisterAsRawShortInt(reg, &s->siVal, &s->siBase);
      break;
    default: // a string, a matrix of either kind and a configuration are a block of data of their own size, so the blocks are copied whole
      s->blocks = getRegisterFullSizeInBlocks(reg);
      s->mem = allocC47Blocks(s->blocks);
      if(s->mem) {
        xcopy(s->mem, getRegisterDataPointer(reg), TO_BYTES(s->blocks));
      }
      else {
        s->blocks = 0;
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE); // the value is not captured, so the restore would put back nothing
      }
      break;
  }
  s->tag = getRegisterTag(reg);
}

// reg may hold a different type (hence allocation size) than the snapshot, so reallocate to s->t before writing back
void restoreRegisterSnapshot(calcRegister_t reg, snap_t *s) {
  switch(s->t) {
    case dtComplex34:
      reallocateRegister(reg, dtComplex34, COMPLEX34_SIZE_IN_BLOCKS, s->tag);
      real34Copy(&s->i, REGISTER_IMAG34_DATA(reg));
      real34Copy(&s->r, REGISTER_REAL34_DATA(reg));
      break;
    case dtReal34:
    case dtTime:
    case dtDate:
      reallocateRegister(reg, s->t, REAL34_SIZE_IN_BLOCKS, s->tag);
      real34Copy(&s->r, REGISTER_REAL34_DATA(reg));
      break;
    case dtLongInteger:
      convertLongIntegerToLongIntegerRegister(s->li, reg);
      longIntegerFree(s->li);
      break;
    case dtShortInteger:
      reallocateRegister(reg, dtShortInteger, SHORT_INTEGER_SIZE_IN_BLOCKS, s->siBase);
      *(REGISTER_SHORT_INTEGER_DATA(reg))=s->siVal;
      setRegisterShortIntegerBase(reg, s->siBase);
      break;
    default: // the blocks the save took a copy of go back as they were, and the register's own blocks are freed whatever it holds now
      if(s->mem) {
        void *dataPtr = allocC47Blocks(s->blocks); // taken before the register's own blocks go, so a refusal leaves the register with the value it has now
        if(dataPtr) {
          freeRegisterData(reg);
          setRegisterDataPointer(reg, dataPtr);
          xcopy(dataPtr, s->mem, TO_BYTES(s->blocks));
        }
        freeC47Blocks(s->mem, s->blocks);
        s->mem = NULL;
        if(!dataPtr) {
          displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
          return; // the register keeps the type and the value it has now
        }
      }
      break;
  }
  setRegisterDataType(reg, s->t, s->tag);
}

// Write a snapshot back and leave it usable, for a loop that puts the same operand back at every element. restoreRegisterSnapshot frees what it owns, so this
// takes a fresh copy of the register straight after writing it back.
void applyRegisterSnapshot(calcRegister_t reg, snap_t *s) {
  restoreRegisterSnapshot(reg, s);
  saveRegisterSnapshot(reg, s);
}

// Release what a snapshot owns without writing it back. A loop driven by applyRegisterSnapshot ends with this, so the result register is left alone.
void freeRegisterSnapshot(snap_t *s) {
  if(s->t == dtLongInteger) {
    longIntegerFree(s->li);
  }
  else if(s->mem) {
    freeC47Blocks(s->mem, s->blocks);
    s->mem = NULL;
    s->blocks = 0;
  }
}


void processRealComplexMonadicFunction(void (*realf)(void), void (*complexf)(void)) {
  processIntRealComplexMonadicFunction(realf, complexf, NULL, NULL);
}

void processIntRealComplexMonadicFunction(void (*realf)(void), void (*complexf)(void), void (*shortintf)(void), void (*longintf)(void)) {
  cplx_t a;
  bool_t cmplxRes = false;
  const uint32_t type = getRegisterDataType(REGISTER_X);

  if(lastFunc != ITM_CHS && !saveLastX()) {              // Compiler witll short circuit and not run saveLastX if the related function is CHS
    return;
  }

  if(type == dtShortInteger) {
    if(shortintf != NULL) {
      shortintf();
      goto done;
    }
    if(longintf != NULL) {
      longintf();
      goto done;
    }
  }

  if(type == dtLongInteger && longintf != NULL) {
    longintf();
  }
  else if(type == dtReal34Matrix && realf != NULL) {
    elementwiseRema(realf);
  }
  else if(type == dtComplex34Matrix && complexf != NULL) {
    elementwiseCxma(complexf);
  }
  else if(getRegisterAsComplexOrReal(REGISTER_X, &a, &cmplxRes)) {
    if(!cmplxRes && realf != NULL) {
      realf();
    }
    else if(complexf != NULL) {
      complexf();
    }
    else {
      badTypeErrorX();
    }
  }

done:
  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}

void processRealComplexDyadicFunction(void (*realf)(void), void (*complexf)(void)) {
  cplx_t x, y;
  const uint32_t typeX = getRegisterDataType(REGISTER_X);
  const uint32_t typeY = getRegisterDataType(REGISTER_Y);
  bool_t xCmplx = false, yCmplx = false, cmplxRes = false;
  const bool_t xNumber = typeIsNumber(typeX, &xCmplx);
  const bool_t yNumber = typeIsNumber(typeY, &yCmplx);

  if(!saveLastX()) {
    return;
  }

  if(typeX == dtReal34Matrix && realf != NULL) {
    if(typeY == dtReal34Matrix) {
      elementwiseRemaRema(realf);
      goto fin;
    }
    else if(typeY == dtComplex34Matrix) {
      if(complexf != NULL) {
        elementwiseCxmaRema(complexf);
      }
      else {
        badTypeError(REGISTER_Y);
      }
      goto fin;
    }
    else if(yNumber) {
      if(yCmplx) {
        if(complexf != NULL) {
          elementwiseCplxRema(complexf);
        }
        else {
          badTypeError(REGISTER_Y);
        }
      }
      else {
        elementwiseRealRema(realf);
      }
      goto fin;
    }
  }
  else if(typeX == dtComplex34Matrix && complexf != NULL) {
    if(typeY == dtReal34Matrix) {
      elementwiseRemaCxma(complexf);
      goto fin;
    }
    else if(typeY == dtComplex34Matrix) {
      elementwiseCxmaCxma(complexf);
      goto fin;
    }
    else if(yNumber) {
      elementwiseCplxCxma(complexf);
      goto fin;
    }
  }
  else if(typeY == dtReal34Matrix && xNumber) {
    if(!xCmplx && realf != NULL) {
      elementwiseRemaReal(realf);
    }
    else if(complexf != NULL) {
      elementwiseRemaCplx(complexf);
    }
    else {
      badTypeErrorX();
    }
    goto fin;
  }
  else if(typeY == dtComplex34Matrix && xNumber) {
    if(complexf != NULL) {
      elementwiseCxmaCplx(complexf);
    }
    else {
      badTypeError(REGISTER_Y);
    }
    goto fin;
  }

  /* Fall through to numeric/numeric or error if not */
  if(!getRegisterAsComplexOrReal(REGISTER_X, &x, &cmplxRes) || !getRegisterAsComplexOrReal(REGISTER_Y, &y, &cmplxRes)) {
    return;
  }

  if(!cmplxRes && realf != NULL) {
    realf();
  }
  else if(complexf != NULL) {
    complexf();
  }
  else {
    badTypeError(xCmplx ? REGISTER_X : REGISTER_Y);
  }

fin:
  adjustResult(REGISTER_X, true, true, REGISTER_X, REGISTER_Y, -1);
}

void processIntRealComplexDyadicFunction(void (*realf)(void), void (*complexf)(void), void (*shortintf)(void), void (*longintf)(void)) {
  const uint32_t typeX = getRegisterDataType(REGISTER_X);
  const uint32_t typeY = getRegisterDataType(REGISTER_Y);
  const bool_t xInt = typeX == dtLongInteger || typeX == dtShortInteger;
  const bool_t yInt = typeY == dtLongInteger || typeY == dtShortInteger;

  if(typeX == dtShortInteger && typeY == dtShortInteger && shortintf != NULL) {
    if(saveLastX()) {
      shortintf();
    }
  }
  else if(xInt && yInt && longintf != NULL) {
    if(saveLastX()) {
      longintf();
    }
  }
  else {
    processRealComplexDyadicFunction(realf, complexf);
    return;
  }
  adjustResult(REGISTER_X, true, true, REGISTER_X, REGISTER_Y, -1);
}
