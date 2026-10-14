// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors


/********************************************//** //JM
 * \file graph.c Graphing module
 ***********************************************/

#include "c47.h"

#if defined(PC_BUILD)
  //Verbose directives can be simulataneously selected
  // #define VERBOSE_SOLVER00   // minimal text
  // #define VERBOSE_SOLVER0  // a lot less text
  // #define VERBOSE_SOLVER1  // a lot less text
  // #define VERBOSE_SOLVER2  // verbose a lot
  // #define VERBOSE_SOLVER_ITERDATA // One long line for each iteration
#else // !PC_BUILD
  #undef VERBOSE_SOLVER00
  #undef VERBOSE_SOLVER0
  #undef VERBOSE_SOLVER1
  #undef VERBOSE_SOLVER2
  #undef VERBOSE_SOLVER_ITERDATA
  #undef STATDEBUG
  #undef GRAPHDEBUG
  #undef GRAPHDEBUG_MIN
#endif // PC_BUILD


//Todo: involve https://en.wikipedia.org/wiki/Brent%27s_method#Brent's_method

#define COMPLEXKICKER true       //flag to allow conversion to complex plane if no convergenge found
#define CHANGE_TO_MOD_SECANT 0   //at iteration nn go to the modified secant method. 0 means immediately
#define CONVERGE_FACTOR 1.0f     //
#define NUMBERITERATIONS 9999    // 35 // Must be smaller than LIM (see STATS)

#define ALLOW_NESTED_PARTPLOT_ON_EXIT 0   // 1: interrupted plot still shows the partial graph (legacy). 0 (default): stop dead on the PLT step, stack intact

#define ctxtSolver2 &ctxtReal39
int16_t osc = 0;
uint8_t DXR = 0, DYR = 0, DXI = 0, DYI = 0;


#if defined(OPTION_GRAPHICS)
  static void fnPlot(uint16_t unusedButMandatoryParameter) {
      lastPlotMode = PLOT_NOTHING;
      strcpy(plotStatMx, "DrwMX");
      fnPlotSQ(0);
      //  C47 advanced plot ^^
  }


    static void initialize_function(void){
      if(graphVariabl1 > 0) {
        #if defined(PC_BUILD)
          //printf(">>> graphVariable = %i\n", graphVariable);
          if(lastErrorCode != 0) {
            #if defined(VERBOSE_SOLVER00)
            printf("ERROR CODE in initialize_functionA: %u\n", lastErrorCode);
            #endif // VERBOSE_SOLVER00
            lastErrorCode = 0;
          }
        #endif //PC_BUILD
      }
      else {
        #if defined(PC_BUILD)
          //printf(">>> graphVariable = %i\n", graphVariable);
          #if defined(VERBOSE_SOLVER00)
            printf("ERROR CODE in initialize_functionB: %u\n", lastErrorCode);
          #endif // VERBOSE_SOLVER00
        #endif //PC_BUILD
      }
    }
  #endif // OPTION_GRAPHICS


  static void execute_rpn_function(void){
    #if defined(GRAPHDEBUG_MIN)
      print_caller(NULL);
    #endif //GRAPHDEBUG_MIN
    if(graphVariabl1 <= 0 || graphVariabl1 > LAST_LABEL) {
      #if defined(PC_BUILD)
        printf("Error: No graph variable %u\n", graphVariabl1);
      #endif //PC_BUILD
      return;
    }

    calcRegister_t regStats = graphVariabl1;
    if(regStats != INVALID_VARIABLE) {
      fnStore(regStats);                  //place X register into x
                                    #if defined(VERBOSE_SOLVER0)
                                      printf("Graph variable x=%u: ", graphVariabl1);
                                      printRegisterToConsole(graphVariabl1, " = ", "\n");
                                    #endif //VERBOSE_SOLVER0

    if(currentSolverStatus & SOLVER_STATUS_RPN_GRAPHER) {
      // a complex program result stays typed so it reaches the plotter; any other type becomes real34, NaN where it has no real value
      reallyRunFunction(ITM_STO, currentSolverVariable);
      fnFillStack(NOPARAM);
      {
        uint16_t savedCurrentSolverProgram = currentSolverProgram;
        uint16_t savedCurrentSolverVariable = currentSolverVariable;
        uint16_t savedCurrentSolverStatus = currentSolverStatus;
        dynamicMenuItem = -1;
        execProgram(currentSolverProgram + FIRST_LABEL);
        currentSolverProgram = savedCurrentSolverProgram;
        currentSolverVariable = savedCurrentSolverVariable;
        currentSolverStatus = savedCurrentSolverStatus;
      }
      if(getRegisterDataType(REGISTER_X) != dtComplex34) {
        real34_t res34;
        if(!getRegisterAsReal34Quiet(REGISTER_X, &res34)) {
          realToReal34(const_NaN, &res34);
        }
        reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
        real34Copy(&res34, REGISTER_REAL34_DATA(REGISTER_X));
      }
    } else {
      parseEquation(currentFormula, EQUATION_PARSER_XEQ, tmpString, tmpString + AIM_BUFFER_LENGTH);
    }
      adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);

                                    #if defined(VERBOSE_SOLVER0)
                                      printf("Graph variable y ");
                                      printRegisterToConsole(REGISTER_X, " = ", "\n");
                                    #endif //VERBOSE_SOLVER0

      if(lastErrorCode != 0) { //failed evaluation: the sample has no value; NaN it, else the stale register content plots incorrect data
                                    #if defined(PC_BUILD) && defined(VERBOSE_SOLVER00)
                                      printf("ERROR CODE in execute_rpn_function: %u\n", lastErrorCode);
                                    #endif // PC_BUILD && VERBOSE_SOLVER00
        real_t nanR;
        realSetNaN(&nanR);
        reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
        realToReal34(&nanR, REGISTER_REAL34_DATA(REGISTER_X));
        if(lastErrorCode != ERROR_SOLVER_ABORT) {   // keep an abort alive to stop the plot; ordinary eval errors just NaN this sample
          lastErrorCode = 0;
        }
      }
      fnRCL(regStats);

                                    #if defined(VERBOSE_SOLVER0)
                                      printRegisterToConsole(REGISTER_X, ">>> Calc x=", "");
                                      printRegisterToConsole(REGISTER_Y, " y=", "");
                                    #endif // VERBOSE_SOLVER0

                                    if(ENABLE_COMPLEXSOLVER_FILE_OUTPUT == 2) {
                                      copySourceRegisterToDestRegister(REGISTER_X, REGISTER_J);
                                      copySourceRegisterToDestRegister(REGISTER_Y, REGISTER_K);
                                      fnP_All_Regs(PRN_XYr);
                                      copySourceRegisterToDestRegister(REGISTER_J, REGISTER_X);
                                      copySourceRegisterToDestRegister(REGISTER_K, REGISTER_Y);
                                    }

    }
    else { //invalid plot variable: nothing was stored or executed, and fnRCL above never ran, so the caller would sample stale REGISTER_Y content; NaN it
                                    #if defined(PC_BUILD) && defined(VERBOSE_SOLVER00)
                                      printf("ERROR in execute_rpn_function; invalid variable: %u\n", lastErrorCode);
                                    #endif // PC_BUILD && VERBOSE_SOLVER00
      real_t nanR;
      realSetNaN(&nanR);
      reallocateRegister(REGISTER_Y, dtReal34, 0, amNone);
      realToReal34(&nanR, REGISTER_REAL34_DATA(REGISTER_Y));
      lastErrorCode = 0;
    }
  }



//###################################################################################
//PLOTTER
  int32_t drawMxN(void){
    uint16_t rows = 0;
    if(plotStatMx[0]!='D') {
      return 0;
    }
    calcRegister_t regStats = findNamedVariable(plotStatMx);
    if(regStats == INVALID_VARIABLE) {
      return 0;
    }
    if(isStatsMatrix(&rows, plotStatMx)) {
      real34Matrix_t stats;
      linkToRealMatrixRegister(regStats, &stats);
      return stats.header.matrixRows;
    }
    else {
      return 0;
    }
  }


  void fnClDrawMx(uint8_t origin) {
                                  #if defined(STATDEBUG)
                                    printf("Clearing Draw Matrix, from %d\n", origin);
                                  #endif // STATDEBUG
    PLOT_ZOOM = 0;
    calcRegister_t regStats = findNamedVariable("DrwMX");
    if(regStats != INVALID_VARIABLE) {
      fnDeleteVariable(regStats);
    };
  }


  static void AddtoDrawMx() {
    real_t x, y;
    uint16_t rows = 0, cols;
                                  #if defined(STATDEBUG)
                                    char prefix[100];
                                    memcpy(prefix, allNamedVariables[regStatsXY - FIRST_NAMED_VARIABLE].variableName + 1, allNamedVariables[regStatsXY - FIRST_NAMED_VARIABLE].variableName[0]);
                                    strcpy(prefix + allNamedVariables[regStatsXY - FIRST_NAMED_VARIABLE].variableName[0], " :");
                                    printf("Adding to Draw Matrix %s\n", prefix);
                                  #endif // STATDEBUG
    calcRegister_t regStats = regStatsXY;
    if(!isStatsMatrixN(&rows, regStats)) {
      regStats = allocateNamedMatrix(plotStatMx, 1, 2);   // bugfix was here. it already creates preps the 1x2 register matrix, no need to realMatrixInit a linked copy as that was never stored back nor freed
      regStatsXY = regStats;
    }
    else {
      if(appendRowAtMatrixRegister(regStats)) {
      }
      else {
        regStats = INVALID_VARIABLE;
      }
    }
    if(regStats != INVALID_VARIABLE) {

      real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), &x);
      real34ToReal(REGISTER_REAL34_DATA(REGISTER_Y), &y);

      real34Matrix_t stats;
      linkToRealMatrixRegister(regStats, &stats);
      rows = stats.header.matrixRows;
      cols = stats.header.matrixColumns;
      realToReal34(&x, &stats.matrixElements[(rows-1) * cols    ]);
      realToReal34(&y, &stats.matrixElements[(rows-1) * cols + 1]);

                                  #if defined(STATDEBUG)
                                    printf("[r, c] [%u, %u]=", rows, cols);
                                    printRealToConsole(&x, "x:=", " ");
                                    printRealToConsole(&y, "y:=", "\n");
                                  #endif // STATDEBUG
    }
    else {
      calcMode = CM_NORMAL;
      displayCalcErrorMessage(ERROR_NOT_ENOUGH_MEMORY_FOR_NEW_MATRIX, ERR_REGISTER_LINE); // Invalid input data type for this operation
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "additional matrix line not added; rows = %i", rows);
        moreInfoOnError("In function AddtoDrawMx:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }



//******************************************************************************************************************************
// Graph plotting helpers and graph_eqn, converted to real_t for all math domain values. Loop counters, indices, booleans, register
// flags and direction selectors stay int / bool_t / int8_t. All real_t arithmetic uses ctxtGraphs (= &ctxtGraphsLocal), narrowed
// to 14 working digits at graph_eqn entry. Helpers are defined first in this file, then graph_eqn at the bottom.
//******************************************************************************************************************************

#define PLOT_DIGITS           16    // Storage size for the graph real_t buffers; must exceed GRAPH_WORKING_DIGITS so every result fits. 16 allocates 21 digits after REAL_MAX_DIGITS rounding.
#define GRAPH_WORKING_DIGITS  14    // Working precision for all graph math. 14 instead of 39 digits is a real speedup and pixels only need ~5 pixels.
                                    // Also sets the too-close limit: ranges narrower than 100x the ULP, e.g. 1.0000000000001 to 1.0000000000002, are unplottable at 14 digits and get widened.
                                    // Register Y reads are rounded to working digits in convertRegisterToReal.

// Context for graph calculations. Initialized at graph_eqn entry from ctxtReal39 to 14 working digits
// 14 vs 39 is a real speedup. All real_t arithmetic in graph_eqn and the helpers use this context via ctxtGraphs.
static  decContext  ctxtGraphsLocal;
#define ctxtGraphs  &ctxtGraphsLocal

//   _R_STR_OF(SS1)  ->  "1.8"   (when #define SS1 1.8)
#define _R_STR(x)     #x
#define _R_STR_OF(x)  _R_STR(x)


// REAL_T_PTR buffers are smaller than a full decNumber since we only need a few digits. realIs*/realChangeSign poke struct fields inline, so
// GCC's bounds checker wrongly thinks the small buffer cannot hold the big struct. The wrappers below hide the inline access; the pragma silences it.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
__attribute__((noinline)) static bool_t graphIsZero    (const real_t *x) { return realIsZero(x);     }
__attribute__((noinline)) static bool_t graphIsNegative(const real_t *x) { return realIsNegative(x); }
#if defined(OPTION_GRAPHICS)
  __attribute__((noinline)) static bool_t graphIsPositive(const real_t *x) { return realIsPositive(x); }
#endif //OPTION_GRAPHICS
__attribute__((noinline)) static void   graphChangeSign(real_t *x)       { realChangeSign(x);        }
#pragma GCC diagnostic pop

#if defined(VERBOSE_SOLVER00) || defined(VERBOSE_SOLVER0)
  // Return-value convenience wrapper of realToDouble; debug prints only
  static double realToDoubleVal(const real_t *r) {
    double d;
    realToDouble(r, &d);
    return d;
  }
#endif // VERBOSE_SOLVER00 || VERBOSE_SOLVER0

static void convertRealToReal34Register(const real_t *x, calcRegister_t destination) {
  reallocateRegister(destination, dtReal34, 0, amNone);
  realToReal34(x, REGISTER_REAL34_DATA(destination));
}

static void convertRealToReal34RegisterPush(const real_t *x, calcRegister_t destination) {
  setSystemFlag(FLAG_ASLIFT);
  liftStack();
  convertRealToReal34Register(x, destination);
  setSystemFlag(FLAG_ASLIFT);
}


// Read a register as real_t. For complex registers, return Re or Im depending on FLAG_IMPLOT
static void convertRegisterToReal(calcRegister_t source, real_t *destination) {
  real_t tmp;
  if(getRegisterDataType(source) == dtComplex34) {
    if(getSystemFlag(FLAG_IMPLOT)) {
      real34ToReal(REGISTER_IMAG34_DATA(source), &tmp);
    }
    else {
      real34ToReal(REGISTER_REAL34_DATA(source), &tmp);
    }
    realPlus(&tmp, destination, ctxtGraphs);
    return;
  }
  if(getSystemFlag(FLAG_IMPLOT)) {
    realSetZero(destination);
    return;
  }
  if(!getRegisterAsRealQuiet(source, &tmp)) {
    realSetNaN(destination);
    return;
  }
  realPlus(&tmp, destination, ctxtGraphs);
}

#if defined(OPTION_GRAPHICS)
// Normalised read for the jump-back analysis: out = (v - origin) / span, dimensionless and O(1) at any data magnitude
static double normCoord(const real_t *v, const real_t *origin, const real_t *span) {
  real_t t;
  double d;
  realSubtract(v, origin, &t, ctxtGraphs);           // t = v - origin
  realDivide(&t, span, &t, ctxtGraphs);              // t = (v - origin) / span
  realToDouble(&t, &d);
  return d;
}
#endif //OPTION_GRAPHICS


// Reduce REGISTER_Y to either Re or Im (per getSystemFlagIM) before AddtoDrawMx etc.
static void reduceRegisterYToComponent(void) {
  if(getRegisterDataType(REGISTER_Y) == dtComplex34) {
    fnSwapXY(NOPARAM);
    if(getSystemFlag(FLAG_IMPLOT)) {
      fnImaginaryPart(NOPARAM);
    }
    else {
      fnRealPart(NOPARAM);
    }
    fnSwapXY(NOPARAM);
  }
  else if(getSystemFlag(FLAG_IMPLOT)) {
    reallocateRegister(REGISTER_Y, dtReal34, REAL34_SIZE_IN_BLOCKS, amNone);
    real34SetZero(REGISTER_REAL34_DATA(REGISTER_Y));
  }
}

// Wrapper around execute_rpn_function that narrows ctxtReal34/39/51/75 to graph-eqn precision
static void execute_rpn_function_graphAcc(void) {
  #if defined(GRAPHDEBUG_MIN)
    print_caller(NULL);
  #endif //GRAPHDEBUG_MIN
  #if defined(LOW_GRAPH_ACC)
    int32_t s34 = ctxtReal34.digits;
    int32_t s39 = ctxtReal39.digits;
    int32_t s51 = ctxtReal51.digits;
    int32_t s75 = ctxtReal75.digits;
    bool_t savedGraphAccActive = graphAccActive;
    graphAccActive = true;   // paired 1:1 with the reduction below; a nested SOLVE reads this to pick the graph convergence tolerance, else it spins to the iteration cap
    ctxtReal34.digits = significantDigitsForEqnGraphs;
    ctxtReal39.digits = significantDigitsForEqnGraphs + 3;
    ctxtReal51.digits = significantDigitsForEqnGraphs + 12;
    ctxtReal75.digits = significantDigitsForEqnGraphs + 18;
    execute_rpn_function();
    ctxtReal34.digits = s34;
    ctxtReal39.digits = s39;
    ctxtReal51.digits = s51;
    ctxtReal75.digits = s75;
    graphAccActive = savedGraphAccActive;
  #else
    execute_rpn_function();
  #endif // LOW_GRAPH_ACC
}

typedef struct {
  uint32_t xData   [REAL_SIZE_IN_BYTES(PLOT_DIGITS) / 4];
  uint32_t yData   [REAL_SIZE_IN_BYTES(PLOT_DIGITS) / 4];
  uint32_t gradData[REAL_SIZE_IN_BYTES(PLOT_DIGITS) / 4];
  bool_t   stored;
} PlotPoint;

#define PP_X(pp)    ((real_t *)((pp).xData))
#define PP_Y(pp)    ((real_t *)((pp).yData))
#define PP_GRAD(pp) ((real_t *)((pp).gradData))


#define SS1 1.8                       // 1.4 grad2/grad2 threshold for 50% dx
#define SS2 2.4                       // 2   grad2/grad2 threshold for jumping back
#define FINE 9                        // (was 20) number of steps to jump
#define JMP 0.8                       // Jump back: 9 x 0.2 (was 20 x 0.1) : -0.8 -0.6 -0.4 -0.2 0 0.2 0.4 0.6 0.8, i.e. 9 steps back for discontinuity (0.9)
#define dJMP 0.2                      // Fine movement in p.u. ddx
#define STEP_OFFSET 0.99999           // Stay off exact integers
#define MIN_IMPROVEMENT_RATIO 1.25    // Minimum improvement needed to justify high-res
#define HIGH_RES_SAMPLE_COUNT   3        // Number of high-res points to evaluate
#define HIGH_RES_SAMPLE_COUNT_R const_3  // real_t alias of HIGH_RES_SAMPLE_COUNT, kept in sync
#define REVERT_THRESHOLD 0.8          // When to revert to previous dx


bool_t validateDiscontinuityResolution(PlotPoint *buffer, int count, const real_t *yBefore, const real_t *yAfter, const real_t *discontinuityThreshold) {
  #if defined(GRAPHDEBUG)
    char strBuf1[64], strBuf2[64], strBuf3[64], strBuf4[64];
    realToString(yBefore,                strBuf1);
    realToString(yAfter,                 strBuf2);
    realToString(discontinuityThreshold, strBuf3);
    printf("\nVALIDATING DISCONTINUITY RESOLUTION:");
    printf("\n  Points: %d, yBefore=%s, yAfter=%s, threshold=%s\n", count, strBuf1, strBuf2, strBuf3);
  #endif // GRAPHDEBUG

  if(count < 3) {
    #if defined(GRAPHDEBUG)
      printf("  INVALID: Not enough points\n");
    #endif // GRAPHDEBUG
    return false;
  }

  // Check if the fine-stepped points show smooth transition
  REAL_T_PTR(maxJump,        PLOT_DIGITS);
  REAL_T_PTR(totalVariation, PLOT_DIGITS);
  REAL_T_PTR(jump,           PLOT_DIGITS);
  REAL_T_PTR(diff,           PLOT_DIGITS);
  realSetZero(maxJump);
  realSetZero(totalVariation);

  // Check continuity between consecutive fine points
  for(int i = 1; i < count; i++) {
    realSubtract(PP_Y(buffer[i]), PP_Y(buffer[i-1]), diff, ctxtGraphs);
    realCopyAbs(diff, jump);
    realAdd(totalVariation, jump, totalVariation, ctxtGraphs);
    if(realCompareGreaterThan(jump, maxJump)) {
      realCopy(jump, maxJump);
    }
  }

  // Also check connection to endpoints
  REAL_T_PTR(startJump, PLOT_DIGITS);
  REAL_T_PTR(endJump,   PLOT_DIGITS);
  realSubtract(PP_Y(buffer[0]),         yBefore, diff, ctxtGraphs);
  realCopyAbs(diff, startJump);
  realSubtract(yAfter, PP_Y(buffer[count-1]), diff, ctxtGraphs);
  realCopyAbs(diff, endJump);

  if(realCompareGreaterThan(startJump, maxJump)) {
    realCopy(startJump, maxJump);
  }
  if(realCompareGreaterThan(endJump, maxJump)) {
    realCopy(endJump, maxJump);
  }

  // If the maximum jump in fine steps is still above threshold, discontinuity persists
  bool_t discontinuityResolved = realCompareLessThan(maxJump, discontinuityThreshold);

  // Additional check: fine points should show reasonable continuity
  REAL_T_PTR(avgVariation, PLOT_DIGITS);
  REAL_T_PTR(threeAvg,     PLOT_DIGITS);
  REAL_T_PTR(halfThresh,   PLOT_DIGITS);
  REAL_T_PTR(countMinus1,  PLOT_DIGITS);
  int32ToReal(count - 1, countMinus1);
  realDivide(totalVariation, countMinus1, avgVariation, ctxtGraphs);
  realMultiply(const_3, avgVariation, threeAvg, ctxtGraphs);
  realCopy(discontinuityThreshold, halfThresh);
  realDivideBy2(halfThresh, ctxtGraphs);

  bool_t below3xAvg = realCompareLessThan(maxJump, threeAvg);
  bool_t belowHalfThresh = realCompareLessThan(maxJump, halfThresh);
  bool_t smoothTransition = below3xAvg || belowHalfThresh;

  #if defined(GRAPHDEBUG)
    realToString(maxJump,      strBuf1);
    realToString(avgVariation, strBuf2);
    realToString(startJump,    strBuf3);
    realToString(endJump,      strBuf4);
    printf("  maxJump=%s, avgVariation=%s\n", strBuf1, strBuf2);
    printf("  startJump=%s, endJump=%s\n", strBuf3, strBuf4);
    printf("  discontinuityResolved=%d, smoothTransition=%d\n", discontinuityResolved, smoothTransition);
  #endif // GRAPHDEBUG
  return discontinuityResolved && smoothTransition;
}



void calculateNewStepSize(int discontinuityDetected, const real_t *grad1, const real_t *grad2, bool_t grad2IncreaseDetected, const real_t *dx0, real_t *newDx) {
  #if defined(GRAPHDEBUG)
    char strBuf1[64], strBuf2[64], strBuf3[64], strBuf4[64], strBuf5[64], strBuf6[64];
    realToString(grad1, strBuf1);
    realToString(grad2, strBuf2);
    realToString(dx0,   strBuf3);
    printf("calculateNewStepSize: discontinuity=%d, grad1=%s, grad2=%s, increase=%d, dx0=%s\n", discontinuityDetected, strBuf1, strBuf2, grad2IncreaseDetected, strBuf3);
  #endif // GRAPHDEBUG

  if(discontinuityDetected > 0 && discontinuityDetected <= FINE) {
    REAL_T_PTR(djmpReal, PLOT_DIGITS);
    stringToReal(_R_STR_OF(dJMP), djmpReal, ctxtGraphs);
    realMultiply(djmpReal, dx0, newDx, ctxtGraphs);
    #if defined(GRAPHDEBUG)
      realToString(newDx, strBuf4);
      printf("  -> Discontinuity mode: newDx=%s\n", strBuf4);
    #endif // GRAPHDEBUG
    return;
  }
  else if(graphIsZero(grad2) || graphIsZero(grad1)) {
    realCopy(dx0, newDx);
    #if defined(GRAPHDEBUG)
      printf("  -> Zero gradient: keeping dx0=%s\n", strBuf3);
    #endif // GRAPHDEBUG
    return;
  }
  else if(grad2IncreaseDetected) {
    REAL_T_PTR(ratio1,  PLOT_DIGITS);
    REAL_T_PTR(ratio2,  PLOT_DIGITS);
    REAL_T_PTR(ss1Real, PLOT_DIGITS);
    realDivide(grad2, grad1, ratio1, ctxtGraphs);
    realDivide(grad1, grad2, ratio2, ctxtGraphs);
    stringToReal(_R_STR_OF(SS1), ss1Real, ctxtGraphs);

    bool_t scaleHalf = realCompareGreaterThan(ratio1, ss1Real) || realCompareGreaterThan(ratio2, ss1Real);

    if(scaleHalf) {
      realMultiply(dx0, const_1on2, newDx, ctxtGraphs);
    }
    else {
      realCopy(dx0, newDx);
    }
    #if defined(GRAPHDEBUG)
      realToString(ratio1, strBuf4);
      realToString(ratio2, strBuf5);
      realToString(newDx,  strBuf6);
      printf("  -> Gradient increase: ratio1=%s, ratio2=%s, newDx=%s\n", strBuf4, strBuf5, strBuf6);
    #endif // GRAPHDEBUG
    return;
  }
  else {
    realCopy(dx0, newDx);
    #if defined(GRAPHDEBUG)
      printf("  -> No change: keeping dx0=%s\n", strBuf3);
    #endif // GRAPHDEBUG
    return;
  }
}



void enterHighResMode(bool_t *inHighResMode, int *highResCount, real_t *highResStartX, real_t *baselineCurvatureChange, real_t *cumulativeCurvatureChange, const real_t *x, const real_t *curvatureChange) {
  #if defined(GRAPHDEBUG)
    char strBuf1[64], strBuf2[64];
    realToString(x,               strBuf1);
    realToString(curvatureChange, strBuf2);
    printf("*** ENTERING HIGH-RES MODE at x=%s, curvatureChange=%s ***\n", strBuf1, strBuf2);
  #endif // GRAPHDEBUG
  *inHighResMode = true;
  *highResCount = 0;
  realCopy(x,               highResStartX);
  realCopy(curvatureChange, baselineCurvatureChange);
  realSetZero(cumulativeCurvatureChange);
}



void commitHighResPointsInOrder(PlotPoint *buffer, int count) {
  #if defined(GRAPHDEBUG)
    char strBuf1[64], strBuf2[64], strBuf3[64];
    printf("*** COMMITTING %d HIGH-RES POINTS ***\n", count);
    for(int i = 0; i < count; i++) {
      realToString(PP_X(buffer[i]),    strBuf1);
      realToString(PP_Y(buffer[i]),    strBuf2);
      realToString(PP_GRAD(buffer[i]), strBuf3);
      printf("  Point %d: x=%s, y=%s, grad=%s, stored=%d\n",
             i, strBuf1, strBuf2, strBuf3, buffer[i].stored);
    }
  #endif // GRAPHDEBUG
  for(int i = 0; i < count; i++) {
    if(!buffer[i].stored) {
      convertRealToReal34RegisterPush(PP_X(buffer[i]), REGISTER_X);
      execute_rpn_function_graphAcc();
      reduceRegisterYToComponent();
      AddtoDrawMx();
      buffer[i].stored = true;
    }
  }
}



void abandonHighResMode(int *highResCount, bool_t *inHighResMode) {
  #if defined(GRAPHDEBUG)
    printf("*** ABANDONING HIGH-RES MODE (discarding %d points) ***\n", *highResCount);
  #endif // GRAPHDEBUG
  *highResCount = 0;
  *inHighResMode = false;
  // High-res points are simply discarded, not added to plot
}



void resetHighResTracking(int *highResCount, bool_t *inHighResMode, real_t *cumulativeCurvatureChange) {
  #if defined(GRAPHDEBUG)
    printf("*** RESETTING HIGH-RES TRACKING ***\n");
  #endif // GRAPHDEBUG
  *highResCount = 0;
  *inHighResMode = false;
  realSetZero(cumulativeCurvatureChange);
}



bool_t detectTrueDiscontinuity(const real_t *y0, const real_t *y1, const real_t *y2, const real_t *grad0, const real_t *grad1, const real_t *grad2, const real_t *yAvg, int count) {  // Distinguish between genuine discontinuities and normal peaks
  if(real34IsSpecial(REGISTER_REAL34_DATA(REGISTER_X)) || ((getRegisterDataType(REGISTER_X) == dtComplex34) && (real34IsSpecial(REGISTER_IMAG34_DATA(REGISTER_X))))) {
    return true;
  }
  if(count < 4) {
    return false;
  }

  REAL_T_PTR(absY1,        PLOT_DIGITS);
  REAL_T_PTR(absY2,        PLOT_DIGITS);
  REAL_T_PTR(hundredYAvg,  PLOT_DIGITS);
  REAL_T_PTR(tenYAvg,      PLOT_DIGITS);
  realCopyAbs(y1, absY1);
  realCopyAbs(y2, absY2);
  realMultiply(const_100, yAvg, hundredYAvg, ctxtGraphs);
  realMultiply(const_10,  yAvg, tenYAvg,     ctxtGraphs);

  bool_t absY2GtHundred = realCompareGreaterThan(absY2, hundredYAvg);
  bool_t absY1LtTen     = realCompareLessThan(absY1, tenYAvg);
  bool_t extremeMagnitudeJump = absY2GtHundred && absY1LtTen;

  bool_t gradientDiscontinuity = false;
  if(!graphIsZero(grad0) && !graphIsZero(grad1) && !graphIsZero(grad2)) {

    // Calculate expected gradient based on trend
    REAL_T_PTR(diffG1G0,        PLOT_DIGITS);
    REAL_T_PTR(expectedGrad,    PLOT_DIGITS);
    REAL_T_PTR(absExpectedGrad, PLOT_DIGITS);
    REAL_T_PTR(absGrad2,        PLOT_DIGITS);
    REAL_T_PTR(epsilon,         PLOT_DIGITS);
    REAL_T_PTR(denom,           PLOT_DIGITS);
    REAL_T_PTR(gradientRatio,   PLOT_DIGITS);
    REAL_T_PTR(diffTmp,         PLOT_DIGITS);
    REAL_T_PTR(absY2mY1,        PLOT_DIGITS);
    REAL_T_PTR(absY1mY0,        PLOT_DIGITS);
    REAL_T_PTR(twentyAbsY1mY0,  PLOT_DIGITS);
    REAL_T_PTR(fiveYAvg,        PLOT_DIGITS);

    realSubtract(grad1, grad0, diffG1G0, ctxtGraphs);                // grad1 - grad0
    realAdd(grad1, diffG1G0, expectedGrad, ctxtGraphs);              // grad1 + (grad1 - grad0)
    realCopyAbs(expectedGrad, absExpectedGrad);
    realCopyAbs(grad2, absGrad2);
    stringToReal("1e-10", epsilon, ctxtGraphs);
    realAdd(absExpectedGrad, epsilon, denom, ctxtGraphs);
    realDivide(absGrad2, denom, gradientRatio, ctxtGraphs);

    realSubtract(y2, y1, diffTmp, ctxtGraphs);
    realCopyAbs(diffTmp, absY2mY1);
    realSubtract(y1, y0, diffTmp, ctxtGraphs);
    realCopyAbs(diffTmp, absY1mY0);

    realMultiply(const_20, absY1mY0, twentyAbsY1mY0, ctxtGraphs);
    realMultiply(const_5,  yAvg,     fiveYAvg,       ctxtGraphs);

    bool_t ratioGt50      = realCompareGreaterThan(gradientRatio, const_50);
    bool_t y2mY1Gt20Y1mY0 = realCompareGreaterThan(absY2mY1, twentyAbsY1mY0);
    bool_t y2mY1Gt5YAvg   = realCompareGreaterThan(absY2mY1, fiveYAvg);

    // Only flag if gradient changes by more than 50x AND the function values suggest discontinuity
    gradientDiscontinuity = ratioGt50 && y2mY1Gt20Y1mY0 && y2mY1Gt5YAvg;

    #if defined(GRAPHDEBUG)
      if(gradientDiscontinuity) {
        char dbgBuf1[64], dbgBuf2[64], dbgBuf3[64];
        realToString(expectedGrad,  dbgBuf1);
        realToString(grad2,         dbgBuf2);
        realToString(gradientRatio, dbgBuf3);
        printf("  gradient details: expected=%s, actual=%s, ratio=%s\n", dbgBuf1, dbgBuf2, dbgBuf3);
      }
    #endif // GRAPHDEBUG
  }

  // Check for sign oscillation that indicates numerical instability not smooth peaks
  bool_t signOscillationInstability = false;
  if(count >= 6) {
    // Look for rapid alternating signs with increasing magnitude - indicates instability
    bool_t y0Pos = !graphIsNegative(y0) && !graphIsZero(y0);
    bool_t y1Pos = !graphIsNegative(y1) && !graphIsZero(y1);
    bool_t y2Pos = !graphIsNegative(y2) && !graphIsZero(y2);
    if(y0Pos != y1Pos && y1Pos != y2Pos) {
      // Alternating signs - check if magnitudes are increasing dramatically
      REAL_T_PTR(mag0,     PLOT_DIGITS);
      REAL_T_PTR(mag1,     PLOT_DIGITS);
      REAL_T_PTR(mag2,     PLOT_DIGITS);
      REAL_T_PTR(fiveMag1, PLOT_DIGITS);
      REAL_T_PTR(fiveMag0, PLOT_DIGITS);
      REAL_T_PTR(tenYAvg2, PLOT_DIGITS);
      realCopyAbs(y0, mag0);
      realCopyAbs(y1, mag1);
      realCopyAbs(y2, mag2);
      realMultiply(const_5,  mag1, fiveMag1, ctxtGraphs);
      realMultiply(const_5,  mag0, fiveMag0, ctxtGraphs);
      realMultiply(const_10, yAvg, tenYAvg2, ctxtGraphs);

      bool_t mag2Gt5Mag1  = realCompareGreaterThan(mag2, fiveMag1);
      bool_t mag1Gt5Mag0  = realCompareGreaterThan(mag1, fiveMag0);
      bool_t mag2Gt10YAvg = realCompareGreaterThan(mag2, tenYAvg2);

      signOscillationInstability = mag2Gt5Mag1 && mag1Gt5Mag0 && mag2Gt10YAvg;
    }
  }

  #if defined(GRAPHDEBUG)
    printf("discontinuity check: extreme=%d, gradient=%d, oscillation=%d\n", extremeMagnitudeJump, gradientDiscontinuity, signOscillationInstability);
  #endif // GRAPHDEBUG
  return extremeMagnitudeJump || gradientDiscontinuity || signOscillationInstability;
}




// =============================================================================
// ASYMPTOTE DETECTION AND RENDERING HELPER FUNCTIONS
// =============================================================================

typedef struct {
  uint32_t xData        [REAL_SIZE_IN_BYTES(PLOT_DIGITS) / 4];   // x-coordinate of asymptote
  uint32_t gapWidthData [REAL_SIZE_IN_BYTES(PLOT_DIGITS) / 4];   // width of the discontinuity gap
  uint32_t maxHeightData[REAL_SIZE_IN_BYTES(PLOT_DIGITS) / 4];   // standard maximum height for rendering
  bool_t   hasPositive;                                          // approaches +infinity
  bool_t   hasNegative;                                          // approaches -infinity
} AsymptoteInfo;

#define AI_X(ai)         ((real_t *)((ai).xData))
#define AI_GAPWIDTH(ai)  ((real_t *)((ai).gapWidthData))
#define AI_MAXHEIGHT(ai) ((real_t *)((ai).maxHeightData))

#define MAX_ASYMPTOTES 10
#define ASYMPTOTE_HEIGHT_RATIO 0.8  // 80% of y-axis range
#define MIN_GAP_WIDTH_RATIO 0.001   // Minimum gap width as ratio of x-range
#define ASYMPTOTE_SAMPLE_POINTS 5   // Points to sample on each side


bool_t detectAndCharacterizeAsymptote(const real_t *xLeft, const real_t *yLeft, const real_t *xRight, const real_t *yRight, const real_t *xGap, const real_t *gapWidth, const real_t *xMin, const real_t *xMax, const real_t *yMin, const real_t *yMax, AsymptoteInfo *asymptote) {
  #if defined(GRAPHDEBUG)
    char strBuf1[64], strBuf2[64], strBuf3[64], strBuf4[64];
    realToString(xGap,     strBuf1);
    realToString(gapWidth, strBuf2);
    realToString(xLeft,    strBuf3);
    realToString(yLeft,    strBuf4);
    printf("Checking asymptote at x=%s, gap=%s\n", strBuf1, strBuf2);
    printf("  Left: x=%s, y=%s\n", strBuf3, strBuf4);
    realToString(xRight, strBuf1);
    realToString(yRight, strBuf2);
    printf("  Right: x=%s, y=%s\n", strBuf1, strBuf2);
  #endif // GRAPHDEBUG

  // Check if gap is significant enough
  REAL_T_PTR(xRange,    PLOT_DIGITS);
  REAL_T_PTR(yRange,    PLOT_DIGITS);
  REAL_T_PTR(minRatio,  PLOT_DIGITS);
  REAL_T_PTR(minGap,    PLOT_DIGITS);
  realSubtract(xMax, xMin, xRange, ctxtGraphs);
  stringToReal(_R_STR_OF(MIN_GAP_WIDTH_RATIO), minRatio, ctxtGraphs);
  realMultiply(minRatio, xRange, minGap, ctxtGraphs);
  if(realCompareLessThan(gapWidth, minGap)) {
    #if defined(GRAPHDEBUG)
      char dbgBuf1[64], dbgBuf2[64];
      realToString(gapWidth, dbgBuf1);
      realToString(minGap,   dbgBuf2);
      printf("  Gap too small: %s < %s\n", dbgBuf1, dbgBuf2);
    #endif // GRAPHDEBUG
    return false;
  }

  // Sample only 2 points on each side to minimize memory usage
  REAL_T_PTR(leftMaxY,  PLOT_DIGITS);
  REAL_T_PTR(rightMaxY, PLOT_DIGITS);
  REAL_T_PTR(leftMinY,  PLOT_DIGITS);
  REAL_T_PTR(rightMinY, PLOT_DIGITS);
  realCopy(yLeft,  leftMaxY);
  realCopy(yLeft,  leftMinY);
  realCopy(yRight, rightMaxY);
  realCopy(yRight, rightMinY);

  // Scratch for sample formulas:
  //   left  side: sampleX = xLeft + (xGap   - xLeft) * (0.7 + 0.2 * i / 2)
  //   right side: sampleX = xGap  + (xRight - xGap)  * (0.2 * i / 2)
  REAL_T_PTR(zeroPoint7, PLOT_DIGITS);
  REAL_T_PTR(zeroPoint2, PLOT_DIGITS);
  REAL_T_PTR(iReal,      PLOT_DIGITS);
  REAL_T_PTR(fineFactor, PLOT_DIGITS);   // 0.2 * i / 2
  REAL_T_PTR(leftFactor, PLOT_DIGITS);   // 0.7 + fineFactor
  REAL_T_PTR(spanLeft,   PLOT_DIGITS);   // xGap - xLeft
  REAL_T_PTR(spanRight,  PLOT_DIGITS);   // xRight - xGap
  REAL_T_PTR(sampleX,    PLOT_DIGITS);
  REAL_T_PTR(sampleY,    PLOT_DIGITS);
  stringToReal("0.7", zeroPoint7, ctxtGraphs);
  stringToReal("0.2", zeroPoint2, ctxtGraphs);
  realSubtract(xGap,   xLeft, spanLeft,  ctxtGraphs);
  realSubtract(xRight, xGap,  spanRight, ctxtGraphs);

  // Sample just 2 points on each side (minimal sampling)
  for(int i = 1; i <= 2; i++) {
    int32ToReal(i, iReal);
    realDivide  (zeroPoint2, const_2,    fineFactor, ctxtGraphs);   // 0.2 / 2
    realMultiply(fineFactor, iReal,      fineFactor, ctxtGraphs);   // (0.2/2) * i
    realAdd     (zeroPoint7, fineFactor, leftFactor, ctxtGraphs);   // 0.7 + ...

    // Left side samples (approaching asymptote from left)
    realMultiply(spanLeft, leftFactor, sampleX, ctxtGraphs);
    realAdd     (xLeft,    sampleX,    sampleX, ctxtGraphs);
    convertRealToReal34RegisterPush(sampleX, REGISTER_X);
    execute_rpn_function_graphAcc();

    // Skip if we get invalid results
    if(real34IsInfinite(REGISTER_REAL34_DATA(REGISTER_Y)) || real34IsNaN(REGISTER_REAL34_DATA(REGISTER_Y))) {
      continue;
    }

    convertRegisterToReal(REGISTER_Y, sampleY);
    if(realCompareGreaterThan(sampleY, leftMaxY)) {
      realCopy(sampleY, leftMaxY);
    }
    if(realCompareLessThan(sampleY, leftMinY)) {
      realCopy(sampleY, leftMinY);
    }

    // Right side samples (approaching asymptote from right)
    realMultiply(spanRight, fineFactor, sampleX, ctxtGraphs);
    realAdd     (xGap,      sampleX,    sampleX, ctxtGraphs);
    convertRealToReal34RegisterPush(sampleX, REGISTER_X);
    execute_rpn_function_graphAcc();

    if(real34IsInfinite(REGISTER_REAL34_DATA(REGISTER_Y)) || real34IsNaN(REGISTER_REAL34_DATA(REGISTER_Y))) {
      continue;
    }

    convertRegisterToReal(REGISTER_Y, sampleY);
    if(realCompareGreaterThan(sampleY, rightMaxY)) {
      realCopy(sampleY, rightMaxY);
    }
    if(realCompareLessThan(sampleY, rightMinY)) {
      realCopy(sampleY, rightMinY);
    }
  }

  // Determine if we have a vertical asymptote based on extreme values
  REAL_T_PTR(extremeThreshold, PLOT_DIGITS);
  REAL_T_PTR(yMaxPlus,         PLOT_DIGITS);
  REAL_T_PTR(yMinMinus,        PLOT_DIGITS);
  realSubtract(yMax, yMin, yRange, ctxtGraphs);
  realMultiply(yRange, const_2, extremeThreshold, ctxtGraphs);   // Values beyond 2x the plot range
  realAdd     (yMax, extremeThreshold, yMaxPlus,  ctxtGraphs);
  realSubtract(yMin, extremeThreshold, yMinMinus, ctxtGraphs);

  bool_t leftGoesPositive  = realCompareGreaterThan(leftMaxY,  yMaxPlus);
  bool_t leftGoesNegative  = realCompareLessThan(leftMinY,  yMinMinus);
  bool_t rightGoesPositive = realCompareGreaterThan(rightMaxY, yMaxPlus);
  bool_t rightGoesNegative = realCompareLessThan(rightMinY, yMinMinus);

  // Must have extreme behavior on at least one side
  if(!(leftGoesPositive || leftGoesNegative || rightGoesPositive || rightGoesNegative)) {
    #if defined(GRAPHDEBUG)
      char dbgBuf1[64], dbgBuf2[64], dbgBuf3[64], dbgBuf4[64];
      realToString(leftMaxY,  dbgBuf1);
      realToString(leftMinY,  dbgBuf2);
      realToString(yMaxPlus,  dbgBuf3);
      realToString(yMinMinus, dbgBuf4);
      printf("  No extreme behavior detected\n");
      printf("    Left: max=%s, min=%s (need >%s or <%s)\n", dbgBuf1, dbgBuf2, dbgBuf3, dbgBuf4);
      realToString(rightMaxY, dbgBuf1);
      realToString(rightMinY, dbgBuf2);
      printf("    Right: max=%s, min=%s\n", dbgBuf1, dbgBuf2);
    #endif // GRAPHDEBUG
    return false;
  }

  // Fill asymptote info
  REAL_T_PTR(heightRatio, PLOT_DIGITS);
  realCopy(xGap,     AI_X(*asymptote));
  realCopy(gapWidth, AI_GAPWIDTH(*asymptote));
  asymptote->hasPositive = leftGoesPositive  || rightGoesPositive;
  asymptote->hasNegative = leftGoesNegative  || rightGoesNegative;
  stringToReal(_R_STR_OF(ASYMPTOTE_HEIGHT_RATIO), heightRatio, ctxtGraphs);
  realMultiply(yRange, heightRatio, AI_MAXHEIGHT(*asymptote), ctxtGraphs);

  #if defined(GRAPHDEBUG)
    char dbgBuf1[64], dbgBuf2[64], dbgBuf3[64];
    realToString(AI_X(*asymptote),         dbgBuf1);
    realToString(AI_GAPWIDTH(*asymptote),  dbgBuf2);
    realToString(AI_MAXHEIGHT(*asymptote), dbgBuf3);
    printf("  ASYMPTOTE DETECTED at x=%s (memory efficient)\n", dbgBuf1);
    printf("    Gap width: %s\n", dbgBuf2);
    printf("    Goes positive: %d, negative: %d\n", asymptote->hasPositive, asymptote->hasNegative);
    printf("    Standard height: %s\n", dbgBuf3);
  #endif // GRAPHDEBUG

  return true;
}



// Direction selectors only, no math content here. Stay as small ints.
typedef struct {
  int8_t dx;   // -1, 0 or +1
  int8_t dy;   // -1, 0 or +1
} PointOffset;

// Three possible templates for asymptotes
TO_QSPI const PointOffset asymptote_offsets_both[] = {
  { -1, -1 }, // (x - offset, -height)
  { -1,  1 }, // (x - offset, +height)
  {  1,  1 }  // (x + offset, +height)
};

TO_QSPI const PointOffset asymptote_offsets_positive[] = {
  { -1,  0 }, // (x - offset, 0)
  { -1,  1 }, // (x - offset, +height)
  {  1,  1 }  // (x + offset, +height)
};

TO_QSPI const PointOffset asymptote_offsets_negative[] = {
  { -1,  0 }, // (x - offset, 0)
  { -1, -1 }, // (x - offset, -height)
  {  1, -1 }  // (x + offset, -height)
};



void renderAsymptote(AsymptoteInfo *asymptote) {
  REAL_T_PTR(xCenter,         PLOT_DIGITS);
  REAL_T_PTR(offset,          PLOT_DIGITS);   // Small x offset
  REAL_T_PTR(x,               PLOT_DIGITS);
  REAL_T_PTR(y,               PLOT_DIGITS);
  realCopy(AI_X(*asymptote), xCenter);
  stringToReal("1e-3",  offset, ctxtGraphs);

  #if defined(GRAPHDEBUG)
    char strBuf1[64];
    realToString(xCenter, strBuf1);
    printf("Rendering asymptote at x=%s with clean 3-point vertical sequence\n", strBuf1);
  #endif // GRAPHDEBUG

  const PointOffset* offsets = NULL;

  if(asymptote->hasPositive && asymptote->hasNegative) {
    offsets = asymptote_offsets_both;
  }
  else if(asymptote->hasPositive) {
    offsets = asymptote_offsets_positive;
  }
  else if(asymptote->hasNegative) {
    offsets = asymptote_offsets_negative;
  }

  if(offsets) {
    for(int i = 0; i < 3; i++) {
      // x = xCenter + dx * offset
      realCopy(xCenter, x);
      if(offsets[i].dx > 0) {
        realAdd(x, offset, x, ctxtGraphs);
      }
      else if(offsets[i].dx < 0) {
        realSubtract(x, offset, x, ctxtGraphs);
      }
      // y = (dy == 0) ? 0 : dy * asymptoteHeight
      if(offsets[i].dy == 0) {
        realSetZero(y);
      }
      else if(offsets[i].dy > 0) {
        realCopy(const_10000, y);
      }
      else {
        realCopy(const_10000, y);
        graphChangeSign(y);
      }
      convertRealToReal34Register(x, REGISTER_X);
      convertRealToReal34Register(y, REGISTER_Y);
      AddtoDrawMx();
    }
  }
  #if defined(GRAPHDEBUG)
    printf("Added 3 asymptote points in clean vertical sequence\n");
  #endif // GRAPHDEBUG
}



bool_t detectTrueDiscontinuityWithAsymptote(const real_t *y0, const real_t *y1, const real_t *y2, const real_t *grad0, const real_t *grad1, const real_t *grad2, const real_t *yAvg, int count, const real_t *x0, const real_t *x1, const real_t *x2, const real_t *xMin, const real_t *xMax, const real_t *yMin, const real_t *yMax, AsymptoteInfo *asymptotes, int *asymptoteCount) {

  if(realIsSpecial(y0) || realIsSpecial(y1) || realIsSpecial(y2)) { //NaN/infinite samples are domain gaps, not discontinuities: no fine-stepping, no asymptote markers
    return false;
  }

  // If not a vertical asymptote, do discontinuity detection
  bool_t hasDiscontinuity = detectTrueDiscontinuity(y0, y1, y2, grad0, grad1, grad2, yAvg, count);

  if(hasDiscontinuity && count >= 4 && *asymptoteCount < MAX_ASYMPTOTES) {
    // Check if this discontinuity might be an asymptote using the detailed method
    REAL_T_PTR(gapWidth, PLOT_DIGITS);
    REAL_T_PTR(xGap,     PLOT_DIGITS);
    REAL_T_PTR(diff,     PLOT_DIGITS);
    REAL_T_PTR(sumX,     PLOT_DIGITS);
    realSubtract(x2, x0, diff, ctxtGraphs);
    realCopyAbs(diff, gapWidth);
    realAdd(x0, x2, sumX, ctxtGraphs);
    realCopy(sumX, xGap);
    realDivideBy2(xGap, ctxtGraphs);                   // (x0 + x2) / 2

    #if defined(GRAPHDEBUG)
      char strBuf1[64], strBuf2[64], strBuf3[64];
      realToString(x0, strBuf1);
      realToString(x1, strBuf2);
      realToString(x2, strBuf3);
      printf("Checking complex discontinuity for asymptote: x0=%s, x1=%s, x2=%s\n", strBuf1, strBuf2, strBuf3);
      realToString(xGap,     strBuf1);
      realToString(gapWidth, strBuf2);
      printf("  Gap center: %s, width: %s\n", strBuf1, strBuf2);
    #endif // GRAPHDEBUG

    AsymptoteInfo candidateAsymptote;

    if(detectAndCharacterizeAsymptote(x0, y0, x2, y2, xGap, gapWidth, xMin, xMax, yMin, yMax, &candidateAsymptote)) {
      // Store the asymptote
      asymptotes[*asymptoteCount] = candidateAsymptote;
      (*asymptoteCount)++;

      // Render the asymptote immediately
      renderAsymptote(&candidateAsymptote);

      #if defined(GRAPHDEBUG)
        printf("Complex asymptote detected and rendered\n");
      #endif // GRAPHDEBUG

      // Return false to prevent normal discontinuity handling
      return false;
    }
  }

  #if defined(GRAPHDEBUG)
    if(hasDiscontinuity) {
      printf("Regular discontinuity detected, not an asymptote\n");
    }
  #endif // GRAPHDEBUG

  return hasDiscontinuity;
}

// =============================================================================
// END OF ASYMPTOTE HELPER FUNCTIONS
// =============================================================================

#if defined(OPTION_GRAPHICS)
  static void graph_eqn(uint16_t mode) {
    #if defined(GRAPHDEBUG_MIN)
      print_caller(NULL);
    #endif //GRAPHDEBUG_MIN
    bool_t plotAborted = false;   // R/S/EXIT or a nested-engine abort: skip the draw and exit dead, do not enter the graph view
    // graph_eqn's 36 working reals and its 10 asymptote records come from the heap, not the frame: 36 x 24 plus 10 x 76 is 1624 bytes, on a 6 kB DM42 program stack.
    // Taken per call so a nested plot through PGMPLT gets its own; every exit leaves through freeWork. The frame falls from 3408 to 1904 bytes on the DM42 build.
    REAL_T_ALLOC(x, PLOT_DIGITS);
    REAL_T_ALLOC(x01, PLOT_DIGITS);
    REAL_T_ALLOC(y01, PLOT_DIGITS);
    REAL_T_ALLOC(y02, PLOT_DIGITS);
    REAL_T_ALLOC(y00, PLOT_DIGITS);                        // Add y00 for improved discontinuity detection
    REAL_T_ALLOC(dy, PLOT_DIGITS);
    REAL_T_ALLOC(dx0, PLOT_DIGITS);
    REAL_T_ALLOC(dx, PLOT_DIGITS);
    REAL_T_ALLOC(grad2, PLOT_DIGITS);
    REAL_T_ALLOC(grad1, PLOT_DIGITS);
    REAL_T_ALLOC(grad0, PLOT_DIGITS);
    REAL_T_ALLOC(prevDx, PLOT_DIGITS);                     // Track previous step size
    REAL_T_ALLOC(yAvg, PLOT_DIGITS);
    REAL_T_ALLOC(x_min_r, PLOT_DIGITS);                    // real_t copy of external double x_min
    REAL_T_ALLOC(x_max_r, PLOT_DIGITS);                    // real_t copy of external double x_max
    REAL_T_ALLOC(y_min_r, PLOT_DIGITS);                    // real_t copy of external double y_min
    REAL_T_ALLOC(y_max_r, PLOT_DIGITS);                    // real_t copy of external double y_max
    REAL_T_ALLOC(jumpBackStartX, PLOT_DIGITS);
    REAL_T_ALLOC(jumpBackStartY, PLOT_DIGITS);
    REAL_T_ALLOC(jumpBackX, PLOT_DIGITS);
    REAL_T_ALLOC(jumpBackDx, PLOT_DIGITS);
    REAL_T_ALLOC(jbY, PLOT_DIGITS);
    REAL_T_ALLOC(discontinuityThreshold, PLOT_DIGITS);
    REAL_T_ALLOC(maxCurvatureChange, PLOT_DIGITS);
    REAL_T_ALLOC(linearSlope, PLOT_DIGITS);
    REAL_T_ALLOC(interpolationError, PLOT_DIGITS);
    REAL_T_ALLOC(curvatureChange, PLOT_DIGITS);
    REAL_T_ALLOC(newDx, PLOT_DIGITS);
    REAL_T_ALLOC(improvementRatio, PLOT_DIGITS);
    REAL_T_ALLOC(highResStartX, PLOT_DIGITS);
    REAL_T_ALLOC(cumulativeCurvatureChange, PLOT_DIGITS);
    REAL_T_ALLOC(baselineCurvatureChange, PLOT_DIGITS);
    REAL_T_ALLOC(savedXBeforeHighres, PLOT_DIGITS);
    REAL_T_ALLOC(savedDxBeforeHighres, PLOT_DIGITS);
    REAL_T_ALLOC(tmpA, PLOT_DIGITS);
    REAL_T_ALLOC(tmpB, PLOT_DIGITS);
    AsymptoteInfo *const asymptotes = malloc(sizeof(AsymptoteInfo) * MAX_ASYMPTOTES);
    if(x == NULL || x01 == NULL || y01 == NULL || y02 == NULL || y00 == NULL || dy == NULL || dx0 == NULL || dx == NULL || grad2 == NULL || grad1 == NULL ||
       grad0 == NULL || prevDx == NULL || yAvg == NULL || x_min_r == NULL || x_max_r == NULL || y_min_r == NULL || y_max_r == NULL || jumpBackStartX == NULL ||
       jumpBackStartY == NULL || jumpBackX == NULL || jumpBackDx == NULL || jbY == NULL || discontinuityThreshold == NULL || maxCurvatureChange == NULL ||
       linearSlope == NULL || interpolationError == NULL || curvatureChange == NULL || newDx == NULL || improvementRatio == NULL || highResStartX == NULL ||
       cumulativeCurvatureChange == NULL || baselineCurvatureChange == NULL || savedXBeforeHighres == NULL || savedDxBeforeHighres == NULL || tmpA == NULL ||
       tmpB == NULL || asymptotes == NULL) {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      goto freeWork;
    }
    currentKeyCode = 255;
    calcMode = CM_GRAPH;
    saveForUndo();
    regStatsXY = findNamedVariable(plotStatMx);

    // Configure the graph-local context: 14 working digits, full ctxtReal39 settings otherwise.
    ctxtGraphsLocal = ctxtReal39;
    ctxtGraphsLocal.digits = GRAPH_WORKING_DIGITS;
    int16_t count = 0;
    int16_t ss0 = 0;
    int16_t ss1 = 0;
    int16_t ss2 = 0;
    uint8_t discontinuityDetected = 0;
    bool_t  grad2IncreaseDetected = false;
    int loop = 0;
    bool_t jumpedBack = false;
    int asymptoteCount = 0;
  #if defined(GRAPHDEBUG)
    char strBuf1[42], strBuf2[42], strBuf3[42], strBuf4[42], strBuf5[42], strBuf6[42];
    REAL_T_PTR(lastSignChange, PLOT_DIGITS);
  #endif // GRAPHDEBUG

    realPlus(x_min, x_min_r, ctxtGraphs);            // x_min_r = x_min rounded to the graph working digits
    realPlus(x_max, x_max_r, ctxtGraphs);            // x_max_r = x_max
    realPlus(y_min, y_min_r, ctxtGraphs);            // y_min_r = y_min
    realPlus(y_max, y_max_r, ctxtGraphs);            // y_max_r = y_max
    realCopy(x_min_r, x01);
    realSetZero(y01);
    realSetZero(y02);
    realSetZero(y00);
    realSetZero(dy);
    realSubtract(x_max_r, x_min_r, tmpA, ctxtGraphs);
    int32ToReal(SCREEN_WIDTH_GRAPH, tmpB);
    realDivide(tmpA, tmpB, tmpA, ctxtGraphs);
    realMultiply(tmpA, const_10, dx0, ctxtGraphs);
    realCopy(dx0, dx);
    realSetOne(grad2);
    realSetOne(grad1);
    realSetOne(grad0);
    realCopy(dx0, prevDx);
    realCopy(const_1on10, yAvg);

  #if defined(GRAPHDEBUG)
    realToString(x_min_r, strBuf1);
    realToString(x_max_r, strBuf2);
    realToString(dx0, strBuf3);
    printf("\n=== GRAPH EQUATION DEBUG START ===\n");
    printf("Initial parameters: xMin=%s, xMax=%s, dx0=%s\n", strBuf1, strBuf2, strBuf3);
    printf("Screen width: %d, SS1=%.1f, SS2=%.1f\n", SCREEN_WIDTH_GRAPH, SS1, SS2);
  #endif // GRAPHDEBUG

    if(graphVariabl1 < FIRST_NAMED_VARIABLE || graphVariabl1 > LAST_NAMED_VARIABLE) {
      regStatsXY = INVALID_VARIABLE;
      goto freeWork;
    }

    fillStackWithReal0();

    convertRealToReal34RegisterPush(x_max_r, REGISTER_X);
    execute_rpn_function_graphAcc();
    convertRegisterToReal(REGISTER_Y, tmpA);
    realCopyAbs(tmpA, tmpA);
    realFMA(const_2, tmpA, yAvg, yAvg, ctxtGraphs);     // yAvg += 2 * |REGISTER_Y|

    if(mode == initDrwMx) {
      fnClDrawMx(3);
      strcpy(plotStatMx, "DrwMX");
      asymptoteCount = 0; // Reset asymptote tracking for new plot
    }
  #if defined(GRAPHDEBUG)
    realToString(dx0, strBuf1);
    printf("dx0=%s discontinuityDetected:%u grad2IncreaseDetected:%u\n", strBuf1, discontinuityDetected, grad2IncreaseDetected);
  #endif // GRAPHDEBUG

    //Main loop, default is 40 x 6 point gaps, across the 240 wide screen
    //  If the gradient is increasing, then the dx is reduced.
    //  That helps going forward, but not looking a the last sample which already jumped too far, so the next part steps back.
    PlotPoint highResBuffer[HIGH_RES_SAMPLE_COUNT];
    int highResCount = 0;
    bool_t inHighResMode = false;
    realSetZero(highResStartX);
    realSetZero(cumulativeCurvatureChange);
    realSetZero(baselineCurvatureChange);
    realSetZero(savedXBeforeHighres);
    realCopy(dx0, savedDxBeforeHighres);

    #if defined(GRAPHDEBUG)
      int cnt = 0;
      int cycleCount = 0;
      realCopy(x_min_r, lastSignChange);
      int signChangeCount = 0;
    #endif //GRAPHDEBUG

    realCopy(x_min_r, x);




// ********** MAIN LOOP ********** STARTS **********

    while(1) {
      // x <= x_max ?
      if(realCompareGreaterThan(x, x_max_r)) break;

      if(lastErrorCode == ERROR_SOLVER_ABORT || programRunStop == PGM_WAITING || exitKeyWaiting()) {   // R/S/EXIT or a nested-engine abort stops the plot (PGM_WAITING survives, lastErrorCode does not)
        lastErrorCode = engineNestingWasRefused ? ERROR_NESTING_TOO_DEEP : ERROR_SOLVER_ABORT;   // a refusal names itself
        if(programRunStop == PGM_RUNNING) {   // halt the outer program too (a plain plot does not otherwise set this); interactive plot left untouched
          programRunStop = PGM_WAITING;
        }
        plotAborted = true;
        break;
      }

      #if defined(GRAPHDEBUG)
        realToString(x, strBuf1);
        realToString(dx, strBuf2);
        printf("\n###################################\n");
        printf("--- Iteration %d: x=%s, dx=%s ---\n", cnt, strBuf1, strBuf2);
      #endif //GRAPHDEBUG

      jumpedBack = false;
      // x = max(x_min, min(x_max, x))
      if(realCompareGreaterThan(x, x_max_r)) {
        realCopy(x_max_r, x);
      }
      if(realCompareLessThan(x, x_min_r)) {
        realCopy(x_min_r, x);
      }

      convertRealToReal34RegisterPush(x, REGISTER_X);
      execute_rpn_function_graphAcc();

      if(lastErrorCode == ERROR_SOLVER_ABORT || programRunStop == PGM_WAITING || exitKeyWaiting()) {   // catch the interrupt in the thin window, before this point is stored and x advanced
        lastErrorCode = engineNestingWasRefused ? ERROR_NESTING_TOO_DEEP : ERROR_SOLVER_ABORT;   // a refusal names itself
        if(programRunStop == PGM_RUNNING) { programRunStop = PGM_WAITING; }
        plotAborted = true;
        break;
      }

      // Handle complex plotting
      if(getSystemFlag(FLAG_CPXPLOT)) {
        fnRCL(REGISTER_Y);
        fnStore(TEMP_REGISTER_1);
        fnImaginaryPart(0);
        fnRCL(TEMP_REGISTER_1);
        fnRealPart(0);
        AddtoDrawMx();
        goto incrementX;
      }

      reduceRegisterYToComponent();
      convertRegisterToReal(REGISTER_Y, y02);

      #if defined(GRAPHDEBUG)
        realToString(y02, strBuf1);
        printf("y02=%s, Anomaly count = %d\n", strBuf1, count);
      #endif // GRAPHDEBUG



// === > === > === > Begin of skip and jump section

      // Calculate gradient and detect anomalies
      if(count > 0) {
        realSubtract(y02, y01, dy, ctxtGraphs);
        if(!realCompareEqual(x, x01)) {
          realSubtract(x, x01, tmpA, ctxtGraphs);
          realDivide(dy, tmpA, grad2, ctxtGraphs);
        }
        else {
          realSetZero(grad2);
        }

        #if defined(GRAPHDEBUG)
          realToString(dy,    strBuf1);
          realToString(grad2, strBuf2);
          printf("dy=%s, grad2=%s\n", strBuf1, strBuf2);

          // Track sign changes for cycle detection
          if(count > 1) {
            bool_t currentPositive = (!graphIsZero(y02) && graphIsPositive(y02));
            bool_t lastPositive    = (!graphIsZero(y01) && graphIsPositive(y01));
            if(currentPositive != lastPositive) {
              signChangeCount++;
              realSubtract(x, lastSignChange, tmpA, ctxtGraphs);
              realToString(x,    strBuf1);
              realToString(tmpA, strBuf2);
              printf("[SIGN CHANGE #%d at x=%s, distance from last=%s]\n",
                     signChangeCount, strBuf1, strBuf2);
              realCopy(x, lastSignChange);
              if(signChangeCount % 2 == 0) {
                cycleCount++;
                printf("[HALF CYCLE #%d COMPLETE]\n", cycleCount);
              }
            }
          }
        #endif // GRAPHDEBUG

        // Update sign states
        ss0 = ss1;
        ss1 = ss2;
        ss2 = graphIsZero(grad2) ? 0 : (graphIsNegative(grad2) ? -1 : 1);
        realCopy(grad1, grad0);
        realCopy(grad2, grad1);

        #if defined(GRAPHDEBUG)
          realToString(grad0, strBuf1);
          realToString(grad1, strBuf2);
          realToString(grad2, strBuf3);
          printf("Signs: ss0=%d, ss1=%d, ss2=%d\n", ss0, ss1, ss2);
          printf("Grads: grad0=%s, grad1=%s, grad2=%s\n", strBuf1, strBuf2, strBuf3);
        #endif // GRAPHDEBUG

        // Detect gradient anomalies using improved logic
        if(!graphIsZero(grad1) && !graphIsZero(grad2)) {
          REAL_T_PTR(absY02OverY01, PLOT_DIGITS);
          REAL_T_PTR(absY01OverY02, PLOT_DIGITS);
          REAL_T_PTR(absG2OverG1,   PLOT_DIGITS);
          REAL_T_PTR(absG1OverG2,   PLOT_DIGITS);
          REAL_T_PTR(absY00,        PLOT_DIGITS);
          REAL_T_PTR(absY01,        PLOT_DIGITS);
          REAL_T_PTR(absY02,        PLOT_DIGITS);
          REAL_T_PTR(twoAbsY01,     PLOT_DIGITS);
          REAL_T_PTR(absG2mG1,      PLOT_DIGITS);
          REAL_T_PTR(absG1mG0,      PLOT_DIGITS);
          REAL_T_PTR(tenAbsG1mG0,   PLOT_DIGITS);
          REAL_T_PTR(ss2_r,         PLOT_DIGITS);
          REAL_T_PTR(oneOhOne,      PLOT_DIGITS);

          stringToReal("1.01", oneOhOne, ctxtGraphs);
          convertDoubleToReal(SS2, ss2_r, ctxtGraphs);

          realDivide(y02, y01, tmpA, ctxtGraphs);
          realCopyAbs(tmpA, absY02OverY01);
          realDivide(y01, y02, tmpA, ctxtGraphs);
          realCopyAbs(tmpA, absY01OverY02);
          realDivide(grad2, grad1, tmpA, ctxtGraphs);
          realCopyAbs(tmpA, absG2OverG1);
          realDivide(grad1, grad2, tmpA, ctxtGraphs);
          realCopyAbs(tmpA, absG1OverG2);
          realCopyAbs(y00, absY00);
          realCopyAbs(y01, absY01);
          realCopyAbs(y02, absY02);
          realMultiply(const_2, absY01, twoAbsY01, ctxtGraphs);
          realSubtract(grad2, grad1, tmpA, ctxtGraphs);
          realCopyAbs(tmpA, absG2mG1);
          realSubtract(grad1, grad0, tmpA, ctxtGraphs);
          realCopyAbs(tmpA, absG1mG0);
          realMultiply(const_10, absG1mG0, tenAbsG1mG0, ctxtGraphs);

          bool_t a1 = realCompareGreaterThan(absY02OverY01, oneOhOne);
          bool_t a2 = realCompareGreaterThan(absG2OverG1, ss2_r);
          bool_t yRatioCheck1 = (a1 && a2);

          bool_t b1 = realCompareGreaterThan(absY01OverY02, oneOhOne);
          bool_t b2 = realCompareGreaterThan(absG1OverG2, ss2_r);
          bool_t yRatioCheck2 = (b1 && b2);

          // Conservative oscillation detection - only flag if it's truly problematic
          bool_t y02BigEnough = realCompareGreaterThan(absY02, twoAbsY01);
          bool_t y00BigEnough = realCompareGreaterThan(absY00, twoAbsY01);
          bool_t signOscillation1 = (ss0 == 1  && ss1 == -1 && ss2 == 1) && y02BigEnough && y00BigEnough;
          bool_t signOscillation2 = (ss0 == -1 && ss1 == 1  && ss2 == -1) && y02BigEnough && y00BigEnough;

          // Zero crossing detection - only if accompanied by large gradient changes
          bool_t bigGradJump = realCompareGreaterThan(absG2mG1, tenAbsG1mG0);
          bool_t y01Pos = !graphIsZero(y01) && graphIsPositive(y01);
          bool_t y02Neg = graphIsNegative(y02);
          bool_t y01Neg = graphIsNegative(y01);
          bool_t y02Pos = !graphIsZero(y02) && graphIsPositive(y02);
          bool_t zeroCrossing1 = (ss1 == 1  && ss2 == -1  && y01Pos && y02Neg) && bigGradJump;
          bool_t zeroCrossing2 = (ss1 == -1 && ss2 == 1   && y01Neg && y02Pos) && bigGradJump;

          grad2IncreaseDetected = yRatioCheck1 || yRatioCheck2 || signOscillation1 || signOscillation2 || zeroCrossing1 || zeroCrossing2;

          #if defined(GRAPHDEBUG)
            realToString(absY02OverY01, strBuf1);
            realToString(absG2OverG1,   strBuf2);
            realToString(absY01OverY02, strBuf3);
            realToString(absG1OverG2,   strBuf4);
            realToString(y01,           strBuf5);
            realToString(y02,           strBuf6);
            printf("Gradient increase checks:\n");
            printf("  yRatio1(%s>1.01 && %s>%.1f): %d\n", strBuf1, strBuf2, SS2, yRatioCheck1);
            printf("  yRatio2(%s>1.01 && %s>%.1f): %d\n", strBuf3, strBuf4, SS2, yRatioCheck2);
            printf("  signOsc1(%d,%d,%d): %d\n", ss0, ss1, ss2, signOscillation1);
            printf("  signOsc2(%d,%d,%d): %d\n", ss0, ss1, ss2, signOscillation2);
            printf("  zeroCross1(ss=%d->%d, y=%s->%s): %d\n", ss1, ss2, strBuf5, strBuf6, zeroCrossing1);
            printf("  zeroCross2(ss=%d->%d, y=%s->%s): %d\n", ss1, ss2, strBuf5, strBuf6, zeroCrossing2);
            printf("  => grad2IncreaseDetected: %d\n", grad2IncreaseDetected);
          #endif // GRAPHDEBUG

        }
        else {
          grad2IncreaseDetected = false;
          #if defined(GRAPHDEBUG)
            printf("Zero gradient detected, no increase flagged\n");
          #endif // GRAPHDEBUG
        }

        // Update running average : Track the largest |y02| seen so far, dampened by count. Initialised before the loop from |f(x_max)|; the count == 0 path is intentionally absent here.
        realCopyAbs(y02, tmpA);
        if(realCompareGreaterThan(tmpA, yAvg)) {
          int32ToReal(count, tmpB);
          realDivide(tmpA, tmpB, tmpA, ctxtGraphs);      // |y02|/count
          realAdd(yAvg, tmpA, yAvg, ctxtGraphs);
        }

        #if defined(GRAPHDEBUG)
          realToString(yAvg, strBuf1);
          printf("yAvg=%s\n", strBuf1);
        #endif // GRAPHDEBUG

        // Use improved discontinuity detection
        if(discontinuityDetected == 0) {
          REAL_T_PTR(x00, PLOT_DIGITS);
          if(count > 1) {
            realSubtract(x01, dx, x00, ctxtGraphs);
          }
          else {
            realCopy(x_min_r, x00);
          }
          // Helper now takes real_t pointers directly and the four plot bounds
          bool_t trueDiscontinuity = detectTrueDiscontinuityWithAsymptote(
            y00,  y01,  y02,
            grad0, grad1, grad2,
            yAvg, count,
            x00,  x01,  x,
            x_min_r, x_max_r, y_min_r, y_max_r,
            asymptotes, &asymptoteCount);
          if(trueDiscontinuity) {
            discontinuityDetected = FINE;
            #if defined(GRAPHDEBUG)
              printf("TRUE DISCONTINUITY DETECTED\n");
            #endif // GRAPHDEBUG
          }
        }

        // Jump-back logic for discontinuities and gradient increases
        if((discontinuityDetected == FINE) || (discontinuityDetected == 0 && grad2IncreaseDetected)) {
          #if defined(GRAPHDEBUG)
            realToString(x,   strBuf1);
            realToString(y02, strBuf2);
            printf("JUMPING BACK: discontinuity=%d, gradIncrease=%d\n", discontinuityDetected, grad2IncreaseDetected);
            printf("  Before jump: x=%s, y=%s\n", strBuf1, strBuf2);
          #endif // GRAPHDEBUG

          // If in high-res mode, abandon it since we're jumping back anyway
          if(inHighResMode) {
            abandonHighResMode(&highResCount, &inHighResMode);
          }

          // Store the current position and original discontinuity trigger
          realCopy(x,   jumpBackStartX);
          realCopy(y02, jumpBackStartY);
          bool_t wasDiscontinuity = (discontinuityDetected == FINE);
          // discontinuityThreshold = wasDiscontinuity ? |y02 - y01| * 0.1 : 0
          if(wasDiscontinuity) {
            realSubtract(y02, y01, tmpA, ctxtGraphs);
            realCopyAbs(tmpA, tmpA);
            realMultiply(tmpA, const_1on10, discontinuityThreshold, ctxtGraphs);
          }
          else {
            realSetZero(discontinuityThreshold);
          }

          // x -= dx * JMP
          convertDoubleToReal(JMP, tmpA, ctxtGraphs);
          realMultiply(dx, tmpA, tmpB, ctxtGraphs);
          realSubtract(x, tmpB, x, ctxtGraphs);
          jumpedBack = true;
          convertRealToReal34RegisterPush(x, REGISTER_X);
          execute_rpn_function_graphAcc();
          reduceRegisterYToComponent();
          convertRegisterToReal(REGISTER_Y, y02);
          // grad2 = (y02 - y01) / (x - x01)
          realSubtract(y02, y01, tmpA, ctxtGraphs);
          realSubtract(x, x01, tmpB, ctxtGraphs);
          realDivide(tmpA, tmpB, grad2, ctxtGraphs);
          ss0 = ss1;
          ss1 = ss2;
          ss2 = graphIsZero(grad2) ? 0 : (graphIsNegative(grad2) ? -1 : 1);

          #if defined(GRAPHDEBUG)
            realToString(x,     strBuf1);
            realToString(y02,   strBuf2);
            realToString(grad2, strBuf3);
            printf("  After jump: x=%s, y=%s, newGrad=%s\n", strBuf1, strBuf2, strBuf3);
            printf("  Will evaluate %d fine-step points\n", FINE);
            if(wasDiscontinuity) {
              realToString(discontinuityThreshold, strBuf4);
              printf("  Discontinuity threshold: %s\n", strBuf4);
            }
          #endif // GRAPHDEBUG

          // Sample the fine-stepped points (PlotPoint still holds doubles)
          PlotPoint jumpBackBuffer[FINE];
          int jumpBackCount = 0;
          realCopy(x, jumpBackX);
          // jumpBackDx = dJMP * dx0
          convertDoubleToReal(dJMP, tmpA, ctxtGraphs);
          realMultiply(tmpA, dx0, jumpBackDx, ctxtGraphs);

          for(int jbStep = 0; jbStep < FINE; jbStep++) {
            // jumpBackX < jumpBackStartX ?
            if(realCompareGreaterEqual(jumpBackX, jumpBackStartX)) break;

            convertRealToReal34RegisterPush(jumpBackX, REGISTER_X);
            execute_rpn_function_graphAcc();
            reduceRegisterYToComponent();
            convertRegisterToReal(REGISTER_Y, jbY);

            jumpBackBuffer[jumpBackCount].stored = false;
            realCopy(jumpBackX, PP_X(jumpBackBuffer[jumpBackCount]));
            realCopy(jbY,       PP_Y(jumpBackBuffer[jumpBackCount]));
            jumpBackCount++;
            realAdd(jumpBackX, jumpBackDx, jumpBackX, ctxtGraphs);
          }

          bool_t shouldCommitPoints = false;

          if(wasDiscontinuity) {
            // For discontinuity cases, validate that fine points actually resolve the discontinuity
            bool_t discontinuityResolved = validateDiscontinuityResolution(
              jumpBackBuffer, jumpBackCount,
              y01, jumpBackStartY, discontinuityThreshold);

            if(discontinuityResolved) {
              #if defined(GRAPHDEBUG)
                printf("  DISCONTINUITY RESOLVED: committing fine points\n");
              #endif // GRAPHDEBUG
              shouldCommitPoints = true;
              discontinuityDetected = 0; // Clear discontinuity flag
            }
            else {
              #if defined(GRAPHDEBUG)
                printf("  DISCONTINUITY PERSISTS: abandoning fine points, but keeping original grid point\n");
              #endif // GRAPHDEBUG
              discontinuityDetected = 0; // Clear flag
              shouldCommitPoints = false;
              // Don't skip the original grid point - restore to original position and let the normal flow handle plotting the original grid point
            }
          }
          else {
            // For gradient increase cases, evaluate if points add significant detail.
            // The analysis runs in normalised coordinates, x' = (x - x01)/spanX and y' = (y - y01)/spanY, so the
            // double maths is O(1) at any data magnitude; the usefulness thresholds are scale-invariant, unchanged.
            if(jumpBackCount >= 3) {
              real_t spanX, spanY;
              realSubtract(jumpBackStartX, x01, &spanX, ctxtGraphs);       // spanX = jumpBackStartX - x01
              realSubtract(jumpBackStartY, y01, &spanY, ctxtGraphs);       // spanY = jumpBackStartY - y01
              if(realIsZero(&spanY)) {                                     // flat region: any y unit gives the same decision, the thresholds scale with it
                realCopy(&spanX, &spanY);
              }
              if(realIsZero(&spanX)) {                                     // zero-width region: nothing to analyse, keep the points
                shouldCommitPoints = true;
              }
              else {
              // Calculate curvature variation in the jump-back region
              double maxCurvD = 0;
              for(int i = 1; i < jumpBackCount - 1; i++) {
                double xi   = normCoord(PP_X(jumpBackBuffer[i]),   x01, &spanX);
                double xim1 = normCoord(PP_X(jumpBackBuffer[i-1]), x01, &spanX);
                double xip1 = normCoord(PP_X(jumpBackBuffer[i+1]), x01, &spanX);
                double yi   = normCoord(PP_Y(jumpBackBuffer[i]),   y01, &spanY);
                double yim1 = normCoord(PP_Y(jumpBackBuffer[i-1]), y01, &spanY);
                double yip1 = normCoord(PP_Y(jumpBackBuffer[i+1]), y01, &spanY);
                double g1d = (yi   - yim1) / (xi   - xim1);
                double g2d = (yip1 - yi  ) / (xip1 - xi  );
                double curvChangeD = fabs(g2d - g1d);
                if(curvChangeD > maxCurvD) {
                  maxCurvD = curvChangeD;
                }
              }
              // Compare with linear interpolation
              double jumpBackStartYd = normCoord(jumpBackStartY, y01, &spanY);     // 1 by construction, or 0 for a flat region
              double jumpBackStartXd = normCoord(jumpBackStartX, x01, &spanX);     // 1 by construction
              double y01d  = 0.0;                                                  // the origin of the normalised coordinates
              double x01d  = 0.0;
              double linearSlopeD = (jumpBackStartYd - y01d) / (jumpBackStartXd - x01d);
              double interpErrD = 0;
              for(int i = 0; i < jumpBackCount; i++) {
                double xi = normCoord(PP_X(jumpBackBuffer[i]), x01, &spanX);
                double yi = normCoord(PP_Y(jumpBackBuffer[i]), y01, &spanY);
                double expectedYd = y01d + linearSlopeD * (xi - x01d);
                double errD = fabs(yi - expectedYd);
                if(errD > interpErrD) {
                  interpErrD = errD;
                }
              }
              // Keep the results in real_t for the debug prints; normalised units
              convertDoubleToReal(maxCurvD,     maxCurvatureChange, ctxtGraphs);
              convertDoubleToReal(linearSlopeD, linearSlope,        ctxtGraphs);
              convertDoubleToReal(interpErrD,   interpolationError, ctxtGraphs);
              // Points are useful if they show significant non-linear behavior
              bool_t jumpBackPointsUseful =
                (interpErrD > 0.1 * fabs(jumpBackStartYd - y01d)) ||
                (maxCurvD   > fabs(linearSlopeD) * 0.5);

              #if defined(GRAPHDEBUG)
                realToString(maxCurvatureChange, strBuf1);
                realToString(interpolationError, strBuf2);
                printf("  Gradient increase evaluation: maxCurv=%s, interpError=%s, useful=%d\n", strBuf1, strBuf2, jumpBackPointsUseful);
              #endif // GRAPHDEBUG
              shouldCommitPoints = jumpBackPointsUseful;
              }
            }
          }

          if(shouldCommitPoints) { // fine points
            #if defined(GRAPHDEBUG)
              printf("  COMMITTING %d jump-back points\n", jumpBackCount);
            #endif // GRAPHDEBUG
            for(int i = 0; i < jumpBackCount; i++) {
              convertRealToReal34RegisterPush(PP_X(jumpBackBuffer[i]), REGISTER_X);
              execute_rpn_function_graphAcc();
              reduceRegisterYToComponent();
              AddtoDrawMx();
            }

            // Also plot the original grid point (jumpBackStartX, jumpBackStartY)
            convertRealToReal34RegisterPush(jumpBackStartX, REGISTER_X);
            execute_rpn_function_graphAcc();
            reduceRegisterYToComponent();
            AddtoDrawMx();

            // Set position to continue from the original grid point
            realCopy(jumpBackStartX, x);
            realCopy(jumpBackStartY, y02);
            realCopy(dx0, dx);              // Reset to original step size
            jumpedBack = false;             // Clear flag since we've handled the plotting

            #if defined(GRAPHDEBUG)
              realToString(x,   strBuf1);
              realToString(y02, strBuf2);
              printf("  Continuing from original grid point: x=%s, y=%s\n", strBuf1, strBuf2);
            #endif // GRAPHDEBUG
          }
          else {  // not fine points
            #if defined(GRAPHDEBUG)
              printf("  DISCARDING %d jump-back points, but preserving original grid flow\n", jumpBackCount);
            #endif // GRAPHDEBUG
            // Restore to the original grid point and continue normal processing, ensures original grid point gets plotted in the normal flow
            realCopy(jumpBackStartX, x);
            realCopy(jumpBackStartY, y02);
            realCopy(dx0, dx);              // Revert to original step size
            jumpedBack = false;             // Clear flag so the original point gets plotted normally

            // Recalculate gradient for the original point
            realSubtract(y02, y01, tmpA, ctxtGraphs);
            realSubtract(x, x01, tmpB, ctxtGraphs);
            realDivide(tmpA, tmpB, grad2, ctxtGraphs);
            ss0 = ss1;
            ss1 = ss2;
            ss2 = graphIsZero(grad2) ? 0 : (graphIsNegative(grad2) ? -1 : 1);

            #if defined(GRAPHDEBUG)
              realToString(x,     strBuf1);
              realToString(y02,   strBuf2);
              realToString(grad2, strBuf3);
              printf("  Restored to original grid: x=%s, y=%s, grad=%s\n", strBuf1, strBuf2, strBuf3);
              printf("  Original grid point will be plotted normally\n");
            #endif // GRAPHDEBUG
          }
        }

        // Calculate curvature change for resolution assessment
        realSetZero(curvatureChange);
        if(count > 1 && !graphIsZero(grad0) && !graphIsZero(grad1)) {
          // curvatureChange = |(grad2 - grad1) - (grad1 - grad0)|
          realSubtract(grad2, grad1, tmpA, ctxtGraphs);
          realSubtract(grad1, grad0, tmpB, ctxtGraphs);
          realSubtract(tmpA, tmpB, tmpA, ctxtGraphs);
          realCopyAbs(tmpA, curvatureChange);
          #if defined(GRAPHDEBUG)
            realToString(curvatureChange, strBuf1);
            realToString(grad2,           strBuf2);
            realToString(grad1,           strBuf3);
            realToString(grad0,           strBuf4);
            printf("curvatureChange=%s = |(%s-%s)-(%s-%s)|\n", strBuf1, strBuf2, strBuf3, strBuf3, strBuf4);
          #endif // GRAPHDEBUG
        }

        // Determine new step size and resolution mode
        calculateNewStepSize(discontinuityDetected, grad1, grad2, grad2IncreaseDetected, dx0, newDx);

        // Check if we're entering high-resolution mode (only if not jumped back) and only trigger high-res for genuine curvature issues, not discontinuity-related step reductions
        // newDx < prevDx * REVERT_THRESHOLD ?
        convertDoubleToReal(REVERT_THRESHOLD, tmpA, ctxtGraphs);
        realMultiply(prevDx, tmpA, tmpB, ctxtGraphs);
        bool_t newDxBelowThresh = realCompareLessThan(newDx, tmpB);
        bool_t curvNonZero = !graphIsZero(curvatureChange) && graphIsPositive(curvatureChange);
        if(!jumpedBack && !inHighResMode && newDxBelowThresh && discontinuityDetected == 0 && curvNonZero) {
          realCopy(x01,    savedXBeforeHighres);     // Save the last good x position
          realCopy(prevDx, savedDxBeforeHighres);

          #if defined(GRAPHDEBUG)
            realToString(newDx,                strBuf1);
            realToString(prevDx,               strBuf2);
            realToString(tmpB,                 strBuf3);
            realToString(curvatureChange,      strBuf4);
            realToString(savedXBeforeHighres,  strBuf5);
            realToString(savedDxBeforeHighres, strBuf6);
            printf("HIGH-RES TRIGGER: newDx(%s) < prevDx(%s) * threshold(%.2f) = %s\n", strBuf1, strBuf2, REVERT_THRESHOLD, strBuf3);
            printf("Curvature-based trigger: discontinuity=%d, curvatureChange=%s\n", discontinuityDetected, strBuf4);
            printf("Saving state: x=%s, dx=%s, cycle=%d\n", strBuf5, strBuf6, cycleCount);
          #endif // GRAPHDEBUG

          enterHighResMode(&inHighResMode, &highResCount,
                           highResStartX, baselineCurvatureChange, cumulativeCurvatureChange,
                           x, curvatureChange);
        }

          #if defined(GRAPHDEBUG)
            else if(!jumpedBack && !inHighResMode && newDxBelowThresh) {
                 realToString(newDx,           strBuf1);
                 realToString(curvatureChange, strBuf2);
                 printf("HIGH-RES BLOCKED: newDx(%s) < threshold but discontinuity=%d or curvatureChange=%s\n", strBuf1, discontinuityDetected, strBuf2);
            }
          #endif // GRAPHDEBUG

        // If in high-res mode, buffer the point and assess improvement
        if(inHighResMode && !jumpedBack) {
          realAdd(cumulativeCurvatureChange, curvatureChange, cumulativeCurvatureChange, ctxtGraphs);

          #if defined(GRAPHDEBUG)
            realToString(cumulativeCurvatureChange, strBuf1);
            realToString(baselineCurvatureChange,   strBuf2);
            printf("HIGH-RES MODE: count=%d/%d, cumCurv=%s, baseline=%s\n", highResCount, HIGH_RES_SAMPLE_COUNT, strBuf1, strBuf2);
          #endif // GRAPHDEBUG

          if(highResCount < HIGH_RES_SAMPLE_COUNT) { // Buffer high-res points in order
            realCopy(x,     PP_X(highResBuffer[highResCount]));
            realCopy(y02,   PP_Y(highResBuffer[highResCount]));
            realCopy(grad2, PP_GRAD(highResBuffer[highResCount]));
            highResBuffer[highResCount].stored = false;
            highResCount++;
            #if defined(GRAPHDEBUG)
              realToString(x,   strBuf1);
              realToString(y02, strBuf2);
              printf("  Buffered point %d: x=%s, y=%s\n", highResCount-1, strBuf1, strBuf2);
            #endif // GRAPHDEBUG
          }
          else {
            // Evaluate if high-res provided sufficient improvement
            // improvementRatio = (baselineCurvatureChange > 0) ? cumCurv / (baseline * count) : 1
            bool_t baselinePos = !graphIsZero(baselineCurvatureChange) && graphIsPositive(baselineCurvatureChange);
            if(baselinePos) {
              realMultiply(baselineCurvatureChange, HIGH_RES_SAMPLE_COUNT_R, tmpB, ctxtGraphs);
              realDivide(cumulativeCurvatureChange, tmpB, improvementRatio, ctxtGraphs);
            }
            else {
              realSetOne(improvementRatio);
            }

            #if defined(GRAPHDEBUG)
              realToString(improvementRatio,          strBuf1);
              realToString(cumulativeCurvatureChange, strBuf2);
              realMultiply(baselineCurvatureChange, HIGH_RES_SAMPLE_COUNT_R, tmpB, ctxtGraphs);
              realToString(tmpB,                      strBuf3);
              printf("EVALUATING HIGH-RES: improvementRatio=%s (need %.2f)\n", strBuf1, MIN_IMPROVEMENT_RATIO);
              printf("  cumCurv=%s, baseline*count=%s\n", strBuf2, strBuf3);
            #endif // GRAPHDEBUG

            convertDoubleToReal(MIN_IMPROVEMENT_RATIO, tmpA, ctxtGraphs);
            if(realCompareGreaterEqual(improvementRatio, tmpA)) {                              // High-res was beneficial, commit buffered points in sequence
              commitHighResPointsInOrder(highResBuffer, highResCount);
              resetHighResTracking(&highResCount, &inHighResMode, cumulativeCurvatureChange);
              // Continue with current step size
            }
            else {                                                   // High-res didn't help, abandon and continue from last good point with larger dx
              #if defined(GRAPHDEBUG)
                realToString(savedXBeforeHighres,  strBuf1);
                realToString(savedDxBeforeHighres, strBuf2);
                printf("HIGH-RES FAILED: reverting to x=%s, dx=%s\n", strBuf1, strBuf2);
              #endif // GRAPHDEBUG
              abandonHighResMode(&highResCount, &inHighResMode);
              realAdd(savedXBeforeHighres, savedDxBeforeHighres, x, ctxtGraphs);
              realCopy(savedDxBeforeHighres, dx);
              // Don't set jumpedBack since we want to continue forward, just with larger steps
              goto incrementX; // Skip to next iteration with the adjusted x
            }
          }
        }

        realCopy(dx,    prevDx);
        realCopy(newDx, dx);

        #if defined(GRAPHDEBUG)
          realToString(prevDx, strBuf1);
          realToString(dx,     strBuf2);
          printf("Step size: prevDx=%s -> dx=%s\n", strBuf1, strBuf2);
        #endif // GRAPHDEBUG
      }

// < === < === < === End of skip and jump section


      // Add point to plot (skip if in high-res buffering mode or jumped back)
      // dx >= 0 ?
      bool_t dxNonNeg = !graphIsNegative(dx);
      if(!jumpedBack && dxNonNeg && !inHighResMode) {
        #if defined(GRAPHDEBUG)
          realToString(x,   strBuf1);
          realToString(y02, strBuf2);
          printf("ADDING POINT TO PLOT: x=%s, y=%s\n", strBuf1, strBuf2);
        #endif // GRAPHDEBUG
        AddtoDrawMx();
      }
      else {
        #if defined(GRAPHDEBUG)
          realToString(dx, strBuf1);
          printf("SKIPPING PLOT: jumpedBack=%d, dx=%s, inHighRes=%d\n", jumpedBack, strBuf1, inHighResMode);
        #endif // GRAPHDEBUG
      }

      // Update state for next iteration
      if(count > 0) {
        realCopy(grad2, grad1);
      }
      realCopy(y01, y00); // Update y00 for improved discontinuity detection
      realCopy(y02, y01);
      realCopy(x,   x01);

      if(discontinuityDetected != 0) {
        discontinuityDetected--;
      }

      count++;
      if(count > 60) {
        break;
      }

      loop++;
      if(checkHalfSec()) {
        progressHalfSecUpdate_Integer(timed, "Iter: ", loop, halfSec_clearZ, halfSec_clearT, halfSec_disp); //timed
      }

      #if defined(DMCP_BUILD)
        if(exitKeyWaiting()) {
          progressHalfSecUpdate_Integer(force+1, "Interrupted Iter:", loop, halfSec_clearZ, halfSec_clearT, halfSec_disp);
          lastErrorCode = ERROR_SOLVER_ABORT;
          if(programRunStop == PGM_RUNNING) { programRunStop = PGM_WAITING; }   // stop the outer program too, else it runs on and re-plots
          plotAborted = true;
          break;
        }
      #endif //DMCP_BUILD

      #if defined(GRAPHDEBUG)
        cnt++;
      #endif //GRAPHDEBUG

      incrementX:
      // x += STEP_OFFSET * dx
      convertDoubleToReal(STEP_OFFSET, tmpA, ctxtGraphs);
      realMultiply(tmpA, dx, tmpB, ctxtGraphs);
      realAdd(x, tmpB, x, ctxtGraphs);
    }

    #if defined(GRAPHDEBUG)
      printf("\n=== GRAPH EQUATION DEBUG END ===\n");
      printf("Total iterations: %d\n", cnt);
      printf("Total sign changes: %d\n", signChangeCount);
      printf("Total half cycles: %d\n", cycleCount);
      printf("================================\n");
    #endif //GRAPHDEBUG

    if(inHighResMode && highResCount > 0) {
      abandonHighResMode(&highResCount, &inHighResMode);
    }

    if(!plotAborted) {   // leave the interrupted stack intact; a completed plot clears it as before
      fillStackWithReal0();
    }

    #if defined(LOW_GRAPH_ACC)
      int32_t s34 = ctxtReal34.digits, s39 = ctxtReal39.digits, s51 = ctxtReal51.digits, s75 = ctxtReal75.digits;
      //Change to SDIGS digit operation for screen graphs;
      if(significantDigitsForEqnGraphs <= 6) {
        ctxtReal34.digits = significantDigitsForScreen;
        ctxtReal39.digits = significantDigitsForScreen+3;
      }
    #endif //LOW_GRAPH_ACC

    bool_t showGraph = !plotAborted;
    #if ALLOW_NESTED_PARTPLOT_ON_EXIT
      showGraph = true;   // legacy: draw the partial graph even on an interrupted plot
    #endif
    if(showGraph) {
      if(!GRAPHMODE) { //Change over hourglass to the left side
        clearScreenOld(clrStatusBar, !clrRegisterLines, !clrSoftkeys);
      }
      calcMode = CM_GRAPH;
      hourGlassIconEnabled = true;       //clear the current portion of statusbar
      showHideHourGlass();
      refreshStatusBar();
      fnPlot(0);
    }
    else {   // interrupted plot: stop dead on the PLT step, do not enter the graph view; stack left intact for inspection
      calcMode = CM_NORMAL;
      screenUpdatingMode = SCRUPD_AUTO;
      screenUpdatingMode |= SCRUPD_SKIP_STATUSBAR_ONE_TIME;
    }
    #if defined(LOW_GRAPH_ACC)
      //Restore saved, not native: PLOT can run nested via PGMPLT
      ctxtReal34.digits = s34;
      ctxtReal39.digits = s39;
      ctxtReal51.digits = s51;
      ctxtReal75.digits = s75;
    #endif //LOW_GRAPH_ACC

    freeWork:
    REAL_T_FREE(x, PLOT_DIGITS);
    REAL_T_FREE(x01, PLOT_DIGITS);
    REAL_T_FREE(y01, PLOT_DIGITS);
    REAL_T_FREE(y02, PLOT_DIGITS);
    REAL_T_FREE(y00, PLOT_DIGITS);
    REAL_T_FREE(dy, PLOT_DIGITS);
    REAL_T_FREE(dx0, PLOT_DIGITS);
    REAL_T_FREE(dx, PLOT_DIGITS);
    REAL_T_FREE(grad2, PLOT_DIGITS);
    REAL_T_FREE(grad1, PLOT_DIGITS);
    REAL_T_FREE(grad0, PLOT_DIGITS);
    REAL_T_FREE(prevDx, PLOT_DIGITS);
    REAL_T_FREE(yAvg, PLOT_DIGITS);
    REAL_T_FREE(x_min_r, PLOT_DIGITS);
    REAL_T_FREE(x_max_r, PLOT_DIGITS);
    REAL_T_FREE(y_min_r, PLOT_DIGITS);
    REAL_T_FREE(y_max_r, PLOT_DIGITS);
    REAL_T_FREE(jumpBackStartX, PLOT_DIGITS);
    REAL_T_FREE(jumpBackStartY, PLOT_DIGITS);
    REAL_T_FREE(jumpBackX, PLOT_DIGITS);
    REAL_T_FREE(jumpBackDx, PLOT_DIGITS);
    REAL_T_FREE(jbY, PLOT_DIGITS);
    REAL_T_FREE(discontinuityThreshold, PLOT_DIGITS);
    REAL_T_FREE(maxCurvatureChange, PLOT_DIGITS);
    REAL_T_FREE(linearSlope, PLOT_DIGITS);
    REAL_T_FREE(interpolationError, PLOT_DIGITS);
    REAL_T_FREE(curvatureChange, PLOT_DIGITS);
    REAL_T_FREE(newDx, PLOT_DIGITS);
    REAL_T_FREE(improvementRatio, PLOT_DIGITS);
    REAL_T_FREE(highResStartX, PLOT_DIGITS);
    REAL_T_FREE(cumulativeCurvatureChange, PLOT_DIGITS);
    REAL_T_FREE(baselineCurvatureChange, PLOT_DIGITS);
    REAL_T_FREE(savedXBeforeHighres, PLOT_DIGITS);
    REAL_T_FREE(savedDxBeforeHighres, PLOT_DIGITS);
    REAL_T_FREE(tmpA, PLOT_DIGITS);
    REAL_T_FREE(tmpB, PLOT_DIGITS);
    free(asymptotes);
  }
#endif //OPTION_GRAPHICS

//******************************************************************************************************************************



void graph_stat(uint16_t unusedButMandatoryParameter) {
    saveForUndo();
    strcpy(plotStatMx, "STATS");
    plotXYroute = true;   // plotXYn: PLSTAT is the only route allowed

    if(statMxN()) {
      lastPlotMode = PLOT_NOTHING;
      calcMode = CM_GRAPH;
      reDraw = true;

      if(!getSystemFlag(FLAG_PLINE) && !getSystemFlag(FLAG_PCROS) && !getSystemFlag(FLAG_PBOX) && !getSystemFlag(FLAG_PPLUS)) {
        fnPline(NOPARAM);
      }

      fillStackWithReal0();
      fnPlotSQ(0);
    }
    else {
      calcMode = CM_NORMAL;
      displayCalcErrorMessage(ERROR_NO_SUMMATION_DATA, ERR_REGISTER_LINE);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        sprintf(errorMessage, "There is no statistical/plot data available!");
        moreInfoOnError("In function graph_stat:", errorMessage, NULL, NULL);
      #endif
    }
}


// COMPLEX SOLVER

#if defined(OPTION_GRAPHICS)
  // =============================================================================
  // SOLVER HELPERS
  // =============================================================================

  static inline bool_t checkRealZeroTol(const real_t *a, const real_t *tol) {
    return realIsZero(a) || realCompareAbsLessThan(a, tol);
  }

  static inline bool_t check2RealZeroTol(const real_t *a, const real_t *b, const real_t *tol) {
    return checkRealZeroTol(a, tol) && checkRealZeroTol(b, tol);
  }

  static void divFunctionComplex(const real_t *a_re, const real_t *a_im, const real_t *b_re, const real_t *b_im, real_t *res_re, real_t *res_im) {
    if(  (realIsZero(a_re) && realIsZero(a_im)) || realIsNaN(a_re) || realIsNaN(a_im) || realIsNaN(b_re) || realIsNaN(b_im)) {
      realSetZero(res_re);
      realSetZero(res_im);
      return;
    }
    if(realIsZero(b_re) && realIsZero(b_im)) {
      stringToReal("1E30", res_re, ctxtSolver2);
      realSetZero(res_im);
      return;
    }
    divComplexComplex(a_re, a_im, b_re, b_im, res_re, res_im, ctxtSolver2);
  }

  bool_t check_osc(real_t *new, real_t *old, uint8_t *ii){
    if( realGetSign(new) ^ realGetSign(old) ) {
      *ii = (*ii << 1) + 1;
    }
    else {
     *ii = *ii << 1;
    }

    switch(*ii & 0b00111111) {
      case 0b010101:
      case 0b101010:

      case 0b111111:
        return true;
      default: ;
    }
    switch(*ii) {
      case 0b01101101:
      case 0b11011011:
      case 0b10110110:

      case 0b00100100:
      case 0b01001001:
      case 0b10010010:

      case 0b11001100:
      case 0b10011001:
      case 0b00110011:
      case 0b01100110:
        return true;
      default: ;
    }
    return false;
  }


  #if defined(PC_BUILD)
    static void printSolverResult(uint16_t iterationCounter) {
      char str[200];
      real34ToString(REGISTER_REAL34_DATA(REGISTER_X), str);
      printf("\n\n\033[1m%2u: %-36s ", significantDigits, str);
      real34ToString(REGISTER_IMAG34_DATA(REGISTER_X), str);
      if(real34IsNegative(REGISTER_IMAG34_DATA(REGISTER_X))) {
        printf("- ix%-36s", str + 1);
      }
      else {
        printf("+ ix%-36s", str);
      }
      printf(" (iter:%2i code:%i)\033[0m\n\n", iterationCounter, real34ToInt32(REGISTER_REAL34_DATA(REGISTER_T)));

    }
  #else
    static inline void printSolverResult(uint16_t iterationCounter) {}
  #endif // PC_BUILD


#if defined(PC_BUILD) //&& (defined(VERBOSE_SOLVER00) || defined(VERBOSE_SOLVER0) || defined(VERBOSE_SOLVER1) || defined(VERBOSE_SOLVER2))
  static void printComplexToConsole(const real_t *re, const real_t *im, const char *before, const char *after) {
    char str[100];
    realToString(re, str);
    printf("%s%43s + ", before, str);
    realToString(im, str);
    printf("%43si %s", str, after);
  }
#endif //PC_BUILD

  static inline void copyComplex(const cplx_t *from, cplx_t *to) {
    realCopy(&from->Real, &to->Real);
    realCopy(&from->Imag, &to->Imag);
  }
static cplx_t cpxSlvBestX;
static real_t cpxSlvBestMagnitudeY;
// saves best solution to cpxSlvBestX and returns true if converging
static bool_t execute_rpn_function_reals(const cplx_t *from, cplx_t *to, real_t *magnitude) {
  convertComplexToResultRegister(&from->Real, &from->Imag, REGISTER_X);
  execute_rpn_function();
  getRegisterAsComplex(REGISTER_Y, &to->Real, &to->Imag);
  complexMagnitude(&to->Real, &to->Imag, magnitude,  ctxtSolver2);
  if(realCompareLessEqual(magnitude, &cpxSlvBestMagnitudeY)) {
    copyComplex(from, &cpxSlvBestX);
    if(realCompareLessThan(magnitude, &cpxSlvBestMagnitudeY)) {
      realCopy(magnitude, &cpxSlvBestMagnitudeY);
    }
    return true;
  }
  return false;
}

static inline void powCplxNat(const cplx_t *base, const uint8_t *exp, cplx_t *res) {
  cplx_t tmp;
  copyComplex(base, &tmp);
  for(uint8_t i = 1; i<*exp; i++) {
     mulComplexComplex(CPLX(tmp), &base->Real,  &base->Imag, CPLX(tmp), ctxtSolver2);
  }
  copyComplex(&tmp, res);
}

  // =============================================================================
  // END SOLVER HELPER FUNCTIONS
  // =============================================================================

  static uint8_t cpxSlvUserRoundingMode;                               // RM while the complex solver runs at RM_ENGINE

  static void complexSolver() {         //Input parameters in registers SREG_STARTX0, SREG_STARTX1
    currentKeyCode = 255;
    if(graphVariabl1 <= 0 || graphVariabl1 > LAST_LABEL) {
#if defined(PC_BUILD) //PC_BUILD
      printf("Error: No complex solver variable %u\n", graphVariabl1);
#endif //PC_BUILD
      return;
    }

    calcMode = CM_NO_UNDO;

    runFunction(ITM_RAD);
    setSystemFlag(FLAG_CPXRES);
    int16_t oscillationIterationCounter = 0;
    int16_t oscillations = 0;
    int16_t convergent = 0;
    int16_t iterAfterBest = 0;
    int iterationCounter = 0;
    bool_t checkNaN = false;
    bool_t Y2IsZero = false;
    bool_t Y2IsCloseToZero = false;
    bool_t dXdYIsZero = false;
    osc = 0;
    DXR = 0, DYR = 0, DXI = 0, DYI = 0;
    int16_t kicker = 1;
    uint8_t yPower = 1;

    // The working values come from the heap, not the frame: five reals at 60 bytes and sixteen complex at 120 is 2220 bytes, and the frame stands for the whole solve while
    // every evaluation of the user formula runs below it. Measured on the DM42 build the frame falls from 2320 bytes to 208. Freed on every exit through freeWork.
    REAL_T_ALLOC(f,             75);
    REAL_T_ALLOC(tol,           75);
    REAL_T_ALLOC(tolClose,      75);
    REAL_T_ALLOC(oldMagnitudeY, 75);
    REAL_T_ALLOC(magnitudeY,    75);

    CPLX_T_ALLOC(X0);
    CPLX_T_ALLOC(X1);
    CPLX_T_ALLOC(X2);
    CPLX_T_ALLOC(X2N);
    CPLX_T_ALLOC(dX);
    CPLX_T_ALLOC(dXold);

    CPLX_T_ALLOC(Y0);
    CPLX_T_ALLOC(Y1);
    CPLX_T_ALLOC(Y2);
    CPLX_T_ALLOC(Y2N);
    CPLX_T_ALLOC(dY);
    CPLX_T_ALLOC(dYold);

    CPLX_T_ALLOC(temp0);
    CPLX_T_ALLOC(temp1);
    CPLX_T_ALLOC(temp2);
    CPLX_T_ALLOC(temp3);

    if(f == NULL || tol == NULL || tolClose == NULL || oldMagnitudeY == NULL || magnitudeY == NULL ||
       X0 == NULL || X1 == NULL || X2 == NULL || X2N == NULL || dX == NULL || dXold == NULL ||
       Y0 == NULL || Y1 == NULL || Y2 == NULL || Y2N == NULL || dY == NULL || dYold == NULL ||
       temp0 == NULL || temp1 == NULL || temp2 == NULL || temp3 == NULL) {
      displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
      calcMode = CM_NORMAL;
      goto freeWork;
    }


    // Initialize
    getRegisterAsComplex(REGISTER_X, CPLX(*X1));
    getRegisterAsComplex(REGISTER_Y, CPLX(*X0));
    copyComplex(X0, &cpxSlvBestX);

    realCopy(const_1e32, &cpxSlvBestMagnitudeY);

    //if input parameters X0 and X1 are the same, add a random number to X0
    if(realCompareEqual(&X0->Real, &X1->Real) && realCompareEqual(&X0->Imag, &X1->Imag)) {
#if defined(PC_BUILD)
      printf(">>> ADD 1 to second input parameter to prevent infinite result\n");
#endif
      realAdd(&X1->Real, const_1, &X1->Real, ctxtSolver2);
    }


    realSetZero(&dXold->Real);
    realSetZero(&dXold->Imag);
    copyComplex(dXold, dYold);
    copyComplex(dXold, X2N);
    copyComplex(dXold, dX);
    // initial value for difference comparison must be larger than tolerance
    realCopy(const_1on10, &dX->Real);
    copyComplex(dX, dY);

    realCopy(const_1on2, f); // factor ()

    // set tolerance from significantDigits and use higher prcision in execute_rpn_function();
    uint16_t signDig  = significantDigits ? significantDigits : 34;

    realSetOne(tol);
    tol->exponent -= signDig <= 4 ? 4 : (signDig > 32 ? 32 : signDig);
    realSetOne(tolClose);
    tolClose->exponent -= signDig <= 4 ? 3 : (signDig > 27 ? 27 : signDig - 1);
    fnSetSignificantDigits(34);

    execute_rpn_function_reals(X0, Y0, magnitudeY);
    execute_rpn_function_reals(X1, Y1, oldMagnitudeY);

    // check if an initial value is a solution
    if(checkRealZeroTol(&cpxSlvBestMagnitudeY, tol)) {
      Y2IsZero = true;
    }
    else {
      subComplex(CPLX(*Y1), CPLX(*Y0), CPLX(*temp1), ctxtSolver2);  //dy=y1-y0
      // avoid equal Y as it causes double iterations
      if(check2RealZeroTol(CPLX(*temp1), tol)) {
        addComplex(CPLX(*X0), const_1e_6, const_0, CPLX(*X0), ctxtSolver2);
        execute_rpn_function_reals(X0, Y0, magnitudeY);
        subComplex(CPLX(*Y1), CPLX(*Y0), CPLX(*temp1), ctxtSolver2);  //dy=y1-y0
      }
      subComplex(CPLX(*X1), CPLX(*X0), CPLX(*temp0), ctxtSolver2);  //dx=x1-x0
      divFunctionComplex( CPLX(*temp0), CPLX(*temp1), CPLX(*temp0));  //dx/dy
      mulComplexComplex( CPLX(*temp0), CPLX(*Y1), CPLX(*temp0), ctxtSolver2);  //deltaX = x1 - x2 = Y1 / (dy/dx) = Y1 x 1/(dy/dx) = Y1 x dx/dy
      subComplex(CPLX(*X1), CPLX(*temp0), CPLX(*X2), ctxtSolver2);  //x2=x1-deltaX
      if(realIsZero(&X2->Imag)) {
        // realCopy(&temp0->Real, &X2->Imag);
        // X2->Imag.exponent -= 1;
        realMultiply(&X2->Real, const39_1on3, &X2->Imag, ctxtSolver2);
      }
    }

                                        #if defined(VERBOSE_SOLVER00) || defined(VERBOSE_SOLVER0) || defined(VERBOSE_SOLVER1) || defined(VERBOSE_SOLVER2)
                                            execute_rpn_function_reals(X2, Y2, magnitudeY);
                                            printf("INIT:   iterationCounter=%d \n", iterationCounter);
                                            printComplexToConsole(CPLX(*X0), "Init X0=", "\n");
                                            printComplexToConsole(CPLX(*X1), "Init X1=", "\n");
                                            printComplexToConsole(CPLX(*X2), "Init X2=", "\n");
                                            printComplexToConsole(CPLX(*Y0), "Init Y0=", "\n");
                                            printComplexToConsole(CPLX(*Y1), "Init Y1=", "\n");
                                            printComplexToConsole(CPLX(*Y2), "Init Y2=", "\n");
                                        #endif // VERBOSE_SOLVER00 || VERBOSE_SOLVER0 || VERBOSE_SOLVER1 || VERBOSE_SOLVER2


    //###############################################################################################################
    //#################################################### Iteration start ##########################################
    while(iterationCounter<NUMBERITERATIONS && !checkNaN && !Y2IsZero && !dXdYIsZero) {
                                        #if defined(VERBOSE_SOLVER0)
                                              printf("\nIteration start\v");
                                        #endif
      if(lastErrorCode != 0) {
                                        #if defined(PC_BUILD)
                                                printf(">>> ERROR CODE INITIALLY NON-ZERO = %d <<<<<\n", lastErrorCode);
                                        #endif //PC_BUILD
        break;
      }

      //Identify oscillations in real or imag: increment osc flag
      osc = check_osc(&dY->Real, &dYold->Real, &DYR);
      osc = (osc << 1) + check_osc(&dY->Imag, &dYold->Imag, &DYI);
      osc = (osc << 1) + check_osc(&dX->Real, &dXold->Real, &DXR);
      osc = (osc << 1) + check_osc(&dX->Imag, &dXold->Imag, &DXI);

      //If osc flag is active, that is any delta polarity change, then increment oscillation count
      if(osc && (realGetExponent(magnitudeY) - realGetExponent(oldMagnitudeY) >= -2)) { //only increment if convergence is less than ca. 1 %, otherwise assume it is a damped oscillation
        oscillations++;
      }
      else {
        oscillations = max(0, oscillations-1);
      }

      //If converging, increment convergence counter
      if(realCompareLessThan(magnitudeY, oldMagnitudeY)) { // && realCompareLessThan(&temp0->Real, &temp0->Imag))
        convergent++;
      }
      else {
        if(Y2IsCloseToZero) {
          Y2IsZero = true; // if close to solution stop if converge strike is over
        }
        else {
          convergent = max(-3, convergent-2);
        }
      }
      realCopy(magnitudeY, oldMagnitudeY);
                                        #if defined(VERBOSE_SOLVER0)
                                              printf("##### iterationCounter= %d osc= %d  conv= %d n\n", iterationCounter, oscillations, convergent);
                                        #endif // VERBOSE_SOLVER0
                                        #if defined(VERBOSE_SOLVER1)
                                              printf("################################### iterationCounter= %d osc= %d  conv= %d ###########################################\n", iterationCounter, oscillations, convergent);
                                        #endif //VERBOSE_SOLVER1

      if(!Y2IsZero) { // only do the convergence and oscillation checks if Y is not zero
        if(convergent > 6 && oscillations > 3) {
          convergent = 2;
          oscillations = 1;
                                          #if defined(VERBOSE_SOLVER0)
                                                  printf("#    --   reset detection\n");
                                                  printf("%i and %i\n", convergent, oscillations);
                                          #endif // VERBOSE_SOLVER0
        }

        if(((convergent <= -2 && kicker > yPower*3) || kicker > 8) && yPower < 5) {
          osc = 0;
          convergent = 0;
          oscillations = 0;
          kicker = 3;
          if(yPower>1) {
            execute_rpn_function_reals(X0, Y0, oldMagnitudeY);
            execute_rpn_function_reals(X1, Y1, magnitudeY);
          }
          yPower += 2;
          powCplxNat(Y0, &yPower, Y0);
          powCplxNat(Y1, &yPower, Y1);
  #if defined(PC_BUILD)
          printf("-------- yPower: %u, iter: %u\n", yPower, iterationCounter);
  #endif // PC_BUILD
        }
        copyComplex(X2, temp0);
        // If increment is oscillating it is assumed that it is unstable and needs to have a complex starting value
        if(iterationCounter==0 || (((oscillations >= 2)
              && (oscillationIterationCounter > 10) // prime - 1 to not sync with oscillation
              && (convergent <= 2)) )) {
          oscillationIterationCounter = 0;
          oscillations = 0;
          convergent = 0;
                                          #if defined(VERBOSE_SOLVER2)
                                                  printComplexToConsole(CPLX(*X2), "\n>>>>>>>>>> from ", "");
                                          #endif // VERBOSE_SOLVER2
          double p2 = 1.0;                       // 2^kicker by exact doubling; kicker is a small integer and this keeps libm's pow out of the link
          for(int16_t k2 = kicker; k2 > 0; k2--) {
            p2 *= 2.0;
          }
          for(int16_t k2 = kicker; k2 < 0; k2++) {
            p2 *= 0.5;
          }
          double kick = 0.8123 * kicker * kicker * p2;
          convertDoubleToReal(kicker%2 ? -kick : kick, &temp1->Real, ctxtSolver2);
          convertDoubleToReal(kick, &temp1->Imag, ctxtSolver2);
          addComplex(CPLX(*temp1), CPLX(*X0), CPLX(*X2), ctxtSolver2);
                                          #if defined(PC_BUILD)
                                                  printf("------- Kick #%d, iter:%u ", kicker, iterationCounter);
                                                  printComplexToConsole(CPLX(*temp1), "added: ", "\n");
                                          #endif  // PC_BUILD
                                          #if defined(VERBOSE_SOLVER2)
                                                  printComplexToConsole(CPLX(*X2), " to ", "\n");
                                          #endif // VERBOSE_SOLVER2
                                                  kicker++;

        }
      }

      //@@@@@@@@@@@@@@@@@ CALCULATE NEW Y2, AND PLAUSIBILITY @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
      // if same as cpxSlvBestX we probably hit the precision limit for this equation?
      subComplex(CPLX(cpxSlvBestX), CPLX(*X2), CPLX(*temp1), ctxtSolver2);
      complexMagnitude(CPLX(*temp1), &temp1->Real,  ctxtSolver2);
      Y2IsCloseToZero = Y2IsCloseToZero || (realCompareLessThan(&cpxSlvBestMagnitudeY, const_1e_6) && realIsZero(&temp1->Real) && realIsZero(&temp1->Imag));

      iterAfterBest = execute_rpn_function_reals(X2, Y2N, magnitudeY) ? 0 : iterAfterBest + 1;
      powCplxNat(Y2N, &yPower, Y2);
      if(realIsInfinite(&Y2->Real) || realIsInfinite(&Y2->Imag)) {
        // Revert kick
                                        #if defined(PC_BUILD)
                                                printf("----- Inf.Y iter:%u  revert kick", iterationCounter);
                                        #endif  // PC_BUILD
        copyComplex(temp0, X2);
        execute_rpn_function_reals(X2, Y2N, magnitudeY);
        powCplxNat(Y2N, &yPower, Y2);
        kicker-=2;
      }



                                        #if defined(VERBOSE_SOLVER1)
                                              printf("    :   iterationCounter=%d", iterationCounter);
                                              printComplexToConsole(CPLX(*X2), " X2=", " ");
                                              printComplexToConsole(CPLX(*Y2N), " Y2=", "\n");
                                        #endif // VERBOSE_SOLVER1

      // check if an acceptable solution is found
      Y2IsZero = Y2IsZero ||   checkRealZeroTol(magnitudeY, tol);
      checkNaN  = checkNaN  ||   realIsNaN(&X2->Real) || realIsNaN(&X2->Imag) ||
        realIsNaN(&Y2N->Real) || realIsNaN(&Y2N->Imag);
      Y2IsCloseToZero = Y2IsCloseToZero ||   checkRealZeroTol(magnitudeY, tolClose);


                                        #if defined(VERBOSE_SOLVER_ITERDATA)
                                            float dbYr, dbYi;
                                            char *arrows[8] = {"→", "↗︎", "↑", "↖︎", "←", "↙︎", "↓", "↘︎"};
                                            realToFloat(&Y2N->Real, &dbYr);
                                            realToFloat(&Y2N->Imag, &dbYi);
                                            uint8_t ang = mod((int)(4.0 * (atan2((double)dbYi, (double)dbYr)) / M_PI+8.5), 8);
                                            double magn =  sqrt((double)dbYr * (double)dbYr + (double)dbYi * (double)dbYi);
                                            printf("#%-4u osc=%-2i conv=%-2i close=%i !best=%-2u Y=%s%5.0e ",
                                                  iterationCounter,
                                                  oscillations,
                                                  convergent,
                                                  Y2IsCloseToZero,
                                                  iterAfterBest,
                                                  arrows[ang%8],
                                                  magn);
                                                  printComplexToConsole(CPLX(*X2), "X=", "\n");
                                        #endif // VERBOSE_SOLVER_ITERDATA

                                        #if defined(VERBOSE_SOLVER00) || defined(VERBOSE_SOLVER0)
                                              if(checkNaN || iterationCounter==NUMBERITERATIONS-1 || Y2IsZero) {
                                                printf("-->A Endflags zero: Y2r=0:%u Y2i=0:%u X2r=NaN:%u X2i=NaN:%u Y2r=NaN:%u Y2i=NaN%u \n",
                                                (uint16_t)realIsZero(&Y2->Real), (uint16_t)realIsZero(&Y2->Imag),
                                                (uint16_t)realIsNaN(&X2->Real), (uint16_t)realIsNaN(&X2->Imag),
                                                (uint16_t)realIsNaN(&Y2->Real), (uint16_t)realIsNaN(&Y2->Imag)
                                                  );
                                              }
                                        #endif // VERBOSE_SOLVER00 || VERBOSE_SOLVER0

                                        #if defined(VERBOSE_SOLVER2)
                                              printf("   iterationCounter=%d checkend=%d X2=", iterationCounter, checkNaN || iterationCounter==NUMBERITERATIONS-1 || Y2IsZero);
                                              printComplexToConsole(CPLX(*X2), "", "");
                                              printComplexToConsole(CPLX(*Y2), "Y2=", "\n");
                                        #endif // VERBOSE_SOLVER2

      //*************** DETERMINE DX and DY, to calculate the slope (or the inverse of the slope in this case) *******************
      copyComplex(dX, dXold);  // store old DELTA values, for oscillation check
      copyComplex(dY, dYold);  // store old DELTA values, for oscillation check

      // ---------- Modified 3 point Secant ------------
                                        #if defined(VERBOSE_SOLVER00) || defined(VERBOSE_SOLVER0) || defined(VERBOSE_SOLVER1) || defined(VERBOSE_SOLVER2)
                                                printf("%3i ---------- Modified 3 point Secant ------------ osc=%d conv=%d\n", iterationCounter, oscillations, convergent);
                                                printComplexToConsole(CPLX(*X0), "           X0=", "\n");
                                                printComplexToConsole(CPLX(*Y0), "           Y0=", "\n");
                                                printComplexToConsole(CPLX(*X1), "           X1=", "\n");
                                                printComplexToConsole(CPLX(*Y1), "           Y1=", "\n");
                                                printComplexToConsole(CPLX(*X2), "           X2=", "\n");
                                                printComplexToConsole(CPLX(*Y2), "           Y2=", "\n");

                                        #endif // VERBOSE_SOLVER00 || VERBOSE_SOLVER0 || VERBOSE_SOLVER1 || VERBOSE_SOLVER2
      if((iterationCounter == 0) || (!Y2IsZero && !dXdYIsZero && !checkNaN)) {

        subComplex(CPLX(*Y2), CPLX(*Y1), CPLX(*dY), ctxtSolver2); // Y2-Y1 = dY
        subComplex(CPLX(*X2), CPLX(*X1), CPLX(*dX), ctxtSolver2); // X2-X1 = dX
        divFunctionComplex(CPLX(*dY), CPLX(*dX),  CPLX(*temp0)); // dY/dX = temp0

                                        #if defined(VERBOSE_SOLVER1)
                                                printComplexToConsole(CPLX(*temp0), " m1=", "\n");
                                        #endif // VERBOSE_SOLVER1

        subComplex(CPLX(*Y2), CPLX(*Y0), CPLX(*temp3), ctxtSolver2);
                                        #if defined(VERBOSE_SOLVER1)
                                                printComplexToConsole(CPLX(*temp3), " Y2-Y0=", "\n");
                                        #endif // VERBOSE_SOLVER1

        mulComplexComplex(CPLX(*temp0), CPLX(*temp3), CPLX(*temp3), ctxtSolver2);
                                        #if defined(VERBOSE_SOLVER1)
                                                printComplexToConsole(CPLX(*temp3), " term1 lower m1*(Y2-Y1)=", "\n");
                                        #endif // VERBOSE_SOLVER1

        subComplex(CPLX(*Y0), CPLX(*Y1), CPLX(*temp1), ctxtSolver2);
                                        #if defined(VERBOSE_SOLVER1)
                                                printComplexToConsole(CPLX(*temp1), " dY2=", "\n");
                                        #endif // VERBOSE_SOLVER1
        subComplex(CPLX(*X0), CPLX(*X1), CPLX(*temp2), ctxtSolver2);
        divFunctionComplex(CPLX(*temp1), CPLX(*temp2), CPLX(*temp1));

                                        #if defined(VERBOSE_SOLVER1)
                                                printComplexToConsole(CPLX(*temp1), " m2=", "\n");
                                        #endif // VERBOSE_SOLVER1
        subComplex(CPLX(*temp0), CPLX(*temp1), CPLX(*temp1), ctxtSolver2);
                                        #if defined(VERBOSE_SOLVER1)
                                                printComplexToConsole(CPLX(*temp1), " m1-m2 diff=", "\n");
                                                printComplexToConsole(CPLX(*Y2), " Y2=", "\n");
                                        #endif // VERBOSE_SOLVER1
        mulComplexComplex(CPLX(*temp1), CPLX(*Y2), CPLX(*temp1), ctxtSolver2);
                                        #if defined(VERBOSE_SOLVER1)
                                                printComplexToConsole(CPLX(*temp1), " term2 lower=", "\n");
                                        #endif // VERBOSE_SOLVER1
        subComplex(CPLX(*temp3), CPLX(*temp1), CPLX(*temp1), ctxtSolver2);

                                        #if defined(VERBOSE_SOLVER1)
                                                printComplexToConsole(CPLX(*temp1), " lower diff=", "\n");
                                        #endif // VERBOSE_SOLVER1
        subComplex(CPLX(*Y2), CPLX(*Y0), CPLX(*X2N), ctxtSolver2);
        //get the 1/slope
        divFunctionComplex(CPLX(*X2N), CPLX(*temp1), CPLX(*X2N));
                                        #if defined(VERBOSE_SOLVER1)
                                                printComplexToConsole(CPLX(*temp0), " 1/slope=", "\n");
                                        #endif // VERBOSE_SOLVER1
        mulComplexComplex(CPLX(*X2N), CPLX(*Y1), CPLX(*X2N), ctxtSolver2); // increment to x is: y1 . DX/DY
        // if converges slow without oscillating then accelerate.
        if(convergent > 10) {
          convertDoubleToReal(1.0 + convergent * 0.1, f, ctxtSolver2); // factor ()
          mulComplexComplex(CPLX(*X2N), f, const_0, CPLX(*X2N), ctxtSolver2); // increment to x is: y1 . DX/DY
        }

                                        #if defined(VERBOSE_SOLVER1)
                                                printRealToConsole(f, "    Factor=        ", "\n");
                                                printComplexToConsole(CPLX(*X0), "    New X =        ", " - (");
                                                printComplexToConsole(CPLX(*temp1), "", ")\n");
                                        #endif // VERBOSE_SOLVER1
        subComplex(CPLX(*X1), CPLX(*X2N), CPLX(*X2N), ctxtSolver2); // subtract as per Newton, x1 - f/f' store temporarily to new x2n
      }

      //#############################################


                                        #if defined(VERBOSE_SOLVER1)
                                              printComplexToConsole(&dX->Real,   &dX->Imag, "               DX=", "");
                                              printComplexToConsole(CPLX(*dY), "DY=", "\n");
                                              printComplexToConsole(&X0->Real,   &X0->Imag, "               X0=", "");
                                              printComplexToConsole(CPLX(*Y0), "Y0=", "\n");
                                              printComplexToConsole(CPLX(*X2N), "   -------> newX2: ", "\n");
                                              printComplexToConsole(&X1->Real,   &X1->Imag, "               X1=", "");
                                              printComplexToConsole(CPLX(*Y1), "Y1=", "\n");
                                              printComplexToConsole(&X2->Real,   &X2->Imag, "               X2=", "");
                                              printComplexToConsole(CPLX(*Y2), "Y2=", "\n");
                                        #endif // VERBOSE_SOLVER1

      copyComplex(Y1, Y0); //old y1 copied to y0
      copyComplex(X1, X0); //old x1 copied to x0
      copyComplex(Y2, Y1); //old y2 copied to y1
      copyComplex(X2, X1); //old x2 copied to x1
      copyComplex(X2N, X2); //new x2

      // complexMagnitude(CPLX(*dX), &temp0->Real,  ctxtSolver2);
      // checkNaN   |=  realIsNaN(&temp0->Real);

                                        #if defined(VERBOSE_SOLVER00) || defined(VERBOSE_SOLVER0) || defined(VERBOSE_SOLVER1)
                                              if(Y2IsZero) {
                                                printf("--B1 Y2IsZero\n");
                                              }
                                              if(checkNaN) {
                                                printf("--B2 CheckNaN\n");
                                              }
                                              if(checkNaN || iterationCounter==NUMBERITERATIONS-1 || Y2IsZero) {
                                                printf("--B3 Endflags: |DXr|=0:%u |DXr|<TOL:%u |DYr|<TOL:%u |DYr|=0:%u |DXr|=NaN:%u |DYr|=NaN:%u \n",
                                                (uint16_t) realIsZero(&temp1->Real),
                                                (uint16_t)(realCompareAbsLessThan(&temp1->Real, tol)),
                                                (uint16_t) realIsZero(&temp0->Real),
                                                (uint16_t)(realCompareAbsLessThan(&temp2->Real, tol)),
                                                (uint16_t) realIsNaN (&temp1->Real),
                                                (uint16_t) realIsNaN (&temp0->Real)
                                                  );
                                              }
                                        #endif // VERBOSE_SOLVER00 || VERBOSE_SOLVER0 || VERBOSE_SOLVER1
      iterationCounter++;
      oscillationIterationCounter++;

                                        #if defined(VERBOSE_SOLVER2)
                                              if(!checkNaN && !(iterationCounter==NUMBERITERATIONS) && !Y2IsZero) {
                                                printf("END     iterationCounter=%d |DX|<TOL:%d ", iterationCounter, realCompareAbsLessThan(&dX->Real, tol));
                                                printComplexToConsole(CPLX(*dX), "", "\n");
                                                printf("END     iterationCounter=%d |DY|<TOL:%d ", iterationCounter, realCompareAbsLessThan(&dY->Real, tol));
                                                printComplexToConsole(CPLX(*dY), "", "\n");
                                                printComplexToConsole(CPLX(*temp1), "END     DY=", "\n");
                                              }
                                        #endif // VERBOSE_SOLVER2

                                        #if defined(VERBOSE_SOLVER1)
                                              printComplexToConsole(CPLX(*dX), ">>> DX=", "");
                                              printComplexToConsole(CPLX(*dY), " DY=", "");
                                              printComplexToConsole(CPLX(*temp0), " 1/SLOPE=", "\n");
                                        #endif // VERBOSE_SOLVER1


      if(checkHalfSec()) {
        if(progressHalfSecUpdate_Integer(timed, "Iter: ", iterationCounter, halfSec_clearZ, halfSec_clearT, halfSec_disp)) { //timed
          showProgressReal(CPLX(*X1), !realIsZero(&X1->Imag));
        }
      }

      if(exitKeyWaiting()) {
        showString("key Waiting ...", &standardFont, 20, 40, vmNormal, false, false);
        progressHalfSecUpdate_Integer(force+1, "Interrupted Iter:", iterationCounter, halfSec_clearZ, halfSec_clearT, halfSec_disp);
        lastErrorCode = ERROR_SOLVER_ABORT;
        if(programRunStop == PGM_RUNNING) { programRunStop = PGM_WAITING; }   // halt the outer program too, like every other abort point
        calcMode = CM_NORMAL;
        screenUpdatingMode = SCRUPD_AUTO;
        screenUpdatingMode |= SCRUPD_SKIP_STATUSBAR_ONE_TIME;
        break;
      }
                                        #if defined(VERBOSE_SOLVER0)
                                              printf("iterationCounter = %i ", iterationCounter);
                                              printComplexToConsole(CPLX(*X1), "X = ", " ");
                                              printComplexToConsole(CPLX(*Y1), "Y = ", "\n");
                                        #endif // VERBOSE_SOLVER0

      if(ENABLE_COMPLEXSOLVER_FILE_OUTPUT == 1) {
        convertComplexToResultRegister(CPLX(*X1), REGISTER_X);
        convertComplexToResultRegister(CPLX(*Y1), REGISTER_Y);
        fnP_All_Regs(PRN_XYr);
      }



    }  //Iteration end

    refreshScreen(200);

    checkNaN =    checkNaN
      || realIsNaN(&X1->Real) || realIsNaN(&X1->Imag)
        || realIsNaN(&X2->Real) || realIsNaN(&X2->Imag);



    bool_t conjugates = false;
    // Test if zeroed complex parts is better
    copyComplex(&cpxSlvBestX, temp0);
    if(checkRealZeroTol(&temp0->Real, tolClose)) {
      realSetZero(&temp0->Real);
      execute_rpn_function_reals(temp0, temp1, magnitudeY);
    }
    copyComplex(&cpxSlvBestX, temp0);
    if(checkRealZeroTol(&temp0->Imag, tolClose)) {
      realSetZero(&temp0->Imag);
      execute_rpn_function_reals(temp0, temp1, magnitudeY);
    }
    else {   // consider conjugates if X not close to Real
      realChangeSign(&temp0->Imag);
      execute_rpn_function_reals(temp0, temp1, magnitudeY);
      conjugates = checkRealZeroTol(magnitudeY, tolClose);
    }


    bool_t   FLAG_FRACTN = getSystemFlag(FLAG_FRACT);
    clearSystemFlag(FLAG_FRACT);


    fnSetSignificantDigits(signDig);
    // reset stack and lift to reasonable height
    fnUndo(0);
    liftStack();
    setSystemFlag(FLAG_ASLIFT);
    liftStack();

    if(!Y2IsZero) {
      temporaryInformation = TI_SOLVER_FAILED;
      displayCalcErrorMessage(ERROR_NO_ROOT_FOUND, ERR_REGISTER_LINE);
      convertDoubleToReal34Register(SOLVER_RESULT_OTHER_FAILURE, REGISTER_T);
    }
    else {
      temporaryInformation = TI_SOLVER_VARIABLE_RESULT;
      lastErrorCode = ERROR_NONE;
      convertDoubleToReal34Register(conjugates ? (double)SOLVER_RESULT_CONJUGATES : (double)SOLVER_RESULT_NORMAL, REGISTER_T);
    }

    setRoundingMode(cpxSlvUserRoundingMode);                           // the results are rounded once, by RM
    convertRealToResultRegister(&cpxSlvBestMagnitudeY, REGISTER_Z, amNone);
    convertComplexToResultRegister(CPLX(*X1), REGISTER_Y);
    convertComplexRegisterToRealIfZeroImag(REGISTER_Y);
    // convertDoubleToReal34Register(iterationCounter, REGISTER_Y);
    convertComplexToResultRegister(CPLX(cpxSlvBestX), REGISTER_X);
    convertComplexRegisterToRealIfZeroImag(REGISTER_X);
    copySourceRegisterToDestRegister(REGISTER_X, graphVariabl1);

    printSolverResult(iterationCounter);

    if(FLAG_FRACTN) {
      setSystemFlag(FLAG_FRACT);
    }

    calcMode = CM_NORMAL;
    SAVED_SIGMA_lastAddRem = SIGMA_NONE;   //prevent undo of last stats add action. REMOVE when STATS are not used anymore

freeWork:
    REAL_T_FREE(f,             75);
    REAL_T_FREE(tol,           75);
    REAL_T_FREE(tolClose,      75);
    REAL_T_FREE(oldMagnitudeY, 75);
    REAL_T_FREE(magnitudeY,    75);

    CPLX_T_FREE(X0);
    CPLX_T_FREE(X1);
    CPLX_T_FREE(X2);
    CPLX_T_FREE(X2N);
    CPLX_T_FREE(dX);
    CPLX_T_FREE(dXold);

    CPLX_T_FREE(Y0);
    CPLX_T_FREE(Y1);
    CPLX_T_FREE(Y2);
    CPLX_T_FREE(Y2N);
    CPLX_T_FREE(dY);
    CPLX_T_FREE(dYold);

    CPLX_T_FREE(temp0);
    CPLX_T_FREE(temp1);
    CPLX_T_FREE(temp2);
    CPLX_T_FREE(temp3);
    return;
  }


  void fnComplexSolver(void) {
    printStatus(1, errorMessageOf(COMPLEX_SOLVER), force);
    saveForUndo();
                                        #if defined(VERBOSE_SOLVER00) || defined(VERBOSE_SOLVER0)
                                          real_t loXd, hiXd;
                                          printRegisterToConsole(REGISTER_Y, ">>> lowerXStartValue=", "");
                                          printRegisterToConsole(REGISTER_X, " higherXStartValue=", "\n");
                                          if(getRegisterAsReal(REGISTER_X, &hiXd) && getRegisterAsReal(REGISTER_Y, &loXd) && realCompareGreaterThan(&hiXd, &loXd)) { //pre-condition the plotter
                                            realCopy(&loXd, x_min);
                                            realCopy(&hiXd, x_max);
                                          }
                                          printf("xmin:%f, xmax:%f\n", realToDoubleVal(x_min), realToDoubleVal(x_max));
                                        #endif // VERBOSE_SOLVER00 || VERBOSE_SOLVER0
    // initialize_function();
    cpxSlvUserRoundingMode = roundingMode;
    setRoundingMode(RM_ENGINE);                                        // the iteration takes no mode from RM
    complexSolver();
    setRoundingMode(cpxSlvUserRoundingMode);
  }

#endif //OPTION_GRAPHICS


//-----------------------------------------------------//-----------------------------------------------------
//Helper for plot range check of pair: swap reversed limits, detect too narrow range and increase to 10% if needed, allow up to 100x minimum range
void graphRangeGuard(real_t *lo, real_t *hi) {
  real_t span, mag, tmpR;
  if(realCompareGreaterThan(lo, hi)) { //swap if entered in incorrect sequence
    realCopy(hi, &tmpR);                                             // tmpR = hi
    realCopy(lo, hi);                                                // hi = lo
    realCopy(&tmpR, lo);                                             // lo = old hi
  }
  realSubtract(hi, lo, &span, &ctxtReal39);                          // span = hi - lo, >= 0 after the swap above
  realCopyAbs(lo, &mag);                                             // mag = |lo|
  realCopyAbs(hi, &tmpR);                                            // tmpR = |hi|
  if(realCompareGreaterThan(&tmpR, &mag)) {
    realCopy(&tmpR, &mag);                                           // mag = max(|lo|, |hi|)
  }
  realSetOne(&tmpR);
  tmpR.exponent = 3 - GRAPH_WORKING_DIGITS;                          // tmpR = 1E-11, 100x the ULP of the graph working precision
  realMultiply(&mag, &tmpR, &tmpR, &ctxtReal39);                     // tmpR = mag * 1E-11, the narrowest plottable span
  if(realCompareLessEqual(&span, &tmpR)) { //span too small to plot at working precision
    if(realIsZero(&mag)) {                 //typically 0 - 0: change to -1 to 1
      int32ToReal(-1, lo);                                           // lo = -1
      int32ToReal(1, hi);                                            // hi = 1
    }
    else {                                 //widen by 10% of the magnitude
      convertDoubleToReal(0.1, &tmpR, &ctxtReal39);                  // tmpR = 0.1
      realMultiply(&mag, &tmpR, &tmpR, &ctxtReal39);                 // tmpR = 0.1 * mag
      realSubtract(lo, &tmpR, lo, &ctxtReal39);                      // lo = lo - 0.1 * mag
      realAdd(hi, &tmpR, hi, &ctxtReal39);                           // hi = hi + 0.1 * mag
    }
  }
}


//-----------------------------------------------------//-----------------------------------------------------
void fnEqSolvGraph (uint16_t func) {
  #if defined(OPTION_GRAPHICS)
      // Ahead of the first switch below, which reallocates the reserved UX/LX registers and reassigns graphVariabl1. fnMvarPlot refuses a programmed PLTf earlier,
      // this covers the menu entries.
      if((func == EQ_PLOT || func == EQ_PLOT_LU) && engineNestingRefused(true)) {
        return;
      }

      // No equation defined: error out before any stack or reserved-variable writes;
      // running these items without a formula crashed in parseEquation (NULL allFormulae).
      // A program plot has no formula by design (execute_rpn_function runs its program), so exempt it from the guard when the func is a plot func, RPN_GRAPHER is set,
      // and currentSolverVariable is valid. The valid variable keeps this clear of the parseEquation fallback below, so its NULL deref stays unreachable,
      // and the func check keeps the guard live for the solve items.
      const bool_t rpnProgramPlot = (func == EQ_PLOT || func == EQ_PLOT_LU)
                                    && (currentSolverStatus & SOLVER_STATUS_RPN_GRAPHER)
                                    && currentSolverVariable >= FIRST_NAMED_VARIABLE
                                    && currentSolverVariable <= LAST_NAMED_VARIABLE;
      if(!rpnProgramPlot && (currentFormula >= numberOfFormulae || allFormulae[currentFormula].pointerToFormulaData == C47_NULL)) {
        calcMode = CM_NORMAL;
        displayCalcErrorMessage(ERROR_NO_EQUATION_DEFINED, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function fnEqSolvGraph:", "no equation defined", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return;
      }
      hourGlassIconEnabled = true;
      showHideHourGlass();
      #if defined(DMCP_BUILD)
        lcd_refresh();
      #else // !DMCP_BUILD
        refreshLcd(NULL);
      #endif // DMCP_BUILD

      real_t x, y;


      switch(func) {
        case EQ_CPXSOLVE_LU:
        case EQ_REALSOLVE_LU: {
          if(getRegisterAsReal(RESERVED_VARIABLE_LEST, &y) && getRegisterAsReal(RESERVED_VARIABLE_UEST, &x)) {
            liftStack();
            setSystemFlag(FLAG_ASLIFT);
            reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
            liftStack();
            reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
            realToReal34(&x, REGISTER_REAL34_DATA(REGISTER_X));
            realToReal34(&y, REGISTER_REAL34_DATA(REGISTER_Y));
            solverEstimatesUsed = true;
          }
          break;
        }
        case EQ_CPXSOLVE:
        case EQ_REALSOLVE: {
          if(getRegisterAsReal(REGISTER_X, &x) && getRegisterAsReal(REGISTER_Y, &y)) {
            reallocateRegister(RESERVED_VARIABLE_UEST, dtReal34, 0, amNone);
            reallocateRegister(RESERVED_VARIABLE_LEST, dtReal34, 0, amNone);
            realToReal34(&x, REGISTER_REAL34_DATA(RESERVED_VARIABLE_UEST));
            realToReal34(&y, REGISTER_REAL34_DATA(RESERVED_VARIABLE_LEST));
            solverEstimatesUsed = false;
          }
          break;
        }
        case EQ_PLOT_LU: {           //uses limits
          if(getRegisterAsReal(RESERVED_VARIABLE_LX, &y) && getRegisterAsReal(RESERVED_VARIABLE_UX, &x)) {
            liftStack();
            setSystemFlag(FLAG_ASLIFT);
            reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
            liftStack();
            reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
            realToReal34(&x, REGISTER_REAL34_DATA(REGISTER_X));
            realToReal34(&y, REGISTER_REAL34_DATA(REGISTER_Y));
          }
          break;
        }
        case EQ_PLOT: {              //uses X, Y
          if(getRegisterAsReal(REGISTER_X, &x) && getRegisterAsReal(REGISTER_Y, &y)) {
            if(realIsSpecial(&x) || realIsSpecial(&y) || realCompareEqual(&x, &y)) { //screen raw incoming range: keep the old UX/LX and error
              calcMode = CM_NORMAL;    //leave the graph screen so the error line renders, same as graph_stat/fnPlotStat
              displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
              #if (EXTRA_INFO_ON_CALC_ERROR == 1)
                moreInfoOnError("In function fnEqSolvGraph:", "plot range limits must be finite and distinct", NULL, NULL);
              #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
              return;
            }
            reallocateRegister(RESERVED_VARIABLE_UX, dtReal34, 0, amNone);
            reallocateRegister(RESERVED_VARIABLE_LX, dtReal34, 0, amNone);
            realToReal34(&x, REGISTER_REAL34_DATA(RESERVED_VARIABLE_UX));
            realToReal34(&y, REGISTER_REAL34_DATA(RESERVED_VARIABLE_LX));
            }
          break;
        }
        default:
          return;
          break;
      }

      if(!(currentSolverVariable >= FIRST_NAMED_VARIABLE && currentSolverVariable <= LAST_NAMED_VARIABLE)) {
        // No plot variable assigned (e.g. a programmed Draw after X.SWAP loaded a fresh formula): auto-assign like the interactive MVAR menu does
        // (softmenus.c) when the formula holds exactly one variable; with several variables the existing error below still applies.
        parseEquation(currentFormula, EQUATION_PARSER_MVAR, aimBuffer, tmpString);
        if(tmpString[0] != 0 && (getNthString((uint8_t *)tmpString, 1))[0] == 0) {
          currentSolverVariable = findOrAllocateNamedVariable(tmpString);
        }
      }

      graphVariabl1 = currentSolverVariable;

      if(graphVariabl1 >= FIRST_NAMED_VARIABLE && graphVariabl1 <= LAST_NAMED_VARIABLE) {
        #if defined(VERBOSE_SOLVER00) || defined(VERBOSE_SOLVER0)
          printf("graphVariabl1 accepted: %i\n", graphVariabl1);
        #endif // VERBOSE_SOLVER00 || VERBOSE_SOLVER0
      }
      else {
        calcMode = CM_NORMAL;
        displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          sprintf(errorMessage, "unexpected parameter %u", graphVariabl1);
          moreInfoOnError("In function fnEqSolvGraph:", errorMessage, NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return;
      }


      //initialize x
      currentSolverStatus &= ~SOLVER_STATUS_READY_TO_EXECUTE;

      switch(func) {
        case EQ_REALSOLVE_LU:
        case EQ_REALSOLVE: {
          if((currentSolverVariable >= FIRST_NAMED_VARIABLE) ) {//&& currentSolverStatus & SOLVER_STATUS_READY_TO_EXECUTE) {
            fnSolve(currentSolverVariable);
          }
          break;
        }

        case EQ_CPXSOLVE_LU:
        case EQ_CPXSOLVE: {
          fnComplexSolver();
          break;
        }

        case EQ_PLOT_LU:
        case EQ_PLOT: {

    //      PLOT_ZMY = 0; removed default zeroing of the zoom factor in eqn as it is irritating with the new y range control
          real_t loX, hiX;
          bool_t rangeOK = getRegisterAsReal(REGISTER_X, &hiX) && getRegisterAsReal(REGISTER_Y, &loX);
          #if defined(VERBOSE_SOLVER00) || defined(VERBOSE_SOLVER0)
            printf(">>> lowerXStartValue=%f  higherXStartValue=%f\n", realToDoubleVal(&loX), realToDoubleVal(&hiX));
          #endif // VERBOSE_SOLVER00 || VERBOSE_SOLVER0

          // fnClDrawMx's delete can shift the plot variable's register; cache the name, re-resolve after.
          char plotVarName[16];
          plotVarName[0] = 0;
          if(currentSolverVariable >= FIRST_NAMED_VARIABLE && currentSolverVariable < FIRST_NAMED_VARIABLE + numberOfNamedVariables) {
            uint8_t len = allNamedVariables[currentSolverVariable - FIRST_NAMED_VARIABLE].variableName[0];
            xcopy(plotVarName, allNamedVariables[currentSolverVariable - FIRST_NAMED_VARIABLE].variableName + 1, len);
            plotVarName[len] = 0;
          }

          fnClDrawMx(5);
          strcpy(plotStatMx, "DrwMX");

          if(plotVarName[0] != 0) {
            calcRegister_t reResolved = findNamedVariable(plotVarName);
            if(reResolved != INVALID_VARIABLE) {
              currentSolverVariable = reResolved;
              graphVariabl1         = reResolved;
            }
          }

          if(rangeOK && realCompareGreaterThan(&hiX, &loX)) { //pre-condition the plotter                 // if(X > Y) { x_min = Y; x_max = X; }
            realCopy(&loX, x_min);                                         // x_min = Y, the lower stack value
            realCopy(&hiX, x_max);                                         // x_max = X, the higher stack value
          }
          graphRangeGuard(x_min, x_max); //swap if entered in incorrect sequence; widen spans too small to plot at working precision
          #if defined(VERBOSE_SOLVER00) || defined(VERBOSE_SOLVER0)
            printf("xmin:%f, xmax:%f\n", realToDoubleVal(x_min), realToDoubleVal(x_max));
          #endif // VERBOSE_SOLVER00 || VERBOSE_SOLVER0

          initialize_function();
          ++engineNestingDepth;                                // one engine level for the whole sweep
          ++plotEngineActive;
          eqnDrawLhsOnly = true;                               // Draw on an equation plots its left hand side; an expression has no = and plots unchanged
          graph_eqn(noInitDrwMx);
          eqnDrawLhsOnly = false;
          --plotEngineActive;
          --engineNestingDepth;

          if(!getSystemFlag(FLAG_PCROS) && !getSystemFlag(FLAG_PBOX) && !getSystemFlag(FLAG_PPLUS)) {
            setSystemFlag(FLAG_PLINE);
          }

          reDraw = true;
          screenUpdatingMode = SCRUPD_AUTO;
          screenUpdatingMode |= SCRUPD_SKIP_STATUSBAR_ONE_TIME;
          refreshScreen(239);
          if(programRunStop == PGM_RUNNING || programRunStop == PGM_PAUSED) {
            // The refresh above dropped a running program back to CM_NORMAL, so a following programmed SNAP would repaint and capture the register
            // display instead of the plot. This copies PLTf (via fnPlotSQ) so the SNAP re-renders it; the SNAP's own refresh performs the CM_NORMAL drop for the program steps that follow.
            calcMode = CM_GRAPH;
            reDraw = true;
          }
          break;
        }
        default: ;
      }
  #endif // OPTION_GRAPHICS
}
