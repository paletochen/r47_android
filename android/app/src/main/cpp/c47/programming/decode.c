// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file decode.c
 ***********************************************/

#include "c47.h"

TO_QSPI const char shuffleReg[4] = {'x', 'y', 'z', 't'};
TO_QSPI const char supDigit[24] = STD_SUP_0 STD_SUP_1 STD_SUP_2 STD_SUP_3 STD_SUP_4 STD_SUP_5 STD_SUP_6 STD_SUP_7 STD_SUP_8 STD_SUP_9;
TO_QSPI const char baseChars[36] = "??" STD_BASE_1 STD_BASE_2 STD_BASE_3 STD_BASE_4 STD_BASE_5 STD_BASE_6 STD_BASE_7 STD_BASE_8 STD_BASE_9 STD_BASE_10 STD_BASE_11 STD_BASE_12 STD_BASE_13 STD_BASE_14 STD_BASE_15 STD_BASE_16;
TO_QSPI const char angleChars[12] = STD_SUP_r STD_SUP_g STD_DEGREE "??" STD_SUP_pir;

#if !defined(DMCP_BUILD)
  void listPrograms(void) {
    uint16_t i, numberOfBytesInStep, stepNumber = 0, programNumber = 0;
    uint8_t *nextStep, *step;

    printf("\nProgram listing");
    step = beginOfProgramMemory;
    while(step) {
      if(step == programList[programNumber].instructionPointer) {
        programNumber++;
        if(programNumber != 1) {
          printf("\n------------------------------------------------------------");
        }
        printf("\nPgm Step   Bytes         OP");
      }

      nextStep = findNextStep(step);
      if(nextStep) {
        numberOfBytesInStep = (uint16_t)(nextStep - step);
        printf("\n%02d  %4d  ", programNumber, ++stepNumber - programList[programNumber - 1].step + 1); fflush(stdout);

        for(i=0; i<numberOfBytesInStep; i++) {
          printf(" %02x", *(step + i)); fflush(stdout);
          if(i == 3 && numberOfBytesInStep > 4) {
            decodeOneStep(step);
            stringToUtf8(tmpString, (uint8_t *)(tmpString + 2000));

            if(!checkOpCodeOfStep(step, ITM_LBL) && !isAtEndOfProgram(step)) { // Not LBL and not END
              printf("   "); fflush(stdout);
            }

            printf("   %s", tmpString + 2000); fflush(stdout);
          }

          if(i%4 == 3 && i != numberOfBytesInStep - 1) {
            printf("\n          "); fflush(stdout);
          }
        }

        if(numberOfBytesInStep <= 4) {
          for(i=1; i<=4 - ((numberOfBytesInStep - 1) % 4); i++) {
            printf("   "); fflush(stdout);
          }
          decodeOneStep(step);
          stringToUtf8(tmpString, (uint8_t *)(tmpString + 2000));

          if(!checkOpCodeOfStep(step, ITM_LBL) && !isAtEndOfProgram(step)) { // Not LBL and not END
            printf("   "); fflush(stdout);
          }

          printf("%s", tmpString + 2000); fflush(stdout);
        }
      }

      step = nextStep;
    }
    printf("\n");
  }


  void listLabelsAndPrograms(void) {
    printf("\nContent of labelList\n");
    printf("num program  step label\n");
    for(int i=0; i<numberOfLabels; i++) {
      printf("%3d%8d%6d ", i, labelList[i].program, labelList[i].step);
      if(labelList[i].step < 0) { // Local label
        if(*(labelList[i].labelPointer) <= 99) { // Local label from 00 to 99
          printf("%02d\n", *(labelList[i].labelPointer));
        }
        else if(*(labelList[i].labelPointer) <= LAST_UC_LOCAL_LABEL) { // Local label from A to L
          printf("%c\n", *(labelList[i].labelPointer) - 100 + 'A');
        }
        else if(*(labelList[i].labelPointer) <= LAST_LOCAL_LABEL) { // Local label from a to l
          printf("%c\n", *(labelList[i].labelPointer) - FIRST_LC_LOCAL_LABEL + 'a');
        }
      }
      else { // Global label
        xcopy(tmpString + 100, labelList[i].labelPointer + 1, *(labelList[i].labelPointer));
        tmpString[100 + *(labelList[i].labelPointer)] = 0;
        stringToUtf8(tmpString + 100, (uint8_t *)tmpString);
        printf("'%s'\n", tmpString);
      }
    }

    printf("\nContent of programList\n");
    printf("program  step OP\n");
    for(int i=0; i<numberOfPrograms; i++) {
      decodeOneStep(programList[i].instructionPointer);
      stringToUtf8(tmpString, (uint8_t *)(tmpString + 2000));
      printf("%7d %5d %s\n", i, programList[i].step, tmpString);
    }
  }
#endif // !DMCP_BUILD


static void getStringLabelOrVariableName(uint8_t *stringAddress) {
  uint8_t stringLength = *(uint8_t *)(stringAddress++);
  xcopy(tmpStringLabelOrVariableName, stringAddress, stringLength);
  tmpStringLabelOrVariableName[stringLength] = 0;
}


static void getIndirectRegister(uint8_t *paramAddress, const char *op) {
  uint8_t opParam = *(uint8_t *)paramAddress;
  if(opParam < REGISTER_X_IN_KS_CODE) { // Global register from 00 to 99
    sprintf(tmpString, "%s " STD_RIGHT_ARROW "%02u", op, opParam);
  }
  else if(opParam <= REGISTER_K_IN_KS_CODE) { // Lettered register from X to K
    sprintf(tmpString, "%s " STD_RIGHT_ARROW "%s", op, indexOfItems[ITM_REG_X + opParam - REGISTER_X_IN_KS_CODE].itemSoftmenuName);
  }
  else if(opParam <= LAST_LOCAL_REGISTER_IN_KS_CODE) { // Local register from .00 to .98
    sprintf(tmpString, "%s " STD_RIGHT_ARROW ".%02d", op, opParam - FIRST_LOCAL_REGISTER_IN_KS_CODE);
  }
  else if(opParam <= REGISTER_W_IN_KS_CODE) { // Lettered register from M to S and E to W
    sprintf(tmpString, "%s " STD_RIGHT_ARROW "%s", op, indexOfItems[ITM_REG_M + opParam - REGISTER_M_IN_KS_CODE].itemSoftmenuName);
  }
  else {
    sprintf(tmpString, "\nIn function getIndirectRegister: %s " STD_RIGHT_ARROW " %u is not a valid parameter!", op, opParam);
  }
}


static void getIndirectVariable(uint8_t *stringAddress, const char *op) {
  char *str = tmpString;
  getStringLabelOrVariableName(stringAddress);
  str = stringCopy(str, op);
  str = stringCopy(str, " " STD_RIGHT_ARROW STD_LEFT_SINGLE_QUOTE);
  str = stringCopy(str, tmpStringLabelOrVariableName);
  str = stringCopy(str, STD_RIGHT_SINGLE_QUOTE);
}


static void decodeOp(uint8_t *paramAddress, const char *op, uint16_t paramMode, uint16_t tamMax) {
uint8_t  opParam   = *(uint8_t *)(paramAddress++);

  switch(paramMode) {
    case PARAM_DECLARE_LABEL: {
      if(opParam <= 99) { // Local label from 00 to 99
        sprintf(tmpString, "%s %02u", op, opParam);
      }
      else if(opParam <= LAST_UC_LOCAL_LABEL) { // Local label from A to L
        sprintf(tmpString, "%s %c", op, 'A' + (opParam - 100));
      }
      else if(opParam <= LAST_LOCAL_LABEL) { // Local label from a to l
        sprintf(tmpString, "%s %c", op, 'a' + (opParam - FIRST_LC_LOCAL_LABEL));;
      }
      else if(opParam == STRING_LABEL_VARIABLE) {
        char *str = tmpString;
        getStringLabelOrVariableName(paramAddress);
        str = stringCopy(str, op);
        str = stringCopy(str, " " STD_LEFT_SINGLE_QUOTE);
        str = stringCopy(str, tmpStringLabelOrVariableName);
        str = stringCopy(str, STD_RIGHT_SINGLE_QUOTE);
      }
      else {
        sprintf(tmpString, "\nIn function decodeOp case PARAM_DECLARE_LABEL: opParam %u is not a valid label!\n", opParam);
      }
      break;
    }

    case PARAM_LABEL: {
      if(opParam <= 99) { // Local label from 00 to 99
        sprintf(tmpString, "%s %02u", op, opParam);
      }
      else if(opParam <= LAST_UC_LOCAL_LABEL) { // Local label from A to L
        sprintf(tmpString, "%s %c", op, 'A' + (opParam - 100));
      }
      else if(opParam <= LAST_LOCAL_LABEL) { // Local label from a to l
        sprintf(tmpString, "%s %c", op, 'a' + (opParam - FIRST_LC_LOCAL_LABEL));
      }
      else if(opParam == STRING_LABEL_VARIABLE) {
        char *str = tmpString;
        getStringLabelOrVariableName(paramAddress);
        str = stringCopy(str, op);
        str = stringCopy(str, " " STD_LEFT_SINGLE_QUOTE);
        str = stringCopy(str, tmpStringLabelOrVariableName);
        str = stringCopy(str, STD_RIGHT_SINGLE_QUOTE);
      }
      else if(opParam == INDIRECT_REGISTER) {
        getIndirectRegister(paramAddress, op);
      }
      else if(opParam == INDIRECT_VARIABLE) {
        getIndirectVariable(paramAddress, op);
      }
      else {
        sprintf(tmpString, "\nIn function decodeOp: case PARAM_LABEL, %s  %u is not a valid parameter!", op, opParam);
      }
      break;
    }

    case PARAM_REGISTER: {
      if(opParam < REGISTER_X_IN_KS_CODE) { // Global register from 00 to 99
        sprintf(tmpString, "%s %02u", op, opParam);
      }
      else if(opParam <= REGISTER_K_IN_KS_CODE) { // Lettered register from X to K
        sprintf(tmpString, "%s %s", op, indexOfItems[ITM_REG_X + opParam - REGISTER_X_IN_KS_CODE].itemSoftmenuName);
      }
      else if(opParam <= LAST_LOCAL_REGISTER_IN_KS_CODE) { // Local register from .00 to .98
        sprintf(tmpString, "%s .%02d", op, opParam - FIRST_LOCAL_REGISTER_IN_KS_CODE);
      }
      else if(opParam <= REGISTER_W_IN_KS_CODE) { // Lettered register from M to S and E to W
        sprintf(tmpString, "%s %s", op, indexOfItems[ITM_REG_M + opParam - REGISTER_M_IN_KS_CODE].itemSoftmenuName);
      }
      else if(opParam == STRING_LABEL_VARIABLE) {
        char *str = tmpString;
        getStringLabelOrVariableName(paramAddress);
        str = stringCopy(str, op);
        str = stringCopy(str, " " STD_LEFT_SINGLE_QUOTE);
        str = stringCopy(str, tmpStringLabelOrVariableName);
        str = stringCopy(str, STD_RIGHT_SINGLE_QUOTE);
      }
      else if(opParam == INDIRECT_REGISTER) {
        getIndirectRegister(paramAddress, op);
      }
      else if(opParam == INDIRECT_VARIABLE) {
        getIndirectVariable(paramAddress, op);
      }
      else {
        sprintf(tmpString, "\nIn function decodeOp: case PARAM_REGISTER, %s  %u is not a valid parameter!", op, opParam);
      }
      break;
    }

    case PARAM_FLAG: {
      if(opParam < FLAG_X) { // Global flag from 00 to 99
        sprintf(tmpString, "%s %02u", op, opParam);
      }
      else if(FLAG_X <= opParam && opParam <= FLAG_K) { // Lettered flag from X to K
        sprintf(tmpString, "%s %c", op, registerFlagLetters[opParam - FLAG_X]);
      }
      else if(opParam <= LAST_LOCAL_FLAG) { // Local flag from .00 to .31
        sprintf(tmpString, "%s .%02d", op, opParam - FIRST_LOCAL_FLAG);
      }
      else if(opParam < FLAG_M) { // Local flag from .32 to .98 are illegal
        sprintf(tmpString, "\nIn function decodeOp: case PARAM_FLAG, %s  %u is not a valid parameter!", op, opParam);
      }
      else if(opParam <= FLAG_W) { // Lettered flag from M to S and E to W
        sprintf(tmpString, "%s %c", op, registerFlagLetters[opParam - FLAG_M + (FLAG_K - FLAG_X + 1)]);
      }
      else if(opParam < SYSTEM_FLAG_NUMBER) { // illegal operands
        sprintf(tmpString, "\nIn function decodeOp: case PARAM_FLAG, %s  %u is not a valid parameter!", op, opParam);
      }
      else if(opParam == SYSTEM_FLAG_NUMBER) {
        if(*paramAddress < 64) {
          sprintf(tmpString, "%s " STD_LEFT_SINGLE_QUOTE "%s" STD_RIGHT_SINGLE_QUOTE, op, indexOfItems[*paramAddress + SFL_TDM24].itemSoftmenuName);
        }
        else {
          sprintf(tmpString, "%s " STD_LEFT_SINGLE_QUOTE "%s" STD_RIGHT_SINGLE_QUOTE, op, indexOfItems[*paramAddress + SFL_MONIT - 64].itemSoftmenuName);
        }
      }
      else if(opParam == INDIRECT_REGISTER) {
        getIndirectRegister(paramAddress, op);
      }
      else if(opParam == INDIRECT_VARIABLE) {
        getIndirectVariable(paramAddress, op);
      }
      else {
        sprintf(tmpString, "\nIn function decodeOp: case PARAM_FLAG, %s  %u is not a valid parameter!", op, opParam);
      }
      break;
    }

    case PARAM_NUMBER_8: {
      if(opParam <= tamMax) { // Value from 0 to 99
        if(tamMax <= 9) {
          sprintf(tmpString, "%s %u", op, opParam);
        }
        else if(tamMax <= 99) {
          sprintf(tmpString, "%s %02u", op, opParam);
        }
        else if(tamMax <= 999) {
          sprintf(tmpString, "%s %03u", op, opParam);
        }
        else {
          sprintf(tmpString, "%s %04u", op, opParam);
        }
      }
      else if(opParam == INDIRECT_REGISTER) {
        getIndirectRegister(paramAddress, op);
      }
      else if(opParam == INDIRECT_VARIABLE) {
        getIndirectVariable(paramAddress, op);
      }
      else {
        sprintf(tmpString, "\nIn function decodeOp: case PARAM_NUMBER, %s  %u is not a valid parameter!", op, opParam);
      }
      break;
    }

    case PARAM_NUMBER_8_16: {
      if(opParam <= 249) { // Value from 0 to 249
        if(tamMax <= 9) {
          sprintf(tmpString, "%s %u", op, opParam);
        }
        else if(tamMax <= 99) {
          sprintf(tmpString, "%s %02u", op, opParam);
        }
        else if(tamMax <= 999) {
          sprintf(tmpString, "%s %03u", op, opParam);
        }
        else {
          sprintf(tmpString, "%s %04u", op, opParam);
        }
      }
      else if(opParam == CNST_BEYOND_250) { // Value from 250 to 499
        sprintf(tmpString, "%s %03u", op, 250 + *(paramAddress));
      }
      else if(opParam == INDIRECT_REGISTER) {
        getIndirectRegister(paramAddress, op);
      }
      else if(opParam == INDIRECT_VARIABLE) {
        getIndirectVariable(paramAddress, op);
      }
      else {
        sprintf(tmpString, "\nIn function decodeOp: case PARAM_NUMBER, %s  %u is not a valid parameter!", op, opParam);
      }
      break;
    }

    case PARAM_NUMBER_16: {
      uint16_t func = (*(paramAddress-3)  << 8) + *(uint8_t *)(paramAddress -2);
      func &= 0x7fff;
      if(isFunctionOldParam16(func)) {  // original Param16 functions without indirection support (little endian parameter)
        sprintf(tmpString, "%s %u", op, opParam + 256 * *(paramAddress));
      }
      else {                        // new Param16 functions with indirection support (big endian parameter)
        if(opParam == INDIRECT_REGISTER) {
          getIndirectRegister(paramAddress, op);
        }
        else if(opParam == INDIRECT_VARIABLE) {
          getIndirectVariable(paramAddress, op);
        }
        else {
          sprintf(tmpString, "%s %u", op, (opParam * 256) + *(paramAddress));
        }
      }
      break;
    }

    case PARAM_COMPARE: {
      if(opParam < REGISTER_X_IN_KS_CODE) { // Global register from 00 to 99
        sprintf(tmpString, "%s %02u", op, opParam);
      }
      else if(opParam <= REGISTER_K_IN_KS_CODE) { // Lettered register from X to K
        sprintf(tmpString, "%s %s", op, indexOfItems[ITM_REG_X + opParam - REGISTER_X_IN_KS_CODE].itemSoftmenuName);
      }
      else if(opParam <= LAST_LOCAL_REGISTER_IN_KS_CODE) { // Local register from .00 to .98
        sprintf(tmpString, "%s .%02d", op, opParam - FIRST_LOCAL_REGISTER_IN_KS_CODE);
      }
      else if(opParam <= REGISTER_W_IN_KS_CODE) { // Lettered register from M to S and E to W
        sprintf(tmpString, "%s %s", op, indexOfItems[ITM_REG_M + opParam - REGISTER_M_IN_KS_CODE].itemSoftmenuName);
      }
      else if(opParam == STRING_LABEL_VARIABLE) {
        char *str = tmpString;
        getStringLabelOrVariableName(paramAddress);
        str = stringCopy(str, op);
        str = stringCopy(str, " " STD_LEFT_SINGLE_QUOTE);
        str = stringCopy(str, tmpStringLabelOrVariableName);
        str = stringCopy(str, STD_RIGHT_SINGLE_QUOTE);
      }
      else if(opParam == VALUE_0) {
        sprintf(tmpString, "%s 0.", op);
      }
      else if(opParam == VALUE_1) {
        sprintf(tmpString, "%s 1.", op);
      }
      else if(opParam == INDIRECT_REGISTER) {
        getIndirectRegister(paramAddress, op);
      }
      else if(opParam == INDIRECT_VARIABLE) {
        getIndirectVariable(paramAddress, op);
      }
      else {
        sprintf(tmpString, "\nIn function decodeOp: case PARAM_COMPARE, %s  %u is not a valid parameter!", op, opParam);
      }
      break;
    }

    case PARAM_KEYG_KEYX: {
      uint8_t *secondParam = findKey2ndParam(paramAddress - 3);
      decodeOp(secondParam + 1, indexOfItems[*secondParam].itemCatalogName, PARAM_LABEL, indexOfItems[*secondParam].tamMinMax & TAM_MAX_MASK);
      xcopy(tmpString + TMP_STR_LENGTH / 2, tmpString, stringByteLength(tmpString) + 1);
      decodeOp(paramAddress - 1, op, PARAM_NUMBER_8, 21);
      tmpString[stringByteLength(tmpString) + 1] = 0;
      tmpString[stringByteLength(tmpString)    ] = ' ';
      xcopy(tmpString + stringByteLength(tmpString), tmpString + TMP_STR_LENGTH / 2, stringByteLength(tmpString + TMP_STR_LENGTH / 2) + 1);
      break;
    }

    case PARAM_SKIP_BACK: {
      sprintf(tmpString, "%s %03u", op, opParam);
      break;
    }

    case PARAM_SHUFFLE: {
      sprintf(tmpString, "%s %c%c%c%c", op, shuffleReg[ opParam & 0x03      ],
                                            shuffleReg[(opParam & 0x0c) >> 2],
                                            shuffleReg[(opParam & 0x30) >> 4],
                                            shuffleReg[(opParam & 0xc0) >> 6]);
      break;
    }

    case PARAM_MENU: {
      if(opParam == STRING_LABEL_VARIABLE) {
        char *str = tmpString;
        getStringLabelOrVariableName(paramAddress);
        str = stringCopy(str, op);
        str = stringCopy(str, " " STD_LEFT_SINGLE_QUOTE);
        str = stringCopy(str, tmpStringLabelOrVariableName);
        str = stringCopy(str, STD_RIGHT_SINGLE_QUOTE);
      }
      else if(opParam == INDIRECT_REGISTER) {
        getIndirectRegister(paramAddress, op);
      }
      else if(opParam == INDIRECT_VARIABLE) {
        getIndirectVariable(paramAddress, op);
      }
      else {
        sprintf(tmpString, "\nIn function decodeOp: case PARAM_MENU, %s  %u is not a valid parameter!", op, opParam);
      }
      break;
    }

    default: {
      sprintf(tmpString, "\nIn function decodeOp: paramMode %u is not valid!\n", paramMode);
    }
  }
}


static void _decodeNumeral(char *startPtr, const char *srcStartPtr, bool_t isLongInt, char **updatedTgtPtr, const char **updatedSrcPtr) {
  int32_t digit;
  char *strPtr = startPtr;
  const char *srcStr = srcStartPtr;

  if(*srcStr == '-') {
    ++srcStr;
  }
  for(digit = 0; ((*srcStr >= '0' && *srcStr <= '9') || (*srcStr >= 'A' && *srcStr <= 'F')); ++digit, ++srcStr) {
  }
  srcStr = srcStartPtr;

  if(*srcStr == '-') {
    *(strPtr++) = *(srcStr++);
  }
  while((*srcStr >= '0' && *srcStr <= '9') || (*srcStr >= 'A' && *srcStr <= 'F') || *srcStr == '.' || *srcStr == ',') {
    if(digit == 0) {
      *(strPtr++) = RADIX34_MARK_CHAR;
      ++srcStr;
    }
    else {
      if(!GROUPRIGHT_DISABLED && digit < -1 && (abs(digit) % GROUPWIDTH_RIGHT) == 1) {
        *(strPtr++) = gapChar1Right[0];
        if(gapChar1Right[1] !=1) *(strPtr++) = gapChar1Right[1];
      }
      *(strPtr++) = *(srcStr++);
      if(!GROUPLEFT_DISABLED && digit > 1 && (digit % GROUPWIDTH_LEFT) == 1) {
        *(strPtr++) = gapChar1Left[0];
        if(gapChar1Left[1]!=1) *(strPtr++) = gapChar1Left[1];
      }
    }
    --digit;
  }
  if(digit == 0 && !isLongInt) {
    *(strPtr++) = RADIX34_MARK_CHAR;
  }

  if(*srcStr == 'e') {
    ++srcStr;
    *(strPtr++) = PRODUCT_SIGN[0];
    *(strPtr++) = PRODUCT_SIGN[1];
    *(strPtr++) = STD_SUB_10[0];
    *(strPtr++) = STD_SUB_10[1];
    if(*srcStr == '-') {
      *(strPtr++) = STD_SUP_MINUS[0];
      *(strPtr++) = STD_SUP_MINUS[1];
      ++srcStr;
    }
    else if(*srcStr == '+') {
      ++srcStr;
    }
    while(*srcStr >= '0' && *srcStr <= '9') {
      *(strPtr++) = supDigit[0 + (*srcStr - '0') * 2];
      *(strPtr++) = supDigit[1 + (*srcStr - '0') * 2];
      ++srcStr;
    }
  }
  else if(*srcStr == '#') { // input not yet closed
    *(strPtr++) = *(srcStr++);
    while(*srcStr >= '0' && *srcStr <= '9') {
      *(strPtr++) = *(srcStr++);
    }
  }

  *strPtr = 0;
  if(updatedTgtPtr) {
    *updatedTgtPtr = strPtr;
  }
  if(updatedSrcPtr) {
    *updatedSrcPtr = srcStr;
  }
}

static void decodeLiteral(uint8_t *literalAddress) {
  decodedIntegerBase = 0;
  switch(*(literalAddress++)) {
    case BINARY_SHORT_INTEGER: {
      reallocateRegister(TEMP_REGISTER_1, dtShortInteger, 0, *(uint8_t *)(literalAddress++));
      xcopy(getRegisterDataPointer(TEMP_REGISTER_1), literalAddress, TO_BYTES(SHORT_INTEGER_SIZE_IN_BLOCKS));
      shortIntegerToDisplayString(TEMP_REGISTER_1, tmpString, false, noBaseOverride);
      break;
    }

    //case BINARY_LONG_INTEGER: {
    //  break;
    //}

    case BINARY_REAL34: {
      real34_t realLiteral;
      xcopy(&realLiteral, literalAddress, REAL34_SIZE_IN_BYTES);
      real34ToDisplayString(&realLiteral, amNone, tmpString, &standardFont, 9999, 34, !LIMITEXP, !FRONTSPACE, NOIRFRAC);
      break;
    }

    case BINARY_COMPLEX34: {
      complex34_t complexLiteral;
      xcopy(VARIABLE_REAL34_DATA(&complexLiteral), literalAddress                       , REAL34_SIZE_IN_BYTES);
      xcopy(VARIABLE_IMAG34_DATA(&complexLiteral), literalAddress + REAL34_SIZE_IN_BYTES, REAL34_SIZE_IN_BYTES);
      complex34ToDisplayString(&complexLiteral, tmpString, &standardFont, 9999, 34, !LIMITEXP, !FRONTSPACE, NOIRFRAC, currentAngularMode, getSystemFlag(FLAG_POLAR));
      break;
    }

    //case BINARY_DATE: {
    //  break;
    //}

    //case BINARY_TIME: {
    //  break;
    //}

    case STRING_SHORT_INTEGER: {
      int32_t digit;
      uint8_t gap = GROUPWIDTH_LEFT;
      char *dispStringPtr = tmpString;
      char *sourceStringPtr = tmpStringLabelOrVariableName;
      uint8_t base = (uint8_t)(*literalAddress);
      decodedIntegerBase = base;
      getStringLabelOrVariableName(literalAddress + 1);

      if(!GROUPLEFT_DISABLED) {
        if(base == 2) {
          gap = 4;
        }
        else if(base == 4 || base == 8 || base == 16) {
          gap = 2;
        }
      }

      if(*sourceStringPtr == '-') {
        ++sourceStringPtr;
      }
      for(digit = 0; (*sourceStringPtr >= '0' && *sourceStringPtr <= '9') || (*sourceStringPtr >= 'A' && *sourceStringPtr <= 'F'); ++digit, ++sourceStringPtr) {
      }
      sourceStringPtr = tmpStringLabelOrVariableName;

      if(*sourceStringPtr == '-') {
        *(dispStringPtr++) = *(sourceStringPtr++);
      }
      while((*sourceStringPtr >= '0' && *sourceStringPtr <= '9') || (*sourceStringPtr >= 'A' && *sourceStringPtr <= 'F')) {
        *(dispStringPtr++) = *(sourceStringPtr++);
        if(gap > 0 && digit > 1 && (digit % gap) == 1) {
          *(dispStringPtr++) = STD_SPACE_PUNCTUATION[0];
          *(dispStringPtr++) = STD_SPACE_PUNCTUATION[1];
        }
        --digit;
      }
      *(dispStringPtr++) = baseChars[base * 2    ];
      *(dispStringPtr++) = baseChars[base * 2 + 1];
      *dispStringPtr = 0;
      break;
    }

    case STRING_LONG_INTEGER: {
      getStringLabelOrVariableName(literalAddress);
      _decodeNumeral(tmpString, tmpStringLabelOrVariableName, true, NULL, NULL);
      break;
    }

    case STRING_REAL34: {
      getStringLabelOrVariableName(literalAddress);
      _decodeNumeral(tmpString, tmpStringLabelOrVariableName, false, NULL, NULL);
      break;
    }

    case STRING_ANGLE_RADIAN:
    case STRING_ANGLE_GRAD:
    case STRING_ANGLE_DEGREE:
    case STRING_ANGLE_MULTPI: {
      getStringLabelOrVariableName(literalAddress);
      _decodeNumeral(tmpString, tmpStringLabelOrVariableName, false, NULL, NULL);
       switch(*(literalAddress - 1)) {
          case STRING_ANGLE_RADIAN: strcat(tmpString,STD_SUP_r); break;
          case STRING_ANGLE_GRAD:   strcat(tmpString,STD_SUP_g); break;
          case STRING_ANGLE_DEGREE: strcat(tmpString,STD_DEGREE);break;
          case STRING_ANGLE_MULTPI: strcat(tmpString,STD_SUP_pir); break;
          default: break;
        }
      break;
    }

    case STRING_COMPLEX34:  {
      char *dispStringPtr = tmpString;
      char *sourceStringPtr = tmpStringLabelOrVariableName;
      getStringLabelOrVariableName(literalAddress);
      _decodeNumeral(dispStringPtr, sourceStringPtr, false, &dispStringPtr, (const char **)&sourceStringPtr);
      if(!getSystemFlag(FLAG_CPXMULT) && aimBuffer[0] == 0 && tmpString[0] == '0' && tmpString[1] == '.' && tmpString[2] == 0) {
        // remove "0." (not "0.0", "0.00" etc.)
        tmpString[0] = 0;
        dispStringPtr = tmpString;
      }
      if(*sourceStringPtr == 'i' || *sourceStringPtr == 'j') { // unlikely
        if(getSystemFlag(FLAG_CPXMULT) || aimBuffer[0] != 0 || tmpString[0] != 0) {
          *(dispStringPtr++) = '+';
        }
        if(getSystemFlag(FLAG_CPXMULT) || aimBuffer[0] != 0) {
          *(dispStringPtr++) = COMPLEX_UNIT[0];
          *(dispStringPtr++) = COMPLEX_UNIT[1];
        }
        ++sourceStringPtr;
      }
      else if(*sourceStringPtr == '+' || *sourceStringPtr == '-') {
        *(dispStringPtr++) = *(sourceStringPtr++);
        if(!getSystemFlag(FLAG_CPXMULT) && aimBuffer[0] == 0 && dispStringPtr - tmpString == 1 && *(sourceStringPtr - 1) == '+') {
          --dispStringPtr;
        }
        if(getSystemFlag(FLAG_CPXMULT) || aimBuffer[0] != 0) {
          *(dispStringPtr++) = COMPLEX_UNIT[0];
          *(dispStringPtr++) = COMPLEX_UNIT[1];
        }
        ++sourceStringPtr;
      }
      if(getSystemFlag(FLAG_CPXMULT) || aimBuffer[0] != 0) {
        *(dispStringPtr++) = PRODUCT_SIGN[0];
        *(dispStringPtr++) = PRODUCT_SIGN[1];
      }
      _decodeNumeral(dispStringPtr, sourceStringPtr, calcMode == CM_PEM && aimBuffer[0] != 0 && (currentStep + 2 == literalAddress), &dispStringPtr, NULL);
      if(!getSystemFlag(FLAG_CPXMULT) && aimBuffer[0] == 0) {
        *(dispStringPtr++) = COMPLEX_UNIT[0];
        *(dispStringPtr++) = COMPLEX_UNIT[1];
        *dispStringPtr = 0;
      }
      break;
    }

    case STRING_LABEL_VARIABLE: {
      char *str = tmpString;
      getStringLabelOrVariableName(literalAddress);
      str = stringCopy(str, STD_LEFT_SINGLE_QUOTE);
      str = stringCopy(str, tmpStringLabelOrVariableName);
      str = stringCopy(str, STD_RIGHT_SINGLE_QUOTE);
      break;
    }

    case STRING_DATE: {
      getStringLabelOrVariableName(literalAddress);
      reallocateRegister(TEMP_REGISTER_1, dtDate, 0, amNone);
      stringToReal34(tmpStringLabelOrVariableName, REGISTER_REAL34_DATA(TEMP_REGISTER_1));
      julianDayToInternalDate(REGISTER_REAL34_DATA(TEMP_REGISTER_1), REGISTER_REAL34_DATA(TEMP_REGISTER_1));
      dateToDisplayString(TEMP_REGISTER_1, tmpString);
      break;
    }

    case STRING_TIME: {
      char *timeStringPtr = tmpString;
      char *sourceStringPtr = tmpStringLabelOrVariableName;
      getStringLabelOrVariableName(literalAddress);
      for(; *sourceStringPtr != '.' && *sourceStringPtr != 0; ++sourceStringPtr) {
        *(timeStringPtr++) = *sourceStringPtr;
      }
      if(*sourceStringPtr == '.') {
        ++sourceStringPtr;
      }
      *(timeStringPtr++) = ':';
      if(*sourceStringPtr != 0) {
        *(timeStringPtr++) = *(sourceStringPtr++);
      }
      else {
        *(timeStringPtr++) = '0';
      }
      if(*sourceStringPtr != 0) {
        *(timeStringPtr++) = *(sourceStringPtr++);
      }
      else {*(timeStringPtr++) = '0';
      }
      *(timeStringPtr++) = ':';
      if(*sourceStringPtr != 0) {
        *(timeStringPtr++) = *(sourceStringPtr++);
      }
      else {
        *(timeStringPtr++) = '0';
      }
      if(*sourceStringPtr != 0) {
        *(timeStringPtr++) = *(sourceStringPtr++);
      }
      else {
        *(timeStringPtr++) = '0';
      }
      if(*sourceStringPtr != 0) {
        *(timeStringPtr++) = '.';
      }
      for(; *sourceStringPtr != 0; ++sourceStringPtr) {
        *(timeStringPtr++) = *sourceStringPtr;
      }
      *(timeStringPtr++) = 0;
      break;
    }

    case STRING_ANGLE_DMS: {
      char *angleStringPtr = tmpString;
      char *sourceStringPtr = tmpStringLabelOrVariableName;
      getStringLabelOrVariableName(literalAddress);
      for(; *sourceStringPtr != '.' && *sourceStringPtr != 0; ++sourceStringPtr) {
        *(angleStringPtr++) = *sourceStringPtr;
      }
      if(*sourceStringPtr == '.') {
        ++sourceStringPtr;
      }
      *(angleStringPtr++) = STD_DEGREE[0];
      *(angleStringPtr++) = STD_DEGREE[1];
      if(*sourceStringPtr != 0) {
        *(angleStringPtr++) = *(sourceStringPtr++);
      }
      else {
        *(angleStringPtr++) = '0';
      }
      if(*sourceStringPtr != 0) {
        *(angleStringPtr++) = *(sourceStringPtr++);
      }
      else {
        *(angleStringPtr++) = '0';
      }
      *(angleStringPtr++) = '\'';
      if(*sourceStringPtr != 0) {
        *(angleStringPtr++) = *(sourceStringPtr++);
      }
      else {
        *(angleStringPtr++) = '0';
      }
      if(*sourceStringPtr != 0) {
        *(angleStringPtr++) = *(sourceStringPtr++);
      }
      else {
        *(angleStringPtr++) = '0';
      }
      if(*sourceStringPtr != 0) {
        *(angleStringPtr++) = '.';
      }
      for(; *sourceStringPtr != 0; ++sourceStringPtr) {
        *(angleStringPtr++) = *sourceStringPtr;
      }
      *(angleStringPtr++) = '"';
      *(angleStringPtr++) = 0;
      break;
    }

    default: {
      #if !defined(DMCP_BUILD)
        printf("\nERROR: in decodeLiteral() %u is not an acceptable parameter for ITM_LITERAL!\n", *(literalAddress - 1));
        printf("At address ram + %" PRIu32 "\n", (uint32_t)((literalAddress - 1) - (uint8_t *)ram));
      #endif // !DMCP_BUILD
    }
  }
}


static void decodeRem(uint8_t *literalAddress) {
  if(*(uint8_t *)(literalAddress++) == STRING_LABEL_VARIABLE) {
    char *str = tmpString;
    getStringLabelOrVariableName(literalAddress);
    str = stringCopy(str, "REM ");
    str = stringCopy(str, STD_LEFT_SINGLE_QUOTE);
    str = stringCopy(str, tmpStringLabelOrVariableName);
    str = stringCopy(str, STD_RIGHT_SINGLE_QUOTE);
  }

  #if !defined(DMCP_BUILD)
    else {
      printf("\nERROR: %u is not an acceptable parameter for ITM_REM!\n", *(uint8_t *)(literalAddress - 1));
    }
  #endif // !DMCP_BUILD
}



static void _decodeOneStep(uint8_t *step, uint16_t textVersion) {
  uint16_t op = *(step++);
  if(op & 0x80) {
    op &= 0x7f;
    op <<= 8;
    op |= *(step++);
  }

  if(op == 0x7fff) { // .END.
    xcopy(tmpString, ".END.", 6);
  }
  else {
    char nameOp[36];
    nameOp[0]=0;
    switch(indexOfItems[op].status & PTP_STATUS) {
      case PTP_NONE: {
        if(FIRST_CONSTANT <= op && op <= LAST_CONSTANT) {
          sprintf(nameOp, "%2i",op - FIRST_CONSTANT + 1);
          strcat(nameOp," ");
          strcat(nameOp,indexOfItems[op].itemCatalogName);
          strcat(nameOp," ");
          strcat(nameOp,indexOfItems[op].itemSoftmenuName);
        }
        if(op == ITM_op_j) sprintf(nameOp,"op_%s", COMPLEX_UNIT);
        else if(op == ITM_op_j_pol) sprintf(nameOp,"op_%s" STD_SUB_SUN, COMPLEX_UNIT);
        if(nameOp[0] == 0) strcpy(nameOp,indexOfItems[op].itemCatalogName[0] != 0 ? indexOfItems[op].itemCatalogName : indexOfItems[op].itemSoftmenuName);
        if(indexOfItems[op].param == multiply || indexOfItems[op].param == divide) expandConversionName(nameOp);
        sprintf(tmpString, "%s%s", (FIRST_CONSTANT <= op && op <= LAST_CONSTANT) ? "# " : "", nameOp);
        break;
      }

      case PTP_DISABLED: {
        #if defined(PC_BUILD)
          printf("\nERROR in decodeOneStep: instruction %u:%s is not programmable!\n", op, indexOfItems[op].itemCatalogName);
        #endif
        break;
      }

      case PTP_LITERAL: {
        decodeLiteral(step);
        break;
      }

      case PTP_REM: {
        decodeRem(step);
        break;
      }

      default: {
        if(op == ITM_INTEGRAL) {
          strcpy(nameOp,indexOfItems[op].itemCatalogName); //   STD_INTEGRAL "fd");
        }
        else
        if(op == ITM_INTEGRAL_YX) {
          strcpy(nameOp,indexOfItems[op].itemCatalogName); //   STD_INTEGRAL "fyxd");
        }
        else {
          if(nameOp[0] == 0) strcpy(nameOp,indexOfItems[op].itemCatalogName);
        }
        decodeOp(step, nameOp, (indexOfItems[op].status & PTP_STATUS) >> 9, indexOfItems[op].tamMinMax & TAM_MAX_MASK);
      }
    }
  }
}

void decodeOneStep(uint8_t *step) {
  _decodeOneStep(step, MODE_NRM);
}

void decodeOneStep_XPORT(uint8_t *step) {
  _decodeOneStep(step, MODE_RTF);
}
