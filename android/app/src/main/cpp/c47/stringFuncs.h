// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file stringFuncs.h
 */
#if !defined(STRINGFUNCS_H)
  #define STRINGFUNCS_H

  void fnAlphaLeng     (uint16_t regist);
  void fnAlphaToX      (uint16_t regist);
  void fnAlphaRR       (uint16_t regist);
  void fnAlphaRL       (uint16_t regist);
  void fnAlphaSR       (uint16_t regist);
  void fnAlphaSL       (uint16_t regist);
  void fnAlphaPos      (uint16_t regist);
  void fnXToAlphaOld   (uint16_t unusedButMandatoryParameter);
  void fnXToAlpha      (uint16_t regist);
  void fnClearAlpha    (uint16_t regist);
  void fnAlphaIP       (uint16_t regist);
  void fn42AlphaRotate (uint16_t unusedButMandatoryParameter);
  void fn42AlphaShift  (uint16_t unusedButMandatoryParameter);
  void fn42Alpha       (uint16_t unusedButMandatoryParameter);
  void fn42Append      (uint16_t unusedButMandatoryParameter);
  void fn42Aip         (uint16_t unusedButMandatoryParameter);
  void fn42Aleng       (uint16_t unusedButMandatoryParameter);
  void fn42Atox        (uint16_t unusedButMandatoryParameter);
  void fn42Xtoa        (uint16_t unusedButMandatoryParameter);
  void fn42Aview       (uint16_t unusedButMandatoryParameter);
  void fn42Cla         (uint16_t unusedButMandatoryParameter);
  void fn42Keyg        (uint16_t unusedButMandatoryParameter);
  void fn42Keyx        (uint16_t unusedButMandatoryParameter);
  void fn42Posa        (uint16_t unusedButMandatoryParameter);
  void fn42Pra         (uint16_t unusedButMandatoryParameter);

  void trimLeadingSpace(char     *stringToTrim);
  void truncateAlphaRegisterTo44Char();
#endif // !STRINGFUNCS_H
