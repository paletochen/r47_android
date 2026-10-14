// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The C47 Authors

#include "c47.h"

static void curtShoI(void) {
  real_t x;
  int32_t cubeRoot;

  convertShortIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);

  if(realIsPositive(&x)) {
    PowerReal(&x, const39_1on3, &x, &ctxtReal39);
  }
  else {
    realSetPositiveSign(&x);
    PowerReal(&x, const39_1on3, &x, &ctxtReal39);
    realSetNegativeSign(&x);
  }

  cubeRoot = realToInt32C47(&x, NULL);

  if(cubeRoot >= 0) {
    *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_build_value((int64_t)cubeRoot, 0);
  }
  else {
    *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_build_value(-(int64_t)cubeRoot, 1);
  }
}



void curtReal(void) {
  real_t x;

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }

  if(realIsInfinite(&x) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function curtReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of curt when flag D is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(realIsPositive(&x)) {
    PowerReal(&x, const39_1on3, &x, &ctxtReal39);
  }
  else {
    realSetPositiveSign(&x);
    PowerReal(&x, const39_1on3, &x, &ctxtReal39);
    realSetNegativeSign(&x);
  }
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}



static  void curtLonI(void) {
  rootLonI(3);
}



void curtCplx(void) {
  real_t a, b;

  if(getRegisterAsComplex(REGISTER_X, &a, &b)) {
    curtComplex(&a, &b, &a, &b, &ctxtReal39);
    convertComplexToResultRegister(&a, &b, REGISTER_X);
  }
}



void curtComplex75(const real_t *real, const real_t *imag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  real_t a, b;

  realCopy(real, &a);
  realCopy(imag, &b);

  if(realIsZero(&b)) {
    if(realIsPositive(&a)) {
      PowerReal(&a, const39_1on3, resReal, realContext);
    }
    else {
      realSetPositiveSign(&a);
      PowerReal(&a, const39_1on3, resReal, realContext);
      realSetNegativeSign(resReal);
    }
    realSetZero(resImag);
  }
  else {
    realRectangularToPolar(&a, &b, &a, &b, realContext);
    PowerReal(&a, const39_1on3, &a, realContext);
    realMultiply(&b, const39_1on3, &b, realContext);
    realPolarToRectangular(&a, &b, resReal, resImag, realContext);
  }
}


#if defined(OPTION_CUBIC_159) || defined(OPTION_EIGEN_159)
// Complex cube root using Halley's method: x_{n+1} = x_n * (x_n³ + 2z) / (2x_n³ + z)
//   The point of using this is we cannot use complex trig without a 159 digit precision Taylor et al.
void curtComplex159(const real_t *zReal, const real_t *zImag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  #define HALLEY_ITER_MAX 15 //8
  //printf("DEBUG curtComplex159: realContext->digits = %d\n", realContext->digits);
  #if defined(PC_BUILD)
    if(realContext->digits != 159) {
      debugf("==============\nPRECISION ISSUE IN curtComplex159==============\\n");
    }
  #endif //PC_BUILD

  // create 159 constants
  REAL_T_PTR(const159_1on3, 159);
  REAL_T_PTR(const159_root3on2, 159);
  // 1/3 = 1 ÷ 3
  realDivide(const_1, const_3, const159_1on3, realContext);
  // sqrt(3)/2 = sqrt(3) ÷ 2
  realSquareRoot(const_3, const159_root3on2, realContext);
  realDivide(const159_root3on2, const_2, const159_root3on2, realContext);

  REAL_T_PTR(xr, 159);
  REAL_T_PTR(xi, 159);
  REAL_T_PTR(zr, 159);
  REAL_T_PTR(zi, 159);
  REAL_T_PTR(x3r, 159);
  REAL_T_PTR(x3i, 159);
  REAL_T_PTR(temp1r, 159);
  REAL_T_PTR(temp1i, 159);
  REAL_T_PTR(temp2r, 159);
  REAL_T_PTR(numr, 159);
  REAL_T_PTR(numi, 159);
  REAL_T_PTR(denomr, 159);
  REAL_T_PTR(denomi, 159);
  REAL_T_PTR(quotr, 159);
  REAL_T_PTR(quoti, 159);
  REAL_T_PTR(temp, 159);
  REAL_T_PTR(denom_mag, 159);

  realSetZero(xr);
  realSetZero(xi);
  realSetZero(zr);
  realSetZero(zi);
  realSetZero(x3r);
  realSetZero(x3i);
  realSetZero(temp1r);
  realSetZero(temp1i);
  realSetZero(temp2r);
  realSetZero(numr);
  realSetZero(numi);
  realSetZero(denomr);
  realSetZero(denomi);
  realSetZero(quotr);
  realSetZero(quoti);
  realSetZero(temp);
  realSetZero(denom_mag);

  realCopy(zReal, zr);
  realCopy(zImag, zi);

  if(realIsZero(zr) && realIsZero(zi)) {
    realSetZero(resReal);
    realSetZero(resImag);
    return;
  }


  // Handle pure imaginary case
  if(realIsZero(zr)) {
    // cbrt(0 + bi) where b != 0
    // Use polar: bi = |b| * e^(i*π/2*sign(b))
    // cbrt = |b|^(1/3) * e^(i*π/6*sign(b))
    // = |b|^(1/3) * (cos(π/6*sign(b)) + i*sin(π/6*sign(b)))
    // cos(π/6) = sqrt(3)/2, sin(π/6) = 1/2 for positive
    // cos(-π/6) = sqrt(3)/2, sin(-π/6) = -1/2 for negative

    realSetPositiveSign(zi);
    realPower(zi, const159_1on3, temp, realContext);

    // xr = |zi|^(1/3) * sqrt(3)/2
    realMultiply(temp, const159_root3on2, xr, realContext);

    // xi = |zi|^(1/3) * 1/2 * sign(zi_original)
    realMultiply(temp, const_1on2, xi, realContext);
    if(realIsNegative(zImag)) {
      realChangeSign(xi);
    }

    realCopy(xr, resReal);
    realCopy(xi, resImag);
    return;
  }


  // Handle real-only case (common for real symmetric matrices)
  if(realIsZero(zi)) {
    if(realIsPositive(zr)) {
      realPower(zr, const159_1on3, xr, realContext);
      realSetZero(xi);
    }
    else {
      realSetPositiveSign(zr); // Temorarily set to positive, handle the sign later after the power function on positive only
      realPower(zr, const159_1on3, xr, realContext);
      realSetNegativeSign(xr);
      realSetZero(xi);
      realSetNegativeSign(zr); // Restore to negative (input was negative real, we're handling cbrt of negative)
    }
    realCopy(xr, resReal);
    realCopy(xi, resImag);
    return;
  }

  // Initial guess: x_0 = z^(1/3) ≈ |z|^(1/3) * (cos(θ/3) + i*sin(θ/3))
  // Simplified: use |z|^(1/3) in direction of z
  realMultiply(zr, zr, temp, realContext);
  realMultiply(zi, zi, denom_mag, realContext);
  realAdd(temp, denom_mag, temp, realContext);
  realSquareRoot(temp, temp, realContext);                // |z| = sqrt(zr² + zi²)
  realPower(temp, const159_1on3, denom_mag, realContext); // |z|^(1/3)

  // Normalize z and scale by |z|^(1/3)
  realDivide(zr, temp, xr, realContext);
  realDivide(zi, temp, xi, realContext);
  realMultiply(xr, denom_mag, xr, realContext);
  realMultiply(xi, denom_mag, xi, realContext);

  //char dbg[2000];
  //realToString(xr, dbg); printf("DEBUG curtComplex159: Initial guess xr = %s\n", dbg);
  //realToString(xi, dbg); printf("DEBUG curtComplex159: Initial guess xi = %s\n", dbg);

  // Halley's method iterations: x_{n+1} = x_n * (x_n³ + 2z) / (2x_n³ + z)
  for(int iter = 0; iter < HALLEY_ITER_MAX; iter++) {                                // not optimal to have fixed iterations. Add a convergence check: if |x_n³ - z| / |z| < 10^-160
    // Compute x³ = x * x²
    // First x² = (xr+xi*i)²
    realMultiply(xr, xr, temp1r, realContext);
    realMultiply(xi, xi, temp1i, realContext);
    realSubtract(temp1r, temp1i, temp1r, realContext); // x²_r
    realMultiply(xr, xi, temp1i, realContext);
    realAdd(temp1i, temp1i, temp1i, realContext); // x²_i

    // Now x³ = x² * x
    realMultiply(temp1r, xr, temp, realContext);
    realMultiply(temp1i, xi, temp2r, realContext);
    realSubtract(temp, temp2r, x3r, realContext);

    realMultiply(temp1r, xi, temp, realContext);
    realMultiply(temp1i, xr, temp2r, realContext);
    realAdd(temp, temp2r, x3i, realContext);

    // Numerator: x³ + 2z
    realMultiply(zr, const_2, temp, realContext);
    realAdd(x3r, temp, numr, realContext);
    realMultiply(zi, const_2, temp, realContext);
    realAdd(x3i, temp, numi, realContext);

    // Denominator: 2x³ + z
    realMultiply(x3r, const_2, temp, realContext);
    realAdd(temp, zr, denomr, realContext);
    realMultiply(x3i, const_2, temp, realContext);
    realAdd(temp, zi, denomi, realContext);

    // Quotient: num / denom = (numr+numi*i) / (denomr+denomi*i)
    // = ((numr*denomr + numi*denomi) + (numi*denomr - numr*denomi)*i) / (denomr² + denomi²)
    realMultiply(denomr, denomr, temp, realContext);
    realMultiply(denomi, denomi, temp2r, realContext);
    realAdd(temp, temp2r, denom_mag, realContext);

    realMultiply(numr, denomr, temp, realContext);
    realMultiply(numi, denomi, temp2r, realContext);
    realAdd(temp, temp2r, temp, realContext);
    realDivide(temp, denom_mag, quotr, realContext);

    realMultiply(numi, denomr, temp, realContext);
    realMultiply(numr, denomi, temp2r, realContext);
    realSubtract(temp, temp2r, temp, realContext);
    realDivide(temp, denom_mag, quoti, realContext);

    // x_{n+1} = x_n * quot = (xr+xi*i) * (quotr+quoti*i)
    realMultiply(xr, quotr, temp, realContext);
    realMultiply(xi, quoti, temp2r, realContext);
    realSubtract(temp, temp2r, temp1r, realContext);

    realMultiply(xr, quoti, temp, realContext);
    realMultiply(xi, quotr, temp2r, realContext);
    realAdd(temp, temp2r, temp1i, realContext);

    realCopy(temp1r, xr);
    realCopy(temp1i, xi);
  }


  // After Halley we have one cube root in (xr, xi), and we do not know which one. To select the principal cube root from the three possibilities: Choose the one with the largest real part, and if competing real parts, use the positive imag part as guid to bring us to the first quadrant
  //char dbg[2000];
  //realToString(xr, dbg); printf("DEBUG curtComplex159: Initial root xr = %s\n", dbg);
  //realToString(xi, dbg); printf("DEBUG curtComplex159: Initial root xi = %s\n", dbg);

  // The three cube roots are: w, w*omega, w*omega^2
  // where omega = -1/2 + i*sqrt(3)/2
  REAL_T_PTR(w1r, 159);
  REAL_T_PTR(w1i, 159);
  REAL_T_PTR(w2r, 159);
  REAL_T_PTR(w2i, 159);
  REAL_T_PTR(w3r, 159);
  REAL_T_PTR(w3i, 159);
  realSetZero(w1r);
  realSetZero(w1i);
  realSetZero(w2r);
  realSetZero(w2i);
  realSetZero(w3r);
  realSetZero(w3i);

  // w1 = current root
  realCopy(xr, w1r);
  realCopy(xi, w1i);

  // omega = -1/2 + i*sqrt(3)/2
  REAL_T_PTR(omega_r, 159);
  REAL_T_PTR(omega_i, 159);
  realSetZero(omega_r);
  realSetZero(omega_i);
  realCopy(const_1on2, omega_r);
  realChangeSign(omega_r); // -1/2
  realSquareRoot(const_3, omega_i, realContext);
  realMultiply(omega_i, const_1on2, omega_i, realContext); // sqrt(3)/2
  //realToString(omega_r, dbg); printf("DEBUG curtComplex159: omega_r = %s\n", dbg);
  //realToString(omega_i, dbg); printf("DEBUG curtComplex159: omega_i = %s\n", dbg);

  // w2 = w1 * omega
  mulComplexComplex(w1r, w1i, omega_r, omega_i, w2r, w2i, realContext);
  //realToString(w2r, dbg); printf("DEBUG curtComplex159: w2r = %s\n", dbg);
  //realToString(w2i, dbg); printf("DEBUG curtComplex159: w2i = %s\n", dbg);

  // w3 = w1 * omega^2 = w1 * conj(omega)
  realChangeSign(omega_i);
  mulComplexComplex(w1r, w1i, omega_r, omega_i, w3r, w3i, realContext);
  //realToString(w3r, dbg); printf("DEBUG curtComplex159: w3r = %s\n", dbg);
  //realToString(w3i, dbg); printf("DEBUG curtComplex159: w3i = %s\n", dbg);

  // Select root with largest real part
  realCopy(w1r, xr);
  realCopy(w1i, xi);
  //printf("DEBUG curtComplex159: Starting with w1 as candidate\n");

  if(realCompareLessThan(xr, w2r)) {
    //printf("DEBUG curtComplex159: w2 has larger real part than current, selecting w2\n");
    realCopy(w2r, xr);
    realCopy(w2i, xi);
  }
  if(realCompareLessThan(xr, w3r)) {
    //printf("DEBUG curtComplex159: w3 has larger real part than current, selecting w3\n");
    realCopy(w3r, xr);
    realCopy(w3i, xi);
  }
  //realToString(xr, dbg); printf("DEBUG curtComplex159: Final selected root xr = %s\n", dbg);
  //realToString(xi, dbg); printf("DEBUG curtComplex159: Final selected root xi = %s\n", dbg);
  realCopy(xr, resReal);
  realCopy(xi, resImag);
}
#endif //OPTION_CUBIC_159 || OPTION_EIGEN_159


void curtComplex(const real_t *zReal, const real_t *zImag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  if(realContext->digits <= 75) {
    curtComplex75(zReal, zImag, resReal, resImag, realContext);
  #if defined(OPTION_CUBIC_159) || defined(OPTION_EIGEN_159)
  }
  else if(realContext->digits <= 159) {
    curtComplex159(zReal, zImag, resReal, resImag, realContext);
  #endif //OPTION_CUBIC_159) || OPTION_EIGEN_159
  }
  else {
    sprintf(errorMessage, "Exceed digits :curtComplex159: %d", (int)(realContext->digits));
    displayBugScreen(errorMessage);
  }
}




/********************************************//**
 * \brief regX ==> regL and curt(regX) ==> regX
 * enables stack lift and refreshes the stack
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnCubeRoot(uint16_t unusedButMandatoryParameter) {
  processIntRealComplexMonadicFunction(&curtReal, &curtCplx, &curtShoI, &curtLonI);
}
