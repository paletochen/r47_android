// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file sumprod.c
 ***********************************************/

#include "c47.h"

//Complex operation:
//  The counter is always Real34.
//  The result of f(n) can be complex, in which case if CPXRES is set, operation continues in complex.
//  If not set, an error is raised.



#if !defined(TESTSUITE_BUILD)

  void showProgressReal(const real_t *a, real_t *ai, bool_t cpx) {
    real34_t a34, ai34;
    #if ENABLE_SOLVER_PROGRESS == 1
        uint8_t savedDisplayFormatDigits = displayFormatDigits;

        clearRegisterLine(REGISTER_Z, true, true);
        clearRegisterLine(REGISTER_Y, true, true);
        clearRegisterLine(REGISTER_X, true, true);

        displayFormatDigits = displayFormat == DF_ALL ? 0 : 33;
        if(!cpx) {
          realToReal34(a, &a34);
          real34ToDisplayString(&a34, amNone, tmpString, &standardFont, 9999, 34, !LIMITEXP, FRONTSPACE, NOIRFRAC);
          showString(tmpString, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
        }
        else {
          realToReal34(a, &a34);
          realToReal34(ai, &ai34);
          real34ToDisplayString(&a34, amNone, tmpString, &standardFont, 9999, 34, !LIMITEXP, FRONTSPACE, NOIRFRAC);
          showString(tmpString, &standardFont, 1, Y_POSITION_OF_REGISTER_Y_LINE + 6, vmNormal, true, true);
          uint32_t x = 0;
          if(real34CompareGreaterEqual(&ai34,const34_0)) {
            x = showString("+", &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
          }
          else {
            x = showString(" ", &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
          }
          real34ToDisplayString(&ai34, amNone, tmpString, &standardFont, 9999, 34, !LIMITEXP, FRONTSPACE, NOIRFRAC);
          strcat(tmpString, COMPLEX_UNIT);
          showString(tmpString, &standardFont, x, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
        }
        displayFormatDigits = savedDisplayFormatDigits;
      force_refresh(force);
    #endif // ENABLE_SOLVER_PROGRESS == 1
  }


  static void _programmableSumProd(uint16_t label, bool_t prod) {
    currentKeyCode = 255;
    int32_t       loop = 0;
    int16_t       finished = 0;
    real_t        resultX, resultXi, resultR, resultRi;
    real34_t      loopStep, loopTo, counter, compare, sign, rLoop;
    bool_t        changedOverToComplex = false;
    longInteger_t iLoop;
    fnToReal(NOPARAM);
    real34Copy(REGISTER_REAL34_DATA(REGISTER_X), &loopStep);
    fnDrop(NOPARAM);
    fnToReal(NOPARAM);
    real34Copy(REGISTER_REAL34_DATA(REGISTER_X), &loopTo);
    fnDrop(NOPARAM);
    fnToReal(NOPARAM);
    real34Copy(REGISTER_REAL34_DATA(REGISTER_X), &counter); //Loopfrom
    realCopy(prod ? const_1 : const_0, &resultR);           //Initialize real accumulator
    realCopy(const_0, &resultRi);                           //Initialize complex accumulator

    real34Subtract(&loopTo, &counter, &rLoop);              //calculate the remaining iteration counter
    if(!real34IsZero(&loopStep)) {
      real34Divide(&rLoop, &loopStep, &rLoop);
    }
    longIntegerInit(iLoop);
    convertReal34ToLongInteger(&rLoop, iLoop, DEC_ROUND_DOWN);
    loop = (int32_t)longIntegerModuloUInt(iLoop, (int32_t)(0x7FFFFFFF));
    longIntegerFree(iLoop);

    if( !real34CompareEqual(&loopTo, &counter) &&
        (  real34IsZero(&loopStep) ||
          (real34CompareGreaterThan(&loopTo, &counter) && real34CompareLessEqual(&loopStep, const34_0)) ||
          (real34CompareLessThan(&loopTo, &counter) && real34CompareGreaterEqual(&loopStep, const34_0))
        )
      ) {
      displayCalcErrorMessage(ERROR_BAD_INPUT, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Counter will not count to destination");
        moreInfoOnError("In function _programmableSumProd:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      ++currentSolverNestingDepth;
      setSystemFlag(FLAG_SOLVING);

      while(lastErrorCode == ERROR_NONE) {

        loop--;
        if(checkHalfSec()) {
          if(progressHalfSecUpdate_Integer(timed, "Loop: ",loop, halfSec_clearZ, halfSec_clearT, halfSec_disp)) {
            showProgressReal(&resultR, &resultRi, changedOverToComplex);
          }
        }

        real34Compare(&counter, &loopTo, &compare);
        real34Compare(&loopStep, const34_0, &sign);
        real34Multiply(&compare, &sign, &compare);
        finished = real34ToInt32(&compare);                       //0 means equal
        if(finished > 0) {
          break;
        }

        if(getRegisterDataType(REGISTER_X) != dtReal34) {
          reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
        }
        real34Copy(&counter, REGISTER_REAL34_DATA(REGISTER_X));
        fnFillStack(NOPARAM);

        dynamicMenuItem = -1;
        execProgram(label);
        if(lastErrorCode != ERROR_NONE) {
          break;
        }

        if(getFlag(FLAG_CPXRES) && (getRegisterDataType(REGISTER_X) == dtComplex34 || !realIsZero(&resultRi))) {
            changedOverToComplex = true;     //Only latch over to complex operation if CPXRES is true, as well as either sum or new f(n) is complex
        }

        if(!changedOverToComplex) {
          fnToReal(NOPARAM);
          if(lastErrorCode != ERROR_NONE) {
            break;
          }
          real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &resultX); //Result accumulated
          if(prod) {
            realMultiply(&resultR, &resultX, &resultR, &ctxtReal75);
          }
          else {
            realAdd(&resultR, &resultX, &resultR, &ctxtReal75);
          }
        }
        else { //dtComplex34
          real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &resultX);  //Result accumulated
          real34ToReal(REGISTER_IMAG34_DATA(REGISTER_X), &resultXi); //Result accumulated
          if(prod) {
            mulComplexComplex(&resultR, &resultRi, &resultX, &resultXi, &resultR, &resultRi, &ctxtReal75);
          }
          else {
            realAdd(&resultR, &resultX, &resultR, &ctxtReal75);
            realAdd(&resultRi, &resultXi, &resultRi, &ctxtReal75);
          }
        }



        #if defined(VERBOSE_COUNTER)
          printf(">>> Fin: %d, Cpx: %d ", finished, changedOverToComplex);
          printReal34ToConsole(&counter," Cnt: ", " ");
          printRealToConsole(&resultX," X: ", " ");
          if(changedOverToComplex) {
            printRealToConsole(&resultXi," Xi: ", " ");
          }
          printRealToConsole(&resultR," SUM: ", "");
          if(changedOverToComplex) {
            printRealToConsole(&resultRi," SUMii: ", " ");
          }
          printf("\n");
        #endif // VERBOSE_COUNTER

        real34Add(&counter, &loopStep, &counter);

        if(finished == 0) {
          break;
        }
      } //WHILE


      if(lastErrorCode == ERROR_NONE) {
        if(!changedOverToComplex) {
          if(getRegisterDataType(REGISTER_X) != dtReal34) {
            reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
          }
          convertRealToReal34ResultRegister(&resultR, REGISTER_X);
        }
        else {
          if(getRegisterDataType(REGISTER_X) != dtComplex34) {
            reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
          }
          convertRealToReal34ResultRegister(&resultR, REGISTER_X);
          convertRealToImag34ResultRegister(&resultRi, REGISTER_X);
        }
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
      }
      else {
        displayCalcErrorMessage(lastErrorCode, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "Error or exit while calculating");
          moreInfoOnError("In function _programmableSumProd:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }

      temporaryInformation = TI_NO_INFO;
      if(programRunStop == PGM_WAITING) {
        programRunStop = PGM_STOPPED;
      }
    } //MAIN IF
    if((--currentSolverNestingDepth) == 0) {
      clearSystemFlag(FLAG_SOLVING);
    }
  }



  static void _checkArgument(uint16_t label, bool_t prod) {
    if(FIRST_LABEL <= label && label <= LAST_LABEL) {
      _programmableSumProd(label, prod);
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
          moreInfoOnError("In function _checkArgument:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }
      else {
        _programmableSumProd(label, prod);
      }
    }
    else {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "unexpected parameter %u", label);
        moreInfoOnError("In function _checkArgument:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }
#endif // !TESTSUITE_BUILD

void fnProgrammableSum(uint16_t label) {
  #if !defined(TESTSUITE_BUILD)
    _checkArgument(label, false);
  #endif // !TESTSUITE_BUILD
}

void fnProgrammableProduct(uint16_t label) {
  #if !defined(TESTSUITE_BUILD)
    _checkArgument(label, true);
  #endif // !TESTSUITE_BUILD
}
