// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file wp34s.h
 ***********************************************/
#if !defined(WP34S_H)
  #define WP34S_H

  /******************************************************
   * This functions are borrowed from the WP34S project
   ******************************************************/

  typedef bool_t (*wp34sDistributionDomainRule_t)(bool_t registersValid, real_t *x, real_t *first, real_t *second, real_t *third, uint16_t parameter);

  void   reduceAngleToRange        (real_t* angle, const real_t** angle45, const real_t** angle90, const real_t** angle180, angularMode_t* angularMode, int32_t savedContextDigits, realContext_t* realContext);
  void   C47_WP34S_Cvt2RadSinCosTan(const real_t *angle, angularMode_t am, real_t *sin, real_t *cos, real_t *tan, realContext_t *realContext);
  void   C47_WP34S_SinCosTanTaylor (const real_t *angle, bool_t swap, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext); // angle in radian, only used in bessel.c & cauchy.c
  void   C47_WP34S_Asin            (const real_t *x, real_t *angle, realContext_t *realContext);
  void   C47_WP34S_Acos            (const real_t *x, real_t *angle, realContext_t *realContext);
  void   C47_WP34S_Atan            (const real_t *x, real_t *angle, realContext_t *realContext);
  void   C47_WP34S_Atan2           (const real_t *y, const real_t *x, real_t *atan, realContext_t *realContext);

  void   WP34S_Factorial       (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_AdjustPositiveUnderflow(real_t *value);
  realContext_t *WP34S_AtLeastContext(realContext_t *realContext, realContext_t *fallbackContext);
  realContext_t *WP34S_BetaCalculationContext(const real_t *a, const real_t *b);
  bool_t WP34S_CheckDistributionParameters(real_t *x, calcRegister_t firstRegister, real_t *first, calcRegister_t secondRegister, real_t *second, calcRegister_t thirdRegister, real_t *third, uint16_t parameter, wp34sDistributionDomainRule_t domainRule);

  // A distribution's four entry points differ in the stat registers its parameters live in, the domain rule, the kernel, and the context each one works at, so one
  // body serves them all and the descriptor states the difference. The kernels keep the argument lists they already have, and arity says which member to call.
  typedef union {
    void (*one)  (const real_t *x, const real_t *a, real_t *res, realContext_t *realContext);
    void (*two)  (const real_t *x, const real_t *a, const real_t *b, real_t *res, realContext_t *realContext);
  } distributionKernel_t;

  #define DIST_CTX_39 0
  #define DIST_CTX_51 1
  #define DIST_CTX_75 2

  // What each distribution does on top of the five lines every entry shares. No two of them take
  // the same set, which is why the difference is in the table rather than in sixty-four routines.
  #define DIST_INV_NAN_ARG      0x01   // the inverse refuses a NaN argument as well as one outside 0 to 1
  #define DIST_INV_REPORT_FIRST 0x02   // the inverse reports the domain error before the special-result branch, not instead of it
  #define DIST_INV_NOROOT       0x04   // an inverse that answers NaN raises ERROR_NO_ROOT_FOUND
  #define DIST_INV_NOROOT_NAN   0x08   // and puts NaN in X when special results are on
  #define DIST_ERR_RETURNS      0x10   // that branch returns rather than falling through to adjustResult
  #define DIST_PDF_INTEGER_ONLY 0x20   // the density is zero at a non-integer argument
  #define DIST_PDF_NAN_IS_ERROR 0x40   // a density that answers NaN raises ERROR_INVALID_DISTRIBUTION_PARAM

  typedef struct {
    uint8_t                       firstRegister;    // the stat register the first parameter lives in
    uint8_t                       secondRegister;   // and the second, 0 when the distribution takes one parameter
    uint8_t                       arity;            // how many parameters the kernel takes after x
    uint8_t                       secondFirst;      // the kernel wants the second parameter before the first
    uint8_t                       flags;            // the DIST_ bits above
    uint8_t                       context[4];       // DIST_CTX_* per entry, in the order density, lower tail, upper tail, inverse
    wp34sDistributionDomainRule_t domainRule;
    distributionKernel_t          kernel[4];        // density, lower tail, upper tail, inverse
  } distribution_t;

  #define DIST_P 0
  #define DIST_L 1
  #define DIST_R 2
  #define DIST_I 3

  #define DIST_EXPON       0
  #define DIST_CAUCHY      1
  #define DIST_LOGIT       2
  #define DIST_WEIB        3
  #define DIST_BINOM       4
  #define DIST_GEOM        5
  #define DIST_POISSON     6
  #define DIST_NEGBINOM    7
  #define DIST_CHI2        8
  #define DIST_T           9
  #define DIST_F          10
  #define DIST_COUNT   11

  extern const distribution_t distributions[DIST_COUNT];

  void fnDistribution(uint16_t param);
  void   WP34S_DistributionProbabilityData(const real_t *probability, real_t *complement, real_t *logProbability, real_t *logComplement, real_t *logit, realContext_t *realContext);
  bool_t WP34S_HasCancellation(const real_t *left, const real_t *right, const real_t *result, int32_t places);
  bool_t WP34S_IsPositiveFinite(const real_t *value);
  void   WP34S_LanczosSum      (const real_t *x, const real_t *offset, real_t *res, realContext_t *realContext);
  void   WP34S_LnExtended      (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_Log1pMinus      (const real_t *value, real_t *res, realContext_t *realContext);
  void   WP34S_MarkPositive    (real_t *value, realContext_t *realContext);
  void   WP34S_MarkProbability (real_t *value, realContext_t *realContext);
  void   WP34S_SetPowerOfTen   (int32_t exponent, real_t *res);
  void   WP34S_LnGamma         (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_Gamma           (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_Ln              (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_Log10           (const real_t *x, real_t *res, realContext_t *realContext);
  //void   WP34S_Log2            (const real_t *x, real_t *res, realContext_t *realContext); never used
  void   WP34S_Logxy           (const real_t *yin, const real_t *xin, real_t *res, realContext_t *realContext);
  void   WP34S_SinhCosh        (const real_t *x, real_t *sinOut, real_t *cosOut, realContext_t *realContext);
  void   WP34S_Tanh            (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_ArcSinh         (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_TinyOddSeries   (const real_t *x, const real_t *divisor, bool_t above, real_t *res, realContext_t *realContext);
  //void   WP34S_ArcCosh         (const real_t *x, real_t *res, realContext_t *realContext); never used
  void   WP34S_ArcTanh         (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_Ln1P            (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_ExpM1           (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_ComplexGamma    (const cplx_t *zin, cplx_t *res, realContext_t *realContext);
  void   WP34S_ComplexLnGamma  (const cplx_t *zin, cplx_t *res, realContext_t *realContext);
  void   WP34S_Mod             (const real_t *x, const real_t *y, real_t *res, realContext_t *realContext);
  void   WP34S_BigMod          (const real_t *x, const real_t *y, real_t *res, realContext_t *realContext);
  void   WP34S_Real34Remainder (const real_t *x, const real_t *y, real_t *res);
  void   mod2Pi                (const real_t *x, real_t *res, realContext_t *realContext);
  bool_t WP34S_RelativeError   (const real_t *x, const real_t *y, const real_t *tol, realContext_t *realContext);
  bool_t WP34S_AbsoluteError   (const real_t *x, const real_t *y, const real_t *tol, realContext_t *realContext);
  bool_t WP34S_ComplexRelativeError (const cplx_t *x, const cplx_t *y, const real_t *tol, realContext_t *realContext);
  bool_t WP34S_ComplexAbsError (const cplx_t *x, const cplx_t *y, const real_t *tol, realContext_t *realContext);
  bool_t WP34S_BetaFraction    (const real_t *a, const real_t *b, const real_t *x, int32_t convergenceExponent, int32_t iterationMax, real_t *res, realContext_t *realContext);
  void   WP34S_GammaP          (const real_t *x, const real_t *a, real_t *res, realContext_t *realContext, bool_t upper, bool_t regularised);
  void   WP34S_Erf             (const real_t *x, real_t *res, realContext_t *realContext);
  #if defined(OPTION_DIST_NORML)
  void   WP34S_Erfc            (const real_t *x, real_t *res, realContext_t *realContext);
  #endif // OPTION_DIST_NORML
  void   WP34S_betai           (const real_t *b, const real_t *a, const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_Bernoulli       (const real_t *x, real_t *res, bool_t bn_star, realContext_t *realContext);
  void   WP34S_Zeta            (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_LambertW        (const real_t *x, real_t *res, bool_t negativeBranch, realContext_t *realContext);
  void   WP34S_ComplexLambertW (const cplx_t *x, cplx_t *res, realContext_t *realContext);
  void   WP34S_InverseW        (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_InverseComplexW (const cplx_t *x, cplx_t *res, realContext_t *realContext);
  void   WP34S_OrthoPoly       (uint16_t kind, const real_t *x, const real_t *n, const real_t *param, real_t *res, realContext_t *realContext);
#endif // !WP34S_H
