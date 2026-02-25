// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file store.h
 */
#if !defined(STORE_H)
  #define STORE_H

  bool_t       isRegInRange  (uint16_t regist);
  bool_t       regInRange    (uint16_t r);
  /**
   * Stores X in an other register.
   *
   * \param[in] registerNumber
   */
  void         fnStore       (uint16_t r);

  /**
   * Adds X to a register.
   *
   * \param[in] registerNumber
   */
  void         fnStoreAdd    (uint16_t r);

  /**
   * Subtracts X from a register.
   *
   * \param[in] registerNumber
   */
  void         fnStoreSub    (uint16_t r);

  /**
   * Multiplies a register by X.
   *
   * \param[in] registerNumber
   */
  void         fnStoreMult   (uint16_t r);

  /**
   * Divides a register by X.
   *
   * \param[in] registerNumber
   */
  void         fnStoreDiv    (uint16_t r);

  /**
   * Keeps in a register min(X, register).
   *
   * \param[in] registerNumber
   */
  void         fnStoreMin    (uint16_t r);

  /**
   * Keeps in a register max(X, register).
   *
   * \param[in] registerNumber
   */
  void         fnStoreMax    (uint16_t r);

  /**
   * Stores the configuration.
   *
   * \param[in] regist
   */
  void         fnStoreConfig (uint16_t r);

  /**
   * Stores the stack.
   *
   * \param[in] regist
   */
  void         fnStoreStack  (uint16_t r);

  /**
   * Stores X in the element I,J of a matrix.
   *
   * \param[in] regist
   */
  void         fnStoreElement(uint16_t unusedButMandatoryParameter);
  void         fnStoreElementPlus(uint16_t unusedButMandatoryParameter);

  /**
   * Stores Y in the element X of a TAM selected vector.
   *
   * \param[in] regist
   */
  void         fnStoreVElement(uint16_t unusedButMandatoryParameter);

  /**
   * Stores X and Y in the indexes I and J.
   *
   * \param[in] regist
   */
  void         fnStoreIJ     (uint16_t unusedButMandatoryParameter);

  void         fn2Sto(uint16_t regist);
  void         fn3Sto(uint16_t regist);

#endif // !STORE_H
