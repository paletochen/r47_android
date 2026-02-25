// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file integrate.c
 ***********************************************/

#include "c47.h"

void fnPgmInt(uint16_t label) {
  if(FIRST_LABEL <= label && label <= LAST_LABEL) {
    currentSolverProgram = label - FIRST_LABEL;
    currentSolverStatus &= ~SOLVER_STATUS_USES_FORMULA;
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
        moreInfoOnError("In function fnPgmInt:", errorMessage, NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
    else {
      currentSolverProgram = label - FIRST_LABEL;
      currentSolverStatus &= ~SOLVER_STATUS_USES_FORMULA;
    }
  }
  else {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "unexpected parameter %u", label);
      moreInfoOnError("In function fnPgmInt:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}

/*DEBUGINT
static void printCurrentSolverStatus(int nn, uint16_t currentSolverStatus) {
printf("%i: currentSolverStatus=%u: ",nn,currentSolverStatus);
if(currentSolverStatus > SOLVER_STATUS_TVM_APPLICATION)   { currentSolverStatus -= SOLVER_STATUS_TVM_APPLICATION;   printf("TVM ");}
if(currentSolverStatus > SOLVER_STATUS_MVAR_BEING_OPENED) { currentSolverStatus -= SOLVER_STATUS_MVAR_BEING_OPENED; printf("MVARopen ");}
if(currentSolverStatus > SOLVER_STATUS_USES_FORMULA)      { currentSolverStatus -= SOLVER_STATUS_USES_FORMULA;      printf("FORMULA ");}
if(currentSolverStatus > SOLVER_STATUS_SINGLE_VARIABLE)   { currentSolverStatus -= SOLVER_STATUS_SINGLE_VARIABLE;   printf("1VAR ");}
if(currentSolverStatus > SOLVER_STATUS_SINGLE_VARIABLE)   { currentSolverStatus -= SOLVER_STATUS_SINGLE_VARIABLE;   printf("1VAR ");}
switch(currentSolverStatus) {
    case SOLVER_STATUS_READY_TO_EXECUTE        : printf("ReadyExec ");break;   //  0x0001 //0001
    case SOLVER_STATUS_INTERACTIVE             : printf("Interactive ");break;   //  0x0002 //0010
//    case SOLVER_STATUS_EQUATION_MODE           : printf("EQN MODE");break;   //  0x000c //1100
    case SOLVER_STATUS_EQUATION_SOLVER         : printf("EQN SLV ");break;   //  0x0000
    case SOLVER_STATUS_EQUATION_INTEGRATE      : printf("EQN INT");break;   //  0x0004 //0100
    case SOLVER_STATUS_EQUATION_1ST_DERIVATIVE : printf("1ST ");break;   //  0x0008 //1000
    case SOLVER_STATUS_EQUATION_2ND_DERIVATIVE : printf("2ND ");break;   //  0x000C //1100
    default:break;
  }
  printf("\n");
}
*/

void _fnIntegrate(uint16_t labelOrVariable, bool_t XY) {

  //printCurrentSolverStatus(3, currentSolverStatus);
  //char yyy[100], yy1[100];
  //int16_t yy0;
  //  __displaySolver(labelOrVariable, yyy , &yy0);
  //  stringToASCII(yyy,yy1);
  //printf("labelOrVariable=%u %s\n",labelOrVariable, yy1);
  //  __displaySolver(currentSolverVariable, yyy , &yy0);
  //  stringToASCII(yyy,yy1);
  //printf("currentSolverVariable=%u %s\n",currentSolverVariable, yy1);
  //  xcopy(yyy, labelList[currentSolverProgram].labelPointer + 1, *(labelList[currentSolverProgram].labelPointer));
  //  yyy[*(labelList[currentSolverProgram].labelPointer)] = 0;
  //  stringToASCII(yyy,yy1);
  //printf("currentSolverProgram %u = %s\n", currentSolverProgram, yy1);

  if((FIRST_LABEL <= labelOrVariable && labelOrVariable <= LAST_LABEL) || (REGISTER_X <= labelOrVariable && labelOrVariable <= REGISTER_T)) {
    // Interactive mode
    fnPgmInt(labelOrVariable);
    if(lastErrorCode == ERROR_NONE) {
      currentSolverStatus = SOLVER_STATUS_INTERACTIVE | SOLVER_STATUS_EQUATION_INTEGRATE;
    }
  }
  else if(!XY && (labelOrVariable == RESERVED_VARIABLE_ACC || labelOrVariable == RESERVED_VARIABLE_ULIM || labelOrVariable == RESERVED_VARIABLE_LLIM)) {
    fnToReal(NOPARAM);
    if(lastErrorCode == ERROR_NONE) {
      real34Copy(REGISTER_REAL34_DATA(REGISTER_X), REGISTER_REAL34_DATA(labelOrVariable));
      switch(labelOrVariable) {
        case RESERVED_VARIABLE_ACC: {
          temporaryInformation = TI_ACC;
          break;
        }
        case RESERVED_VARIABLE_ULIM: {
          temporaryInformation = TI_ULIM;
          break;
        }
        case RESERVED_VARIABLE_LLIM: {
          temporaryInformation = TI_LLIM;
          break;
        }
      }
    }
  }
  else if(!(currentSolverStatus & SOLVER_STATUS_USES_FORMULA) && (FIRST_NAMED_VARIABLE <= labelOrVariable && labelOrVariable <= LAST_NAMED_VARIABLE) && currentSolverProgram >= numberOfLabels) {
    displayCalcErrorMessage(ERROR_NO_PROGRAM_SPECIFIED, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "label %u not found", labelOrVariable);
      moreInfoOnError("In function _fnIntegrate:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
  else if(FIRST_NAMED_VARIABLE <= labelOrVariable && labelOrVariable <= LAST_NAMED_VARIABLE) {
    real_t acc, ulim, llim, res;
    bool_t smallerEpsilon = false;
    real34ToReal(REGISTER_REAL34_DATA(RESERVED_VARIABLE_ACC),  &acc);
    real34ToReal(REGISTER_REAL34_DATA(RESERVED_VARIABLE_ULIM), &ulim);
    real34ToReal(REGISTER_REAL34_DATA(RESERVED_VARIABLE_LLIM), &llim);
    smallerEpsilon = realCompareAbsLessThan(&acc, const_1e_16) && ((realCompareAbsLessThan(&ulim, const_1e_32) || realCompareAbsLessThan(&llim, const_1e_32)));
    #if USE_MICHALSKI_MOSIG_TANH_SINH == 1
      smallerEpsilon = smallerEpsilon && (realIsSpecial(&ulim) || realIsSpecial(&llim)); // smallerEpsilon not needed
    #endif // USE_MICHALSKI_MOSIG_TANH_SINH == 1
    if(realIsZero(&acc)) { // it may freeze if ACC=0
      realCopy(const_1e_32, &acc);   //used to be const_1e_6143
    }
    if(real34CompareEqual(REGISTER_REAL34_DATA(RESERVED_VARIABLE_ULIM),REGISTER_REAL34_DATA(RESERVED_VARIABLE_LLIM) )) {
      int32ToReal(0,&res);
      int32ToReal(0,&acc);
      goto done;
    }

calcRegister_t regist = labelOrVariable;  //at this point, it is a register variable
saveForUndo();

#define SPEEDUPEXPERIMENT
//#undef SPEEDUPEXPERIMENT

#ifdef SPEEDUPEXPERIMENT
    real_t digits;
    uint8_t significantDigitsMem = significantDigits;
    int32_t digitsN = 0;
    WP34S_Ln(&acc, &digits, &ctxtReal39);
    realDivide(&digits, const_ln10, &digits, &ctxtReal39);
    digitsN = max(min(-realToInt32C47(&digits),34-3),1);
    #ifdef PC_BUILD
      printRealToConsole(&digits, "digits: ","\n");
      printf("----->>>> digitsN=%i, smallerEpsilon=%u\n",digitsN,smallerEpsilon);
      printRealToConsole(&acc, "acc: ","\n");
      printRealToConsole(&llim, "llim: ","\n");
      printRealToConsole(&ulim, "ulim: ","\n");
    #endif

    if(digitsN == 6) {
      #ifdef PC_BUILD
        printf("Special accuracy test case: N=6 Reducing DEC to single precision and SDIGS digits to %i etc.\n",digitsN+3);
      #endif
      significantDigits = digitsN+3;
      ctxtReal4.digits  = 7;
      ctxtReal34.digits = digitsN+3;
      ctxtReal39.digits = digitsN+5;
      ctxtReal51.digits = digitsN+7;
      ctxtReal75.digits = digitsN+13;
      integrate(regist, &llim, &ulim, &acc, &res, &ctxtReal4);
        //WP34S_Ln(&acc, &digits, &ctxtReal39);
        //realDivide(&digits, const_ln10, &digits, &ctxtReal39);
        //digitsN = max(min(-realToInt32C47(&digits),34-3),1);
        //#ifdef PC_BUILD
        //  printf("nnn=%i\n",digitsN);
        //#endif
        //real_t tt;
        //int32ToReal(-digitsN, &tt);
        //realRescale(&res, &res, &tt, &ctxtReal4);
      significantDigits = significantDigitsMem;
      ctxtReal4.digits  = 6;
      ctxtReal34.digits = 34;
      ctxtReal39.digits = 39;
      ctxtReal51.digits = 51;
      ctxtReal75.digits = 75;
    }
    else
    if(digitsN <= 10) {
      #ifdef PC_BUILD
        printf("Special accuracy test case: N<=10 Reducing SDIGS digits to %i etc.\n",digitsN+3);
      #endif
      significantDigits = digitsN+3;
      ctxtReal4.digits  = digitsN+3;
      ctxtReal34.digits = digitsN+3;
      ctxtReal39.digits = digitsN+5;
      ctxtReal51.digits = digitsN+7;
      ctxtReal75.digits = digitsN+13;
      integrate(regist, &llim, &ulim, &acc, &res, &ctxtReal39);
        //WP34S_Ln(&acc, &digits, &ctxtReal39);
        //realDivide(&digits, const_ln10, &digits, &ctxtReal39);
        //digitsN = max(min(-realToInt32C47(&digits),34-3),1);
        //#ifdef PC_BUILD
        //  printf("nnn=%i\n",digitsN);
        //#endif
        //real_t tt;
        //int32ToReal(-digitsN, &tt);
        //realRescale(&res, &res, &tt, &ctxtReal39);  or ose ACC. But best is to use N decimals. This does not work right
      significantDigits = significantDigitsMem;
      ctxtReal4.digits  = 6;
      ctxtReal34.digits = 34;
      ctxtReal39.digits = 39;
      ctxtReal51.digits = 51;
      ctxtReal75.digits = 75;
    }
    else {
    #ifdef PC_BUILD
      printf("Temporary Debugging info. Can be deleted once done.\n");
      printRealToConsole(&llim,"llim:","\n");
      printRealToConsole(&ulim,"ulim:","\n");
      printRealToConsole(&acc,"acc:","\n");
    #endif //PC_BUILD
    integrate(regist, &llim, &ulim, &acc, &res, smallerEpsilon ? &ctxtReal75 : &ctxtReal39);
    #ifdef PC_BUILD
      printf("Temporary Debugging info. Can be deleted once done.\n");
      printRealToConsole(&res,"res:","\n");
    #endif //PC_BUILD
    }
#else //SPEEDUPEXPERIMENT
    integrate(regist, &llim, &ulim, &acc, &res, smallerEpsilon ? &ctxtReal75 : &ctxtReal39);
#endif //SPEEDUPEXPERIMENT

done:
    fnUndo(0);
    liftStack();
    liftStack();

    convertRealToReal34ResultRegister(&res, REGISTER_X);
    convertRealToReal34ResultRegister(&acc, REGISTER_Y);
    if(lastErrorCode != ERROR_SOLVER_ABORT) {
      temporaryInformation = TI_INTEGRAL;
    }
    adjustResult(REGISTER_X, false, false, REGISTER_X, -1, -1);
  }
  else {
    displayCalcErrorMessage(ERROR_OUT_OF_RANGE, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "unexpected parameter %u", labelOrVariable);
      moreInfoOnError("In function _fnIntegrate:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}


void fnIntegrate(uint16_t labelOrVariable) {
  //printf("fnIntegrate\n");
  _fnIntegrate(labelOrVariable, false);
}


void fnIntegrateYX(uint16_t labelOrVariable) {
  //printf("fnIntegrateYX\n");
  //printRegisterToConsole(REGISTER_X,"X:",", ");
  //printRegisterToConsole(REGISTER_Y,"Y:","\n");
  real_t x, y;
  if(getRegisterAsReal(REGISTER_X, &x) && getRegisterAsReal(REGISTER_Y, &y)) {
    realToReal34(&x, REGISTER_REAL34_DATA(RESERVED_VARIABLE_ULIM));
    realToReal34(&y, REGISTER_REAL34_DATA(RESERVED_VARIABLE_LLIM));
    fnDrop(NOPARAM);
    fnDrop(NOPARAM);
  }
  _fnIntegrate(labelOrVariable, true);
}



void fnIntVar(uint16_t unusedButMandatoryParameter) {
  #if !defined(TESTSUITE_BUILD)
    const char *var = (char *)getNthString(dynamicSoftmenu[softmenuStack[0].softmenuId].menuContent, dynamicMenuItem);
    const uint16_t regist = findOrAllocateNamedVariable(var);
    bool_t doubleVarPress = regist == currentSolverVariable;
    currentSolverVariable = regist;
    if(doubleVarPress && currentSolverStatus & SOLVER_STATUS_READY_TO_EXECUTE) {
      if((currentSolverStatus & SOLVER_STATUS_INTERACTIVE) && !(currentSolverStatus & SOLVER_STATUS_USES_FORMULA)) {
        showSoftmenu(-MNU_Sfdx);     //in case of RPN formula, which does not yet work with DRAW
      }
      else {
        showSoftmenu(-MNU_Sf_TOOL);  //in case of EQN
      }
    }
    else {
      reallyRunFunction(ITM_STO, regist);
      currentSolverStatus |= SOLVER_STATUS_READY_TO_EXECUTE;
      temporaryInformation = TI_SOLVER_VARIABLE;
    }
  #endif // !TESTSUITE_BUILD
}



static void _integratorIteration(void) {
  if(lastErrorCode == ERROR_SOLVER_ABORT) {
    return;
  }
                            if(ENABLE_INTEGRATOR_FILE_OUTPUT == 1) {
                              copySourceRegisterToDestRegister(REGISTER_X, TEMP_REGISTER_1);
                            }
  if(currentSolverStatus & SOLVER_STATUS_USES_FORMULA) {
    parseEquation(currentFormula, EQUATION_PARSER_XEQ, tmpString, tmpString + AIM_BUFFER_LENGTH);
  }
  else {
    dynamicMenuItem = -1;
    execProgram(currentSolverProgram + FIRST_LABEL);
  }
                            if(ENABLE_INTEGRATOR_FILE_OUTPUT == 1) {
                              copySourceRegisterToDestRegister(TEMP_REGISTER_1,REGISTER_Y);
                              fnP_All_Regs(PRN_XYr);
                            }
}



// The following routine is ported from WP34s.
// The description below is as is.


// Double Exponential Integration for the wp34s calculator
//
// v1.4x-412 (20180312) by M. César Rodríguez GGPL
//
// USAGE:
//  Inputs:
//    - integration limits in X (upper limit) and Y (lower limit)
//  Outputs:
//    - approximate value of integral in X
//    - estimated error in Y
//    - upper integration limit in Z
//    - lower integration limit in T
//    - updates L (with upper limit)
//    - in SSIZE8 mode, all ABCD will be corrupted
//
//  Remarks:
//    - accepts +Infinite and -Infinite as integration limits. If none
//      of the limits is infinite, the method applied is the tanh-sinh
//      one. If both are infinite then the sinh-sinh method is selected.
//      Otherwise the exp-sinh method is used
//    - the program tries to get all displayed digits correct, so
//      increasing the number of displayed digits will (hopefully) give
//      closer approximations (needing more time). But, in DBLON mode, the
//      program ignores the number of displayed digits and does its best
//      to get as much correct digits as possible
//    - during program execution the succesive aproximations are
//      displayed (as with the built-in integration program). Pressing
//      [<-] will interrupt the program & the last approximation
//      will be returned. The estimated error will be 0 in this case
//    - if the computed error estimation is not much smaller than the computed
//      result, it is assumed that all digits of the result are corrupted by
//      roundoff. In such cases, the reported result is 0 and the reported
//      error estimation equals the sum of the absolute values of the computed
//      result and error. This usually happens when the integral evaluates to 0
//    - if the user has set the D flag, many errors when evaluating the
//      integrand (say 0/0 or 1/0, overflows or ...) will not raise an error,
//      but return infinite or NaN instead. Such conditions will be detected
//      by this program that will continue the computation simply ignoring
//      the offending point
//    - keep in mind that, when both integration limits are infinite (or
//      one is infinite and the other 0) the integrand may be evaluated
//      at really bigs/small points (up to 1e±199 in DBLOFF mode and
//      1e±3088 in DBLON mode). Ensure that your integrand program
//      behaves as expected for such arguments (you may consider to set
//      flag D in some circumstances)
//    - the double exponential method relies on the function to be
//      integrated being analytic over the integration interval, except,
//      perhaps, at the interval ends. Thus, if it is known that the
//      integrand, _or any of its derivatives_, has a discontinuity at some
//      point inside the integration interval, it is advised to break the
//      interval at such point and compute two separate integrals, one
//      at each side of the problematic point. Better results will be
//      get this way. Thus, beware of abs, fp, ip and such non analytic
//      functions inside the integrand program. Other methods (Romberg)
//      may behave better in this respect, but, on the other side, the
//      double exponential method manages discontinuities (of the
//      integrand or its derivatives) at the interval edges better and
//      is usually way faster that the Romberg method
//    - as many other quadrature methods (Romberg included), the
//      double exponential algorithm does not manage well highly
//      oscillatory integrands. Here, highly oscillatory means that the
//      integrand changes sign many (or infinite) times along the
//      integration interval
//
//  Method used:
//    The algorith used here is adapted from that described in:
//      H. Takahasi and M. Mori. "Double Exponential Formulas for Numerical
//      Integration." Publications of RIMS, Kyoto University 9 (1974),
//      721-741.
//    Another useful reference may be:
//      David H. Bailey, Xiaoye S. Li and Karthik Jeyabalan, "A comparison
//      of three high-precision quadrature schemes," Experimental
//      Mathematics, vol. 14 (2005), no. 3, pg 317-329.
//
// 264 steps
// 16 local registers

static void DEI_xeq_user(calcRegister_t regist, const real_t *x, real_t *res, realContext_t *realContext) {
  if(lastErrorCode == ERROR_SOLVER_ABORT) { // Aborted?
    realZero(res);
  }
  // call user's function  -------------------------------
  else if(!realIsSpecial(x)) { // abscissa is good?
    //bool_t d = getSystemFlag(FLAG_SPCRES);
    //clearSystemFlag(FLAG_SPCRES);
    reallocateRegister(regist, dtReal34, 0, amNone);
    realToReal34(x, REGISTER_REAL34_DATA(regist));
    fnFillStack(NOPARAM);
    //printReal34ToConsole(REGISTER_REAL34_DATA(regist), "", " -> ");
    _integratorIteration();
    //printReal34ToConsole(REGISTER_REAL34_DATA(REGISTER_X), "", "\n"); fflush(stdout);
    real34ToReal(REGISTER_REAL34_DATA(REGISTER_X), res);
    if(realIsSpecial(res)) { // do not stop in error (if flag D was set)
      realZero(res);
    }
    //if(d) {
    //  setSystemFlag(FLAG_SPCRES); // set flag D for internal calculations
    //}
  }
  else { // DEI_bad_absc::
    realZero(res);
  }
}


void _showProgress(const real_t *ss, const real_t *bma2, const real_t *h, const real_t *a, const real_t *b, const real_t *fact, realContext_t *realContext) {
  #if !defined (TESTSUITE_BUILD)
    real_t res;
    clearRegisterLine(REGISTER_Z, true, true);
    clearRegisterLine(REGISTER_Y, true, true);
    clearRegisterLine(REGISTER_X, true, true);
    uint8_t savedDisplayFormatDigits = displayFormatDigits;
    displayFormatDigits = displayFormat == DF_ALL ? 0 : 33;
    real34_t rtmp34;
    real_t tmpr;
    realMultiply(ss, bma2, &res, realContext);
    realMultiply(&res, h, &res, realContext); // load the integral result,
    realMultiply(&res, fact, &res, realContext);
    realToReal34(&res,&rtmp34);
    real34ToDisplayString(&rtmp34, amNone, tmpString, &standardFont, 9999, 34, !LIMITEXP, FRONTSPACE, NOIRFRAC);
    showString(tmpString, &standardFont, 1, Y_POSITION_OF_REGISTER_X_LINE + 6, vmNormal, true, true);
    realSubtract(a,b,&tmpr,realContext);
    realToReal34(&tmpr,&rtmp34);
    real34ToDisplayString(&rtmp34, amNone, tmpString, &standardFont, 9999, 34, !LIMITEXP, FRONTSPACE, NOIRFRAC);
    showString(tmpString, &standardFont, 1, Y_POSITION_OF_REGISTER_Y_LINE + 6, vmNormal, true, true);
    displayFormatDigits = savedDisplayFormatDigits;
    #if defined DMCP_BUILD
      lcd_refresh();
    #endif //DMCP_BUILD
  #endif //TESTSUITE_BUILD
}


#if USE_NEW_DEI_INTEGRATION_CODE == 0
static void _integrate(calcRegister_t regist, const real_t *a, const real_t *b, real_t *acc, real_t *res, realContext_t *realContext) { // Double-Exponential Integration
  currentKeyCode = 255;
  bool_t exitSignalled = false;
  real_t bma2;            // (b - a)/2, a & b are the integration limits
  real_t bpa2;            // (b + 2)/2
  real_t eps;             // epsilon
  real_t thr;             // convergence threshold
  real_t lvl;             // level counter
  real_t tm;              // maximum abscissa in the t domain
  real_t h;               // space between abscissas in the t domain
  real_t ssp;             // partial sum at each level
  real_t j;               // abscissa counter
  real_t ch;              // cosh(t)
  real_t w;               // weight
  real_t rp;              // abscissa (r) or fplus + fminus (p)
  real_t ss;              // value of integral
  real_t ss1;             // previous ss

  real_t tmp, z, y, x;

  bool_t rev   = false;   // b is < a
  bool_t ES    = false;   // expsinh mode
  bool_t bpa2z = false;   // bpa2 is zero in sinhsinh or expsinh modes
  bool_t left  = false;   // expsinh case with -Infinite limit
  bool_t TS    = false;   // tanhsinh mode
  bool_t lg0   = false;   // true if level > 0

  #if !defined(TESTSUITE_BUILD)
    int loop = 0;
  #endif //TESTSUITE_BUILD

  // check arguments  ************************************
  if(realIsNaN(a) || realIsNaN(b)) { // check for invalid limits
    realCopy(const_NaN, res); // a or b is NaN, exit
    realCopy(const_NaN, acc);
    return;
  }
  if(realCompareEqual(a, b)) { // check for equal limits
    realZero(res); // a == b, return 0
    realZero(acc);
    return;
  }
  // set mode flags, bma2 and bpa2  **********************
  if(realCompareGreaterThan(a, b)) { // a > b?
    rev = true; // yes, flag it
  }
  if(realIsInfinite(a)) { // a == ±inf?
    ES = true; // yes, flag it (temporarily in ES)
  }
  if(realIsInfinite(b)) { // b == ±inf?
    bpa2z = true; // yes, flag it (temporarily in bpa2z)
  }
  // set the ES, bpa2z, left & TS flags according to the
  // a & b values ++++++++++++++++++++++++++++++++++++++++
  if(ES && bpa2z) { // skip if a != ±inf or b != ±inf
    // here in the sinhsinh case  --------------
    realZero(&y);
    realCopy(const_2, &x); // 2*bpa2 = 0, 2*bma2 = 2
    ES = false; // as this flag was dirty
    // [0 2 b a]
  } // done with sinhsinh
  //DEI_inf_1::
  else if(!(ES || bpa2z)) { // skip if a == ±inf or b == ±inf
    // here in the tanhsinh case  --------------
    realAdd(b, a, &y, realContext); // 2*bpa2 ready
    realSubtract(b, a, &x, realContext);
    realSetPositiveSign(&x); // 2*bma2 ready
    TS = true; // flag the tanhsinh case
    // [|b-a| a+b a a]
  } // done with tanhsinh
  //DEI_inf_2::
  else {
    // here in the expsinh case ----------------
    const real_t *aa = a, *bb = b;
    if(realIsInfinite(b)) {
      bb = a; aa = b; // now X is the finite limit
    }
    if(realCompareGreaterThan(bb, aa)) { // finite limit > infinite one?
      left = true; // yes, left case
    }
    realAdd(bb, bb, &y, realContext); // 2*bpa2 ready
    bpa2z = false; // as this flag was dirty
    if(realIsZero(bb)) { // finite limit == 0?
      bpa2z = true; // yes, flag it
    }
    ES = true; // flag the expsinh case
    realCopy(const_2, &x); // 2*bma2 ready
  }
  //DEI_ba2_rdy::
                                               // here with 2*bpa & 2*bma2 on stack
                                               // and mode flags ready
  realMultiply(&x, const_1on2, &bma2, realContext); // save bma2
  realMultiply(&y, const_1on2, &bpa2, realContext); // save bpa2
  // [bpa2 bma2]
  // done with mode flags, bpa2 and bma2  ****************
  // compute precision parameters ************************
  // [rewritten here for WP43]
  // [1e-34 -34 bpa2 bma2]
  realMultiply(acc, const_1on10, &eps, realContext);
  realMultiply(&eps, &eps, &eps, realContext);
  //if(realCompareAbsLessThan(&eps, const_1e_32)) {
  //  realCopy(const_1e_32, &eps); // save epsilon = 10^-37
  //}
  if(!realIsSpecial(a) && !realIsSpecial(b) && (realIsZero(a) || realIsZero(b))) { // WP43
    if(realCompareAbsLessThan(&eps, const_1e_6143)) {                              // WP43
      realCopy(const_1e_6143, &eps);                                               // WP43
    }                                                                              // WP43
  }                                                                                // WP43

  realCopy(acc, &thr); // save the convergence threshold [<- ACC]
  //real34ToReal(const34_7, &lvl); // maxlevel = round(log2(digits)) + 2 [pre-calculated]
  realCopy(realContext->digits >= 48 ? const_8 : const_7, &lvl);                   // WP43
  if(bpa2z) { // the sinhsinh case and the expsinh case
              // when the finite limit == 0 can use a
              // smaller epsilon
    realNextToward(const_0, const_1, &eps, &ctxtReal34); // save epsilon for such cases
  }
  // precision parameters (epsilon, thr, level) ready ****
  // compute maximum t value (tm) ************************
  // tm is computed in such a way that all abscissas are
  // representable at the given precision and that the
  // integrand is never evaluated at the limits
  // DEI_tanhsinh::
  if(TS) {// tanhsinh mode?
    // start tm computation for tanh-sinh mode
    realCopy(realCompareGreaterThan(&bma2, const_1) ? const_1 : &bma2, &x);
    realAdd(&x, &x, &x, realContext); // X = 2*MIN(1, bma2)
  } // goto remaining tm calculation
  // DEI_ss_es::
  else if(!bpa2z) { // neither SS nor ES with 0 limit?
    // start tm computation for ES with != 0 limit
    realDivide(const_1on2, &bpa2, &x, realContext);
    realSetPositiveSign(&x); // X = ABS(1/(2*bpa2))
  } // goto remaining tm calculation
  // DEI_ss_es0::
  else {
    // start tm computation for SS & ES0 cases
    realSquareRoot(&eps, &x, realContext); // X = SQRT(eps)
  }
  // DEI_cont::
  realDivide(&x, &eps, &x, realContext); // continue tm computation for all cases
  WP34S_Ln(&x, &x, realContext);
  if(!TS) { // not in TS mode?
    realAdd(&x, &x, &x, realContext); // 2*ln(...) for all cases except TS
  }
  realAdd(&x, &x, &x, realContext); // 4*ln(...) for all cases except TS (2*LN(...))
  realDivide(&x, const_pi, &x, realContext);
  WP34S_Ln(&x, &tm, realContext); // tm done
  if(realIsNaN(&tm)) {
    realCopy(const_minusInfinity, &tm);
  }
  // maximum t (tm) ready ********************************
  // level loop ******************************************
  realZero(&ss); realZero(&ss1);
  realCopy(&ss, &z); // to let Z = ss (which here is 0)
  realCopy(const_2, &h); // h = 2
  do { // DEI_lvl_loop::
    realCopy(&z, &ss1); // update ss1
    realCopy(const_0, &ssp); // ssp = 0
    realCopy(const_1, &j); // j = 1
    realMultiply(&h, const_1on2, &h, realContext); // h /= 2
    realCopy(&h, &x); // X = t
    // j loop ++++++++++++++++++++++++++++++++++++++++++++++
    // compute abscissas and weights  ----------------------
    do { // DEI_j_loop::
      #if !defined(TESTSUITE_BUILD)
        char tmps[100];
        exitSignalled |= exitKeyWaiting();
        loop++;
        if(checkHalfSec()) {
          sprintf(tmps,"Level:  %i Iter: ",(int16_t)realToInt32C47(&lvl));
          if(progressHalfSecUpdate_Integer(timed, tmps, loop, halfSec_clearZ, halfSec_clearT, halfSec_disp)) {; //timed
            #if defined(PC_BUILD)
              printf("%s %i\n",tmps,loop);
            #endif //PC_BUILD
            #if ENABLE_SOLVER_PROGRESS == 1
              //Error indication: incomplete, set to 0-0
              _showProgress(&ss, &bma2, &h, const_0, const_0, const_pi, realContext);
            #endif //ENABLE_SOLVER_PROGRESS
            if(!interruptedLoop && exitSignalled) {  //First EXIT press
              #if !defined(INTEGRATION_TWO_STAGE_EXIT)
                displayCalcErrorMessage(ERROR_SOLVER_ABORT, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
                return;
              #endif //INTEGRATION_TWO_STAGE_EXIT
              exitSignalled = false;
              interruptedLoop = 1;
            }
            if(interruptedLoop) {
              sprintf(tmps,"Level %i. %5.1fs or EXIT: Iter: ",(int16_t)k, (float)(40.0 - ((interruptedLoop++)/2.0)));
              radixProcess(tmps,tmps);
              progressHalfSecUpdate_Integer(force+1, tmps, loop, halfSec_clearZ, halfSec_clearT, halfSec_disp);
              if(exitSignalled || interruptedLoop >= 40) {      // Direct exit by exiting and simulating the end values
                exitSignalled = false;
                realMultiply(&sslast, &bma2, res, realContext);
                realMultiply(res, &h, res, realContext); // load the integral result,
                realMultiply(res, const_2, res, realContext);
                realCopy(&errval, acc); // its error value,
                displayCalcErrorMessage(ERROR_SOLVER_ABORT, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
                return;
              }
            }
          }
        }
      #endif //TESTSUITE_BUILD

      WP34S_SinhCosh(&x, NULL, &x, realContext); // cosh(t) (cosh is much faster than sinh/tanh)
      realCopy(&x, &ch); // save for later
      //realMultiply(&x, &x, &x, realContext);
      //realSubtract(&x, const_1, &x, realContext);
      realFMA(&x, &x, const__1, &x, realContext);
      realSquareRoot(&x, &x, realContext);
      realMultiply(&x, const_piOn2, &x, realContext); // pi/2*sqrt(cosh(t)^2 - 1) = pi/2*sinh(t)
      if(ES) { // ES mode?
        realExp(&x, &x, realContext); // yes, want EXP
      }
      else {
        WP34S_SinhCosh(&x, NULL, &x, realContext); // no, want COSH
      }
      realCopy(&x, &y); // FILL
      if(TS) { // TS mode?
        realMultiply(&x, &x, &x, realContext); // end weight computation for TS mode
        realDivide(const_1, &x, &x, realContext);
      } // DEI_not_ts::
      realCopy(&x, &w); // save weight
      // stack filled with exp/cosh of pi/2*sinh(t)
      realCopy(&y, &x);
      if(!ES) { // ES mode?
        //realMultiply(&x, &x, &x, realContext);      // no ES mode, get r (the abscissa in the
        //realSubtract(&x, const_1, &x, realContext); // normalized domain)
        realFMA(&x, &x, const__1, &x, realContext);
        realSquareRoot(&x, &x, realContext);
      } // DEI_es_skip::
      if(TS) { // TS mode?
        realDivide(&x, &y, &x, realContext); // yes, adjust r
      }
      if(left) {// ES mode -infinity?
        realChangeSign(&x); // yes, adjust r
      }
      realCopy(&x, &rp); // save normalized abscissa
      // done with abscissas and weights  --------------------
      // evaluate integrand ----------------------------------
      //realMultiply(&x, &bma2, &x, realContext); // r*(b - a)/2
      //realAdd(&x, &bpa2, &x, realContext); // (b + a)/2 + r*(b - a)/2
      realFMA(&x, &bma2, &bpa2, &x, realContext);
      DEI_xeq_user(regist, &x, &x, realContext); // f(bpa2 + bma2*r)
      realMultiply(&x, &w, &x, realContext); // fplus*w
      realCopy(&x, &tmp); realCopy(&rp, &x); realCopy(&tmp, &rp); // p = fplus*w stored, r in X
      realCopy(&x, &z);
      // RCL bpa2    // (b + a)/2
      // RCL bma2    // (b - a)/2
      if(!ES) { // ES mode?
        realMultiply(&bma2, &z, &x, realContext); // no ES mode, "normal" abscissa
        realSubtract(&bpa2, &x, &x, realContext);
      }
      else { // DEI_es_mode::
        realDivide(&bma2, &z, &x, realContext); // ES mode, "inverse" abscissa
        realAdd(&bpa2, &x, &x, realContext); // X = (b + a)/2 ± (b - a)/2*(r or 1/r)
      } // DEI_call_usr_m::
      DEI_xeq_user(regist, &x, &x, realContext); // f(bpa2 ± bma2*(r or 1/r))
      if(ES) { // ES mode?
        realDivide(&x, &w, &x, realContext); // ES mode, inverse weight, fminus/w
      }
      else {
        realMultiply(&x, &w, &x, realContext); // no ES mode, normal weight, fminus*w
      }
      realAdd(&x, &rp, &x, realContext); // p = fplus*w + fminus*(w or 1/w)
      realMultiply(&x, &ch, &x, realContext); // X = ch*p
      if(realIsSpecial(&x)) {
        realZero(&x); // safety check for weight
      }
      realAdd(&ssp, &x, &ssp, realContext); // ssp += p
      // done with integrand  --------------------------------
      // level 0 is special ----------------------------------
      if(lg0) { // is level == 0?
        realAdd(&j, const_1, &j, realContext); // no, sweep odd abscissas, j += 1
        // early test (mainly) for the sinhsinh case
        realCopyAbs(&x, &y); // X = ABS(p)
        realMultiply(&ssp, &eps, &x, realContext);
        realSetPositiveSign(&x); // X = ABS(ssp*eps), Y = ABS(p)
        if(exitSignalled || realCompareGreaterEqual(&x, &y)) { // ABS(p) <= ABS(eps*eps)?
          break;
        }
      }
      // if level > 0 done  ----------------------------------
      // DEI_updte_j::
      realAdd(&j, const_1, &j, realContext); // j += 1
      realMultiply(&j, &h, &x, realContext); // X = t = j*h
      if(lastErrorCode == ERROR_SOLVER_ABORT) { // Aborted?
        break;
      }
    } while(realCompareLessEqual(&x, &tm)); // t <= tm?
                                            // yes, continue j loop
    // done with j loop ++++++++++++++++++++++++++++++++++++
    // DEI_j_exit::
    realCopy(&ssp, &x); // no, update ss. X = ssp
    realAdd(&ss, &x, &ss, realContext); // ss += ssp
    if(!lg0) { // is level > 0 ----------------------------
      realCopy(&x, &z);
      realCopy(const_1, &y); // no, add 1st series term
      if(left) { // left?
        realChangeSign(&y);
      }
      realCopy(&bpa2, &x); // (b + a)/2
      if(ES) {// ES mode?
        realAdd(&y, &x, &x, realContext);
      }
      DEI_xeq_user(regist, &x, &x, realContext); // f(bpa2 (+/-1 in ES mode))
      realAdd(&ss, &x, &ss, realContext); // ss += f(bpa2)
    }
    // done with level 0  ----------------------------------
    // apply constant coeffs to sum ------------------------
    // DEI_chk_conv::
    realMultiply(&ss, &bma2, &x, realContext); // ss*bma2
    realMultiply(&x, &h, &x, realContext); // ss*bma2*h
    realMultiply(&x, const_piOn2, &x, realContext); // ss*bma2*h*pi/2
    if(rev) { // reverse?
      realChangeSign(&x); // yes,so change sign
    }
    // done with constant coeffs  --------------------------
    // show progress  --------------------------------------
    //          TOP?        // show value of approximation
    //            MSG MSG_INTEGRATE
    // check convergence  ----------------------------------
    realCopy(&x, &z);
    realAdd(&ssp, &ssp, &y, realContext); // 2*ssp
    realSubtract(&y, &ss, &y, realContext); // 2*ssp - ss
    realSetPositiveSign(&y); // ABS(2*ssp - ss)
    realMultiply(&thr, &ss, &x, realContext); // thr*ss
    realSetPositiveSign(&x); // ABS(thr*ss)
    if(exitSignalled || realCompareGreaterThan(&x, &y)) { // ABS(2*ssp - ss) < ABS(thr*ss)?
      break; // yes, computation done
    }
    lg0 = true; // mark level 0 done,
    realSubtract(&lvl, const_1, &lvl, realContext); // update level &...
    if(lastErrorCode == ERROR_SOLVER_ABORT) { // Aborted?
      break;
    }
  } while(realCompareGreaterEqual(&lvl, const_0)); // loop.
  // DEI_fin::
  realCopy(&z, &x); // recall result
  // done with level loop ********************************
  // compute error estimation and check for bad results **
  // here with result in X
  // DEI_chk_bad::
  realCopyAbs(&x, &y); // stack: ss-|ss|-ss-?
  realSubtract(&x, &ss1, &x, realContext); // stack: (ss-ss1)-|ss|-ss-?
  realSetPositiveSign(&x); // stack: err-|ss|-ss-?
  realMultiply(&x, const_10, &tmp, realContext); // stack: 10*err-err-|ss|-ss
  if(!exitSignalled && realCompareGreaterEqual(&tmp, &y)) { // 10*err < |ss|?
    // [tmp x y z]
    // no, bad result. stack: |ss|-err-10*err-ss
    // [y x tmp z]
    realAdd(&x, &y, &y, realContext); // stack: new_err-10*err-ss-ss
    // [0 y tmp z]
    // stack: 0-new_err-10*err-ss
    realZero(&z); // stack: 0-new_err-10*err-0
  } // DEI_res_okay::
  else { // yes, assume result is OK
    realCopy(&x, &tmp);
  }
  realCopy(&z, &x); realCopy(&tmp, &y); // stack: 0-err-... or ss-err-...
  // done with bad results  ******************************
  //  exit  **********************************************
  // DEI_exit::
  // [rewritten here for WP43]
  realCopy(&x, res);
  realCopy(&y, acc);
}



// The following routine is ported from Tanh-Sinh Quadrature
// https://newtonexcelbach.com/2020/10/29/numerical-integration-with-tanh-sinh-quadrature-v-5-0/
// Copyright © 2010, 2020  Graeme Dennes
// Licensed under GPL v3 or any later versions
// Originally written in Excel VBA; translated into C by MihailJP
// The description below is as is.

//    QUAD_TANH_SINH for finite intervals.
//    This is the fastest and simplest high-performance T-S program I have found.
//
//    It is based on the HP RPN calculator source code listing provided at
//    https://www.hpmuseum.org/forum/thread-8021.html.
//    The code shrunk to two Do loops as shown below.
//
//    The full RPN source code for the WP 34S calculator integration
//    program was written by M. César Rodríguez in 2017 for inclusion in
//    the WP 34s calculator software for numerical integration. It
//    covers the four intervals with four different programs:
//         a. finite interval, (a,b)
//         b. Right semi-infinite interval, (a,inf)
//         c. Left semi-infinite interval, (-inf,b)
//         d. Infinite interval, (-inf,inf)
//
//    The RPN source code selected is the first of the three versions
//    presented on the web page, shown as v1.2r-393 (20170327), and stated
//    as being suitable for keying in by hand. The RPN source code was
//    distilled down to the specific finite interval code which acted
//    as the basis of this present VBA program.
//
// The Tanh-Sinh Transform which transforms the function value is g(z) = f(x(z)) * dxdz(z)
//
// where   x(z) = (b + a) / 2 + (b - a) / 2 * TANH(SINH(z))
// and     dxdz(z) = (b - a) / 2 * COSH(z) / COSH(SINH(z))^2
//
// The purpose of the T-S transform is to transform a function over (-1,1) to a new function
// on the entire real line (-inf,inf), where the two integrals have the same value.
//
// It is the transformed function which is integrated by the T-S integrator
// using Trapezoidal summation.
// Because of the double exponential growth of the denominator in dxdz(z), the +-z sample
// points used for the trapezoidal rule do not have to step very far before g(z) becomes zero
// or sufficiently small for exit and termination. In other words, g(z) approaches zero
// at a double exponential rate. That is the power of the technique used in the T-S method.
//
// However, we don't calculate x(z) and dxdz(z) directly as above, as the term
// 1/COSH(SINH(z))^2 can overflow for large z.
//
// To prevent this, we calculate x(z) and dxdz(z) this way, courtesy of the Michalski & Mosig T-S integrator.
// The VBA code translated by the author from the Rodriguez RPN source was modified to use this technique.
// It improved the accuracy and reduced the execution time.
//
// 1. Enter value for z (z <= 709)
//
// 2. exz = exp(z)
//
// 3. q = exp(-2 * sinh(z))
//      = exp(-2 * (exz - 1/exz) / 2)
//      = exp(-(exz - 1/exz))
//      = exp(1/exz - exz)
//
// 4. delta = 2 * q / (1 + q) = (1 - tanh(sinh z))
//
// 5. x(z) = (b + a) / 2 + (b - a) / 2 * delta
//
// 6. fxz = f(x(z))
//
// 7. dxdz = dxdz(z) = (b - a) / 2 * (exz + 1/exz) * delta / (1 + q)
//
// 8. The transformed function = y = fxz * dxdz
//
// Now, for large positive z (6.5 < z < 709), q simply underflows harmlessly to zero.
// Negative z is handled by using the symmetry of the trapezoidal rule about
// the midpoint of the interval.

#if USE_MICHALSKI_MOSIG_TANH_SINH == 1
static void _integrate_mm(calcRegister_t regist, const real_t *llim, const real_t *ulim, real_t *acc, real_t *res, realContext_t *realContext) { // Double-Exponential Integration
  #if !defined(TESTSUITE_BUILD)
    int16_t interruptedLoop = 0;
    currentKeyCode = 255;
    bool_t exitSignalled = false;
  #endif //TESTSUITE_BUILD
  real_t a, b;
  real_t errval, bpa2, bma2;
  real_t eps, tol, h;
  real_t t, w, r, fp;
  real_t fm, p, expt, u;
  real_t ssp, ss, sslast; real_t x;

  real_t tmp;
  int k, maxlevel, j, evals;

  #if !defined(TESTSUITE_BUILD)
    int loop = 0;
  #endif //TESTSUITE_BUILD

  // Get the two limits of integration and the integrating variable:
  if(realCompareGreaterThan(llim, ulim)) { // Ensure the upper limit is greater than the lower limit
    realCopy(llim, &b); // upper limit in LLIM
    realCopy(ulim, &a); // lower limit in ULIM
  }
  else {
    realCopy(llim, &a); // lower limit
    realCopy(ulim, &b); // upper limit
  }
  // c = Parms(1, 1) ' intvar

  // Set some program constants. These are the optimum values.
  // epsilon
  realMultiply(acc, acc, &eps, realContext);
  // convergence tolerance
  realCopy(acc, &tol);
  // max level
  maxlevel = 7;

  #ifdef PC_BUILD
    printf"Temporary Debugging info. Can be deleted once done.\n";
    printRealToConsole(acc,"acc:","\n");
    printRealToConsole(&eps,"eps:","\n");
    printf("digits %i\n",realContext->digits);
    printf("regist %u\n",regist);
    printf("currentSolverStatus=%u, screenUpdatingMode=%u\n",currentSolverStatus, screenUpdatingMode);
  #endif //PC_BUILD

  realSubtract(&b, &a, &bma2, realContext); // interval half-length
  realMultiply(&bma2, const_1on2, &bma2, realContext);
  realAdd(&b, &a, &bpa2, realContext); // centre of interval
  realMultiply(&bpa2, const_1on2, &bpa2, realContext);
  k = 0; // level counter
  DEI_xeq_user(regist, &bpa2, &ss, realContext); // centre of interval
  evals = 1;
  realCopy(const_2, &h);
  realZero(&sslast);
  realZero(&errval);
  do {
    realZero(&ssp);
    j = 1;
    realMultiply(&h, const_1on2, &h, realContext); //h = 2 ^ -k

    do {
      #if !defined(TESTSUITE_BUILD)
        char tmps[64];
        exitSignalled |= exitKeyWaiting();
        loop++;
        if(checkHalfSec()) {
          sprintf(tmps,"Level: %i/%i Iter: ",(int16_t)k, (int16_t)maxlevel);
          if(progressHalfSecUpdate_Integer(timed, tmps, loop, !interruptedLoop, !interruptedLoop, !interruptedLoop)) { ; //timed
            #if defined(PC_BUILD)
              printf("%s %i\n",tmps,loop);
            #endif //PC_BUILD
            #if ENABLE_SOLVER_PROGRESS == 1
              _showProgress(&sslast, &bma2, &h, &errval, const_0, const_2, realContext);
            #endif //ENABLE_SOLVER_PROGRESS
            if(!interruptedLoop && exitSignalled) {  //First EXIT press
              #if !defined(INTEGRATION_TWO_STAGE_EXIT)
                displayCalcErrorMessage(ERROR_SOLVER_ABORT, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
                return;
              #endif //INTEGRATION_TWO_STAGE_EXIT
              exitSignalled = false;
              interruptedLoop = 1;
            }
            if(interruptedLoop) {
              sprintf(tmps,"Level %i. %5.1fs or EXIT: Iter: ",(int16_t)k, (float)(40.0 - ((interruptedLoop++)/2.0)));
              radixProcess(tmps,tmps);
              progressHalfSecUpdate_Integer(force+1, tmps, loop, halfSec_clearZ, halfSec_clearT, halfSec_disp);
              if(exitSignalled || interruptedLoop >= 40) {      // Direct exit by exiting and simulating the end values
                exitSignalled = false;
                realMultiply(&sslast, &bma2, res, realContext);
                realMultiply(res, &h, res, realContext); // load the integral result,
                realMultiply(res, const_2, res, realContext);
                realCopy(&errval, acc); // its error value,
                displayCalcErrorMessage(ERROR_SOLVER_ABORT, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
                return;
              }
            }
          }
        }
      #endif //TESTSUITE_BUILD

      int32ToReal(j, &t);
      realMultiply(&t, &h, &t, realContext); // t = h * j
      //If t > 6.56 Then Exit Do
      if(realCompareGreaterThan(&t, const_7)) {
        break;
      }

      realExp(&t, &expt, realContext);

      realFMA(&expt, &expt, const__1, &u, realContext);
      realChangeSign(&u);
      realDivide(&u, &expt, &u, realContext);
      realExp(&u, &u, realContext); // = exp(-(expt - 1/expt)) = exp(-2 (expt -1/expt) /2) = exp(-2 sinh t)

      realAdd(const_1, &u, &r, realContext);
      realDivide(&u, &r, &r, realContext);
      realAdd(&r, &r, &r, realContext); // r = 2 * u / (1 + u) ' r = 1 - tanh(sinh t)
      // Added so as to check that r <> 0 and r <> 1 to ensure r hasn't rounded to 0 or 1 when tanh(sinh(t)) is very close to 0 or 1.
      if(!realIsZero(&r) && !realCompareEqual(&r, const_1)) {
        realMultiply(&bma2, &r, &x, realContext);
      }
      else {
        break;
      }
      // Added to check that (a + x) > a to ensure (a + x) hasn't rounded to a when x is very small.
      // This prevents calculation of the function at lower limit a.
      realAdd(&a, &x, &tmp, realContext);
      if(realCompareGreaterThan(&tmp, &a)) {
        DEI_xeq_user(regist, &tmp, &fp, realContext);
        ++evals;
      }
      // Added to check that (b - x) < b to ensure (b - x) hasn't rounded to b when x is very small.
      // This prevents calculation of the function at upper limit b.
      realSubtract(&b, &x, &tmp, realContext);
      if(realCompareLessThan(&tmp, &b)) {
        DEI_xeq_user(regist, &tmp, &fm, realContext);
        ++evals;
      }
      realAdd(const_1, &u, &w, realContext);
      realDivide(&r, &w, &w, realContext);
      realFMA(&expt, &expt, const_1, &tmp, realContext);
      realDivide(&tmp, &expt, &tmp, realContext);
      realMultiply(&tmp, &w, &w, realContext); // w = cosh t / cosh^2 (sinh t) See separate proof.

      realAdd(&fp, &fm, &p, realContext);
      realMultiply(&p, &w, &p, realContext);
      realAdd(&ssp, &p, &ssp, realContext);
      j += 1 + (k > 2);
      realMultiply(&ssp, &eps, &tmp, realContext);
    } while(realCompareAbsLessThan(&tmp, &p));

    realAdd(&ss, &ssp, &ss, realContext);
    realDivide(&sslast, &ss, &errval, realContext);
    realFMA(const_2, &errval, const__1, &errval, realContext);
    realSetPositiveSign(&errval);
    realCopy(&ss, &sslast);
    ++k;

  } while(realCompareGreaterEqual(&errval, &tol) && k <= maxlevel);
  realMultiply(&ss, &bma2, res, realContext);
  realMultiply(res, &h, res, realContext); // load the integral result,
  realCopy(&errval, acc); // its error value,
  //Result(3) = evals ' func evals
  //Result(4) = MicroTimer - Result(4) ' and the calculation time
#endif // USE_MICHALSKI_MOSIG_TANH_SINH == 1

#endif // USE_NEW_DEI_INTEGRATION_CODE

#if USE_NEW_DEI_INTEGRATION_CODE == 2
/*
   The new DEI (double exponential integral) code is an improvement on the
   Michalski-Mosig code. The document
       https://www.genivia.com/files/qthsh.pdf
   contains a comparision of several DEI integration routines along with the reasoning
   behind the improved code.

   The new code here consists of two main functions, along with a couple of helpers.

   dbl_exp_int_new is the main integration routine. It is mostly a direct
   copy of the code on pp 33-34 of the above document, using the same variable names
   where possible. double variables are replaced by real_t.
   It handles cases where none, one, or both limits are infinite.

   exp_sinh_opt_d chooses an optimum "split point" for the integration routine
   in the case of one infinite limit, before the integral is calculated.
   It involves few function calls and can speed up such integrals significantly.
   The code comes from pp 31-32 of the above document. Again, the code is translated
   as directly as possible, and real_t variables are used.

   The helper function is:
   DEI_xeq_usr_adr is a helper function used by exp_sinh_opt_d to find f(a+d/r) and f(a+d*r), where
   f is the user function being integrated.

   The macro USE_NEW_DEI_INTEGRATION_CODE, defined in src/c47/defines.h, controls the
   use of the new routines.

   When USE_NEW_DEI_INTEGRATION_CODE == 0 the new routines are not used.
   When USE_NEW_DEI_INTEGRATION_CODE == 1 the new integration routine is used but not the split point routine.
   When USE_NEW_DEI_INTEGRATION_CODE == 2 both routines are used.
*/

/*
  Helper function called by exp_sinh_opt_d routine to obtain values
  of fl, fr, and h by calling the user function.
*/
static void DEI_xeq_user_adr(calcRegister_t regist, const real_t* a, const real_t* d, const real_t* r, real_t* fl, real_t* fr, real_t* h, realContext_t *realContext) {
  real_t s1;
  realDivide(d, r, &s1, realContext); // s1 = d/r
  realAdd(a, &s1, &s1, realContext); // s1 = a+d/r
  DEI_xeq_user(regist, &s1, fl, realContext); // fl = f(a+d/r)

  realMultiply(d, r, &s1, realContext); // s1 = d*r
  realAdd(a, &s1, &s1, realContext); // s1 = a+d*r
  DEI_xeq_user(regist, &s1, fr, realContext); // fr = f(a+d*r)
  realMultiply(fr, r, fr, realContext);
  realMultiply(fr, r, fr, realContext); // fr = r*r*f(a+d*r)

  realSubtract(fl, fr, h, realContext); // h = fl - fr
}

/*
  A couple of macros used in exp_sinh_opt_d
  and undefined afterwards.
*/

#define IS_INFINITE(X) (realIsInfinite(X) || realIsNaN(X))
#define IS_FINITE(X) (!(realIsInfinite(X) || realIsNaN(X)))

/*
  exp_sinh_opt_d eturns optimized Exp-Sinh (i.e, semi-infinite)
  integral split point d.
  This code comes from the qthsh.pdf document.
  It attempts to estimate the maximum value of x*f(x)
  and to use that as the split point, rather than the default value of 1.
  It works surprisingly well given the small number of function evaluations it uses.
  See the discussion towards the end of this thread:
  https://www.hpmuseum.org/forum/thread-16549-post-184118.html#pid184118
*/

static real_t* exp_sinh_opt_d(calcRegister_t regist, const real_t* a, const real_t* eps, real_t* d, realContext_t *realContext) {

  real_t fl, fr, h2;
  real_t r, h;
  real_t lfl, lfr, lr, s;

  DEI_xeq_user_adr(regist, a, d, const_2, &fl, &fr, &h2, realContext);

  if(IS_INFINITE(&h2) || (realIsZero(&fl) && realIsZero(&fr))) return d;
  // function undefined or zero - don't bother.

  uint16_t i = 1, j = 32; // j=32 is optimal to find r
  real_t s1; // scratch variable

  realZero(&s); //
  realCopy(const_2, &lr);
  do { // find max j such that fl and fr are both finite - will usually be 16.
    j /= 2;
    uInt32ToReal(1 << (i + j), &r);
    DEI_xeq_user_adr(regist, a, d, &r, &fl, &fr, &h, realContext);
  } while(j > 1 && IS_INFINITE(&h));

  if(j > 1 && IS_FINITE(&h) && (realGetSign(&h) != realGetSign(&h2))) {

    realCopy(&fl, &lfl);
    realCopy(&fr, &lfr);

    do { // bisect in 4 iterations
      j /= 2;
      uInt32ToReal(1 << (i + j), &r);
      DEI_xeq_user_adr(regist, a, d, &r, &fl, &fr, &h, realContext);
      if(IS_FINITE(&h)) {
        realCopyAbs(&h, &s1);
        realAdd(&s, &s1, &s, realContext); // sum |h| to remove noisy cases - probably not needed with decNumbers
        if(realGetSign(&h) == realGetSign(&h2)) {
          i += j; // search right half
        }
        else { // search left half
          realCopy(&fl, &lfl);
          realCopy(&fr, &lfr);
          realCopy(&r, &lr);
        }
      }
    } while(j > 1);

    if(realCompareGreaterThan(&s, const_1e_32)) { // if sum of |h| > small ...
      realSubtract(&lfl, &lfr, &h, realContext);
      realCopy(&lr, &r);
      if(!realIsZero(&h)) { // if last diff != 0, back up r by one step
        realDivide(&r, const_2, &r, realContext);
      }
      realSetPositiveSign(&lfl);
      realSetPositiveSign(&lfr);
      if(realCompareLessThan(&lfl, &lfr)) {
        realDivide(d, &r, d, realContext); // move d closer to the finite endpoint
      }
      else {
        realMultiply(d, &r, d, realContext);  // move d closer to the infinite endpoint
      }
    }
  }

  return d;
}
#undef IS_FINITE
#undef IS_INFINITE
#endif // USE_NEW_DEI_INTEGRATION_CODE == 2

#if USE_NEW_DEI_INTEGRATION_CODE > 0
/*
  This is the new integration routine. It is mostly a direct
  translation of the final version of the improved quad routine
  in the document https://www.genivia.com/files/qthsh.pdf
  a and b are the lower and upper limits of integration. Either or both may be infinite.
  sign is set to -1 by the calling routine integrate() if a > b originally.
  error is the desired fractional error; the estimated fractional error is returned in it.
  The result is returned in result.
  Code in integrate() below ensures that a < b.
 */

static void dbl_exp_int_new(calcRegister_t regist, const real_t *a, const real_t *b, real_t *error, real_t *result, int sign, realContext_t *realContext) {
  #if !defined(TESTSUITE_BUILD)
    int16_t interruptedLoop = 0;
    currentKeyCode = 255;
    bool_t exitSignalled = false;
  #endif //TESTSUITE_BUILD

  real_t c, d, s, v, h, y, eps;
  real_t s1, s2, s3; // scratch variables

  int k = 0, mode = 0; // Tanh-Sinh = 0, Exp-Sinh = 1, Sinh-Sinh = 2

  const int maxlevel = 7;

  #if !defined(TESTSUITE_BUILD)
  int loop = 0;
  #endif //TESTSUITE_BUILD

  realCopy(error, &eps);

  realZero(&c);
  realCopy(const_1, &d);
  realCopy(const_2, &h);

  if(realIsNaN(a) || realIsNaN(b)) { // check for invalid limits
    realCopy(const_NaN, result); // a or b is NaN, exit
    realCopy(const_NaN, error);
    return;
  }

  if(realCompareEqual(a, b)) { // check for equal limits
    realZero(result); // a == b, return 0
    realZero(error);
    return;
  }

  realZero(error);  // initial error is zero
  realZero(result); // initial result is zero

  if((!realIsInfinite(a)) && (!realIsInfinite(b))) {
    realAdd(a, b, &c, realContext);
    realDivide(&c, const_2, &c, realContext);

    realSubtract(b, a, &d, realContext);
    realDivide(&d, const_2, &d, realContext);

    realCopy(&c, &v);
  }
  else if(!realIsInfinite(a)) { // int from a to infinity
    mode = 1; // Exp-Sinh
    realCopy(a, &c); // c = a

    // use d = 1 (set above), or optimise
    #if USE_NEW_DEI_INTEGRATION_CODE == 2
    realCopy(exp_sinh_opt_d(regist, a, &eps, &d, realContext), &d);
    #endif
    realAdd(a, &d, &v, realContext); // v = a + d

  }
  else if(!realIsInfinite(b)) { // int from -infinity to b
    mode = 1; // Exp-Sinh
    realCopy(b, &c); // c = b
    sign = -sign;

    realMinus(&d, &d, realContext); // d = -1
    // either use d = -1, or optimise
    #if USE_NEW_DEI_INTEGRATION_CODE == 2
    realCopy(exp_sinh_opt_d(regist, b, &eps, &d, realContext), &d);
    #endif
    realAdd(b, &d, &v, realContext); // v = b + d
  }
  else {
    mode = 2; // Sinh-Sinh
    realZero(&v);
  }

  DEI_xeq_user(regist, &v, &s, realContext);

  // Now a, b, c, d, v, and mode have the correct values.
  // k has been set to zero; h is 2.
  // s = f(v)

  do {
    real_t p, q, fp, fm, t, eh;
    realZero(&p);
    realZero(&fp);
    realZero(&fm);

    realDivide(&h, const_2, &h, realContext);
    realExp(&h, &eh, realContext);
    realCopy(&eh, &t);

    if(k > 0) {
      realMultiply(&eh, &eh, &eh, realContext);
    }

    if(mode == 0) { // Tanh-Sinh
      do {
        real_t u, r, w, x;
        // Progress display code goes inside inner loop, to be consistent with previous code.
        // The result and error are in .... result and error! No multiplication needed.
        // In showprogress: result as ss; const_1 for bma2 and h, and fact;
        // error for a and const_0 for b.
        // Note that result and error do not change within this inner loop so
        // only the loop counter changes each time.
        #if !defined(TESTSUITE_BUILD)
          char tmps[64];
          exitSignalled |= exitKeyWaiting();
          loop++;
          if(checkHalfSec()) {
            sprintf(tmps,"Level: %i/%i Iter: ",(int16_t)k, (int16_t)maxlevel);
            if(progressHalfSecUpdate_Integer(timed, tmps, loop, !interruptedLoop, !interruptedLoop, !interruptedLoop)) { ; //timed
            #if defined(PC_BUILD)
              printf("%s %i\n",tmps,loop);
            #endif //PC_BUILD
              #if ENABLE_SOLVER_PROGRESS == 1
              _showProgress(result, const_1, const_1, error, const_0, const_1, realContext);
              #endif //ENABLE_SOLVER_PROGRESS
            if(!interruptedLoop && exitSignalled) {  //First EXIT press
                #if !defined(INTEGRATION_TWO_STAGE_EXIT)
                  displayCalcErrorMessage(ERROR_SOLVER_ABORT, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
                  return;
                #endif //INTEGRATION_TWO_STAGE_EXIT
                exitSignalled = false;
                interruptedLoop = 1;
              }
              if(interruptedLoop) {
                sprintf(tmps,"Level %i. %5.1fs or EXIT: Iter: ",(int16_t)k, (float)(40.0 - ((interruptedLoop++)/2.0)));
                radixProcess(tmps,tmps);
                progressHalfSecUpdate_Integer(force+1, tmps, loop, halfSec_clearZ, halfSec_clearT, halfSec_disp);
                if(exitSignalled || interruptedLoop >= 40) {      // Direct exit
                  exitSignalled = false;
                  displayCalcErrorMessage(ERROR_SOLVER_ABORT, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
                  return;
                }
              }
            }
          }
        #endif //TESTSUITE_BUILD


        realDivide(const_1, &t, &s1, realContext); // s1 stores 1/t
        realSubtract(&s1, &t, &u, realContext);
        realExp(&u, &u, realContext); // u = exp(1/t-t)

        realAdd(&u, const_1, &r, realContext);
        realDivide(&u, &r, &r, realContext);
        realMultiply(&r, const_2, &r, realContext); // r = 2*u/(1+u);

        realAdd(&t, &s1, &s2, realContext);
        realMultiply(&r, &s2, &w, realContext);
        realAdd(&u, const_1, &s2, realContext);
        realDivide(&w, &s2, &w, realContext); // w = (t+1/t)*r/(1+u);

        realMultiply(&d, &r, &x, realContext); // x = d*r;

        realAdd(a, &x, &s1, realContext);
        if(realCompareGreaterThan(&s1, a)) { // if too close to a then reuse previous fp
          DEI_xeq_user(regist, &s1, &y, realContext);
          if(!realIsInfinite(&y))
            realCopy(&y, &fp);  // if f(x) is finite, add to local sum
        }

        realSubtract(b, &x, &s1, realContext);
        if(realCompareLessThan(&s1, b)) { // if too close to a then reuse previous fp
          DEI_xeq_user(regist, &s1, &y, realContext);
          if(!realIsInfinite(&y))
            realCopy(&y, &fm);  // if f(x) is finite, add to local sum
        }

        realAdd(&fp, &fm, &s1, realContext);
        realMultiply(&s1, &w, &q, realContext); // q = w*(fp+fm)
        realAdd(&p, &q, &p, realContext); // p += q
        realMultiply(&t, &eh, &t, realContext); // t *= eh

        realMultiply(&eps, realCopyAbs(&p, &s1), &s2, realContext); // s2 = eps*abs(p)
      } while(realCompareGreaterThan(realCopyAbs(&q, &s1), &s2)); // while abs(q) > eps*abs(p)
    }

    else {
      realDivide(&t, const_2, &t, realContext);
      do {
        real_t r, w, x;

        #if !defined(TESTSUITE_BUILD)
          char tmps[64];
          exitSignalled |= exitKeyWaiting();
          loop++;
          if(checkHalfSec()) {
            sprintf(tmps,"Level: %i/%i Iter: ",(int16_t)k, (int16_t)maxlevel);
            if(progressHalfSecUpdate_Integer(timed, tmps, loop, !interruptedLoop, !interruptedLoop, !interruptedLoop)) { ; //timed
              #if defined(PC_BUILD)
                printf("%s %i\n",tmps,loop);
              #endif //PC_BUILD
              #if ENABLE_SOLVER_PROGRESS == 1
                _showProgress(result, const_1, const_1, error, const_0, const_1, realContext);
              #endif //ENABLE_SOLVER_PROGRESS
              if(!interruptedLoop && exitSignalled) {  //First EXIT press
                #if !defined(INTEGRATION_TWO_STAGE_EXIT)
                  displayCalcErrorMessage(ERROR_SOLVER_ABORT, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
                  return;
                #endif //INTEGRATION_TWO_STAGE_EXIT
                exitSignalled = false;
                interruptedLoop = 1;
              }
              if(interruptedLoop) {
                sprintf(tmps,"Level %i. %5.1fs or EXIT: Iter: ",(int16_t)k, (float)(40.0 - ((interruptedLoop++)/2.0)));
                radixProcess(tmps,tmps);
                progressHalfSecUpdate_Integer(force+1, tmps, loop, halfSec_clearZ, halfSec_clearT, halfSec_disp);
                if(exitSignalled || interruptedLoop >= 40) {      // Direct exit
                  exitSignalled = false;
                  displayCalcErrorMessage(ERROR_SOLVER_ABORT, ERR_REGISTER_LINE, NIM_REGISTER_LINE);
                  return;
                }
              }
            }
          }
        #endif //TESTSUITE_BUILD

        realMultiply(&t, const_4, &s1, realContext); // s1 = 4t
        realDivide(const_1, &s1, &s1, realContext); // s1 = 0.25/t
        realSubtract(&t, &s1, &s1, realContext); // s1 = t - 0.25/t
        realExp(&s1, &r, realContext); // r = exp(t-0.25/t)

        realCopy(&r, &w);

        realZero(&q);

        if(mode == 1) { // Exp-Sinh
          realAdd(&c, realDivide(&d, &r, &s1, realContext), &x, realContext); // x = c+d/r; // d/r in s1
          if(realCompareEqual(&x, &c)) {
            break;
          }
          DEI_xeq_user(regist, &x, &y, realContext);
          if(!realIsInfinite(&y)) { // if f(x) is finite, add to local sum
            realAdd(&q, realDivide(&y, &w, &s1, realContext), &q, realContext);
          }
        }
        else { // Sinh-Sinh
          realSubtract(&r, realDivide(const_1, &r, &s2, realContext), &s1, realContext);
          realDivide(&s1, const_2, &r, realContext); // r = (r-1/r)/2

          realAdd(&w, realDivide(const_1, &w, &s2, realContext), &s1, realContext);
          realDivide(&s1, const_2, &w, realContext); // w = (w+1/w)/2

          realSubtract(&c, realMultiply(&d, &r, &s1, realContext), &x, realContext); // x = c-d*r; // d*r in s1
          DEI_xeq_user(regist, &x, &y, realContext);
          if(!realIsInfinite(&y)) { // if f(x) is finite, add to local sum
            realAdd(&q, realMultiply(&y, &w, &s1, realContext), &q, realContext);
          }
        }

        realAdd(&c, realMultiply(&d, &r, &s1, realContext), &x, realContext); // x = c+d*r; // d*r in s1
        DEI_xeq_user(regist, &x, &y, realContext);

        if(!realIsInfinite(&y)) { // if f(x) is finite, add to local sum
          realAdd(&q, realMultiply(&y, &w, &s1, realContext), &q, realContext);
        }

        realDivide(const_1, realMultiply(&t, const_4, &s2, realContext), &s1, realContext); //s1 = 1/(4t)
        realMultiply(&q, realAdd(&t, &s1, &s2, realContext), &q, realContext); // q = q * (t + 1/4t)
        realAdd(&p, &q, &p, realContext); // p += q;
        realMultiply(&t, &eh, &t, realContext); // t *= eh;
        realMultiply(&eps, realCopyAbs(&p, &s1), &s2, realContext); // s2 = eps*abs(p)
      } while(realCompareGreaterThan(realCopyAbs(&q, &s1), &s2)); // while abs(q) > eps*abs(p)
    }

    realSubtract(&s, &p, &v, realContext); // v = s - p
    realAdd(&s, &p, &s, realContext); // s+=p

    realCopyAbs(&s, &s1); // s1 = abs(s)
    realCopyAbs(&v, &s2); // s2 = abs(v)
    ++k;
    // We want to calculate an estimate of the integral here.
    // It is sign*s*h*d.
    realMultiply(&d, realMultiply(&s, &h, &s3, realContext), result, realContext);
    if(sign == -1) {
      realMinus(result, result, realContext); // result = sign*s*h*d
    }
    // realDivide(&s2, realAdd(&s1, &eps, &s3, realContext), error, realContext); // error = abs(v)/(abs(s)+eps)
    realDivide(&s2, realAdd(&s1, &s2, &s3, realContext), error, realContext); // error = abs(v)/(abs(s)+abs(v)) - ND change
    if(realIsNaN(error)) {
      realCopy(const_1, error); // only happens when v, s both zero
    }
  } while(realCompareGreaterThan( &s2, realMultiply(const_10, realMultiply(&eps, &s1, &s3, realContext), &s3, realContext)) && k <= maxlevel); // while abs(v) > 10*eps*abs(s)
  return;
}

#endif // USE_NEW_DEI_INTEGRATION_CODE > 0

void integrate(calcRegister_t regist, const real_t *a, const real_t *b, real_t *acc, real_t *res, realContext_t *realContext) {
  bool_t was_solving = getSystemFlag(FLAG_SOLVING);
  ++currentSolverNestingDepth;
  setSystemFlag(FLAG_INTING);
  clearSystemFlag(FLAG_SOLVING);
  #if USE_NEW_DEI_INTEGRATION_CODE > 0
  if(realCompareLessThan(a, b)) {
    dbl_exp_int_new(regist, a, b, acc, res, 1, realContext);
  }
  else { // a, b might be NaN or both equal; handled in function.
    dbl_exp_int_new(regist, b, a, acc, res, -1, realContext);
  }
  #else
    #if USE_MICHALSKI_MOSIG_TANH_SINH == 1
    if(!realIsSpecial(a) && !realIsSpecial(b)) {
      _integrate_mm(regist, a, b, acc, res, realContext);
    }
    else
  #endif // USE_MICHALSKI_MOSIG_TANH_SINH == 1
    {
      _integrate(regist, a, b, acc, res, realContext);
    }
  #endif // USE_NEW_DEI_INTEGRATION_CODE
  if((--currentSolverNestingDepth) == 0) {
    clearSystemFlag(FLAG_INTING);
  }
  else if(was_solving) {
    clearSystemFlag(FLAG_INTING);
    setSystemFlag(FLAG_SOLVING);
  }
}
