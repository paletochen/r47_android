// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file ui/matrixEditor.c
 ***********************************************/

#if !defined(MATRIXEDITOR_H)
  #define MATRIXEDITOR_H

  #define MATRIX_LINE_WIDTH            380
  //#define MATRIX_LINE_WIDTH_LARGE      120
  //#define MATRIX_LINE_WIDTH_SMALL      93
  //#define MATRIX_CHAR_LEN              30
  #define MATRIX_MAX_ROWS              5
  #define MATRIX_MAX_COLUMNS           11

  /**
   * Opens the Matrix Editor.
   *
   * \param[in] regist
   */
  void       fnEditMatrix                   (uint16_t regist);

  /**
   * Recalls old element in the Matrix Editor.
   *
   * \param[in] unusedParamButMandatory
   */
  void       fnOldMatrix                    (uint16_t unusedParamButMandatory);

  /**
   * Go to an element in the Matrix Editor.
   *
   * \param[in] unusedParamButMandatory
   */
  void       fnGoToElement                  (uint16_t unusedParamButMandatory);

  void       fnGoToRow                      (uint16_t row);
  void       fnGoToColumn                   (uint16_t col);

  /**
   * Set grow mode.
   *
   * \param[in] growFlag
   */
  void       fnSetGrowMode                  (uint16_t growFlag);

  /**
   * Increment or decrement of register I as row pointer.
   *
   * \param[in] mode
   */
  void       fnIncDecI                      (uint16_t mode);

  /**
   * Increment or decrement of register J as column pointer.
   *
   * \param[in] mode
   */
  void       fnIncDecJ                      (uint16_t mode);

  /**
   * Insert a row.
   *
   * \param[in] unusedParamButMandatory
   */
  void       fnInsRow                       (uint16_t unusedParamButMandatory);
  void       fnInsCol                       (uint16_t unusedParamButMandatory);
  void       fnAddRow                       (uint16_t unusedParamButMandatory);
  void       fnAddCol                       (uint16_t unusedParamButMandatory);

  /**
   * Delete a row.
   *
   * \param[in] unusedParamButMandatory
   */
  void       fnDelRow                       (uint16_t unusedParamButMandatory);
  void       fnDelCol                       (uint16_t unusedParamButMandatory);

  int16_t  getIRegisterAsInt              (bool_t asArrayPointer);
  int16_t  getJRegisterAsInt              (bool_t asArrayPointer);
  void     setIRegisterAsInt              (bool_t asArrayPointer, int16_t toStore);
  void     setJRegisterAsInt              (bool_t asArrayPointer, int16_t toStore);

  bool_t   wrapIJ                         (uint16_t rows, uint16_t cols);

#endif // !MATRIXEDITOR_H
