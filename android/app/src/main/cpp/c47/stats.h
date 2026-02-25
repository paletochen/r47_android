// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors


/**
 * \file stats.h
 * Statistical functions.
 */
#if !defined(STATS_H)
  #define STATS_H

  #define SIGMA_PLUS  1
  #define SIGMA_MINUS 2
  #define SIGMA_NONE  0

  /**
   * Adds a value to the statistic registers.
   *
   * \param[in] unusedButMandatoryParameter
   */
  void   fnSigmaAddRem         (uint16_t plusMinus);

  void   fnStatSum             (uint16_t sum);

  /**
   * SUM ==> regX, regY.
   * regX = SUM x, regY = SUM y
   *
   * \param[in] unusedButMandatoryParameter
   */
  void   fnSumXY               (uint16_t unusedButMandatoryParameter);

  /**
   * Xmin ==> regX, regY.
   * regX = min x, regY = min y
   *
   * \param[in] unusedButMandatoryParameter
   */
  void   fnXmin                (uint16_t unusedButMandatoryParameter);

  /**
   * Xmax ==> regX, regY.
   * regX = max x, regY = max y
   *
   * \param[in] unusedButMandatoryParameter
   */
  void   fnXmax                (uint16_t unusedButMandatoryParameter);

  /**
   * Xrange ==> regX, regY.
   * regX = max x - min x, regY = max y - min y
   *
   * \param[in] unusedButMandatoryParameter
   */
  void   fnRangeXY             (uint16_t unusedButMandatoryParameter);

  void   fnClSigma             (uint16_t unusedButMandatoryParameter);

  /**
   * Verifies that the statistical registers are allocated and that there are enough data.
   * An appropriate error message is displayed if either condition fails.
   *
   * \param[in] unusedButMandatoryParameter
   * \return bool_t
   */
  bool_t isStatsMatrixN(uint16_t *rows, calcRegister_t regStats);
  bool_t isStatsMatrix(uint16_t *rows, char *mx);

  bool_t checkMinimumDataPoints(const real_t *n);
  void   initStatisticalSums   (void);
  void   clearStatisticalSums  (void);
  void   reLoadStatisticalSums (void);
  void   calcSigma             (uint16_t maxOffset);

  void   fnSetLoBin              (uint16_t unusedButMandatoryParameter);
  void   fnSetHiBin              (uint16_t unusedButMandatoryParameter);
  void   fnSetNBins              (uint16_t unusedButMandatoryParameter);
  void   fnConvertStatsToHisto   (uint16_t statsVariableToHistogram);
  void   setStatisticalSumsUpdate(bool_t para);

#endif // STATS_H
