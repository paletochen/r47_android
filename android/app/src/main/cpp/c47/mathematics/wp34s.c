// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file wp34s.c
 ***********************************************/

#include "c47.h"

/******************************************************
 * This functions are borrowed from the WP34S project
 ******************************************************/



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
static void C47_WP34S_SinCosTanTaylor_temp1071 (const real_t *angle, bool_t swap, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext); // angle in radian


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
    convertAngleFromTo(angle, angularMode, amRadian, realContext);
    if(savedContextDigits >= 1071) {
      C47_WP34S_SinCosTanTaylor_temp1071(angle, *swap, (*swap) ? cosOut : sinOut, (*swap) ? sinOut : cosOut, tanOut, realContext); // angle in radian
    }
    else {
      C47_WP34S_SinCosTanTaylor_temp75(angle, *swap, (*swap) ? cosOut : sinOut, (*swap) ? sinOut : cosOut, tanOut, realContext); // angle in radian
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
static void C47_WP34S_Cvt2RadSinCosTan_75temp(const real_t *an, angularMode_t angularMode, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext) {
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
        displayCalcErrorMessage(ERROR_SOLVER_ABORT, REGISTER_T, NIM_REGISTER_LINE);
        break;
      }
    #endif //PC_BUILD

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
  real_t angle, a2, t, j, z, sin, cos, epsilonOrCompare;

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

  doTaylorIterations(a, &angle, &a2, &t, &j, &z, &sin, &cos, sinOut, cosOut, &epsilonOrCompare, doEpsilon, epsilonDigits, realContext);

  realContext->digits = savedContextDigits;

  if(sinOut != NULL) {
    realPlus(&sin, sinOut, realContext);
  }

  if(cosOut != NULL) {
    realPlus(&cos, cosOut, realContext);
  }

  if(tanOut != NULL) {
    if(sinOut == NULL || cosOut == NULL) {
      realSetNaN(tanOut);
    }
    else {
      if(swap) {
        realDivide(&cos, &sin, tanOut, realContext);
      }
      else {
        realDivide(&sin, &cos, tanOut, realContext);
      }
    }
  }
}


//used only by XFN 1071
static void C47_WP34S_Cvt2RadSinCosTan_1071temp(const real_t *an, angularMode_t angularMode, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext) {
  bool_t sinNeg = false, cosNeg = false, swap = false;
  REAL_T_PTR(angle, 1071);


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

  realCopy(an, angle);

  int32_t savedContextDigits = realContext->digits;

  doWP34S_SinCosTanTaylor(angle, &sinNeg, &cosNeg, &swap, sinOut, cosOut, tanOut, angularMode, savedContextDigits, realContext);

  }


void C47_WP34S_Cvt2RadSinCosTan(const real_t *an, angularMode_t angularMode, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext) {
  if(realContext->digits >= 1071) {
    C47_WP34S_Cvt2RadSinCosTan_1071temp(an, angularMode, sinOut, cosOut, tanOut, realContext);
  }
  else {
    C47_WP34S_Cvt2RadSinCosTan_75temp(an, angularMode, sinOut, cosOut, tanOut, realContext);
  }
}


//Used by normal C47 TRIG as well as XFN
// Calculate sin, cos by Taylor series and tan by division, for 1071 contexts
void C47_WP34S_SinCosTanTaylor_temp1071(const real_t *a, bool_t swap, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext) { // a in radian
  REAL_T_PTR(angle, 1071);
  REAL_T_PTR(a2, 1071);
  REAL_T_PTR(t, 1071);
  REAL_T_PTR(j, 1071);
  REAL_T_PTR(z, 1071);
  REAL_T_PTR(sin, 1071);
  REAL_T_PTR(cos, 1071);
  REAL_T_PTR(epsilonOrCompare, 1071);

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
}



void C47_WP34S_SinCosTanTaylor(const real_t *a, bool_t swap, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext) { // a in radian
  if(realContext->digits >= 1071) {
    C47_WP34S_SinCosTanTaylor_temp1071(a, swap, sinOut, cosOut, tanOut, realContext); // a in radian
  }
  else {
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
        displayCalcErrorMessage(ERROR_SOLVER_ABORT, REGISTER_T, NIM_REGISTER_LINE);
        break;
      }
    #endif //PC_BUILD


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


static void WP34S_Atan_75temp(const real_t *x, real_t *angle, realContext_t *realContext) {
  bool_t doEpsilon = false;
  real_t a, b, a2, t, j, z, last, epsilon; //-- added epsilon for convergence;
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

  if(!doAtan( &a, angle, &a2, &t, &j, &z, x, &b, &epsilon, &last,
              doEpsilon, epsilonDigits,
              &doubles, &invert, &neg,
              realContext)) {
    realContext->digits = savedContextDigits;
    return; //NaN
  }
  realContext->digits = savedContextDigits;
}


static void C47do_WP34S_Atan_1071temp(const real_t *x, real_t *angle, realContext_t *realContext) {
  REAL_T_PTR(a, 1071);
  REAL_T_PTR(b, 1071);
  REAL_T_PTR(a2, 1071);
  REAL_T_PTR(t, 1071);
  REAL_T_PTR(j, 1071);
  REAL_T_PTR(z, 1071);
  REAL_T_PTR(last, 1071);
  REAL_T_PTR(epsilon, 1071);
  int doubles = 0;
  int invert;
  int neg;
  if(!doAtan( a, angle, a2, t, j, z, x, b, epsilon, last, true, 1040, &doubles, &invert, &neg, realContext)) {
    return; //NaN
  }
}

void C47_WP34S_Atan(const real_t *x, real_t *angle, realContext_t *realContext) {
  if(realContext->digits >= 1071) {
    C47do_WP34S_Atan_1071temp(x, angle, realContext);
  }
  else {
    WP34S_Atan_75temp(x, angle, realContext);
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


static void WP34S_Atan2_75temp(const real_t *y, const real_t *x, real_t *atan, realContext_t *realContext) {
  real_t r, t;
  if(!doAtan2(y, x, atan, &r, &t, realContext)) {
    return; //NaN
  }
}

static void C47do_WP34S_Atan2_1071temp(const real_t *y, const real_t *x, real_t *atan, realContext_t *realContext) {
  REAL_T_PTR(r, 1071);
  REAL_T_PTR(t, 1071);
  if(!doAtan2(y, x, atan, r, t, realContext)) {
    return; //NaN
  }
}

void C47_WP34S_Atan2(const real_t *y, const real_t *x, real_t *atan, realContext_t *realContext) {
  if(realContext->digits >= 1071) {
    C47do_WP34S_Atan2_1071temp(y, x, atan, realContext);
  }
  else {
    WP34S_Atan2_75temp(y, x, atan, realContext);
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
  realMultiply(x, x, z, realContext);
  realSubtract(const_1, z, z, realContext);
  realSquareRoot(z, z, realContext);
  realAdd(z, const_1, z, realContext);
  realDivide(x, z, z, realContext);
  C47_WP34S_Atan(z, abx, realContext);
  realAdd(abx, abx, angle, realContext);
  return true;
}


static void WP34S_Asin_75temp(const real_t *x, real_t *angle, realContext_t *realContext) {
  real_t abx, z;
  if(!doAsin(x, angle, &abx, &z, realContext)) {
    return; //NaN
  }
}

static void C47do_WP34S_Asin_1071temp(const real_t *x, real_t *angle, realContext_t *realContext) {
  REAL_T_PTR(abx, 1071);
  REAL_T_PTR(z, 1071);
  if(!doAsin(x, angle, abx, z, realContext)) {
    return; //NaN
  }
}

void C47_WP34S_Asin(const real_t *x, real_t *angle, realContext_t *realContext) {
  if(realContext->digits >= 1071) {
    C47do_WP34S_Asin_1071temp(x, angle, realContext);
  }
  else {
    WP34S_Asin_75temp(x, angle, realContext);
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
    realMultiply(x, x, z, realContext);
    realSubtract(const_1, z, z, realContext);
    realSquareRoot(z, z, realContext);
    realSubtract(const_1, x, abx, realContext);
    realDivide(abx, z, z, realContext);
    C47_WP34S_Atan(z, abx, realContext);
    realAdd(abx, abx, angle, realContext);
  }
  return true;
}


static void WP34S_Acos_75temp(const real_t *x, real_t *angle, realContext_t *realContext) {
  real_t abx, z;
  if(!doAcos(x, angle, &abx, &z, realContext)) {
    return; //NaN
  }
}

static void C47do_WP34S_Acos_1071temp(const real_t *x, real_t *angle, realContext_t *realContext) {
  REAL_T_PTR(abx, 1071);
  REAL_T_PTR(z, 1071);
  if(!doAcos(x, angle, abx, z, realContext)) {
    return; //NaN
  }
}

void C47_WP34S_Acos(const real_t *x, real_t *angle, realContext_t *realContext) {
  if(realContext->digits >= 1071) {
    C47do_WP34S_Acos_1071temp(x, angle, realContext);
  }
  else {
    WP34S_Acos_75temp(x, angle, realContext);
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
    WP34S_Mod(xin, const_2, &t, realContext);
    realMultiply(&t, const39_pi, &t, realContext);                            // t = xin·pi
    C47_WP34S_SinCosTanTaylor_temp75(&t, false, &x, NULL, NULL, realContext); // x = sin(xin·pi)

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


void WP34S_ArcSinh(const real_t *x, real_t *res, realContext_t *realContext) {
  real_t a;

  realMultiply(x, x, &a, realContext);   // a = x²
  realAdd(&a, const_1, &a, realContext); // a = x² + 1
  realSquareRoot(&a, &a, realContext);   // a = sqrt(x²+1)
  realAdd(&a, const_1, &a, realContext); // a = sqrt(x²+1)+1
  realDivide(x, &a, &a, realContext);    // a = x / (sqrt(x²+1)+1)
  realAdd(&a, const_1, &a, realContext); // a = x / (sqrt(x²+1)+1) + 1
  realMultiply(x, &a, &a, realContext);  // y = x * (x / (sqrt(x²+1)+1) + 1)
  WP34S_Ln1P(&a, res, realContext);      // res = ln(1 + (x * (x / (sqrt(x²+1)+1) + 1)))
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


void WP34S_ArcTanh(const real_t *x, real_t *res, realContext_t *realContext) {
  real_t y, z;

  if(realIsNaN(x)) {
    realSetNaN(res);
  }

  // Not the obvious formula but more stable...
  realSubtract(const_1, x, &z, realContext);      // z = 1-x
  realDivide(x, &z, &y, realContext);             // y = x / (1-x)
  realMultiply(&y, const_2, &z, realContext);     // z = 2x / (1-x)
  WP34S_Ln1P(&z, &y, realContext);                // y = ln(1 + 2x / (1-x))
  realMultiply(&y, const_1on2, res, realContext); // res = ln(1 + 2x / (1-x)) / 2
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
    realCopy(const__1, res);
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
#endif
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
#endif
}
#endif


void WP34S_Mod(const real_t *x, const real_t *y, real_t *res, realContext_t *realContext) {
#if defined(DMCP_BUILD) && HARDWARE_MODEL == HWM_DM42
  REAL_T_PTR(small, 2139); // Fallback size
  doMod(x, y, res, realContext, 2139, small);
#else
  REAL_T_PTR(temp, 12321);
  doMod(x, y, res, realContext, 6147, temp);
#endif
}


void WP34S_BigMod(const real_t *x, const real_t *y, real_t *res, realContext_t *realContext) {
#if defined(DMCP_BUILD) && HARDWARE_MODEL == HWM_DM42
  REAL_T_PTR(small, 2139); // Fallback size
  doMod(x, y, res, realContext, 2139, small);
#else
  REAL_T_PTR(temp, 12321);
  doMod(x, y, res, realContext, 12321, temp);                  //printf("\n******  ****** NOT MATCHED 2pi !! ****** ******\n");
#endif
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


void WP34S_Erfc(const real_t *x, real_t *res, realContext_t *realContext) {
  real_t p;

  realSquareRoot(const_2, &p, realContext);
  realMultiply(x, &p, &p, realContext);
  realChangeSign(&p);
  WP34S_Cdf_Q(&p, &p, realContext);
  realMultiply(&p, const_2, res, realContext);
}


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
#if !defined(SAVE_SPACE_DM42_12ORTHO)
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
#endif //!defined(SAVE_SPACE_DM42_12ORTHO)
}
