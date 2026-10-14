// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file wp34s.c
 ***********************************************/

#include "c47.h"

/******************************************************
 * This functions are borrowed from the WP34S project
 ******************************************************/


#if !defined(PC_BUILD)
  #undef CACHE_DEBUG
  #undef CACHE_VERIFY
#endif

/****************************************************************************************************
 * Modified for direct handling of 1071 digit contexts.
 * Take care not to use the 1071 version for the DM42 hardware
 ****************************************************************************************************/
#undef DEBUGTAYLOR

#define TaylorIterationMax 1000
#if !defined(PC_BUILD)
  #undef DEBUGTAYLOR
#endif
static void C47_WP34S_SinCosTanTaylor_temp75   (const real_t *angle, bool_t swap, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext); // angle in radian
#if defined(OPTION_XFN_1000)
  static void C47_WP34S_SinCosTanTaylor_temp1071 (const real_t *angle, bool_t swap, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext); // angle in radian
#endif // OPTION_XFN_1000


void reduceAngleToRange(real_t* angle, const real_t** angle45, const real_t** angle90, const real_t** angle180, angularMode_t* angularMode, int32_t savedContextDigits, realContext_t* realContext) {
  switch(*angularMode) {
    case amRadian: {
      if(savedContextDigits >= 1071) {
        *angle45  = const1071_piOn4;
        *angle90  = const1071_piOn2;
        *angle180 = const1071_pi;
      }
      else {
        *angle45  = const75_piOn4;
        *angle90  = const75_piOn2;
        *angle180 = const75_pi;
      }
      mod2Pi(angle, angle, realContext); // mod(angle, 2pi) --> angle
      break;
    }

    case amMultPi: {
      *angle45  = const_1on4;
      *angle90  = const_1on2;
      *angle180 = const_1;
      WP34S_Mod(angle, const_2, angle, realContext); // mod(angle, 2) --> angle
      break;
    }

    case amGrad: {
      *angle45  = const_50;
      *angle90  = const_100;
      *angle180 = const_200;
      WP34S_Mod(angle, const_400, angle, realContext); // mod(angle, 400g) --> angle
      break;
    }

    case amDegree:
    case amDMS: {
      *angle45  = const_45;
      *angle90  = const_90;
      *angle180 = const_180;
      WP34S_Mod(angle, const_360, angle, realContext); // mod(angle, 360°) --> angle
      *angularMode = amDegree;
      break;
    }

    default: {
    }
  }
}


// called from WP34S_Cvt2RadSinCosTan for 75 digits max, by by agm, sin, sinc, cos, tan, multiple elliptic functions, exp (complex), fib, gd, tanh, WP34S_Zeta
// called from C47_WP34S_Cvt2RadSinCosTan for 1071+ XFN
static void doWP34S_SinCosTanTaylor(real_t* angle, bool* sinNeg, bool* cosNeg, bool* swap, real_t* sinOut, real_t* cosOut, real_t* tanOut, angularMode_t angularMode, int32_t savedContextDigits, realContext_t* realContext) {
  const real_t *angle45, *angle90, *angle180;
  angle45  = const_0;
  angle90  = const_0;
  angle180 = const_0;

  #if defined(DEBUGTAYLOR)
   realToString(angle, tmpString);
   //tmpString[80]=0;
   printf("Angle:   %s\n", tmpString);
  #endif //DEBUGTAYLOR

  // sin(-x) = -sin(x), cos(-x) = cos(x)
  if(realIsNegative(angle)) {
    *sinNeg = true;
    realSetPositiveSign(angle);
  }

  reduceAngleToRange(angle, &angle45, &angle90, &angle180, &angularMode, savedContextDigits, realContext);

  #if defined(DEBUGTAYLOR)
   realToString(angle, tmpString);
   //tmpString[80]=0;
   printf("Reduced: %s\n", tmpString);
  #endif //DEBUGTAYLOR

  // sin(180+x) = -sin(x), cos(180+x) = -cos(x)
  if(realCompareGreaterEqual(angle, angle180)) {        // angle >= 180°
    realSubtract(angle, angle180, angle, realContext); // angle - 180° --> angle
    *sinNeg = !(*sinNeg);
    *cosNeg = !(*cosNeg);
  }

  // sin(90+x) = cos(x), cos(90+x) = -sin(x)
  if(realCompareGreaterEqual(angle, angle90)) {        // angle >= 90°
    realSubtract(angle, angle90, angle, realContext); // angle - 90° --> angle
    *swap = true;
    *cosNeg = !(*cosNeg);
  }

  // sin(90-x) = cos(x), cos(90-x) = sin(x)
  if(realCompareEqual(angle, angle45)) { // angle == 45°
    if(sinOut != NULL) {
     realCopy(const39_root2on2, sinOut);
    }
    if(cosOut != NULL) {
      realCopy(const39_root2on2, cosOut);
    }
    if(tanOut != NULL) {
      realSetOne(tanOut);
    }
  }
  else { // angle < 90
    if(realCompareGreaterThan(angle, angle45)) {        // angle > 45°
      realSubtract(angle90, angle, angle, realContext); // 90° - angle  --> angle
      *swap = !(*swap);
    }
    const bool_t angle30 = (angularMode == amDegree && realCompareEqual(angle, const_30));   // exactly 30°: its sine is 1/2
    convertAngleFromTo(angle, angularMode, amRadian, realContext);
    #if defined(OPTION_XFN_1000)
      if(savedContextDigits >= 1071) {
        C47_WP34S_SinCosTanTaylor_temp1071(angle, *swap, (*swap) ? cosOut : sinOut, (*swap) ? sinOut : cosOut, tanOut, realContext); // angle in radian
      }
      else
    #endif // OPTION_XFN_1000
    {
      C47_WP34S_SinCosTanTaylor_temp75(angle, *swap, (*swap) ? cosOut : sinOut, (*swap) ? sinOut : cosOut, tanOut, realContext); // angle in radian
    }
    real_t *sinOfAngle = (*swap) ? cosOut : sinOut;
    if(angle30 && sinOfAngle != NULL) {
      realCopy(const_1on2, sinOfAngle);
    }
  }

  realContext->digits = savedContextDigits;

  if(sinOut != NULL) {
    if(*sinNeg) {
      realSetNegativeSign(sinOut);
      if(tanOut != NULL) {
        realSetNegativeSign(tanOut);
      }
    }
    if(realIsZero(sinOut)) {
      realSetPositiveSign(sinOut);
      if(tanOut != NULL) {
        realSetPositiveSign(tanOut);
      }
    }
    realPlus(sinOut, sinOut, realContext);
  }

  if(cosOut != NULL) {
    if(*cosNeg) {
      realSetNegativeSign(cosOut);
      if(tanOut != NULL) {
        realChangeSign(tanOut);
      }
    }
    if(realIsZero(cosOut)) {
      realSetPositiveSign(cosOut);
    }
    realPlus(cosOut, cosOut, realContext);
  }

  if(tanOut != NULL && realIsZero(cosOut)) {
    realSetPositiveSign(tanOut);
    realPlus(tanOut, tanOut, realContext);
  }
}


// Called directly 75 digits max, by by agm, sin, sinc, cos, tan, multiple elliptic functions, exp (complex), fib, gd, tanh, WP34S_Zeta
//
// Have to be careful here to ensure that every function we call can handle the increased size of the numbers we're using.
static void C47_WP34S_Cvt2RadSinCosTan_75_helper(const real_t *an, angularMode_t angularMode, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext) {
  bool_t sinNeg = false, cosNeg = false, swap = false;
  real_t angle;

  if(realIsNaN(an)) {
    if(sinOut != NULL) {
      realSetNaN(sinOut);
    }
    if(cosOut != NULL) {
      realSetNaN(cosOut);
    }
    if(tanOut != NULL) {
      realSetNaN(tanOut);
    }
   return;
  }

  realCopy(an, &angle);

  int32_t savedContextDigits = realContext->digits;

  if(realContext->digits > 51) {
    realContext->digits = 75;
  }
  else {
    realContext->digits = 51;
  }

  doWP34S_SinCosTanTaylor(&angle, &sinNeg, &cosNeg, &swap, sinOut, cosOut, tanOut, angularMode, savedContextDigits, realContext);
}





static void doTaylorIterations(const real_t *a, real_t* angle, real_t* a2, real_t* t, real_t* j, real_t* z, real_t* sin, real_t* cos, real_t *sinOut, real_t *cosOut, real_t* epsilonOrCompare, const bool_t doEpsilon, const int epsilonDigits, realContext_t *realContext) {
  char tmpEpsilon[16];
  bool_t endSin = (sinOut == NULL), endCos = (cosOut == NULL);
  int i;

  if(doEpsilon) {
    sprintf(tmpEpsilon, "1E-%d", epsilonDigits);
    stringToReal(tmpEpsilon, epsilonOrCompare, realContext);
  }
  realCopy(a, angle);
  realMultiply(angle, angle, a2, realContext);
  realSetOne(j);
  realSetOne(t);
  realSetOne(sin);
  realSetOne(cos);

  for(i=1; !(endSin && endCos) && i<TaylorIterationMax; i++) { // it goes up to 31 max in the test suite
    realAdd(j, const_1, j, realContext);
    realDivide(a2, j, z, realContext);
    realMultiply(t, z, t, realContext);
    realChangeSign(t);
    int tExp = realGetExponent(t);

    if(!endCos) {
      realCopy(cos, z);
      realAdd(cos, t, cos, realContext);
      if(doEpsilon) {
        realCopyAbs(t, z);
      }
      else {
        realCompare(cos, z, epsilonOrCompare, realContext);
      }
      endCos = (!doEpsilon && realIsZero(epsilonOrCompare)) || (doEpsilon && realCompareLessThan(z, epsilonOrCompare));
    }

    realAdd(j, const_1, j, realContext);
    realDivide(t, j, t, realContext);
    tExp = max(tExp, realGetExponent(t));

    if(!endSin) {
      realCopy(sin, z);
      realAdd(sin, t, sin, realContext);
      if(doEpsilon) {
        realCopyAbs(t, z);
      }
      else {
        realCompare(sin, z, epsilonOrCompare, realContext);
      }
      endSin = (!doEpsilon && realIsZero(epsilonOrCompare)) || (doEpsilon && realCompareLessThan(z, epsilonOrCompare));
    }

    if(explicitTaylorIterVisibilitySelection && checkHalfSec()) {
      char ss[100];
      sprintf(ss, "Taylor Iter: %d/%d; Dig: %d/", i, TaylorIterationMax, -(int16_t)tExp);
      ss[40] = 0; //Hard limit to screen display
      #if defined(DEBUGTAYLOR)
        printf("%s%d\n", ss, epsilonDigits);
      #endif //DEBUGTAYLOR
      if(progressHalfSecUpdate_Integer(timed, ss, epsilonDigits, halfSec_clearZ, halfSec_clearT, halfSec_disp)) { //timed
      }
    }
    #if !defined(PC_BUILD)
      if(exitKeyWaiting()) {
        progressHalfSecUpdate_Integer(force+1, "Interrupted Iter:", i, halfSec_clearZ, halfSec_clearT, halfSec_disp);
        displayCalcErrorMessage(ERROR_SOLVER_ABORT, REGISTER_T);
        break;
      }
    #endif //!PC_BUILD

    #if defined(DEBUGTAYLOR)
      if(i > 1 && i % 1 == 0) { //left mod for printing interleaved status
        realToString(sin, tmpString);
        tmpString[80]=0;
        printf("Taylor progress: n=%3d, sin=%s", i, tmpString);
        realToString(cos, tmpString);
        tmpString[80]=0;
        printf(" cos=%s\n", tmpString);
      }
    #endif //DEBUGTAYLOR
  }

  if(realIsZero(cos)) {
    realSetPositiveSign(cos);
  }
  if(realIsZero(sin)) {
    realSetPositiveSign(sin);
  }
  realMultiply(sin, angle, sin, realContext);
  explicitTaylorIterVisibilitySelection = false;
}


// used by normal TRIG, used from externally from bessel.c
// Calculate sin, cos by Taylor series and tan by division
void C47_WP34S_SinCosTanTaylor_temp75(const real_t *a, bool_t swap, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext) { // a in radian
  bool_t doEpsilon = false;
  int   epsilonDigits;
  // The eight working reals come from the heap, not the frame: eight decNumbers at 60 bytes is 480 of this function's 672 byte frame, and it sits on the integrand path
  // of a plotted integral. The 1071 digit twin below is untouched.
  REAL_T_ALLOC(angle,            75);
  REAL_T_ALLOC(a2,               75);
  REAL_T_ALLOC(t,                75);
  REAL_T_ALLOC(j,                75);
  REAL_T_ALLOC(z,                75);
  REAL_T_ALLOC(sin,              75);
  REAL_T_ALLOC(cos,              75);
  REAL_T_ALLOC(epsilonOrCompare, 75);
  if(angle == NULL || a2 == NULL || t == NULL || j == NULL || z == NULL || sin == NULL || cos == NULL || epsilonOrCompare == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    goto freeWork;
  }

  int32_t savedContextDigits = realContext->digits;

  if(realContext->digits > 51) {
    realContext->digits = 75;
    epsilonDigits = 72;           //only applicable if doEpsilon is true, as well as in the iteration display
    doEpsilon = true;
  }
  else {
    realContext->digits = 51;
    epsilonDigits = 39;           //only applicable if doEpsilon is true, as well as in the iteration display
    doEpsilon = false;            //stay compaitble with the old Taylor
  }

  doTaylorIterations(a, angle, a2, t, j, z, sin, cos, sinOut, cosOut, epsilonOrCompare, doEpsilon, epsilonDigits, realContext);

  realContext->digits = savedContextDigits;

  if(sinOut != NULL) {
    realPlus(sin, sinOut, realContext);
  }

  if(cosOut != NULL) {
    realPlus(cos, cosOut, realContext);
  }

  if(tanOut != NULL) {
    if(sinOut == NULL || cosOut == NULL) {
      realSetNaN(tanOut);
    }
    else {
      if(swap) {
        realDivide(cos, sin, tanOut, realContext);
      }
      else {
        realDivide(sin, cos, tanOut, realContext);
      }
    }
  }

freeWork:
  REAL_T_FREE(angle,            75);
  REAL_T_FREE(a2,               75);
  REAL_T_FREE(t,                75);
  REAL_T_FREE(j,                75);
  REAL_T_FREE(z,                75);
  REAL_T_FREE(sin,              75);
  REAL_T_FREE(cos,              75);
  REAL_T_FREE(epsilonOrCompare, 75);
}


#if defined(OPTION_XFN_1000)
//used only by XFN 1071
static void C47_WP34S_Cvt2RadSinCosTan_1071_helper(const real_t *an, angularMode_t angularMode, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext) {
  bool_t sinNeg = false, cosNeg = false, swap = false;
  // One 1071 digit decNumber at 724 bytes, of this function's 752 byte frame. Freed on both exits through freeWork.
  REAL_T_ALLOC(angle, 1071);
  if(angle == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    goto freeWork;
  }

  if(realIsNaN(an)) {
    if(sinOut != NULL) {
     realSetNaN(sinOut);
    }
    if(cosOut != NULL) {
      realSetNaN(cosOut);
    }
    if(tanOut != NULL) {
      realSetNaN(tanOut);
    }
   goto freeWork;
  }

  realCopy(an, angle);

  int32_t savedContextDigits = realContext->digits;

  doWP34S_SinCosTanTaylor(angle, &sinNeg, &cosNeg, &swap, sinOut, cosOut, tanOut, angularMode, savedContextDigits, realContext);

freeWork:
  REAL_T_FREE(angle, 1071);
  }
#endif // OPTION_XFN_1000


void C47_WP34S_Cvt2RadSinCosTan(const real_t *an, angularMode_t angularMode, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext) {
  #if defined(OPTION_XFN_1000)
    if(realContext->digits >= 1071) {
      C47_WP34S_Cvt2RadSinCosTan_1071_helper(an, angularMode, sinOut, cosOut, tanOut, realContext);
    }
    else
  #endif // OPTION_XFN_1000
  {
    C47_WP34S_Cvt2RadSinCosTan_75_helper(an, angularMode, sinOut, cosOut, tanOut, realContext);
  }
}


#if defined(OPTION_XFN_1000)
//Used by normal C47 TRIG as well as XFN
// Calculate sin, cos by Taylor series and tan by division, for 1071 contexts
void C47_WP34S_SinCosTanTaylor_temp1071(const real_t *a, bool_t swap, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext) { // a in radian

  // The eight working reals come from the heap, not the frame: eight 1071 digit decNumbers at 724 bytes is 5792 of this function's 5832 byte frame, on an arm build with
  // OPTION_XFN_1000 forced on. That frame is the stack reason XFN 1000 is kept off the DM42, defines.h:36.
  REAL_T_ALLOC(angle,            1071);
  REAL_T_ALLOC(a2,               1071);
  REAL_T_ALLOC(t,                1071);
  REAL_T_ALLOC(j,                1071);
  REAL_T_ALLOC(z,                1071);
  REAL_T_ALLOC(sin,              1071);
  REAL_T_ALLOC(cos,              1071);
  REAL_T_ALLOC(epsilonOrCompare, 1071);
  if(angle == NULL || a2 == NULL || t == NULL || j == NULL || z == NULL || sin == NULL || cos == NULL || epsilonOrCompare == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    goto freeWork;
  }

  doTaylorIterations(a, angle, a2, t, j, z, sin, cos, sinOut, cosOut, epsilonOrCompare, true /*doEpsilon*/, 1040, realContext);

  if(sinOut != NULL) {
    realPlus(sin, sinOut, realContext);
  }
  if(cosOut != NULL) {
    realPlus(cos, cosOut, realContext);
  }
  if(tanOut != NULL) {
    if(sinOut == NULL || cosOut == NULL) {
      realSetNaN(tanOut);
    }
    else {
      if(swap) {
        realDivide(cos, sin, tanOut, realContext);
      }
      else {
        realDivide(sin, cos, tanOut, realContext);
      }
    }
  }

freeWork:
  REAL_T_FREE(angle,            1071);
  REAL_T_FREE(a2,               1071);
  REAL_T_FREE(t,                1071);
  REAL_T_FREE(j,                1071);
  REAL_T_FREE(z,                1071);
  REAL_T_FREE(sin,              1071);
  REAL_T_FREE(cos,              1071);
  REAL_T_FREE(epsilonOrCompare, 1071);
}
#endif // OPTION_XFN_1000


void C47_WP34S_SinCosTanTaylor(const real_t *a, bool_t swap, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext) { // a in radian
  #if defined(OPTION_XFN_1000)
    if(realContext->digits >= 1071) {
      C47_WP34S_SinCosTanTaylor_temp1071(a, swap, sinOut, cosOut, tanOut, realContext); // a in radian
    }
    else
  #endif // OPTION_XFN_1000
  {
    C47_WP34S_SinCosTanTaylor_temp75(a, swap, sinOut, cosOut, tanOut, realContext); // a in radian
  }
}


static bool_t doAtan(real_t *a, real_t *angle, real_t *a2, real_t *t, real_t *j, real_t *z, const real_t *x, real_t *b, real_t *epsilon, real_t *last, const bool_t doEpsilon, int epsilonDigits, int* doubles, int* invert, int* neg, realContext_t *realContext){
  bool_t conditionToKeepIterating = false;
  char tmpEpsilon[16]; //-- for epsilon string conversion
  //-- use epsilon convergence instead of exact equality
  if(doEpsilon) {
    sprintf(tmpEpsilon, "1E-%d", epsilonDigits);
    stringToReal(tmpEpsilon, epsilon, realContext);
    //-- create const_1on10 equivalent for 1071 precision, temporary use of z - up to for loop below
    //uInt32ToReal(10, z);
    //realDivide(const_1, z, z, realContext);
  }


  *neg = realIsNegative(x);

  if(realIsNaN(x)) {
    realSetNaN(angle);
    return false;
  }

  realCopy(x, a);

  // arrange for a >= 0
  if(*neg) {
    realChangeSign(a);
  }

  // reduce range to 0 <= a < 1, using atan(x) = pi/2 - atan(1/x)
  *invert = realCompareGreaterThan(a, const_1);
  if(*invert) {
    realDivide(const_1, a, a, realContext);
  }

  // Range reduce to small enough limit to use taylor series using:
  //  tan(x/2) = tan(x)/(1+sqrt(1+tan(x)²))
  for(int n=0; n<TaylorIterationMax; n++) {
    if(!doEpsilon && realCompareLessEqual(a, const_1on10)) {
      break;
    }
    else if(doEpsilon && realCompareLessEqual(a, const_1on10)){//  z)) {
      break;
    }

    (*doubles)++;
    // a = a/(1+sqrt(1+a²)) -- at most 3 iterations.
    realMultiply(a, a, b, realContext);
    realAdd(b, const_1, b, realContext);
    realSquareRoot(b, b, realContext);
    realAdd(b, const_1, b, realContext);
    realDivide(a, b, a, realContext);
  }

  // Now Taylor series
  // atan(x) = x(1-x²/3+x⁴/5-x⁶/7...)
  // We calculate pairs of terms and stop when the estimate doesn't change
  uInt32ToReal(3, angle);
  uInt32ToReal(5, j);
  realMultiply(a, a, a2, realContext); // a²
  realCopy(a2, t);
  realDivide(t, angle, angle, realContext); // s = 1-t/3 -- first two terms
  realSubtract(const_1, angle, angle, realContext);

  int i = 0;
  do { // Loop until there is no digits changed
    realCopy(angle, last);

    realMultiply(t, a2, t, realContext);
    realDivide(t, j, z, realContext);
    realAdd(angle, z, angle, realContext);

    realAdd(j, const_2, j, realContext);

    realMultiply(t, a2, t, realContext);
    realDivide(t, j, z, realContext);
    realSubtract(angle, z, angle, realContext);

    realAdd(j, const_2, j, realContext);

    if(doEpsilon) {
      //-- use epsilon convergence test instead of exact equality
      realSubtract(angle, last, b, realContext);
      realCopyAbs(b, b);
      realSubtract(b, epsilon, b, realContext);
      conditionToKeepIterating = realIsPositive(b);
    }
     else {
      realSubtract(angle, last, b, realContext);
      realPlus(b, b, realContext);
      conditionToKeepIterating = !realIsZero(b);
    }


    if(explicitTaylorIterVisibilitySelection && checkHalfSec()) {
      char ss[100];
      sprintf(ss, "Taylor Iter: %d/%d; Dig: %d/", i, TaylorIterationMax, -(int16_t)realGetExponent(b));
      ss[40] = 0; //Hard limit to screen display
      #if defined(DEBUGTAYLOR)
        printf("%s%d\n", ss, epsilonDigits);
      #endif //DEBUGTAYLOR
      if(progressHalfSecUpdate_Integer(timed, ss, epsilonDigits, halfSec_clearZ, halfSec_clearT, halfSec_disp)) { //timed
      }
    }
    #if !defined(PC_BUILD)
      if(exitKeyWaiting()) {
        progressHalfSecUpdate_Integer(force+1, "Interrupted Iter:", i, halfSec_clearZ, halfSec_clearT, halfSec_disp);
        displayCalcErrorMessage(ERROR_SOLVER_ABORT, REGISTER_T);
        break;
      }
    #endif //!PC_BUILD


    #if defined(DEBUGTAYLOR)
      if(i > 1 && i % 1 == 0) { //left mod for printing interleaved status
        realToString(angle, tmpString);
        tmpString[80]=0;
        printf("Taylor progress Atan: n=%3d, angle=%s", i, tmpString);
        realToString(b , tmpString);
        tmpString[80]=0;
        printf(" diff=%s\n", tmpString);
      }
    #endif //DEBUGTAYLOR
    i++;

  } while(conditionToKeepIterating && i < TaylorIterationMax);

  realMultiply(angle, a, angle, realContext);

  while(*doubles) {
    realAdd(angle, angle, angle, realContext);
    (*doubles)--;
  }
  if(*invert) {
    realSubtract(((realContext->digits) > 51) ? const1071_piOn2 : const39_piOn2, angle, angle, realContext);
  }
  if(*neg) {
    realChangeSign(angle);
  }
  return true;
}


// ---------------------------------------------------------------------------
// Generic cache helpers for 1-input and 2-input trig wrappers
//
// Each cache is a single most-recent-call slot. The stored result is treated as
// a pure function of the key: the input value(s), the effective compute
// precision, and the rounding mode. Nothing else in realContext (emax/emin/
// clamp) changes an angle result, whose magnitude is always O(1), and the
// angular-mode conversion happens in the caller AFTER the cache, so the angular
// mode is deliberately not part of the key.
//
// The caches are global single slots. cache*_call therefore writes the key and
// the result together AFTER computing, never a bare key before: a reentrant
// call (a display refresh computing another angle mid-computation, say) can only
// leave its OWN complete, correct pair, which the outer call then overwrites
// with its own - there is no window where a stale key and a fresh result coexist.
// This ordering is the reason the wrappers are safe without any in-progress flag
// or scheduling assumption; do not move cache_prepare before compute. Build with
// CACHE_VERIFY to recompute on every hit and abort on any mismatch, which turns
// the "key is complete" assumption into a checked one.
// ---------------------------------------------------------------------------
typedef struct { bool_t valid; int32_t digits; int32_t round; real_t x, result;    } cache1_t;
typedef struct { bool_t valid; int32_t digits; int32_t round; real_t y, x, result; } cache2_t;

typedef void (*trig1Compute_t)(const real_t *x, real_t *out, realContext_t *ctx);
typedef void (*trig2Compute_t)(const real_t *y, const real_t *x, real_t *out, realContext_t *ctx);

static void cache_commit(bool_t *valid, real_t *cacheResult, const real_t *result) {
  if(!realIsSpecial(result)) { realCopy(result, cacheResult); *valid = true; }
}

// zero sign is part of the key
static bool_t cacheKeyEqual(const real_t *a, const real_t *b) {
  return realCompareIdentical(a, b);
}

#if defined(CACHE_VERIFY)
  static void cacheVerifyFail(const char *which) {
    // stderr is unbuffered: abort() would drop a buffered stdout line.
    fprintf(stderr, "CACHE_VERIFY_MISMATCH in %s: a cache hit disagreed with a fresh recompute\n", which);
    fflush(stderr);
    abort();
  }
#endif // CACHE_VERIFY

// digits is the effective compute precision, not the request
static bool_t cache1_check(cache1_t *c, const real_t *x, real_t *result, int32_t digits, realContext_t *ctx) {
  if(c->valid && digits == c->digits && (int32_t)ctx->round == c->round && cacheKeyEqual(&c->x, x)) {
    realCopy(&c->result, result);
    return true;
  }
  return false;
}
static void cache1_prepare(cache1_t *c, const real_t *x, int32_t digits, realContext_t *ctx) {
  realCopy(x, &c->x);
  c->digits = digits;
  c->round  = (int32_t)ctx->round;
  c->valid  = false;
}

static bool_t cache2_check(cache2_t *c, const real_t *y, const real_t *x, real_t *result, int32_t digits, realContext_t *ctx) {
  if(c->valid && digits == c->digits && (int32_t)ctx->round == c->round && cacheKeyEqual(&c->y, y) && cacheKeyEqual(&c->x, x)) {
    realCopy(&c->result, result);
    return true;
  }
  return false;
}
static void cache2_prepare(cache2_t *c, const real_t *y, const real_t *x, int32_t digits, realContext_t *ctx) {
  realCopy(y, &c->y);
  realCopy(x, &c->x);
  c->digits = digits;
  c->round  = (int32_t)ctx->round;
  c->valid  = false;
}

// One cached 1-input trig call. Copies the input first (so `compute` is safe even
// when the caller aliases in==out), returns a hit, otherwise computes and stores
// key+result together (see the ordering note above). Under CACHE_VERIFY every hit
// is recomputed and compared, so an incomplete key aborts loudly instead of
// returning a stale value.
static void cache1_call(cache1_t *c, trig1Compute_t compute, const char *name, const real_t *x, real_t *out, int32_t effDigits, realContext_t *ctx) {
  real_t localX;
  realCopy(x, &localX);
  if(cache1_check(c, &localX, out, effDigits, ctx)) {
    #if defined(CACHE_DEBUG)
      printf("   %s: quick return for repeated value\n", name);
    #endif // CACHE_DEBUG
    #if defined(CACHE_VERIFY)
      real_t verify;
      compute(&localX, &verify, ctx);
      if(!cacheKeyEqual(&verify, out)) { cacheVerifyFail(name); }
    #endif // CACHE_VERIFY
    return;
  }
  #if defined(CACHE_DEBUG)
    printf("%s: long process calc\n", name);
  #endif // CACHE_DEBUG
  compute(&localX, out, ctx);                 // compute first: any nested call leaves its own complete pair
  cache1_prepare(c, &localX, effDigits, ctx); // then stamp this call's key
  cache_commit(&c->valid, &c->result, out);   // and result, together
  #if !defined(CACHE_DEBUG) && !defined(CACHE_VERIFY)
    (void)name;
  #endif
}

static void cache2_call(cache2_t *c, trig2Compute_t compute, const char *name, const real_t *y, const real_t *x, real_t *out, int32_t effDigits, realContext_t *ctx) {
  real_t localY, localX;
  realCopy(y, &localY);
  realCopy(x, &localX);
  if(cache2_check(c, &localY, &localX, out, effDigits, ctx)) {
    #if defined(CACHE_DEBUG)
      printf("   %s: quick return for repeated value\n", name);
    #endif // CACHE_DEBUG
    #if defined(CACHE_VERIFY)
      real_t verify;
      compute(&localY, &localX, &verify, ctx);
      if(!cacheKeyEqual(&verify, out)) { cacheVerifyFail(name); }
    #endif // CACHE_VERIFY
    return;
  }
  #if defined(CACHE_DEBUG)
    printf("%s: long process calc\n", name);
  #endif // CACHE_DEBUG
  compute(&localY, &localX, out, ctx);                 // compute first (see cache1_call)
  cache2_prepare(c, &localY, &localX, effDigits, ctx); // then stamp key
  cache_commit(&c->valid, &c->result, out);            // and result, together
  #if !defined(CACHE_DEBUG) && !defined(CACHE_VERIFY)
    (void)name;
  #endif
}


// The precision atan actually computes at. Requests above 39 go to the Taylor path at 75.
// At or below 39 the table path honours the request plus a couple of guard digits: about a
// dozen chained operations cost roughly one digit of accumulated rounding, measured as a
// steady 0.8 to 1.0 digits at every precision from 3 up to 39, so computing two beyond the
// request returns what the caller asked for with a digit to spare. The cap at 39 is why
// atan has always returned 38 correct digits to a 39 digit caller, table path or not.
//
// No lower floor: the loss is proportional, not a cliff, so a caller asking for little
// gets little quickly. Graph plotting narrows ctxtReal39 to significantDigits + 3, as low
// as 4, and pays for 6 rather than for 39.
//
// Both compute functions and the cache wrapper must agree on this, or the cache could hand
// a low precision result to a high precision request.
#define ATAN_GUARD_DIGITS 2
static int32_t atanEffectiveDigits(int32_t requestedDigits) {
  if(requestedDigits > 39) {
    return 75;
  }
  requestedDigits += ATAN_GUARD_DIGITS;
  return (requestedDigits > 39) ? 39 : requestedDigits;
}


// atan(j/10) for j = 1..9; the ends are const_0 and const39_piOn4. The step must divide a
// power of ten so that j/step is exact at every working precision: a tenth is, 1/12 is not,
// and its rounding would break the identity the reduction rests on.
#define ATAN_TABLE_STEP 10
static const real_t * const atanTable[ATAN_TABLE_STEP + 1] = {
  const_0,            const39_atan1on10,  const39_atan2on10,  const39_atan3on10,
  const39_atan4on10,  const39_atan5on10,  const39_atan6on10,  const39_atan7on10,
  const39_atan8on10,  const39_atan9on10,  const39_piOn4
};

// P(v) with v = u*u, magnitudes only (the loop below negates v instead of tracking signs).
// Eight of the thirteen were constrained to the Taylor coefficients +-1/(2k+1) and only
// atanP08..12 left free for the fit. Eight constraints still land at 2.9e-41 against a
// 1e-40 budget, so they cost no extra term; a ninth would not fit. Holding the first at
// exactly 1 is what makes atan(x) come out as exactly x for tiny x rather than merely
// rounding to it.
#define ATAN_COEFFICIENTS 13
static const real_t * const atanPoly[ATAN_COEFFICIENTS] = {
  const_1,          const39_1on3,     const_1on5,       const39_1on7,
  const39_1on9,     const39_1on11,    const39_1on13,    const39_1on15,
  const39_atanP08,  const39_atanP09,  const39_atanP10,  const39_atanP11,
  const39_atanP12
};


/********************************************//**
 * \brief atan for requests at or below 39 digits
 *
 * Reduces through the addition formula against a tenth spaced table
 *
 *     a = |x|, and a > 1 is inverted by atan(a) = pi/2 - atan(1/a)
 *     j = round(10a),  c = j/10,  u = (a - c)/(1 + a*c)   so |u| <= 1/20
 *     atan(a) = atanTable[j] + u * P(u*u)
 *
 * which costs one division where the Taylor path spent three square roots, and then
 * finishes with a fixed length polynomial instead of a convergence loop. Horner starts
 * below the top term once u is small enough that the leading terms cannot reach the
 * requested precision, so small arguments stay cheap.
 *
 * Truncation error is 2.9e-41 as the coefficients are stored, an order of magnitude under
 * the 5e-40 the 39 digit table constants contribute anyway, so the polynomial is not what
 * limits the answer. At 39 digits this returns about 38 correct digits, a little better
 * than the Taylor path it replaces.
 ***********************************************/
static void WP34S_Atan_table_compute(const real_t *x, real_t *angle, realContext_t *realContext) {
  REAL_T_ALLOC(a, 39);   // also the accumulator further down
  REAL_T_ALLOC(u, 39);
  REAL_T_ALLOC(v, 39);
  REAL_T_ALLOC(c, 39);
  bool_t neg, invert;
  int32_t j, k, top, exponent, savedContextDigits;

  // The four 39 digit heap reals are smaller than the declared real_t. GCC's bounds checker gives warning; example fix from graph.c
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Warray-bounds"

  if(a == NULL || u == NULL || v == NULL || c == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    goto freeWork;
  }

  if(realIsNaN(x)) {
    realSetNaN(angle);
    goto freeWork;
  }

  savedContextDigits  = realContext->digits;
  realContext->digits = atanEffectiveDigits(savedContextDigits);

  neg = realIsNegative(x);                        // kept: the result is negated again at the end
  realCopyAbs(x, a);

  invert = realCompareGreaterThan(a, const_1);
  if(invert) {
    realDivide(const_1, a, a, realContext);   // 1/inf is 0, which lands on j = 0 and gives pi/2
  }

  // j = round(10a), always in [0, 10] and used directly as the index into atanTable. c is
  // borrowed for the scaled copy here and holds the table centre from the next block on.
  // Multiplying by ten in a decimal representation is nothing but a shift of the exponent,
  // so the scaling is free and exact; the rounding is left to realToIntegralValue. a is |x|
  // with anything above 1 inverted, and rounding 1/a can reach 1 but never pass it, so 10a
  // stays inside [0, 10] and no clamp is needed. realToInt32C47 cannot break that either:
  // every one of its failure paths returns 0.
  realCopy(a, c);
  c->exponent++;
  realToIntegralValue(c, c, DEC_ROUND_HALF_UP, realContext);
  j = realToInt32C47(c, NULL);

  // c = j/10, exact for the same reason and for j = 0 too, since a zero stays a zero
  int32ToReal(j, c);
  c->exponent--;
  realMultiply(a, c, v, realContext);
  realAdd(v, const_1, v, realContext);
  realSubtract(a, c, u, realContext);
  realDivide(u, v, u, realContext);

  realMultiply(u, u, v, realContext);        // v = u*u

  // v^k drops below the last digit being computed once k * |log10 v| passes that many
  // digits, so every coefficient above that index is dead weight. The + 6 keeps the cut a
  // few digits clear of the rounding noise, at the cost of at most one extra term.
  top = ATAN_COEFFICIENTS - 1;
  if(realIsZero(v)) {
    top = 0;
  }
  else {
    exponent = -realGetExponent(v);
    if(exponent > 0) {
      int32_t needed = (realContext->digits + 6 + exponent - 1) / exponent;
      if(needed < top) {
        top = needed;
      }
    }
  }

  // The series alternates: c0 - c1*v + c2*v^2 - ... = c0 - v*(c1 - v*(c2 - ...)). Negating v
  // turns every Horner step into the same acc*(-v) + c operation, so atanPoly can hold plain
  // magnitudes and neither the loop nor its seed needs to track the sign.
  realChangeSign(v);
  realCopy(atanPoly[top], a);
  for(k = top - 1; k >= 0; k--) {
    realMultiply(a, v, a, realContext);
    realAdd(a, atanPoly[k], a, realContext);
  }
  realMultiply(a, u, a, realContext);
  realAdd(a, atanTable[j], angle, realContext);

  if(invert) {
    realSubtract(const39_piOn2, angle, angle, realContext);
  }
  if(neg) {
    realChangeSign(angle);
  }

  realContext->digits = savedContextDigits;
  #pragma GCC diagnostic pop

freeWork:
  REAL_T_FREE(a, 39);
  REAL_T_FREE(u, 39);
  REAL_T_FREE(v, 39);
  REAL_T_FREE(c, 39);
}


// The Taylor series with sqrt halving, now reached only for requests above 39 digits.
static void WP34S_Atan_75_compute(const real_t *x, real_t *angle, realContext_t *realContext) {
  bool_t doEpsilon = false;
  // The eight working reals come from the heap, not the frame: 480 of this function's 656 bytes, on the integrand path of a plotted integral. The 1071 digit twin below
  // keeps its stack buffers untouched.
  REAL_T_ALLOC(a,       75);
  REAL_T_ALLOC(b,       75);
  REAL_T_ALLOC(a2,      75);
  REAL_T_ALLOC(t,       75);
  REAL_T_ALLOC(j,       75);
  REAL_T_ALLOC(z,       75);
  REAL_T_ALLOC(last,    75);
  REAL_T_ALLOC(epsilon, 75);   //-- added epsilon for convergence;
  int doubles = 0;
  int invert;
  int neg;
  int32_t savedContextDigits = realContext->digits;
  int epsilonDigits;

  if(realContext->digits > 39) {
    realContext->digits = 75;
    epsilonDigits = 72;           //only applicable if doEpsilon is true, as well as in the iteration display
    doEpsilon = true;
  }
  else {
    realContext->digits = 39;
    epsilonDigits = 39;           //only applicable if doEpsilon is true, as well as in the iteration display
    doEpsilon = false;            //stay compaitble with the old Taylor
  }

  if(a == NULL || b == NULL || a2 == NULL || t == NULL || j == NULL || z == NULL || last == NULL || epsilon == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    realContext->digits = savedContextDigits;
    goto freeWork;
  }

  if(!doAtan( a, angle, a2, t, j, z, x, b, epsilon, last,
              doEpsilon, epsilonDigits,
              &doubles, &invert, &neg,
              realContext)) {
    realContext->digits = savedContextDigits;
    goto freeWork; //NaN
  }
  realContext->digits = savedContextDigits;

freeWork:
  REAL_T_FREE(a,       75);
  REAL_T_FREE(b,       75);
  REAL_T_FREE(a2,      75);
  REAL_T_FREE(t,       75);
  REAL_T_FREE(j,       75);
  REAL_T_FREE(z,       75);
  REAL_T_FREE(last,    75);
  REAL_T_FREE(epsilon, 75);
}


// Cached wrapper for WP34S_Atan. Returns the previous result when the input,
// effective precision and rounding mode all match; see cache1_call.
static cache1_t atanCache;
static void WP34S_Atan_75_helper(const real_t *x, real_t *angle, realContext_t *realContext) {
  #if defined(CACHE_DEBUG)
    print_caller("WP34S_Atan");             // traced here, not in cache1_call: print_caller reports its own caller
  #endif // CACHE_DEBUG
  int32_t effDigits = atanEffectiveDigits(realContext->digits); // precision actually computed at, not the request
  cache1_call(&atanCache,
              (effDigits > 39) ? WP34S_Atan_75_compute : WP34S_Atan_table_compute,
              "WP34S_Atan", x, angle, effDigits, realContext);
}


#if defined(OPTION_XFN_1000)
static void C47do_WP34S_Atan_1071_helper(const real_t *x, real_t *angle, realContext_t *realContext) {
  // The eight working reals come from the heap, not the frame: eight 1071 digit decNumbers at 724 bytes is 5792 of this function's 5864 byte frame, the largest frame in
  // the build. Freed on both exits through freeWork.
  REAL_T_ALLOC(a,       1071);
  REAL_T_ALLOC(b,       1071);
  REAL_T_ALLOC(a2,      1071);
  REAL_T_ALLOC(t,       1071);
  REAL_T_ALLOC(j,       1071);
  REAL_T_ALLOC(z,       1071);
  REAL_T_ALLOC(last,    1071);
  REAL_T_ALLOC(epsilon, 1071);
  if(a == NULL || b == NULL || a2 == NULL || t == NULL || j == NULL || z == NULL || last == NULL || epsilon == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    goto freeWork;
  }
  int doubles = 0;
  int invert;
  int neg;
  if(!doAtan( a, angle, a2, t, j, z, x, b, epsilon, last, true, 1040, &doubles, &invert, &neg, realContext)) {
    goto freeWork; //NaN
  }

freeWork:
  REAL_T_FREE(a,       1071);
  REAL_T_FREE(b,       1071);
  REAL_T_FREE(a2,      1071);
  REAL_T_FREE(t,       1071);
  REAL_T_FREE(j,       1071);
  REAL_T_FREE(z,       1071);
  REAL_T_FREE(last,    1071);
  REAL_T_FREE(epsilon, 1071);
}
#endif // OPTION_XFN_1000


void C47_WP34S_Atan(const real_t *x, real_t *angle, realContext_t *realContext) {
  #if defined(OPTION_XFN_1000)
    if(realContext->digits >= 1071) {
      C47do_WP34S_Atan_1071_helper(x, angle, realContext);
    }
    else
  #endif // OPTION_XFN_1000
  {
    WP34S_Atan_75_helper(x, angle, realContext);
  }
}


#define _pi(d)     (d > 51 ? (d > 75 ? const1071_pi     : const75_pi)     : const39_pi)
#define _piOn2(d)  (d > 51 ? (d > 75 ? const1071_piOn2  : const75_piOn2)  : const39_piOn2)
#define _piOn4(d)  (d > 51 ? (d > 75 ? const1071_piOn4  : const75_piOn4)  : const39_piOn4)
#define _3piOn4(d) (d > 51 ? (d > 75 ? const1071_3piOn4 : const75_3piOn4) : const39_3piOn4)

static bool_t doAtan2(const real_t *y, const real_t *x, real_t *atan, real_t *r, real_t *t, realContext_t *realContext) {
  const bool_t xNeg = realIsNegative(x);
  const bool_t yNeg = realIsNegative(y);

  if(realIsNaN(x) || realIsNaN(y)) {
    realSetNaN(atan);
    return false;
  }

  if(realCompareEqual(y, const_0)) {
    if(yNeg) {
      if(realCompareEqual(x, const_0)) {
        if(xNeg) {
          realMinus( _pi(realContext->digits), atan, realContext);
        }
        else {
          realCopy(y, atan);
        }
      }
      else if(xNeg) {
        realMinus( _pi(realContext->digits), atan, realContext);
      }
      else {
        realCopy(y, atan);
      }
    }
    else {
      if(realCompareEqual(x, const_0)) {
        if(xNeg) {
          realCopy( _pi(realContext->digits), atan);
        }
        else {
          realSetZero(atan);
        }
      }
      else if(xNeg) {
        realCopy( _pi(realContext->digits), atan);
      }
      else {
        realSetZero(atan);
      }
    }
    return true;
  }

  if(realCompareEqual(x, const_0)) {
    realCopy( _piOn2(realContext->digits), atan);
    if(yNeg) {
      realSetNegativeSign(atan);
    }
    return true;
  }

  if(realIsInfinite(x)) {
    if(xNeg) {
      if(realIsInfinite(y)) {
        realCopy( _3piOn4(realContext->digits), atan);
        if(yNeg) {
          realSetNegativeSign(atan);
        }
      }
      else {
        realCopy( _pi(realContext->digits), atan);
        if(yNeg) {
          realSetNegativeSign(atan);
        }
      }
    }
    else {
      if(realIsInfinite(y)) {
        realCopy( _piOn4(realContext->digits), atan);
        if(yNeg) {
          realSetNegativeSign(atan);
        }
      }
      else {
        realSetZero(atan);
        if(yNeg) {
          realSetNegativeSign(atan);
        }
      }
    }
    return true;
  }

  if(realIsInfinite(y)) {
    realCopy( _piOn2(realContext->digits), atan);
    if(yNeg) {
      realSetNegativeSign(atan);
    }
    return true;
  }

  realDivide(y, x, t, realContext);
  C47_WP34S_Atan(t, r, realContext);
  if(xNeg) {
    realCopy( _pi(realContext->digits), t);
    if(yNeg) {
     realSetNegativeSign(t);
    }
  }
  else {
    realSetZero(t);
  }

  realAdd(r, t, atan, realContext);
  if(realCompareEqual(atan, const_0) && yNeg) {
    realSetNegativeSign(atan);
  }
  return true;
}


static void WP34S_Atan2_75_compute(const real_t *y, const real_t *x, real_t *atan, realContext_t *realContext) {
  real_t r, t;
  int32_t savedContextDigits = realContext->digits;
  if(realContext->digits > 75) {
    realContext->digits = 75;
  }
  if(!doAtan2(y, x, atan, &r, &t, realContext)) {
    realContext->digits = savedContextDigits;
    return; //NaN
  }
  realContext->digits = savedContextDigits;
}


// Cached wrapper for WP34S_Atan2. Returns the previous result when both inputs,
// the effective precision and rounding mode match; see cache2_call.
static cache2_t atan2Cache;
static void WP34S_Atan2_75_helper(const real_t *y, const real_t *x, real_t *atan, realContext_t *realContext) {
  #if defined(CACHE_DEBUG)
    print_caller("WP34S_Atan2");            // traced here, not in cache2_call: print_caller reports its own caller
  #endif // CACHE_DEBUG
  int32_t effDigits = (realContext->digits > 75) ? 75 : realContext->digits; // precision WP34S_Atan2_75_compute computes at
  cache2_call(&atan2Cache, WP34S_Atan2_75_compute, "WP34S_Atan2", y, x, atan, effDigits, realContext);
}


#if defined(OPTION_XFN_1000)
static void C47do_WP34S_Atan2_1071_helper(const real_t *y, const real_t *x, real_t *atan, realContext_t *realContext) {
  // Two 1071 digit decNumbers at 724 bytes is 1448 of this function's 1464 byte frame. Freed on both exits through freeWork.
  REAL_T_ALLOC(r, 1071);
  REAL_T_ALLOC(t, 1071);
  if(r == NULL || t == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    goto freeWork;
  }
  if(!doAtan2(y, x, atan, r, t, realContext)) {
    goto freeWork; //NaN
  }

freeWork:
  REAL_T_FREE(r, 1071);
  REAL_T_FREE(t, 1071);
}
#endif // OPTION_XFN_1000


void C47_WP34S_Atan2(const real_t *y, const real_t *x, real_t *atan, realContext_t *realContext) {
  #if defined(OPTION_XFN_1000)
    if(realContext->digits >= 1071) {
      C47do_WP34S_Atan2_1071_helper(y, x, atan, realContext);
    }
    else
  #endif // OPTION_XFN_1000
  {
    WP34S_Atan2_75_helper(y, x, atan, realContext);
  }
}


static bool_t doAsin(const real_t *x, real_t *angle, real_t *abx, real_t *z, realContext_t *realContext) {
  if(realIsNaN(x)) {
    realSetNaN(angle);
    return false;
  }
  realCopyAbs(x, abx);
  if(realCompareGreaterThan(abx, const_1)) {
    realSetNaN(angle);
    return false;
  }
  // angle = 2*atan(x/(1+sqrt(1-x*x)))
  realSubtract(const_1, abx, z, realContext);     // 1-x*x as (1-|x|)(1+|x|): both factors are exact, where 1-x*x cancels near |x| = 1
  realAdd(const_1, abx, abx, realContext);
  realMultiply(z, abx, z, realContext);
  realSquareRoot(z, z, realContext);
  realAdd(z, const_1, z, realContext);
  realDivide(x, z, z, realContext);
  C47_WP34S_Atan(z, abx, realContext);
  realAdd(abx, abx, angle, realContext);
  return true;
}


static void WP34S_Asin_75_compute(const real_t *x, real_t *angle, realContext_t *realContext) {
  real_t abx, z;
  int32_t savedContextDigits = realContext->digits;
  if(realContext->digits > 75) {
    realContext->digits = 75;
  }
  if(!doAsin(x, angle, &abx, &z, realContext)) {
    realContext->digits = savedContextDigits;
    return; //NaN
  }
  realContext->digits = savedContextDigits;
}


// Cached wrapper for WP34S_Asin. See cache1_call.
static cache1_t asinCache;
static void WP34S_Asin_75_helper(const real_t *x, real_t *angle, realContext_t *realContext) {
  #if defined(CACHE_DEBUG)
    print_caller("WP34S_Asin");             // traced here, not in cache1_call: print_caller reports its own caller
  #endif // CACHE_DEBUG
  int32_t effDigits = (realContext->digits > 75) ? 75 : realContext->digits; // precision WP34S_Asin_75_compute computes at
  cache1_call(&asinCache, WP34S_Asin_75_compute, "WP34S_Asin", x, angle, effDigits, realContext);
}


#if defined(OPTION_XFN_1000)
static void C47do_WP34S_Asin_1071_helper(const real_t *x, real_t *angle, realContext_t *realContext) {
  // Two 1071 digit decNumbers at 724 bytes is 1448 of this function's 1464 byte frame. Freed on both exits through freeWork.
  REAL_T_ALLOC(abx, 1071);
  REAL_T_ALLOC(z,   1071);
  if(abx == NULL || z == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    goto freeWork;
  }
  if(!doAsin(x, angle, abx, z, realContext)) {
    goto freeWork; //NaN
  }

freeWork:
  REAL_T_FREE(abx, 1071);
  REAL_T_FREE(z,   1071);
}
#endif // OPTION_XFN_1000


void C47_WP34S_Asin(const real_t *x, real_t *angle, realContext_t *realContext) {
  #if defined(OPTION_XFN_1000)
    if(realContext->digits >= 1071) {
      C47do_WP34S_Asin_1071_helper(x, angle, realContext);
    }
    else
  #endif // OPTION_XFN_1000
  {
    WP34S_Asin_75_helper(x, angle, realContext);
  }
}



static bool_t doAcos(const real_t *x, real_t *angle, real_t *abx, real_t *z, realContext_t *realContext) {
  if(realIsNaN(x)) {
    realSetNaN(angle);
    return false;
  }
  realCopyAbs(x, abx);
  if(realCompareGreaterThan(abx, const_1)) {
    realSetNaN(angle);
    return false;
  }
  // angle = 2*atan((1-x)/sqrt(1-x*x))
  if(realCompareEqual(x, const_1)) {
    realSetZero(angle);
  }
  else {
    realSubtract(const_1, abx, z, realContext);   // 1-x*x as (1-|x|)(1+|x|): both factors are exact, where 1-x*x cancels near |x| = 1
    realAdd(const_1, abx, abx, realContext);
    realMultiply(z, abx, z, realContext);
    realSquareRoot(z, z, realContext);
    realSubtract(const_1, x, abx, realContext);
    realDivide(abx, z, z, realContext);
    C47_WP34S_Atan(z, abx, realContext);
    realAdd(abx, abx, angle, realContext);
  }
  return true;
}


static void WP34S_Acos_75_compute(const real_t *x, real_t *angle, realContext_t *realContext) {
  real_t abx, z;
  int32_t savedContextDigits = realContext->digits;
  if(realContext->digits > 75) {
    realContext->digits = 75;
  }
  if(!doAcos(x, angle, &abx, &z, realContext)) {
    realContext->digits = savedContextDigits;
    return; //NaN
  }
  realContext->digits = savedContextDigits;
}


// Cached wrapper for WP34S_Acos. See cache1_call.
static cache1_t acosCache;
static void WP34S_Acos_75_helper(const real_t *x, real_t *angle, realContext_t *realContext) {
  #if defined(CACHE_DEBUG)
    print_caller("WP34S_Acos");             // traced here, not in cache1_call: print_caller reports its own caller
  #endif // CACHE_DEBUG
  int32_t effDigits = (realContext->digits > 75) ? 75 : realContext->digits; // precision WP34S_Acos_75_compute computes at
  cache1_call(&acosCache, WP34S_Acos_75_compute, "WP34S_Acos", x, angle, effDigits, realContext);
}


#if defined(OPTION_XFN_1000)
static void C47do_WP34S_Acos_1071_helper(const real_t *x, real_t *angle, realContext_t *realContext) {
  // Two 1071 digit decNumbers at 724 bytes is 1448 of this function's 1464 byte frame. Freed on both exits through freeWork.
  REAL_T_ALLOC(abx, 1071);
  REAL_T_ALLOC(z,   1071);
  if(abx == NULL || z == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    goto freeWork;
  }
  if(!doAcos(x, angle, abx, z, realContext)) {
    goto freeWork; //NaN
  }

freeWork:
  REAL_T_FREE(abx, 1071);
  REAL_T_FREE(z,   1071);
}
#endif // OPTION_XFN_1000


void C47_WP34S_Acos(const real_t *x, real_t *angle, realContext_t *realContext) {
  #if defined(OPTION_XFN_1000)
    if(realContext->digits >= 1071) {
      C47do_WP34S_Acos_1071_helper(x, angle, realContext);
    }
    else
  #endif // OPTION_XFN_1000
  {
    WP34S_Acos_75_helper(x, angle, realContext);
  }
}



static void WP34S_Calc_Gamma_LnGamma_Lanczos(const real_t *xin, real_t *res, bool_t calculateLnGamma, realContext_t *realContext) {
  real_t r, s, t, u, v, x;
  int32_t k, savedContextDigits;

  savedContextDigits = realContext->digits;
  if(realContext->digits < 51) {
    realContext->digits = 51;
  }

  realSubtract(xin, const_1, &x, realContext);
  realSetZero(&s);
  realAdd(&x, const_29, &t, realContext);
  for(k=28; k>=0; k--) {
    realDivide((real_t *)(((uint8_t *)const51_gammaC01) + k*REAL_SIZE_IN_BYTES(51)), &t, &u, realContext);
    realSubtract(&t, const_1, &t, realContext);
    realAdd(&s, &u, &s, realContext);
  }

  realAdd(&s, const51_gammaC00, &t, realContext);
  WP34S_Ln(&t, &s, realContext);

  //  r = z + g + 0.5;
  realAdd(&x, const_gammaR, &r, realContext); // const_gammaR is g + 0.5

  //  r = log(R[0][0]) + (z+0.5) * log(r) - r;
  WP34S_Ln(&r, &u, realContext);
  realAdd(&x, const_1on2, &t, realContext);
  realMultiply(&u, &t, &v, realContext);

  realSubtract(&v, &r, &u, realContext);

  if(calculateLnGamma) {
    realAdd(&u, &s, &x, realContext);
  }
  else {
    realAdd(&u, &s, &x, realContext);
    realExp(&x, &x, realContext);
  }

  realContext->digits = savedContextDigits;

  realPlus(&x, res, realContext);
}


// common code for the [GAMMA] and LN[GAMMA]
static void WP34S_Gamma_LnGamma(const real_t *xin, const bool_t calculateLnGamma, real_t *res, realContext_t *realContext) {
  real_t x, t;
  bool_t reflect = false;

  // Check for special cases
  if(realIsSpecial(xin)) {
    if(realIsInfinite(xin) && realIsPositive(xin)) {
      realSetPlusInfinity(res);
      return;
    }

    realSetNaN(res);
    return;
  }

  // Handle x approximately zero case
  if(realCompareAbsLessThan(xin, const_1e_24)) {
    if(realIsZero(xin)) {
      realSetNaN(res);
      return;
    }
    realDivide(const_1, xin, &x, realContext);
    realSubtract(&x, const39_egamma, res, realContext);
    if(calculateLnGamma) {
      WP34S_Ln(res, res, realContext);
    }
    return;
  }

  // Correct our argument and begin the inversion if it is negative
  if(realCompareLessEqual(xin, const_0)) {
    reflect = true;
    realSubtract(const_1, xin, &t, realContext); // t = 1 - xin
    if(realIsAnInteger(&t)) {
      realSetNaN(res);
      return;
    }
  }
  else {
    // Provide a fast path evaluation for positive integer arguments that aren't too large
    // The threshold for overflow is 205! (i.e. 204! is within range and 205! isn't).
    if(realIsAnInteger(xin) && realCompareLessEqual(xin, const_205)) {
      realSubtract(xin, const_1, &x, realContext); // x = xin - 1
      realSetOne(res);
      while(realCompareGreaterEqual(&x, const_2)) {
        realMultiply(res, &x, res, realContext);
        realSubtract(&x, const_1, &x, realContext);
      }
      if(calculateLnGamma) {
        WP34S_Ln(res, res, realContext);
      }
      return;
    }
    realCopy(xin, &t); // t = xin
  }

  WP34S_Calc_Gamma_LnGamma_Lanczos(&t, res, calculateLnGamma, realContext); // t=1-xin or t=xin and res=gamma(xin) or res=lngamma(xin) or res=gamma(1-xin) or res=lngamma(1-xin)

  if(reflect) {
    // figure out xin * PI mod 2PI
    bool_t sinNegated = false;
    WP34S_Mod(xin, const_2, &t, realContext);                                 // t in (-2, 2], with the sign of xin
    realCopyAbs(&t, &x);
    if(realCompareGreaterThan(&x, const_3on2)) {                              // t is taken to its nearest integer, exactly: sin(pi·t) = sin(pi·(t∓2)) = -sin(pi·(t∓1)).
      realSubtract(&x, const_2, &x, realContext);                             // The argument is then small where sin(pi·xin) is, so no digit is lost near an integer xin
    }
    else if(realCompareGreaterThan(&x, const_1on2)) {
      realSubtract(&x, const_1, &x, realContext);
      sinNegated = true;
    }
    if(realIsNegative(&t)) {
      realChangeSign(&x);
    }
    realCopy(&x, &t);
    realMultiply(&t, const39_pi, &t, realContext);                            // t = xin·pi
    C47_WP34S_SinCosTanTaylor_temp75(&t, false, &x, NULL, NULL, realContext); // x = sin(xin·pi)
    if(sinNegated) {
      realChangeSign(&x);
    }

    if(calculateLnGamma) {
      realDivide(const39_pi, &x, &t, realContext);                            // t = pi / sin(pi·xin)
      WP34S_Ln(&t, &t, realContext);                                          // t = ln(pi / sin(pi·xin))
      realSubtract(&t, res, res, realContext);                                // res = ln(pi / sin(pi·xin)) - lngamma(1-xin)
    }
    else {
      realMultiply(&x, res, &t, realContext);                                 // t = sin(pi·xin) × gamma(1-xin)
      realDivide(const39_pi, &t, res, realContext);                           // res = pi / (sin(pi·xin)·gamma(1-xin))
    }
  }
}


void WP34S_Factorial(const real_t *xin, real_t *res, realContext_t *realContext) {
  real_t x;

  realAdd(xin, const_1, &x, realContext);
  WP34S_Gamma_LnGamma(&x, false, res, realContext);
}


void WP34S_Gamma(const real_t *xin, real_t *res, realContext_t *realContext) {
  real_t x;

  realCopy(xin, &x);
  WP34S_Gamma_LnGamma(&x, false, res, realContext);
}


void WP34S_LnGamma(const real_t *xin, real_t *res, realContext_t *realContext) {
  real_t x;

  realCopy(xin, &x);
  WP34S_Gamma_LnGamma(&x, true, res, realContext);
}


/* Natural logarithm.
 *
 * Take advantage of the fact that we store our numbers in the form: m * 10^e
 * so log(m * 10^e) = log(m) + e * log(10)
 * do this so that m is always in the range 0.1 <= m < 2.  However if the number
 * is already in the range 0.5 .. 1.5, this step is skipped.
 *
 * Then use the fact that ln(x²) = 2 * ln(x) to range reduce the mantissa
 * into 1/sqrt(2) <= m < 2.
 *
 * Finally, apply the series expansion:
 *   ln(x) = 2(a+a³/3+a⁵/5+...) where a=(x-1)/(x+1)
 * which converges quickly for an argument near unity.
 */
void WP34S_Ln(const real_t *xin, real_t *res, realContext_t *realContext) {
  real_t z, t, f, n, m, i, v, w, e;
  int32_t expon;

  if(realIsSpecial(xin)) {
    if(realIsNaN(xin) || realIsNegative(xin)) {
      realSetNaN(res);
      return;
    }

    realSetPlusInfinity(res);
    return;
  }

  if(realCompareLessEqual(xin, const_0)) {
    if(realIsNegative(xin)) {
      realSetNaN(res);
      return;
    }

    realSetMinusInfinity(res);
    return;
  }

  realCopy(xin, &z);
  realCopy(const_2, &f);
  realSubtract(xin, const_1, &t, realContext);
  realCopy(&t, &v);
  realSetPositiveSign(&v);
  if(realCompareGreaterThan(&v, const_1on2)) {
    expon = z.exponent + z.digits;
    z.exponent = -z.digits;
  }
  else {
    expon = 0;
  }

  /* The too high case never happens
  while(dn_le(const_2, &z)) {
    realMultiply(&f, const_2, &f, realContext);
    realSquareRoot(&z, &z, realContext);
  } */

  // Range reduce the value by repeated square roots.
  // Making the constant here larger will reduce the number of later
  // iterations at the expense of more square root operations.
  while(realCompareLessEqual(&z, const39_root2on2)) {
    realMultiply(&f, const_2, &f, realContext);
    realSquareRoot(&z, &z, realContext);
  }

  realAdd(&z, const_1, &t, realContext);
  realSubtract(&z, const_1, &v, realContext);
  realDivide(&v, &t, &n, realContext);
  realCopy(&n, &v);
  realMultiply(&v, &v, &m, realContext);
  realCopy(const_3, &i);

  int32ToReal(1 - realContext->digits, &t); // t is the exponent
  realPower10(&t, &z, realContext); // z is the max error

  for(;;) {
    realMultiply(&m, &n, &n, realContext);
    realDivide(&n, &i, &e, realContext);
    realAdd(&v, &e, &w, realContext);
    if(WP34S_RelativeError(&w, &v, &z, realContext)) {
      break;
    }
    realCopy(&w, &v);
    realAdd(&i, const_2, &i, realContext);
  }

  realMultiply(&f, &w, res, realContext);
  if(expon == 0) {
    return;
  }

  int32ToReal(expon, &e);
  realMultiply(&e, const39_ln10, &w, realContext);
  realAdd(res, &w, res, realContext);
}


void WP34S_Log(const real_t *xin, const real_t *base, real_t *res, realContext_t *realContext) {
  real_t y;

  if(realIsSpecial(xin)) {
    if(realIsNaN(xin) || realIsNegative(xin)) {
      realSetNaN(res);
      return;
    }

    realSetPlusInfinity(res);
    return;
  }

  WP34S_Ln(xin, &y, realContext);

  realDivide(&y, base, res, realContext);
}


/* never used
void WP34S_Log2(const real_t *xin, real_t *res, realContext_t *realContext) {
  WP34S_Log(xin, const39_ln2, res, realContext);
}
*/


void WP34S_Log10(const real_t *xin, real_t *res, realContext_t *realContext) {
  WP34S_Log(xin, const39_ln10, res, realContext);
}


void WP34S_Logxy(const real_t *yin, const real_t *xin, real_t *res, realContext_t *realContext) {
  real_t lx;

  WP34S_Ln(xin, &lx, realContext);
  WP34S_Log(yin, &lx, res, realContext);
}


bool_t WP34S_RelativeError(const real_t *x, const real_t *y, const real_t *tol, realContext_t *realContext) {
  real_t a;

  if(realIsZero(x)) {
    realCopyAbs(y, &a);
    return realCompareLessThan(&a, tol);
  }

  realSubtract(x, y, &a, realContext);
  realDivide(&a, x, &a, realContext);
  realSetPositiveSign(&a);
  return realCompareLessThan(&a, tol);
}


bool_t WP34S_AbsoluteError(const real_t *x, const real_t *y, const real_t *tol, realContext_t *realContext) {
  real_t a;
  realSubtract(x, y, &a, realContext);
  return realCompareAbsLessThan(&a, tol);
}


bool_t WP34S_ComplexRelativeError(const real_t *xReal, const real_t *xImag, const real_t *yReal, const real_t *yImag, const real_t *tol, realContext_t *realContext) {
  real_t a, b;

  if(realIsZero(xReal) && realIsZero(xImag)) {
    complexMagnitude(yReal, yImag, &a, realContext);
  }
  else {
    realSubtract(xReal, yReal, &a, realContext);
    realSubtract(xImag, yImag, &b, realContext);
    divComplexComplex(&a, &b, xReal, xImag, &a, &b, realContext);
    complexMagnitude(&a, &b, &a, realContext);
  }
  return realCompareLessThan(&a, tol);
}


bool_t WP34S_ComplexAbsError(const real_t *xReal, const real_t *xImag, const real_t *yReal, const real_t *yImag, const real_t *tol, realContext_t *realContext) {
  real_t a, b, r;

  realSubtract(xReal, yReal, &a, realContext), realSubtract(xImag, yImag, &b, realContext);
  complexMagnitude(&a, &b, &r, realContext);
  return realCompareLessThan(&r, tol);
}


/* Hyperbolic functions.
 * We start with a utility routine that calculates sinh and cosh.
 * We do the sihn as (e^x - 1) (e^x + 1) / (2 e^x) for numerical stability
 * reasons if the value of x is smallish.
 */
void WP34S_SinhCosh(const real_t *x, real_t *sinhOut, real_t *coshOut, realContext_t *realContext) {
  real_t t, u, v;

  if(realIsNaN(x)) {
    if(sinhOut != NULL) {
      realSetNaN(sinhOut);
    }
    if(coshOut != NULL) {
      realSetNaN(coshOut);
    }
    return;
  }

  if(sinhOut != NULL) {
    if(realCompareAbsLessThan(x, const_1on2)) {
      WP34S_ExpM1(x, &u, realContext);                         // u = e^x - 1
      realMultiply(&u, const_1on2, &t, realContext);           // t = (e^x - 1) / 2

      realAdd(&u, const_1, &u, realContext);                   // u = e^x
      realDivide(&t, &u, &v, realContext);                     // v = (e^x - 1) / 2e^x

      realAdd(&u, const_1, &u, realContext);                   // u = e^x + 1
      realMultiply(&u, &v, sinhOut, realContext);              // sinhOut = (e^x - 1)(e^x + 1) / 2e^x
    }
    else {
      realExp(x, &u, realContext);                             // u = e^x
      realDivide(const_1, &u, &v, realContext);                // v = e^-x
      realSubtract(&u, &v, sinhOut, realContext);              // sinhOut = (e^x + e^-x)
      realMultiply(sinhOut, const_1on2, sinhOut, realContext); // sinhOut = (e^x + e^-x)/2
    }
  }
  if(coshOut != NULL) {
   realExp(x, &u, realContext);                                // u = e^x
   realDivide(const_1, &u, &v, realContext);                   // v = e^-x
   realAdd(&u, &v, coshOut, realContext);                      // coshOut = (e^x + e^-x)
   realMultiply(coshOut, const_1on2, coshOut, realContext);    // coshOut = (e^x + e^-x)/2
  }
}


void WP34S_Tanh(const real_t *x, real_t *res, realContext_t *realContext) {
  if(realIsNaN(x)) {
    realSetNaN(res);
  }
  else if(realCompareAbsGreaterThan(x, const_47)) { // equals 1 to 39 digits
    realCopy((realIsPositive(x) ? const_1 : const__1), res);
  }
  else {
    real_t a, b;

    realAdd(x, x, &a, realContext);        // a = 2x
    WP34S_ExpM1(&a, &b, realContext);      // b = exp(2x) - 1
    realAdd(&b, const_2, &a, realContext); // a = exp(2x) - 1 + 2 = exp(2x) + 1
    realDivide(&b, &a, res, realContext);  // res = (exp(2x) - 1) / (exp(2x) + 1)
  }
}


void WP34S_ArcSinh(const real_t *xin, real_t *res, realContext_t *realContext) {
  real_t a, xabs;
  real_t *x = &xabs;
  const bool_t negative = realIsNegative(xin);

  realCopyAbs(xin, x);                   // arsinh is odd: the formula below cancels for x < 0, so it runs on |x|
  if(!realIsZero(x) && realGetExponent(x) < -18) {   // arsinh x = x - x³/6 to 72 digits, below |x| by less than half a unit at 34 digits
    realMultiply(x, x, &a, realContext);
    realMultiply(&a, x, &a, realContext);
    realDivide(&a, const_6, &a, realContext);
    realSubtract(x, &a, res, realContext);
    if(realCompareEqual(res, x)) {       // x³/6 is below the context's last digit: one step toward 0 keeps the result below |x|
      realNextToward(x, const_0, res, realContext);
    }
    if(negative) {
      realChangeSign(res);
    }
    return;
  }
  realMultiply(x, x, &a, realContext);   // a = x²
  realAdd(&a, const_1, &a, realContext); // a = x² + 1
  realSquareRoot(&a, &a, realContext);   // a = sqrt(x²+1)
  realAdd(&a, const_1, &a, realContext); // a = sqrt(x²+1)+1
  realDivide(x, &a, &a, realContext);    // a = x / (sqrt(x²+1)+1)
  realAdd(&a, const_1, &a, realContext); // a = x / (sqrt(x²+1)+1) + 1
  realMultiply(x, &a, &a, realContext);  // y = x * (x / (sqrt(x²+1)+1) + 1)
  WP34S_Ln1P(&a, res, realContext);      // res = ln(1 + (x * (x / (sqrt(x²+1)+1) + 1)))
  if(negative) {
    realChangeSign(res);
  }
}



/* never used
void WP34S_ArcCosh(const real_t *xin, real_t *res, realContext_t *realContext) {
  real_t x, z;

  realCopy(xin, &x);
  realMultiply(&x, &x, res, realContext);      // res = x²
  realSubtract(res, const_1, &z, realContext); // z = x² - 1
  realSquareRoot(&z, res, realContext);        // res = sqrt(x²-1)
  realAdd(res, &x, &z, realContext);           // z = x + sqrt(x²-1)
  WP34S_Ln(&z, res, realContext);              // res = ln(x + sqrt(x²-1))
}
*/


void WP34S_ArcTanh(const real_t *xin, real_t *res, realContext_t *realContext) {
  real_t y, z, xabs;
  real_t *x = &xabs;
  const bool_t negative = realIsNegative(xin);

  if(realIsNaN(xin)) {
    realSetNaN(res);
  }

  // Not the obvious formula but more stable...
  realCopyAbs(xin, x);                            // artanh is odd: near -1 the formula takes 1 + z of a z near -1, so it runs on |x|, where 1-x is exact
  realSubtract(const_1, x, &z, realContext);      // z = 1-x
  realDivide(x, &z, &y, realContext);             // y = x / (1-x)
  realMultiply(&y, const_2, &z, realContext);     // z = 2x / (1-x)
  WP34S_Ln1P(&z, &y, realContext);                // y = ln(1 + 2x / (1-x))
  realMultiply(&y, const_1on2, res, realContext); // res = ln(1 + 2x / (1-x)) / 2
  if(negative) {
    realChangeSign(res);
  }
}


/* ln(1+x) */
void WP34S_Ln1P(const real_t *x, real_t *res, realContext_t *realContext) {
  real_t u, v, w;

  if(realIsSpecial(x) || realIsZero(x)) {
    realCopy(x, res);
  }
  else {
    realAdd(x, const_1, &u, realContext);       // u = x+1
    realSubtract(&u, const_1, &v, realContext); // v = x
    if(realIsZero(&v)) {
      realCopy(x, res);
    }
    else {
      realDivide(x, &v, &w, realContext);
      WP34S_Ln(&u, &v, realContext);
      realMultiply(&v, &w, res, realContext);
    }
  }
}


/* exp(x)-1 */
void WP34S_ExpM1(const real_t *x, real_t *res, realContext_t *realContext) {
  real_t u, v, w;

  if(!realExpLimitCheck(x, res, const__1)) {
    return;
  }

  realExp(x, &u, realContext);
  realSubtract(&u, const_1, &v, realContext);
  if(realIsZero(&v)) { // |x| is very little
    realCopy(x, res);
  }
  else if(realCompareEqual(&v, const__1)) {
    realSetNegativeOne(res);
  }
  else if(realCompareAbsLessThan(x, const_1on10)) {
    realMultiply(&v, x, &w, realContext);
    WP34S_Ln(&u, &v, realContext);
    realDivide(&w, &v, res, realContext);
  }
  else {
    realCopy(&v, res);
  }
}


static void WP34S_CalcComplexLnGamma_Lanczos(const real_t *zReal, const real_t *zImag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  real_t rReal, sReal, tReal, uReal, vReal;
  real_t rImag, sImag, tImag, uImag, vImag;
  int k;
  int32_t savedContextDigits;

  savedContextDigits = realContext->digits;
  if(realContext->digits < 51) {
    realContext->digits = 51;
  }

  realSetZero(&uReal);
  realSetZero(&uImag);
  realAdd(zReal, const_29, &tReal, realContext);
  realCopy(zImag, &tImag);
  for(k=28; k>=0; k--) {
    divRealComplex((real_t *)(((uint8_t *)const51_gammaC01) + k*REAL_SIZE_IN_BYTES(51)), &tReal, &tImag, &sReal, &sImag, realContext);
    realSubtract(&tReal, const_1, &tReal, realContext);
    realAdd(&uReal, &sReal, &uReal, realContext);
    realAdd(&uImag, &sImag, &uImag, realContext);
  }
  realAdd(&uReal, const51_gammaC00, &tReal, realContext);
  realCopy(&uImag, &tImag);
  lnComplex(&tReal, &tImag, &sReal, &sImag, realContext);  // (s1, s2)

  realAdd(zReal, const_gammaR, &rReal, realContext);
  realCopy(zImag, &rImag);
  lnComplex(&rReal, &rImag, &uReal, &uImag, realContext);

  realAdd(zReal, const_1on2, &tReal, realContext);
  realCopy(zImag, &tImag);
  mulComplexComplex(&tReal, &tImag, &uReal, &uImag, &vReal, &vImag, realContext);

  realSubtract(&vReal, &rReal, &uReal, realContext);
  realSubtract(&vImag, zImag, &uImag, realContext);

  realContext->digits = savedContextDigits;

  realAdd(&uReal, &sReal, resReal, realContext);
  realAdd(&uImag, &sImag, resImag, realContext);
}


static void WP34S_ComplexGammaLnGamma(const real_t *zReal, const real_t *zImag, const bool_t calculateLnGamma, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  real_t sinPiZReal, tReal, uReal, xReal;
  real_t sinPiZImag, tImag, uImag, xImag;
  bool_t reflect = false;

  // Check for special cases
  if(realIsSpecial(zReal) || realIsSpecial(zImag)) {
    if(realIsNaN(zReal) || realIsNaN(zImag)) {
      realSetNaN(resReal);
      realSetNaN(resImag);
    }
    else {
      if(realIsInfinite(zReal)) {
        if(realIsInfinite(zImag) || realIsNegative(zReal)) {
          realSetNaN(resReal);
          realSetNaN(resImag);
        }
        else {
          realSetPlusInfinity(resReal);
          realSetZero(resImag);
        }
      }
      else {
        realSetZero(resReal);
        realSetZero(resImag);
      }
    }
    return;
  }

  // Correct our argument and begin the inversion if it is negative
  if(realIsNegative(zReal)) {
    reflect = true;
    realSubtract(const_1, zReal, &tReal, realContext);
    if(realIsZero(zImag) && realIsAnInteger(&tReal)) {
      realSetNaN(resReal);
      realSetNaN(resImag);
      return;
    }
    realSubtract(&tReal, const_1, &xReal, realContext);
    realMinus(zImag, &xImag, realContext);
  }
  else {
    realSubtract(zReal, const_1, &xReal, realContext);
    realCopy(zImag, &xImag);
  }

  // Sum the series
  WP34S_CalcComplexLnGamma_Lanczos(&xReal, &xImag, resReal, resImag, realContext);
  if(!calculateLnGamma) {
    expComplex(resReal, resImag, resReal, resImag, realContext);
  }

  // Finally invert if we started with a negative argument
  if(reflect) {
    realMultiply(zReal, const39_pi, &tReal, realContext);
    realMultiply(zImag, const39_pi, &tImag, realContext);
    sinComplex(&tReal, &tImag, &sinPiZReal, &sinPiZImag, realContext);
    if(!calculateLnGamma) {
      mulComplexComplex(&sinPiZReal, &sinPiZImag, resReal, resImag, &uReal, &uImag, realContext);
      divRealComplex(const39_pi, &uReal, &uImag, resReal, resImag, realContext);
    }
    else {
      divRealComplex(const39_pi, &sinPiZReal, &sinPiZImag, &uReal, &uImag, realContext);
      lnComplex(&uReal, &uImag, &tReal, &tImag, realContext);
      realSubtract(&tReal, resReal, resReal, realContext);
      realSubtract(&tImag, resImag, resImag, realContext);
    }
  }
}


void WP34S_ComplexGamma(const real_t *zinReal, const real_t *zinImag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  real_t zReal, zImag;

  realCopy(zinReal, &zReal);
  realCopy(zinImag, &zImag);
  WP34S_ComplexGammaLnGamma(&zReal, &zImag, false, resReal, resImag, realContext);
}


void WP34S_ComplexLnGamma(const real_t *zinReal, const real_t *zinImag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  real_t zReal, zImag;

  realCopy(zinReal, &zReal);
  realCopy(zinImag, &zImag);
  WP34S_ComplexGammaLnGamma(&zReal, &zImag, true, resReal, resImag, realContext);
}


static void doMod(const real_t *x, const real_t *y, real_t *res, realContext_t *realContext,
                  unsigned int digits, real_t *out) {
  realContext_t c = *realContext;

  c.digits = digits;
  realDivideRemainder(x, y, out, &c);
  realPlus(out, res, realContext);
}


#if 0
// Original e900193 fixes 2025-08 Pauli
// When the memory allocation below is in place, 1E700 SIN (REAL), as well as 700 10^x SIN (LI) results in -NaN
void WP34S_Mod_Pauli(const real_t *x, const real_t *y, real_t *res, realContext_t *realContext) {
#if defined(DMCP_BUILD) && HARDWARE_MODEL == HWM_DM42
  unsigned int digits = 6147;
  const size_t blocks = TO_BLOCKS(REAL_SIZE_IN_BYTES(6147));
  real_t *tofree = allocC47Blocks(blocks);
  REAL_T_PTR(small, 2139); // Fallback size
  real_t *out;

  out = tofree == NULL ? small : tofree;
  doMod(x, y, res, realContext, digits, out);
  freeC47Blocks(out, blocks);
#else
  REAL_T_PTR(temp, 6147);

  doMod(x, y, res, realContext, 6147, temp);
#endif // DMCP_BUILD && HARDWARE_MODEL == HWM_DM42
}


// Original e900193 fixes 2025-08 Pauli
// When the memory allocation below is in place, 1E700 SIN (REAL), as well as 700 10^x SIN (LI) results in -NaN
void WP34S_BigMod_Pauli(const real_t *x, const real_t *y, real_t *res, realContext_t *realContext) {
#if defined(DMCP_BUILD) && HARDWARE_MODEL == HWM_DM42
  unsigned int digits = 12321;
  const size_t blocks = TO_BLOCKS(REAL_SIZE_IN_BYTES(12321));
  real_t *tofree = allocC47Blocks(blocks);
  REAL_T_PTR(small, 2139); // Fallback size
  real_t *out;

  out = tofree == NULL ? small : tofree;
  doMod(x, y, res, realContext, digits, out);
  freeC47Blocks(out, blocks);
#else
  REAL_T_PTR(temp, 12321);

  doMod(x, y, res, realContext, 12321, temp);
#endif // DMCP_BUILD && HARDWARE_MODEL == HWM_DM42
}
#endif // 0


// long integer handling: see longIntegerAngleReduction in registerValueConversions.c, case amRadian. A long integer angle never arrives here as an integer. There,
// longIntegerToString writes it base 10 into tmpString and decNumberFromString reads that into a 2139 digit real, which is the x this function reduces against
// const6147_2pi, and the result is rounded down to real75 in the same place. Angles over 1000 base 10 digits are refused before the conversion, and that context sits
// at 2139 digits because 6147 overran the stack.
// The reduction buffer comes from the heap, not the frame: 1436 bytes of this function's 1488 byte frame on the DM42, 8224 on every other build. Argument reduction runs
// inside the integrand of a plotted integral, so this frame sits under two engine frames.
void WP34S_Mod(const real_t *x, const real_t *y, real_t *res, realContext_t *realContext) {
#if defined(DMCP_BUILD) && HARDWARE_MODEL == HWM_DM42
  REAL_T_ALLOC(small, 2139); // Fallback size
  if(small == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    realSetNaN(res);
  }
  else {
    doMod(x, y, res, realContext, 2139, small);
  }
  REAL_T_FREE(small, 2139);
#else
  REAL_T_ALLOC(temp, 12321);
  if(temp == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    realSetNaN(res);
  }
  else {
    doMod(x, y, res, realContext, 6147, temp);
  }
  REAL_T_FREE(temp, 12321);
#endif // DMCP_BUILD && HARDWARE_MODEL == HWM_DM42
}


// Same buffer sizes as WP34S_Mod above, taken the same way. This is the one mod2Pi calls.
void WP34S_BigMod(const real_t *x, const real_t *y, real_t *res, realContext_t *realContext) {
#if defined(DMCP_BUILD) && HARDWARE_MODEL == HWM_DM42
  REAL_T_ALLOC(small, 2139); // Fallback size
  if(small == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    realSetNaN(res);
  }
  else {
    doMod(x, y, res, realContext, 2139, small);
  }
  REAL_T_FREE(small, 2139);
#else
  REAL_T_ALLOC(temp, 12321);
  if(temp == NULL) {
    displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE);
    realSetNaN(res);
  }
  else {
    doMod(x, y, res, realContext, 12321, temp);                //printf("\n******  ****** NOT MATCHED 2pi !! ****** ******\n");
  }
  REAL_T_FREE(temp, 12321);
#endif // DMCP_BUILD && HARDWARE_MODEL == HWM_DM42
}


void mod2Pi(const real_t *x, real_t *res, realContext_t *realContext) {
  WP34S_BigMod(x, const6147_2pi, res, realContext);
}

static void gser(const real_t *a, const real_t *x, const real_t *gln, real_t *res, realContext_t *realContext) {
  real_t ap, del, sum, t, u;
  int32_t i;

  if(realCompareLessEqual(x, const_0)) {
    realSetZero(res);
    return;
  }
  realCopy(a, &ap);
  realDivide(const_1, a, &sum, realContext);
  realCopy(&sum, &del);
  for(i=0; i<1000; i++) {
    realAdd(&ap, const_1, &ap, realContext);
    realDivide(x, &ap, &t, realContext);
    realMultiply(&del, &t, &del, realContext);
    realAdd(&sum, &del, &t, realContext);
    if(realCompareEqual(&sum, &t)) {
      break;
    }
    realCopy(&t, &sum);
  }
  WP34S_Ln(x, &t, realContext);
  realMultiply(&t, a, &u, realContext);
  realSubtract(&u, x, &t, realContext);
  realSubtract(&t, gln, &u, realContext);
  realExp(&u, &t, realContext);
  realMultiply(&sum, &t, res, realContext);
  return;
}


static void gcheckSmall(real_t *v, realContext_t *realContext) {
  if(realCompareAbsLessThan(v, const_1e_10000)) {
    realCopy(const_1e_10000, v);
  }
}


static void gcf(const real_t *a, const real_t *x, const real_t *gln, real_t *res, realContext_t *realContext) {
  real_t an, b, c, d, h, t, u, v, i;
  int32_t n;

  realAdd(x, const_1, &t, realContext);
  realSubtract(&t, a, &b, realContext);    // b = (x+1) - a
  gcheckSmall(&b, realContext);
  realSetPlusInfinity(&c);
  realDivide(const_1, &b, &d, realContext);
  realCopy(&d, &h);
  realSetZero(&i);
  for(n=0; n<1000; n++) {
    realAdd(&i, const_1, &i, realContext);
    realSubtract(a, &i, &t, realContext);   // t = a-i
    realMultiply(&i, &t, &an, realContext); // an = -i (i-a)
    realAdd(&b, const_2, &b, realContext);
    realMultiply(&an, &d, &t, realContext);
    realAdd(&t, &b, &v, realContext);
    gcheckSmall(&v, realContext);
    realDivide(const_1, &v, &d, realContext);
    realDivide(&an, &c, &t, realContext);
    realAdd(&b, &t, &c, realContext);
    gcheckSmall(&c, realContext);
    realMultiply(&d, &c, &t, realContext);
    realMultiply(&h, &t, &u, realContext);
    if(realCompareEqual(&u, &h)) {
      break;
    }
    realCopy(&u, &h);
  }
  WP34S_Ln(x, &t, realContext);
  realMultiply(&t, a, &u, realContext);
  realSubtract(&u, x, &t, realContext);
  realSubtract(&t, gln, &u, realContext);
  realExp(&u, &t, realContext);
  realMultiply(&t, &h, res, realContext);
  return;
}


void WP34S_GammaP(const real_t *x, const real_t *a, real_t *res, realContext_t *realContext, bool_t upper, bool_t regularised) {
  real_t z, lga;

  if(realIsNegative(x) || realCompareLessEqual(a, const_0) || realIsNaN(x) || realIsNaN(a) || realIsInfinite(a)) {
    realSetNaN(res);
    return;
  }

  if(realIsInfinite(x)) {
    if(upper) {
      if(regularised) {
        realSetOne(res);
        return;
      }

      WP34S_Gamma(a, res, realContext);
      return;
    }

    realSetZero(res);
    return;
  }

  realAdd(a, const_1, &lga, realContext);
  realCompare(x, &lga, &z, realContext);
  if(regularised) {
    WP34S_LnGamma(a, &lga, realContext);
  }
  else {
    realSetZero(&lga);
  }
  if(realIsNegative(&z)) {
    /* Deal with a difficult case by using the other expansion */
    if(realCompareGreaterThan(a, const_9000)) {
      realCopy(const_995on1000, &z);
      realMultiply(a, &z, &z, realContext);
      if(realCompareGreaterThan(x, &z)) {
        goto use_cf;
      }
    }

    gser(a, x, &lga, res, realContext);
    if(upper) {
      goto invert;
    }
  }
  else {
    use_cf:
    gcf(a, x, &lga, res, realContext);
    if(!upper) {
      goto invert;
    }
  }
  return;

  invert:
  if(regularised) {
    realSubtract(const_1, res, res, realContext);
    return;
  }
  WP34S_Gamma(a, &z, realContext);
  realSubtract(&z, res, res, realContext);
  return;
}


// erf and erfc were embedded library functions on WP 34S
void WP34S_Erf(const real_t *x, real_t *res, realContext_t *realContext) {
  real_t p, q;

  if(realIsInfinite(x)) {
    int32ToReal(realIsNegative(x) ? -1 : 1, res);
    return;
  }

  realMultiply(x, x, &p, realContext);
  WP34S_GammaP(&p, const_1on2, &p, realContext, false, false);
  realSquareRoot(const39_pi, &q, realContext);
  realDivide(&p, &q, &p, realContext);
  if(realIsNegative(x)) {
    realChangeSign(&p);
  }
  realCopy(&p, res);
  return;
}


#if defined(OPTION_DIST_NORMAL)   // its only step is WP34S_Cdf_Q, which normal.c stubs out with the option
void WP34S_Erfc(const real_t *x, real_t *res, realContext_t *realContext) {
  real_t p;

  realSquareRoot(const_2, &p, realContext);
  realMultiply(x, &p, &p, realContext);
  realChangeSign(&p);
  WP34S_Cdf_Q(&p, &p, realContext);
  realMultiply(&p, const_2, res, realContext);
}
#endif // OPTION_DIST_NORMAL


static void check_low(real_t *d) {
  real_t real_1e_32;

  realSetOne(&real_1e_32);
  real_1e_32.exponent -= 32;
  if(realCompareAbsLessThan(d, &real_1e_32)) {
    realCopy(d, &real_1e_32);
  }
}


static void ib_step(const real_t *aa, real_t *d, real_t *c, realContext_t *realContext) {
  real_t t, u;

  realMultiply(aa, d, &t, realContext);
  realAdd(&t, const_1, &u, realContext);   // d = 1+aa*d
  check_low(&u);
  realDivide(const_1, &u, d, realContext);
  realDivide(aa, c, &t, realContext);
  realAdd(&t, const_1, c, realContext);    // c = 1+aa/c
  check_low(c);
}


static void betacf(const real_t *a, const real_t *b, const real_t *x, real_t *r, realContext_t *realContext) {
  real_t aa, c, d, apb, am1, ap1, m, m2, oldr;
  int i;
  real_t t, u, v, w;
  int32_t loop = 0;

  realAdd(a, const_1, &ap1, realContext);        // ap1 = 1+a
  realSubtract(a, const_1, &am1, realContext);   // am1 = a-1
  realAdd(a, b, &apb, realContext);              // apb = a+b
  realSetOne(&c);                                // c = 1
  realDivide(x, &ap1, &t, realContext);
  realMultiply(&t, &apb, &u, realContext);
  realSubtract(const_1, &u, &t, realContext);    // t = 1-apb*x/ap1
  check_low(&t);
  realDivide(const_1, &t, &d, realContext);      // d = 1/t
  realCopy(&d, r);    // res = d
  realSetZero(&m);
  for(i=0; i<500; i++) {
    realCopy(r, &oldr);
    realAdd(&m, const_1, &m, realContext);       // m = i+1
    realMultiply(&m, const_2, &m2, realContext);
    realSubtract(b, &m, &t, realContext);
    realMultiply(&t, &m, &u, realContext);
    realMultiply(&u, x, &t, realContext);        // t = m*(b-m)*x
    realAdd(&am1, &m2, &u, realContext);
    realAdd(a, &m2, &v, realContext);
    realMultiply(&u, &v, &w, realContext);       // w = (am1+m2)*(a+m2)
    realDivide(&t, &w, &aa, realContext);        // aa = t/w
    ib_step(&aa, &d, &c, realContext);
    realMultiply(r, &d, &t, realContext);
    realMultiply(&t, &c, r, realContext);        // r = r*d*c
    realAdd(a, &m, &t, realContext);
    realAdd(&apb, &m, &u, realContext);
    realMultiply(&t, &u, &w, realContext);
    realMultiply(&w, x, &t, realContext);
    realMinus(&t, &w, realContext);              // w = -(a+m)*(apb+m)*x
    realAdd(a, &m2, &t, realContext);
    realAdd(&ap1, &m2, &u, realContext);
    realMultiply(&t, &u, &v, realContext);       // v = (a+m2)*(ap1+m2)
    realDivide(&w, &v, &aa, realContext);        // aa = w/v
    ib_step(&aa, &d, &c, realContext);
    realMultiply(&d, &c, &v, realContext);
    realMultiply(r, &v, r, realContext);         // r *= d*c
    if(realCompareEqual(&oldr, r)) {
      break;
    }
    if(monitorExit(&loop, "Iter: ")) {
      break;
    }
  }
}


// Regularised incomplete beta function Ix(a, b)
void WP34S_betai(const real_t *b, const real_t *a, const real_t *x, real_t *res, realContext_t *realContext) {
  real_t t, u, v, w, y;
  int32_t limit = 0;

  realCompare(const_1, x, &t, realContext);
  if(realIsNegative(x) || realIsNegative(&t)) {
    realSetNaN(res);
   return;
  }

  if(realIsZero(x) || realIsZero(&t)) {
    limit = 1;
  }
  else {
    LnBeta(a, b, &u, realContext);
    WP34S_Ln(x, &v, realContext);              // v = ln(x)
    realMultiply(a, &v, &t, realContext);
    realSubtract(&t, &u, &v, realContext);     // v = lng(...)+a.ln(x)
    realSubtract(const_1, x, &y, realContext); // y = 1-x
    WP34S_Ln(&y, &u, realContext);             // u = ln(1-x)
    realMultiply(&u, b, &t, realContext);
    realAdd(&t, &v, &u, realContext);          // u = lng(...)+a.ln(x)+b.ln(1-x)
    realExp(&u, &w, realContext);
  }

  realAdd(a, b, &v, realContext);
  realAdd(&v, const_2, &u, realContext);       // u = a+b+2
  realAdd(a, const_1, &t, realContext);        // t = a+1
  realDivide(&t, &u, &v, realContext);         // u = (a+1)/(a+b+2)
  if(realCompareLessThan(x, &v)) {
    if(limit) {
      realSetZero(res);
    }
    else {
      betacf(a, b, x, &t, realContext);
      realDivide(&t, a, &u, realContext);
      realMultiply(&w, &u, res, realContext);
    }
  }
  else {
    if(limit) {
      realSetOne(res);
    }
    else {
      betacf(b, a, &y, &t, realContext);
      realDivide(&t, b, &u, realContext);
      realMultiply(&w, &u, &t, realContext);
      realSubtract(const_1, &t, res, realContext);
    }
  }
}


void WP34S_Bernoulli(const real_t *x, real_t *res, bool_t bn_star, realContext_t *realContext) {
  real_t p;

  if((!realIsAnInteger(x)) || realCompareLessThan(x, const_0)) {
    realSetNaN(res);
    return;
  }
  if(realIsZero(x)) {// Bn_0
    realCopy(bn_star ? const_NaN : const_1, res);
    return;
  }
  if(!bn_star) {
    if(realCompareEqual(x, const_1)) { // zeta_0
      realCopy(const_1on2, res);
      realChangeSign(res);
      return;
    }
    else if(realMultiply(x, const_1on2, &p, realContext), (!realIsAnInteger(&p))) { // Bn_odd
      realSetZero(res);
      return;
    }
    realCopy(x, &p);
  }
  else {
    realMultiply(x, const_2, &p, realContext);
  }

  // bernoulli
  realSubtract(&p, const_1, &p, realContext);
  realChangeSign(&p);
  WP34S_Zeta(&p, &p, realContext);
  realMultiply(&p, x, &p, realContext);
  realChangeSign(&p);

  if(bn_star) {
    realMultiply(&p, const_2, &p, realContext);
    realSetPositiveSign(&p);
  }
  realCopy(&p, res);
}


/**************************************************************************/
/* Zeta function implementation based on Jean-Marc Baillard's from:
 * http://hp41programs.yolasite.com/zeta.php
 * This is the same algorithm as the C version uses, just with fewer terms and
 * with the constants computed on the fly.
 */

static void zeta_calc(const real_t *x, real_t *reg1, real_t *reg7, real_t *res, realContext_t *realContext) {
  real_t p, q, r, s, reg0, reg3, reg4, reg5, reg6;
  int32_t loop = 0;

  int32ToReal(60/*44*/, &reg0);
  int32ToReal(60/*44*/, &reg3);
  int32ToReal(1, &reg4);
  int32ToReal(1, &reg5);
  int32ToReal(-1, &reg6);
  realSetZero(&p);
  do { // zeta_loop
    realMinus(reg1, &q, realContext);
    realPower(&reg0, &q, &q, realContext);
    realMultiply(&reg5, &q, &q, realContext);
    realChangeSign(&reg6);
    realMultiply(&q, &reg6, &q, realContext);
    realAdd(&p, &q, &p, realContext);
    realMultiply(&reg0, const_2, &q, realContext);
    realMultiply(&q, &reg0, &q, realContext);
    realSubtract(&q, &reg0, &q, realContext);
    realMultiply(&q, &reg4, &q, realContext);
    realMultiply(&reg3, &reg3, &r, realContext);
    realSubtract(&reg0, const_1, &s, realContext);
    realMultiply(&s, &s, &s, realContext);
    realSubtract(&r, &s, &r, realContext);
    realMultiply(&r, const_2, &r, realContext);
    realDivide(&q, &r, &q, realContext);
    realCopy(&q, &reg4);
    realAdd(&q, &reg5, &reg5, realContext);
    realSubtract(&reg0, const_1, &reg0, realContext);
    if(monitorExit(&loop, "Iter: ")) {
      break;
    }
  } while(realCompareGreaterThan(&reg0, const_0));
  realDivide(&p, &reg5, &p, realContext);
  realSubtract(const_1, reg1, &r, realContext);
  realMultiply(const39_ln2, &r, &r, realContext);
  WP34S_ExpM1(&r, &q, realContext);
  realDivide(&p, &q, res, realContext);
}


void WP34S_Zeta(const real_t *x, real_t *res, realContext_t *realContext) {
  real_t p, q, r, reg1, reg7;

  if(realIsZero(x)) {
    realCopy(const_1on2, res);
    realChangeSign(res);
    return;
  }

  // zeta_int
  realCopy(x, &reg1);
  realCopy(x, &reg7);
  if(realCompareGreaterThan(x, const_1on2)) {
    zeta_calc(x, &reg1, &reg7, res, realContext);
  }
  else { // zeta_neg
    realSubtract(const_1, x, &q, realContext);
    realCopy(&q, &reg1);
    zeta_calc(&q, &reg1, &reg7, &p, realContext);
    C47_WP34S_Asin(const_1, &q, realContext);
    realMultiply(&q, &reg7, &q, realContext);
    C47_WP34S_Cvt2RadSinCosTan(&q, amRadian, &r, NULL, NULL, realContext);
    realMultiply(&p, &r, &p, realContext);
    realDivide(&p, const39_pi, &p, realContext);
    realPower(const39_2pi, &reg7, &q, realContext);
    realMultiply(&p, &q, &p, realContext);
    realCopy(&reg1, &q);
    WP34S_Gamma(&q, &r, realContext);
    realMultiply(&r, &p, res, realContext);
  }
}


/**************************************************************************/
/* A rough guess is made and then Newton's method is used to attain
 * convergence.  The C version (in ../trunk/unused/lambertW.c) works
 * similarily but uses Halley's method which converges a cubically
 * instead of quadratically.
 *
 * This code is based on a discussion on the MoHPC.  Search the archives
 * for "Lambert W (HP-41)" starting on the 2 Sept 2012.  The actual code
 * is Dieter's from a message posted on the 23rd of Sept 2012.
 */
void WP34S_LambertW(const real_t *x, real_t *res, bool_t negativeBranch, realContext_t *realContext) {
  real_t p, q, r, reg0, reg1, reg2;
  bool_t converged = false;

  if(realIsSpecial(x)) {
   realSetNaN(res);
   return;
  }
  if(realIsZero(x)) {
   realCopy(negativeBranch ? const_minusInfinity : const_0, res);
   return;
  }
  if(negativeBranch && realIsPositive(x)) {
   realSetNaN(res);
   return;
  }

  // LamW0_common
  realCopy(x, &reg0);
  int32ToReal(7, &reg1);
  int32ToReal(negativeBranch ? 25 : 35, &p), p.exponent -= 2, realChangeSign(&p);
  if(realCompareLessEqual(&reg0, &p)) {
    realDivide(const_1, const39_eE, &q, realContext);
    realAdd(&reg0, &q, &q, realContext);
    realMultiply(&q, const39_eE, &q, realContext);
    realCopy(&q, &reg2);
    realMultiply(&q, const_2, &q, realContext);
    realSquareRoot(&q, &r, realContext);
    if(negativeBranch) {
      realChangeSign(&r);
    }
    realMultiply(&q, const39_1on3, &q, realContext);
    realSubtract(&r, &q, &q, realContext);

    // Newton iteration for W+1
    do { //LamW0_wp1_newton
      // FILL and x close to -1/e
      realMinus(&q, &p, realContext);
      WP34S_ExpM1(&p, &p, realContext);
      realMultiply(&p, &reg0, &p, realContext);
      realMultiply(&p, const39_eE, &p, realContext);
      realSubtract(&p, &q, &p, realContext);
      realAdd(&p, &reg2, &p, realContext);
      realDivide(&p, &q, &p, realContext);
      realAdd(&p, &q, &p, realContext);
      if(converged) {
        break;
      }
      if(WP34S_AbsoluteError(&p, &q, (realContext == &ctxtReal39) ? const_1e_37 : const_1e_49, realContext)) { // test if absolute error
        converged = true;
      }
      realSubtract(&reg1, const_1, &reg1, realContext);
      realCopy(&p, &q);
    } while(realCompareGreaterThan(&reg1, const_0));
    // LamW0_converged
    realSubtract(&p, const_1, res, realContext);
  }
  else {// LamW0_normal
    if(negativeBranch) {// LamW0_smallx
      realMinus(&reg0, &q, realContext);
      WP34S_Ln(&q, &q, realContext);
      realMinus(&q, &r, realContext);
      WP34S_Ln(&r, &r, realContext);
      realSubtract(&q, &r, &q, realContext);
    }
    else {
      WP34S_Ln1P(&reg0, &q, realContext);
      if(realCompareGreaterThan(&q, const_1)) {
        WP34S_Ln(&q, &r, realContext);
        realSubtract(&q, &r, &q, realContext);
      }
    }
    // Newton-Halley iteration for W
    // x not close to -1/e
    do { // LamW0_halley
      realExp(&q, &r, realContext);
      realDivide(&reg0, &r, &r, realContext);
      realSubtract(&q, &r, &r, realContext);
      realAdd(&q, const_1, &p, realContext);
      realDivide(&r, &p, &r, realContext);
      realDivide(const_1, &p, &p, realContext);
      realAdd(&p, const_1, &p, realContext);
      realMultiply(&p, &r, &p, realContext);
      realMultiply(&p, const_1on2, &p, realContext);
      realChangeSign(&p);
      realAdd(&p, const_1, &p, realContext);
      realDivide(&r, &p, &r, realContext);
      realSubtract(&q, &r, &r, realContext);
      // R Q Q
      if(converged) {
        break;
      }
      if(WP34S_RelativeError(&r, &q, (realContext == &ctxtReal39) ? const_1e_37 : const_1e_49, realContext)) { // test if relative error
        converged = true;
      }
      realSubtract(&reg1, const_1, &reg1, realContext);
      realCopy(&r, &q);
    } while(realCompareGreaterThan(&reg1, const_0));
    // LamW0_finish
    realCopy(&r, res);
  }
}


/**************************************************************************/
/* The positive branch of the complex W function.
 *
 * This code is based on Jean-Marc Baillard's HP-41 version from:
 * http://hp41programs.yolasite.com/lambertw.php
 *
 * Register use:
 * .00/.01 z
 * .02/.03 w
 * .04/.05 temporary
 */
void WP34S_ComplexLambertW(const real_t *xReal, const real_t *xImag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  real_t pr, pi, qr, qi, zr, zi, wr, wi, tr, ti;

  realCopy(xReal, &zr), realCopy(xImag, &zi);
  realSetOne(&wr);
  realSetOne(&wi);
  realAdd(xReal, const_1, &pr, realContext);
  realCopy(xImag, &pi);
  if(realIsZero(&zi) && realIsNegative(&zr) && realCompareGreaterEqual(&zr, const__1)) {
    // Close to -1/e, the series is very slow to converge
    realSetOne(&pr);
    realCopy(realIsNegative(&pi) ? const__1 : const_1, &pi);
  }
  else if(!realIsZero(&pr) || !realIsZero(&pi)) {
    lnComplex(&pr, &pi, &pr, &pi, realContext);
    realCopy(&pr, &wr);
    realCopy(&pi, &wi);
  }
  while(1) { // LamW_cloop
    expComplex(&pr, &pi, &qr, &qi, realContext);
    realCopy(&qr, &tr);
    realCopy(&qi, &ti);
    mulComplexComplex(&qr, &qi, &wr, &wi, &qr, &qi, realContext);
    realAdd(&tr, &qr, &tr, realContext);
    realAdd(&ti, &qi, &ti, realContext);
    realSubtract(&qr, &zr, &qr, realContext);
    realSubtract(&qi, &zi, &qi, realContext);
    divComplexComplex(&qr, &qi, &tr, &ti, &qr, &qi, realContext);
    realSubtract(&wr, &qr, &wr, realContext);
    realSubtract(&wi, &qi, &wi, realContext);
    if(WP34S_ComplexAbsError(&wr, &wi, &pr, &pi, const_1e_37, realContext)) {
      break;
    }
    realCopy(&wr, &pr);
    realCopy(&wi, &pi);
  }
  realCopy(&wr, resReal);
  realCopy(&wi, resImag);
}


/**************************************************************************/
/* The inverse W function in both real and complex domains.
 */


void WP34S_InverseW(const real_t *x, real_t *res, realContext_t *realContext) {
  real_t p;

  realExp(x, &p, realContext);
  realMultiply(&p, x, res, realContext);
}


void WP34S_InverseComplexW(const real_t *xReal, const real_t *xImag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  real_t p, q;

  expComplex(xReal, xImag, &p, &q, realContext);
  mulComplexComplex(&p, &q, xReal, xImag, resReal, resImag, realContext);
}


// Orthogonal Polynomials, common function
void WP34S_OrthoPoly(uint16_t kind, const real_t *rX, const real_t *rN, const real_t *rParam, real_t *res, realContext_t *realContext) {
#if defined(OPTION_ORTHO)
  real_t a, b, c, d, i;
  real_t rT0, rT1, incB;
  real_t p, q;
  bool_t incA = false, incC = false;

  // ortho_default
  if(realIsSpecial(rX) || (!realIsAnInteger(rN)) || realIsNegative(rN)) {
    realSetNaN(res);
    return;
  }
  if(realIsZero(rN)) {
    realSetOne(res);
    return;
  }
  // Here we are free from the limitation of ISG since the code is ported to C.
  //if(realCompareGreaterEqual(rN, const_1000)) {
  //  realSetNaN(res);
  //  return;
  //}
  realSetOne(&rT0);
  // Now initialise everything else
  realCopy(const_2, &i);
  realCopy(const_2, &d);
  realSetOne(&c);
  realSetOne(&b);
  realCopy(rX, &rT1);
  realMultiply(rX, const_2, &a, realContext);

  // We must initialise this too
  realSetZero(&incB);

  switch(kind) {
  /**************************************************************************/
  /* Legendre's Pn                                                          */
    case ORTHOPOLY_LEGENDRE_P: {
      realAdd(&a, rX, &a, realContext);
      realMultiply(rX, const_2, &d, realContext);
      goto ortho_allinc;
    }

  /**************************************************************************/
  /* Chebychev's Tn                                                         */
    case ORTHOPOLY_CHEBYSHEV_T: {
      break;
    }

  /**************************************************************************/
  /* Chebychev's Un                                                         */
    case ORTHOPOLY_CHEBYSHEV_U: {
      realAdd(&rT1, rX, &rT1, realContext);
      break;
    }

  /**************************************************************************/
  /* Laguerre's Ln and Ln with parameter alpha                              */
    case ORTHOPOLY_LAGUERRE_L:
    case ORTHOPOLY_LAGUERRE_L_ALPHA: {
      // laguerre_common
      if(realIsSpecial(rParam) || realCompareLessEqual(rParam, const__1)) {
        realSetNaN(res);
        return;
      }
      realAdd(&b, rParam, &b, realContext);
      realAdd(rParam, const_3, &a, realContext);
      realSubtract(&a, rX, &a, realContext);
      realAdd(rParam, const_1, &rT1, realContext);
      realSubtract(&rT1, rX, &rT1, realContext);
      ortho_allinc:
      incA = true;
      realSetOne(&incB);
      incC = true;
      break;
    }

  /**************************************************************************/
  /* Hermite's He (Hn)                                                      */
    case ORTHOPOLY_HERMITE_HE: {
      realCopy(rX, &a);
      realSetOne(&incB);
      break;
    }

  /**************************************************************************/
  /* Hermite's H  (Hnp)                                                     */
    case ORTHOPOLY_HERMITE_H: {
      realAdd(&rT1, rX, &rT1, realContext);
      realCopy(const_2, &b);
      realCopy(const_2, &incB);
      break;
    }
  }

  /**************************************************************************/
  /* Common evaluation code                                                 */
  /* Everything is assumed properly set up at this point.                   */
  /* ortho_common                                                           */
  while(realCompareLessEqual(&i, rN)) { // ortho_loop
    realMultiply(&rT1, &a, &p, realContext);
    realMultiply(&rT0, &b, &q, realContext);
    realCopy(&rT1, &rT0);
    realSubtract(&p, &q, &rT1, realContext);

    if(incC) {
      realAdd(&c, const_1, &c, realContext);
      realDivide(&rT1, &c, &rT1, realContext);
    } // ortho_noC
    if(incA) {
      realAdd(&a, &d, &a, realContext);
    } // ortho_noA
    realAdd(&b, &incB, &b, realContext);
    realAdd(&i, const_1, &i, realContext);
  }
  realCopy(&rT1, res);
#endif //defined(OPTION_ORTHO)
}

