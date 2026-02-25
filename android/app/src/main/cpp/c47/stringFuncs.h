// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file stringFuncs.h
 */
#if !defined(STRINGFUNCS_H)
  #define STRINGFUNCS_H

  void fnAlphaLeng(uint16_t regist);
  void fnAlphaToX (uint16_t regist);
  void fnAlphaRR  (uint16_t regist);
  void fnAlphaRL  (uint16_t regist);
  void fnAlphaSR  (uint16_t regist);
  void fnAlphaSL  (uint16_t regist);
  void fnAlphaPos (uint16_t regist);
  void fnXToAlpha (uint16_t unusedButMandatoryParameter);
#endif // !STRINGFUNCS_H
