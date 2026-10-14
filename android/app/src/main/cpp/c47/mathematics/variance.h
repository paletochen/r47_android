// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file mean.h
 ***********************************************/
#if !defined(VARIANCE_H)
  #define VARIANCE_H

  void fnWeightedSampleStdDev     (uint16_t unusedButMandatoryParameter);
  void fnWeightedPopulationStdDev (uint16_t unusedButMandatoryParameter);
  void fnWeightedStandardError    (uint16_t unusedButMandatoryParameter);
  void fnSampleStdDev             (uint16_t unusedButMandatoryParameter);
  void fnPopulationStdDev         (uint16_t unusedButMandatoryParameter);
  void fnStandardError            (uint16_t unusedButMandatoryParameter);
  void fnGeometricSampleStdDev    (uint16_t unusedButMandatoryParameter);
  void fnGeometricPopulationStdDev(uint16_t unusedButMandatoryParameter);
  void fnGeometricStandardError   (uint16_t unusedButMandatoryParameter);
  void fnPopulationCovariance     (uint16_t unusedButMandatoryParameter);
  void fnSampleCovariance         (uint16_t unusedButMandatoryParameter);
  void fnCoefficientDetermination (uint16_t unusedButMandatoryParameter);
  void fnMinExpStdDev             (uint16_t unusedButMandatoryParameter);
  void fnStatSMI                  (real_t *SMI);
  void fnStatR                    (real_t *RR, real_t *SXY, real_t *SX, real_t *SY);
  void fnStatSXY                  (real_t *SXY);
  void fnStatSX_SY                (real_t *SX, real_t *SY);
  bool_t processCurvefitSA        (real_t *SA0, real_t *SA1);
  void fnStatSa                   (uint16_t unusedButMandatoryParameter);
#endif // !VARIANCE_H
