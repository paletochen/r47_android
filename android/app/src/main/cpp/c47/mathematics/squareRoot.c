// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file squareRoot.c
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
    convertComplexPartsToResultRegister(const_0, &a, REGISTER_X);
  }
  else {
    *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_intSqrt(*(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)));
  }
}



static void sqrtReal(void) {
  real_t a;

  if(!getRegisterAsReal(REGISTER_X, &a)) {
    return;
  }

  if(realIsInfinite(&a) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function sqrtReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of sqrt when flag SPCRES is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(realIsZero(&a)) {
    convertRealToResultRegister(&a, REGISTER_X, amNone);
  }
  else if(realIsPositive(&a)) {
    real34_t root;

    if(rootRegisterToReal34(REGISTER_X, 2u, true, &root)) {
      convertReal34ToReal34Register(&root, REGISTER_X);
      return;
    }
    realSquareRoot(&a, &a, &ctxtReal39);
    convertRealToResultRegister(&a, REGISTER_X, amNone);
  }
  else if(getFlag(FLAG_CPXRES)) {
    realSetPositiveSign(&a);
    realSquareRoot(&a, &a, &ctxtReal39);
    convertComplexPartsToResultRegister(const_0, &a, REGISTER_X);
  }
  else {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, STD_SQUARE_ROOT STD_x_UNDER_ROOT " doesn't work on a negative real when flag I is not set!");
      moreInfoOnError("In function sqrtReal:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
}



//input in X, output in X
void rootLonI(int32_t n) {
  longInteger_t lgInt, rem, root;
  real34_t roundedRoot;

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
  if(n == 2 && longIntegerIsNegative(lgInt) && getFlag(FLAG_CPXRES)) {
    longInteger_t magnitude;

    longIntegerInit(magnitude);
    longIntegerCopy(lgInt, magnitude);
    longIntegerSetPositiveSign(magnitude);
    const bool_t exactRoot = rootLongIntegerToReal34(magnitude, 2u, &roundedRoot);
    longIntegerFree(magnitude);
    if(exactRoot) {
      convertReal34ToComplex34Register(const34_0, &roundedRoot, REGISTER_X);
      goto end;
    }
  }
  if(rootLongIntegerToReal34(lgInt, n, &roundedRoot)) {
    convertReal34ToReal34Register(&roundedRoot, REGISTER_X);
    goto end;
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
//    if(!getRegisterAsReal(REGISTER_X, &x)) {
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
  cplx_t a;

  if(getRegisterAsComplex(REGISTER_X, &a)) {
    sqrtComplex(&a, &a, &ctxtReal39);
    convertComplexToResultRegister(&a, REGISTER_X);
  }
}



void sqrtComplex(const cplx_t *z, cplx_t *res, realContext_t *realContext) {
  if(realIsZero(&z->Imag) && realIsNegative(&z->Real)) {
    realMinus(&z->Real, &res->Imag, realContext);
    realSquareRoot(&res->Imag, &res->Imag, realContext);
    realSetZero(&res->Real);
  }
  else if(realIsZero(&z->Imag)) {
    realSquareRoot(&z->Real, &res->Real, realContext);
    realSetZero(&res->Imag);
  }
  else if(realIsInfinite(&z->Imag)) {  // whatever the real part, NaN included
    realCopyAbs(&z->Imag, &res->Real);
    realCopy(&z->Imag, &res->Imag);
  }
  else {
    // t = sqrt((|x| + |z|) / 2) adds two terms of one sign, so no digits cancel, next to the negative real axis included. The other part is y / 2t: the root is
    // t + i y/2t for x >= 0, and |y|/2t + i t with the sign of y for x < 0. A NaN part comes out as NaN + i NaN.
    real_t t, u;
    const bool_t imagNegative = realIsNegative(&z->Imag);
    if(realIsInfinite(&z->Real) && !realIsNaN(&z->Imag)) {  // t is infinite and y / 2t is 0: the formula gives the same, with zeros at the smallest exponent
      realCopyAbs(&z->Real, &t);
      realSetZero(&u);
    }
    else {
      complexMagnitude(z, &t, realContext);
      realCopyAbs(&z->Real, &u);
      realAdd(&u, &t, &t, realContext);
      realDivideBy2(&t, realContext);
      realSquareRoot(&t, &t, realContext);
      realDivide(&z->Imag, &t, &u, realContext);
      realDivideBy2(&u, realContext);
    }
    if(realIsNegative(&z->Real)) {
      realCopyAbs(&u, &res->Real);
      realCopy(&t, &res->Imag);
      if(imagNegative) {
        realSetNegativeSign(&res->Imag);
      }
    }
    else {
      realCopy(&t, &res->Real);
      realCopy(&u, &res->Imag);
    }
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
void sqrtComplex159(const cplx159_t *z, cplx159_t *res, realContext_t *realContext) {
  REAL_T_PTR(xr, 159);
  REAL_T_PTR(xi, 159);
  REAL_T_PTR(zr, 159);
  REAL_T_PTR(zi, 159);
  REAL_T_PTR(temp1, 159);
  REAL_T_PTR(temp2, 159);
  REAL_T_PTR(temp3, 159);
  REAL_T_PTR(temp4, 159);
  REAL_T_PTR(denom, 159);
  REAL_T_PTR(mag, 159);

  realSetZero(xr);
  realSetZero(xi);
  realSetZero(zr);
  realSetZero(zi);
  realSetZero(temp1);
  realSetZero(temp2);
  realSetZero(temp3);
  realSetZero(temp4);
  realSetZero(denom);
  realSetZero(mag);

  // Copy inputs
  realCopy(&z->Real, zr);
  realCopy(&z->Imag, zi);

  if(realIsZero(zr) && realIsZero(zi)) {
    realSetZero(&res->Real);
    realSetZero(&res->Imag);
    return;
  }

  // Initial guess: x_0 based on |z|^(1/2)
  // mag = sqrt(zr² + zi²)
  realMultiply(zr, zr, temp1, realContext);
  realMultiply(zi, zi, temp2, realContext);
  realAdd(temp1, temp2, mag, realContext);
  realSquareRoot(mag, mag, realContext);

  // Initial guess: xr = sqrt((mag + zr)/2), xi = sign(zi)*sqrt((mag - zr)/2)
  // With zi zero or below the precision of zr, mag rounds to one unit below |zr| and one radicand to a negative value. It starts at zero instead, and the first
  // Newton step forms that part.
  realAdd(mag, zr, temp1, realContext);
  realDivideBy2(temp1, realContext);
  if(realIsNegative(temp1)) {
    realSetZero(temp1);
  }
  realSquareRoot(temp1, xr, realContext);

  realSubtract(mag, zr, temp2, realContext);
  realDivideBy2(temp2, realContext);
  if(realIsNegative(temp2)) {
    realSetZero(temp2);
  }
  realSquareRoot(temp2, xi, realContext);
  if(realIsNegative(zi)) {
    realChangeSign(xi);
  }

  // Newton-Raphson iterations: x_{n+1} = (x_n + z/x_n) / 2
  for(int iter = 0; iter < 10; iter++) {
    // Compute z/x_n = (zr+zi*i) / (xr+xi*i)
    // = ((zr*xr + zi*xi) + (zi*xr - zr*xi)*i) / (xr² + xi²)

    // denom = xr² + xi²
    realMultiply(xr, xr, temp1, realContext);
    realMultiply(xi, xi, temp2, realContext);
    realAdd(temp1, temp2, denom, realContext);

    // temp3 = (zr*xr + zi*xi) / denom
    realMultiply(zr, xr, temp1, realContext);
    realMultiply(zi, xi, temp2, realContext);
    realAdd(temp1, temp2, temp1, realContext);
    realDivide(temp1, denom, temp3, realContext);

    // temp4 = (zi*xr - zr*xi) / denom
    realMultiply(zi, xr, temp1, realContext);
    realMultiply(zr, xi, temp2, realContext);
    realSubtract(temp1, temp2, temp1, realContext);
    realDivide(temp1, denom, temp4, realContext);

    // x_{n+1} = (x_n + z/x_n) / 2
    realAdd(xr, temp3, xr, realContext);
    realDivideBy2(xr, realContext);

    realAdd(xi, temp4, xi, realContext);
    realDivideBy2(xi, realContext);
  }

  realCopy(xr, &res->Real);
  realCopy(xi, &res->Imag);
}
#endif //OPTION_SQUARE_159 || OPTION_CUBIC_159 || OPTION_EIGEN_159



/********************************************//**
 * \brief regX ==> regL and sqrt(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnSquareRoot(uint16_t unusedButMandatoryParameter) {
  const uint32_t type = getRegisterDataType(REGISTER_X);

  if(type == dtReal34Matrix || type == dtComplex34Matrix) {
#if defined(OPTION_EIGEN)
    fnMatrixSquareRoot(NOPARAM);
#else
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE);
#endif // OPTION_EIGEN
  }
  else {
    processIntRealComplexMonadicFunction(&sqrtReal, &sqrtCplx, &sqrtShoI, &sqrtLonI);
  }
}
