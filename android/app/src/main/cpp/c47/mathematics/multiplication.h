// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file multiplication.h
 ***********************************************/
#if !defined(MULTIPLICATION_H)
  #define MULTIPLICATION_H

  void fnMultiply (uint16_t unusedButMandatoryParameter);

  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    void mulError   (void);
  #else // (EXTRA_INFO_ON_CALC_ERROR != 1)
    #define mulError typeError
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)

  void mulComplexi(const real_t *inReal, const real_t *inImag, real_t *productReal, real_t *productImag);
  void mulComplexComplex(const real_t *factor1Real, const real_t *factor1Imag, const real_t *factor2Real, const real_t *factor2Imag, real_t *productReal, real_t *productImag, realContext_t *realContext);
  void mulComplexReal(const real_t *factor1Real, const real_t *factor1Imag, const real_t *factor2, real_t *productReal, real_t *productImag, realContext_t *realContext);

  // This re-write is needed as the mixing of decNumber types cannot deal with the real_t temporary variable withinn mulComplexComplex(). The 159 series is needed for 39 digit precision in cuberoots and the cubic formula solver
//  void mulComplexComplex159(const real_t *factor1Real, const real_t *factor1Imag, const real_t *factor2Real, const real_t *factor2Imag, real_t *productReal, real_t *productImag, realContext_t *realContext);

  //      RegYRegX
  void mulLonILonI(void);
  void mulLonITime(void);
  void mulLonIRema(void);
  void mulLonICxma(void);
  void mulLonIShoI(void);
  void mulLonIReal(void);
  void mulLonICplx(void);

  //      RegYRegX
  void mulTimeLonI(void);
  void mulTimeShoI(void);
  void mulTimeReal(void);

  //      RegYRegX
  void mulRemaLonI(void);
  void mulRemaRema(void);
  void mulRemaCxma(void);
  void mulRemaShoI(void);
  void mulRemaReal(void);
  void mulRemaCplx(void);

  //      RegYRegX
  void mulCxmaLonI(void);
  void mulCxmaRema(void);
  void mulCxmaCxma(void);
  void mulCxmaShoI(void);
  void mulCxmaReal(void);
  void mulCxmaCplx(void);

  //      RegYRegX
  void mulShoILonI(void);
  void mulShoITime(void);
  void mulShoIRema(void);
  void mulShoICxma(void);
  void mulShoIShoI(void);
  void mulShoIReal(void);
  void mulShoICplx(void);

  //      RegYRegX
  void mulRealLonI(void);
  void mulRealTime(void);
  void mulRealRema(void);
  void mulRealCxma(void);
  void mulRealShoI(void);
  void mulRealReal(void);
  void mulRealCplx(void);

  //      RegYRegX
  void mulCplxLonI(void);
  void mulCplxRema(void);
  void mulCplxCxma(void);
  void mulCplxShoI(void);
  void mulCplxReal(void);
  void mulCplxCplx(void);
#endif // !MULTIPLICATION_H
