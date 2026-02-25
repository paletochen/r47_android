// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file setClearFlipBits.c
 ***********************************************/

#include "c47.h"

/********************************************//**
 * \brief regX ==> regL and CB(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnCb(uint16_t bit) { // bit from 0=LSB to shortIntegerWordSize-1=MSB
  uint64_t w;
  uint32_t base;

  if(bit > shortIntegerWordSize) {
    displayCalcErrorMessage(ERROR_WORD_SIZE_TOO_SMALL, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot calculate CB(%d) word size is %d", bit, shortIntegerWordSize);
      moreInfoOnError("In function fnCb:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }
  if (!getRegisterAsRawShortInt(REGISTER_X, &w, &base) || !saveLastX())
    return;

  reallocateRegister(REGISTER_X, dtShortInteger, SHORT_INTEGER_SIZE_IN_BLOCKS, base);
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = w & ~((uint64_t)1 << bit);
}



/********************************************//**
 * \brief regX ==> regL and SB(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSb(uint16_t bit) { // bit from 0=LSB to shortIntegerWordSize-1=MSB
  uint64_t w;
  uint32_t base;

  if(bit > shortIntegerWordSize) {
    displayCalcErrorMessage(ERROR_WORD_SIZE_TOO_SMALL, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot calculate SB(%d) word size is %d", bit, shortIntegerWordSize);
      moreInfoOnError("In function fnSb:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }
  if (!getRegisterAsRawShortInt(REGISTER_X, &w, &base) || !saveLastX())
    return;

  reallocateRegister(REGISTER_X, dtShortInteger, SHORT_INTEGER_SIZE_IN_BLOCKS, base);
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = w | ((uint64_t)1 << bit);
}



/********************************************//**
 * \brief regX ==> regL and FB(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnFb(uint16_t bit) { // bit from 0=LSB to shortIntegerWordSize-1=MSB
  uint64_t w;
  uint32_t base;

  if(bit > shortIntegerWordSize) {
    displayCalcErrorMessage(ERROR_WORD_SIZE_TOO_SMALL, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot calculate FB(%d) word size is %d", bit, shortIntegerWordSize);
      moreInfoOnError("In function fnFb:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }
  if (!getRegisterAsRawShortInt(REGISTER_X, &w, &base) || !saveLastX())
    return;

  reallocateRegister(REGISTER_X, dtShortInteger, SHORT_INTEGER_SIZE_IN_BLOCKS, base);
  *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = w ^ ((uint64_t)1 << bit);
}



/********************************************//**
 * \brief bit clear in register X
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnBc(uint16_t bit) { // bit from 0=LSB to shortIntegerWordSize-1=MSB
  uint64_t w;

  if(bit > shortIntegerWordSize) {
    displayCalcErrorMessage(ERROR_WORD_SIZE_TOO_SMALL, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot calculate BC?(%d) word size is %d", bit, shortIntegerWordSize);
      moreInfoOnError("In function fnBc:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }
  if (!getRegisterAsRawShortInt(REGISTER_X, &w, NULL))
    return;
  thereIsSomethingToUndo = false;
  SET_TI_TRUE_FALSE((w & ((uint64_t)1 << bit)) == 0);
}



/********************************************//**
 * \brief bit set in register X
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnBs(uint16_t bit) { // bit from 0=LSB to shortIntegerWordSize-1=MSB
  uint64_t w;

  if(bit > shortIntegerWordSize) {
    displayCalcErrorMessage(ERROR_WORD_SIZE_TOO_SMALL, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "cannot calculate BS?(%d) word size is %d", bit, shortIntegerWordSize);
      moreInfoOnError("In function fnBs:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }
  if (!getRegisterAsRawShortInt(REGISTER_X, &w, NULL))
    return;
  thereIsSomethingToUndo = false;
  SET_TI_TRUE_FALSE((w & ((uint64_t)1 << bit)) != 0);
}
