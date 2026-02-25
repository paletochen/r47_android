// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file equation.h
 ***********************************************/
#if !defined(EQUATION_H)
  #define EQUATION_H

  #define EQUATION_AIM_BUFFER  0xffff

  #define EQUATION_NO_CURSOR   0xffff

  #define EQUATION_PARSER_MVAR 0
  #define EQUATION_PARSER_XEQ  1

  void   fnEqNew        (uint16_t unusedButMandatoryParameter);
  void   fnEqEdit       (uint16_t unusedButMandatoryParameter);
  void   fnEqCla        (void);
  void   fnEqDelete     (uint16_t unusedButMandatoryParameter);
  void   fnEqCursorLeft (uint16_t unusedButMandatoryParameter);
  void   fnEqCursorRight(uint16_t unusedButMandatoryParameter);
  void   fnEqCalc       (uint16_t unusedButMandatoryParameter);

  /**
   * Stores a formula. This function reallocates memory for the formula.
   *
   * \param[in]  equationId       Equation ID
   * \param[in]  equationString   Equation in null-terminated string
   */
  void   setEquation    (uint16_t equationId, const char *equationString);

  /**
   * Deletes a formula. This function frees memory for the formula and shifts equation ID.
   *
   * \param[in]  equationId       Equation ID
   */
  void   deleteEquation (uint16_t equationId);

  /**
   * Shows a formula at the softmenu.
   *
   * \param[in]  equationId      Equation ID
   * \param[in]  startAt         From which character the formula shall be shown, in number of glyphs
   * \param[in]  cursorAt        Position of cursor, in number of glyphs, alternatively `EQUATION_NO_CURSOR` to hide cursor
   * \param[in]  dryRun          Set `true` to make sure no output displayed (performs dry run if `true`)
   * \param[out] cursorShown     Unless `NULL` is specified, returns whether the cursor has been (or would be) shown
   * \param[out] rightEllipsis   Unless `NULL` is specified, returns whether the right ellipsis has been (or would be) shown
   */
  void   showEquation   (uint16_t equationId, uint16_t startAt, uint16_t cursorAt, bool_t dryRun, bool_t *cursorShown, bool_t *rightEllipsis);

  /**
   * Parses a formula.
   *
   * \param[in]  equationId   Equation ID
   * \param[in]  parseMode    `EQUATION_PARSER_MVAR` to make `MVAR` menu, `EQUATION_PARSER_XEQ` to evaluate
   * \param[out] buffer       Working buffer
   * \param[out] mvarBuffer   For `EQUATION_PARSER_MVAR`, resulting `MVAR` list. for `EQUATION_PARSER_XEQ`, operator buffer.
   */
  void   parseEquation  (uint16_t equationId, uint16_t parseMode, char *buffer, char *mvarBuffer);

  /**
   * Check the number of function parameters.
   *
   * \param[in]  item         Function item
   * \param[out] true/false   True for dyadic functions, false for monadic functions.
   */
  bool   isDyadicFunction (uint16_t item);

#endif // !EQUATION_H
