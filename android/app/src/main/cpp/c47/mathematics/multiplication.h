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

  void mulComplexi(const cplx_t *in, cplx_t *product);
  void mulComplexComplex(const cplx_t *factor1, const cplx_t *factor2, cplx_t *product, realContext_t *realContext);
  void mulComplexReal(const cplx_t *factor1, const real_t *factor2, cplx_t *product, realContext_t *realContext);

  // This re-write is needed as the mixing of decNumber types cannot deal with the real_t temporary variable withinn mulComplexComplex(). The 159 series is needed for 39 digit precision in cuberoots and the cubic formula solver
  #if defined(OPTION_CUBIC_159) || defined(OPTION_SQUARE_159) || defined(OPTION_EIGEN_159)
    void mulComplexComplex159(const cplx159_t *factor1, const cplx159_t *factor2, cplx159_t *product, realContext_t *realContext);
    void mulComplexReal159(const cplx159_t *factor1, const real_t *factor2, cplx159_t *product, realContext_t *realContext);
  #endif // OPTION_CUBIC_159 || OPTION_SQUARE_159 || OPTION_EIGEN_159

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
