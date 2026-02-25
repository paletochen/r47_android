// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file sqrt.c
 ***********************************************/

#include "c47.h"

static void sqrtShoI(void) {
  int32_t signValue;

  WP34S_extract_value(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)), &signValue);
  if(signValue && getFlag(FLAG_CPXRES)) {
    real_t a;
    convertShortIntegerRegisterToReal(REGISTER_X, &a, &ctxtReal39);
    reallocateRegister(REGISTER_X, dtComplex34, 0, amNone);
    realSetPositiveSign(&a);
    realSquareRoot(&a, &a, &ctxtReal39);
    convertComplexToResultRegister(const_0, &a, REGISTER_X);
  }
  else {
    *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_intSqrt(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)));
  }
}



static void sqrtReal(void) {
  real_t a;

  if(!getRegisterAsReal(REGISTER_X, &a))
    return;

  if(realIsInfinite(&a) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function sqrtReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of sqrt when flag D is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(realIsPositive(&a)) {
    realSquareRoot(&a, &a, &ctxtReal39);
    convertRealToResultRegister(&a, REGISTER_X, amNone);
  }
  else if(getFlag(FLAG_CPXRES)) {
    realSetPositiveSign(&a);
    realSquareRoot(&a, &a, &ctxtReal39);
    convertComplexToResultRegister(const_0, &a, REGISTER_X);
  }
  else {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, STD_SQUARE_ROOT STD_x_UNDER_ROOT " doesn't work on a negative real when flag I is not set!");
      moreInfoOnError("In function sqrtReal:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}



//input in X, output in X
void rootLonI(int32_t n) {
  longInteger_t lgInt, rem, root;

  if(!getRegisterAsLongInt(REGISTER_X, lgInt, NULL)) {
    goto end;
  }
  bool_t nIsOdd = (n % 2) != 0;
  if(n > 0 && (nIsOdd || (!nIsOdd && !longIntegerIsNegative(lgInt)))) {
    longIntegerInit(rem);
    longIntegerInit(root);
    mpz_rootrem(root, rem, lgInt, n); // square root
    if(longIntegerIsZero(rem)) {
      convertLongIntegerToLongIntegerRegister(root, REGISTER_X);
      longIntegerFree(rem);
      longIntegerFree(root);
      goto end;
    }
    longIntegerFree(rem);
    longIntegerFree(root);
  }
  //fall through to Real calculation with errors
  if(n == 2) {
    sqrtReal();
  }
  else if(n == 3) {
    curtReal();
  }
end:
  longIntegerFree(lgInt);
//This section is not needed as only sqrt and qubert are using this helper function
//  else { // fallthrough n is even and x < 1
//    real_t x;
//    if (!getRegisterAsReal(REGISTER_X, &x)) {
//      return;
//    }
//    real_t nn;
//    if(n <= 2147483647) {
//      int32ToReal(n, &nn);
//      xthRootReal(&x, &nn, &ctxtReal75);
//    }
//  }
}



static  void sqrtLonI(void) {
  rootLonI(2);
}



static void sqrtCplx(void) {
  real_t a, b;

  if(getRegisterAsComplex(REGISTER_X, &a, &b)) {
    sqrtComplex(&a, &b, &a, &b, &ctxtReal39);
    convertComplexToResultRegister(&a, &b, REGISTER_X);
  }
}



void sqrtComplex75(const real_t *real, const real_t *imag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  if(realIsZero(imag) && realIsNegative(real)) {
    realMinus(real, resImag, realContext);
    realSquareRoot(resImag, resImag, realContext);
    realZero(resReal);
  }
  else if(realIsZero(imag)) {
    realSquareRoot(real, resReal, realContext);
    realZero(resImag);
  }
  else {
    realRectangularToPolar(real, imag, resReal, resImag, realContext);
    realSquareRoot(resReal, resReal, realContext);
    realMultiply(resImag, const_1on2, resImag, realContext);
    realPolarToRectangular(resReal, resImag, resReal, resImag, realContext);
  }
}


#if defined(OPTION_SQUARE_159) || defined(OPTION_CUBIC_159) || defined(OPTION_EIGEN_159)
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
// This re-write is needed as the mixing of decNumber types cannot deal with the real_t temporary variable withinn mulComplexComplex().
// The two complex root functions are written without trig as the current WP34 Taylor method cannot handle the 159 series is needed.
// A first write of a method using the WP34 Taylor lib is included below, in which case the sqrtComplex159, curtComplex159 and mulComplexComplex159 can be removed.
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Complex square root using Newton-Raphson: x_{n+1} = (x_n + z/x_n) / 2
void sqrtComplex159(const real_t *zReal, const real_t *zImag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  real159_t xr, xi, zr, zi;
  real159_t temp1, temp2, temp3, temp4, denom, mag;
  
  realZero((real_t *)&xr); realZero((real_t *)&xi);
  realZero((real_t *)&zr); realZero((real_t *)&zi);
  realZero((real_t *)&temp1); realZero((real_t *)&temp2);
  realZero((real_t *)&temp3); realZero((real_t *)&temp4);
  realZero((real_t *)&denom); realZero((real_t *)&mag);
  
  // Copy inputs
  realCopy(zReal, (real_t *)&zr);
  realCopy(zImag, (real_t *)&zi);
  
  if(realIsZero((real_t *)&zr) && realIsZero((real_t *)&zi)) {
    realZero(resReal);
    realZero(resImag);
    return;
  }
  
  // Initial guess: x_0 based on |z|^(1/2)
  // mag = sqrt(zr² + zi²)
  realMultiply((real_t *)&zr, (real_t *)&zr, (real_t *)&temp1, realContext);
  realMultiply((real_t *)&zi, (real_t *)&zi, (real_t *)&temp2, realContext);
  realAdd((real_t *)&temp1, (real_t *)&temp2, (real_t *)&mag, realContext);
  realSquareRoot((real_t *)&mag, (real_t *)&mag, realContext);
  
  // Initial guess: xr = sqrt((mag + zr)/2), xi = sign(zi)*sqrt((mag - zr)/2)
  realAdd((real_t *)&mag, (real_t *)&zr, (real_t *)&temp1, realContext);
  realDivideBy2((real_t *)&temp1, realContext);
  realSquareRoot((real_t *)&temp1, (real_t *)&xr, realContext);
  
  realSubtract((real_t *)&mag, (real_t *)&zr, (real_t *)&temp2, realContext);
  realDivideBy2((real_t *)&temp2, realContext);
  realSquareRoot((real_t *)&temp2, (real_t *)&xi, realContext);
  if(realIsNegative((real_t *)&zi)) {
    realChangeSign((real_t *)&xi);
  }
  
  // Newton-Raphson iterations: x_{n+1} = (x_n + z/x_n) / 2
  for(int iter = 0; iter < 10; iter++) {
    // Compute z/x_n = (zr+zi*i) / (xr+xi*i)
    // = ((zr*xr + zi*xi) + (zi*xr - zr*xi)*i) / (xr² + xi²)
    
    // denom = xr² + xi²
    realMultiply((real_t *)&xr, (real_t *)&xr, (real_t *)&temp1, realContext);
    realMultiply((real_t *)&xi, (real_t *)&xi, (real_t *)&temp2, realContext);
    realAdd((real_t *)&temp1, (real_t *)&temp2, (real_t *)&denom, realContext);
    
    // temp3 = (zr*xr + zi*xi) / denom
    realMultiply((real_t *)&zr, (real_t *)&xr, (real_t *)&temp1, realContext);
    realMultiply((real_t *)&zi, (real_t *)&xi, (real_t *)&temp2, realContext);
    realAdd((real_t *)&temp1, (real_t *)&temp2, (real_t *)&temp1, realContext);
    realDivide((real_t *)&temp1, (real_t *)&denom, (real_t *)&temp3, realContext);
    
    // temp4 = (zi*xr - zr*xi) / denom
    realMultiply((real_t *)&zi, (real_t *)&xr, (real_t *)&temp1, realContext);
    realMultiply((real_t *)&zr, (real_t *)&xi, (real_t *)&temp2, realContext);
    realSubtract((real_t *)&temp1, (real_t *)&temp2, (real_t *)&temp1, realContext);
    realDivide((real_t *)&temp1, (real_t *)&denom, (real_t *)&temp4, realContext);
    
    // x_{n+1} = (x_n + z/x_n) / 2
    realAdd((real_t *)&xr, (real_t *)&temp3, (real_t *)&xr, realContext);
    realDivideBy2((real_t *)&xr, realContext);
    
    realAdd((real_t *)&xi, (real_t *)&temp4, (real_t *)&xi, realContext);
    realDivideBy2((real_t *)&xi, realContext);
  }
  
  realCopy((real_t *)&xr, resReal);
  realCopy((real_t *)&xi, resImag);
}
#endif //OPTION_SQUARE_159 || OPTION_CUBIC_159 || OPTION_EIGEN_159


void sqrtComplex(const real_t *real, const real_t *imag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  if(realContext->digits <= 75) {
    sqrtComplex75(real, imag, resReal, resImag, realContext);
#if defined(OPTION_SQUARE_159) ||defined(OPTION_CUBIC_159) || defined(OPTION_EIGEN_159)
  } else
  if(realContext->digits <= 159) {
    sqrtComplex159(real, imag, resReal, resImag, realContext);
#endif //OPTION_SQUARE_159 || OPTION_CUBIC_159) || OPTION_EIGEN_159
  } else {
    sprintf(errorMessage, "Exceed digits :sqrtComplex: %d", (int)(realContext->digits));
    displayBugScreen(errorMessage);
  }
}



/********************************************//**
 * \brief regX ==> regL and sqrt(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSquareRoot(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexMonadicFunction(&sqrtReal, &sqrtCplx, &sqrtShoI, &sqrtLonI);
}
