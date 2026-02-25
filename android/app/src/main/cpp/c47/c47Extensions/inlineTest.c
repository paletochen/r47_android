// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file inlineTest.c
 ***********************************************/

#include "c47.h"

#if !defined(INLINE_TEST)
  //extern bool_t   testEnabled;
  //extern uint16_t testBitset;
  void   fnSwStart           (uint8_t nr) {}
  void   fnSwStop            (uint8_t nr) {}
  void   fnSetInlineTest     (uint16_t drConfig) {}
  void   fnGetInlineTestBsToX(uint16_t unusedButMandatoryParameter) {}
  void   fnSetInlineTestXToBs(uint16_t unusedButMandatoryParameter) {}
  void   fnSysFreeMem        (uint16_t unusedButMandatoryParameter) {}
  bool_t fnTestBitIsSet      (uint8_t bit) { return false; }
#endif // !INLINE_TEST


#if defined(INLINE_TEST)
  uint32_t swStart[4];
  uint32_t swStop[4];
  /********************************************//**
  * \brief Start StopWatch
  *
  * \param[in] void
  * \return void
  ***********************************************/
  void fnSwStart(uint8_t nr) {
    #if defined(DMCP_BUILD)
      swStart[nr] = sys_current_ms();
    #endif // DMCP_BUILD

    #if defined(PC_BUILD)
      swStart[nr] = g_get_monotonic_time();
    #endif // PC_BUILD
  }


  /** integer to string
   * C++ version 0.4 char* style "itoa":
   * Written by Lukás Chmela
   * Released under GPLv3.
   */
  char* itoa(int value, char* result, int base) {
    // check that the base if valid
    if(base < 2 || base > 16) {
      *result = '\0';
      return result;
    }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
      tmp_value = value;
      value /= base;
      *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while(value);

    // Apply negative sign
    if(tmp_value < 0) {
      *ptr++ = '-';
    }
    *ptr-- = '\0';
    while(ptr1 < ptr) {
      tmp_char = *ptr;
      *ptr--= *ptr1;
      *ptr1++ = tmp_char;
    }
    return result;
  }


  /********************************************//**
  * \brief Stop StopWatch
  *
  * \param[in] void
  * \return void
  ***********************************************/
  void fnSwStop(uint8_t nr) {
    #if defined(DMCP_BUILD)
      swStop[nr] = sys_current_ms();
    #endif // DMCP_BUILD

    #if defined(PC_BUILD)
      swStop[nr] = g_get_monotonic_time();
    #endif // PC_BUILD

    #if !defined(TESTSUITE_BUILD)
      uint32_t swTime = swStop[nr] - swStart[nr];
      char snum[50];
    #endif // !TESTSUITE_BUILD

    #if defined(DMCP_BUILD)
      showString("ms:", &standardFont, 30, 40 +nr*20, vmNormal, false, false);
    #endif // DMCP_BUILD

    #if defined(PC_BUILD)
      showString(STD_mu "s:", &standardFont, 30, 40 +nr*20, vmNormal, false, false);
    #endif // PC_BUILD

    #if !defined(TESTSUITE_BUILD)
      itoa(nr, snum, 10);
      showString(snum, &standardFont, 20, 40 +nr*20, vmNormal, false, false);
      itoa(swTime, snum, 10);
      strcat(snum, "         ");
      showString(snum, &standardFont, 60, 40 +nr*20, vmNormal, false, false);
    #endif // !TESTSUITE_BUILD
  }


  /********************************************//**
  * \brief Sets/resets flag
  *
  * \param[in] drConfig uint16_t
  * \return void
  ***********************************************/
  void fnSetInlineTest(uint16_t drConfig) {
    switch(drConfig) {
    case JC_ITM_TST:
      testEnabled = !testEnabled;
      fnRefreshState();                                       //jm
      break;

    default:
      break;
    }
  }


  /********************************************//**
  * \brief Get value of testBitset to X
  *
  * \param[in] unusedButMandatoryParameter uint16_t
  * \return void
  ***********************************************/
  void fnGetInlineTestBsToX(uint16_t unusedButMandatoryParameter) {
    char snum[10];
    longInteger_t mem;
    longIntegerInit(mem);
    liftStack();

    itoa(testBitset, snum, 10);
    stringToLongInteger(snum, 10, mem);

    convertLongIntegerToLongIntegerRegister(mem, REGISTER_X);
    longIntegerFree(mem);

    //refreshStack();
  }



  /********************************************//**
  * \brief Set X to testBitset value
  *
  * \param[in] unusedButMandatoryParameter uint16_t
  * \return void
  ***********************************************/
  void fnSetInlineTestXToBs(uint16_t unusedButMandatoryParameter) {
    uint16_t X_REG;
    longInteger_t lgInt;

    if(getRegisterDataType(REGISTER_X) == dtLongInteger) {
      convertLongIntegerRegisterToLongInteger(REGISTER_X, lgInt);
      longIntegerToAllocatedString(lgInt, tmpString, TMP_STR_LENGTH);
      longIntegerToInt32(lgInt, X_REG);
      longIntegerFree(lgInt);
      testBitset = mod(X_REG, 0x10000);
    }
  }



  /********************************************//**
  * \brief sys_free_mem
  *
  * \param[in] unusedButMandatoryParameter uint16_t
  * \return void
  ***********************************************/
  void fnSysFreeMem(uint16_t unusedButMandatoryParameter) {
    real_t value;

    saveForUndo();
    liftStack();

    #if defined(PC_BUILD)
      int32ToReal(2147483647, &value);
    #endif // PC_BUILD

    #if defined(DMCP_BUILD)
      int32ToReal(sys_free_mem(), &value);
    #endif // DMCP_BUILD

    //value.exponent -= 3; // value = value / 1000
    realToReal34(&value, REGISTER_REAL34_DATA(REGISTER_X));
  }



  /********************************************//**
  * \brief Test if bit is set in testBitset
  *
  * \param[in] bit uint8_t
  * \return void
  ***********************************************/
  bool_t fnTestBitIsSet(uint8_t bit) {
    uint16_t new_num = testBitset >> (bit);

    return (new_num & 1);
  }
#endif // INLINE_TEST
