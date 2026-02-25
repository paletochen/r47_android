// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

#include "reservedRegisterLookup.h"

#if !defined(TESTSUITE_BUILD)
  TO_QSPI const reservedVariableDescStr_t varDescr[] = {

/*  VAR_NO_X        0 */ { ""},
/*  VAR_NO_Y        1 */ { ""},
/*  VAR_NO_Z        2 */ { ""},
/*  VAR_NO_T        3 */ { ""},
/*  VAR_NO_A        4 */ { ""},
/*  VAR_NO_B        5 */ { ""},
/*  VAR_NO_C        6 */ { ""},
/*  VAR_NO_D        7 */ { ""},
/*  VAR_NO_L        8 */ { ""},
/*  VAR_NO_I        9 */ { ""},
/*  VAR_NO_J       10 */ { ""},
/*  VAR_NO_K       11 */ { ""},
/*  VAR_NO_M       12 */ { ""},
/*  VAR_NO_N       13 */ { ""},
/*  VAR_NO_P       14 */ { ""},
/*  VAR_NO_Q       15 */ { ""},
/*  VAR_NO_R       16 */ { ""},
/*  VAR_NO_S       17 */ { ""},
/*  VAR_NO_E       18 */ { ""},
/*  VAR_NO_F       19 */ { ""},
/*  VAR_NO_G       20 */ { ""},
/*  VAR_NO_H       21 */ { ""},
/*  VAR_NO_O       22 */ { ""},
/*  VAR_NO_U       23 */ { ""},
/*  VAR_NO_V       24 */ { ""},
/*  VAR_NO_W       25 */ { ""},
/*  VAR_NO_ADM     26 */ { ""},
/*  VAR_NO_DENMAX  27 */ { ""},
/*  VAR_NO_ISM     28 */ { ""},
/*  VAR_NO_REALDF  29 */ { ""},
/*  VAR_NO_NDEC    30 */ { ""},
/*  VAR_NO_ACC     31 */ { ""},
/*  VAR_NO_ULIM    32 */ { ""},
/*  VAR_NO_LLIM    33 */ { ""},
/*  VAR_NO_FV      34 */ { " Future Value ="  },
/*  VAR_NO_IPONA   35 */ { " I%YR = APR ="    },
/*  VAR_NO_NPPER   36 */ { " n pay periods =" },
/*  VAR_NO_PPERONA 37 */ { " Pay periods YR ="},
/*  VAR_NO_PMT     38 */ { " Payment ="       },
/*  VAR_NO_PV      39 */ { " Present Value =" },
/*  VAR_NO_GRAMOD  40 */ { ""},
/*  VAR_NO_UX      41 */ { ""},
/*  VAR_NO_LX      42 */ { ""},
/*  VAR_NO_CPERONA 43 */ { " Compounding periods YR ="},
/*  VAR_NO_UEST    44 */ { ""},
/*  VAR_NO_LEST    45v*/ { ""},
/*  VAR_NO_UY      46 */ { ""},
/*  VAR_NO_LY      47 */ { ""}

};
#endif //TESTSUITE_BUILD



TO_QSPI const reservedVariableHeader_t allReservedVariables[] = { // MUST be in the same order as the reserved variables in item.c item 1165 and upwards
/*  VAR_NO_X        0 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'X',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_Y        1 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'Y',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_Z        2 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'Z',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_T        3 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'T',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_A        4 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'A',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_B        5 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'B',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_C        6 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'C',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_D        7 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'D',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_L        8 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'L',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_I        9 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'I',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_J       10 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'J',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_K       11 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'K',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_M       12 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'M',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_N       13 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'N',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_P       14 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'P',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_Q       15 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'Q',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_R       16 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'R',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_S       17 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'S',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_E       18 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'E',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_F       19 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'F',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_G       20 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'G',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_H       21 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'H',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_O       22 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'O',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_U       23 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'U',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_V       24 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'V',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_W       25 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = 0,             .tag = 0,           .readOnly = 0, .notUsed = 0}, .reservedVariableName = {1, 'W',  0,   0,   0,   0,   0,   0} },
/*  VAR_NO_ADM     26 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = dtLongInteger, .tag = LI_POSITIVE, .readOnly = 1, .notUsed = 0}, .reservedVariableName = {3, 'A', 'D', 'M',  0,   0,   0,   0} },
/*  VAR_NO_DENMAX  27 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = dtLongInteger, .tag = LI_POSITIVE, .readOnly = 1, .notUsed = 0}, .reservedVariableName = {5, 'D', '.', 'M', 'A', 'X',  0,   0} },
/*  VAR_NO_ISM     28 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = dtLongInteger, .tag = LI_POSITIVE, .readOnly = 1, .notUsed = 0}, .reservedVariableName = {3, 'I', 'S', 'M',  0,   0,   0,   0} },
/*  VAR_NO_REALDF  29 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = dtLongInteger, .tag = LI_POSITIVE, .readOnly = 1, .notUsed = 0}, .reservedVariableName = {6, 'R', 'E', 'A', 'L', 'D', 'F',  0} },
/*  VAR_NO_NDEC    30 */  { .header = {.pointerToRegisterData = C47_NULL,   .dataType = dtLongInteger, .tag = LI_POSITIVE, .readOnly = 1, .notUsed = 0}, .reservedVariableName = {4, '#', 'D', 'E', 'C',  0,   0,   0} },
/*  VAR_NO_ACC     31 */  { .header = {.pointerToRegisterData = 0,          .dataType = dtReal34,      .tag = amNone,      .readOnly = 0, .notUsed = 0}, .reservedVariableName = {3, 'A', 'C', 'C',  0,   0,   0,   0} },
/*  VAR_NO_ULIM    32 */  { .header = {.pointerToRegisterData = 4,          .dataType = dtReal34,      .tag = amNone,      .readOnly = 0, .notUsed = 0}, .reservedVariableName = {5, 161, 145, 'L', 'i', 'm',  0,   0} },
/*  VAR_NO_LLIM    33 */  { .header = {.pointerToRegisterData = 8,          .dataType = dtReal34,      .tag = amNone,      .readOnly = 0, .notUsed = 0}, .reservedVariableName = {5, 161, 147, 'L', 'i', 'm',  0,   0} },
/*  VAR_NO_FV      34 */  { .header = {.pointerToRegisterData = 12,         .dataType = dtReal34,      .tag = amNone,      .readOnly = 0, .notUsed = 0}, .reservedVariableName = {2, 'F', 'V',  0,   0,   0,   0,   0} },
/*  VAR_NO_IPONA   35 */  { .header = {.pointerToRegisterData = 16,         .dataType = dtReal34,      .tag = amNone,      .readOnly = 0, .notUsed = 0}, .reservedVariableName = {4, 'I', '%', '/', 'a',  0,   0,   0} },
/*  VAR_NO_NPPER   36 */  { .header = {.pointerToRegisterData = 20,         .dataType = dtReal34,      .tag = amNone,      .readOnly = 0, .notUsed = 0}, .reservedVariableName = {5, 'N', 'P', 'P', 'E', 'R',  0,   0} },
/*  VAR_NO_PPERONA 37 */  { .header = {.pointerToRegisterData = 24,         .dataType = dtReal34,      .tag = amNone,      .readOnly = 0, .notUsed = 0}, .reservedVariableName = {6, 'P', 'P', 'E', 'R', '/',  'a', 0} },
/*  VAR_NO_PMT     38 */  { .header = {.pointerToRegisterData = 28,         .dataType = dtReal34,      .tag = amNone,      .readOnly = 0, .notUsed = 0}, .reservedVariableName = {3, 'P', 'M', 'T',  0,   0,   0,   0} },
/*  VAR_NO_PV      39 */  { .header = {.pointerToRegisterData = 32,         .dataType = dtReal34,      .tag = amNone,      .readOnly = 0, .notUsed = 0}, .reservedVariableName = {2, 'P', 'V',  0,   0,   0,   0,   0} },
/*  VAR_NO_GRAMOD  40 */  { .header = {.pointerToRegisterData = 36,         .dataType = dtLongInteger, .tag = LI_POSITIVE, .readOnly = 0, .notUsed = 0}, .reservedVariableName = {6, 'G', 'R', 'A', 'M', 'O', 'D',  0} },
/*  VAR_NO_UX      41 */  { .header = {.pointerToRegisterData = 40,         .dataType = dtReal34,      .tag = amNone,      .readOnly = 0, .notUsed = 0}, .reservedVariableName = {3, 161, 145, 'X',  0,   0,   0,   0} },
/*  VAR_NO_LX      42 */  { .header = {.pointerToRegisterData = 44,         .dataType = dtReal34,      .tag = amNone,      .readOnly = 0, .notUsed = 0}, .reservedVariableName = {3, 161, 147, 'X',  0,   0,   0,   0} },
/*  VAR_NO_CPERONA 43 */  { .header = {.pointerToRegisterData = 48,         .dataType = dtReal34,      .tag = amNone,      .readOnly = 0, .notUsed = 0}, .reservedVariableName = {6, 'C', 'P', 'E', 'R', '/', 'a',  0} },
/*  VAR_NO_UEST    44 */  { .header = {.pointerToRegisterData = 52,         .dataType = dtReal34,      .tag = amNone,      .readOnly = 0, .notUsed = 0}, .reservedVariableName = {5, 161, 145, 'E', 'S', 'T',  0,   0} },
/*  VAR_NO_LEST    45 */  { .header = {.pointerToRegisterData = 56,         .dataType = dtReal34,      .tag = amNone,      .readOnly = 0, .notUsed = 0}, .reservedVariableName = {5, 161, 147, 'E', 'S', 'T',  0,   0} },
/*  VAR_NO_UY      46 */  { .header = {.pointerToRegisterData = 60,         .dataType = dtReal34,      .tag = amNone,      .readOnly = 0, .notUsed = 0}, .reservedVariableName = {3, 161, 145, 'Y',  0,   0,   0,   0} },
/*  VAR_NO_LY      47 */  { .header = {.pointerToRegisterData = 64,         .dataType = dtReal34,      .tag = amNone,      .readOnly = 0, .notUsed = 0}, .reservedVariableName = {3, 161, 147, 'Y',  0,   0,   0,   0} },
};

// REMEMBER: SET LAST_RESERVED_VARIABLE in defines.h


static inline registerHeader_t *POINTER_TO_LOCAL_REGISTER(const calcRegister_t a) {
  return (registerHeader_t *)(currentLocalRegisters + a);
}



uint32_t getRegisterDataType(calcRegister_t regist) {
  if(regist <= LAST_GLOBAL_REGISTER) { // Global register
    return globalRegister[regist].dataType;
  }

  else if(regist <= LAST_NAMED_VARIABLE) { // Named variable
    if(numberOfNamedVariables > 0) {
      regist -= FIRST_NAMED_VARIABLE;
      if(regist < numberOfNamedVariables) {
        return allNamedVariables[regist].header.dataType;
      }
      #if defined(PC_BUILD)
        else {
          sprintf(errorMessage, "named variable %" PRId16, regist);
          sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "Must be from 0 to %" PRIu16, (uint16_t)(numberOfNamedVariables - 1));
          moreInfoOnError("In function getRegisterDataType:", errorMessage, "is not defined!", errorMessage + ERROR_MESSAGE_LENGTH/2);
        }
      #endif // PC_BUILD
    }
    else {
      sprintf(errorMessage, commonBugScreenMessages[bugMsgNoNamedVariables], "getRegisterDataType");
      displayBugScreen(errorMessage);
    }
  }

  else if(regist <= LAST_RESERVED_VARIABLE) { // System named variable
    regist -= FIRST_RESERVED_VARIABLE;
    if(regist < NUMBER_OF_LETTERED_VARIABLES) { // Lettered register
      return globalRegister[regist + REGISTER_X].dataType;
    }
    else {
      return allReservedVariables[regist].header.dataType;
    }
  }

  else if(regist <= LAST_LOCAL_REGISTER) { // Local register
    if(currentLocalRegisters != NULL) {
      regist -= FIRST_LOCAL_REGISTER;
      if(regist < currentNumberOfLocalRegisters) {
        return POINTER_TO_LOCAL_REGISTER(regist)->dataType;
      }
      #if defined(PC_BUILD)
        else {
          sprintf(errorMessage, "local register %" PRId16, regist);
          sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "Must be from 0 to %" PRIu8, (uint8_t)(currentNumberOfLocalRegisters - 1));
          moreInfoOnError("In function getRegisterDataType:", errorMessage, "is not defined!", errorMessage + ERROR_MESSAGE_LENGTH/2);
        }
      #endif // PC_BUILD
    }
    #if defined(PC_BUILD)
      else {
        moreInfoOnError("In function getRegisterDataType:", "no local registers defined!", "", "");
      }
    #endif // PC_BUILD
  }

  else {
    sprintf(errorMessage, commonBugScreenMessages[bugMsgRegistMustBeLessThan], "getRegisterDataType", regist, LAST_RESERVED_VARIABLE + 1);
    displayBugScreen(errorMessage);
  }

  return 31u; // non-existent data type
}



void *getRegisterDataPointer(calcRegister_t regist) {
  if(regist <= LAST_GLOBAL_REGISTER) { // Global register
    return TO_PCMEMPTR(globalRegister[regist].pointerToRegisterData);
  }

  else if(regist <= LAST_NAMED_VARIABLE) { // Named variable
    if(numberOfNamedVariables > 0) {
      regist -= FIRST_NAMED_VARIABLE;
      if(regist < numberOfNamedVariables) {
        return TO_PCMEMPTR(allNamedVariables[regist].header.pointerToRegisterData);
      }
      #if defined(PC_BUILD)
        else {
          sprintf(errorMessage, "named variable %" PRId16, regist);
          sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "Must be from 0 to %" PRIu16, (uint16_t)(numberOfNamedVariables - 1));
          moreInfoOnError("In function getRegisterDataPointer:", errorMessage, "is not defined!", errorMessage + ERROR_MESSAGE_LENGTH/2);
        }
      #endif // PC_BUILD
    }
    else {
      sprintf(errorMessage, commonBugScreenMessages[bugMsgNoNamedVariables], "getRegisterDataPointer");
      displayBugScreen(errorMessage);
    }
  }

  else if(regist <= LAST_RESERVED_VARIABLE) { // System named variable
    regist -= FIRST_RESERVED_VARIABLE;
    return TO_PCMEMPTR(allReservedVariables[regist].header.pointerToRegisterData);
  }

  else if(regist <= LAST_LOCAL_REGISTER) { // Local register
    if(currentLocalRegisters != NULL) {
      regist -= FIRST_LOCAL_REGISTER;
      if(regist < currentNumberOfLocalRegisters) {
        return TO_PCMEMPTR(POINTER_TO_LOCAL_REGISTER(regist)->pointerToRegisterData);
      }
      #if defined(PC_BUILD)
        else {
          sprintf(errorMessage, "local register %" PRId16, regist);
          sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "Must be from 0 to %" PRIu8, (uint8_t)(currentNumberOfLocalRegisters - 1));
          moreInfoOnError("In function getRegisterDataPointer:", errorMessage, "is not defined!", errorMessage + ERROR_MESSAGE_LENGTH/2);
        }
      #endif // PC_BUILD
    }
    #if defined(PC_BUILD)
      else {
        moreInfoOnError("In function getRegisterDataPointer:", "no local registers defined!", "", NULL);
      }
    #endif // PC_BUILD
  }

  else {
    sprintf(errorMessage, commonBugScreenMessages[bugMsgRegistMustBeLessThan], "getRegisterDataPointer", regist, LAST_RESERVED_VARIABLE + 1);
    displayBugScreen(errorMessage);
  }

  return NULL;
}



uint32_t getRegisterTag(calcRegister_t regist) {
  if(regist <= LAST_GLOBAL_REGISTER) { // Global register
    return globalRegister[regist].tag;
  }

  else if(regist <= LAST_NAMED_VARIABLE) { // Named variable
    if(numberOfNamedVariables > 0) {
      regist -= FIRST_NAMED_VARIABLE;
      if(regist < numberOfNamedVariables) {
        return allNamedVariables[regist].header.tag;
      }
      #if defined(PC_BUILD)
        else {
          sprintf(errorMessage, "named variable %" PRId16, regist);
          sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "Must be from 0 to %" PRIu16, (uint16_t)(numberOfNamedVariables - 1));
          moreInfoOnError("In function getRegisterTag:", errorMessage, "is not defined!", errorMessage + ERROR_MESSAGE_LENGTH/2);
        }
      #endif // PC_BUILD
    }
    else {
      sprintf(errorMessage, commonBugScreenMessages[bugMsgNoNamedVariables], "getRegisterTag");
      displayBugScreen(errorMessage);
    }
  }

  else if(regist <= LAST_RESERVED_VARIABLE) { // System named variable
    regist -= FIRST_RESERVED_VARIABLE;
    return allReservedVariables[regist].header.tag;
  }

  else if(regist <= LAST_LOCAL_REGISTER) { // Local register
    if(currentLocalRegisters != NULL) {
      regist -= FIRST_LOCAL_REGISTER;
      if(regist < currentNumberOfLocalRegisters) {
        return POINTER_TO_LOCAL_REGISTER(regist)->tag;
      }
      #if defined(PC_BUILD)
        else {
          sprintf(errorMessage, "local register %" PRId16, regist);
          sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "Must be from 0 to %" PRIu8, (uint8_t)(currentNumberOfLocalRegisters - 1));
          moreInfoOnError("In function getRegisterTag:", errorMessage, "is not defined!", errorMessage + ERROR_MESSAGE_LENGTH/2);
        }
      #endif // PC_BUILD
    }
    #if defined(PC_BUILD)
      else {
       moreInfoOnError("In function getRegisterTag:", "no local registers defined!", "", "");
      }
    #endif // PC_BUILD
  }

  else {
    sprintf(errorMessage, commonBugScreenMessages[bugMsgRegistMustBeLessThan], "getRegisterTag", regist, LAST_RESERVED_VARIABLE + 1);
    displayBugScreen(errorMessage);
  }

  return 0;
}



void setRegisterDataType(calcRegister_t regist, uint16_t dataType, const uint32_t tag) {
  if(regist <= LAST_GLOBAL_REGISTER) { // Global register
    globalRegister[regist].dataType = dataType;
    globalRegister[regist].tag = tag;
  }

  else if(regist <= LAST_NAMED_VARIABLE) { // Named variable
    if(numberOfNamedVariables > 0) {
      regist -= FIRST_NAMED_VARIABLE;
      if(regist < numberOfNamedVariables) {
        allNamedVariables[regist].header.dataType = dataType;
        allNamedVariables[regist].header.tag = tag;
      }
      #if defined(PC_BUILD)
        else {
          sprintf(errorMessage, "named variable %" PRId16, regist);
          sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "Must be from 0 to %" PRIu16, (uint16_t)(numberOfNamedVariables - 1));
          moreInfoOnError("In function setRegisterDataType:", errorMessage, "is not defined!", errorMessage + ERROR_MESSAGE_LENGTH/2);
        }
      #endif // PC_BUILD
    }
    else {
      sprintf(errorMessage, commonBugScreenMessages[bugMsgNoNamedVariables], "setRegisterDataType");
      displayBugScreen(errorMessage);
    }
  }

  else if(regist <= LAST_RESERVED_VARIABLE) { // System named variable
    regist -= FIRST_RESERVED_VARIABLE;
    if(allReservedVariables[regist].header.pointerToRegisterData != C47_NULL && allReservedVariables[regist].header.readOnly == 0) {
      allNamedVariables[regist].header.dataType = dataType;
      allNamedVariables[regist].header.tag = tag;
    }
  }

  else if(regist <= LAST_LOCAL_REGISTER) { // Local register
    if(currentLocalRegisters != NULL) {
      regist -= FIRST_LOCAL_REGISTER;
      if(regist < currentNumberOfLocalRegisters) {
        POINTER_TO_LOCAL_REGISTER(regist)->dataType = dataType;
        POINTER_TO_LOCAL_REGISTER(regist)->tag = tag;
      }
      #if defined(PC_BUILD)
        else {
          sprintf(errorMessage, "local register %" PRId16, regist);
          sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "Must be from 0 to %" PRIu8, (uint8_t)(currentNumberOfLocalRegisters - 1));
          moreInfoOnError("In function setRegisterDataType:", errorMessage, "is not defined!", errorMessage + ERROR_MESSAGE_LENGTH/2);
        }
      #endif // PC_BUILD
    }
    #if defined(PC_BUILD)
      else {
       moreInfoOnError("In function setRegisterDataType:", "no local registers defined!", "", "");
      }
    #endif // PC_BUILD
  }

  else {
    sprintf(errorMessage, commonBugScreenMessages[bugMsgRegistMustBeLessThan], "setRegisterDataType", regist, LAST_RESERVED_VARIABLE + 1);
    displayBugScreen(errorMessage);
  }
}



void setRegisterDataPointer(calcRegister_t regist, const void *memPtr) {
  uint32_t dataPointer = TO_C47MEMPTR(memPtr);

  if(regist <= LAST_GLOBAL_REGISTER) { // Global register
    globalRegister[regist].pointerToRegisterData = dataPointer;
  }

  else if(regist <= LAST_NAMED_VARIABLE) { // Named variable
    if(numberOfNamedVariables > 0) {
      regist -= FIRST_NAMED_VARIABLE;
      if(regist < numberOfNamedVariables) {
        allNamedVariables[regist].header.pointerToRegisterData = dataPointer;
      }
      #if defined(PC_BUILD)
        else {
          sprintf(errorMessage, "named variable %" PRId16, regist);
          sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "Must be from 0 to %" PRIu16, (uint16_t)(numberOfNamedVariables - 1));
          moreInfoOnError("In function setRegisterDataPointer:", errorMessage, "is not defined!", errorMessage + ERROR_MESSAGE_LENGTH/2);
        }
      #endif // PC_BUILD
    }
    #if defined(PC_BUILD)
      else {
        moreInfoOnError("In function setRegisterDataPointer:", "no local registers defined!", NULL, NULL);
      }
    #endif // PC_BUILD
  }

  else if(regist <= LAST_RESERVED_VARIABLE) { // System named variable
  }

  else if(regist <= LAST_LOCAL_REGISTER) { // Local register
    if(currentLocalRegisters != NULL) {
      regist -= FIRST_LOCAL_REGISTER;
      if(regist < currentNumberOfLocalRegisters) {
        POINTER_TO_LOCAL_REGISTER(regist)->pointerToRegisterData = dataPointer;
      }
      #if defined(PC_BUILD)
        else {
          sprintf(errorMessage, "local register %" PRId16, regist);
          sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "Must be from 0 to %" PRIu8, (uint8_t)(currentNumberOfLocalRegisters - 1));
          moreInfoOnError("In function setRegisterDataPointer:", errorMessage, "is not defined!", errorMessage + ERROR_MESSAGE_LENGTH/2);
        }
      #endif // PC_BUILD
    }
    #if defined(PC_BUILD)
      else {
        moreInfoOnError("In function setRegisterDataPointer:", "no local registers defined!", "", "");
      }
    #endif // PC_BUILD
  }

  else {
    sprintf(errorMessage, commonBugScreenMessages[bugMsgRegistMustBeLessThan], "setRegisterDataPointer", regist, LAST_RESERVED_VARIABLE + 1);
    displayBugScreen(errorMessage);
  }
}



void setRegisterTag(calcRegister_t regist, const uint32_t tag) {
  if(regist <= LAST_GLOBAL_REGISTER) { // Global register
    globalRegister[regist].tag = tag;
  }

  else if(regist <= LAST_NAMED_VARIABLE) { // Named variable
    if(numberOfNamedVariables > 0) {
      regist -= FIRST_NAMED_VARIABLE;
      if(regist < numberOfNamedVariables) {
        allNamedVariables[regist].header.tag = tag;
      }
      #if defined(PC_BUILD)
        else {
          sprintf(errorMessage, "named variable %" PRId16, regist);
          sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "Must be from 0 to %" PRIu16, (uint16_t)(numberOfNamedVariables - 1));
          moreInfoOnError("In function setRegisterTag:", errorMessage, "is not defined!", errorMessage + ERROR_MESSAGE_LENGTH/2);
        }
      #endif // PC_BUILD
    }
    else {
      sprintf(errorMessage, commonBugScreenMessages[bugMsgNoNamedVariables], "setRegisterTag");
      displayBugScreen(errorMessage);
    }
  }

  else if(regist <= LAST_RESERVED_VARIABLE) { // System named variable
  }

  else if(regist <= LAST_LOCAL_REGISTER) { // Local register
    if(currentLocalRegisters != NULL) {
      regist -= FIRST_LOCAL_REGISTER;
      if(regist < currentNumberOfLocalRegisters) {
        POINTER_TO_LOCAL_REGISTER(regist)->tag = tag;
      }
      #if defined(PC_BUILD)
        else {
          sprintf(errorMessage, "local register %" PRId16, regist);
          sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "Must be from 0 to %" PRIu8, (uint8_t)(currentNumberOfLocalRegisters - 1));
          moreInfoOnError("In function setRegisterTag:", errorMessage, "is not defined!", errorMessage + ERROR_MESSAGE_LENGTH/2);
        }
      #endif // PC_BUILD
    }
    #if defined(PC_BUILD)
      else {
        moreInfoOnError("In function setRegisterTag:", "no local registers defined!", "", "");
      }
    #endif // PC_BUILD
  }

  else {
    sprintf(errorMessage, commonBugScreenMessages[bugMsgRegistMustBeLessThan], "setRegisterTag", regist, LAST_RESERVED_VARIABLE + 1);
    displayBugScreen(errorMessage);
  }
}



static bool_t initLocalRegisters(calcRegister_t r) {
  bool_t isMemIssue = false;

  if(lastIntegerBase == 0 && (Input_Default == ID_43S || Input_Default == ID_DP)) {                 //JM defaults JMZERO
    void *newMem = allocC47Blocks(REAL34_SIZE_IN_BLOCKS);
    if(newMem) {
      setRegisterDataType(r, dtReal34, amNone);
      setRegisterDataPointer(r, newMem);
      real34Zero(REGISTER_REAL34_DATA(r));
    }
    else {
      isMemIssue = true;
    }
  }
  else if(lastIntegerBase == 0 && Input_Default == ID_CPXDP) {                //JM defaults vv
    void *newMem = allocC47Blocks(COMPLEX34_SIZE_IN_BLOCKS);
    if(newMem) {
      setRegisterDataType(r, dtComplex34, amNone);
      if(getSystemFlag(FLAG_POLAR)) {
        setRegisterTag(r, currentAngularMode | amPolar);
      }
      setRegisterDataPointer(r, newMem);
      real34Zero(REGISTER_REAL34_DATA(r));
      real34Zero(REGISTER_IMAG34_DATA(r));
    }
    else {
      isMemIssue = true;
    }
  }                                                   //JM defaults ^^
  else if(lastIntegerBase == 0 && Input_Default == ID_LI) {                   //JM defaults vv
    longInteger_t lgInt;
    longIntegerInit(lgInt);
    uInt32ToLongInteger(0u, lgInt);
    convertLongIntegerToLongIntegerRegister(lgInt, r);
    longIntegerFree(lgInt);
  }                                                   //JM defaults ^^
  else if(lastIntegerBase !=0) {                   //JM defaults vv
    longInteger_t lgInt;
    longIntegerInit(lgInt);
    uInt32ToLongInteger(0u, lgInt);
    convertLongIntegerToShortIntegerRegister(lgInt, lastIntegerBase == 0 ? 10 : lastIntegerBase, r);
    longIntegerFree(lgInt);
  }                                                   //JM defaults ^^

  return isMemIssue;
}



void allocateLocalRegisters(uint16_t numberOfRegistersToAllocate) {
  subroutineLevelHeader_t *oldSubroutineLevelData = currentSubroutineLevelData;

  if(numberOfRegistersToAllocate > 99) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "You can allocate up to 99 registers, you requested %" PRIu16, numberOfRegistersToAllocate);
      moreInfoOnError("In function allocateLocalRegisters:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  uint16_t r;
  if(currentLocalFlags == NULL) {
    // 1st allocation of local registers in this level of subroutine

    //TOCHECK XXXX JM (old)
    if((currentSubroutineLevelData = reallocC47Blocks(currentSubroutineLevelData,
                                                      TO_BLOCKS(sizeof(subroutineLevelHeader_t)),
                                                      TO_BLOCKS(sizeof(subroutineLevelHeader_t) + sizeof(localFlags_t) + numberOfRegistersToAllocate*sizeof(registerHeader_t))))) {
      currentLocalFlags = LOCAL_FLAGS_AFTER_SUBROUTINE_LEVEL_HEADER(currentSubroutineLevelData);
      currentNumberOfLocalFlags = NUMBER_OF_LOCAL_FLAGS;
      *currentLocalFlags = 0;
      currentLocalRegisters = (numberOfRegistersToAllocate == 0 ? NULL : LOCAL_REGISTER_HEADERS_AFTER_LOCAL_FLAGS(currentLocalFlags));
      currentNumberOfLocalRegisters = numberOfRegistersToAllocate;

      #if defined VERBOSE_REGISTERS
        printStatus(0, "allocateLocalRegisters1",force);
      #endif //VERBOSE_REGISTERS

      // All the new local registers are real34s initialized to 0.0
      for(r=FIRST_LOCAL_REGISTER; r<FIRST_LOCAL_REGISTER+numberOfRegistersToAllocate; r++) {
        if(initLocalRegisters(r)) {
          // Not enough memory!
          for(uint16_t rr = FIRST_LOCAL_REGISTER; rr < r; rr++) {
            freeRegisterData(FIRST_LOCAL_REGISTER + rr);
          }
          reduceC47Blocks(currentSubroutineLevelData,
                          TO_BLOCKS(sizeof(subroutineLevelHeader_t) + sizeof(localFlags_t) + numberOfRegistersToAllocate*sizeof(registerHeader_t)),
                          TO_BLOCKS(sizeof(subroutineLevelHeader_t)));
          currentLocalFlags = NULL;
          currentNumberOfLocalFlags = 0;
          currentLocalRegisters = NULL;
          currentNumberOfLocalRegisters = 0;
          displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
          return;
        }
      }                                                   //JM defaults ^^

    #if defined VERBOSE_REGISTERS
      printStatus(0, " ", force);
    #endif //VERBOSE_REGISTERS

    }
    else {
      currentSubroutineLevelData = oldSubroutineLevelData;
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      return;
    }
  }
  else if(numberOfRegistersToAllocate != currentNumberOfLocalRegisters) {
    // The number of allocated local registers changes
    if(numberOfRegistersToAllocate > currentNumberOfLocalRegisters) { // The number of local registers increases
      uint8_t oldNumberOfLocalRegisters = currentNumberOfLocalRegisters;
      if((currentSubroutineLevelData = reallocC47Blocks(currentSubroutineLevelData,
                                                        TO_BLOCKS(sizeof(subroutineLevelHeader_t) + sizeof(localFlags_t) + oldNumberOfLocalRegisters*sizeof(registerHeader_t)),
                                                        TO_BLOCKS(sizeof(subroutineLevelHeader_t) + sizeof(localFlags_t) + numberOfRegistersToAllocate*sizeof(registerHeader_t))))) {
        currentLocalFlags = LOCAL_FLAGS_AFTER_SUBROUTINE_LEVEL_HEADER(currentSubroutineLevelData);
        currentLocalRegisters = LOCAL_REGISTER_HEADERS_AFTER_LOCAL_FLAGS(currentLocalFlags);
        currentNumberOfLocalRegisters = numberOfRegistersToAllocate;

        // All the new local registers are real34s initialized to 0.0
        for(r=FIRST_LOCAL_REGISTER+oldNumberOfLocalRegisters; r<FIRST_LOCAL_REGISTER+numberOfRegistersToAllocate; r++) {
          if(initLocalRegisters(r)) {
            // Not enough memory!
            for(uint16_t rr = FIRST_LOCAL_REGISTER + oldNumberOfLocalRegisters; rr < r; rr++) {
              freeRegisterData(FIRST_LOCAL_REGISTER + rr);
            }
            reduceC47Blocks(currentSubroutineLevelData,
                            TO_BLOCKS(sizeof(subroutineLevelHeader_t) + sizeof(localFlags_t) + numberOfRegistersToAllocate*sizeof(registerHeader_t)),
                            TO_BLOCKS(sizeof(subroutineLevelHeader_t) + sizeof(localFlags_t) + oldNumberOfLocalRegisters*sizeof(registerHeader_t)));
            currentLocalFlags = LOCAL_FLAGS_AFTER_SUBROUTINE_LEVEL_HEADER(currentSubroutineLevelData);
            currentLocalRegisters = (oldNumberOfLocalRegisters == 0 ? NULL : LOCAL_REGISTER_HEADERS_AFTER_LOCAL_FLAGS(currentLocalFlags));
            currentNumberOfLocalRegisters = oldNumberOfLocalRegisters;
            displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
            return;
          }
        }
        #if defined VERBOSE_REGISTERS
          printStatus(0, " ", force);
        #endif //VERBOSE_REGISTERS
      }
      else {
        currentSubroutineLevelData = oldSubroutineLevelData;
        displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
        return;
      }
    }
    else { // Ther number of local register decreases
      #if defined VERBOSE_REGISTERS
       printStatus(0, "allocateLocalRegisters3",force);
      #endif //VERBOSE_REGISTERS
      // free memory allocated to the data of the deleted local registers
      for(r=numberOfRegistersToAllocate; r<currentNumberOfLocalRegisters; r++) {
        freeRegisterData(FIRST_LOCAL_REGISTER + r);
      }
      reduceC47Blocks(currentSubroutineLevelData,
                      TO_BLOCKS(sizeof(subroutineLevelHeader_t) + sizeof(localFlags_t) + currentNumberOfLocalRegisters*sizeof(registerHeader_t)),
                      TO_BLOCKS(sizeof(subroutineLevelHeader_t) + sizeof(localFlags_t) + numberOfRegistersToAllocate*sizeof(registerHeader_t)));
      currentLocalFlags = LOCAL_FLAGS_AFTER_SUBROUTINE_LEVEL_HEADER(currentSubroutineLevelData);
      currentLocalRegisters = (numberOfRegistersToAllocate == 0 ? NULL : LOCAL_REGISTER_HEADERS_AFTER_LOCAL_FLAGS(currentLocalFlags));
      currentNumberOfLocalRegisters = numberOfRegistersToAllocate;
      #if defined VERBOSE_REGISTERS
        printStatus(0, "", force);
      #endif //VERBOSE_REGISTERS
      return;
    }
  }
  else {
    return;
  }

  if(currentSubroutineLevel == 0) {
    allSubroutineLevels.ptrToSubroutineLevel0Header = TO_C47MEMPTR(currentSubroutineLevelData);
  }
  else {
    ((subroutineLevelHeader_t *)(TO_PCMEMPTR(currentPtrToPreviousLevel)))->ptrToNextLevel = TO_C47MEMPTR(currentSubroutineLevelData);
  }
}



bool_t validateName(const char *name) {
  if(stringGlyphLength(name)  > 7) {
    return false; // Given name is too long
  }
  else if(stringGlyphLength(name) == 0) {
    return false; // Given name is empty
  }

  // Check for the 1st character
  if(                                          compareChar(name, STD_A                   ) < 0) {
    return false;
  }
  if(compareChar(name, STD_Z          ) > 0 && compareChar(name, STD_a                   ) < 0) {
    return false;
  }
  if(compareChar(name, STD_z          ) > 0 && compareChar(name, STD_A_GRAVE             ) < 0) {
    return false;
  }
  if(                                          compareChar(name, STD_CROSS               ) ==0) {
    return false;
  }
  if(                                          compareChar(name, STD_DIVIDE              ) ==0) {
    return false;
  }
  if(compareChar(name, STD_z_CARON    ) > 0 && compareChar(name, STD_iota_DIALYTIKA_TONOS) < 0) {
    return false;
  }
  if(compareChar(name, STD_sampi      ) > 0 && compareChar(name, STD_SUB_alpha           ) < 0) {
    return false;
  }
  if(compareChar(name, STD_SUB_mu     ) > 0 && compareChar(name, STD_SUP_a               ) < 0) {
    return false;
  }
  if(compareChar(name, STD_SUB_Z      ) > 0                                                   ) {
    return false;
  }

  // Check for the following characters
  for(name += (*name & 0x80) ? 2 : 1; *name != 0; name += (*name & 0x80) ? 2 : 1) {
    switch(*name) {
      case '+':
      case '-':
      case ':':
      case '/':
      case '^':
      case '(':
      case ')':
      case '=':
      case ';':
      case '|':
      case '!':
      case ' ': {
        return false;
      }
      default: {
        if(compareChar(name, STD_CROSS) == 0) {
          return false;
        }
    }
  }
  }

  return true;
}



bool_t isUniqueMenuName(const char *name) {

  // Built-in menu items
  for(uint32_t i = 0; i < LAST_ITEM; ++i) {
    switch(indexOfItems[i].status & CAT_STATUS) {
      case CAT_MENU: {
        if(compareString(name, indexOfItems[i].itemCatalogName, CMP_NAME) == 0) {
          return false;
        }
      }
    }
  }

  // User menus
  for(uint32_t i = 0; i < numberOfUserMenus; ++i) {
    if(compareString(name, userMenus[i].menuName, CMP_NAME) == 0) {
      return false;
    }
  }

  return true;
}



static calcRegister_t _findReservedVariable(const char *variableName) {
  #if defined VERBOSE_REGISTERS
    printStatus(0, "_findReservedVariable",force);
  #endif //VERBOSE_REGISTERS

  uint8_t len = stringGlyphLength(variableName);
  const struct reservedRegister *reg = lookupReservedVariableName(variableName, len);

  if (reg != NULL)
    return reg->reg;

  #if defined VERBOSE_REGISTERS
    printStatus(0, " ",force);
  #endif //VERBOSE_REGISTERS
  return INVALID_VARIABLE;
}



void allocateNamedVariable(const char *variableName, dataType_t dataType, uint16_t fullDataSizeInBlocks) {
  calcRegister_t regist;
  uint8_t len;

  if(stringGlyphLength(variableName) < 1 || stringGlyphLength(variableName) > 7) {
    #if defined(PC_BUILD)
      sprintf(errorMessage, "the name %s", variableName);
      moreInfoOnError("In function allocateNamedVariable:", errorMessage, "is incorrect! The length must be", "from 1 to 7 glyphs!");
    #endif // PC_BUILD
    return;
  }

  if(_findReservedVariable(variableName) != INVALID_VARIABLE) {
    displayCalcErrorMessage(ERROR_INVALID_NAME, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
    #if defined(PC_BUILD)
      sprintf(errorMessage, "the name %s", variableName);
      moreInfoOnError("In function allocateNamedVariable:", errorMessage, "clashes with a reserved variable!", NULL);
    #endif // PC_BUILD
    return;
  }

  if(!validateName(variableName)) {
    displayCalcErrorMessage(ERROR_INVALID_NAME, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
    #if defined(PC_BUILD)
      sprintf(errorMessage, "the name %s", variableName);
      moreInfoOnError("In function allocateNamedVariable:", errorMessage, "is incorrect! The name does not follow", "the naming convention!");
    #endif // PC_BUILD
    return;
  }

  if(numberOfNamedVariables == 0) { // First named variable
    if((allNamedVariables = allocC47Blocks(TO_BLOCKS(sizeof(namedVariableHeader_t))))) {
      numberOfNamedVariables = 1;

      regist = 0;
    }
    else { // unlikely but possible
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      return;
    }
  }
  else {
    regist = numberOfNamedVariables;
    if(regist == LAST_NAMED_VARIABLE - FIRST_NAMED_VARIABLE + 1) {
      displayCalcErrorMessage(ERROR_TOO_MANY_VARIABLES, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      #if defined(PC_BUILD)
        sprintf(errorMessage, "%d named variables!", LAST_NAMED_VARIABLE - FIRST_NAMED_VARIABLE + 1);
        moreInfoOnError("In function allocateNamedVariable:", "you can allocate up to", errorMessage, NULL);
      #endif // PC_BUILD
      return;
    }

    namedVariableHeader_t *origNamedVariables = allNamedVariables;
    if((allNamedVariables = reallocC47Blocks(allNamedVariables, TO_BLOCKS(sizeof(namedVariableHeader_t) * numberOfNamedVariables), TO_BLOCKS(sizeof(namedVariableHeader_t) * (numberOfNamedVariables + 1))))) {
      numberOfNamedVariables++;
    }
    else {
      allNamedVariables = origNamedVariables;
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      return;
    }
  }

  #if defined VERBOSE_REGISTERS
    printStatus(0, "allocateNamedVariable",force);
  #endif //VERBOSE_REGISTERS
  len = stringByteLength(variableName);
  allNamedVariables[regist].variableName[0] = len;
  // Ensure that we terminate with \0 in the string to make in place comparisons easier
  memset(allNamedVariables[regist].variableName + 1, 0, 15);
  xcopy(allNamedVariables[regist].variableName + 1, variableName, len);

  regist += FIRST_NAMED_VARIABLE;
  setRegisterDataType(regist, dataType, amNone);
  setRegisterDataPointer(regist, allocC47Blocks(fullDataSizeInBlocks));
  #if defined VERBOSE_REGISTERS
    printStatus(0, " ",force);
  #endif //VERBOSE_REGISTERS
}



calcRegister_t findNamedVariable(const char *variableName) {
  calcRegister_t regist = INVALID_VARIABLE;
  uint8_t len = stringGlyphLength(variableName);
  if(len < 1 || len > 7) {
    return regist;
  }

  regist = _findReservedVariable(variableName);
  if(regist != INVALID_VARIABLE) {
    return regist;
  }

  #if defined VERBOSE_REGISTERS
    printStatus(0, "findNamedVariable",force);
  #endif //VERBOSE_REGISTERS
  //printf("|%20s|%20s|\n",(char *)(allNamedVariables[0].variableName + 1), variableName);
  for(int i = 0; i < numberOfNamedVariables; i++) {
    if(compareString((char *)(allNamedVariables[i].variableName + 1), variableName, CMP_NAME) == 0) {
      regist = i + FIRST_NAMED_VARIABLE;
      break;
    }
  }
  #if defined VERBOSE_REGISTERS
    printStatus(0, " ",force);
  #endif //VERBOSE_REGISTERS
  return regist;
}



calcRegister_t findOrAllocateNamedVariable(const char *variableName) {
  calcRegister_t regist = INVALID_VARIABLE;
  uint8_t len = stringGlyphLength(variableName);
  if(len < 1 || len > 7) {
    return regist;
  }
  regist = findNamedVariable(variableName);
  if(regist == INVALID_VARIABLE && numberOfNamedVariables <= (LAST_NAMED_VARIABLE - FIRST_NAMED_VARIABLE)) {
    allocateNamedVariable(variableName, dtReal34, REAL34_SIZE_IN_BLOCKS);
    if(lastErrorCode == ERROR_NONE) {
      // New variables are zero by default - although this might be immediately overridden, it might require an
      // initial value, such as when STO+
      regist = FIRST_NAMED_VARIABLE + numberOfNamedVariables - 1;
      real34Zero(REGISTER_REAL34_DATA(regist));
    }
    else {
      // Failed attempt to allocate a new named variable: there is not enough memory or the name is invalid.
      // It is impossible to reach the limitation of number of named variables.
      return INVALID_VARIABLE;
    }
  }
  return regist;
}



void fnDeleteVariable(uint16_t regist) {
  #if defined VERBOSE_REGISTERS
    printStatus(0, "fnDeleteVariable",force);
  #endif //VERBOSE_REGISTERS
  if(regist >= FIRST_NAMED_VARIABLE && regist < (FIRST_NAMED_VARIABLE + numberOfNamedVariables)) {
    removeUserItemAssignments(ITM_RCL,(char *)allNamedVariables[regist - FIRST_NAMED_VARIABLE].variableName+1);   // Remove assignments before deleting the variable
    freeRegisterData(regist);
    for(uint16_t i = (regist - FIRST_NAMED_VARIABLE); i < (numberOfNamedVariables - 1); ++i) {
      allNamedVariables[i] = allNamedVariables[i + 1];
    }
    allNamedVariables[numberOfNamedVariables - 1].header.descriptor = 0;
    allNamedVariables[numberOfNamedVariables - 1].variableName[0] = 0;
    allNamedVariables[numberOfNamedVariables - 1].variableName[1] = 0;
    reduceC47Blocks(allNamedVariables, TO_BLOCKS(sizeof(namedVariableHeader_t) * numberOfNamedVariables), TO_BLOCKS(sizeof(namedVariableHeader_t) * (numberOfNamedVariables - 1)));
    numberOfNamedVariables -= 1;
  }
  else if(regist >= FIRST_NAMED_VARIABLE && regist < LAST_NAMED_VARIABLE) {
    displayCalcErrorMessage(ERROR_UNDEF_SOURCE_VAR, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
  }
  else {
    displayCalcErrorMessage(ERROR_CANNOT_DELETE_PREDEF_ITEM, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
  }
  #if defined VERBOSE_REGISTERS
    printStatus(0, " ",force);
  #endif //VERBOSE_REGISTERS
}


void fnDeleteAllVariables(uint16_t confirmation) {
  if((confirmation == NOT_CONFIRMED) && (programRunStop != PGM_RUNNING)) {
    setConfirmationMode(fnDeleteAllVariables);
  }
  else {
    for(uint16_t var = numberOfNamedVariables; var > 0; var--) {  // Remove all user variables and user variables assignments
      fnDeleteVariable(FIRST_NAMED_VARIABLE + var -1);
    }
    initSimEqMatABX();
    if(programRunStop != PGM_RUNNING) {
      temporaryInformation = TI_DEL_ALL_VARIABLES;
    }
    else {
      temporaryInformation = TI_NO_INFO;
    }
  }
}


void fnClearAllVariables(uint16_t confirmation) {
  if((confirmation == NOT_CONFIRMED) && (programRunStop != PGM_RUNNING)) {
    setConfirmationMode(fnClearAllVariables);
  }
  else {
    for(uint16_t i = numberOfNamedVariables; i > 0; i--) {  // Clear all user variables
      if((compareString((char *)(allNamedVariables[i].variableName + 1), "STATS", CMP_NAME) != 0) &&
         (compareString((char *)(allNamedVariables[i].variableName + 1), "HISTO", CMP_NAME) != 0) &&
         (compareString((char *)(allNamedVariables[i].variableName + 1), "Mat_A", CMP_NAME) != 0) &&
         (compareString((char *)(allNamedVariables[i].variableName + 1), "Mat_B", CMP_NAME) != 0) &&
         (compareString((char *)(allNamedVariables[i].variableName + 1), "Mat_X", CMP_NAME) != 0))
      clearRegister(FIRST_NAMED_VARIABLE + i -1);
    }
    fnClSigma(CONFIRMED);                // Clear and release the memory of all statistical sums
    calcRegister_t regist;               // Clear SIM EQ Mat_A, Mat_B & Mat_X
    regist = findOrAllocateNamedVariable("Mat_A");
    if(regist != INVALID_VARIABLE) {
      initMatrixRegister(regist, 1, 1, false);
    }
    regist = findOrAllocateNamedVariable("Mat_B");
    if(regist != INVALID_VARIABLE) {
      initMatrixRegister(regist, 1, 1, false);
    }
    regist = findOrAllocateNamedVariable("Mat_X");
    if(regist != INVALID_VARIABLE) {
      initMatrixRegister(regist, 1, 1, false);
    }
    if(programRunStop != PGM_RUNNING) {
      temporaryInformation = TI_CLEAR_ALL_VARIABLES;
    }
    else {
      temporaryInformation = TI_NO_INFO;
    }
  }
}


void setRegisterMaxDataLengthInBlocks(calcRegister_t regist, uint16_t maxDataLen) {
  if(regist <= LAST_GLOBAL_REGISTER) { // Global register
    ((strLgIntHeader_t *)TO_PCMEMPTR(globalRegister[regist].pointerToRegisterData))->dataMaxLengthInBlocks = maxDataLen;
  }

  else if(regist <= LAST_NAMED_VARIABLE) { // Named variable
    if(numberOfNamedVariables > 0) {
      if(regist - FIRST_NAMED_VARIABLE < numberOfNamedVariables) {
        ((strLgIntHeader_t *)getRegisterDataPointer(regist))->dataMaxLengthInBlocks = maxDataLen;
      }
      else {
        sprintf(errorMessage, commonBugScreenMessages[bugMsgNotDefinedMustBe], "setRegisterMaxDataLengthInBlocks", "named variable", (uint16_t)(regist - FIRST_NAMED_VARIABLE), (uint16_t)(numberOfNamedVariables - 1));
        displayBugScreen(errorMessage);
      }
    }
    #if defined(PC_BUILD)
      else {
        moreInfoOnError("In function setRegisterMaxDataLengthInBlocks:", "no named variables defined!", NULL, NULL);
      }
    #endif // PC_BUILD
  }

  else if(regist <= LAST_RESERVED_VARIABLE) { // System named variable
    regist -= FIRST_RESERVED_VARIABLE;
    ((strLgIntHeader_t *)getRegisterDataPointer(regist))->dataMaxLengthInBlocks = maxDataLen;
  }

  else if(regist <= LAST_LOCAL_REGISTER) { // Local register
    if(currentLocalRegisters != NULL) {
      if(regist-FIRST_LOCAL_REGISTER < currentNumberOfLocalRegisters) {
        ((strLgIntHeader_t *)getRegisterDataPointer(regist))->dataMaxLengthInBlocks = maxDataLen;
      }
      #if defined(PC_BUILD)
        else {
          sprintf(errorMessage, "local register %" PRId16 " is not defined!", (uint16_t)(regist - FIRST_LOCAL_REGISTER));
          sprintf(errorMessage + ERROR_MESSAGE_LENGTH/2, "Must be from 0 to %" PRIu8, (uint8_t)(currentNumberOfLocalRegisters - 1));
          moreInfoOnError("In function setRegisterMaxDataLengthInBlocks:", errorMessage, errorMessage + ERROR_MESSAGE_LENGTH/2, NULL);
        }
      #endif // PC_BUILD
    }
    #if defined(PC_BUILD)
      else {
       moreInfoOnError("In function setRegisterMaxDataLengthInBlocks:", "no local registers defined!", NULL, NULL);
      }
    #endif // PC_BUILD
  }

  else {
    sprintf(errorMessage, commonBugScreenMessages[bugMsgRegistMustBeLessThan], "setRegisterMaxDataLengthInBlocks", regist, LAST_RESERVED_VARIABLE + 1);
    displayBugScreen(errorMessage);
  }
}



uint16_t getRegisterMaxDataLengthInBlocks(calcRegister_t regist) {
  void *db = NULL;

  if(regist <= LAST_GLOBAL_REGISTER) { // Global register
      db = TO_PCMEMPTR(globalRegister[regist].pointerToRegisterData);
  }

  else if(regist <= LAST_NAMED_VARIABLE) { // Named variable
    if(numberOfNamedVariables != 0) {
      regist -= FIRST_NAMED_VARIABLE;
      if(regist < numberOfNamedVariables) {
        db = TO_PCMEMPTR(allNamedVariables[regist].header.pointerToRegisterData);
      }
      else {
        sprintf(errorMessage, commonBugScreenMessages[bugMsgNotDefinedMustBe], "getRegisterMaxDataLengthInBlocks", "named variable", regist, (uint16_t)(numberOfNamedVariables - 1));
        displayBugScreen(errorMessage);
      }
    }
    #if defined(PC_BUILD)
      else {
        moreInfoOnError("In function getRegisterMaxDataLengthInBlocks:", "no named variables defined!", NULL, NULL);
      }
    #endif // PC_BUILD
  }

  else if(regist <= LAST_RESERVED_VARIABLE) { // System named variable
    regist -= FIRST_RESERVED_VARIABLE;
    db = TO_PCMEMPTR(allReservedVariables[regist].header.pointerToRegisterData);
  }

  else if(regist <= LAST_LOCAL_REGISTER) { // Local register
    if(currentLocalRegisters != NULL) {
      if(regist-FIRST_LOCAL_REGISTER < currentNumberOfLocalRegisters) {
        db = TO_PCMEMPTR(POINTER_TO_LOCAL_REGISTER(regist-FIRST_LOCAL_REGISTER)->pointerToRegisterData);
      }
      else {
        sprintf(errorMessage, commonBugScreenMessages[bugMsgNotDefinedMustBe], "getRegisterMaxDataLengthInBlocks", "local register", (uint16_t)(regist - FIRST_LOCAL_REGISTER), (uint8_t)(currentNumberOfLocalRegisters - 1));
        displayBugScreen(errorMessage);
      }
    }
    #if defined(PC_BUILD)
      else {
        moreInfoOnError("In function getRegisterMaxDataLengthInBlocks:", "no local registers defined!", NULL, NULL);
      }
    #endif // PC_BUILD
  }

  else {
    sprintf(errorMessage, commonBugScreenMessages[bugMsgRegistMustBeLessThan], "getRegisterMaxDataLengthInBlocks", regist, LAST_RESERVED_VARIABLE + 1);
    displayBugScreen(errorMessage);
  }

  if(db) {
    if(getRegisterDataType(regist) == dtReal34Matrix) {
      return ((matrixHeader_t *)db)->matrixRows * ((matrixHeader_t *)db)->matrixColumns * REAL34_SIZE_IN_BLOCKS;
    }
    else if(getRegisterDataType(regist) == dtComplex34Matrix) {
      return ((matrixHeader_t *)db)->matrixRows * ((matrixHeader_t *)db)->matrixColumns * COMPLEX34_SIZE_IN_BLOCKS;
    }
    else {
      return ((strLgIntHeader_t *)db)->dataMaxLengthInBlocks;
    }
  }
  return 0;
}



uint16_t getRegisterFullSizeInBlocks(calcRegister_t regist) {
  switch(getRegisterDataType(regist)) {
    case dtLongInteger: {
      return REGISTER_STRING_HEADER(regist)->dataMaxLengthInBlocks + TO_BLOCKS(sizeof(strLgIntHeader_t));
    }
    case dtTime: {
      return REAL34_SIZE_IN_BLOCKS;
    }
    case dtDate: {
      return REAL34_SIZE_IN_BLOCKS;
    }
    case dtString: {
      return REGISTER_LONG_INTEGER_HEADER(regist)->dataMaxLengthInBlocks + TO_BLOCKS(sizeof(strLgIntHeader_t));
    }
    case dtReal34Matrix: {
      return TO_BLOCKS((REGISTER_MATRIX_HEADER(regist)->matrixRows * REGISTER_MATRIX_HEADER(regist)->matrixColumns) * REAL34_SIZE_IN_BYTES + sizeof(matrixHeader_t)); break;
    }
    case dtComplex34Matrix: {
      return TO_BLOCKS((REGISTER_MATRIX_HEADER(regist)->matrixRows * REGISTER_MATRIX_HEADER(regist)->matrixColumns) * COMPLEX34_SIZE_IN_BYTES + sizeof(matrixHeader_t)); break;
    }
    case dtShortInteger: {
      return SHORT_INTEGER_SIZE_IN_BLOCKS;
    }
    case dtReal34: {
      return REAL34_SIZE_IN_BLOCKS;
    }
    case dtComplex34: {
      return COMPLEX34_SIZE_IN_BLOCKS;
    }
    case dtConfig: {
      return CONFIG_SIZE_IN_BLOCKS;
    }
    default: {
      sprintf(errorMessage, commonBugScreenMessages[bugMsgDataTypeUnknown], "getRegisterFullSizeInBlocks", getDataTypeName(getRegisterDataType(regist), false, false));
      displayBugScreen(errorMessage);
      return 0;
    }
  }
}



void clearRegister(calcRegister_t regist) {
  if((lastIntegerBase == 0) && (Input_Default == ID_43S || Input_Default == ID_DP)) {                       //JM defaults JMZERO
    if(getRegisterDataType(regist) == dtReal34) {
      real34Zero(REGISTER_REAL34_DATA(regist));
      setRegisterTag(regist, amNone);
    }
    else{
      reallocateRegister(regist, dtReal34, 0, amNone);
      real34Zero(REGISTER_REAL34_DATA(regist));
    }
  }                                                                             //JM defaults ^^
  else if((lastIntegerBase == 0) && (Input_Default == ID_CPXDP)) {                                          //JM defaults vv
    if(getRegisterDataType(regist) == dtComplex34) {
      real34Zero(REGISTER_REAL34_DATA(regist));
      real34Zero(REGISTER_IMAG34_DATA(regist));
      if(getSystemFlag(FLAG_POLAR)) {
        setRegisterTag(regist, currentAngularMode | amPolar);
      }
      else {
        setRegisterTag(regist, amNone);
      }
    }
    else{
      if(getSystemFlag(FLAG_POLAR)) {
        reallocateRegister(regist, dtComplex34, 0, currentAngularMode | amPolar);
      }
      else {
        reallocateRegister(regist, dtComplex34, 0, amNone);
      }
      real34Zero(REGISTER_REAL34_DATA(regist));
      real34Zero(REGISTER_IMAG34_DATA(regist));
    }
  }                                                                             //JM defaults ^^
  else if((lastIntegerBase == 0) && (Input_Default == ID_LI)) {                                             //JM defaults vv
    //JM comment: Not checking if already the correct type, just changing it. Wasting some steps.
    longInteger_t lgInt;
    longIntegerInit(lgInt);
    uint16_t val =0;
    uInt32ToLongInteger(val, lgInt);
    convertLongIntegerToLongIntegerRegister(lgInt, regist);
    longIntegerFree(lgInt);
  }                                                                             //JM defaults ^^
  else if(lastIntegerBase !=0) {                                             //JM defaults vv
    //JM comment: Not checking if already the correct type, just changing it. Wasting some steps.
    longInteger_t lgInt;
    longIntegerInit(lgInt);
    uint16_t val =0;
    uInt32ToLongInteger(val, lgInt);
    convertLongIntegerToShortIntegerRegister(lgInt, lastIntegerBase == 0 ? 10:lastIntegerBase, regist);
    longIntegerFree(lgInt);
  }                                                                             //JM defaults ^^
}



void fnClearRegisters(uint16_t confirmation) {
  if((confirmation == NOT_CONFIRMED) && (programRunStop != PGM_RUNNING)) {
    setConfirmationMode(fnClearRegisters);
  }
  else {
    calcRegister_t regist;

    for(regist=0; regist<REGISTER_X; regist++) {
      clearRegister(regist);
    }

    for(regist=0; regist<currentNumberOfLocalRegisters; regist++) {
      clearRegister(FIRST_LOCAL_REGISTER + regist);
    }

    if(!getSystemFlag(FLAG_SSIZE8)) { // Stack size = 4
      for(regist=REGISTER_A; regist<=REGISTER_D; regist++) {
        clearRegister(regist);
      }
    }

    for(regist=REGISTER_I; regist<=REGISTER_W; regist++) {
      clearRegister(regist);
    }
  }
}



void fnGetLocR(uint16_t unusedButMandatoryParameter) {
  longInteger_t locR;

  liftStack();

  longIntegerInit(locR);
  uInt32ToLongInteger(currentNumberOfLocalRegisters, locR);
  convertLongIntegerToLongIntegerRegister(locR, REGISTER_X);
  longIntegerFree(locR);
}


/* Given a real register/value, check for NaNs, infinities and sign correct zeroes */
static void adjustRealRegister(calcRegister_t reg, real34_t *val) {
  if(real34IsInfinite(val)) {
    displayCalcErrorMessage(real34IsPositive(val) ? ERROR_OVERFLOW_PLUS_INF : ERROR_OVERFLOW_MINUS_INF , ERR_REGISTER_LINE, reg);
  }
  else if(0 && real34IsNaN(val)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, reg);
  }
  else if(real34IsZero(val)) {
    real34SetPositiveSign(val);
  }
}

void adjustResult(calcRegister_t res, bool_t dropY, bool_t setCpxRes, calcRegister_t op1, calcRegister_t op2, calcRegister_t op3) {
  uint32_t resultDataType;
  bool_t oneArgumentIsComplex = false;

  if(op1 >= 0) {
    oneArgumentIsComplex = oneArgumentIsComplex || getRegisterDataType(op1) == dtComplex34 || getRegisterDataType(op1) == dtComplex34Matrix;
  }

  if(op2 >= 0) {
    oneArgumentIsComplex = oneArgumentIsComplex || getRegisterDataType(op2) == dtComplex34 || getRegisterDataType(op2) == dtComplex34Matrix;
  }

  if(op3 >= 0) {
    oneArgumentIsComplex = oneArgumentIsComplex || getRegisterDataType(op3) == dtComplex34 || getRegisterDataType(op3) == dtComplex34Matrix;
  }

  resultDataType = getRegisterDataType(res);
  if(getSystemFlag(FLAG_SPCRES) == false && lastErrorCode == 0) {
    switch(resultDataType) {
      case dtReal34:
      case dtTime:
      case dtDate: {
        adjustRealRegister(res, REGISTER_REAL34_DATA(res));
        break;
      }

      case dtComplex34: {
        adjustRealRegister(res, REGISTER_REAL34_DATA(res));
        adjustRealRegister(res, REGISTER_IMAG34_DATA(res));
        break;
      }

      case dtReal34Matrix: {
        real34Matrix_t matrix;
        linkToRealMatrixRegister(res, &matrix);
        for(uint32_t i = 0; i < matrix.header.matrixRows * matrix.header.matrixColumns; i++) {
          adjustRealRegister(res, VARIABLE_REAL34_DATA(&matrix.matrixElements[i]));
        }
        break;
      }

      case dtComplex34Matrix: {
        complex34Matrix_t matrix;
        linkToComplexMatrixRegister(res, &matrix);
        for(uint32_t i = 0; i < matrix.header.matrixRows * matrix.header.matrixColumns; i++) {
          adjustRealRegister(res, VARIABLE_REAL34_DATA(&matrix.matrixElements[i]));
          adjustRealRegister(res, VARIABLE_IMAG34_DATA(&matrix.matrixElements[i]));
        }
        break;
      }

      default: {
        break;
    }
  }
  }

  if(lastErrorCode == 0) {
    if(resultDataType == dtTime) {
      checkTimeRange(REGISTER_REAL34_DATA(res));
    }
    if(resultDataType == dtDate) {
      checkDateRange(REGISTER_REAL34_DATA(res));
    }
  }

  if(lastErrorCode != 0) {
    #if defined(TESTSUITE_BUILD)
      #if defined(DEBUGUNDO)
        printf(">>> undo from adjustResult\n");
      #endif // DEBUGUNDO
      undo();
    #endif //TESTSUITE_BUILD
    return;
  }

  if(setCpxRes && oneArgumentIsComplex && resultDataType != dtString) {
    fnSetFlag(FLAG_CPXRES);
    fnRefreshState();                                 //drJM
  }

  // Round the register value
  switch(resultDataType) {
    real_t tmp;

    case dtReal34: {
      if(significantDigits == 0 || significantDigits >= 34) {
        break;
      }

      real34ToReal(REGISTER_REAL34_DATA(res), &tmp);
      convertRealToReal34ResultRegister(&tmp, res);
      break;
    }

    case dtComplex34: {
      if(significantDigits == 0 || significantDigits >= 34) {
        break;
      }

      real34ToReal(REGISTER_REAL34_DATA(res), &tmp);
      convertRealToReal34ResultRegister(&tmp, res);
      real34ToReal(REGISTER_IMAG34_DATA(res), &tmp);
      convertRealToImag34ResultRegister(&tmp, res);
      break;
    }

    case dtReal34Matrix: {
      if(significantDigits == 0 || significantDigits >= 34) {
        break;
      }

      rsdRema(significantDigits);
      break;
    }

    case dtComplex34Matrix: {
      if(significantDigits == 0 || significantDigits >= 34) {
        break;
      }

      rsdCxma(significantDigits);
      break;
    }

    default: {
      break;
    }
  }

  if(dropY) {
    fnDropY(NOPARAM);
  }
}



void copySourceRegisterToDestRegister(calcRegister_t sourceRegister, calcRegister_t destRegister) {
  if(FIRST_RESERVED_VARIABLE <= destRegister && destRegister < FIRST_NAMED_RESERVED_VARIABLE) {
    destRegister = destRegister - FIRST_RESERVED_VARIABLE + REGISTER_X;
  }

  if(FIRST_RESERVED_VARIABLE <= sourceRegister && sourceRegister < FIRST_NAMED_RESERVED_VARIABLE) {
    sourceRegister = sourceRegister - FIRST_RESERVED_VARIABLE + REGISTER_X;
  }
  else if(sourceRegister == RESERVED_VARIABLE_ADM) {
    longInteger_t longIntVar;
    longIntegerInit(longIntVar);
    switch(currentAngularMode) {
      case amDMS: {
        uInt32ToLongInteger(1u, longIntVar);
        break;
      }
      case amRadian: {
        uInt32ToLongInteger(2u, longIntVar);
        break;
      }
      case amMultPi: {
        uInt32ToLongInteger(3u, longIntVar);
        break;
      }
      case amGrad: {
        uInt32ToLongInteger(4u, longIntVar);
        break;
      }
      default: {
        uInt32ToLongInteger(0u, longIntVar);
        break;
      }
    }
    convertLongIntegerToLongIntegerRegister(longIntVar, destRegister);
    longIntegerFree(longIntVar);
    return;
  }
  else if(sourceRegister == RESERVED_VARIABLE_DENMAX) {
    longInteger_t longIntVar;
    longIntegerInit(longIntVar);
    uInt32ToLongInteger(denMax, longIntVar);
    convertLongIntegerToLongIntegerRegister(longIntVar, destRegister);
    longIntegerFree(longIntVar);
    return;
  }
  else if(sourceRegister == RESERVED_VARIABLE_ISM) {
    longInteger_t longIntVar;
    longIntegerInit(longIntVar);
    int32ToLongInteger((shortIntegerMode==SIM_2COMPL ? 2 : (shortIntegerMode==SIM_1COMPL ? 1 : (shortIntegerMode==SIM_UNSIGN ? 0 : -1))), longIntVar);
    convertLongIntegerToLongIntegerRegister(longIntVar, destRegister);
    longIntegerFree(longIntVar);
    return;
  }
  else if(sourceRegister == RESERVED_VARIABLE_REALDF) {
    longInteger_t longIntVar;
    longIntegerInit(longIntVar);
    uInt32ToLongInteger(displayFormat, longIntVar);
    convertLongIntegerToLongIntegerRegister(longIntVar, destRegister);
    longIntegerFree(longIntVar);
    return;
  }
  else if(sourceRegister == RESERVED_VARIABLE_NDEC) {
    longInteger_t longIntVar;
    longIntegerInit(longIntVar);
    uInt32ToLongInteger(displayFormatDigits, longIntVar);
    convertLongIntegerToLongIntegerRegister(longIntVar, destRegister);
    longIntegerFree(longIntVar);
    return;
  }

  if(   getRegisterDataType(destRegister) != getRegisterDataType(sourceRegister)
    || getRegisterFullSizeInBlocks(destRegister) != getRegisterFullSizeInBlocks(sourceRegister)) {
    uint32_t sizeInBlocks;

    switch(getRegisterDataType(sourceRegister)) {
      case dtLongInteger:     sizeInBlocks = REGISTER_LONG_INTEGER_HEADER(sourceRegister)->dataMaxLengthInBlocks;                                                                               break;
      case dtString:          sizeInBlocks = REGISTER_STRING_HEADER(sourceRegister)->dataMaxLengthInBlocks;                                                                                     break;
      case dtReal34Matrix:    sizeInBlocks = TO_BLOCKS((REGISTER_MATRIX_HEADER(sourceRegister)->matrixRows * REGISTER_MATRIX_HEADER(sourceRegister)->matrixColumns) * REAL34_SIZE_IN_BYTES);    break;
      case dtComplex34Matrix: sizeInBlocks = TO_BLOCKS((REGISTER_MATRIX_HEADER(sourceRegister)->matrixRows * REGISTER_MATRIX_HEADER(sourceRegister)->matrixColumns) * COMPLEX34_SIZE_IN_BYTES); break;

      case dtTime:
      case dtDate:
      case dtShortInteger:
      case dtReal34:
      case dtComplex34:
      case dtConfig:          sizeInBlocks = 0;                                                                                                                                                 break;

      default: sprintf(errorMessage, commonBugScreenMessages[bugMsgDataTypeUnknown], "copySourceRegisterToDestRegister", getDataTypeName(getRegisterDataType(sourceRegister), false, false));
               displayBugScreen(errorMessage);
               sizeInBlocks = 0;
    }
    reallocateRegister(destRegister, getRegisterDataType(sourceRegister), sizeInBlocks, amNone);

    //busy checking all re-allocate to see if we can do a bit of fuzzy logic determination of POLAR/RECR

    if(lastErrorCode == ERROR_RAM_FULL) {
      return;
    }
  }

  switch(getRegisterDataType(sourceRegister)) {
    case dtReal34Matrix: {
      xcopy(REGISTER_MATRIX_HEADER(destRegister),
            REGISTER_MATRIX_HEADER(sourceRegister),
            sizeof(matrixHeader_t) + (REGISTER_MATRIX_HEADER(sourceRegister)->matrixRows * REGISTER_MATRIX_HEADER(sourceRegister)->matrixColumns) * REAL34_SIZE_IN_BYTES);
      break;
    }
    case dtComplex34Matrix: {
      xcopy(REGISTER_MATRIX_HEADER(destRegister),
            REGISTER_MATRIX_HEADER(sourceRegister),
            sizeof(matrixHeader_t) + (REGISTER_MATRIX_HEADER(sourceRegister)->matrixRows * REGISTER_MATRIX_HEADER(sourceRegister)->matrixColumns) * COMPLEX34_SIZE_IN_BYTES);
      break;
    }
    default: {
      xcopy(getRegisterDataPointer(destRegister),
            getRegisterDataPointer(sourceRegister),
            TO_BYTES(getRegisterFullSizeInBlocks(sourceRegister)));
  }
  }
  setRegisterTag(destRegister, getRegisterTag(sourceRegister));
}



int16_t indirectAddressing(calcRegister_t regist, uint16_t parameterType, int16_t minValue, int16_t maxValue, bool_t tryAllocate) {
  int16_t value;
  bool_t isValidAlpha = false;
  #if defined(PC_BUILD)
    printf("parameterType %u\n", parameterType); fflush(stdout);
    printf("currentNumberOfLocalFlags %u\n", currentNumberOfLocalFlags); fflush(stdout);
  #endif   // PC_BUILD

  switch(parameterType) {
    case INDPM_REGISTER: {
    // Temorarily assign the maximum value to the maximum register
    // We need to do better range checking later
      maxValue = FIRST_NAMED_VARIABLE + numberOfNamedVariables - 1;
      break;
    }
    case INDPM_FLAG: {
    // Temorarily assign the maximum value to the last additional global flag
    // We need to do better range checking later for the gap between the last local flag (143) and the first additional global flag (211)
      //maxValue = NUMBER_OF_GLOBAL_FLAGS + currentNumberOfLocalFlags - 1;
      maxValue = FLAG_W;
      break;
    }
    case INDPM_LABEL: {
      maxValue = LAST_LOCAL_LABEL;
      break;
    }
  }

  if(regist >= FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters &&
     (regist < FIRST_NAMED_VARIABLE ||
        regist >= FIRST_NAMED_VARIABLE + numberOfNamedVariables)) {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if defined(PC_BUILD)
      sprintf(errorMessage, "local indirection register .%02d", regist - FIRST_LOCAL_REGISTER);
      moreInfoOnError("In function indirectAddressing:", errorMessage, "is not defined!", NULL);
    #endif // PC_BUILD
    return FAILED_INDIRECTION;
  }

  else if(getRegisterDataType(regist) == dtReal34 && parameterType != INDPM_MENU) {
    real34_t maxValue34plusOne;

    int32ToReal34(maxValue+1, &maxValue34plusOne);
    if(real34CompareLessThan(REGISTER_REAL34_DATA(regist), const34_0) || real34CompareGreaterEqual(REGISTER_REAL34_DATA(regist), &maxValue34plusOne)) {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
      #if defined(PC_BUILD)
        real34ToString(REGISTER_REAL34_DATA(regist), errorMessage);
        sprintf(tmpString, "register %" PRId16 " = %s:", regist, errorMessage);
        moreInfoOnError("In function indirectAddressing:", tmpString, "this value is negative or too big!", NULL);
      #endif // PC_BUILD
      return FAILED_INDIRECTION;
    }
    value = real34ToInt32(REGISTER_REAL34_DATA(regist));
  }

  else if(getRegisterDataType(regist) == dtLongInteger && parameterType != INDPM_MENU) {
    longInteger_t lgInt;

    convertLongIntegerRegisterToLongInteger(regist, lgInt);
    if(longIntegerIsNegative(lgInt) || longIntegerCompareUInt(lgInt, maxValue) > 0) {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
      #if defined(PC_BUILD)
        longIntegerToAllocatedString(lgInt, errorMessage, ERROR_MESSAGE_LENGTH);
        sprintf(tmpString, "register %" PRId16 " = %s:", regist, errorMessage);
        moreInfoOnError("In function indirectAddressing:", tmpString, "this value is negative or too big!", NULL);
      #endif // PC_BUILD
      longIntegerFree(lgInt);
      return FAILED_INDIRECTION;
    }
    longIntegerToUInt32(lgInt, value);
    longIntegerFree(lgInt);
  }

  else if(getRegisterDataType(regist) == dtShortInteger && parameterType != INDPM_MENU) {
    uint64_t val;
    int16_t sign;

    convertShortIntegerRegisterToUInt64(regist, &sign, &val);
    if(sign == 1 || val > 180) {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
      #if defined(PC_BUILD)
        shortIntegerToDisplayString(regist, errorMessage, false, noBaseOverride);
        sprintf(tmpString, "register %" PRId16 " = %s:", regist, errorMessage);
        moreInfoOnError("In function indirectAddressing:", tmpString, "this value is negative or too big!", NULL);
      #endif // PC_BUILD
      return FAILED_INDIRECTION;
    }
    value = val;
  }

  else if(getRegisterDataType(regist) == dtString && parameterType == INDPM_REGISTER) {
    value = (tryAllocate ? findOrAllocateNamedVariable(REGISTER_STRING_DATA(regist)) : findNamedVariable(REGISTER_STRING_DATA(regist)));
    isValidAlpha = true;
    if((value == INVALID_VARIABLE) && (lastErrorCode != ERROR_ENTER_NEW_NAME)) {
      displayCalcErrorMessage(ERROR_UNDEF_SOURCE_VAR, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "string '%s' is not a named variable - tryAllocate is %s", REGISTER_STRING_DATA(regist),(tryAllocate? "true" : "false"));
        moreInfoOnError("In function indirectAddressing:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return FAILED_INDIRECTION;
    }
  }

  else if(getRegisterDataType(regist) == dtString && parameterType == INDPM_LABEL) {
    value = findNamedLabel(REGISTER_STRING_DATA(regist));
    isValidAlpha = true;
    if(value == INVALID_VARIABLE) {
      displayCalcErrorMessage(ERROR_LABEL_NOT_FOUND, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "string '%s' is not a named label", REGISTER_STRING_DATA(regist));
        moreInfoOnError("In function indirectAddressing:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return FAILED_INDIRECTION;
    }
  }

  else if(getRegisterDataType(regist) == dtString && parameterType == INDPM_MENU) {
    value = findMenu(REGISTER_STRING_DATA(regist));
    isValidAlpha = true;
    if(value == INVALID_MENU) {
      displayCalcErrorMessage(ERROR_UNDEF_MENU, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "string '%s' is not a menu name", REGISTER_STRING_DATA(regist));
        moreInfoOnError("In function indirectAddressing:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return FAILED_INDIRECTION;
    }
  }

  else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
    #if defined(PC_BUILD)
      sprintf(errorMessage, "register %" PRId16 " is %s:", regist, getRegisterDataTypeName(regist, true, false));
      moreInfoOnError("In function indirectAddressing:", errorMessage, "not suited for indirect addressing!", NULL);
    #endif // PC_BUILD
    return FAILED_INDIRECTION;
  }

  if(minValue <= value && (value <= maxValue || isValidAlpha)) {
    if(parameterType == INDPM_REGISTER) {
      if(value <= LAST_SPARE_REGISTERS_IN_KS_CODE) { // Global register from 00 to 99, Lettered register from X to W, or Local register from .00 to .98
        value = regKStoC(value);
      }
    }
    else if((parameterType == INDPM_FLAG) && (value > LAST_LOCAL_FLAG) && (value < FLAG_M)) {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
      #if defined(PC_BUILD)
        sprintf(errorMessage, "local flag value = %d! Should be from %d to %d", value, FIRST_LOCAL_FLAG, LAST_LOCAL_FLAG);
        moreInfoOnError("In function indirectAddressing:", errorMessage, NULL, NULL);
      #endif // PC_BUILD
      return FAILED_INDIRECTION;
    }
    return value;
  }
  else {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if defined(PC_BUILD)
      sprintf(errorMessage, "value = %d! Should be from %d to %d.", value, minValue, maxValue);
      moreInfoOnError("In function indirectAddressing:", errorMessage, NULL, NULL);
    #endif // PC_BUILD
    return FAILED_INDIRECTION;
  }
}



#if !defined(DMCP_BUILD)
  void printRegisterToConsole(calcRegister_t regist, const char *before, const char *after) {
    char str[3000];

    if(getRegisterDataType(regist) == dtReal34) {
      real34ToString(REGISTER_REAL34_DATA(regist), str);
      printf("%s", before);
      printf("real34 %s %s", str, getAngularModeName(getRegisterAngularMode(regist)));
    }

    else if(getRegisterDataType(regist) == dtComplex34) {
      real34ToString(REGISTER_REAL34_DATA(regist), str);
      printf("%s", before);
      printf("complex34 %s ", str);

      real34ToString(REGISTER_IMAG34_DATA(regist), str);
      if(real34IsNegative(REGISTER_IMAG34_DATA(regist))) {
        printf("%s", before);
        printf("- ix%s", str + 1);
      }
      else {
        printf("%s", before);
        printf("+ ix%s", str);
      }
    }

    else if(getRegisterDataType(regist) == dtString) {
      stringToUtf8(REGISTER_STRING_DATA(regist), (uint8_t *)str);
      printf("%s", before);
      printf("string (%" PRIu32 " + %" PRIu32 " bytes) |%s|", (uint32_t)sizeof(strLgIntHeader_t), TO_BYTES(getRegisterMaxDataLengthInBlocks(regist)), str);
    }

    else if(getRegisterDataType(regist) == dtShortInteger) {
      uint64_t value = *(REGISTER_SHORT_INTEGER_DATA(regist));
      printf("%s", before);
      printf("short integer %08x-%08x (base %" PRIu32 ")", (unsigned int)(value>>32), (unsigned int)(value&0xffffffff), getRegisterTag(regist));
    }

    else if(getRegisterDataType(regist) == dtConfig) {
      printf("%s", before);
      printf("Configuration data");
    }

    else if(getRegisterDataType(regist) == dtLongInteger) {
      longInteger_t lgInt;

      convertLongIntegerRegisterToLongInteger(regist, lgInt);
      longIntegerToAllocatedString(lgInt, str, sizeof(str));
      longIntegerFree(lgInt);
      printf("%s", before);
      printf("long integer (%" PRIu32 " + %" PRIu32 " bytes) %s", (uint32_t)sizeof(strLgIntHeader_t), TO_BYTES(getRegisterMaxDataLengthInBlocks(regist)), str);
    }

    else if(getRegisterDataType(regist) == dtTime) {
      real34ToString(REGISTER_REAL34_DATA(regist), str);
      printf("%s", before);
      printf("time %s", str);
    }

    else if(getRegisterDataType(regist) == dtDate) {
      real34ToString(REGISTER_REAL34_DATA(regist), str);
      printf("%s", before);
      printf("date %s", str);
    }

    else if(getRegisterDataType(regist) == dtReal34Matrix) {
      uint16_t r, c;
      real34Matrix_t mat;
      linkToRealMatrixRegister(regist, &mat);
      for(r = 0; r < mat.header.matrixRows; ++r) {
        printf("ReMa %s", before);
        printf("Row %3i: ",r);
        for(c = 0; c < mat.header.matrixColumns; ++c) {
          real34ToString(&mat.matrixElements[r * mat.header.matrixColumns + c], str);
          printf("%s ", str);
        }
        printf("\n");
      }
    }

    else if(getRegisterDataType(regist) == dtComplex34Matrix) {
      uint16_t r, c;
      complex34Matrix_t mat;
      linkToComplexMatrixRegister(regist, &mat);
      for(r = 0; r < mat.header.matrixRows; ++r) {
        printf("(compact CxMa): %s", before);
        printf("Row %3i: ",r);
        for(c = 0; c < mat.header.matrixColumns; ++c) {
          real_t tmpr;
          char str[100];
          uint32_t offset = (r * mat.header.matrixRows + c) * 2;
          real34ToReal(&mat.matrixElements + offset, &tmpr);
          realPlus(&tmpr, &tmpr, &ctxtReal4);       // Real part
          if (realGetExponent(&tmpr) < -50)
            printf("[≈0 ");
          else {
            realToString(&tmpr, str);
            if (strstr(str, "Infinity")) {
                {char tmp[256]; strcpy(tmp, str); char *p = tmp; char *q = str;
                 while ((p = strstr(p, "Infinity"))) {
                     *p = 0;
                     q += sprintf(q, "%s∞", tmp);
                     p += 8;
                     strcpy(tmp, p);
                 }
                 strcpy(q, tmp);
                }
            }
            printf("[%s", str);
          }
          real34ToReal(&mat.matrixElements + offset + 1, &tmpr);
          realPlus(&tmpr, &tmpr, &ctxtReal4);       // Imag part
          if (realGetExponent(&tmpr) < -50)
            printf(" i≈0] ");
          else {
            realToString(&tmpr, str);
            if (strstr(str, "Infinity")) {
                {char tmp[256]; strcpy(tmp, str); char *p = tmp; char *q = str;
                 while ((p = strstr(p, "Infinity"))) {
                     *p = 0;
                     q += sprintf(q, "%s∞", tmp);
                     p += 8;
                     strcpy(tmp, p);
                 }
                 strcpy(q, tmp);
                }
            }
            printf(" i%s] ", str);
          }
        }
        printf("\n");
      }
    }




    else {
      printf("%s", before);
      sprintf(errorMessage, "In printRegisterToConsole: data type %s not supported", getRegisterDataTypeName(regist ,false, false));
      displayBugScreen(errorMessage);
    }

    printf("%s", after);
  }


  void printStringToConsole(const char *str, const char *before, const char *after) {
    uint16_t loop = 0;
    printf("%s", before);
    char s2[2000];

    stringToASCII(str,s2);
    printf("\"%s\"",s2);

    for(int16_t ii=0; ii<stringByteLength(s2) && ii<40; ii++) {
      printf(" ");
    }
    while(str[loop] != 0) {
      if(str[loop] & 0x80) {
        printf(" %2x%2x",(uint8_t)(str[loop]),(uint8_t)(str[loop+1]));
        loop++;
        loop++;
      }
      else {
        printf("   %2x",(uint8_t)(str[loop++]));
      }
    }
    printf("%s", after);
  }



  void printReal34ToConsole(const real34_t *value, const char *before, const char *after) {
    char str[100];

    real34ToString(value, str);
    printf("%sreal34 %s%s", before, str, after);
  }



  void printRealToConsole(const real_t *value, const char *before, const char *after) {
    char str[1000];

    realToString(value, str);
    printf("%sreal%" PRId32 " %s%s", before, value->digits, str, after);

  /*  int32_t i, exponent, last;

    if(realIsNaN(value)) {
      printf("NaN");
      return;
    }

    if(realIsNegative(value)) {
      printf("-");
    }

    if(realIsInfinite(value)) {
      printf("infinite");
      return;
    }

    if(realIsZero(value)) {
      printf("0");
      return;
    }

    if(value->digits % DECDPUN) {
      i = value->digits/DECDPUN;
    }
    else {
      i = value->digits/DECDPUN - 1;
    }

    while(value->lsu[i] == 0) i--;
    printf("%" PRIu16, value->lsu[i--]);

    exponent = value->exponent;
    last = 0;
    while(exponent <= -DECDPUN && value->lsu[last] == 0) {
      last++;
      exponent += DECDPUN;
    }

    for(; i>=last; i--) {
      printf(" %03" PRIu16, value->lsu[i]);
    }

    if(exponent != 0) {
      printf(" e %" PRId32, exponent);
    }*/
  }



  void printComplex34ToConsole(const complex34_t *value, const char *before, const char *after) {
    char str[100];

    real34ToString((real34_t *)value, str);
    printf("%scomplex34 %s + ", before, str);
    real34ToString((real34_t *)value + 1, str);
    printf("%si%s", str, after);
  }



  void printRegisterDescriptorToConsole(calcRegister_t regist) {
    registerHeader_t registerHeader;

    registerHeader.descriptor = 0xFFFFFFFF;

    if(regist <= LAST_GLOBAL_REGISTER) { // Global register
      registerHeader = globalRegister[regist];
    }

    else if(regist <= LAST_NAMED_VARIABLE) { // Named variable
      if(numberOfNamedVariables > 0) {
        regist -= FIRST_NAMED_VARIABLE;
        if(regist < numberOfNamedVariables) {
          registerHeader = allNamedVariables[regist].header;
        }
      }
    }

    else if(regist <= LAST_LOCAL_REGISTER) { // Local register
      if(currentNumberOfLocalRegisters > 0) {
        regist -= FIRST_LOCAL_REGISTER;
        if(regist < currentNumberOfLocalRegisters) {
          registerHeader = *POINTER_TO_LOCAL_REGISTER(regist);
        }
      }
    }

    printf("Header informations of register %d\n", regist);
    printf("    reg ptr   = %u\n", registerHeader.pointerToRegisterData);
    printf("    data type = %u = %s\n", registerHeader.dataType, getDataTypeName(registerHeader.dataType, false, false));
    if(registerHeader.dataType == dtLongInteger || registerHeader.dataType == dtString) {
      printf("    data ptr  = %u\n", registerHeader.pointerToRegisterData + 1);
      printf("    data size = %" PRIu16 "\n", *(uint16_t *)TO_PCMEMPTR(globalRegister[regist].pointerToRegisterData));
    }
    printf("    tag       = %u\n", registerHeader.tag);
  }



  void printLongIntegerToConsole(const longInteger_t value, const char *before, const char *after) {
    char str[3000];

    longIntegerToAllocatedString(value, str, sizeof(str));
    printf("%slong integer (%" PRIu64 " + %" PRIu64 " <%" PRIu64 " reserved> bytes) %s%s", before, (uint64_t)(sizeof(value->_mp_size) + sizeof(value->_mp_d) + sizeof(value->_mp_alloc)), (uint64_t)longIntegerSizeInBytes(value), (uint64_t)(value->_mp_alloc * LIMB_SIZE), str, after);
  }
#endif // !DMCP_BUILD



void reallocateRegister(calcRegister_t regist, uint32_t dataType, uint16_t dataSizeWithoutDataLenBlocks, uint32_t tag) { // dataSize without data length in blocks, this includes the trailing 0 for strings
  switch(dataType) {
    case dtComplex34:       dataSizeWithoutDataLenBlocks = COMPLEX34_SIZE_IN_BLOCKS;     break;

    case dtReal34:
    case dtTime:
    case dtDate:            dataSizeWithoutDataLenBlocks = REAL34_SIZE_IN_BLOCKS;        break;

    case dtShortInteger:    dataSizeWithoutDataLenBlocks = SHORT_INTEGER_SIZE_IN_BLOCKS; break;

    case dtConfig:          dataSizeWithoutDataLenBlocks = CONFIG_SIZE_IN_BLOCKS;        break;
    default: ;
  }

  uint16_t dataSizeWithDataLenBlocks = dataSizeWithoutDataLenBlocks;

  //printf("reallocateRegister: %d to %s tag=%u (%u bytes excluding maxSize) begin\n", regist, getDataTypeName(dataType, false, false), tag, dataSizeWithoutDataLenBlocks);
  if(dataType == dtString) {
    dataSizeWithDataLenBlocks = dataSizeWithoutDataLenBlocks + TO_BLOCKS(sizeof(strLgIntHeader_t));
  }
  else if(dataType == dtReal34Matrix || dataType == dtComplex34Matrix) {
    dataSizeWithDataLenBlocks = dataSizeWithoutDataLenBlocks + TO_BLOCKS(sizeof(matrixHeader_t));
  }
  else if(dataType == dtLongInteger) {
    if(TO_BYTES(dataSizeWithoutDataLenBlocks) % LIMB_SIZE != 0) {
      dataSizeWithoutDataLenBlocks = ((dataSizeWithoutDataLenBlocks / TO_BLOCKS(LIMB_SIZE)) + 1) * TO_BLOCKS(LIMB_SIZE);
    }
    dataSizeWithDataLenBlocks = dataSizeWithoutDataLenBlocks + TO_BLOCKS(sizeof(strLgIntHeader_t));
  }

  if(getRegisterDataType(regist) != dataType || ((getRegisterDataType(regist) == dtString || getRegisterDataType(regist) == dtLongInteger || getRegisterDataType(regist) == dtReal34Matrix || getRegisterDataType(regist) == dtComplex34Matrix) && getRegisterMaxDataLengthInBlocks(regist) != dataSizeWithoutDataLenBlocks)) {
    if(!isMemoryBlockAvailable(dataSizeWithDataLenBlocks)) {
      #if defined(PC_BUILD)
        printf("In function reallocateRegister: required %" PRIu16 " blocks for register #%" PRId16 " but no data blocks with enough size are available!\n", dataSizeWithoutDataLenBlocks, regist); fflush(stdout);
      #endif // PC_BUILD
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
      return;
    }
    freeRegisterData(regist);
    setRegisterDataPointer(regist, allocC47Blocks(dataSizeWithDataLenBlocks));
    setRegisterDataType(regist, dataType, tag);

    // After reallocating a register to a matrix, you MUST set
    if(dataType == dtReal34Matrix || dataType == dtComplex34Matrix) {
      REGISTER_MATRIX_HEADER(regist)->matrixRows = 1;
      REGISTER_MATRIX_HEADER(regist)->matrixColumns = 1;
    }
    else {
      setRegisterMaxDataLengthInBlocks(regist, dataSizeWithoutDataLenBlocks);
    }
  }

  if((dataType == dtComplex34) && getSystemFlag(FLAG_POLAR)) {
    setRegisterTag(regist, currentAngularMode | amPolar);
  }
  else {
    setRegisterTag(regist, tag);
  }

  //sprintf(tmpString, "reallocateRegister %d to %s tag=%u (%u bytes including dataLen) done", regist, getDataTypeName(dataType, false, false), tag, dataSizeWithDataLenBlocks);
  //memoryDump(tmpString);
}



void fnToReal(uint16_t unusedButMandatoryParameter) {

  if(getRegisterDataType(REGISTER_X) == dtComplex34) {
    real_t b;
    if(real34IsZero(REGISTER_IMAG34_DATA(REGISTER_X))) {
      real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &b);
      convertRealToResultRegister(&b, REGISTER_X, amNone);
      return;
    }
  }
  //else           //JM Remove comment if we don't want the usual data type error if the complex real part is not 0

  switch(getRegisterDataType(REGISTER_X)) {
    case dtLongInteger: {
      copySourceRegisterToDestRegister(REGISTER_X, REGISTER_L);
      convertLongIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
      break;
    }

    case dtShortInteger: {
      copySourceRegisterToDestRegister(REGISTER_X, REGISTER_L);
      convertShortIntegerRegisterToReal34Register(REGISTER_X, REGISTER_X);
      setLastintegerBasetoZero();
      break;
    }

    case dtReal34: {
      copySourceRegisterToDestRegister(REGISTER_X, REGISTER_L);
      if(getRegisterAngularMode(REGISTER_X) != amNone) {
        if(getRegisterAngularMode(REGISTER_X) == amDMS) {
          temporaryInformation = TI_FROM_DMS;
        }
        setRegisterAngularMode(REGISTER_X, amNone);
      }
      break;
    }

    case dtTime: {
      temporaryInformation = TI_FROM_HMS;
      copySourceRegisterToDestRegister(REGISTER_X, REGISTER_L);
      convertTimeRegisterToReal34Register(REGISTER_X, REGISTER_X);
      break;
    }

    case dtDate: {
      temporaryInformation = TI_FROM_DATEX;
      copySourceRegisterToDestRegister(REGISTER_X, REGISTER_L);
      convertDateRegisterToReal34Register(REGISTER_X, REGISTER_X);
      break;
    }

    default: {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "data type %s cannot be converted to a real34!", getRegisterDataTypeName(REGISTER_X, false, false));
        moreInfoOnError("In function fnToReal:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
  }
}
}


bool_t saveLastX(void) {
  copySourceRegisterToDestRegister(REGISTER_X, REGISTER_L);
  return lastErrorCode == ERROR_NONE;
}


static uint8_t getRegParam(bool_t *f, uint16_t *s, uint16_t *n, uint16_t *d) {
  real_t x, p;

  if(getRegisterDataType(REGISTER_X) == dtReal34) {
    *s = *n = 0;
    if(d) {
      *d = 0;
    }
    real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x);
    if(!realCompareAbsLessThan(&x, const_1000)) {
      return ERROR_OUT_OF_RANGE;
    }

    if(f) {
      *f = realIsNegative(&x);
    }
    if(f == NULL && realIsNegative(&x)) {
      return ERROR_OUT_OF_RANGE;
    }
    realSetPositiveSign(&x);

    realToIntegralValue(&x, &p, DEC_ROUND_DOWN, &ctxtReal39);
    *s = realToInt32C47(&p);

    realSubtract(&x, &p, &x, &ctxtReal39);
    x.exponent += 2;
    realToIntegralValue(&x, &p, DEC_ROUND_DOWN, &ctxtReal39);
    *n = realToInt32C47(&p);

    if(d) {
      realSubtract(&x, &p, &x, &ctxtReal39);
      x.exponent += 3;
      realToIntegralValue(&x, &p, DEC_ROUND_DOWN, &ctxtReal39);
      *d = realToInt32C47(&p);
    }

    if(*s < REGISTER_X) { // global numbered registers
      if(*s + *n >= REGISTER_X) {
        return ERROR_OUT_OF_RANGE;
      }
      else if(*n == 0) {
        *n = REGISTER_X - *s;
      }
    }
    else if(*s < FIRST_LOCAL_REGISTER) { // stack and global lettered registers (XYZT ABCD LIJK)
      if(*s + *n >= FIRST_LOCAL_REGISTER) {
        return ERROR_OUT_OF_RANGE;
      }
      else if(*n == 0) {
        *n = FIRST_LOCAL_REGISTER - *s;
      }
    }
    else if(*s < FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters) { // local registers
      if(*s + *n >= FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters) {
        return ERROR_OUT_OF_RANGE;
      }
      else if(f && *f) {
        return ERROR_OUT_OF_RANGE;
      }
      else if(*n == 0) {
        *n = FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters - *s;
      }
    }
    else {
      return ERROR_OUT_OF_RANGE;
    }

    if(d) {
      if(*d < REGISTER_X) { // global numbered registers
        if(*d + *n >= REGISTER_X) {
          return ERROR_OUT_OF_RANGE;
        }
      }
      else if(*d < FIRST_LOCAL_REGISTER) { // stack and global lettered registers (XYZT ABCD LIJK)
        if(*d + *n >= FIRST_LOCAL_REGISTER) {
          return ERROR_OUT_OF_RANGE;
        }
      }
      else if(*d < FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters) { // local registers
        if(*d + *n >= FIRST_LOCAL_REGISTER + currentNumberOfLocalRegisters) {
          return ERROR_OUT_OF_RANGE;
        }
      }
      else {
        return ERROR_OUT_OF_RANGE;
      }
    }

    return ERROR_NONE;
  }
  else {
    *s = *n = 0;
    if(d) {
      *d = 0;
    }
    return ERROR_INVALID_DATA_TYPE_FOR_OP;
  }
}


void fnRegClr(uint16_t unusedButMandatoryParameter) {
  uint16_t s, n;

  if((lastErrorCode = getRegParam(NULL, &s, &n, NULL)) == ERROR_NONE) {
    for(int i = s; i < (s + n); ++i) {
      clearRegister(i);
    }
  }
  else {
    displayCalcErrorMessage(lastErrorCode, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
  }
}


static void sortReg(uint16_t range_start, uint16_t range_end) {
  int8_t res;

  if(range_start == range_end) {
    // do nothing
  }
  else if(range_start + 1 == range_end) {
    if(registerCmp(range_start, range_end, &res)) {
      if(res > 0) {
        registerHeader_t savedRegisterHeader = globalRegister[range_start];
        globalRegister[range_start] = globalRegister[range_end];
        globalRegister[range_end] = savedRegisterHeader;
      }
    }
  }
  else {
    const uint16_t range_center = (range_end - range_start) / 2 + range_start;
    uint16_t pos1 = range_start, pos2 = range_center + 1;
    registerHeader_t *sortedReg = allocC47Blocks(TO_BLOCKS(sizeof(registerHeader_t)) * (range_end - range_start + 1));
    if(lastErrorCode == ERROR_RAM_FULL) {
      return; // unlikely
    }

    if(sortedReg) {
      sortReg(range_start,      range_center);
      sortReg(range_center + 1, range_end   );

      for(uint16_t i = 0; i <= (range_end - range_start); ++i) {
        if(registerCmp(pos1, pos2, &res)) {
          if(pos2 > range_end) {
            sortedReg[i] = globalRegister[pos1++];
          }
          else if(pos1 > range_center) {
            sortedReg[i] = globalRegister[pos2++];
          }
          else if(res > 0) {
            sortedReg[i] = globalRegister[pos2++];
          }
          else {
            sortedReg[i] = globalRegister[pos1++];
          }
        }
      }
      for(uint16_t i = 0; i <= (range_end - range_start); ++i) {
        globalRegister[range_start + i] = sortedReg[i];
      }
      freeC47Blocks(sortedReg, TO_BLOCKS(sizeof(registerHeader_t)) * (range_end - range_start + 1));
    }
    else { // unlikely
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
    }
  }
}


void fnRegSort(uint16_t unusedButMandatoryParameter) {
  uint16_t s, n;

  if((lastErrorCode = getRegParam(NULL, &s, &n, NULL)) == ERROR_NONE) {
    switch(getRegisterDataType(s)) {
      case dtLongInteger:
      case dtShortInteger:
      case dtReal34: {
        for(int i = s + 1; i < (s + n); ++i) {
          if((getRegisterDataType(i) != dtLongInteger) && (getRegisterDataType(i) != dtShortInteger) && (getRegisterDataType(i) != dtReal34)) {
            displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
            break;
          }
        }
        break;
      }
      case dtTime:
      case dtDate:
      case dtString: {
        for(int i = s + 1; i < (s + n); ++i) {
          if(getRegisterDataType(i) != getRegisterDataType(s)) {
            displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
            break;
          }
        }
        break;
    }
    }
    if(lastErrorCode == ERROR_NONE) {
      sortReg(s, s + n - 1);
    }
  }
  else {
    displayCalcErrorMessage(lastErrorCode, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
  }
}


void fnRegCopy(uint16_t unusedButMandatoryParameter) {
  bool_t f;
  uint16_t s, n, d;

  if((lastErrorCode = getRegParam(&f, &s, &n, &d)) == ERROR_NONE) {
    if(f) {
      doLoad(LM_REGISTERS_PARTIAL, s, n, d, manualLoad);
    }
    else {
      if(s > d) {
        for(int i = 0; i < n; ++i) {
          copySourceRegisterToDestRegister(s + i, d + i);
          if(lastErrorCode == ERROR_RAM_FULL) {
            return; // abort if not enough memory
          }
        }
      }
      else if(s < d) {
        for(int i = n - 1; i >= 0; --i) {
          copySourceRegisterToDestRegister(s + i, d + i);
          if(lastErrorCode == ERROR_RAM_FULL) {
            return; // abort if not enough memory
          }
        }
      }
    }
  }
  else {
    displayCalcErrorMessage(lastErrorCode, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
  }
}


void fnRegSwap(uint16_t unusedButMandatoryParameter) {
  uint16_t s, n, d;

  if((lastErrorCode = getRegParam(NULL, &s, &n, &d)) == ERROR_NONE) {
    if((d < s + n) && (s < d + n)) { // overlap
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
    }
    else {
      for(int i = 0; i < n; ++i) {
        registerHeader_t savedRegisterHeader = globalRegister[s + i];
        globalRegister[s + i] = globalRegister[d + i];
        globalRegister[d + i] = savedRegisterHeader;
      }
    }
  }
  else {
    displayCalcErrorMessage(lastErrorCode, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
  }
}


bool_t isFunctionAllowingNewVariable(uint16_t op) {
  switch(op) {
    case ITM_INPUT:
    case ITM_STO:
    case ITM_STOADD:
    case ITM_STOSUB:
    case ITM_STOMULT:
    case ITM_STODIV:
    case ITM_KEYQ:
    case ITM_M_DIM:
    case ITM_MVAR:
    case ITM_SOLVE:
    case ITM_STOCFG:
    case ITM_STOMAX:
    case ITM_STOMIN:
    case ITM_XtoALPHA:
    case ITM_Xex:
    case ITM_Yex:
    case ITM_Zex:
    case ITM_Tex:
    case ITM_INTEGRAL:
      return true;

    default:
      return false;
  }
}

