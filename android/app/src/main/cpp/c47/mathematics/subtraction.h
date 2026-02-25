// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file subtraction.h
 ***********************************************/
#if !defined(SUBTRACTION_H)
  #define SUBTRACTION_H

  void fnSubtract (uint16_t unusedButMandatoryParameter);
  void subComplex(const real_t *aReal, const real_t *aImag, const real_t *bReal, const real_t *bImag, real_t *resReal, real_t *resImag, realContext_t *realContext);

  #if (EXTRA_INFO_ON_CALC_ERROR == 1)
    void subError   (void);
  #else // (EXTRA_INFO_ON_CALC_ERROR != 1)
    #define subError typeError
  #endif // (EXTRA_INFO_ON_CALC_ERROR == 1)

  //      RegYRegX
  void subLonILonI(void);
  void subLonIRema(void);
  void subLonICxma(void);
  void subLonITime(void);
  void subLonIShoI(void);
  void subLonIReal(void);
  void subLonICplx(void);

  //      RegYRegX
  void subTimeLonI(void);
  void subTimeTime(void);
  void subTimeReal(void);

  //      RegYRegX
  void subDateLonI(void);
  void subDateDate(void);
  void subDateReal(void);

  //      RegYRegX
  void subRemaLonI(void);
  void subRemaRema(void);
  void subRemaCxma(void);
  void subRemaShoI(void);
  void subRemaReal(void);
  void subRemaCplx(void);

  //      RegYRegX
  void subCxmaLonI(void);
  void subCxmaRema(void);
  void subCxmaCxma(void);
  void subCxmaShoI(void);
  void subCxmaReal(void);
  void subCxmaCplx(void);

  //      RegYRegX
  void subShoILonI(void);
  void subShoIRema(void);
  void subShoICxma(void);
  void subShoIShoI(void);
  void subShoIReal(void);
  void subShoICplx(void);

  //      RegYRegX
  void subRealLonI(void);
  void subRealRema(void);
  void subRealCxma(void);
  void subRealTime(void);
  void subRealShoI(void);
  void subRealReal(void);
  void subRealCplx(void);

  //      RegYRegX
  void subCplxLonI(void);
  void subCplxRema(void);
  void subCplxCxma(void);
  void subCplxShoI(void);
  void subCplxReal(void);
  void subCplxCplx(void);
#endif // !SUBTRACTION_H
