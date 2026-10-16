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



  void showProgressReal(const cplx_t *a, bool_t cpx) {
    real34_t a34, ai34;
    #if ENABLE_SOLVER_PROGRESS == 1
        uint8_t savedDisplayFormatDigits = displayFormatDigits;

        clearRegisterLine(REGISTER_Z, true, true);
        clearRegisterLine(REGISTER_Y, true, true);
        clearRegisterLine(REGISTER_X, true, true);

        displayFormatDigits = displayFormat == DF_ALL ? 0 : 33;
        if(!cpx) {
          realToReal34(&a->Real, &a34);
          real34ToDisplayString(&a34, amNone, tmpString, &standardFont, 9999, 34, !LIMITEXP, FRONTSPACE, NOIRFRAC);
          showString(tmpString, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
        }
        else {
          realToReal34(&a->Real, &a34);
          realToReal34(&a->Imag, &ai34);
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


  /********************************************//**
   * \brief Adds one finite term and retains the rounding residual
   *
   * \param[in,out] sum leading accumulator
   * \param[in,out] correction accumulated residual
   * \param[in] term new term
   * \return void
  ***********************************************/
  static void _sumProdAdd(real_t *sum, real_t *correction, const real_t *term) {
    REAL_T_PTR(rounded, 75);
    REAL_T_PTR(residual, 75);
    const real_t *larger = sum;
    const real_t *smaller = term;

    if(realIsSpecial(sum) || realIsSpecial(term)) {
      realAdd(sum, term, sum, &ctxtReal75);
      return;
    }
    if(realCompareAbsLessThan(larger, smaller)) {
      larger = term;
      smaller = sum;
    }
    realAdd(larger, smaller, rounded, &ctxtReal75);
    realSubtract(rounded, larger, residual, &ctxtReal75);
    realSubtract(smaller, residual, residual, &ctxtReal75);
    realAdd(correction, residual, correction, &ctxtReal75);
    realCopy(rounded, sum);
  }


  /********************************************//**
   * \brief Separates a finite product from its common decimal exponent
   *
   * \param[in,out] real real component
   * \param[in,out] imag imaginary component, or NULL
   * \param[in,out] exponent accumulated common exponent
   * \return false if the exponent counter cannot represent the scale
   ***********************************************/
  static bool_t _sumProdNormalise(real_t *real, real_t *imag, int64_t *exponent) {
    int32_t scale;

    if(realIsSpecial(real) || (imag != NULL && realIsSpecial(imag)) || (realIsZero(real) && (imag == NULL || realIsZero(imag)))) {
      return true;
    }
    if(realIsZero(real)) {
      scale = realGetExponent(imag);
    }
    else if(imag == NULL || realIsZero(imag)) {
      scale = realGetExponent(real);
    }
    else {
      const int32_t realExponent = realGetExponent(real);
      const int32_t imagExponent = realGetExponent(imag);
      scale = realExponent > imagExponent ? realExponent : imagExponent;
    }
    if((scale > 0 && *exponent > INT64_MAX - scale) || (scale < 0 && *exponent < INT64_MIN - scale)) {
      displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
      return false;
    }
    if(!realIsZero(real)) {
      real->exponent -= scale;
    }
    if(imag != NULL && !realIsZero(imag)) {
      imag->exponent -= scale;
    }
    *exponent += scale;
    return true;
  }


  /********************************************//**
   * \brief Reapplies a product's common decimal exponent
   *
   * Exponents outside the work context are clamped beyond the Real34 range;
   * final conversion then applies the selected overflow or underflow mode.
   *
   * \param[in,out] value product component
   * \param[in] exponent common exponent
   * \return void
   ***********************************************/
  static void _sumProdRestoreExponent(real_t *value, int64_t exponent) {
    int64_t adjusted, bounded;

    if(realIsSpecial(value) || realIsZero(value)) {
      return;
    }
    adjusted = (int64_t)realGetExponent(value) + exponent;
    bounded = adjusted < (int64_t)ctxtReal75.emax ? adjusted : (int64_t)ctxtReal75.emax;
    bounded = bounded > (int64_t)ctxtReal75.emin ? bounded : (int64_t)ctxtReal75.emin;
    value->exponent += (int32_t)(bounded - realGetExponent(value));
  }


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
    cplx_t        resultX, resultR, correctionR;
    real34_t      loopStep, loopTo, counter, compare, sign, rLoop, moved;
    bool_t        changedOverToComplex = false;
    int64_t       productExponent = 0;
    longInteger_t iLoop;
    fnToReal(NOPARAM);
    if(lastErrorCode != ERROR_NONE) {
      return;
    }
    real34Copy(REGISTER_REAL34_DATA(REGISTER_X), &loopStep);
    fnDrop(NOPARAM);
    fnToReal(NOPARAM);
    if(lastErrorCode != ERROR_NONE) {
      return;
    }
    real34Copy(REGISTER_REAL34_DATA(REGISTER_X), &loopTo);
    fnDrop(NOPARAM);
    fnToReal(NOPARAM);
    if(lastErrorCode != ERROR_NONE) {
      return;
    }
    real34Copy(REGISTER_REAL34_DATA(REGISTER_X), &counter); //Loopfrom
    realCopy(prod ? const_1 : const_0, &resultR.Real);      //Initialize real accumulator
    realSetZero(&resultR.Imag);                                //Initialize complex accumulator
    realSetZero(&correctionR.Real);
    realSetZero(&correctionR.Imag);

    if(real34IsSpecial(&loopStep) || real34IsSpecial(&loopTo) || real34IsSpecial(&counter)) {
      displayCalcErrorMessage(ERROR_BAD_INPUT, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function _programmableSumProd:", "Range controls must be finite", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      return;
    }

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
            realCopy(&resultR.Real, &resultX.Real);
            realCopy(&resultR.Imag, &resultX.Imag);
            if(prod) {
              _sumProdRestoreExponent(&resultX.Real, productExponent);
              _sumProdRestoreExponent(&resultX.Imag, productExponent);
            }
            else {
              realAdd(&resultX.Real, &correctionR.Real, &resultX.Real, &ctxtReal75);
              realAdd(&resultX.Imag, &correctionR.Imag, &resultX.Imag, &ctxtReal75);
            }
            showProgressReal(&resultX, changedOverToComplex);
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

        const bool_t termIsComplex = getRegisterDataType(REGISTER_X) == dtComplex34;
        if(termIsComplex) {
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

        if(termIsComplex) {
          real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &resultX.Real);
          real34ToReal(REGISTER_IMAG34_DATA(REGISTER_X), &resultX.Imag);
        }
        else {
          fnToReal(NOPARAM);
          if(lastErrorCode != ERROR_NONE) {
            break;
          }
          real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &resultX.Real);
          realSetZero(&resultX.Imag);
        }

        #if defined(OPTION_INFSUMS)
        if(inf && early->pass < UINT32_MAX) {               //counted for the complex branch too, and saturates
          early->pass++;
        }
        #endif // OPTION_INFSUMS

        if(!changedOverToComplex) {
          if(prod) {
            realMultiply(&resultR.Real, &resultX.Real, &resultR.Real, &ctxtReal75);
            if(!_sumProdNormalise(&resultR.Real, NULL, &productExponent)) {
              break;
            }
          }
          else {
            _sumProdAdd(&resultR.Real, &correctionR.Real, &resultX.Real);
            #if defined(OPTION_INFSUMS)
            if(inf) {
              realCopy(&resultX.Real, &early->term);        //the term itself, which outlives a saturated total
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
                  realAdd(&resultR.Real, &correctionR.Real, &resultX.Real, &ctxtReal75);
                  realDivide(&resultX.Real, &early->scale, &early->allowance, &ctxtReal75);  //one last digit of the answer
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
          if(prod) {
            mulComplexComplex(&resultR, &resultX, &resultR, &ctxtReal75);
            if(!_sumProdNormalise(&resultR.Real, &resultR.Imag, &productExponent)) {
              break;
            }
          }
          else {
            _sumProdAdd(&resultR.Real, &correctionR.Real, &resultX.Real);
            _sumProdAdd(&resultR.Imag, &correctionR.Imag, &resultX.Imag);
          }
        }



        #if defined(VERBOSE_COUNTER)
          printf(">>> Fin: %d, Cpx: %d ", finished, changedOverToComplex);
          printReal34ToConsole(&counter, " Cnt: ", " ");
          printRealToConsole(&resultX.Real, " X: ", " ");
          if(changedOverToComplex) {
            printRealToConsole(&resultX.Imag, " Xi: ", " ");
          }
          printRealToConsole(&resultR.Real, " SUM: ", "");
          if(changedOverToComplex) {
            printRealToConsole(&resultR.Imag, " SUMii: ", " ");
          }
          printf("\n");
        #endif // VERBOSE_COUNTER

        if(finished == 0) {
          break;
        }
        real34Add(&counter, &loopStep, &moved);
        if(real34CompareEqual(&moved, &counter)) {
          displayCalcErrorMessage(ERROR_STEP_OF_ZERO, ERR_REGISTER_LINE);
          break;
        }
        real34Copy(&moved, &counter);
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
        if(prod) {
          _sumProdRestoreExponent(&resultR.Real, productExponent);
          _sumProdRestoreExponent(&resultR.Imag, productExponent);
        }
        else {
          realAdd(&resultR.Real, &correctionR.Real, &resultR.Real, &ctxtReal75);
          realAdd(&resultR.Imag, &correctionR.Imag, &resultR.Imag, &ctxtReal75);
        }
        if(!changedOverToComplex) {
          if(getRegisterDataType(REGISTER_X) != dtReal34) {
            reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
          }
          convertRealToReal34ResultRegister(&resultR.Real, REGISTER_X);
        }
        else {
          if(getRegisterDataType(REGISTER_X) != dtComplex34) {
            reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
          }
          convertRealToReal34ResultRegister(&resultR.Real, REGISTER_X);
          convertRealToImag34ResultRegister(&resultR.Imag, REGISTER_X);
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
