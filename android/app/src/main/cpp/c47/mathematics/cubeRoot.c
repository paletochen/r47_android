// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The C47 Authors

#include "c47.h"

/********************************************//**
 * \brief Replaces an approximation with an exact Real34 cube root when one exists
 *
 * A nearest 34-digit candidate is reduced before verification. An exact decimal cube
 * with a 34-digit coefficient has a reduced root coefficient of at most 12 digits, so
 * two 39-digit multiplications verify it exactly.
 *
 * \param[in] operand cube-root operand
 * \param[in,out] root approximate cube root, replaced when exact
 * \return void
 ***********************************************/
static void correctExactReal34Cube(const real_t *operand, real_t *root) {
  realContext_t nearestContext = ctxtReal34;
  real_t nearest, cube;

  if(realIsSpecial(operand) || realIsZero(operand)) {
    return;
  }
  nearestContext.round = DEC_ROUND_HALF_EVEN;
  realPlus(root, &nearest, &nearestContext);
  realReduce(&nearest, &nearest, &ctxtReal39);
  if(nearest.digits > 12) {
    return;
  }
  realMultiply(&nearest, &nearest, &cube, &ctxtReal39);
  realMultiply(&cube, &nearest, &cube, &ctxtReal39);
  if(realCompareEqual(&cube, operand)) {
    realCopy(&nearest, root);
  }
}


static void curtShoI(void) {
  real_t x, operand, nearest, cube;
  int32_t cubeRoot;

  convertShortIntegerRegisterToReal(REGISTER_X, &x, &ctxtReal39);
  realCopy(&x, &operand);

  if(realIsPositive(&x)) {
    PowerReal(&x, const39_1on3, &x, &ctxtReal39);
  }
  else {
    realSetPositiveSign(&x);
    PowerReal(&x, const39_1on3, &x, &ctxtReal39);
    realSetNegativeSign(&x);
  }

  realToIntegralValue(&x, &nearest, DEC_ROUND_HALF_UP, &ctxtReal39);   // PowerReal() lands a hair under an exact cube
  realMultiply(&nearest, &nearest, &cube, &ctxtReal39);
  realMultiply(&cube, &nearest, &cube, &ctxtReal39);
  if(realCompareEqual(&cube, &operand)) {                             // the operand is a cube, so its root is exact
    realCopy(&nearest, &x);
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
  real_t x, operand;
  real34_t root;

  if(!getRegisterAsReal(REGISTER_X, &x)) {
    return;
  }
  realCopy(&x, &operand);

  if(realIsInfinite(&x) && !getSystemFlag(FLAG_SPCRES)) {
    displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      moreInfoOnError("In function curtReal:", "cannot use " STD_PLUS_MINUS STD_INFINITY " as X input of curt when flag SPCRES is not set", NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    return;
  }

  if(realIsZero(&x)) {
    convertRealToResultRegister(&x, REGISTER_X, amNone);
    return;
  }
  if(rootRegisterToReal34(REGISTER_X, 3u, true, &root)) {   // the cube root of the integer coefficient, rounded once by RM
    real34ToReal(&root, &x);
  }
  else if(realIsPositive(&x)) {
    PowerReal(&x, const39_1on3, &x, &ctxtReal39);
  }
  else {
    realSetPositiveSign(&x);
    PowerReal(&x, const39_1on3, &x, &ctxtReal39);
    realSetNegativeSign(&x);
  }
  correctExactReal34Cube(&operand, &x);
  convertRealToResultRegister(&x, REGISTER_X, amNone);
}



static  void curtLonI(void) {
  rootLonI(3);
}



void curtCplx(void) {
  real_t operandReal;
  cplx_t a;
  bool_t operandImaginaryZero;

  if(getRegisterAsComplex(REGISTER_X, &a)) {
    realCopy(&a.Real, &operandReal);
    operandImaginaryZero = realIsZero(&a.Imag);
    curtComplex(&a, &a, &ctxtReal39);
    if(operandImaginaryZero) {
      correctExactReal34Cube(&operandReal, &a.Real);
    }
    convertComplexToResultRegister(&a, REGISTER_X);
  }
}



void curtComplex(const cplx_t *z, cplx_t *res, realContext_t *realContext) {
  cplx_t w;

  realCopy(&z->Real, &w.Real);
  realCopy(&z->Imag, &w.Imag);

  if(realIsZero(&w.Imag)) {
    if(realIsPositive(&w.Real)) {
      PowerReal(&w.Real, const39_1on3, &res->Real, realContext);
    }
    else {
      realSetPositiveSign(&w.Real);
      PowerReal(&w.Real, const39_1on3, &res->Real, realContext);
      realSetNegativeSign(&res->Real);
    }
    realCopy(&w.Imag, &res->Imag);
  }
  else if(realIsInfinite(&w.Real) && !realIsSpecial(&w.Imag)) {
    realSetPlusInfinity(&res->Real);
    if(realIsPositive(&w.Real)) {
      realSetZero(&res->Imag);
    }
    else {
      realSetPlusInfinity(&res->Imag);
    }
    if(realIsNegative(&w.Imag)) {
      realSetNegativeSign(&res->Imag);
    }
  }
  else {
    realRectangularToPolar(&w, &w.Mag, &w.Arg, realContext);
    PowerReal(&w.Mag, const39_1on3, &w.Mag, realContext);
    realMultiply(&w.Arg, const39_1on3, &w.Arg, realContext);
    realPolarToRectangular(&w.Mag, &w.Arg, res, realContext);
  }
}


#if defined(OPTION_CUBIC_159) || defined(OPTION_EIGEN_159)
// Complex cube root using Halley's method: x_{n+1} = x_n * (x_n³ + 2z) / (2x_n³ + z)
//   The point of using this is we cannot use complex trig without a 159 digit precision Taylor et al.
void curtComplex159(const cplx159_t *z, cplx159_t *res, realContext_t *realContext) {
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

  realCopy(&z->Real, zr);
  realCopy(&z->Imag, zi);

  if(realIsZero(zr) && realIsZero(zi)) {
    realSetZero(&res->Real);
    realSetZero(&res->Imag);
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
    if(realIsNegative(&z->Imag)) {
      realChangeSign(xi);
    }

    realCopy(xr, &res->Real);
    realCopy(xi, &res->Imag);
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
    realCopy(xr, &res->Real);
    realCopy(xi, &res->Imag);
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
  cplx159_t w1, w2, w3;
  realSetZero(&w1.Real);
  realSetZero(&w1.Imag);
  realSetZero(&w2.Real);
  realSetZero(&w2.Imag);
  realSetZero(&w3.Real);
  realSetZero(&w3.Imag);

  // w1 = current root
  realCopy(xr, &w1.Real);
  realCopy(xi, &w1.Imag);

  // omega = -1/2 + i*sqrt(3)/2
  cplx159_t omega;
  realSetZero(&omega.Real);
  realSetZero(&omega.Imag);
  realCopy(const_1on2, &omega.Real);
  realChangeSign(&omega.Real); // -1/2
  realSquareRoot(const_3, &omega.Imag, realContext);
  realMultiply(&omega.Imag, const_1on2, &omega.Imag, realContext); // sqrt(3)/2
  //realToString(omega_r, dbg); printf("DEBUG curtComplex159: omega_r = %s\n", dbg);
  //realToString(omega_i, dbg); printf("DEBUG curtComplex159: omega_i = %s\n", dbg);

  // w2 = w1 * omega
  mulComplexComplex159(&w1, &omega, &w2, realContext);
  //realToString(w2r, dbg); printf("DEBUG curtComplex159: w2r = %s\n", dbg);
  //realToString(w2i, dbg); printf("DEBUG curtComplex159: w2i = %s\n", dbg);

  // w3 = w1 * omega^2 = w1 * conj(omega)
  realChangeSign(&omega.Imag);
  mulComplexComplex159(&w1, &omega, &w3, realContext);
  //realToString(w3r, dbg); printf("DEBUG curtComplex159: w3r = %s\n", dbg);
  //realToString(w3i, dbg); printf("DEBUG curtComplex159: w3i = %s\n", dbg);

  // Select root with largest real part
  realCopy(&w1.Real, xr);
  realCopy(&w1.Imag, xi);
  //printf("DEBUG curtComplex159: Starting with w1 as candidate\n");

  if(realCompareLessThan(xr, &w2.Real)) {
    //printf("DEBUG curtComplex159: w2 has larger real part than current, selecting w2\n");
    realCopy(&w2.Real, xr);
    realCopy(&w2.Imag, xi);
  }
  if(realCompareLessThan(xr, &w3.Real)) {
    //printf("DEBUG curtComplex159: w3 has larger real part than current, selecting w3\n");
    realCopy(&w3.Real, xr);
    realCopy(&w3.Imag, xi);
  }
  //realToString(xr, dbg); printf("DEBUG curtComplex159: Final selected root xr = %s\n", dbg);
  //realToString(xi, dbg); printf("DEBUG curtComplex159: Final selected root xi = %s\n", dbg);
  realCopy(xr, &res->Real);
  realCopy(xi, &res->Imag);
}
#endif //OPTION_CUBIC_159 || OPTION_EIGEN_159



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
