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
          if(real34CompareGreaterEqual(&ai34, const34_0)) {
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


  // Early abort, reached through the infinity sum only: the passes still to come cannot reach the
  // last digit of the answer, so the run stops. Real sums only.
  #define EARLY_ABORT_WATCH_FROM  10   //first iteration whose term is judged; anything before it is ignored
  #define EARLY_ABORT_NOT_BEFORE  50   //earliest iteration a run may stop on
  #define EARLY_ABORT_IN_COMPLEX  false//the test needs a magnitude, so a complex run counts to the end

  #define SUMMING     false            //what _checkArgument and _programmableSumProd do with each term
  #define MULTIPLYING true
  #define RUNALL      NULL             //no early stop state, so every iteration the caller asked for

  #if defined(OPTION_INFSUMS)
  typedef struct {                     //only the infinity sum carries this, on its own frame
    real_t   previousTerm, term, remaining, allowance, scale;
    bool_t   haveTerm, falling;
    uint32_t pass;
  } earlyAbort_t;
  #else // OPTION_INFSUMS
  typedef void earlyAbort_t;           //no early stop state exists, so every caller passes RUNALL
  #endif // OPTION_INFSUMS


  static void _programmableSumProd(uint16_t label, bool_t prod, earlyAbort_t *early) {
    currentKeyCode = 255;
    #if defined(OPTION_INFSUMS)
    const bool_t  inf = (early != NULL);
    #else // OPTION_INFSUMS
    (void)early;
    #endif // OPTION_INFSUMS
    const bool_t  cpxAllowed = getFlag(FLAG_CPXRES);        //read once: the term program itself can set CPXRES
    int32_t       loop = 0;
    int16_t       finished = 0;
    real_t        resultX, resultXi, resultR, resultRi;
    real34_t      loopStep, loopTo, counter, compare, sign, rLoop, moved;
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
    realSetZero(&resultRi);                                    //Initialize complex accumulator

    const uint8_t guardRoundingMode = roundingMode;
    setRoundingMode(RM_ENGINE);                             // the count and the test below round as the counter step does
    real34Subtract(&loopTo, &counter, &rLoop);              //calculate the remaining iteration counter
    if(!real34IsZero(&loopStep)) {
      real34Divide(&rLoop, &loopStep, &rLoop);
    }
    // convertReal34ToLongInteger initialises iLoop; do not pre-init it (leak).
    convertReal34ToLongInteger(&rLoop, iLoop, DEC_ROUND_DOWN);
    loop = (int32_t)longIntegerModuloUInt(iLoop, (int32_t)(0x7FFFFFFF));
    longIntegerFree(iLoop);

    real34Add(&counter, &loopStep, &moved);                 // the counter plus the step, against the counter, which is the test the FOR structure makes
    setRoundingMode(guardRoundingMode);
    if(real34CompareEqual(&moved, &counter)) {              // a step of zero, and a step too small for the counter's digits, are the same fault
      displayCalcErrorMessage(ERROR_STEP_OF_ZERO, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Counter will not move");
        moreInfoOnError("In function _programmableSumProd:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else if( !real34CompareEqual(&loopTo, &counter) &&
        (  real34IsZero(&loopStep) ||                         // unreachable, leave a backup in case input filtering change
          (real34CompareGreaterThan(&loopTo, &counter) && real34CompareLessEqual(&loopStep, const34_0)) ||
          (real34CompareLessThan(&loopTo, &counter) && real34CompareGreaterEqual(&loopStep, const34_0))
        )
      ) {
      displayCalcErrorMessage(ERROR_BAD_INPUT, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "Counter will not count to destination");
        moreInfoOnError("In function _programmableSumProd:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      ++currentSolverNestingDepth;
      setSystemFlag(FLAG_SOLVING);
      const uint8_t userRoundingMode = roundingMode;
      setRoundingMode(RM_ENGINE);                           // the engine iterates at RM_ENGINE, and the sum is stored once by RM

      #if defined(OPTION_INFSUMS)
      if(inf) {
        realSetZero(&early->previousTerm);
        early->haveTerm = false;
        early->falling  = true;
        early->pass     = 0;
        int32ToReal(significantDigits == 0 ? 34 : significantDigits, &early->scale);
        realPower(const_10, &early->scale, &early->scale, &ctxtReal75);       //10^SDIGS, fixed for the run
      }
      #endif // OPTION_INFSUMS

      while(lastErrorCode == ERROR_NONE) {

        loop--;
        if(checkHalfSec()) {
          if(progressHalfSecUpdate_Integer(timed, "Loop: ", loop, halfSec_clearZ, halfSec_clearT, halfSec_disp)) {
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
        setRoundingMode(RM_ENGINE);                         // a mode the term program sets does not reach the engine
        if(lastErrorCode != ERROR_NONE) {
          break;
        }

        if(getRegisterDataType(REGISTER_X) == dtComplex34 || !realIsZero(&resultRi)) {
          if(cpxAllowed) {
            changedOverToComplex = true;     //Only latch over to complex operation if CPXRES is true, as well as either sum or new f(n) is complex
          }
          else {
            displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
            #if (EXTRA_INFO_ON_CALC_ERROR == 1)
              sprintf(errorMessage, "f(n) returned a complex value while flag I is not set!");
              moreInfoOnError("In function _programmableSumProd:", errorMessage, NULL, NULL);
            #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
            break;
          }
        }

        #if defined(OPTION_INFSUMS)
        if(inf && early->pass < UINT32_MAX) {               //counted for the complex branch too, and saturates
          early->pass++;
        }
        #endif // OPTION_INFSUMS

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
            #if defined(OPTION_INFSUMS)
            if(inf) {
              realCopy(&resultX, &early->term);             //the term itself, which outlives a saturated total
              realSetPositiveSign(&early->term);
              if(!realIsZero(&early->term)) {               //a term of zero says nothing about the terms after it
                if(early->haveTerm && early->pass >= EARLY_ABORT_WATCH_FROM
                                   && realCompareGreaterThan(&early->term, &early->previousTerm)) {
                  early->falling = false;                   //one rise after the watch point disqualifies the run
                }
                if(early->falling && early->pass >= EARLY_ABORT_NOT_BEFORE) {
                  real34Subtract(&loopTo, &counter, &rLoop);             //iterations still to come, no integer count
                  if(!real34IsZero(&loopStep)) {
                    real34Divide(&rLoop, &loopStep, &rLoop);
                  }
                  real34ToReal(&rLoop, &early->remaining);
                  realSetPositiveSign(&early->remaining);
                  realDivide(&resultR, &early->scale, &early->allowance, &ctxtReal75);  //one last digit of the answer
                  realSetPositiveSign(&early->allowance);
                  realDivide(&early->allowance, &early->term, &early->allowance, &ctxtReal75); //iterations it is worth
                  if(realCompareLessThan(&early->remaining, &early->allowance)) {
                    break;
                  }
                }
                realCopy(&early->term, &early->previousTerm);
                early->haveTerm = true;
              }
            }
            #endif // OPTION_INFSUMS
          }
        }
        else { //dtComplex34, and EARLY_ABORT_IN_COMPLEX is false, so this branch always runs the full count
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
          printReal34ToConsole(&counter, " Cnt: ", " ");
          printRealToConsole(&resultX, " X: ", " ");
          if(changedOverToComplex) {
            printRealToConsole(&resultXi, " Xi: ", " ");
          }
          printRealToConsole(&resultR, " SUM: ", "");
          if(changedOverToComplex) {
            printRealToConsole(&resultRi, " SUMii: ", " ");
          }
          printf("\n");
        #endif // VERBOSE_COUNTER

        real34Add(&counter, &loopStep, &counter);

        if(finished == 0) {
          break;
        }
      } //WHILE
      setRoundingMode(userRoundingMode);


      if(lastErrorCode == ERROR_NONE) {
        #if defined(OPTION_INFSUMS)
        if(inf) {                                           //iterations actually run, so a short run is visible
          longInteger_t iPass;
          longIntegerInit(iPass);
          uInt32ToLongInteger(early->pass, iPass);
          convertLongIntegerToLongIntegerRegister(iPass, REGISTER_Y);
          longIntegerFree(iPass);
        }
        #endif // OPTION_INFSUMS
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
        displayCalcErrorMessage(lastErrorCode, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "Error or exit while calculating");
          moreInfoOnError("In function _programmableSumProd:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      }

      temporaryInformation = TI_NO_INFO;
      if(programRunStop == PGM_WAITING) {
        programRunStop = PGM_STOPPED;
      }

      if((--currentSolverNestingDepth) == 0) {
        clearSystemFlag(FLAG_SOLVING);
      }
    } //MAIN IF
  }



  static void _checkArgument(uint16_t label, bool_t prod, earlyAbort_t *early) {
    label = findProgramLabel(label, "In function _checkArgument:");
    if(label != INVALID_VARIABLE) {
      _programmableSumProd(label, prod, early);
    }
  }

void fnProgrammableSum(uint16_t label) {
  _checkArgument(label, SUMMING, RUNALL);
}

#if defined(OPTION_INFSUMS)
void fnProgrammableSumInf(uint16_t label) {
  earlyAbort_t earlyAbort;                                  //this frame carries the early stop state
  _checkArgument(label, SUMMING, &earlyAbort);
}
#endif // OPTION_INFSUMS

void fnProgrammableProduct(uint16_t label) {
  _checkArgument(label, MULTIPLYING, RUNALL);
}
