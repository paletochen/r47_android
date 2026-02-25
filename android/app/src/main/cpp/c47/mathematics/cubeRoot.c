// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The C47 Authors

#include "c47.h"

static void curtShoI(void) {
  real_t x;
  int32_t cubeRoot;

  convertShortIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);

  if(realIsPositive(&x)) {
    PowerReal(&x, const_1on3, &x, &ctxtReal39);
  }
  else {
    realSetPositiveSign(&x);
    PowerReal(&x, const_1on3, &x, &ctxtReal39);
    realSetNegativeSign(&x);
  }

  cubeRoot = realToInt32C47(&x);

  if(cubeRoot >= 0) {
    *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_build_value((int64_t)cubeRoot, 0);
  }
  else {
    *(REGISTER_SHORT_INTEGER_DATA(REGISTER_X)) = WP34S_build_value(-(int64_t)cubeRoot, 1);
  }
}



void curtReal(void) {
  real_t x;

  if(!getRegisterAsReal(REGISTER_X, &x))
    return;

  if(realIsInfinite(&x) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function curtReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of curt when flag D is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(realIsPositive(&x)) {
    PowerReal(&x, const_1on3, &x, &ctxtReal39);
  }
  else {
    realSetPositiveSign(&x);
    PowerReal(&x, const_1on3, &x, &ctxtReal39);
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
      PowerReal(&a, const_1on3, resReal, realContext);
    }
    else {
      realSetPositiveSign(&a);
      PowerReal(&a, const_1on3, resReal, realContext);
      realSetNegativeSign(resReal);
    }
    realZero(resImag);
  }
  else {
    realRectangularToPolar(&a, &b, &a, &b, realContext);
    PowerReal(&a, const_1on3, &a, realContext);
    realMultiply(&b, const_1on3, &b, realContext);
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
  real159_t const159_1on3, const159_root3on2;
  realZero((real_t *)&const159_1on3);
  realZero((real_t *)&const159_root3on2);
  // 1/3 = 1 ÷ 3
  realDivide(const_1, const_3, (real_t *)&const159_1on3, realContext);
  // sqrt(3)/2 = sqrt(3) ÷ 2
  realSquareRoot(const_3, (real_t *)&const159_root3on2, realContext);
  realDivide((real_t *)&const159_root3on2, const_2, (real_t *)&const159_root3on2, realContext);

  real159_t xr, xi, zr, zi;
  real159_t x3r, x3i, temp1r, temp1i, temp2r, temp2i, numr, numi, denomr, denomi, quotr, quoti;
  real159_t temp, denom_mag;

  realZero((real_t *)&xr); realZero((real_t *)&xi);
  realZero((real_t *)&zr); realZero((real_t *)&zi);
  realZero((real_t *)&x3r); realZero((real_t *)&x3i);
  realZero((real_t *)&temp1r); realZero((real_t *)&temp1i);
  realZero((real_t *)&temp2r); realZero((real_t *)&temp2i);
  realZero((real_t *)&numr); realZero((real_t *)&numi);
  realZero((real_t *)&denomr); realZero((real_t *)&denomi);
  realZero((real_t *)&quotr); realZero((real_t *)&quoti);
  realZero((real_t *)&temp); realZero((real_t *)&denom_mag);

  realCopy(zReal, (real_t *)&zr);
  realCopy(zImag, (real_t *)&zi);

  if(realIsZero((real_t *)&zr) && realIsZero((real_t *)&zi)) {
    realZero(resReal);
    realZero(resImag);
    return;
  }


  // Handle pure imaginary case
  if(realIsZero((real_t *)&zr)) {
    // cbrt(0 + bi) where b != 0
    // Use polar: bi = |b| * e^(i*π/2*sign(b))
    // cbrt = |b|^(1/3) * e^(i*π/6*sign(b))
    // = |b|^(1/3) * (cos(π/6*sign(b)) + i*sin(π/6*sign(b)))
    // cos(π/6) = sqrt(3)/2, sin(π/6) = 1/2 for positive
    // cos(-π/6) = sqrt(3)/2, sin(-π/6) = -1/2 for negative

    realSetPositiveSign((real_t *)&zi);
    realPower((real_t *)&zi, (real_t *)&const159_1on3, (real_t *)&temp, realContext);

    // xr = |zi|^(1/3) * sqrt(3)/2
    realMultiply((real_t *)&temp, (real_t *)&const159_root3on2, (real_t *)&xr, realContext);

    // xi = |zi|^(1/3) * 1/2 * sign(zi_original)
    realMultiply((real_t *)&temp, const_1on2, (real_t *)&xi, realContext);
    if(realIsNegative(zImag)) {
      realChangeSign((real_t *)&xi);
    }

    realCopy((real_t *)&xr, resReal);
    realCopy((real_t *)&xi, resImag);
    return;
  }


  // Handle real-only case (common for real symmetric matrices)
  if(realIsZero((real_t *)&zi)) {
    if(realIsPositive((real_t *)&zr)) {
      realPower((real_t *)&zr, (real_t *)&const159_1on3, (real_t *)&xr, realContext);
      realZero((real_t *)&xi);
    } else {
      realSetPositiveSign((real_t *)&zr);
      realPower((real_t *)&zr, (real_t *)&const159_1on3, (real_t *)&xr, realContext);
      realSetNegativeSign((real_t *)&xr);
      realZero((real_t *)&xi);
      realSetPositiveSign((real_t *)&zr); // Restore sign
      realSetNegativeSign((real_t *)&zr);
    }
    realCopy((real_t *)&xr, resReal);
    realCopy((real_t *)&xi, resImag);
    return;
  }

  // Initial guess: x_0 = z^(1/3) ≈ |z|^(1/3) * (cos(θ/3) + i*sin(θ/3))
  // Simplified: use |z|^(1/3) in direction of z
  realMultiply((real_t *)&zr, (real_t *)&zr, (real_t *)&temp, realContext);
  realMultiply((real_t *)&zi, (real_t *)&zi, (real_t *)&denom_mag, realContext);
  realAdd((real_t *)&temp, (real_t *)&denom_mag, (real_t *)&temp, realContext);
  realSquareRoot((real_t *)&temp, (real_t *)&temp, realContext); // |z| = sqrt(zr² + zi²)
  realPower((real_t *)&temp, (real_t *)&const159_1on3, (real_t *)&denom_mag, realContext); // |z|^(1/3)

  // Normalize z and scale by |z|^(1/3)
  realDivide((real_t *)&zr, (real_t *)&temp, (real_t *)&xr, realContext);
  realDivide((real_t *)&zi, (real_t *)&temp, (real_t *)&xi, realContext);
  realMultiply((real_t *)&xr, (real_t *)&denom_mag, (real_t *)&xr, realContext);
  realMultiply((real_t *)&xi, (real_t *)&denom_mag, (real_t *)&xi, realContext);

  //char dbg[2000];
  //realToString((real_t *)&xr, dbg); printf("DEBUG curtComplex159: Initial guess xr = %s\n", dbg);
  //realToString((real_t *)&xi, dbg); printf("DEBUG curtComplex159: Initial guess xi = %s\n", dbg);

  // Halley's method iterations: x_{n+1} = x_n * (x_n³ + 2z) / (2x_n³ + z)
  for(int iter = 0; iter < HALLEY_ITER_MAX; iter++) {                                // not optimal to have fixed iterations. Add a convergence check: if |x_n³ - z| / |z| < 10^-160
    // Compute x³ = x * x²
    // First x² = (xr+xi*i)²
    realMultiply((real_t *)&xr, (real_t *)&xr, (real_t *)&temp1r, realContext);
    realMultiply((real_t *)&xi, (real_t *)&xi, (real_t *)&temp1i, realContext);
    realSubtract((real_t *)&temp1r, (real_t *)&temp1i, (real_t *)&temp1r, realContext); // x²_r
    realMultiply((real_t *)&xr, (real_t *)&xi, (real_t *)&temp1i, realContext);
    realAdd((real_t *)&temp1i, (real_t *)&temp1i, (real_t *)&temp1i, realContext); // x²_i

    // Now x³ = x² * x
    realMultiply((real_t *)&temp1r, (real_t *)&xr, (real_t *)&temp, realContext);
    realMultiply((real_t *)&temp1i, (real_t *)&xi, (real_t *)&temp2r, realContext);
    realSubtract((real_t *)&temp, (real_t *)&temp2r, (real_t *)&x3r, realContext);

    realMultiply((real_t *)&temp1r, (real_t *)&xi, (real_t *)&temp, realContext);
    realMultiply((real_t *)&temp1i, (real_t *)&xr, (real_t *)&temp2r, realContext);
    realAdd((real_t *)&temp, (real_t *)&temp2r, (real_t *)&x3i, realContext);

    // Numerator: x³ + 2z
    realMultiply((real_t *)&zr, const_2, (real_t *)&temp, realContext);
    realAdd((real_t *)&x3r, (real_t *)&temp, (real_t *)&numr, realContext);
    realMultiply((real_t *)&zi, const_2, (real_t *)&temp, realContext);
    realAdd((real_t *)&x3i, (real_t *)&temp, (real_t *)&numi, realContext);

    // Denominator: 2x³ + z
    realMultiply((real_t *)&x3r, const_2, (real_t *)&temp, realContext);
    realAdd((real_t *)&temp, (real_t *)&zr, (real_t *)&denomr, realContext);
    realMultiply((real_t *)&x3i, const_2, (real_t *)&temp, realContext);
    realAdd((real_t *)&temp, (real_t *)&zi, (real_t *)&denomi, realContext);

    // Quotient: num / denom = (numr+numi*i) / (denomr+denomi*i)
    // = ((numr*denomr + numi*denomi) + (numi*denomr - numr*denomi)*i) / (denomr² + denomi²)
    realMultiply((real_t *)&denomr, (real_t *)&denomr, (real_t *)&temp, realContext);
    realMultiply((real_t *)&denomi, (real_t *)&denomi, (real_t *)&temp2r, realContext);
    realAdd((real_t *)&temp, (real_t *)&temp2r, (real_t *)&denom_mag, realContext);

    realMultiply((real_t *)&numr, (real_t *)&denomr, (real_t *)&temp, realContext);
    realMultiply((real_t *)&numi, (real_t *)&denomi, (real_t *)&temp2r, realContext);
    realAdd((real_t *)&temp, (real_t *)&temp2r, (real_t *)&temp, realContext);
    realDivide((real_t *)&temp, (real_t *)&denom_mag, (real_t *)&quotr, realContext);

    realMultiply((real_t *)&numi, (real_t *)&denomr, (real_t *)&temp, realContext);
    realMultiply((real_t *)&numr, (real_t *)&denomi, (real_t *)&temp2r, realContext);
    realSubtract((real_t *)&temp, (real_t *)&temp2r, (real_t *)&temp, realContext);
    realDivide((real_t *)&temp, (real_t *)&denom_mag, (real_t *)&quoti, realContext);

    // x_{n+1} = x_n * quot = (xr+xi*i) * (quotr+quoti*i)
    realMultiply((real_t *)&xr, (real_t *)&quotr, (real_t *)&temp, realContext);
    realMultiply((real_t *)&xi, (real_t *)&quoti, (real_t *)&temp2r, realContext);
    realSubtract((real_t *)&temp, (real_t *)&temp2r, (real_t *)&temp1r, realContext);

    realMultiply((real_t *)&xr, (real_t *)&quoti, (real_t *)&temp, realContext);
    realMultiply((real_t *)&xi, (real_t *)&quotr, (real_t *)&temp2r, realContext);
    realAdd((real_t *)&temp, (real_t *)&temp2r, (real_t *)&temp1i, realContext);

    realCopy((real_t *)&temp1r, (real_t *)&xr);
    realCopy((real_t *)&temp1i, (real_t *)&xi);
  }


  // After Halley we have one cube root in (xr, xi), and we do not know which one. To select the principal cube root from the three possibilities: Choose the one with the largest real part, and if competing real parts, use the positive imag part as guid to bring us to the first quadrant
  //char dbg[2000];
  //realToString((real_t *)&xr, dbg); printf("DEBUG curtComplex159: Initial root xr = %s\n", dbg);
  //realToString((real_t *)&xi, dbg); printf("DEBUG curtComplex159: Initial root xi = %s\n", dbg);

  // The three cube roots are: w, w*omega, w*omega^2
  // where omega = -1/2 + i*sqrt(3)/2
  real159_t w1r, w1i, w2r, w2i, w3r, w3i;
  realZero((real_t *)&w1r); realZero((real_t *)&w1i);
  realZero((real_t *)&w2r); realZero((real_t *)&w2i);
  realZero((real_t *)&w3r); realZero((real_t *)&w3i);

  // w1 = current root
  realCopy((real_t *)&xr, (real_t *)&w1r);
  realCopy((real_t *)&xi, (real_t *)&w1i);

  // omega = -1/2 + i*sqrt(3)/2
  real159_t omega_r, omega_i;
  realZero((real_t *)&omega_r);
  realZero((real_t *)&omega_i);
  realCopy(const_1on2, (real_t *)&omega_r);
  realChangeSign((real_t *)&omega_r); // -1/2
  realSquareRoot(const_3, (real_t *)&omega_i, realContext);
  realDivide((real_t *)&omega_i, const_2, (real_t *)&omega_i, realContext); // sqrt(3)/2
  //realToString((real_t *)&omega_r, dbg); printf("DEBUG curtComplex159: omega_r = %s\n", dbg);
  //realToString((real_t *)&omega_i, dbg); printf("DEBUG curtComplex159: omega_i = %s\n", dbg);

  // w2 = w1 * omega
  mulComplexComplex((real_t *)&w1r, (real_t *)&w1i, (real_t *)&omega_r, (real_t *)&omega_i, (real_t *)&w2r, (real_t *)&w2i, realContext);
  //realToString((real_t *)&w2r, dbg); printf("DEBUG curtComplex159: w2r = %s\n", dbg);
  //realToString((real_t *)&w2i, dbg); printf("DEBUG curtComplex159: w2i = %s\n", dbg);

  // w3 = w1 * omega^2 = w1 * conj(omega)
  realChangeSign((real_t *)&omega_i);
  mulComplexComplex((real_t *)&w1r, (real_t *)&w1i, (real_t *)&omega_r, (real_t *)&omega_i, (real_t *)&w3r, (real_t *)&w3i, realContext);
  //realToString((real_t *)&w3r, dbg); printf("DEBUG curtComplex159: w3r = %s\n", dbg);
  //realToString((real_t *)&w3i, dbg); printf("DEBUG curtComplex159: w3i = %s\n", dbg);

  // Select root with largest real part
  realCopy((real_t *)&w1r, (real_t *)&xr);
  realCopy((real_t *)&w1i, (real_t *)&xi);
  //printf("DEBUG curtComplex159: Starting with w1 as candidate\n");

  if(realCompareLessThan((real_t *)&xr, (real_t *)&w2r)) {
    //printf("DEBUG curtComplex159: w2 has larger real part than current, selecting w2\n");
    realCopy((real_t *)&w2r, (real_t *)&xr);
    realCopy((real_t *)&w2i, (real_t *)&xi);
  }
  if(realCompareLessThan((real_t *)&xr, (real_t *)&w3r)) {
    //printf("DEBUG curtComplex159: w3 has larger real part than current, selecting w3\n");
    realCopy((real_t *)&w3r, (real_t *)&xr);
    realCopy((real_t *)&w3i, (real_t *)&xi);
  }
  //realToString((real_t *)&xr, dbg); printf("DEBUG curtComplex159: Final selected root xr = %s\n", dbg);
  //realToString((real_t *)&xi, dbg); printf("DEBUG curtComplex159: Final selected root xi = %s\n", dbg);
  realCopy((real_t *)&xr, resReal);
  realCopy((real_t *)&xi, resImag);
}
#endif //OPTION_CUBIC_159 || OPTION_EIGEN_159


void curtComplex(const real_t *zReal, const real_t *zImag, real_t *resReal, real_t *resImag, realContext_t *realContext) {
  if(realContext->digits <= 75) {
    curtComplex75(zReal, zImag, resReal, resImag, realContext);
#if defined(OPTION_CUBIC_159) || defined(OPTION_EIGEN_159)
  } else
  if(realContext->digits <= 159) {
    curtComplex159(zReal, zImag, resReal, resImag, realContext);
#endif //OPTION_CUBIC_159) || OPTION_EIGEN_159
  } else {
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
