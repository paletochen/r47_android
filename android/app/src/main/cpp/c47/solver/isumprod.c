// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file isumprod.c
 ***********************************************/

#include "c47.h"

#if !defined(TESTSUITE_BUILD)

  static void _showProgress(const longInteger_t a) {
    #if ENABLE_SOLVER_PROGRESS == 1
        uint8_t savedDisplayFormatDigits = displayFormatDigits;

        clearRegisterLine(REGISTER_Z, true, true);
        clearRegisterLine(REGISTER_Y, true, true);
        clearRegisterLine(REGISTER_X, true, true);

        displayFormatDigits = displayFormat == DF_ALL ? 0 : 33;
        convertLongIntegerToLongIntegerRegister(a, TEMP_REGISTER_1);
        longIntegerRegisterToDisplayString(TEMP_REGISTER_1, tmpString, TMP_STR_LENGTH, 400, 400, false);//JM added last parameter: Allow LARGELI
        showString(tmpString, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
        displayFormatDigits = savedDisplayFormatDigits;
        force_refresh(force);
  #endif // ENABLE_SOLVER_PROGRESS == 1
  }


  static void _programmableiSumProd(uint16_t label, bool_t prod) {
    currentKeyCode = 255;
    int32_t       loop = 0;
    int16_t       finished = 0;
    longInteger_t resultLi, xLi;
    longInteger_t loopStep, loopTo, iCounter, iLoop;
    longIntegerInit(loopStep);
    longIntegerInit(loopTo);
    longIntegerInit(iCounter);
    longIntegerInit(iLoop);
    longIntegerInit(resultLi);
    convertLongIntegerRegisterToLongInteger(REGISTER_Y, loopTo);
    convertLongIntegerRegisterToLongInteger(REGISTER_X, loopStep);
    convertLongIntegerRegisterToLongInteger(REGISTER_Z, iCounter); //Loopfrom counter initial value = From
    uInt32ToLongInteger(prod ? 1u : 0u, resultLi);                     //Initialize long integer accumulator

    longIntegerSubtract(loopTo, iCounter, iLoop);                  //calculate the remaining iteration counter
    if(!longIntegerIsZero(loopStep)) {
      longIntegerDivide(iLoop, loopStep, iLoop);
    }
    loop = (int32_t)longIntegerModuloUInt(iLoop, (int32_t)(0x7FFFFFFF));

    if(longIntegerCompare(loopTo, iCounter) != 0 &&
        (longIntegerIsZero(loopStep) ||
          (longIntegerCompare(loopTo, iCounter) > 0 && longIntegerCompareUInt(loopStep, 0) <=0) ||
          (longIntegerCompare(loopTo, iCounter) < 0 && longIntegerCompareUInt(loopStep, 0) >=0) )
    ) {
      displayCalcErrorMessage(ERROR_BAD_INPUT, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Counter will not count to destination");
        moreInfoOnError("In function _programmableiSumProd:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      ++currentSolverNestingDepth;
      setSystemFlag(FLAG_SOLVING);

      while(lastErrorCode == ERROR_NONE) {

        loop--;
        if(checkHalfSec()) {
          if(progressHalfSecUpdate_Integer(timed, "Loop: ",loop, halfSec_clearZ, halfSec_clearT, halfSec_disp)) {
            _showProgress(resultLi);
          }
        }

        finished = longIntegerCompare(iCounter, loopTo);            // 0 mean equal
        finished = finished * longIntegerCompareUInt(loopStep, 0);
        if(finished > 0) {
          break;
        }

        convertLongIntegerToLongIntegerRegister(iCounter, REGISTER_X);
        fnFillStack(NOPARAM);

        #if defined(VERBOSE_COUNTER)
          printRegisterToConsole(REGISTER_X,"[f(",") ");
        #endif //VERBOSE_COUNTER


        dynamicMenuItem = -1;
        execProgram(label);
        if(lastErrorCode != ERROR_NONE) {
          break;
        }

        #if defined(VERBOSE_COUNTER)
          printRegisterToConsole(REGISTER_X," = ","] ");
          printLongIntegerToConsole(resultLi," + "," ");
        #endif //VERBOSE_COUNTER

        if(getRegisterDataType(REGISTER_X) != dtLongInteger) {
          lastErrorCode = ERROR_INVALID_DATA_TYPE_FOR_OP;
          break;
        }


        convertLongIntegerRegisterToLongInteger(REGISTER_X, xLi); //Result accumulated
        if(prod) {
          longIntegerMultiply(resultLi, xLi, resultLi);
        }
        else {
          longIntegerAdd(resultLi, xLi, resultLi);
        }
        longIntegerFree(xLi);

        #if defined(VERBOSE_COUNTER)
          printLongIntegerToConsole(resultLi,"= "," \n");
        #endif //VERBOSE_COUNTER

        longIntegerAdd(iCounter, loopStep, iCounter);

        if(finished == 0) {
          break;
        }
      } //WHILE


      if(lastErrorCode == ERROR_NONE) {
        convertLongIntegerToLongIntegerRegister(resultLi, REGISTER_X);
      }
      else {
        displayCalcErrorMessage(lastErrorCode, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "Error while calculating");
          moreInfoOnError("In function _programmableiSumProd:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }

      longIntegerFree(resultLi);
      longIntegerFree(iLoop);
      longIntegerFree(iCounter);
      longIntegerFree(loopTo);
      longIntegerFree(loopStep);

      temporaryInformation = TI_NO_INFO;
      if(programRunStop == PGM_WAITING) {
        programRunStop = PGM_STOPPED;
      }

    } //MAIN IF
    if((--currentSolverNestingDepth) == 0) {
      clearSystemFlag(FLAG_SOLVING);
    }
  }


static bool_t _checkRegisters(void) {
  if(getRegisterDataType(REGISTER_X) != dtLongInteger) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Long integer expected");
      moreInfoOnError("In function _checkRegisters:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return true;
  }
  else if(getRegisterDataType(REGISTER_Y) != dtLongInteger) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_Y);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Long integer expected");
      moreInfoOnError("In function _checkRegisters:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return true;
  }
  else if(getRegisterDataType(REGISTER_Z) != dtLongInteger) {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE, REGISTER_Z);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "Long integer expected");
      moreInfoOnError("In function _checkRegisters:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return true;
  }
  return false;
}


static void _checkiArgument(uint16_t label, bool_t prod) {
    if(FIRST_LABEL <= label && label <= LAST_LABEL) {
      if(!_checkRegisters()) {
        _programmableiSumProd(label, prod);
      }
    }
    else if(REGISTER_X <= label && label <= REGISTER_T) {
      // Interactive mode
      char buf[2];
      buf[0] = letteredRegisterName((calcRegister_t)label);
      buf[1] = 0;
      label = findNamedLabel(buf);
      if(label == INVALID_VARIABLE) {
        displayCalcErrorMessage(ERROR_LABEL_NOT_FOUND, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "string '%s' is not a named label", buf);
          moreInfoOnError("In function _checkiArgument:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      else {
        if(!_checkRegisters()) {
          _programmableiSumProd(label, prod);
        }
      }

    }
    else {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "unexpected parameter %u", label);
        moreInfoOnError("In function _checkiArgument:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
#endif // !TESTSUITE_BUILD

void fnProgrammableiSum(uint16_t label) {
  #if !defined(TESTSUITE_BUILD)
    _checkiArgument(label, false);
  #endif // !TESTSUITE_BUILD
}

void fnProgrammableiProduct(uint16_t label) {
  #if !defined(TESTSUITE_BUILD)
    _checkiArgument(label, true);
  #endif // !TESTSUITE_BUILD
}
