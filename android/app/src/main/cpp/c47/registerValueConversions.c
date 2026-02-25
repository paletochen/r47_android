// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

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

  if(getRegisterLongIntegerSign(regist) == LI_NEGATIVE) {
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
    if(overflow && !getSystemFlag(FLAG_OVERFLOW))
      setSystemFlag(FLAG_OVERFLOW);
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
  uint8_t bcd[DECQUAD_Pmax];
  int32_t sign, exponent;
  real34_t real34;

  real34ToIntegralValue(re34, &real34, roundingMode);
  sign = real34GetCoefficient(&real34, bcd);
  exponent = real34GetExponent(&real34);

  longIntegerInit(lgInt);
  uInt32ToLongInteger(bcd[0], lgInt);

  for(int i=1; i<DECQUAD_Pmax; i++) {
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



void convertReal34ToLongIntegerRegister(const real34_t *real34, calcRegister_t dest, enum rounding roundingMode) {
  longInteger_t lgInt;

  convertReal34ToLongInteger(real34, lgInt, roundingMode);
  convertLongIntegerToLongIntegerRegister(lgInt, dest);

  longIntegerFree(lgInt);
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



void realToIntegralValue(const real_t *source, real_t *destination, enum rounding mode, realContext_t *realContext) {
  enum rounding savedRoundingMode;

  savedRoundingMode = realContext->round;
  realContext->round = mode;
  realContext->status = 0;
  decNumberToIntegralValue(destination, source, realContext);
  realContext->round = savedRoundingMode;
}



void convertRealToReal34ResultRegister(const real_t *real, calcRegister_t dest) {
  real_t rounded;
  roundToSignificantDigits(real, &rounded, significantDigits == 0 ? 34 : significantDigits, &ctxtReal75);
  realToReal34(&rounded, REGISTER_REAL34_DATA(dest));
}

void convertRealToImag34ResultRegister(const real_t *real, calcRegister_t dest) {
  real_t rounded;
  roundToSignificantDigits(real, &rounded, significantDigits == 0 ? 34 : significantDigits, &ctxtReal75);
  realToReal34(&rounded, REGISTER_IMAG34_DATA(dest));
}

void convertRealToResultRegister(const real_t *x, calcRegister_t dest, angularMode_t angle) {
  reallocateRegister(dest, dtReal34, REAL34_SIZE_IN_BLOCKS, angle);
  convertRealToReal34ResultRegister(x, dest);
}

void convertComplexToResultRegister(const real_t *real, const real_t *imag, calcRegister_t dest) {
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
  real34_t real34, value34;
  real34Copy(REGISTER_REAL34_DATA(source), &real34);
  int32ToReal34(3600, &value34);
  reallocateRegister(destination, dtReal34, REAL34_SIZE_IN_BLOCKS, amNone);
  real34Divide(&real34, &value34, REGISTER_REAL34_DATA(destination));
}



void convertReal34RegisterToTimeRegister(calcRegister_t source, calcRegister_t destination) {
  real34_t real34, value34;
  real34Copy(REGISTER_REAL34_DATA(source), &real34);
  int32ToReal34(3600, &value34);
  reallocateRegister(destination, dtTime, REAL34_SIZE_IN_BLOCKS, amNone);
  real34Multiply(&real34, &value34, REGISTER_REAL34_DATA(destination));
}



void convertLongIntegerRegisterToTimeRegister(calcRegister_t source, calcRegister_t destination) {
  convertLongIntegerRegisterToReal34Register(source, destination);
  convertReal34RegisterToTimeRegister(destination, destination);
}



void convertDateRegisterToReal34Register(calcRegister_t source, calcRegister_t destination) {
  real34_t y, m, d, j, val;
  bool_t isNegative;

  internalDateToJulianDay(REGISTER_REAL34_DATA(source), &j);
  decomposeJulianDay(&j, &y, &m, &d);
  isNegative = real34IsNegative(&y);
  real34SetPositiveSign(&y);

  if(getSystemFlag(FLAG_YMD)) {
    int32ToReal34(100, &val), real34Divide(&m, &val, &m);
    int32ToReal34(10000, &val), real34Divide(&d, &val, &d);
  }
  else if(getSystemFlag(FLAG_MDY)) {
    int32ToReal34(100, &val), real34Divide(&d, &val, &d);
    int32ToReal34(1000000, &val), real34Divide(&y, &val, &y);
  }
  else if(getSystemFlag(FLAG_DMY)) {
    int32ToReal34(100, &val), real34Divide(&m, &val, &m);
    int32ToReal34(1000000, &val), real34Divide(&y, &val, &y);
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
  real34ToIntegralValue(&part2, &part1, DEC_ROUND_DOWN);            //Y D or M
  real34Subtract(&part2, &part1, &part2);
  int32ToReal34(100, &val), real34Multiply(&part2, &val, &part2);

  real34Copy(&part2, &part3);
  real34ToIntegralValue(&part2, &part2, DEC_ROUND_DOWN);            //M M or D
  real34Subtract(&part3, &part2, &part3);
  int32ToReal34(getSystemFlag(FLAG_YMD) ? 100 : 10000, &val), real34Multiply(&part3, &val, &part3);  // dd yyyy or yyyy
  real34ToIntegralValue(&part3, &part3, DEC_ROUND_DOWN);

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
    lastCenturyHighUsedtmp = lastCenturyHighUsed & 0x3FFF;

    // remember last used century if the century is not an abbreviation, i.e. if YYYY > 100, ignore neg value YYYY
    if(getSystemFlag(FLAG_YMD)) {
      if(real34CompareGreaterEqual(&part1,const34_100)) {
        lastCenturyHighUsedtmp = 100*(int16_t)(real34ToInt32(&part1) / 100) + 99;
      }
    }
    else //FLAG_MDY //FLAG_DMY
    if(real34CompareGreaterEqual(&part3,const34_100)) {
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
            uInt32ToReal34(dateInfo.year,&part1);
          #elif defined(PC_BUILD) // PC_BUILD
            uInt32ToReal34(timeInfo->tm_year + 1900,&part1);
          #endif // PC_BUILD
        }
      }
      //FLAG_MDY //FLAG_DMY
      else if(real34IsZero(&part3)) {
        #if defined(DMCP_BUILD)
          uInt32ToReal34(dateInfo.year,&part3);
        #elif defined(PC_BUILD) // PC_BUILD
          uInt32ToReal34(timeInfo->tm_year + 1900,&part3);
        #endif // PC_BUILD
      }
    }



    //Only YY digits
    // thresholdYYHigh = 1950 :     Automatic, 2024 ==> 2000-2099. 29 ==> 2029, 59 ==> 2059
    //                              1950-2049. 29 ==> 2029, 59 ==> 1959
    //                              if yy > 49, then yy += 1900 else yy += 2000
    int16_t thresholdYYHigh = max(0, (int16_t)(lastCenturyHighUsed & 0x3FFF) - 99);
    if(getSystemFlag(FLAG_YMD)) {
      if(!(lastCenturyHighUsed & 0x8000) && real34CompareLessThan(&part1,const34_100)) {
        int16_t yy = (int16_t)(real34ToInt32(&part1));
        if(yy >= (thresholdYYHigh) % 100) {
          yy += (thresholdYYHigh - thresholdYYHigh % 100);
        }
        else {
          yy += (thresholdYYHigh - thresholdYYHigh % 100) + 100;
        }
        int32ToReal34((int32_t)yy,&part1);
      }
    }
    //FLAG_MDY //FLAG_DMY
    else if(!(lastCenturyHighUsed & 0x8000) && real34CompareLessThan(&part3,const34_100)) {
      int16_t yy = (int16_t)(real34ToInt32(&part3));
      if(yy >= (thresholdYYHigh) % 100) {
        yy += (thresholdYYHigh - thresholdYYHigh % 100);
      }
      else {
        yy += (thresholdYYHigh - thresholdYYHigh % 100) + 100;
      }
      int32ToReal34((int32_t)yy,&part3);
    }
  }



  if((getSystemFlag(FLAG_YMD) && !isValidDay(&part1, &part2, &part3)) ||
    ( getSystemFlag(FLAG_MDY) && !isValidDay(&part3, &part1, &part2)) ||
    ( getSystemFlag(FLAG_DMY) && !isValidDay(&part3, &part2, &part1))) {
      displayCalcErrorMessage(ERROR_BAD_TIME_OR_DATE_INPUT, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function convertReal34RegisterToDateRegister:", "Invalid date input like 30 Feb.", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
  }


  //update stored YYYY and add the control bits again
  if(handleYY && !(lastCenturyHighUsed & 0x8000) && followYY()) {
    lastCenturyHighUsed = (lastCenturyHighUsed & ~0x3FFF) | (lastCenturyHighUsedtmp & 0x3FFF);
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

  int32ToReal34(86400, &val), real34Multiply(REGISTER_REAL34_DATA(destination), &val, REGISTER_REAL34_DATA(destination));
  int32ToReal34(43200, &val), real34Add(REGISTER_REAL34_DATA(destination), &val, REGISTER_REAL34_DATA(destination));
}



void convertReal34MatrixRegisterToReal34Matrix(calcRegister_t regist, real34Matrix_t *matrix) {
  matrixHeader_t *matrixHeader = REGISTER_MATRIX_HEADER(regist);

  if(realMatrixInit(matrix, matrixHeader->matrixRows, matrixHeader->matrixColumns)) {
    if(matrix->matrixElements) {
      xcopy(matrix->matrixElements, REGISTER_REAL34_MATRIX_ELEMENTS(regist), (matrix->header.matrixColumns * matrix->header.matrixRows) * REAL34_SIZE_IN_BYTES);
    }
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
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
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
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
        real34Zero(VARIABLE_IMAG34_DATA(&destination->matrixElements[i]));
      }
    }
  }
  else {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
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


void sci_fmt(char *buf, int n, double x) {
/*
 * Usage:
 *   char buf[32];
 *   sci_fmt(buf, sizeof(buf), x);  // replaces snprintf(buf, sizeof(buf), "%.16e", x);
 *
 * Output format (if buffer allows):
 *   [-]d.dddddddddddddddde±dd\0 (up to 25–30 bytes depending on exponent digits)
 */
    int exp = 0, i = 0;
    if (x < 0) {
        buf[i++] = '-';
        x = -x;
    }

    while (x && x < 1.0) x *= 10.0, exp--;
    while (x >= 10.0) x /= 10.0, exp++;

    unsigned long long m = (unsigned long long)(x * 1e15 + 0.5);
    if (m >= 10000000000000000ULL) {
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

    for (int j = 1; j < 15 && i < n - 6; j++) {
        buf[i++] = '0' + (m / divs[j]) % 10;
    }

    i += snprintf(buf + i, n - i, "e%+03d", exp);
    buf[i] = 0;
}



#if !defined(TESTSUITE_BUILD)
  void convertDoubleToString(double x, int16_t n, char *buff) { //Reformatting real strings that are formatted according to different locale settings
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
        strcpy(buff,"NaN");
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
        printf("ERROR in convertDoubleToReal34Register: %s\n",buff);
      }
    #endif // PC_BUILD
  }


  void convertDoubleToReal34RegisterPush(double x, calcRegister_t destination) {
    setSystemFlag(FLAG_ASLIFT);
    liftStack();
    convertDoubleToReal34Register(x, destination);
    setSystemFlag(FLAG_ASLIFT);
  }



#endif // !TESTSUITE_BUILD



double convertRegisterToDouble(calcRegister_t regist) {
  real_t regReal, regImag;

  return getRegisterAsComplex(regist, &regReal, &regImag) ? fnRealToFloat(&regReal) : DOUBLE_NOT_INIT;
}


//Pauli volunteered this fuction, rev 1 2021-10-10
#if DECDPUN != 3
  #error DECDPUN must be 3
#endif

static float fnRealToFloat(const real_t *r){
  int s = 0;
  int j, n, e;

  static const float exps[] = {
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

//#define realToReal39(source, destination) decQuadFromNumber ((real39_t *)(destination), source, &ctxtReal39)

void realToFloat(const real_t *vv, float *v) {
  *v = fnRealToFloat(vv);
}


static double fnRealToDouble(const real_t *r) {
    char buffer[100];
        if (realIsSpecial(r)) {
        if (realIsNaN(r)) return 0.0 / 0.0;
        return realIsPositive(r) ? 1.0 / 0.0 : -1.0 / 0.0;
    }
    if (realIsZero(r)) {
        return realIsPositive(r) ? 0.0 : -0.0;
    }
    decNumberToString((decNumber*)r, buffer);
    return strtod(buffer, NULL);
}



void realToDouble(const real_t *vv, double *v) {
  *v = fnRealToDouble(vv);
}

static bool_t typeIsNumber(uint32_t type, bool_t *cmplx) {
  switch(type) {
    case dtComplex34:
      if(cmplx != NULL)
          *cmplx = true;
      return true;

    case dtLongInteger:
    case dtShortInteger:
    case dtReal34:
      if(cmplx != NULL)
          *cmplx = false;
      return true;
  }
  return false;
}

void badTypeError(calcRegister_t reg) {
  displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_T);
#if (EXTRA_INFO_ON_CALC_ERROR == 1)
  sprintf(errorMessage, "cannot convert Register %d from %s", reg, getRegisterDataTypeName(reg, true, false));
  moreInfoOnError("In function badTypeError:", errorMessage, NULL, NULL);
#endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
}

void badDomainError(calcRegister_t reg) {
  displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_T);
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

bool_t getRegisterAsComplex(calcRegister_t reg, real_t *r, real_t *i) {
  switch(getRegisterDataType(reg)) {
    case dtLongInteger:
      convertLongIntegerRegisterToReal(reg, r, &ctxtReal75);
      break;

    case dtShortInteger:
      convertShortIntegerRegisterToReal(reg, r, &ctxtReal34);
      break;

    case dtReal34:
      real34ToReal(REGISTER_REAL34_DATA(reg), r);
      break;

    case dtComplex34:
      real34ToReal(REGISTER_REAL34_DATA(reg), r);
      real34ToReal(REGISTER_IMAG34_DATA(reg), i);
      return true;

    default: {
      badTypeError(reg);
      return false;
    }
  }
  realZero(i);
  return true;
}

bool_t getRegisterAsComplexOrAnyRealQuiet(calcRegister_t reg, real_t *r, real_t *i, bool_t *cmplx) {
  switch(getRegisterDataType(reg)) {
    case dtLongInteger:
      convertLongIntegerRegisterToReal(reg, r, &ctxtReal75);
      break;

    case dtShortInteger:
      convertShortIntegerRegisterToReal(reg, r, &ctxtReal34);
      break;

    case dtTime:
    case dtDate:
    case dtReal34:
      real34ToReal(REGISTER_REAL34_DATA(reg), r);
      break;

    case dtComplex34:
      real34ToReal(REGISTER_REAL34_DATA(reg), r);
      real34ToReal(REGISTER_IMAG34_DATA(reg), i);
      if(cmplx != NULL)
        *cmplx = true;
      return true;

    default:
      return false;
  }
  realZero(i);
  return true;
}

bool_t getRegisterAsComplexOrAnyReal(calcRegister_t reg, real_t *r, real_t *i, bool_t *cmplx) {
  const bool_t ret = getRegisterAsComplexOrAnyRealQuiet(reg, r, i, cmplx);

  if (!ret)
    badTypeError(reg);
  return ret;
}

bool_t getRegisterAsComplexOrRealQuiet(calcRegister_t reg, real_t *r, real_t *i, bool_t *cmplx) {
  const uint32_t t = getRegisterDataType(reg);

  if (t == dtTime || t == dtDate)
    return false;
  return getRegisterAsComplexOrAnyRealQuiet(reg, r, i, cmplx);
}

bool_t getRegisterAsComplexOrReal(calcRegister_t reg, real_t *r, real_t *i, bool_t *cmplx) {
  const bool_t ret = getRegisterAsComplexOrRealQuiet(reg, r, i, cmplx);

  if (!ret)
    badTypeError(reg);
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

  if (t == dtDate || t ==dtTime)
    return false;
  return getRegisterAsAnyRealQuiet(reg, val);
}

bool_t getRegisterAsReal(calcRegister_t reg, real_t *val) {
  bool_t res = getRegisterAsRealQuiet(reg, val);

  if(!res)
    badTypeError(reg);
  return res;
}

bool_t getRegisterAsAnyReal(calcRegister_t reg, real_t *val) {
  bool_t res = getRegisterAsAnyRealQuiet(reg, val);

  if(!res)
    badTypeError(reg);
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
      u64 = *(uint32_t *)(ival->_mp_d);
      if(abs(ival->_mp_size) > 1) {
        u64 |= (int64_t)(*(((uint32_t *)(ival->_mp_d)) + 1)) << 32;
      }
      of = abs(ival->_mp_size) > 2 || (u64 & shortIntegerMask) != u64;
      u64 &= shortIntegerMask;
    #else // 64 bit
      u64 = *(uint64_t *)(ival->_mp_d) & shortIntegerMask;
      of = abs(ival->_mp_size) > 1 || (u64 & shortIntegerMask) != u64;
    #endif // OS32BIT
      *sign = longIntegerIsNegative(ival);
      longIntegerFree(ival);
      *val = u64;
      break;

    case dtComplex34:
    case dtReal34:
      if(getRegisterAsReal(reg, &rval)) {
        if (realIsSpecial(&rval)) {
          badDomainError(reg);
          return false;
        }
        *sign = realIsNegative(&rval);
        realSetPositiveSign(&rval);
        frac = !realIsAnInteger(&rval);
        u64 = realToUint64C47(&rval);
        of = (u64 & shortIntegerMask) != u64;
        if(!of)
          switch (shortIntegerMode) {
            case SIM_UNSIGN:
              of = realCompareGreaterEqual(&rval, const_2p64);
              break;
            case SIM_2COMPL:
              if(*sign)
                of = realCompareGreaterThan(&rval, const_2p63);
              else
                /* fall through */
            case SIM_1COMPL:
            case SIM_SIGNMT:
              of = realCompareGreaterEqual(&rval, const_2p63);
              break;
          }
        *val = u64;
        break;
      }
      /* fall through */

    default:
      badTypeError(reg);
      return false;
  }

  if(overflow != NULL)
    *overflow = of;
  if(fractional != NULL)
    *fractional = frac;
  return true;
}

bool_t getRegisterAsRawShortInt(calcRegister_t reg, uint64_t *val, uint32_t *base) {
  bool_t sign, overflow, fractional;
  uint64_t v;
  uint32_t b;

  if (getRegisterDataType(reg) == dtShortInteger) {
    v = *REGISTER_SHORT_INTEGER_DATA(reg);
    b = getRegisterShortIntegerBase(reg);
    goto finish;
  }
  if (!getRegisterAsShortInt(reg, &sign, &v, &overflow, &fractional))
    return false;
  if (overflow || fractional) {
    badDomainError(reg);
    return false;
  }
  v = (uint64_t)WP34S_build_value(v, sign);
  b = lastIntegerBase != 0 ? lastIntegerBase : 10;
finish:
  if (base != NULL)
    *base = b;
  *val = v;
  return true;
}

int getRegisterAsLongIntQuiet(calcRegister_t reg, longInteger_t val, bool_t *fractional) {
  real_t rval;
  bool_t frac = false;

  longIntegerInit(val);
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
        if (realIsSpecial(&rval))
          return ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN;
        if (!realIsAnInteger(&rval)) {
          realToIntegralValue(&rval, &rval, DEC_ROUND_DOWN, &ctxtReal39);
          frac = true;
        }
        convertRealToLongInteger(&rval, val, DEC_ROUND_DOWN);
        break;
      }
      /* fall through */

    default:
      return ERROR_INVALID_DATA_TYPE_FOR_OP;
  }
  if (fractional != NULL)
    *fractional = frac;
  return ERROR_NONE;
}

bool_t getRegisterAsLongInt(calcRegister_t reg, longInteger_t val, bool_t *fractional) {
  const int err = getRegisterAsLongIntQuiet(reg, val, fractional);

  if(err != ERROR_NONE)
    displayCalcErrorMessage(err, ERR_REGISTER_LINE, REGISTER_T);
  return err == ERROR_NONE;
}

static void longIntegerAngleReduction(calcRegister_t regist, angularMode_t angularMode, real_t *reducedAngle) {
  uint32_t oneTurn;
  longInteger_t angle;

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
      real2139_t reducedAngleTmp, reducedAngleTmp2;  // This cannot be increased to 6147 further. 6147 overruns the stack even if we just have the type in here also when using 2139 digits below.
      realContext_t c = ctxtReal75;
      c.digits = 2139;                               // Cannot be increased further. It works well on 1071, worked for a few tests already on 2139 but crashes if this goes to 6147 (together with the real_xxx above)
                                                     // The minimum required for 1000 digits input reduction is slightly less than double, so 1071 is maybe ok for 99.99% cases, but 2139 is preferred as theoretically you will not have a case where 2139 will not work.
      convertLongIntegerRegisterToLongInteger(regist, angle);

      if(longIntegerBase10Digits(angle) > 1000) {
        displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function longIntegerAngleReduction:", "Invalid integer size for angle reduction in radians: exponent too large.", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        longIntegerFree(angle);
        return;
      }

      longIntegerToString(angle, 10, tmpString);   //replaced mpz_get_str(tmpString, 10, angle);
      decNumberFromString((real_t *)&reducedAngleTmp, tmpString, &c);
      WP34S_Mod((real_t *)&reducedAngleTmp, (real_t *)const6147_2pi, (real_t *)&reducedAngleTmp2, &c);
      realPlus((real_t *)&reducedAngleTmp2, reducedAngle, &ctxtReal75);
      longIntegerFree(angle);
      return;
    }
    default: { //amNone
      convertLongIntegerRegisterToReal(regist, reducedAngle, &ctxtReal75);
      return;
    }
  }

  convertLongIntegerRegisterToLongInteger(regist, angle);
  uInt32ToReal(longIntegerModuloUInt(angle, oneTurn), reducedAngle);
  longIntegerFree(angle);
}



bool_t getRegisterAsRealAngle(calcRegister_t reg, real_t *val, angularMode_t *xAngularMode) {
  switch(getRegisterDataType(reg)) {
    case dtLongInteger:
      longIntegerAngleReduction(reg, currentAngularMode, val); 
      // out of range error rolled into longIntegerAngleReduction as the longintegr is not accessible here except for converting again
      *xAngularMode = currentAngularMode;
      break;

    case dtShortInteger:
      convertShortIntegerRegisterToReal(reg, val, &ctxtReal34);
      *xAngularMode = currentAngularMode;
      break;

    case dtReal34:
      real34ToReal(REGISTER_REAL34_DATA(reg), val);
      *xAngularMode = getRegisterAngularMode(reg);
      if(*xAngularMode == amNone)
        *xAngularMode = currentAngularMode;
      if(*xAngularMode == amRadian && realGetExponent(val) > 999) {
        displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
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

void processRealComplexMonadicFunction(void (*realf)(void), void (*complexf)(void)) {
  processIntRealComplexMonadicFunction(realf, complexf, NULL, NULL);
}

void processIntRealComplexMonadicFunction(void (*realf)(void), void (*complexf)(void), void (*shortintf)(void), void (*longintf)(void)) {
  real_t aReal, aImag;
  bool_t cmplxRes = false;
  const uint32_t type = getRegisterDataType(REGISTER_X);

  if(lastFunc != ITM_CHS && !saveLastX())                 // Compiler witll short circuit and not run saveLastX if the related function is CHS
    return;

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

  if(type == dtLongInteger && longintf != NULL)
    longintf();
  else if(type == dtReal34Matrix && realf != NULL)
    elementwiseRema(realf);
  else if(type == dtComplex34Matrix && complexf != NULL)
    elementwiseCxma(complexf);
  else if(getRegisterAsComplexOrReal(REGISTER_X, &aReal, &aImag, &cmplxRes)) {
    if(!cmplxRes && realf != NULL)
      realf();
    else if(complexf != NULL)
      complexf();
    else
      badTypeErrorX();
  }

done:
  adjustResult(REGISTER_X, false, true, REGISTER_X, -1, -1);
}

void processRealComplexDyadicFunction(void (*realf)(void), void (*complexf)(void)) {
  real_t xReal, xImag, yReal, yImag;
  const uint32_t typeX = getRegisterDataType(REGISTER_X);
  const uint32_t typeY = getRegisterDataType(REGISTER_Y);
  bool_t xCmplx = false, yCmplx = false, cmplxRes = false;
  const bool_t xNumber = typeIsNumber(typeX, &xCmplx);
  const bool_t yNumber = typeIsNumber(typeY, &yCmplx);

  if(!saveLastX())
    return;

  if(typeX == dtReal34Matrix && realf != NULL) {
    if(typeY == dtReal34Matrix) {
      elementwiseRemaRema(realf);
      goto fin;
    }
    else if(typeY == dtComplex34Matrix) {
      if(complexf != NULL)
        elementwiseCxmaRema(complexf);
      else
        badTypeError(REGISTER_Y);
      goto fin;
    }
    else if(yNumber) {
      if(yCmplx) {
        if(complexf != NULL)
          elementwiseCplxRema(complexf);
        else
          badTypeError(REGISTER_Y);
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
    if(!xCmplx && realf != NULL)
      elementwiseRemaReal(realf);
    else if(complexf != NULL)
      elementwiseRemaCplx(complexf);
    else
      badTypeErrorX();
    goto fin;
  }
  else if(typeY == dtComplex34Matrix && xNumber) {
    elementwiseCxmaCplx(complexf);
    goto fin;
  }

  /* Fall through to numeric/numeric or error if not */
  if(!getRegisterAsComplexOrReal(REGISTER_X, &xReal, &xImag, &cmplxRes)
      || !getRegisterAsComplexOrReal(REGISTER_Y, &yReal, &yImag, &cmplxRes))
    return;

  if(!cmplxRes && realf != NULL)
    realf();
  else if(complexf != NULL)
    complexf();
  else
    badTypeError(xCmplx ? REGISTER_X : REGISTER_Y);

fin:
  adjustResult(REGISTER_X, true, true, REGISTER_X, REGISTER_Y, -1);
}

void processIntRealComplexDyadicFunction(void (*realf)(void), void (*complexf)(void), void (*shortintf)(void), void (*longintf)(void)) {
  const uint32_t typeX = getRegisterDataType(REGISTER_X);
  const uint32_t typeY = getRegisterDataType(REGISTER_Y);
  const bool_t xInt = typeX == dtLongInteger || typeX == dtShortInteger;
  const bool_t yInt = typeY == dtLongInteger || typeY == dtShortInteger;

  if(typeX == dtShortInteger && typeY == dtShortInteger && shortintf != NULL) {
    if(saveLastX())
      shortintf();
  }
  else if(xInt && yInt && longintf != NULL) {
    if(saveLastX())
      longintf();
  }
  else {
    processRealComplexDyadicFunction(realf, complexf);
    return;
  }
  adjustResult(REGISTER_X, true, true, REGISTER_X, REGISTER_Y, -1);
}
