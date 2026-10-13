// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors


/********************************************//**
 * \file solve.c
 ***********************************************/

#include "c47.h"


// This mdule as two control defines, controlled from defines.h:
//   OPTION_TVM_FORMULAS
//   OPTION_TVM_NEWTON


#define  SOLVERDEBUG // only progress indicators
#undef  SOLVERDEBUG
#define SOLVERDEBUG2 // more details
#undef SOLVERDEBUG2

#if defined(TESTSUITE_BUILD)
  #undef SOLVERDEBUG
  #undef SOLVERDEBUG2
#endif // TESTSUITE_BUILD

//The main real solver is converted to 39 digit operation internally, with a 39-digit interface to TVM, and the legacy 34-digit interface to the other legacy callers
#define ctxtSolver   ctxtReal39
#define ctxtSolverHi ctxtReal39
//The 39-digit solver tolerances must be lower in order to leverage the guard digits
#define solverTvmTol 34
#define solverTvmZer (solverTvmTol + 1)




// True when a further PLOT, INT or SOLVE may not start. INT and SOLVE nest to MAX_ENGINE_NESTING_DEPTH. PLOT runs only as the outermost engine, because the draw
// matrix DrwMX, its register regStatsXY and the screen are single, and where PLOT_NESTING_ALLOWED is 0 nothing at all runs inside a plot. Each engine calls this at
// its entry, ahead of that engine's own saveForUndo: the undo buffer is one global, and an engine refused after saving over it leaves the engine one level out
// restoring an image that no longer matches its registers. PGM_WAITING carries the stop across the unwind and engineNestingWasRefused carries the reason, both of
// them durable where lastErrorCode is not: FLAG_IGN1ER wipes it, so an error raised here never reaches the engine that reports the abort. The standing undo image
// is disarmed because adjustResult replays that image on any error code.
bool_t engineNestingRefused(bool_t isPlot) {
  if(isPlot ? (engineNestingDepth == 0)
            : (engineNestingDepth < MAX_ENGINE_NESTING_DEPTH && (PLOT_NESTING_ALLOWED || plotEngineActive == 0))) {
    engineNestingWasRefused = false;
    return false;
  }
  programRunStop = PGM_WAITING;
  thereIsSomethingToUndo = false;
  engineNestingWasRefused = true;
  return true;
}


void fnPgmSlv(uint16_t label) {
  label = findProgramLabel(label, "In function fnPgmSlv:");
  if(label != INVALID_VARIABLE) {
    currentSolverProgram = label - FIRST_LABEL;
    currentSolverStatus &= ~SOLVER_STATUS_USES_FORMULA;
  }
}


// ====== === temporarily / permanently use the PGMSOLVE's "currentSolverProgram" for plotting
void fnPgmPlt(uint16_t label) {
  fnPgmSlv(label);
}

void fnSolve(uint16_t labelOrVariable) {
  if((FIRST_UC_LOCAL_LABEL <= labelOrVariable && labelOrVariable <= LAST_LOCAL_LABEL) || (FIRST_LABEL <= labelOrVariable && labelOrVariable <= LAST_LABEL) || (REGISTER_X <= labelOrVariable && labelOrVariable <= REGISTER_T)) {
    // Interactive mode
    fnPgmSlv(labelOrVariable);
    if(lastErrorCode == ERROR_NONE) {
      currentSolverStatus = SOLVER_STATUS_INTERACTIVE;
    }
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
  else if(!(currentSolverStatus & SOLVER_STATUS_USES_FORMULA) && (FIRST_NAMED_VARIABLE <= labelOrVariable && labelOrVariable <= LAST_NAMED_VARIABLE) && currentSolverProgram >= numberOfLabels) {
    displayCalcErrorMessage(ERROR_NO_PROGRAM_SPECIFIED, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "label %u not found", labelOrVariable);
      moreInfoOnError("In function fnSolve:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
  else if(FIRST_NAMED_VARIABLE <= labelOrVariable && labelOrVariable <= LAST_NAMED_VARIABLE) {
    // Execute
    real34_t z, y, x;
    real_t tmp;
    int resultCode = 0;

    if(engineNestingRefused(false)) {
      return;
    }
    if(getRegisterAsReal34Quiet(REGISTER_Y, &y) && getRegisterAsReal34Quiet(REGISTER_X, &x)) {
      fnDrop(NOPARAM);
      fnDrop(NOPARAM);
      saveForUndo(); //repeat after dropping the input parameters
      currentSolverVariable = labelOrVariable;
      screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;
      refreshScreen(0);
      resultCode = solver(labelOrVariable, &y, &x, &z, &y, &x);

      fnUndo(0);
      liftStack();
      setSystemFlag(FLAG_ASLIFT);
      liftStack();
      setSystemFlag(FLAG_ASLIFT);
      liftStack();
      setSystemFlag(FLAG_ASLIFT);
      liftStack();
      real34ToReal(&z, &tmp), convertRealToReal34ResultRegister(&tmp, REGISTER_Z);
      real34ToReal(&y, &tmp), convertRealToReal34ResultRegister(&tmp, REGISTER_Y);
      real34ToReal(&x, &tmp), convertRealToReal34ResultRegister(&tmp, REGISTER_X);
      int32ToReal34(resultCode, REGISTER_REAL34_DATA(REGISTER_T));
      #if (defined PC_BUILD) && (defined SOLVERDEBUG2)
        printf("Close ");
        printRegisterToConsole(REGISTER_X, "X=", " ");
        printRegisterToConsole(REGISTER_Y, "Xold=", "\n");
      #endif
      switch(resultCode) {
        case SOLVER_RESULT_NORMAL: {
          copySourceRegisterToDestRegister(REGISTER_X, labelOrVariable);
          temporaryInformation = TI_SOLVER_VARIABLE_RESULT;
          lastErrorCode = ERROR_NONE;
          break;
        }
        case SOLVER_RESULT_SIGN_REVERSAL: {
          temporaryInformation = TI_SOLVER_FAILED;
          displayCalcErrorMessage(ERROR_LARGE_DELTA_AND_OPPOSITE_SIGN, ERR_REGISTER_LINE);
          break;
        }
        case SOLVER_RESULT_EXTREMUM: {
          temporaryInformation = TI_SOLVER_FAILED;
          displayCalcErrorMessage(ERROR_SOLVER_REACHED_LOCAL_EXTREMUM, ERR_REGISTER_LINE);
          break;
        }
        case SOLVER_RESULT_BAD_GUESS: {
          temporaryInformation = TI_SOLVER_FAILED;
          displayCalcErrorMessage(ERROR_INITIAL_GUESS_OUT_OF_DOMAIN, ERR_REGISTER_LINE);
          break;
        }
        case SOLVER_RESULT_CONSTANT: {
          temporaryInformation = TI_SOLVER_FAILED;
          displayCalcErrorMessage(ERROR_FUNCTION_VALUES_LOOK_CONSTANT, ERR_REGISTER_LINE);
          break;
        }
        case SOLVER_RESULT_OTHER_FAILURE: {
          temporaryInformation = TI_SOLVER_FAILED;
          displayCalcErrorMessage(ERROR_NO_ROOT_FOUND, ERR_REGISTER_LINE);
          break;
        }
        case SOLVER_RESULT_ABORTED: {
          temporaryInformation = TI_SOLVER_FAILED;
          displayCalcErrorMessage(ERROR_SOLVER_ABORT, ERR_REGISTER_LINE);
          programRunStop = PGM_WAITING;   // R/S halts the whole program on the SOLVE step
          break;
        }
      }
      adjustResult(REGISTER_X, false, false, REGISTER_X, REGISTER_Y, -1);


    }
    else {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType(REGISTER_X));
        moreInfoOnError("In function fnSolve:", errorMessage, "is not a real number.", "");
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }
  else {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "unexpected parameter %u", labelOrVariable);
      moreInfoOnError("In function fnSolve:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
}


// ====== === temporarily / permanently use the SOLVE's "currentSolverVariable" for plotting

void fnMvarPlot(uint16_t labelOrVariable) {
  if((FIRST_UC_LOCAL_LABEL <= labelOrVariable && labelOrVariable <= LAST_LOCAL_LABEL) || (FIRST_LABEL <= labelOrVariable && labelOrVariable <= LAST_LABEL) || (REGISTER_X <= labelOrVariable && labelOrVariable <= REGISTER_T)) {
    // Interactive mode
    fnPgmSlv(labelOrVariable);
    if(lastErrorCode == ERROR_NONE) {
      currentSolverStatus = SOLVER_STATUS_INTERACTIVE | SOLVER_STATUS_RPN_GRAPHER;      //Full assignment, clearing not needed: currentSolverStatus &= ~SOLVER_STATUS_EQUATION_GRAPHER;
    }
  }
  else if(!(currentSolverStatus & SOLVER_STATUS_USES_FORMULA) && (FIRST_NAMED_VARIABLE <= labelOrVariable && labelOrVariable <= LAST_NAMED_VARIABLE) && currentSolverProgram >= numberOfLabels) {
    displayCalcErrorMessage(ERROR_NO_PROGRAM_SPECIFIED, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "label %u not found", labelOrVariable);
      moreInfoOnError("In function fnMvarPlot:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
  else if(FIRST_NAMED_VARIABLE <= labelOrVariable && labelOrVariable <= LAST_NAMED_VARIABLE) {
    // Execute
    real34_t xl, xh;
    if(engineNestingRefused(true)) {
      return;
    }
    if(getRegisterAsReal34Quiet(REGISTER_Y, &xl) && getRegisterAsReal34Quiet(REGISTER_X, &xh)) {
      saveForUndo(); //repeat after dropping the input parameters
      currentSolverVariable = labelOrVariable;
      screenUpdatingMode &= ~SCRUPD_MANUAL_MENU;
      refreshScreen(0);
      currentSolverStatus |= SOLVER_STATUS_RPN_GRAPHER;// | SOLVER_STATUS_INTERACTIVE | SOLVER_STATUS_READY_TO_EXECUTE;
      currentSolverStatus &= ~(SOLVER_STATUS_EQUATION_GRAPHER);
      #if defined(GRAPHDEBUG_MIN)
        printf("LAUNCH PLOT, currentSolverProgram:%d currentSolverVariable=%d currentSolverStatus=%d\n", currentSolverProgram, currentSolverVariable, currentSolverStatus);
      #endif //GRAPHDEBUG_MIN
      fnPlotf(NOPARAM); // uses currentSolverVariable=labelOrVariable & 

      //printf("DONE calcMode=%d\n",calcMode);
      //fnSNAP(NOPARAM);                // !!! NOT PUTTING IT HERE: BEST NOT FOR PROGRAMABILITY. USER CAN SNAP.
      //showSoftmenu(-MNU_PLOT_FUNC);   // !!! NOT PUTTING IT HERE: BEST NOT FOR PROGRAMABILITY. USER TO OPEN INTERACTIVE MODE MENU 'PLFUNC'.
      fnUndo(0);

    }
    else {
      displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "DataType %" PRIu32, getRegisterDataType(REGISTER_X));
        moreInfoOnError("In function fnMvarPlot:", errorMessage, "is not a real number.", "");
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
    }
  }
  else {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "unexpected parameter %u", labelOrVariable);
      moreInfoOnError("In function fnMvarPlot:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
}



void fnSolveVar(uint16_t unusedButMandatoryParameter) {
    printStatus(1, errorMessageOf(REAL_SOLVER), force);
    const dynamicSoftmenu_t *dynamic = currentDynamicSoftmenu();
    if(dynamic == NULL || dynamicMenuItem < 0 || dynamicMenuItem >= dynamic->numItems) {
      return;
    }
    const char *var = (char *)getNthString(dynamic->menuContent, dynamicMenuItem);
    const uint16_t regist = findOrAllocateNamedVariable(var);
    const uint16_t nameLength = stringByteLength(var) + 1;
    if(currentMvarLabel != INVALID_VARIABLE) {
      if(currentSolverStatus & SOLVER_STATUS_INTERACTIVE) { // MNU_MVAR was displayed by the Solver
        reallyRunFunction(ITM_STO, regist);
      }
      else {  // MNU_MVAR was displayed by VARMNU
        if(entryStatus & 0x01) { // MVAR menu key pressed after a user entry: save the value in the variable
          entryStatus &= 0xfe;
          currentSolverVariable = regist;
          reallyRunFunction(ITM_STO, regist);
          temporaryInformation = TI_SOLVER_VARIABLE;
        }
        else { // MVAR menu key pressed without a a user entry: store the variable name in the alpha register and continue program execution
          const uint16_t alpha_register = (varMenu42 ? alphaRegister : REGISTER_K);
          reallocateRegister(alpha_register, dtString, TO_BLOCKS(nameLength) , amNone);
          xcopy(REGISTER_STRING_DATA(alpha_register), var, nameLength);
          dynamicMenuItem = -1;
          runProgram(false, INVALID_VARIABLE);
        }
      }
    }
    else if((currentSolverStatus & SOLVER_STATUS_EQUATION_MODE) == SOLVER_STATUS_EQUATION_1ST_DERIVATIVE || (currentSolverStatus & SOLVER_STATUS_EQUATION_MODE) == SOLVER_STATUS_EQUATION_2ND_DERIVATIVE) {
      if(compareString(var, STD_delta STD_SUB_d, CMP_NAME) == 0) {   // the step key: it takes a value like any other, but it is not what the derivative is taken against
        real_t given;

        if(getRegisterAsRealQuiet(REGISTER_X, &given) && realIsInfinite(&given)) {
          entryStatus &= 0xfe;
          fnDeleteVariable(regist);   // an infinite step is no step: the variable goes, which is the only way to be rid of it, and the engine picks the step again
        }
        else if(entryStatus & 0x01) {
          entryStatus &= 0xfe;
          reallyRunFunction(ITM_STO, regist);
        }
        temporaryInformation = TI_DERIV_STEP;   // names the step itself: the selection is untouched, so TI_SOLVER_VARIABLE would name the variable last chosen
        return;
      }
      currentSolverVariable = regist;
      if(entryStatus & 0x01) { // MVAR menu key pressed after a user entry: save the value in the variable
        entryStatus &= 0xfe;
        reallyRunFunction(ITM_STO, regist);
        temporaryInformation = TI_SOLVER_VARIABLE;
      }
      else { // MVAR menu key pressed without a user entry: the value stands, so differentiate with respect to this variable
        if((currentSolverStatus & SOLVER_STATUS_EQUATION_MODE) == SOLVER_STATUS_EQUATION_1ST_DERIVATIVE) {
          fn1stDerivEq(NOPARAM);
        }
        else {
          fn2ndDerivEq(NOPARAM);
        }
      }
    }
    else if(currentSolverStatus & SOLVER_STATUS_READY_TO_EXECUTE) {
      if(currentSolverStatus & SOLVER_STATUS_RPN_GRAPHER) {
        reallyRunFunction(ITM_PLTf, regist);
      } else {
        reallyRunFunction(ITM_SOLVE, regist);
      }
    }
    else {
      currentSolverVariable = regist;
      reallyRunFunction(ITM_STO, regist);
      currentSolverStatus |= SOLVER_STATUS_READY_TO_EXECUTE;
      temporaryInformation = TI_SOLVER_VARIABLE;
    }
}

static void _executeSolver(calcRegister_t variable, const real34_t *val, real34_t *res) {
  const uint8_t engineRoundingMode = roundingMode;  // RM_ENGINE inside solver(), RM for a plot sample
  reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
  real34Copy(val, REGISTER_REAL34_DATA(REGISTER_X));
  if(currentSolverStatus & SOLVER_STATUS_TVM_APPLICATION) {
    copySourceRegisterToDestRegister(REGISTER_X, variable);
  }
  else {
    reallyRunFunction(ITM_STO, variable);
    fnFillStack(NOPARAM);
  }
  if(lastErrorCode == ERROR_SOLVER_ABORT) {
    realToReal34(const_NaN, res);
    return;
  }
  if(currentSolverStatus & SOLVER_STATUS_USES_FORMULA) {
    parseEquation(currentFormula, EQUATION_PARSER_XEQ, tmpString, tmpString + AIM_BUFFER_LENGTH);
  }
  else {
    // also stack the variable + control flags so a nested SOLVE cannot leak into the outer point (enables PLOT(SOLVE), SOLVE(SOLVE))
    uint16_t savedCurrentSolverProgram = currentSolverProgram;
    uint16_t savedCurrentSolverVariable = currentSolverVariable;
    uint16_t savedCurrentSolverStatus = currentSolverStatus;
    dynamicMenuItem = -1;
    execProgram(currentSolverProgram + FIRST_LABEL);
    currentSolverProgram = savedCurrentSolverProgram;
    currentSolverVariable = savedCurrentSolverVariable;
    currentSolverStatus = savedCurrentSolverStatus;
  }
  setRoundingMode(engineRoundingMode);  // a mode the function sets does not reach the engine
  if(lastErrorCode == ERROR_OVERFLOW_PLUS_INF) {
    realToReal34(const_plusInfinity, res);
    lastErrorCode = ERROR_NONE;
  }
  else if(lastErrorCode == ERROR_OVERFLOW_MINUS_INF) {
    realToReal34(const_minusInfinity, res);
    lastErrorCode = ERROR_NONE;
  }
  else if(lastErrorCode != ERROR_NONE) {
    realToReal34(const_NaN, res);
  }
  else if(getRegisterAsReal34Quiet(REGISTER_X, res)) {
    ;
  }
  else if(getRegisterDataType(REGISTER_X) == dtComplex34 && real34IsZero(REGISTER_REAL34_DATA(REGISTER_X))) {
    real34Copy(REGISTER_IMAG34_DATA(REGISTER_X), res);
    real34ChangeSign(res);
  }
  else {
    realToReal34(const_NaN, res);
  }
}

void _executeSolverReal(calcRegister_t variable, const real_t *val, real_t *res, real_t *deriv) {
  if(currentSolverStatus & SOLVER_STATUS_TVM_APPLICATION) {
    // pass real_t value via ioVal, result comes back in same variable as real_t
    realCopy(val, res);
    tvmEquation(variable, res, deriv);
  }
  else {
    real34_t val34, res34;
    realToReal34(val, &val34);
    _executeSolver(variable, &val34, &res34);
    real34ToReal(&res34, res);
  }
}


  static void _linearInterpolation(const real_t *a, const real_t *b, const real_t *fa, const real_t *fb, real_t *res, real_t *slope, realContext_t *realContext) {
    real_t amb, famfb;
    realSubtract(a, b, &amb, realContext);
    realSubtract(fa, fb, &famfb, realContext);
    if(slope) {
      realDivide(&famfb, &amb, slope, realContext);
    }
    if(res) {
      realDivide(&amb, &famfb, &amb, realContext);
      realMultiply(&amb, fb, &amb, realContext);
      realSubtract(b, &amb, res, realContext);
    }
  }

  static void _inverseQuadraticInterpolation(const real_t *a, const real_t *b, const real_t *c, const real_t *fa, const real_t *fb, const real_t *fc, real_t *res, realContext_t *realContext) {
    real_t val, num, den, tmp;

    realMultiply(fb, fc, &num, realContext);
    realMultiply(&num, a, &num, realContext);
    realSubtract(fa, fb, &den, realContext);
    realSubtract(fa, fc, &tmp, realContext);
    realMultiply(&den, &tmp, &den, realContext);
    realDivide(&num, &den, &val, realContext);

    realMultiply(fa, fc, &num, realContext);
    realMultiply(&num, b, &num, realContext);
    realSubtract(fb, fa, &den, realContext);
    realSubtract(fb, fc, &tmp, realContext);
    realMultiply(&den, &tmp, &den, realContext);
    realDivide(const_1, &den, &den, realContext);
    realFMA(&num, &den, &val, &val, realContext);

    realMultiply(fa, fb, &num, realContext);
    realMultiply(&num, c, &num, realContext);
    realSubtract(fc, fa, &den, realContext);
    realSubtract(fc, fb, &tmp, realContext);
    realMultiply(&den, &tmp, &den, realContext);
    realDivide(const_1, &den, &den, realContext);
    realFMA(&num, &den, &val, res, realContext);
  }



  static void _showProgress(const real34_t *a, const real34_t *b, const real34_t *fa, const real34_t *fb) {
    #if ENABLE_SOLVER_PROGRESS == 1
        const real34_t *c;
        if((currentSolverStatus & (SOLVER_STATUS_TVM_APPLICATION & SOLVER_STATUS_USES_FORMULA)) == 0 && currentSolverNestingDepth == 1 ) { // programRunStop != PGM_RUNNING) { //proposed omission to make progress monitoring while in program running, it can be switched off with MONIT. Not final.
          uint8_t savedDisplayFormatDigits = displayFormatDigits;

          if(real34CompareGreaterThan(a, b)) {
            c = a;
            a  = b;
            b  = c;
            c = fa;
            fa = fb;
            fb = c;
          }

          clearRegisterLine(REGISTER_Z, true, true);
          clearRegisterLine(REGISTER_Y, true, true);
          clearRegisterLine(REGISTER_X, true, true);

          displayFormatDigits = displayFormat == DF_ALL ? 0 : 33;
          real34ToDisplayString(a, amNone, tmpString, &standardFont, 9999, 34, !LIMITEXP, FRONTSPACE, NOIRFRAC);
          showString(tmpString, &standardFont, 1, Y_POSITION_OF_REGISTER_Y_LINE + 6, vmNormal, true, true);
          showString(real34IsSpecial(fa) ? "?" : real34IsZero(fa) ? "" : real34IsPositive(fa) ? "+" : "-", &standardFont, SCREEN_WIDTH - 10 /* width of '+' */, Y_POSITION_OF_REGISTER_Y_LINE + 6, vmNormal, true, true);
          real34ToDisplayString(b, amNone, tmpString, &standardFont, 9999, 34, !LIMITEXP, FRONTSPACE, NOIRFRAC);
          showString(tmpString, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
          showString(real34IsSpecial(fb) ? "?" : real34IsZero(fb) ? "" : real34IsPositive(fb) ? "+" : "-", &standardFont, SCREEN_WIDTH - 10 /* width of '+' */, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
          displayFormatDigits = savedDisplayFormatDigits;

        #if defined(DMCP_BUILD)
          lcd_refresh();
        #endif //DMCP_BUILD
      }
    #endif // ENABLE_SOLVER_PROGRESS == 1
  }




int solver(calcRegister_t variable, const real34_t *y, const real34_t *x, real34_t *resZ, real34_t *resY, real34_t *resX) {
  currentKeyCode = 255;
  const uint8_t userRoundingMode = roundingMode;
  setRoundingMode(RM_ENGINE);                                                  // the iteration takes no mode from RM

    typedef enum {
      SOLVER_METHOD_BRENT,
      SOLVER_METHOD_NEWTON
    } solverMethod_t;
    solverMethod_t currentMethod = SOLVER_METHOD_BRENT;
    #if defined(OPTION_TVM_NEWTON)
      bool_t newton_polish_mode = false;
      int newton_polish_count = 0;
      bool_t first_newton_iter = true;
      int stall_count = 0;
      int newton_iter_count = 0;
      bool_t newtonDisabled = false;
    #endif //OPTION_TVM_NEWTON
    bool_t newtonInitialized = false;
    int result = SOLVER_RESULT_NORMAL;

    real34_t antiLevel34;
    // The working reals come from the heap, not the frame: thirty-seven decNumbers at 60 bytes, and the frame stands for the whole solve while every evaluation of the user
    // program or formula runs below it. Measured on the DM42 build the frame falls from 1952 bytes to 552. The reals declared inside the iteration loop are taken here as
    // well, so the loop allocates nothing; each is still written before it is read on every pass. Freed on every exit through freeWork.
    REAL_T_ALLOC(aa,                75);
    REAL_T_ALLOC(bb,                75);
    REAL_T_ALLOC(bb1,               75);
    REAL_T_ALLOC(bb2,               75);
    REAL_T_ALLOC(faa,               75);
    REAL_T_ALLOC(fbb,               75);
    REAL_T_ALLOC(fbb1,              75);
    REAL_T_ALLOC(mm,                75);
    REAL_T_ALLOC(ss,                75);
    REAL_T_ALLOC(secantSlopeA,      75);
    REAL_T_ALLOC(secantSlopeB,      75);
    REAL_T_ALLOC(delta,             75);
    REAL_T_ALLOC(deltaB,            75);
    REAL_T_ALLOC(smb,               75);
    REAL_T_ALLOC(tol,               75);
    REAL_T_ALLOC(fbp1,              75);
    REAL_T_ALLOC(tmp,               75);
    REAL_T_ALLOC(tolAlmostZero,     75);
    REAL_T_ALLOC(tol1,              75);
    REAL_T_ALLOC(minBracketSpacing, 75);
    REAL_T_ALLOC(prevResX,          75);
    REAL_T_ALLOC(antiLevel,         75);
    REAL_T_ALLOC(bracketWidth,      75);
    REAL_T_ALLOC(resXr,             75);
    REAL_T_ALLOC(resZr,             75);
    #if defined(OPTION_TVM_NEWTON)
      REAL_T_ALLOC(newton_x,        75);
      REAL_T_ALLOC(prev_fx,         75);
      REAL_T_ALLOC(prev_x,          75);
      REAL_T_ALLOC(brent_best_x,    75);
      REAL_T_ALLOC(brent_best_fx,   75);
      REAL_T_ALLOC(relativeWidth,   75);
      REAL_T_ALLOC(fullBracket,     75);
      REAL_T_ALLOC(newton_trial,    75);
      REAL_T_ALLOC(newton_fx,       75);
      REAL_T_ALLOC(newton_deriv,    75);
      REAL_T_ALLOC(newton_step,     75);
      REAL_T_ALLOC(tol_converged,   75);
    #endif //OPTION_TVM_NEWTON

    if(aa == NULL || bb == NULL || bb1 == NULL || bb2 == NULL || faa == NULL || fbb == NULL || fbb1 == NULL || mm == NULL || ss == NULL || secantSlopeA == NULL ||
       secantSlopeB == NULL || delta == NULL || deltaB == NULL || smb == NULL || tol == NULL || fbp1 == NULL || tmp == NULL || tolAlmostZero == NULL || tol1 == NULL ||
       minBracketSpacing == NULL || prevResX == NULL || antiLevel == NULL || bracketWidth == NULL || resXr == NULL || resZr == NULL
       #if defined(OPTION_TVM_NEWTON)
         || newton_x == NULL || prev_fx == NULL || prev_x == NULL || brent_best_x == NULL || brent_best_fx == NULL || relativeWidth == NULL || fullBracket == NULL ||
         newton_trial == NULL || newton_fx == NULL || newton_deriv == NULL || newton_step == NULL || tol_converged == NULL
       #endif //OPTION_TVM_NEWTON
       ) {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      result = SOLVER_RESULT_OTHER_FAILURE;
      goto freeWork;
    }

    #if defined(OPTION_TVM_NEWTON)
      realSetNaN(brent_best_x);
      realSetNaN(brent_best_fx);
    #endif //OPTION_TVM_NEWTON
    real_t *bp1 = mm;
    bool_t extendRange = false;
    bool_t originallyLevel = false;
    bool_t extremum = false;
    bool_t was_inting = getSystemFlag(FLAG_INTING);
    int loop = 0;
    int16_t getOutOfLevel = 17;
    bool_t fbIsAlmostZero = false;
    realSetNaN(prevResX);

    if(currentSolverStatus & SOLVER_STATUS_TVM_APPLICATION) {
      realSetOne(tol);
      tol->exponent -= (significantDigits == 0 || significantDigits == 34) ? solverTvmTol : significantDigits;

      realSetOne(tolAlmostZero);
      tolAlmostZero->exponent -= (significantDigits == 0 || significantDigits == 34) ? solverTvmZer : (significantDigits + 1);

      realSetOne(minBracketSpacing);
      minBracketSpacing->exponent -= (significantDigits == 0 || significantDigits == 34) ? solverTvmTol : significantDigits;
    }
    else if(graphAccActive) {
      // graphAccActive (true only inside execute_rpn_function_graphAcc) means contexts are narrowed to significantDigitsForEqnGraphs+3.
      // follow the grapher: converge to graph precision. Set/cleared with the reduction.
      realSetOne(tol);
      tol->exponent -= significantDigitsForEqnGraphs;
      realSetOne(minBracketSpacing);
      minBracketSpacing->exponent -= significantDigitsForEqnGraphs;
      realCopy(const_1e_34, tolAlmostZero);   // residual floor stays at real34 precision
    }
    else {
      convergenceTolerence(tol);
      realCopy(const_1e_34, tolAlmostZero);
      realCopy(const_1e_32, minBracketSpacing);
    }

    if(engineNestingRefused(false)) {                            // ahead of the flags and both depth counters
      result = SOLVER_RESULT_ABORTED;
      goto freeWork;
    }
    ++engineNestingDepth;
    ++currentSolverNestingDepth;
    setSystemFlag(FLAG_SOLVING);
    clearSystemFlag(FLAG_INTING);

    realSetZero(delta);

    real34ToReal(y, aa);
    real34ToReal(y, bb1);
    real34ToReal(x, bb);
    realSetNaN(bb2);


    //determine the tolerance (ULP) with which levelness will be detected
    real34NextPlus(x, &antiLevel34);
    if(real34IsInfinite(&antiLevel34)) {
      real34NextMinus(x, &antiLevel34);
      real34Subtract(x, &antiLevel34, &antiLevel34);
    }
    else {
      real34Subtract(x, &antiLevel34, &antiLevel34);
    }


    if(realCompareEqual(bb, aa)) {                              //try solve the originallyLevel problem by forcing the inputs marginally not equal, causing the originallyLevel flag not to be set.
retryLevel:
      if(--getOutOfLevel >= 0) {
        #if (defined PC_BUILD) && (defined SOLVERDEBUG2)
          printf("Solver retry Level:%2i ", getOutOfLevel);
          printReal34ToConsole(&antiLevel34, " antiLevel34:", "\n");
        #endif //PC_BUILD
        real34Multiply(&antiLevel34, const34_153, &antiLevel34);  //increase it for the next round so long
        real34Minus(&antiLevel34, &antiLevel34);                  //let the increment be 2 orders of magnitude larger, and flip sign so we can cover negatives equally well.
        real34ToReal(&antiLevel34, antiLevel);
        if(real34IsPositive(&antiLevel34)) {
          realAdd(bb, antiLevel, bb, &ctxtSolver);            //Add this value to the right hand starting value
        }
        else {
          realAdd(aa, antiLevel, aa, &ctxtSolver);            //Add this value to the left hand starting value
          realAdd(bb1, antiLevel, bb1, &ctxtSolver);          //Add this value to the left hand starting value
        }
      }
    }
    #if (defined PC_BUILD) && (defined SOLVERDEBUG2)
      printf("Start %d  ", loop);
      printRealToConsole(aa, "  a=", " ");
      printRealToConsole(bb, "b=", "\n");
    #endif

    realSubtract(bb, aa, ss, &ctxtSolver);
    if(realCompareAbsLessThan(ss, minBracketSpacing)) {
      realCopy(minBracketSpacing, ss);
      if(realCompareLessThan(bb, aa)) {
        realSetNegativeSign(ss);
      }
      realSubtract(aa, ss, aa, &ctxtSolver);
      realAdd(bb, ss, bb, &ctxtSolver);
    }

    _executeSolverReal(variable, bb1, fbb1, NULL);
    if(lastErrorCode != ERROR_NONE) {
      result = SOLVER_RESULT_BAD_GUESS;
    }
    realCopy(fbb1, faa);

    // calculation
    _executeSolverReal(variable, bb, fbb, NULL);
    //printRealToConsole(bb1, "JJ2: f(&b1=",")  ");
    //printRealToConsole(fbb1, "", "\n");
    //printRealToConsole(bb, "JJ2: f(&b=", ")  ");
    //printRealToConsole(fbb, "", "\n");

    if(lastErrorCode != ERROR_NONE) {
      result = SOLVER_RESULT_BAD_GUESS;
    }

    if(realIsSpecial(fbb) || realIsSpecial(fbb1)) {
      result = SOLVER_RESULT_BAD_GUESS;
    }
    else if(realIsNegative(fbb) == realIsNegative(fbb1)) {
      extendRange = true;
    }

    if(realCompareEqual(fbb, fbb1)) {
      if(getOutOfLevel >= 0) {             //try solve the originallyLevel problem by forcing the inputs marginally not equal, causing the originallyLevel flag not to be set.
        goto retryLevel;
      }
      else {
        originallyLevel = true;
      }
    }

    if(realCompareAbsLessThan(faa, fbb)) {
      realCopy(bb, tmp);
      realCopy(aa, bb);
      realCopy(tmp, aa);
      realCopy(tmp, bb1);
      realCopy(fbb, tmp);
      realCopy(faa, fbb);
      realCopy(tmp, faa);
      realCopy(tmp, fbb1);
    }

    if(realIsZero(faa) || realIsZero(fbb)) { // already is a root?
      --engineNestingDepth;
      if((--currentSolverNestingDepth) == 0) {
        clearSystemFlag(FLAG_SOLVING);
      }
      else if(was_inting) {
        clearSystemFlag(FLAG_SOLVING);
        setSystemFlag(FLAG_INTING);
      }
      real34SetZero(resZ);
      real34SetZero(resY);
      realToReal34(realIsZero(faa) ? aa : bb, resX);

      reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
      real34Copy(resX, REGISTER_REAL34_DATA(REGISTER_X));
      copySourceRegisterToDestRegister(REGISTER_X, variable);

      result = SOLVER_RESULT_NORMAL;
      goto freeWork;
    }




    // =========== ITERATION START =============
    do {

      #if (defined PC_BUILD) && ((defined SOLVERDEBUG) || (defined SOLVERDEBUG2))
        const char* methodName[] = {"BRENT", "NEWTON"};
        if(currentMethod == SOLVER_METHOD_NEWTON && newtonInitialized) {
          #if defined(OPTION_TVM_NEWTON)
            char str_x[200];
            realToString(newton_x, str_x);
            printf("Iter %-4d (%-6s)  x=%s\n", loop, methodName[currentMethod], str_x);
          #endif //OPTION_TVM_NEWTON
        }
        else {
          char str_a[200], str_b[200];
          #if defined(SOLVERDEBUG2)
            char str_fa[200], str_fb[200];
            realToString(faa, str_fa);
            realToString(fbb, str_fb);
          #endif
          realToString(aa, str_a);
          realToString(bb, str_b);

          printf("Iter %-4d (%-6s)  a=%-50s b=%s", loop, methodName[currentMethod], str_a, str_b);
          #if defined(SOLVERDEBUG2)
            printf("\n%65sfa=%-50s fb=%s", "", str_fa, str_fb);
          #endif
          printf("\n");
        }
      #endif // PC_BUILD && (SOLVERDEBUG || SOLVERDEBUG2)

      loop++;
      if(checkHalfSec()) {
        char ss[10];
        strcpy(ss, "Iter: ");
        ss[4] = currentMethod == SOLVER_METHOD_BRENT ? ':' : '=';
        if(progressHalfSecUpdate_Integer(timed, ss, loop, halfSec_clearZ, halfSec_clearT, halfSec_disp)) { //timed
          real34_t a34, b34, fa34, fb34;
          realToReal34(aa, &a34);
          realToReal34(bb, &b34);
          realToReal34(faa, &fa34);
          realToReal34(fbb, &fb34);
          _showProgress(&a34, &b34, &fa34, &fb34);
        }
      }

      if(exitKeyWaiting() || (programRunStop == PGM_WAITING)) {   // key/EXIT, or a nested engine already aborted (PGM_WAITING survives, lastErrorCode does not)
          progressHalfSecUpdate_Integer(force+1, "Interrupted Iter:", loop, halfSec_clearZ, halfSec_clearT, halfSec_disp);
          programRunStop = PGM_WAITING;
          displayCalcErrorMessage(ERROR_SOLVER_ABORT, REGISTER_T);
        break;
      }

      // pre-calculation
      if(realIsSpecial(bb2)) {
        realSubtract(bb, bb1, deltaB, &ctxtSolverHi);
      }
      else {
        realSubtract(bb1, bb2, deltaB, &ctxtSolverHi);
      }
      realSetPositiveSign(deltaB);

      _linearInterpolation(aa, bb, faa, fbb, NULL, secantSlopeA, &ctxtSolverHi);

      // interpolation
      if(!(realCompareEqual(faa, fbb) || realCompareEqual(faa, fbb1) || realCompareEqual(fbb, fbb1))) { // inverse quadratic interpolation
        _linearInterpolation(bb, bb1, fbb, fbb1, NULL, secantSlopeB, &ctxtSolverHi);
        _inverseQuadraticInterpolation(aa, bb, bb1, faa, fbb, fbb1, ss, &ctxtSolverHi);
      }
      else { // linear interpolation
        _linearInterpolation(bb, bb1, fbb, fbb1, ss, secantSlopeB, &ctxtSolverHi);
      }

      realSubtract(ss, bb, smb, &ctxtSolverHi);
      realMultiply(smb, const_2, smb, &ctxtSolverHi);
      realSetPositiveSign(smb);


      realSubtract(bb, bb1, bracketWidth, &ctxtSolver);
      realSetPositiveSign(bracketWidth);

      if(currentMethod != SOLVER_METHOD_NEWTON || !newtonInitialized) {
        // bisection
        realAdd(aa, bb, mm, &ctxtSolver);
        realMultiply(mm, const_1on2, mm, &ctxtSolver);
      }


      #if defined(OPTION_TVM_NEWTON)
      // Method selection: switch to Newton when bracket is tight enough (relative)
      if((currentSolverStatus & SOLVER_STATUS_TVM_APPLICATION) &&
         (variable == RESERVED_VARIABLE_IPONA ||
          variable == RESERVED_VARIABLE_NPPER ||
          variable == RESERVED_VARIABLE_PV ||
          variable == RESERVED_VARIABLE_PMT ||
          variable == RESERVED_VARIABLE_FV) &&
         currentMethod != SOLVER_METHOD_NEWTON && loop >= 5) {

        // Check relative bracket width: |bb - aa| / |bb| < see below %
        realSubtract(bb, aa, fullBracket, &ctxtSolver);
        realSetPositiveSign(fullBracket);
        realDivide(fullBracket, bb, relativeWidth, &ctxtSolver);
        realSetPositiveSign(relativeWidth);

        // Hand off to Newton when bracket is tight enough:
        // Path 1 (bb >= -25): Normal case - use relative bracket width
        //   - Condition A: Very tight bracket (<= 10% relative)
        //   - Condition B: Moderate bracket (<= 1000% relative) AND tiny residual (<= 1e-10)
        // Path 2 (bb < -25): Near-zero root - use absolute bracket width
        //   - Absolute bracket must be <= 1e-35
        bool_t handoff = false;
        if(realGetExponentComp(bb) >= -25) {
          // Normal case: use relative width
          if(realGetExponentComp(relativeWidth) <= -2) {
            handoff = true;  // Very tight relative bracket (<= 10%)
          }
          else if(realGetExponentComp(relativeWidth) <= 1 && realGetExponentComp(fbb) <= -10) {
            handoff = true;  // Moderate bracket (<= 1000%) + tiny residual
          }
        }
        else {
          // Near-zero root (bb < -25): use absolute bracket
          if(realGetExponentComp(bracketWidth) <= -35) {
            handoff = true;  // Absolute bracket tight enough for Newton
          }
        }

        //printf("realGetExponent(relativeWidth) = %d  realGetExponent(fbb)= %d handoff = %d\n",realGetExponent(relativeWidth), realGetExponent(fbb), handoff);
        if(handoff && !newtonDisabled) {
          currentMethod = SOLVER_METHOD_NEWTON;
          realCopy(bb, newton_x);  // Use best point (smallest residual), not midpoint
          newtonInitialized = true;
        }
      }


      // next point - select based on current method
      if(currentMethod == SOLVER_METHOD_NEWTON && newtonInitialized) {
        // Newton step: x_new = x - f(x)/f'(x)
        realCopy(newton_x, newton_trial);
        _executeSolverReal(variable, newton_trial, newton_fx, newton_deriv);

        // Check for Newton failure conditions
        if(realIsZero(newton_deriv) || realIsSpecial(newton_fx)) {
          currentMethod = SOLVER_METHOD_BRENT;
          newtonInitialized = false;
          bp1 = mm;  // Fall back to bisection
        }
        else {
          realDivide(newton_fx, newton_deriv, newton_step, &ctxtSolver);
          realSubtract(newton_x, newton_step, newton_x, &ctxtSolver);
          newton_iter_count++;
          if(newton_iter_count > 12) {
            // Newton oscillating at precision limit - accept current best and exit
            realToReal34(newton_fx, resZ);
            realToReal34(newton_x, resY);
            realToReal34(newton_x, resX);
            reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
            real34Copy(resX, REGISTER_REAL34_DATA(REGISTER_X));
            copySourceRegisterToDestRegister(REGISTER_X, variable);
            --engineNestingDepth;
            if((--currentSolverNestingDepth) == 0) {
              clearSystemFlag(FLAG_SOLVING);
            }
            first_newton_iter = true;
            result = SOLVER_RESULT_NORMAL;
            goto freeWork;
          }

          // Evaluate f at new point for convergence check
          realCopy(newton_x, newton_trial);
          _executeSolverReal(variable, newton_trial, newton_fx, NULL);
          bp1 = newton_x;
        }
      }
      else
      #endif //OPTION_TVM_NEWTON

      {
        if(extendRange) {
          realSubtract(bb, aa, tmp, &ctxtSolver);
          realMultiply(tmp, const_2, tmp, &ctxtSolver);
          realAdd(bb, tmp, ss, &ctxtSolver);  // Use ss instead of b
          bp1 = ss;
        }
        else if(realCompareEqual(bb, ss)) {
          bp1 = mm;
        }
        else if(realCompareLessThan(delta, deltaB) && realCompareLessThan(smb, deltaB)) {
          bp1 = ss;
        }
        else {
          bp1 = mm;
        }
      }


      // calculation
      _executeSolverReal(variable, bp1, fbp1, NULL);

      #if defined(OPTION_TVM_NEWTON)
      // Newton convergence check and divergence detection
        if(currentMethod == SOLVER_METHOD_NEWTON && newtonInitialized) {
          if(newton_polish_mode) {
            newton_polish_count++;
            if(newton_polish_count > 2) {
              // Polish complete: one Newton step was taken, now compare against saved Brent result and keep better
              newton_polish_mode = false;
              if(realCompareAbsLessThan(fbp1, fbb)) {
                // Newton improved - use Newton's result
                realToReal34(fbp1, resZ);
                realToReal34(newton_x, resY);
                realToReal34(newton_x, resX);
                reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
                real34Copy(resX, REGISTER_REAL34_DATA(REGISTER_X));
                copySourceRegisterToDestRegister(REGISTER_X, variable);
                --engineNestingDepth;
                if((--currentSolverNestingDepth) == 0) {
                  clearSystemFlag(FLAG_SOLVING);
                }
                result = SOLVER_RESULT_NORMAL;
                goto freeWork;
              }
              else {
                // Newton made it worse - revert to Brent's saved result
                realToReal34(brent_best_fx, resZ);
                realToReal34(brent_best_x, resY);
                realToReal34(brent_best_x, resX);
                reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
                real34Copy(resX, REGISTER_REAL34_DATA(REGISTER_X));
                copySourceRegisterToDestRegister(REGISTER_X, variable);
                --engineNestingDepth;
                if((--currentSolverNestingDepth) == 0) {
                  clearSystemFlag(FLAG_SOLVING);
                }
                result = SOLVER_RESULT_NORMAL;
                goto freeWork;
              }
            }
          }


        // Is Newton diverging
        if(!first_newton_iter && realCompareAbsLessThan(prev_fx, fbp1)) {
          // got worse - fall back to Brent permanently
          currentMethod = SOLVER_METHOD_BRENT;
          newtonInitialized = false;
          newtonDisabled = true;
          newton_iter_count = 0;
          first_newton_iter = true;
          stall_count = 0;
        }
        else if(!first_newton_iter && realCompareEqual(newton_x, prev_x)) {
          // x stopped changing - is it converged or stalled?
          realSetOne(tol_converged);
          tol_converged->exponent = -37;

          if(realCompareAbsLessThan(fbp1, tol_converged)) {
            // Converged - exit now
            realToReal34(fbp1, resZ);
            realToReal34(newton_x, resY);
            realToReal34(newton_x, resX);
            reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
            real34Copy(resX, REGISTER_REAL34_DATA(REGISTER_X));
            copySourceRegisterToDestRegister(REGISTER_X, variable);
            --engineNestingDepth;
            if((--currentSolverNestingDepth) == 0) {
              clearSystemFlag(FLAG_SOLVING);
            }
            first_newton_iter = true;
            stall_count = 0;
            result = SOLVER_RESULT_NORMAL;
            goto freeWork;
          }
          else {
            // Real stall - residual still large
            stall_count++;
            if(stall_count >= 3) {
              currentMethod = SOLVER_METHOD_BRENT;
              newtonInitialized = false;
              first_newton_iter = true;
              stall_count = 0;
            }
          }
        }
        else {
          // Save current residual for next iteration
          realCopy(fbp1, prev_fx);
          realCopy(newton_x, prev_x);
          stall_count = 0;
          // Check convergence only if Newton has actually run
          if(!first_newton_iter && realCompareAbsLessThan(fbp1, tolAlmostZero)) {
            realToReal34(fbp1, resZ);
            realToReal34(newton_x, resY);
            realToReal34(newton_x, resX);
            reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
            real34Copy(resX, REGISTER_REAL34_DATA(REGISTER_X));
            copySourceRegisterToDestRegister(REGISTER_X, variable);
            --engineNestingDepth;
            if((--currentSolverNestingDepth) == 0) {
              clearSystemFlag(FLAG_SOLVING);
            }
            first_newton_iter = true;  // Reset for next solve
            #if (defined PC_BUILD) && (defined SOLVERDEBUG2)
              printf("  Newton end Iter %d (%s)  ", loop, methodName[currentMethod]);
              printRealToConsole(newton_x, "x=", "\n");
            #endif
            result = SOLVER_RESULT_NORMAL;
            goto freeWork;
          }
          first_newton_iter = false;  // Mark that Newton has now run
        }
      }
      #endif //OPTION_TVM_NEWTON


      // Calculate convergence flags (needed for exit conditions)
      bool_t bb_bb1_converged = WP34S_RelativeError(bb, bb1, tol, &ctxtSolver);
      bool_t b1_b2_Equal = realCompareEqual(bb1, bb2);
      bool_t b_b1_Equal  = realCompareEqual(bb,  bb1);

      // Bracket update - only for Brent method
      if(currentMethod != SOLVER_METHOD_NEWTON || !newtonInitialized) {
        // Continue Brent
        if(extendRange && (realIsNegative(secantSlopeA) != realIsNegative(secantSlopeB)) && !realIsZero(secantSlopeA) && !realIsZero(secantSlopeB)) {
          extendRange = false;
          extremum = (realIsNegative(fbb) == realIsNegative(fbp1));
        }

        if(!realIsSpecial(bp1) && !realIsSpecial(fbp1)) {
          if(extendRange) {
            if((realIsNegative(fbb) != realIsNegative(fbp1)) || (realIsNegative(fbb) != realIsNegative(faa))) {
              extendRange = false;
              originallyLevel = false;
            }
            if((realIsNegative(faa) == realIsNegative(fbp1)) && realCompareAbsLessThan(fbb, fbp1)) {
              extendRange = false;
              originallyLevel = false;
              extremum = true;
            }
          }
          else if(realIsNegative(faa) == realIsNegative(fbp1)) {
            realCopy(bb, aa);
            realCopy(fbb, faa);
          }
          else {
            extendRange = false;
            extremum = false;
            if(!realCompareEqual(fbb, faa)) {
              originallyLevel = false;
            }
          }

          if(realCompareAbsLessThan(faa, fbp1)) {
            realCopy(bp1, tmp);
            realCopy(aa, bp1);
            realCopy(tmp, aa);
            realCopy(fbp1, tmp);
            realCopy(faa, fbp1);
            realCopy(tmp, faa);
            realCopy(aa, bb);
            realCopy(faa, fbb);
          }

          if(bp1 == ss) {
            realSetNaN(bb2);
          }
          else {
            realCopy(bb1, bb2);
          }
          realCopy(bb, bb1);
          realCopy(fbb, fbb1);
          realCopy(bp1, bb);
          realCopy(fbp1, fbb);
          //printRealToConsole(bb1, "PP: &b1=", "  ");
          //printRealToConsole(bb, "&b=", "\n");
        }

        else if(originallyLevel && (realIsInfinite(bb) || realIsInfinite(aa))) {
          result = SOLVER_RESULT_CONSTANT;
        }
        else if(extendRange) {
          extendRange = false;
          originallyLevel = false;
          extremum = true;
        }
        else if(realIsNegative(faa) != realIsNegative(fbb)) {
          result = SOLVER_RESULT_SIGN_REVERSAL;
        }
        else {
          result = SOLVER_RESULT_EXTREMUM;
        }


        // Stricter residual tolerance near zero
        if(realCompareAbsLessThan(bb, const_1e_16)) {
          stringToReal("1e-120", tol1, &ctxtSolver);
          fbIsAlmostZero = realCompareAbsLessThan(fbb, tol1);
        }
        else {
          fbIsAlmostZero = realCompareAbsLessThan(fbb, tolAlmostZero);
        }

        //printf("\nSOLVER_RESULT_NORMAL:%i\n",result == SOLVER_RESULT_NORMAL);
        //printf("   bb_bb1_converged:%i b1_b2_Equal:%i b_b1_Equal:%i originallyLevel:%i, extremum=%d\n", bb_bb1_converged, b1_b2_Equal, b_b1_Equal, originallyLevel, extremum);
        //printRealToConsole(bb, "  bb=", " ");
        //printRealToConsole(fbb, "fbb=", " ");
        //printRealToConsole(tolAlmostZero, "tolAlmostZero=", "\n");

        if(result != SOLVER_RESULT_NORMAL) {
          break;
        }
      }


      // End conditions

      if( (!realIsSpecial(bb2) && b1_b2_Equal ) &&
        ( (extendRange || bb_bb1_converged) || extremum )  ) {
        #if (defined OPTION_TVM_NEWTON) && (defined PC_BUILD) && (defined SOLVERDEBUG2)
          printf("  Exit via first end-condition, newton_polish_mode=%d\n", newton_polish_mode);
        #endif

        break;
      }
      if(!originallyLevel && ((!extendRange && bb_bb1_converged) || b_b1_Equal || fbIsAlmostZero) ) {
        #if defined(OPTION_TVM_NEWTON)
          if(currentMethod == SOLVER_METHOD_BRENT &&
             (currentSolverStatus & SOLVER_STATUS_TVM_APPLICATION) &&
             (variable == RESERVED_VARIABLE_IPONA ||
              variable == RESERVED_VARIABLE_NPPER ||
              variable == RESERVED_VARIABLE_PV ||
              variable == RESERVED_VARIABLE_PMT ||
              variable == RESERVED_VARIABLE_FV)) {
            currentMethod = SOLVER_METHOD_NEWTON;
            realCopy(bb, newton_x);
            newtonInitialized = true;
            newton_polish_mode = true;
            newton_polish_count = 0;
            // Save Brent's best result before polish in case Newton makes it worse
            realCopy(bb, brent_best_x);
            realCopy(fbb, brent_best_fx);
            // Don't break - do Newton polish

            }
            else if(newton_polish_mode && newton_polish_count > 0) {
            // Polish done - compare Newton result against saved Brent result
            newton_polish_mode = false;
            newton_polish_count = 0;
            if(realCompareAbsLessThan(fbp1, brent_best_fx)) {
              // Newton improved - use Newton's result
              realToReal34(fbp1, resZ);
              realToReal34(newton_x, resY);
              realToReal34(newton_x, resX);
            }
            else {
              // Newton made it worse - revert to saved Brent result
              realToReal34(brent_best_fx, resZ);
              realToReal34(brent_best_x, resY);
              realToReal34(brent_best_x, resX);
            }
            goto solver_polish_exit;

          }
          else {
            break;
          }
        #else
          break;
        #endif // OPTION_TVM_NEWTON
      }
      if(loop > (currentSolverStatus & SOLVER_STATUS_TVM_APPLICATION ? 2000 : 10000)) {
        result = SOLVER_RESULT_OTHER_FAILURE;
        break;
      }

    } while(true);
   //==== ITER END ====




    #if (defined PC_BUILD) && (defined SOLVERDEBUG)
      printf("Ended iter %d  ", loop);
      printRealToConsole(aa, "a=", " ");
      printRealToConsole(bb, "b=", "\n");
    #endif

    _executeSolverReal(variable, bb, tmp, NULL);
    realToReal34(tmp, resZ);
    realToReal34(bb1, resY);
    realToReal34(bb, resX);
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    real34Copy(resX, REGISTER_REAL34_DATA(REGISTER_X));
    copySourceRegisterToDestRegister(REGISTER_X, variable);
    goto solver_final_print;


#if defined(OPTION_TVM_NEWTON)
    solver_polish_exit:
#endif //OPTION_TVM_NEWTON

    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    real34Copy(resX, REGISTER_REAL34_DATA(REGISTER_X));
    copySourceRegisterToDestRegister(REGISTER_X, variable);

solver_final_print:;
    #if (defined PC_BUILD) && ((defined SOLVERDEBUG) || (defined SOLVERDEBUG2))
      printReal34ToConsole(REGISTER_REAL34_DATA(REGISTER_X), "Final real34  =", "\n");
    #endif


    if((extendRange && !originallyLevel) || extremum) {
      result = SOLVER_RESULT_EXTREMUM;
    }

    if((--currentSolverNestingDepth) == 0) {
      clearSystemFlag(FLAG_SOLVING);
    }
    else if(was_inting) {
      clearSystemFlag(FLAG_SOLVING);
      setSystemFlag(FLAG_INTING);
    }

    if(fbIsAlmostZero) {  //    if(real34IsZero(&fb)) {
      result = SOLVER_RESULT_NORMAL;
    }

    if(result == SOLVER_RESULT_EXTREMUM) { // Check if the result is really an extremum
      bool_t retainSolvingFlag = getSystemFlag(FLAG_SOLVING);
      setSystemFlag(FLAG_SOLVING);
      realCopy(minBracketSpacing, tmp);
      while(true) {
        real34ToReal(resX, resXr);
        real34ToReal(resZ, resZr);
        realAdd(resXr, tmp, aa, &ctxtSolver);
        realSubtract(resXr, tmp, bb, &ctxtSolver);
        _executeSolverReal(variable, aa, faa, NULL);
        _executeSolverReal(variable, bb, fbb, NULL);
        realSubtract(faa, resZr, faa, &ctxtSolver);
        realSubtract(fbb, resZr, fbb, &ctxtSolver);
        if(realIsSpecial(faa) || realIsSpecial(fbb)) {
          result = SOLVER_RESULT_OTHER_FAILURE;
          break;
        }
        else if((currentSolverStatus & SOLVER_STATUS_TVM_APPLICATION) && (realIsSpecial(aa) || realIsSpecial(bb))) { // terminate the TVM solver to avoid possible infinite loop
          result = SOLVER_RESULT_CONSTANT;
          break;
        }
        else if(realIsZero(faa) || realIsZero(fbb)) {
          realMultiply(tmp, const_100, tmp, &ctxtSolver);
        }
        else if(realIsNegative(faa) == realIsNegative(fbb)) { // true extremum
          break;
        }
        else { // not an extremum
          result = SOLVER_RESULT_OTHER_FAILURE;
          break;
        }
      }
      if(!retainSolvingFlag) {
        clearSystemFlag(FLAG_SOLVING);
      }
    }
    --engineNestingDepth;                                        // after the extremum check above, which re-runs the user program

    if(result == SOLVER_RESULT_NORMAL && real34IsInfinite(REGISTER_REAL34_DATA(variable)) && extendRange && real34IsZero(resZ)) {
      result = SOLVER_RESULT_CONSTANT;
    }

    if(lastErrorCode == ERROR_SOLVER_ABORT) {
      result = SOLVER_RESULT_ABORTED;
    }

freeWork:
    REAL_T_FREE(aa,                75);
    REAL_T_FREE(bb,                75);
    REAL_T_FREE(bb1,               75);
    REAL_T_FREE(bb2,               75);
    REAL_T_FREE(faa,               75);
    REAL_T_FREE(fbb,               75);
    REAL_T_FREE(fbb1,              75);
    REAL_T_FREE(mm,                75);
    REAL_T_FREE(ss,                75);
    REAL_T_FREE(secantSlopeA,      75);
    REAL_T_FREE(secantSlopeB,      75);
    REAL_T_FREE(delta,             75);
    REAL_T_FREE(deltaB,            75);
    REAL_T_FREE(smb,               75);
    REAL_T_FREE(tol,               75);
    REAL_T_FREE(fbp1,              75);
    REAL_T_FREE(tmp,               75);
    REAL_T_FREE(tolAlmostZero,     75);
    REAL_T_FREE(tol1,              75);
    REAL_T_FREE(minBracketSpacing, 75);
    REAL_T_FREE(prevResX,          75);
    REAL_T_FREE(antiLevel,         75);
    REAL_T_FREE(bracketWidth,      75);
    REAL_T_FREE(resXr,             75);
    REAL_T_FREE(resZr,             75);
    #if defined(OPTION_TVM_NEWTON)
      REAL_T_FREE(newton_x,        75);
      REAL_T_FREE(prev_fx,         75);
      REAL_T_FREE(prev_x,          75);
      REAL_T_FREE(brent_best_x,    75);
      REAL_T_FREE(brent_best_fx,   75);
      REAL_T_FREE(relativeWidth,   75);
      REAL_T_FREE(fullBracket,     75);
      REAL_T_FREE(newton_trial,    75);
      REAL_T_FREE(newton_fx,       75);
      REAL_T_FREE(newton_deriv,    75);
      REAL_T_FREE(newton_step,     75);
      REAL_T_FREE(tol_converged,   75);
    #endif //OPTION_TVM_NEWTON
    setRoundingMode(userRoundingMode);
    return result;
}
