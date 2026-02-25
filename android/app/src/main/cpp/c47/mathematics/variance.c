// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file variance.c
 ***********************************************/

#include "c47.h"

/* Standard deviations and standard errors.
 *
 * The computation involves subtraction of large numbers that quite possible
 * are close to each other.  Because of this, the start of the computation is
 * performed using a FMA for the difference.
 */
static void do_stddev(const real_t *sumXX, const real_t *sumX, const real_t *numberX, int sample, int rootn, int exp, int regIndex) {
  real_t tempReal1, tempReal2, tempReal3;
  const real_t *p = numberX;
  realContext_t *realContext = &ctxtReal75; // Summation data with 75 digits

  realDivide(sumX, numberX, &tempReal2, realContext);
  realChangeSign(&tempReal2);
  realFMA(sumX, &tempReal2, sumXX, &tempReal1, realContext);
  if(sample) {
    realSubtract(numberX, const_1, &tempReal3, realContext);
    p = &tempReal3;
  }
  realDivide(&tempReal1, p, &tempReal2, realContext);
  if(realIsNegative(&tempReal2) || realIsZero(&tempReal2)) {
    realZero(&tempReal1);
  }
  else {
    realSquareRoot(&tempReal2, &tempReal1, realContext);
  }

  p = &tempReal1;
  if(rootn) {
    realSquareRoot(numberX, &tempReal2, realContext);
    realDivide(&tempReal1, &tempReal2, &tempReal3, realContext);
    p = &tempReal3;
  }
  if(exp) {
    realExp(p, &tempReal2, realContext);
    p = &tempReal2;
  }
  realToReal34(p, REGISTER_REAL34_DATA(regIndex));
}

static void calculateStandardDeviation(const real_t *sumX2, const real_t *sumX,
                                       const real_t *number,
                                       const real_t *sumY2, const real_t *sumY,
                                       int sample, int rootn, int exp,
                                       int displayInfo) {
  if(checkMinimumDataPoints(const_2)) {
    liftStack();
    setSystemFlag(FLAG_ASLIFT);
    liftStack();

    do_stddev(sumX2, sumX, number, sample, rootn, exp, REGISTER_X);
    do_stddev(sumY2, sumY, number, sample, rootn, exp, REGISTER_Y);

    temporaryInformation = displayInfo;
  }
}

/* Standard deviation (sample and population) and standard error for arithmetic mean */
void fnSampleStdDev(uint16_t unusedButMandatoryParameter) {
  calculateStandardDeviation(SIGMA_X2, SIGMA_X, SIGMA_N, SIGMA_Y2, SIGMA_Y, 1, 0, 0, TI_SAMPLSTDDEV);
}

void fnPopulationStdDev(uint16_t unusedButMandatoryParameter) {
  calculateStandardDeviation(SIGMA_X2, SIGMA_X, SIGMA_N, SIGMA_Y2, SIGMA_Y, 0, 0, 0, TI_POPLSTDDEV);
}

void fnStandardError(uint16_t unusedButMandatoryParameter) {
  calculateStandardDeviation(SIGMA_X2, SIGMA_X, SIGMA_N, SIGMA_Y2, SIGMA_Y, 1, 1, 0, TI_STDERR);
}

/* Standard deviation (sample and population) and standard error for geometric mean */
void fnGeometricSampleStdDev(uint16_t unusedButMandatoryParameter) {
  calculateStandardDeviation(SIGMA_ln2X, SIGMA_lnX, SIGMA_N, SIGMA_ln2Y, SIGMA_lnY, 1, 0, 1, TI_GEOMSAMPLSTDDEV);
}

void fnGeometricPopulationStdDev(uint16_t unusedButMandatoryParameter) {
  calculateStandardDeviation(SIGMA_ln2X, SIGMA_lnX, SIGMA_N, SIGMA_ln2Y, SIGMA_lnY, 0, 0, 1, TI_GEOMPOPLSTDDEV);
}

void fnGeometricStandardError(uint16_t unusedButMandatoryParameter) {
  calculateStandardDeviation(SIGMA_ln2X, SIGMA_lnX, SIGMA_N, SIGMA_ln2Y, SIGMA_lnY, 1, 1, 1, TI_GEOMSTDERR);
}

// Weighted standard deviation, standard error
static void calculateWeightedStandardDeviation(int sample, int rootn, int exp, int display) {
  if(checkMinimumDataPoints(const_2)) {
    liftStack();
    setSystemFlag(FLAG_ASLIFT);
    do_stddev(SIGMA_X2Y, SIGMA_XY, SIGMA_Y, sample, rootn, exp, REGISTER_X);

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
  realContext_t *realContext = &ctxtReal75; // Summation data with 75 digits
  real_t TT,UU;
  if(checkMinimumDataPoints(const_2)) {
    realMultiply(SIGMA_N,SIGMA_X2, &TT, realContext);    //do sx
    realMultiply(SIGMA_X,SIGMA_X, &UU, realContext);
    realSubtract(&TT,&UU,&TT,realContext);
    realDivide(&TT,SIGMA_N,&TT,realContext);
    realSubtract(SIGMA_N,const_1,&UU,realContext);
    realDivide(&TT,&UU,&TT,realContext);
    realSquareRoot(&TT,SX,realContext);                 //this is sx

    realMultiply(SIGMA_N,SIGMA_Y2, &TT, realContext);    //do sy
    realMultiply(SIGMA_Y,SIGMA_Y, &UU, realContext);
    realSubtract(&TT,&UU,&TT,realContext);
    realDivide(&TT,SIGMA_N,&TT,realContext);
    realSubtract(SIGMA_N,const_1,&UU,realContext);
    realDivide(&TT,&UU,&TT,realContext);
    realSquareRoot(&TT,SY,realContext);                 //this is sy
  }
}

void fnStatSXY(real_t *SXY){
  realContext_t *realContext = &ctxtReal75; // Summation data with 75 digits
  real_t SS,TT;
  if(checkMinimumDataPoints(const_2)) {
    realMultiply(SIGMA_N,SIGMA_XY, &SS, realContext);    //do sxy
    realMultiply(SIGMA_X,SIGMA_Y, &TT, realContext);
    realSubtract(&SS,&TT,&SS,realContext);
    realDivide(&SS,SIGMA_N,&SS,realContext);
    realSubtract(SIGMA_N,const_1,&TT,realContext);
    realDivide(&SS,&TT,SXY,realContext);
  }
}

void fnPopulationCovariance(uint16_t unusedButMandatoryParameter){    //COVxy
  realContext_t *realContext = &ctxtReal75; // Summation data with 75 digits
  real_t TT,SXY;
  if(checkMinimumDataPoints(const_2)) {
    fnStatSXY(&SXY);
    realSubtract(SIGMA_N,const_1,&TT,realContext);
    realMultiply(&SXY,&TT,&SXY,realContext);
    realDivide(&SXY,SIGMA_N,&TT,realContext);

    liftStack();
    setSystemFlag(FLAG_ASLIFT);
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    convertRealToReal34ResultRegister(&TT, REGISTER_X);
    temporaryInformation = TI_COV;
  }
}

void fnSampleCovariance(uint16_t unusedButMandatoryParameter){    //sxy
  real_t SXY;
  if(checkMinimumDataPoints(const_2)) {
    fnStatSXY(&SXY);
    liftStack();
    setSystemFlag(FLAG_ASLIFT);
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    convertRealToReal34ResultRegister(&SXY, REGISTER_X);
    temporaryInformation = TI_SXY;
  }
}

void fnStatR(real_t *RR, real_t *SXY, real_t *SX, real_t *SY){
  realContext_t *realContext = &ctxtReal75; // Summation data with 75 digits
  if(checkMinimumDataPoints(const_2)) {
    fnStatSX_SY(SX,SY);
    fnStatSXY(SXY);
    realDivide(SXY,SX,RR,realContext);                 //this is sxy/sx
    realDivide(RR,SY,RR,realContext);                 //this is sxy/sx/sy
  }
}

void fnCoefficientDetermination(uint16_t unusedButMandatoryParameter){  //r
  real_t RR,SMI,aa0,aa1,aa2;
  if(checkMinimumDataPoints(const_2)) {
    if(lrChosen == 0) {                    //if lrChosen contains something, the stat data exists, otherwise set it to linear. lrSelection still has 1 at this point, i.e. the * will not appear.
      lrChosen = CF_LINEAR_FITTING;
    }
    processCurvefitSelection(lrChosen,&RR,&SMI,&aa0,&aa1,&aa2);
    liftStack();
    setSystemFlag(FLAG_ASLIFT);
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    convertRealToReal34ResultRegister(&RR, REGISTER_X);
    temporaryInformation = TI_CORR;
  }
}

void fnStatSMI(real_t *SMI){
  realContext_t *realContext = &ctxtReal75; // Summation data with 75 digits
  real_t RR,SXY,SX,SY,RR2,SX2,SY2,UU,SS,TT;
  if(checkMinimumDataPoints(const_2)) {
    fnStatR(&RR,&SXY,&SX,&SY);
    realMultiply(&SX,&SX,&SX2,realContext);               //   --> sx^2
    realMultiply(&SY,&SY,&SY2,realContext);               //   --> sy^2
    realMultiply(&RR,&RR,&RR2,realContext);               //   --> r^2
    realSubtract(const_1,&RR2,&SS,realContext);           //  sqrt[ (1-r^2) / (r^2.SY2 - SX^2) * SX2.SY2 ]

    realMultiply(&RR2,&SY2,&TT,realContext);
    realAdd     (&TT,&SX2,&TT,realContext);
    realDivide  (&SS,&TT,&UU,realContext);
    realMultiply(&UU,&SX2,&UU,realContext);
    realMultiply(&UU,&SY2,&UU,realContext);               //  --> smi2
    realSquareRoot(&UU, SMI, &ctxtReal39);
  }
}

void fnMinExpStdDev(uint16_t unusedButMandatoryParameter){ //smi
  real_t SMI,RR,aa0,aa1,aa2,n30;
  uInt32ToReal(30,&n30);
  if(checkMinimumDataPoints(&n30)) {
    lrChosen = CF_ORTHOGONAL_FITTING;                      //force to ORTHOF only
    lrSelection = CF_ORTHOGONAL_FITTING;
    processCurvefitSelection(lrChosen,&RR,&SMI,&aa0,&aa1,&aa2);
    liftStack();
    setSystemFlag(FLAG_ASLIFT);
    reallocateRegister(REGISTER_X, dtReal34, 0, amNone);
    convertRealToReal34ResultRegister(&SMI, REGISTER_X);
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
  real_t RR,MX,SX,SY,RR2,MX2,SX2,SY2,UU,SS,TT,aa0,aa1,aa2,SMI;

  //LINF:   SA0 & SA1: RR2, n, Sy, Sx, Sx^2, Mx^2
  //EXPF:   new value for SY2, SY and use REX
  //POWERF: new values for SX2, SX,  SY2, SY, MX
  //LOGF:   new values for SX2, SX, MX
  //ORTOF:  new values same as LINF

  if(checkMinimumDataPoints(const_2)) {
    realCopy(const_0,&aa0);
    realCopy(const_0,&aa1);
    realCopy(const_0,&aa2);

    if(lrChosen == 0) {                    //if lrChosen contains something, the stat data exists, otherwise set it to linear. lrSelection still has 1 at this point, i.e. the * will not appear.
      lrChosen = CF_LINEAR_FITTING;
    }
    //Replaced fnStatR       (&RR,&SXY,&SX,&SY);
    processCurvefitSelectionAll(lrChosen, &RR, &MX, &MX2, &SX2, &SY2, &SMI, &aa0, &aa1, &aa2);  //Can be optimised a lot, as fnStatR is also called here
    realMultiply  (&RR,&RR,&RR2,realContext);               //   --> r^2

    #if defined(STATDEBUG) && defined(PC_BUILD)
      printf("##### processCurvefitSA:\n");
      printRealToConsole(&RR2, "§§ RR^2: ","\n");
      printRealToConsole(&MX2, "§§ MX^2: ","\n");
      printRealToConsole(&SX2, "§§ SX^2: ","\n");
      printRealToConsole(&SY2, "§§ SY^2: ","\n");
    #endif // STATDEBUG && PC_BUILD


    switch(orOrtho(lrChosen)) {
      case CF_LINEAR_FITTING:
      case CF_ORTHOGONAL_FITTING:
      case CF_EXPONENTIAL_FITTING:
      case CF_POWER_FITTING:
      case CF_LOGARITHMIC_FITTING: {
        //All parameters set from processCurvefitSelectionAll
        break;
      }
      default: {
        displayCalcErrorMessage(ERROR_NO_ERRORS_CALCULABLE, ERR_REGISTER_LINE, REGISTER_X);
        #if (EXTRA_INFO_ON_CALC_ERROR == 1)
          moreInfoOnError("In function processCurvefitSA:", "No errors are calculable for the selected/chosen model!", NULL, NULL);
        #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
        return false;
      }
    }

    realSquareRoot(&SX2,&SX,realContext);
    realSquareRoot(&SY2,&SY,realContext);

    //RR2,SY,SX,SX2,MX2 SS,TT,UU > SA1 SA0
    realSubtract  (const_1, &RR2, &SS, realContext);       //SA1 = f( RR2, n, SY, SX )
    realSubtract  (SIGMA_N, const_2, &TT, realContext);
    realDivide    (&SS, &TT, &UU, realContext);
    realSquareRoot(&UU, &UU, &ctxtReal39);
    realMultiply  (&UU, &SY, &UU, &ctxtReal39);
    realDivide    (&UU, &SX, SA1, &ctxtReal39);

    realSubtract  (SIGMA_N, const_1, &SS, realContext);    //SA0 = f( n, SX2, MX2, SA1 )
    realDivide    (&SS, SIGMA_N, &SS, realContext);        //SS = (n-1)/n
    realMultiply  (&SS, &SX2, &SS, realContext);           //SS = SS * SX^2
    realAdd       (&SS, &MX2, &SS, realContext);
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
