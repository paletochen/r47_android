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

  void   WP34S_Cvt2RadSinCosTan    (const real_t *angle, angularMode_t am, real_t *sin, real_t *cos, real_t *tan, realContext_t *realContext);
  void   WP34S_SinCosTanTaylor     (const real_t *angle, bool_t swap, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext); // angle in radian
  void   C47_WP34S_Cvt2RadSinCosTan(const real_t *angle, angularMode_t am, real_t *sin, real_t *cos, real_t *tan, realContext_t *realContext);
  void   C47_WP34S_SinCosTanTaylor (const real_t *angle, bool_t swap, real_t *sinOut, real_t *cosOut, real_t *tanOut, realContext_t *realContext); // angle in radian
  void   C47_WP34S_Asin            (const real_t *x, real_t *angle, realContext_t *realContext);
  void   C47_WP34S_Acos            (const real_t *x, real_t *angle, realContext_t *realContext);
  void   C47_WP34S_Atan            (const real_t *x, real_t *angle, realContext_t *realContext);
  void   C47_WP34S_Atan2           (const real_t *y, const real_t *x, real_t *atan, realContext_t *realContext);

  void   WP34S_Atan            (const real_t *x, real_t *angle, realContext_t *realContext);
  void   WP34S_Atan2           (const real_t *y, const real_t *x, real_t *angle, realContext_t *realContext);
  void   WP34S_Asin            (const real_t *x, real_t *angle, realContext_t *realContext);
  void   WP34S_Acos            (const real_t *x, real_t *angle, realContext_t *realContext);
  void   WP34S_Factorial       (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_LnGamma         (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_Gamma           (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_Ln              (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_Log10           (const real_t *x, real_t *res, realContext_t *realContext);
  //void   WP34S_Log2            (const real_t *x, real_t *res, realContext_t *realContext); never used
  void   WP34S_Logxy           (const real_t *yin, const real_t *xin, real_t *res, realContext_t *realContext);
  void   WP34S_SinhCosh        (const real_t *x, real_t *sinOut, real_t *cosOut, realContext_t *realContext);
  void   WP34S_Tanh            (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_ArcSinh         (const real_t *x, real_t *res, realContext_t *realContext);
  //void   WP34S_ArcCosh         (const real_t *x, real_t *res, realContext_t *realContext); never used
  void   WP34S_ArcTanh         (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_Ln1P            (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_ExpM1           (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_ComplexGamma    (const real_t *zinReal, const real_t *zinImag, real_t *resReal, real_t *resImag, realContext_t *realContext);
  void   WP34S_ComplexLnGamma  (const real_t *zinReal, const real_t *zinImag, real_t *resReal, real_t *resImag, realContext_t *realContext);
  void   WP34S_Mod             (const real_t *x, const real_t *y, real_t *res, realContext_t *realContext);
  void   WP34S_BigMod          (const real_t *x, const real_t *y, real_t *res, realContext_t *realContext);
  void   mod2Pi                (const real_t *x, real_t *res, realContext_t *realContext);
  bool_t WP34S_RelativeError   (const real_t *x, const real_t *y, const real_t *tol, realContext_t *realContext);
  bool_t WP34S_AbsoluteError   (const real_t *x, const real_t *y, const real_t *tol, realContext_t *realContext);
  bool_t WP34S_ComplexRelativeError (const real_t *xReal, const real_t *xImag, const real_t *yReal, const real_t *yImag, const real_t *tol, realContext_t *realContext);
  bool_t WP34S_ComplexAbsError (const real_t *xReal, const real_t *xImag, const real_t *yReal, const real_t *yImag, const real_t *tol, realContext_t *realContext);
  void   WP34S_GammaP          (const real_t *x, const real_t *a, real_t *res, realContext_t *realContext, bool_t upper, bool_t regularised);
  void   WP34S_Erf             (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_Erfc            (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_betai           (const real_t *b, const real_t *a, const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_Bernoulli       (const real_t *x, real_t *res, bool_t bn_star, realContext_t *realContext);
  void   WP34S_Zeta            (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_LambertW        (const real_t *x, real_t *res, bool_t negativeBranch, realContext_t *realContext);
  void   WP34S_ComplexLambertW (const real_t *xReal, const real_t *xImag, real_t *resReal, real_t *resImag, realContext_t *realContext);
  void   WP34S_InverseW        (const real_t *x, real_t *res, realContext_t *realContext);
  void   WP34S_InverseComplexW (const real_t *xReal, const real_t *xImag, real_t *resReal, real_t *resImag, realContext_t *realContext);
  void   WP34S_OrthoPoly       (uint16_t kind, const real_t *x, const real_t *n, const real_t *param, real_t *res, realContext_t *realContext);
#endif // !WP34S_H
