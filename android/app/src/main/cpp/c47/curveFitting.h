// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file curveFitting.h
 ***********************************************/
#if !defined(CURVEFITTING_H)
#define CURVEFITTING_H

  void      fnCurveFittingReset        (uint16_t control);
  void      fnCurveFitting_T           (uint16_t curveFitting);
  void      fnCurveFitting             (uint16_t curveFitting);
  void      processCurvefitSelection   (uint16_t selection, real_t *RR_, real_t *SMI_, real_t *aa0, real_t *aa1, real_t *aa2);
  void      processCurvefitSelectionAll(uint16_t selection, real_t *RR_, real_t *MX, real_t *MX2, real_t *SX2, real_t *SY2, real_t *SMI_, real_t *aa0, real_t *aa1, real_t *aa2);
  uint16_t  lrCountOnes                (uint16_t curveFitting);
  uint16_t  minLRDataPoints            (uint16_t selection);
  void      yIsFnx                     (uint8_t  USEFLOAT, uint16_t selection, double x, double *y, double a0, double a1, double a2, real_t *XX, real_t *YY, real_t *RR, real_t *SMI, real_t *aa0, real_t *aa1, real_t *aa2);
  void      fnYIsFnx                   (uint16_t unusedButMandatoryParameter);
  void      fnXIsFny                   (uint16_t unusedButMandatoryParameter);
  void      fnCurveFittingLR           (uint16_t unusedButMandatoryParameter);

  /* The argument is a bit mask where parameter n is pushed if bit n is set */
  void      fnProcessLR                (uint16_t resultType);
#endif // !CURVEFITTING_H
