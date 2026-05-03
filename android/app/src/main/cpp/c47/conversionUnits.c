// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file conversionUnits.c
 ***********************************************/

#include "c47.h"

#define inverting true
#define noninverting false



static void unitConversion(const real_t * const coefficient, uint16_t multiplyDivide, bool_t invert) {
  real_t reX;

  if(!getRegisterAsReal(REGISTER_X, &reX)) {
    return;
  }

  if(!saveLastX()) {
    return;
  }

  if(invert && realIsZero(&reX)) {
    if(getSystemFlag(FLAG_SPCRES)) {
      convertRealToResultRegister(realIsNegative(&reX) ? const_minusInfinity : const_plusInfinity, REGISTER_X, amNone);
    }
    else {
      displayCalcErrorMessage(ERROR_ARG_EXCEEDS_FUNCTION_DOMAIN, ERR_REGISTER_LINE, REGISTER_X);
      #if (EXTRA_INFO_ON_CALC_ERROR == 1)
        moreInfoOnError("In function unitConversion:", "cannot calculate divide by zero", NULL, NULL);
      #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)
    }
  }

  if(invert) {
    realDivide(const_1, &reX, &reX, &ctxtReal39);
  }

  if(multiplyDivide == multiply) {
    realMultiply(&reX, coefficient, &reX, &ctxtReal39);
  }
  else {
    realDivide(&reX, coefficient, &reX, &ctxtReal39);
  }

  convertRealToResultRegister(&reX, REGISTER_X, amNone);
  adjustResult(REGISTER_X, false, false, -1, -1, -1);
}

TO_QSPI static const real_t *conversionFactors[constFactorEND] = {
    [constFactorFt2Hectare]   = const_Ft2ToHa,      /*   0 */
    [constFactorFt2M2]        = const_Ft2ToM2,
    [constFactorHectareKm2]   = const_100,
    [constFactorAcreHa]       = const_AccreToHa,
    [constFactorAcreusHa]     = const39_AccreusToHa,
    [constFactorAtmPa]        = const_AtmToPa,
    [constFactorAuM]          = const_AuToM,
    [constFactorBarPa]        = const_BarToPa,
    [constFactorBtuJ]         = const_BtuToJ,
    [constFactorCalJ]         = const_CalToJ,
    [constFactorLbfftNm]      = const_LbfftToNm,    /*  10 */
    [constFactorCwtKg]        = const_CwtToKg,
    [constFactorFtM]          = const_FtToM,
    [constFactorSfeetM]       = const39_SfeetToM,
    [constFactorFlozukIn3]    = const_FlozukToIn3,
    [constFactorFlozukMl]     = const_FlozukToMl,
    [constFactorFlozusIn3]    = const_FlozusToIn3,
    [constFactorFlozusMl]     = const_FlozusToMl,
    [constFactorFt3toGalUS]   = const_Ft3ToGalUS,
    [constFactorGalukL]       = const_GalukToL,
    [constFactorGalusL]       = const_GalusToL,     /*  20 */
    [constFactorHpeW]         = const_HpeToW,
    [constFactorHpmW]         = const_HpmToW,
    [constFactorHpukW]        = const_HpukToW,
    [constFactorInhgPa]       = const_InhgToPa,
    [constFactorInchMm]       = const_InchToMm,
    [constFactorWhJ]          = const_WhToJ,
    [constFactorLbKg]         = const_LbToKg,
    [constFactorOzG]          = const_OzToG,
    [constFactorShortcwtKg]   = const_ShortcwtToKg,
    [constFactorStoneKg]      = const_StoneToKg,    /*  30 */
    [constFactorShorttonKg]   = const_ShorttonToKg,
    [constFactorTonKg]        = const_TonToKg,
    [constFactorLiangKg]      = const_20,
    [constFactorTrozG]        = const_TrozToG,
    [constFactorLbfN]         = const_LbfToN,
    [constFactorLyM]          = const_LyToM,
    [constFactorMmhgPa]       = const_MmhgToPa,
    [constFactorMiKm]         = const_MiToKm,
    [constFactorNmiKm]        = const_NmiToKm,
    [constFactorPcM]          = const39_PcToM,      /*  40 */
    [constFactorPointMm]      = const39_PointToMm,
    [constFactorMileM]        = const_MiToM,
    [constFactorYardM]        = const_YardToM,
    [constFactorPsiPa]        = const39_PsiToPa,
    [constFactorTorrPa]       = const39_TorrToPa,
    [constFactorYearS]        = const_YearToS,
    [constFactorCaratG]       = const_CaratToG,
    [constFactorJinKg]        = const_2,
    [constFactorQuartL]       = const_QuartToL,
    [constFactorFathomM]      = const_FathomToM,    /*  50 */
    [constFactorNMiM]         = const_NmiToM,
    [constFactorBarrelM3]     = const_BarrelToM3,
    [constFactorHectareM2]    = const_10000,
    [constFactorMuM2]         = const_MuToM2,
    [constFactorLiM]          = const_LiToM,
    [constFactorChiM]         = const_3,
    [constFactorYinM]         = const_YinToM,
    [constFactorCunM]         = const_CunToM,
    [constFactorZhangM]       = const_ZhangToM,
    [constFactorFenM]         = const_FenToM,       /*  60 */
    [constFactorMi2Km2]       = const_MiSqToKmSq,
    [constFactorNmi2Km2]      = const_NmiSqToKmSq,
    [constFactorKmphmps]      = const39_Kmphmps,
    [constFactorRpmDegps]     = const_RpmDegps,
    [constFactorMphmps]       = const_Mphmps,
    [constFactorRpmRadps]     = const39_RpmRadps,
    [constFactorInchCm]       = const_InchToCm,
    [constFactorNmiMi]        = const39_NmiToMi,
    [constFactorFurtom]       = const_furToM,
    [constFactorFtntos]       = const_ftnToS,       /*  70 */
    [constFactorFpftomps]     = const_fpfToMps,
    [constFactorBrdstom]      = const_brdsTom,
    [constFactorFirtokg]      = const_firToKg,
    [constFactorFpftokph]     = const_fpfToKph,
    [constFactorBrdstoin]     = const_brdsToIn,
    [constFactorFirtolb]      = const_firToLb,
    [constFactorFpftomph]     = const_fpfToMph,
    [constFactorFpstokph]     = const_fpsToKph,
    [constFactorFpstomps]     = const_fpsToMps,
    [constFactorL100Tokml]    = const_100,          /*  80 */
    [constFactorKmletok100K]  = NULL,
    [constFactorK100Ktokmk]   = const_100,
    [constFactorL100Tomgus]   = NULL,
    [constFactorMgeustok100M] = NULL,
    [constFactorK100Ktok100M] = const_MiToKm,
    [constFactorL100Tomguk]   = NULL,
    [constFactorMgeuktok100M] = NULL,
    [constFactorK100Mtomik]   = const_100,
    [constFactorCupcFzus]     = const_CupcFzus,
    [constFactorCupcMl]       = const_CupcMl,       /*  90 */
    [constFactorCupukFzuk]    = const_CupukFzuk,
    [constFactorCupukMl]      = const_CupukMl,
    [constFactorFzukCupuk]    = const_CupukFzuk,
    [constFactorFzukTbspuk]   = const_FzukTbspuk,
    [constFactorFzukTspuk]    = const_FzukTspuk,
    [constFactorFzusCupc]     = const_CupcFzus,
    [constFactorFzusTbspc]    = const_FzusTbspc,
    [constFactorFzusTspc]     = const_FzusTspc,
    [constFactorMlCupc]       = const_CupcMl,
    [constFactorMlCupuk]      = const_CupukMl,      /* 100 */
    [constFactorMlPintlq]     = const_PintlqMl,
    [constFactorMlPintuk]     = const_PintukMl,
    [constFactorMlQt]         = const_QtMl,
    [constFactorMlQtus]       = const_QtusMl,
    [constFactorMlTbspc]      = const_TbspcMl,
    [constFactorMlTbspuk]     = const_TbspukMl,
    [constFactorMlTspc]       = const_TspcMl,
    [constFactorMlTspuk]      = const39_TspukMl,
    [constFactorPintlqMl]     = const_PintlqMl,
    [constFactorPintukMl]     = const_PintukMl,     /* 110 */
    [constFactorQtMl]         = const_QtMl,
    [constFactorQtusMl]       = const_QtusMl,
    [constFactorTbspcFzus]    = const_FzusTbspc,
    [constFactorTbspcMl]      = const_TbspcMl,
    [constFactorTbspukFzuk]   = const_FzukTbspuk,
    [constFactorTbspukMl]     = const_TbspukMl,
    [constFactorTspcFzus]     = const_FzusTspc,
    [constFactorTspcMl]       = const_TspcMl,
    [constFactorTspukFzuk]    = const_FzukTspuk,
    [constFactorTspukMl]      = const39_TspukMl,    /* 120 */
    [constFactorMlIn3]        = const_In3Ml,
    [constFactorIn3Ml]        = const_In3Ml,
    [constFactorFt3Gluk]      = const_Ft3Gluk,
    [constFactorGlukFt3]      = const_Ft3Gluk,
    [constFactorLFt3]         = const_Ft3L,
    [constFactorFt3L]         = const_Ft3L,
    [constFactorLQtus]        = const_LQtus,
    [constFactorQtusL]        = const_LQtus,
    [constFactorGlukFzuk]     = const_GlukFzuk,
    [constFactorFzukGluk]     = const_GlukFzuk,     /* 130 */
    [constFactorGlusFzus]     = const_GlusFzus,
    [constFactorFzusGlus]     = const_GlusFzus,
    [constFactoreVJ]          = const_e,
    [constFactorJeV]          = const_e,
    [constFactormmBanana]     = const_bananamm,
    [constFactorBananamm]     = const_bananamm,
    [constFactorInchBanana]   = const39_bananaInch,
    [constFactorBananaInch]   = const39_bananaInch,
    [constFactorErgJ]         = const_ErgToJ,
    [constFactorFoeJ]         = const_FoeToJ,       /* 140 */
  };

void fnUnitConvert(uint16_t arg) {
    const uint16_t multiply = arg & 0x8000;
    const bool_t invert = (arg & 0x4000) != 0;
    const uint16_t idx = arg & 0x3fff;

    unitConversion(conversionFactors[idx], multiply, invert);
}


//  {[(x - B) / C] * D} + E
TO_QSPI static const real_t *cvtTempConsts[13][4] = {
  //   B              C             D             E
  {const_0,       const_1,       const_9on5,    const_32     }, // ITM_CtoF     ix =  0
  {const_32,      const_9on5,    const_1,       const_0      }, // ITM_FtoC     ix =  1
  {const_0,       const_1,       const_1,       const_273p15 }, // ITM_CtoK     ix =  2
  {const_273p15,  const_1,       const_1,       const_0      }, // ITM_KtoC     ix =  3
  {const_0,       const_9on5,    const_1,       const_0      }, // ITM_RAtoK    ix =  4
  {const_0,       const_1,       const_9on5,    const_0      }, // ITM_KtoRA    ix =  5
  {const_459p67,  const_1,       const_1,       const_0      }, // ITM_RAtoF    ix =  6
  {const_0,       const_1,       const_1,       const_459p67 }, // ITM_FtoRA    ix =  7
  {const_0,       const39_kBeVK, const_1,       const_0      }, // ITM_EVKBtoK  ix =  8
  {const_0,       const_1,       const39_kBeVK, const_0      }, // ITM_KtoEVKB  ix =  9
  {const_32,      const_9on5,    const_1,       const_273p15 }, // ITM_FtoK     ix = 10
  {const_273p15,  const_1,       const_9on5,    const_32     }, // ITM_KtoF     ix = 11
};

void fnCvtTemp(uint16_t ix) {
  real_t reX;

  if(!getRegisterAsReal(REGISTER_X, &reX)) {
    return;
  }

  if(!saveLastX()) {
    return;
  }

  //  (x - B) / C * D + E

  //printf("ix = %d\n",ix);
  //printRealToConsole(cvtTempConsts[ix][0], "(x - "," ) \n");
  //printRealToConsole(cvtTempConsts[ix][1], "/ "," \n");
  //printRealToConsole(cvtTempConsts[ix][2], "x "," \n");
  //printRealToConsole(cvtTempConsts[ix][3], "+ ","\n");
  if(cvtTempConsts[ix][0] != const_0) {realSubtract(&reX, cvtTempConsts[ix][0], &reX, &ctxtReal39);}
  if(cvtTempConsts[ix][1] != const_1) {realDivide  (&reX, cvtTempConsts[ix][1], &reX, &ctxtReal39);}
  if(cvtTempConsts[ix][2] != const_1) {realMultiply(&reX, cvtTempConsts[ix][2], &reX, &ctxtReal39);}
  if(cvtTempConsts[ix][3] != const_0) {realAdd     (&reX, cvtTempConsts[ix][3], &reX, &ctxtReal39);}
  //printRealToConsole(&reX, "Rex: ","\n");

  convertRealToResultRegister(&reX, REGISTER_X, amNone);

  adjustResult(REGISTER_X, false, false, -1, -1, -1);
}


void fnCvtDegRad(uint16_t multiplyDivide) {
  if(getRegisterDataType(REGISTER_X) == dtReal34 && (
    ((getRegisterAngularMode(REGISTER_X) == amDegree) && multiplyDivide == divide) || ((getRegisterAngularMode(REGISTER_X) == amRadian) && multiplyDivide == multiply) )) {
    setRegisterAngularMode(REGISTER_X, amNone);
  }
  unitConversion(const39_180onPi, multiplyDivide, noninverting);
}

void fnCvtDegGrad(uint16_t multiplyDivide) {
  if(getRegisterDataType(REGISTER_X) == dtReal34 && (
    ((getRegisterAngularMode(REGISTER_X) == amDegree) && multiplyDivide == divide) || ((getRegisterAngularMode(REGISTER_X) == amGrad) && multiplyDivide == multiply) )) {
    setRegisterAngularMode(REGISTER_X, amNone);
  }
  unitConversion(const_9on10, multiplyDivide, noninverting);
}

void fnCvtGradRad(uint16_t multiplyDivide) {
  if(getRegisterDataType(REGISTER_X) == dtReal34 && (
    ((getRegisterAngularMode(REGISTER_X) == amGrad) && multiplyDivide == divide) || ((getRegisterAngularMode(REGISTER_X) == amRadian) && multiplyDivide == multiply) )) {
    setRegisterAngularMode(REGISTER_X, amNone);
  }
  unitConversion(const39_200onPi, multiplyDivide, noninverting);
}

void fnKmletok100K   (uint16_t multiplyDivide) {
  //note multiplyDivide is not used, as the formula is biderectional!
  //100*liter_equivalent  / (value), both ways
  real_t factor;
  realMultiply(const_GaluseqE, const_100, &factor, &ctxtReal39);
  realDivide(&factor, const_GalusToL, &factor, &ctxtReal39);
  unitConversion(&factor, multiply, inverting);
}

void fnL100Tomgus   (uint16_t multiplyDivide) {
  //note multiplyDivide is not used, as the formula is biderectional!
  //100 *gallon_US/mile   /  (value), both ways
  real_t factor;
  realMultiply(const_100, const_GalusToL, &factor, &ctxtReal39);
  realDivide(&factor, const_MiToKm, &factor, &ctxtReal39);
  unitConversion(&factor, multiply, inverting);
}

void fnMgeustok100M   (uint16_t multiplyDivide) {
  //note multiplyDivide is not used, as the formula is biderectional!
  //100*gallon_US_equivalent / (value), both ways
  real_t factor;
  realMultiply(const_GaluseqE, const_100, &factor, &ctxtReal39);
  unitConversion(&factor, multiply, inverting);
}

void fnL100Tomguk   (uint16_t multiplyDivide) {
  //note multiplyDivide is not used, as the formula is biderectional!
  //100*gallon_UK/mile  / (value), both ways
  real_t factor;
  realMultiply(const_100, const_GalukToL, &factor, &ctxtReal39);
  realDivide(&factor, const_MiToKm, &factor, &ctxtReal39);
  unitConversion(&factor, multiply, inverting);
}

void fnMgeuktok100M   (uint16_t multiplyDivide) {
  //note multiplyDivide is not used, as the formula is biderectional!
  //100*gallon_UK_equivalent  / (value), both ways
  //const_GalukToL / const_GalusToL * 33.7 * 100
  real_t factor;
  realMultiply(const_GaluseqE, const_100, &factor, &ctxtReal39);
  realMultiply(&factor, const_GalukToL, &factor, &ctxtReal39);
  realDivide(&factor, const_GalusToL, &factor, &ctxtReal39);
  unitConversion(&factor, multiply, inverting);
}


void fnCvtHMSHR   (uint16_t multiplyDivide) {
  if(multiplyDivide == divide) {
    fnHMStoTM(0);
    fnToReal(0);
  }
  else {
    fnHRtoTM(0);
    fnFrom_ms(0);
  }
}





/********************************************//**
 * \brief Converts power or field ratio to dB
 * dB = (10 or 20) * log10((power or field) ratio) this is the exact formula
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnCvtRatioDb(uint16_t tenOrTwenty) { // ten: power ratio   twenty: field ratio
  real_t reX;

  if(!getRegisterAsReal(REGISTER_X, &reX)) {
    return;
  }

  if(!saveLastX()) {
    return;
  }

  WP34S_Log10(&reX, &reX, &ctxtReal39);
  realMultiply(&reX, (tenOrTwenty == 10 ? const_10 : const_20), &reX, &ctxtReal39);

  convertRealToResultRegister(&reX, REGISTER_X, amNone);

  adjustResult(REGISTER_X, false, false, -1, -1, -1);
}



/********************************************//**
 * \brief Converts dB to power or field ratio
 * (power or field) ratio = 10^(dB / 10 or 20) this is the exact formula
 *
 * \param[in] unusedButMandatoryParameter uint16_t
 * \return void
 ***********************************************/
void fnCvtDbRatio(uint16_t tenOrTwenty) { // ten: power ratio   twenty: field ratio
  real_t reX;

  if(!getRegisterAsReal(REGISTER_X, &reX)) {
    return;
  }

  if(!saveLastX()) {
    return;
  }

  realDivide(&reX, (tenOrTwenty == 10 ? const_10 : const_20), &reX, &ctxtReal39);
  realPower10(&reX, &reX, &ctxtReal39);

  convertRealToResultRegister(&reX, REGISTER_X, amNone);

  adjustResult(REGISTER_X, false, false, -1, -1, -1);
}
