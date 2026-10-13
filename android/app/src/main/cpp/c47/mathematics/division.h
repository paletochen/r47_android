// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file division.h
 ***********************************************/
#if !defined(DIVISION_H)
  #define DIVISION_H

  void fnDivide(uint16_t unusedButMandatoryParameter);

  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    void divError(void);
  #else // (EXTRA_INFO_ON_CALC_ERROR == 1)
    #define divError typeError
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)

  void divComplexComplex(const real_t *numerReal, const real_t *numerImag,  const real_t *denomReal, const real_t *denomImag,   real_t *quotientReal, real_t *quotientImag, realContext_t *realContext);
  void divRealComplex   (const real_t *numer,                               const real_t *denomReal, const real_t *denomImag,   real_t *quotientReal, real_t *quotientImag, realContext_t *realContext);
  void divComplexReal   (const real_t *numerReal, const real_t *numerImag,  const real_t *denom,                                real_t *quotientReal, real_t *quotientImag, realContext_t *realContext);

//  void divComplexComplex159(const real_t *numerReal, const real_t *numerImag, const real_t *denomReal, const real_t *denomImag, real_t *quotientReal, real_t *quotientImag, realContext_t *realContext);

  //      RegYRegX
  void divLonILonI(void);
  void divLonIRema(void);
  void divLonICxma(void);
  void divLonIShoI(void);
  void divLonIReal(void);
  void divLonICplx(void);
  void divLonITime(void);

  //      RegYRegX
  void divTimeLonI(void);
  void divTimeShoI(void);
  void divTimeReal(void);
  void divTimeTime(void);

  //      RegYRegX
  void divRemaLonI(void);
  void divRemaRema(void);
  void divRemaCxma(void);
  void divRemaShoI(void);
  void divRemaReal(void);
  void divRemaCplx(void);

  //      RegYRegX
  void divCxmaLonI(void);
  void divCxmaRema(void);
  void divCxmaCxma(void);
  void divCxmaShoI(void);
  void divCxmaReal(void);
  void divCxmaCplx(void);

  //      RegYRegX
  void divShoILonI(void);
  void divShoIRema(void);
  void divShoICxma(void);
  void divShoIShoI(void);
  void divShoIReal(void);
  void divShoICplx(void);
  void divShoITime(void);

  //      RegYRegX
  void divRealLonI(void);
  void divRealRema(void);
  void divRealCxma(void);
  void divRealShoI(void);
  void divRealReal(void);
  void divRealCplx(void);
  void divRealTime(void);

  //      RegYRegX
  void divCplxLonI(void);
  void divCplxRema(void);
  void divCplxCxma(void);
  void divCplxShoI(void);
  void divCplxReal(void);
  void divCplxCplx(void);
#endif // !DIVISION_H
