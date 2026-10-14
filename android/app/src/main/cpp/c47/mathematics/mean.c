// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file mean.c
 ***********************************************/

#include "c47.h"

/********************************************//**
 * \brief x bar ==> regX, regY
 * enables stack lift and refreshes the stack.
 * regX = MEAN x, regY = MEAN y
 *
 * \param[in] displayInfo int
 * \param[in] sumX real_t
 * \param[in] numberX real_t
 * \param[in] sumY real_t
 * \param[in] numberY real_t
 * \param[in] transform void (*)(const real_t *operand, real_t *result)
 * \return void
 ***********************************************/
static void calculateMean(int displayInfo, real_t *sumX, real_t *numberX, real_t *sumY, real_t *numberY, void (*transform)(const real_t *operand, real_t *result)) {
  if(checkMinimumDataPoints(const_1)) {
    real_t tempReal1, tempReal2;

    liftStack();

    if(sumY != NULL) {
      setSystemFlag(FLAG_ASLIFT);
      liftStack();
    }

    realDivide(sumX, numberX, &tempReal1, &ctxtReal39);
    if(transform != NULL) {
      (*transform)(&tempReal1, &tempReal2);
    }
    real_t *mean = transform == NULL ? &tempReal1 : &tempReal2;
    convertRealToReal34ResultRegister(mean, REGISTER_X);

    if(sumY != NULL) {
      realDivide(sumY, numberY, &tempReal1, &ctxtReal39);
      if(transform != NULL) {
        (*transform)(&tempReal1, &tempReal2);
      }
      convertRealToReal34ResultRegister(mean, REGISTER_Y);
    }

    temporaryInformation = displayInfo;
  }
}

/********************************************//**
 * \brief x bar ==> regX, regY
 * enables stack lift and refreshes the stack.
 * regX = MEAN x, regY = MEAN y
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnMeanXY(uint16_t unusedButMandatoryParameter) {
  calculateMean(TI_MEANX_MEANY, SIGMA_X, SIGMA_N, SIGMA_Y, SIGMA_N, NULL);
}

void fnMeanX(uint16_t unusedButMandatoryParameter) {
  calculateMean(TI_MEANX, SIGMA_X, SIGMA_N, NULL, NULL, NULL);
}


static void geometricMeanTransform(const real_t *operand, real_t *result) {
  realExp(operand, result, &ctxtReal39);
}

/********************************************//**
 * \brief x bar sub G ==> regX, regY
 * enables stack lift and refreshes the stack.
 * regX = GEOMTERIC MEAN x, regY = GEOMTERIC MEAN y
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnGeometricMeanXY(uint16_t unusedButMandatoryParameter) {
  calculateMean(TI_GEOMMEANX_GEOMMEANY, SIGMA_lnX, SIGMA_N, SIGMA_lnY, SIGMA_N, &geometricMeanTransform);
}

/********************************************//**
 * \brief x bar sub H ==> regX, regY
 * enables stack lift and refreshes the stack.
 * regX = HARMONIC MEAN x, regY = HARMONIC MEAN y
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnHarmonicMeanXY(uint16_t unusedButMandatoryParameter) {
  calculateMean(TI_HARMMEANX_HARMMEANY, SIGMA_N, SIGMA_1onX, SIGMA_N, SIGMA_1onY, NULL);
}

static void RMSMeanTransform(const real_t *operand, real_t *result) {
  realSquareRoot(operand, result, &ctxtReal39);
}

/********************************************//**
 * \brief x bar sub R sub M sub S ==> regX, regY
 * enables stack lift and refreshes the stack.
 * regX = RMS MEAN x, regY = RMS MEAN y
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnRMSMeanXY(uint16_t unusedButMandatoryParameter) {
  calculateMean(TI_RMSMEANX_RMSMEANY, SIGMA_X2, SIGMA_N, SIGMA_Y2, SIGMA_N, &RMSMeanTransform);
}

/********************************************//**
 * \brief x bar sub W ==> regX
 * enables stack lift and refreshes the stack.
 * regX = Weighted MEAN
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnWeightedMeanX(uint16_t unusedButMandatoryParameter) {
  real_t mean;

  if(statisticalSumsPointer == NULL) {
    displayCalcErrorMessage(ERROR_NO_SUMMATION_DATA, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "There is no statistical data available!");
      moreInfoOnError("In function fnWeightedMeanX:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else if(realCompareLessThan(SIGMA_Y, const_1)) {
    displayCalcErrorMessage(ERROR_TOO_FEW_DATA, ERR_REGISTER_LINE, REGISTER_X);
    #if (EXTRA_INFO_ON_CALC_ERROR == 1)
      sprintf(errorMessage, "There is insufficient statistical data available!");
      moreInfoOnError("In function fnWeightedMeanX:", errorMessage, NULL, NULL);
    #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
  }
  else {
    liftStack();
    setSystemFlag(FLAG_ASLIFT);

    realDivide(SIGMA_XY, SIGMA_Y, &mean, &ctxtReal39);
    convertRealToReal34ResultRegister(&mean, REGISTER_X);

    temporaryInformation = TI_WEIGHTEDMEANX;
  }
}
