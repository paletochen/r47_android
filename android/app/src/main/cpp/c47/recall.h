// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file recall.h
 */
#if !defined(RECALL_H)
  #define RECALL_H

  /**
   * Recalls a register in X.
   *
   * \param[in] regist
   */
  void         fnRecall       (uint16_t r);
  void         fnRecallPlusSkip(uint16_t regist);

  /**
   * Recalls register L in X.
   *
   * \param[in] unusedButMandatoryParameter
   */
  void         fnLastX        (uint16_t unusedButMandatoryParameter);

  /**
   * Adds a register to X.
   *
   * \param[in] regist
   */
  void         fnRecallAdd    (uint16_t r);

  /**
   * Subtracts a register from X.
   *
   * \param[in] regist
   */
  void         fnRecallSub    (uint16_t r);

  /**
   * Multiplies X by a register.
   *
   * \param[in] regist
   */
  void         fnRecallMult   (uint16_t r);

  /**
   * Divides X by a register.
   *
   * \param[in] regist
   */
  void         fnRecallDiv    (uint16_t r);

  /**
   * Keeps in X min(X, register).
   *
   * \param[in] regist
   */
  void         fnRecallMin    (uint16_t r);

  /**
   * Keeps in X max(X, register).
   *
   * \param[in] regist
   */
  void         fnRecallMax    (uint16_t r);

  /**
   * Recalls a configuration.
   *
   * \param[in] regist
   */
  void         fnRecallConfig (uint16_t r);

  /**
   * Recalls a stack.
   *
   * \param[in] regist
   */
  void         fnRecallStack  (uint16_t r);

  /**
   * Recalls the matrix element I,J in X.
   *
   * \param[in] regist
   */
  void         fnRecallElement(uint16_t unusedButMandatoryParameter);
  void         fnRecallElementPlus(uint16_t unusedButMandatoryParameter);

  /**
   * Recalls the vector element X in TAM selected vector.
   *
   * \param[in] regist
   */
  void         fnRecallVElement(uint16_t regist);

  /**
   * Recalls the indexes I and J in X and Y.
   *
   * \param[in] regist
   */
  void         fnRecallIJ     (uint16_t unusedButMandatoryParameter);


  void         fn2Rcl(uint16_t regist);
  void         fn3Rcl(uint16_t regist);

#endif // !RECALL_H
