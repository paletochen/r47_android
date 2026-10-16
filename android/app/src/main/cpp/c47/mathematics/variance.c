// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file variance.c
 ***********************************************/

#include "c47.h"

typedef enum {
  DEVIATION_X,
  DEVIATION_Y,
  DEVIATION_LNX,
  DEVIATION_LNY,
  DEVIATION_WEIGHTED_X
} deviationSource_t;

/**
 * \brief Obtains one stored observation component with its requested transformation.
 *
 * \param stats Stored observations
 * \param row Matrix row
 * \param cols Matrix column count
 * \param source Observation component and optional transformation
 * \param value Result
 */
static void statisticalValue(const real34Matrix_t *stats, uint16_t row, uint16_t cols, deviationSource_t source, real_t *value) {
  const bool_t yColumn = source == DEVIATION_Y || source == DEVIATION_LNY;
  real34ToReal(stats->matrixElements + row * cols + yColumn, value);
  if(source == DEVIATION_LNX || source == DEVIATION_LNY) {
    WP34S_Ln(value, value, &ctxtReal75);
  }
}

/**
 * \brief Returns the stored second moment of an observation component.
 *
 * \param source Observation component and optional transformation
 * \return Sum of the squared component, weighted when source is DEVIATION_WEIGHTED_X
 */
static const real_t *secondMoment(deviationSource_t source) {
  switch(source) {
    case DEVIATION_Y:          return SIGMA_Y2;
    case DEVIATION_LNX:        return SIGMA_ln2X;
    case DEVIATION_LNY:        return SIGMA_ln2Y;
    case DEVIATION_WEIGHTED_X: return SIGMA_X2Y;
    default:                   return SIGMA_X2;
  }
}

/**
 * \brief Returns the stored product moment of two observation components.
 *
 * \param sourceX First observation component and optional transformation
 * \param sourceY Second observation component and optional transformation
 * \return Sum of the products of the two components
 */
static const real_t *productMoment(deviationSource_t sourceX, deviationSource_t sourceY) {
  if(sourceX == DEVIATION_LNX) {
    return sourceY == DEVIATION_LNY ? SIGMA_lnXlnY : SIGMA_YlnX;
  }
  return sourceY == DEVIATION_LNY ? SIGMA_XlnY : SIGMA_XY;
}

/**
 * \brief Returns true when the observations in STATS give the stored sums of x^2 and y^2.
 *
 * A long integer wider than 34 digits enters the sums with all its digits and STATS rounded to 34, which changes its square. Every value below 1E32 is stored as
 * it was summed, so the squares in STATS are summed again, as addSigma sums them, only when an extreme value reaches 1E32.
 *
 * \return true when the centred sums may take their observations from STATS
 */
static bool_t statsMatchSums(void) {
  real34Matrix_t stats;
  REAL_T_PTR(value, 75);
  REAL_T_PTR(sumX2, 75);
  REAL_T_PTR(sumY2, 75);
  uint16_t rows, cols;

  if(realCompareAbsLessThan(SIGMA_XMIN, const_1e32) && realCompareAbsLessThan(SIGMA_XMAX, const_1e32)
     && realCompareAbsLessThan(SIGMA_YMIN, const_1e32) && realCompareAbsLessThan(SIGMA_YMAX, const_1e32)) {
    return true;
  }
  linkToRealMatrixRegister(findNamedVariable(statMx), &stats);
  rows = stats.header.matrixRows;
  cols = stats.header.matrixColumns;
  realSetZero(sumX2);
  realSetZero(sumY2);
  for(uint16_t i = 0; i < rows; ++i) {
    real34ToReal(stats.matrixElements + i * cols, value);
    realMultiply(value, value, value, &ctxtReal75);
    realAdd(sumX2, value, sumX2, &ctxtReal75);
    real34ToReal(stats.matrixElements + i * cols + 1, value);
    realMultiply(value, value, value, &ctxtReal75);
    realAdd(sumY2, value, sumY2, &ctxtReal75);
  }
  return realCompareEqual(sumX2, SIGMA_X2) && realCompareEqual(sumY2, SIGMA_Y2);
}

/**
 * \brief Forms a centred sum from the stored sums: moment - sumX * sumY / number.
 *
 * The FMA forms the product exactly. The quotient is formed to 150 digits, so its rounding moves the result by at most |sumX * sumY / number| * 5E-150: at 75 digits
 * that bound is 1.5 for three long integers next to 1E37, whose centred square sum is 18.7.
 *
 * \param sumY Stored first moment the mean is formed from
 * \param number Observation count or total weight
 * \param sumX Stored first moment it multiplies
 * \param moment Stored second or product moment
 * \param res Centred sum
 */
static void centredFromSums(const real_t *sumY, const real_t *number, const real_t *sumX, const real_t *moment, real_t *res) {
  realContext_t context = ctxtReal75;
  REAL_T_PTR(mean, 150);

  context.digits = 150;
  realDivide(sumY, number, mean, &context);
  realChangeSign(mean);
  realFMA(sumX, mean, moment, res, &ctxtReal75);
}

/**
 * \brief Forms the centred square sum from the stored Real34 observations.
 *
 * \param sumX Stored first moment, weighted when source is DEVIATION_WEIGHTED_X
 * \param numberX Observation count or total weight
 * \param source Observation component and optional transformation
 * \param res Centred square sum
 * \param realContext Context used for centred arithmetic
 */
static void centredSquareSum(const real_t *sumX, const real_t *numberX, deviationSource_t source, real_t *res, realContext_t *realContext) {
  real34Matrix_t stats;
  REAL_T_PTR(mean, 75);
  REAL_T_PTR(value, 75);
  REAL_T_PTR(weight, 51);
  REAL_T_PTR(square, 51);
  calcRegister_t regStats;
  uint16_t rows, cols;

  realDivide(sumX, numberX, mean, &ctxtReal75);
  realSetZero(res);

  if(!isStatsMatrix(&rows, statMx) || !statsMatchSums()) {  // after DELVall, and for long integers wider than 34 digits, the stored sums replace the observations
    centredFromSums(sumX, numberX, sumX, secondMoment(source), res);
    return;
  }
  regStats = findNamedVariable(statMx);
  linkToRealMatrixRegister(regStats, &stats);
  rows = stats.header.matrixRows;
  cols = stats.header.matrixColumns;

  for(uint16_t i = 0; i < rows; ++i) {
    statisticalValue(&stats, i, cols, source, value);
    realSubtract(value, mean, value, realContext);

    if(source == DEVIATION_WEIGHTED_X) {
      realMultiply(value, value, square, realContext);
      real34ToReal(stats.matrixElements + i * cols + 1, weight);
      realFMA(weight, square, res, res, realContext);
    }
    else {
      realFMA(value, value, res, res, realContext);
    }
  }
}

/**
 * \brief Forms the centred product sum from the stored Real34 observations.
 *
 * \param sumX Stored first moment for the first component
 * \param sumY Stored first moment for the second component
 * \param sourceX First observation component and optional transformation
 * \param sourceY Second observation component and optional transformation
 * \param res Centred product sum
 * \param realContext Context used for centred arithmetic
 */
static void centredProductSum(const real_t *sumX, const real_t *sumY, deviationSource_t sourceX, deviationSource_t sourceY, real_t *res, realContext_t *realContext) {
  real34Matrix_t stats;
  REAL_T_PTR(meanX, 75);
  REAL_T_PTR(meanY, 75);
  REAL_T_PTR(x, 75);
  REAL_T_PTR(y, 75);
  calcRegister_t regStats;
  uint16_t rows, cols;

  realDivide(sumX, SIGMA_N, meanX, &ctxtReal75);
  realDivide(sumY, SIGMA_N, meanY, &ctxtReal75);
  realSetZero(res);

  if(!isStatsMatrix(&rows, statMx) || !statsMatchSums()) {
    centredFromSums(sumY, SIGMA_N, sumX, productMoment(sourceX, sourceY), res);
    return;
  }
  regStats = findNamedVariable(statMx);
  linkToRealMatrixRegister(regStats, &stats);
  rows = stats.header.matrixRows;
  cols = stats.header.matrixColumns;

  for(uint16_t i = 0; i < rows; ++i) {
    statisticalValue(&stats, i, cols, sourceX, x);
    statisticalValue(&stats, i, cols, sourceY, y);
    realSubtract(x, meanX, x, realContext);
    realSubtract(y, meanY, y, realContext);
    realFMA(x, y, res, res, realContext);
  }
}

/**
 * \brief Forms the squared residual sum for a regression line.
 *
 * \param sumX Stored first moment for the first component
 * \param sumY Stored first moment for the second component
 * \param sourceX First observation component and optional transformation
 * \param sourceY Second observation component and optional transformation
 * \param ratio Regression slope, or its reciprocal for a steep line
 * \param steep Selects the reciprocal-slope residual
 * \param res Squared residual sum
 */
static void centredResidualSquareSum(const real_t *sumX, const real_t *sumY, deviationSource_t sourceX, deviationSource_t sourceY,
                                     const real_t *ratio, bool_t steep, real_t *res) {
  real34Matrix_t stats;
  REAL_T_PTR(meanX, 75);
  REAL_T_PTR(meanY, 75);
  REAL_T_PTR(x, 75);
  REAL_T_PTR(y, 75);
  REAL_T_PTR(negativeRatio, 75);
  REAL_T_PTR(residual, 75);
  calcRegister_t regStats;
  uint16_t rows, cols;

  realDivide(sumX, SIGMA_N, meanX, &ctxtReal75);
  realDivide(sumY, SIGMA_N, meanY, &ctxtReal75);
  realMinus(ratio, negativeRatio, &ctxtReal75);
  realSetZero(res);

  if(!isStatsMatrix(&rows, statMx) || !statsMatchSums()) {  // from the centred sums: Syy - 2 b Sxy + b^2 Sxx, or b^2 Syy - 2 b Sxy + Sxx for the reciprocal slope
    centredSquareSum(sumX, SIGMA_N, sourceX, steep ? y : x, &ctxtReal75);
    centredSquareSum(sumY, SIGMA_N, sourceY, steep ? x : y, &ctxtReal75);
    centredProductSum(sumX, sumY, sourceX, sourceY, residual, &ctxtReal75);
    realAdd(residual, residual, residual, &ctxtReal75);
    realChangeSign(residual);
    realFMA(ratio, x, residual, x, &ctxtReal75);
    realFMA(ratio, x, y, res, &ctxtReal75);
    return;
  }
  regStats = findNamedVariable(statMx);
  linkToRealMatrixRegister(regStats, &stats);
  rows = stats.header.matrixRows;
  cols = stats.header.matrixColumns;

  for(uint16_t i = 0; i < rows; ++i) {
    statisticalValue(&stats, i, cols, sourceX, x);
    statisticalValue(&stats, i, cols, sourceY, y);
    realSubtract(x, meanX, x, &ctxtReal75);
    realSubtract(y, meanY, y, &ctxtReal75);
    if(steep) {
      realChangeSign(x);
      realFMA(ratio, y, x, residual, &ctxtReal75);
    }
    else {
      realFMA(negativeRatio, x, y, residual, &ctxtReal75);
    }
    realFMA(residual, residual, res, res, &ctxtReal39);
  }
}

/* Standard deviations and standard errors. */
static void do_stddev(const real_t *sumX, const real_t *numberX, deviationSource_t source, int sample, int rootn, int exp, int regIndex) {
  real_t tempReal1, tempReal2, tempReal3;
  const real_t *p = numberX;
  realContext_t *coreContext = exp ? &ctxtReal51 : &ctxtReal39;
  realContext_t *resultContext = &ctxtReal39;

  centredSquareSum(sumX, numberX, source, &tempReal1, coreContext);
  if(sample) {
    realSubtract(numberX, const_1, &tempReal3, coreContext);
    p = &tempReal3;
  }
  realDivide(&tempReal1, p, &tempReal2, coreContext);
  if(rootn) {
    // Divide before the root to remove a second root and division.
    realDivide(&tempReal2, numberX, &tempReal2, coreContext);
  }
  if(realIsNegative(&tempReal2) || realIsZero(&tempReal2)) {
    realSetZero(&tempReal1);
  }
  else {
    realSquareRoot(&tempReal2, &tempReal1, resultContext);
  }

  p = &tempReal1;
  if(exp) {
    realExp(p, &tempReal2, resultContext);
    p = &tempReal2;
  }
  realToReal34(p, REGISTER_REAL34_DATA(regIndex));
}

static void calculateStandardDeviation(const real_t *sumX, deviationSource_t sourceX,
                                       const real_t *number,
                                       const real_t *sumY, deviationSource_t sourceY,
                                       int sample, int rootn, int exp,
                                       int displayInfo) {
  if(checkMinimumDataPoints(const_2)) {
    liftStack();
    setSystemFlag(FLAG_ASLIFT);
    liftStack();

    do_stddev(sumX, number, sourceX, sample, rootn, exp, REGISTER_X);
    do_stddev(sumY, number, sourceY, sample, rootn, exp, REGISTER_Y);

    temporaryInformation = displayInfo;
  }
}

/* Standard deviation (sample and population) and standard error for arithmetic mean */
void fnSampleStdDev(uint16_t unusedButMandatoryParameter) {
  calculateStandardDeviation(SIGMA_X, DEVIATION_X, SIGMA_N, SIGMA_Y, DEVIATION_Y, 1, 0, 0, TI_SAMPLSTDDEV);
}

void fnPopulationStdDev(uint16_t unusedButMandatoryParameter) {
  calculateStandardDeviation(SIGMA_X, DEVIATION_X, SIGMA_N, SIGMA_Y, DEVIATION_Y, 0, 0, 0, TI_POPLSTDDEV);
}

void fnStandardError(uint16_t unusedButMandatoryParameter) {
  calculateStandardDeviation(SIGMA_X, DEVIATION_X, SIGMA_N, SIGMA_Y, DEVIATION_Y, 1, 1, 0, TI_STDERR);
}

/* Standard deviation (sample and population) and standard error for geometric mean */
void fnGeometricSampleStdDev(uint16_t unusedButMandatoryParameter) {
  calculateStandardDeviation(SIGMA_lnX, DEVIATION_LNX, SIGMA_N, SIGMA_lnY, DEVIATION_LNY, 1, 0, 1, TI_GEOMSAMPLSTDDEV);
}

void fnGeometricPopulationStdDev(uint16_t unusedButMandatoryParameter) {
  calculateStandardDeviation(SIGMA_lnX, DEVIATION_LNX, SIGMA_N, SIGMA_lnY, DEVIATION_LNY, 0, 0, 1, TI_GEOMPOPLSTDDEV);
}

void fnGeometricStandardError(uint16_t unusedButMandatoryParameter) {
  calculateStandardDeviation(SIGMA_lnX, DEVIATION_LNX, SIGMA_N, SIGMA_lnY, DEVIATION_LNY, 1, 1, 1, TI_GEOMSTDERR);
}

// Weighted standard deviation, standard error
static void calculateWeightedStandardDeviation(int sample, int rootn, int exp, int display) {
  if(checkMinimumDataPoints(const_2)) {
    const bool_t invalidWeight = !realIsNaN(SIGMA_Y)
                              && (sample ? realCompareLessEqual(SIGMA_Y, const_1)
                                         : realCompareLessEqual(SIGMA_Y, const_0));
    if(invalidWeight) {
      displayCalcErrorMessage(ERROR_TOO_FEW_DATA, ERR_REGISTER_LINE);
      return;
    }
    liftStack();
    setSystemFlag(FLAG_ASLIFT);
    do_stddev(SIGMA_XY, SIGMA_Y, DEVIATION_WEIGHTED_X, sample, rootn, exp, REGISTER_X);

    temporaryInformation = display;
  }
}

void fnWeightedSampleStdDev(uint16_t unusedButMandatoryParameter) {
  calculateWeightedStandardDeviation(1, 0, 0, TI_WEIGHTEDSAMPLSTDDEV);
}

void fnWeightedPopulationStdDev(uint16_t unusedButMandatoryParameter) {
  calculateWeightedStandardDeviation(0, 0, 0, TI_WEIGHTEDPOPLSTDDEV);
}

void fnWeightedStandardError(uint16_t unusedButMandatoryParameter) {
  calculateWeightedStandardDeviation(1, 1, 0, TI_WEIGHTEDSTDERR);
}

void fnStatSX_SY(real_t *SX, real_t *SY){
  realContext_t *realContext = &ctxtReal39;
  real_t TT;
  if(checkMinimumDataPoints(const_2)) {
    realSubtract(SIGMA_N, const_1, &TT, realContext);
    centredSquareSum(SIGMA_X, SIGMA_N, DEVIATION_X, SX, realContext);
    realDivide(SX, &TT, SX, realContext);
    realSquareRoot(SX, SX, realContext);

    centredSquareSum(SIGMA_Y, SIGMA_N, DEVIATION_Y, SY, realContext);
    realDivide(SY, &TT, SY, realContext);
    realSquareRoot(SY, SY, realContext);
  }
}

void fnStatSXY(real_t *SXY){
  realContext_t *realContext = &ctxtReal75;
  real_t TT;
  if(checkMinimumDataPoints(const_2)) {
    centredProductSum(SIGMA_X, SIGMA_Y, DEVIATION_X, DEVIATION_Y, SXY, realContext);
    realSubtract(SIGMA_N, const_1, &TT, realContext);
    realDivide(SXY, &TT, SXY, realContext);
  }
}

void fnPopulationCovariance(uint16_t unusedButMandatoryParameter){    //COVxy
  real_t SXY;
  if(checkMinimumDataPoints(const_2)) {
    centredProductSum(SIGMA_X, SIGMA_Y, DEVIATION_X, DEVIATION_Y, &SXY, &ctxtReal75);
    realDivide(&SXY, SIGMA_N, &SXY, &ctxtReal34);

    liftStack();
    setSystemFlag(FLAG_ASLIFT);
    convertRealToResultRegister(&SXY, REGISTER_X, amNone);
    temporaryInformation = TI_COV;
  }
}

void fnSampleCovariance(uint16_t unusedButMandatoryParameter){    //sxy
  real_t SXY, TT;
  if(checkMinimumDataPoints(const_2)) {
    centredProductSum(SIGMA_X, SIGMA_Y, DEVIATION_X, DEVIATION_Y, &SXY, &ctxtReal75);
    realSubtract(SIGMA_N, const_1, &TT, &ctxtReal34);
    realDivide(&SXY, &TT, &SXY, &ctxtReal34);
    liftStack();
    setSystemFlag(FLAG_ASLIFT);
    convertRealToResultRegister(&SXY, REGISTER_X, amNone);
    temporaryInformation = TI_SXY;
  }
}

void fnStatR(real_t *RR, real_t *SXY, real_t *SX, real_t *SY){
  realContext_t *realContext = &ctxtReal75;
  real_t rootX, rootY, TT;
  if(checkMinimumDataPoints(const_2)) {
    centredSquareSum(SIGMA_X, SIGMA_N, DEVIATION_X, SX, realContext);
    centredSquareSum(SIGMA_Y, SIGMA_N, DEVIATION_Y, SY, realContext);
    centredProductSum(SIGMA_X, SIGMA_Y, DEVIATION_X, DEVIATION_Y, SXY, realContext);

    realSquareRoot(SX, &rootX, realContext);
    realSquareRoot(SY, &rootY, realContext);
    realDivide(SXY, &rootX, RR, realContext);
    realDivide(RR, &rootY, RR, realContext);
    if(!realIsNaN(RR) && realCompareGreaterThan(RR, const_1)) {
      realCopy(const_1, RR);
    }
    else if(!realIsNaN(RR) && realCompareLessThan(RR, const__1)) {
      realCopy(const__1, RR);
    }

    realSubtract(SIGMA_N, const_1, &TT, realContext);
    realDivide(SX, &TT, SX, realContext);
    realSquareRoot(SX, SX, realContext);
    realDivide(SY, &TT, SY, realContext);
    realSquareRoot(SY, SY, realContext);
    realDivide(SXY, &TT, SXY, realContext);
  }
}

void fnCoefficientDetermination(uint16_t unusedButMandatoryParameter){  //r
  real_t RR, SMI, aa0, aa1, aa2;
  if(checkMinimumDataPoints(const_2)) {
    if(lrChosen == 0) {                    //if lrChosen contains something, the stat data exists, otherwise set it to linear. lrSelection still has 1 at this point, i.e. the * will not appear.
      lrChosen = CF_LINEAR_FITTING;
    }
    processCurvefitSelection(lrChosen, &RR, &SMI, &aa0, &aa1, &aa2);
    liftStack();
    setSystemFlag(FLAG_ASLIFT);
    convertRealToResultRegister(&RR, REGISTER_X, amNone);
    temporaryInformation = TI_CORR;
  }
}

void fnStatSMI(real_t *SMI){
  real_t absCXY, CXX, CXY, ratio, residualSum, scale2, denominator;
  if(checkMinimumDataPoints(const_2)) {
    centredSquareSum(SIGMA_X, SIGMA_N, DEVIATION_X, &CXX, &ctxtReal75);
    centredProductSum(SIGMA_X, SIGMA_Y, DEVIATION_X, DEVIATION_Y, &CXY, &ctxtReal75);
    realCopyAbs(&CXY, &absCXY);
    if(realCompareLessEqual(&absCXY, &CXX)) {
      realDivide(&CXY, &CXX, &ratio, &ctxtReal75);
      centredResidualSquareSum(SIGMA_X, SIGMA_Y, DEVIATION_X, DEVIATION_Y, &ratio, false, &residualSum);
    }
    else {
      realDivide(&CXX, &CXY, &ratio, &ctxtReal75);
      centredResidualSquareSum(SIGMA_X, SIGMA_Y, DEVIATION_X, DEVIATION_Y, &ratio, true, &residualSum);
    }
    realFMA(&ratio, &ratio, const_1, &scale2, &ctxtReal39);
    realSubtract(SIGMA_N, const_1, &denominator, &ctxtReal39);
    realMultiply(&denominator, &scale2, &denominator, &ctxtReal39);
    realDivide(&residualSum, &denominator, SMI, &ctxtReal39);
    realSquareRoot(SMI, SMI, &ctxtReal39);
  }
}

void fnMinExpStdDev(uint16_t unusedButMandatoryParameter){ //smi
  real_t SMI, RR, aa0, aa1, aa2, n30;
  uInt32ToReal(30, &n30);
  if(checkMinimumDataPoints(&n30)) {
    lrChosen = CF_ORTHOGONAL_FITTING;                      //force to ORTHOF only
    lrSelection = CF_ORTHOGONAL_FITTING;
    processCurvefitSelection(lrChosen, &RR, &SMI, &aa0, &aa1, &aa2);
    liftStack();
    setSystemFlag(FLAG_ASLIFT);
    convertRealToResultRegister(&SMI, REGISTER_X, amNone);
    temporaryInformation = TI_SMI;
  }
}


/********************************************//**
 * \brief s(a) ==> regX, regY
 * regX = s(a0), regY = s(a1)
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
bool_t processCurvefitSA(real_t *SA0, real_t *SA1) {
  realContext_t *realContext = &ctxtReal75; // Summation data with 75 digits
  real_t RR, MX, RR2, MX2, SX2, SY2, UU, SS, TT, aa0, aa1, aa2, SMI;
  real_t CXX, CXY, meanX, residualSum, slope;
  const real_t *sumX;
  const real_t *sumY;
  deviationSource_t sourceX;
  deviationSource_t sourceY;

  //LINF:   SA0 & SA1: RR2, n, Sy, Sx, Sx^2, Mx^2
  //EXPF:   new value for SY2, SY and use REX
  //POWERF: new values for SX2, SX,  SY2, SY, MX
  //LOGF:   new values for SX2, SX, MX
  //ORTOF:  new values same as LINF

  if(checkMinimumDataPoints(const_3)) {
    realSetZero(&aa0);
    realSetZero(&aa1);
    realSetZero(&aa2);

    if(lrChosen == 0) {                    //if lrChosen contains something, the stat data exists, otherwise set it to linear. lrSelection still has 1 at this point, i.e. the * will not appear.
      lrChosen = CF_LINEAR_FITTING;
    }
    //Replaced fnStatR       (&RR, &SXY, &SX, &SY);
    processCurvefitSelectionAll(lrChosen, &RR, &MX, &MX2, &SX2, &SY2, &SMI, &aa0, &aa1, &aa2);  //Can be optimised a lot, as fnStatR is also called here
    realMultiply  (&RR, &RR, &RR2, realContext);               //   --> r^2

    #if defined(STATDEBUG) && defined(PC_BUILD)
      printf("##### processCurvefitSA:\n");
      printRealToConsole(&RR2, "§§ RR^2: ", "\n");
      printRealToConsole(&MX2, "§§ MX^2: ", "\n");
      printRealToConsole(&SX2, "§§ SX^2: ", "\n");
      printRealToConsole(&SY2, "§§ SY^2: ", "\n");
    #endif // STATDEBUG && PC_BUILD


    sumX = SIGMA_X;
    sumY = SIGMA_Y;
    sourceX = DEVIATION_X;
    sourceY = DEVIATION_Y;
    switch(orOrtho(lrChosen)) {
      case CF_LINEAR_FITTING:
      case CF_ORTHOGONAL_FITTING: {
        break;
      }
      case CF_EXPONENTIAL_FITTING: {
        sumY = SIGMA_lnY;
        sourceY = DEVIATION_LNY;
        break;
      }
      case CF_POWER_FITTING: {
        sumX = SIGMA_lnX;
        sumY = SIGMA_lnY;
        sourceX = DEVIATION_LNX;
        sourceY = DEVIATION_LNY;
        break;
      }
      case CF_LOGARITHMIC_FITTING: {
        sumX = SIGMA_lnX;
        sourceX = DEVIATION_LNX;
        break;
      }
      default: {
        displayCalcErrorMessage(ERROR_NO_ERRORS_CALCULABLE, ERR_REGISTER_LINE);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function processCurvefitSA:", "No errors are calculable for the selected/chosen model!", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return false;
      }
    }

    centredSquareSum(sumX, SIGMA_N, sourceX, &CXX, &ctxtReal75);
    centredProductSum(sumX, sumY, sourceX, sourceY, &CXY, &ctxtReal75);
    realDivide(&CXY, &CXX, &slope, &ctxtReal75);
    centredResidualSquareSum(sumX, sumY, sourceX, sourceY, &slope, false, &residualSum);

    realSubtract  (SIGMA_N, const_2, &TT, &ctxtReal39);
    realMultiply  (&TT, &CXX, &UU, &ctxtReal39);
    realDivide    (&residualSum, &UU, SA1, &ctxtReal39);
    realSquareRoot(SA1, SA1, &ctxtReal39);

    realDivide    (sumX, SIGMA_N, &meanX, &ctxtReal75);
    realDivide    (&CXX, SIGMA_N, &SS, &ctxtReal39);
    realFMA       (&meanX, &meanX, &SS, &SS, &ctxtReal39);
    realSquareRoot(&SS, &SS, &ctxtReal39);
    realMultiply  (&SS, SA1, SA0, &ctxtReal39);
    return true;
  }
  else {
    return false;
  }
}


void fnStatSa(uint16_t unusedButMandatoryParameter) {
  real_t SA0, SA1;

  if(processCurvefitSA(&SA0, &SA1)) {

    liftStack();
    setSystemFlag(FLAG_ASLIFT);
    liftStack();

    convertRealToReal34ResultRegister(&SA0, REGISTER_X);
    convertRealToReal34ResultRegister(&SA1, REGISTER_Y);
    temporaryInformation = TI_SA;
  }
}
