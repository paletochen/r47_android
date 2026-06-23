// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file conversionUnits.h
 ***********************************************/
#if !defined(CONVERSIONUNITS_H)
  #define CONVERSIONUNITS_H

  enum {
    constFactorFt2Hectare,      /*   0 */
    constFactorFt2M2,
    constFactorHectareKm2,
    constFactorAcreHa,
    constFactorAcreusHa,
    constFactorAtmPa,
    constFactorAuM,
    constFactorBarPa,
    constFactorBtuJ,
    constFactorCalJ,
    constFactorLbfftNm,         /*  10 */
    constFactorCwtKg,
    constFactorFtM,
    constFactorSfeetM,
    constFactorFlozukIn3,
    constFactorFlozukMl,
    constFactorFlozusIn3,
    constFactorFlozusMl,
    constFactorFt3toGalUS,
    constFactorGalukL,
    constFactorGalusL,          /*  20 */
    constFactorHpeW,
    constFactorHpmW,
    constFactorHpukW,
    constFactorInhgPa,
    constFactorInchMm,
    constFactorWhJ,
    constFactorLbKg,
    constFactorOzG,
    constFactorShortcwtKg,
    constFactorStoneKg,         /*  30 */
    constFactorShorttonKg,
    constFactorTonKg,
    constFactorLiangKg,
    constFactorTrozG,
    constFactorLbfN,
    constFactorLyM,
    constFactorMmhgPa,
    constFactorMiKm,
    constFactorNmiKm,
    constFactorPcM,             /*  40 */
    constFactorPointMm,
    constFactorMileM,
    constFactorYardM,
    constFactorPsiPa,
    constFactorTorrPa,
    constFactorYearS,
    constFactorCaratG,
    constFactorJinKg,
    constFactorQuartL,
    constFactorFathomM,         /*  50 */
    constFactorNMiM,
    constFactorBarrelM3,
    constFactorHectareM2,
    constFactorMuM2,
    constFactorLiM,
    constFactorChiM,
    constFactorYinM,
    constFactorCunM,
    constFactorZhangM,
    constFactorFenM,            /*  60 */
    constFactorMi2Km2,
    constFactorNmi2Km2,
    constFactorKmphmps,
    constFactorRpmDegps,
    constFactorMphmps,
    constFactorRpmRadps,
    constFactorInchCm,
    constFactorNmiMi,
    constFactorFurtom,
    constFactorFtntos,          /*  70 */
    constFactorFpftomps,
    constFactorBrdstom,
    constFactorFirtokg,
    constFactorFpftokph,
    constFactorBrdstoin,
    constFactorFirtolb,
    constFactorFpftomph,
    constFactorFpstokph,
    constFactorFpstomps,
    constFactorL100Tokml,       /*  80 */
    constFactorKmletok100K,
    constFactorK100Ktokmk,
    constFactorL100Tomgus,
    constFactorMgeustok100M,
    constFactorK100Ktok100M,
    constFactorL100Tomguk,
    constFactorMgeuktok100M,
    constFactorK100Mtomik,
    constFactorCupcFzus,
    constFactorCupcMl,          /*  90 */
    constFactorCupukFzuk,
    constFactorCupukMl,
    constFactorFzukCupuk,
    constFactorFzukTbspuk,
    constFactorFzukTspuk,
    constFactorFzusCupc,
    constFactorFzusTbspc,
    constFactorFzusTspc,
    constFactorMlCupc,
    constFactorMlCupuk,         /* 100 */
    constFactorMlPintlq,
    constFactorMlPintuk,
    constFactorMlQt,
    constFactorMlQtus,
    constFactorMlTbspc,
    constFactorMlTbspuk,
    constFactorMlTspc,
    constFactorMlTspuk,
    constFactorPintlqMl,
    constFactorPintukMl,        /* 110 */
    constFactorQtMl,
    constFactorQtusMl,
    constFactorTbspcFzus,
    constFactorTbspcMl,
    constFactorTbspukFzuk,
    constFactorTbspukMl,
    constFactorTspcFzus,
    constFactorTspcMl,
    constFactorTspukFzuk,
    constFactorTspukMl,         /* 120 */
    constFactorMlIn3,
    constFactorIn3Ml,
    constFactorFt3Gluk,
    constFactorGlukFt3,
    constFactorLFt3,
    constFactorFt3L,
    constFactorLQtus,
    constFactorQtusL,
    constFactorGlukFzuk,
    constFactorFzukGluk,        /* 130 */
    constFactorGlusFzus,
    constFactorFzusGlus,
    constFactoreVJ,
    constFactorJeV,
    constFactormmBanana,
    constFactorBananamm,
    constFactorInchBanana,
    constFactorBananaInch,
    constFactorErgJ,
    constFactorFoeJ,            /* 140 */
    constFactorKnotMps,
    constFactor180onPi,
    constFactorSlugKg,
    constFactorSlinchKg,
    constFactorBlobKg,
    constFactorTonneKg,
    constFactorLbsft2Pa,
    constFactorInlbsNm,
    constFactorLbsftNpm,
    constFactorKgfN,            /* 150 */
    constFactorKsiMpa,
    constFactorLbsBlob,
    constFactorLbsin3Tmm3,
    constFactorLbsin3Kgm3,
    constFactorKgm3Blobin3,
    constFactorKgm3Tmm3,
    constFactorLbsftKgm,
    constFactorIn3Mm3,
    constFactorIn2Mm2,
    constFactorIn4Mm4,          /* 160 */
    constFactorIn6Mm6,
    constFactorKgmNpm,
    constFactorInchM,           /* 163 */
    constFactorLbfKgf,
    
    constFactorEND              /* MUST be last */
  };



  #define NUM_CONVERT_PAIRS  314
  extern const fInMim_t MimFunctionsType3Conv[NUM_CONVERT_PAIRS];

  bool_t  isStandardPair             (int16_t item1Nr, int16_t item2Nr);
  int16_t conversionPartner          (int16_t input, int16_t *unity, int8_t *exponent, uint8_t *type);
  bool_t  isItemConversion           (int16_t itemNr);
  bool_t  isOneOfAConvertPair        (uint16_t x, int16_t itemNr, int16_t *oddNrPartner);
  void    runConversionToSI          (int16_t itemNr);
  void    runConversionFromSI        (int16_t itemNr);
  bool_t  areBothConvertConfigurable (int16_t item1Nr, int16_t item2Nr);
  void    fullConvSoftMenuItemNameInclHPCONV(int16_t item, char *outString);
  void    executionConversionPartner (int16_t item, int16_t *itemNrPair, char *pairName);

  void fnUnitConvert  (uint16_t multiplyDivide);

  // Temperature
  void fnCvtTemp      (uint16_t ix);

  // ...
  void fnCvtRatioDb   (uint16_t tenOrTwenty);
  void fnCvtDbRatio   (uint16_t tenOrTwenty);

  // Angle
  void fnCvtDegRad    (uint16_t multiplyDivide);
  void fnCvtDegGrad   (uint16_t multiplyDivide);
  void fnCvtGradRad   (uint16_t multiplyDivide);

 //YMMV
  void fnKmletok100K  (uint16_t multiplyDivide);
  void fnL100Tomgus   (uint16_t multiplyDivide);
  void fnMgeustok100M (uint16_t multiplyDivide);
  void fnL100Tomguk   (uint16_t multiplyDivide);
  void fnMgeuktok100M (uint16_t multiplyDivide);

  //TIME
  void fnCvtHMSHR     (uint16_t multiplyDivide);



#endif // !CONVERSIONUNITS_H
