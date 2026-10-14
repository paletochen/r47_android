// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/********************************************//**
 * \file matrixEditor.h
 ***********************************************/

#if !defined(MATRIXEDITOR_H)
  #define MATRIXEDITOR_H

  #define MATRIX_LINE_WIDTH            380
  //#define MATRIX_LINE_WIDTH_LARGE      120
  //#define MATRIX_LINE_WIDTH_SMALL      93
  //#define MATRIX_CHAR_LEN              30
  #define MATRIX_MAX_ROWS              9    // measure cap outside SHOW
  #if defined(OPTION_MX_SHOW)
    #define MATRIX_MAX_ROWS_ON_SHOW    11   // sizes the row scratch arrays
  #else // !OPTION_MX_SHOW
    #define MATRIX_MAX_ROWS_ON_SHOW    9    // sizes the row scratch arrays
  #endif // OPTION_MX_SHOW
  #define MATRIX_MAX_ROWS_ON_VIEW      7    // VIEW keeps the stack, so it stops at the bottom of the register lines (above menu)
  #define MATRIX_MAX_ROWS_ON_STACK     5    // matrix on stack grows from the X line up
  #define MATRIX_MAX_COLUMNS           14   // max width: no radix on the elements: single digit columns fit 14

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

  /**
   * Which matrix was indexed, and the shadow row/column under it. Contents are opaque: saveMatrixIndexState() fills it and restoreMatrixIndexState() puts it back.
   */
  typedef struct {
    uint16_t matrixIndex;
    int16_t  savedI;
    int16_t  savedJ;
  } matrixIndexState_t;

  /**
   * Opens a scratch index for a function that walks a matrix itself.
   *
   * Remembers which matrix was indexed and the shadow row/column under it, and diverts I and J to that shadow so the walk cannot touch the user's registers.
   * They keep their value and their type, whatever those are. A Matrix Editor open underneath keeps its cursor.
   *
   * Pair with restoreMatrixIndexState() and route EVERY exit through it: while the shadow is open, reads of I and J see the walking index rather than the user's.
   * Call this only once the operation cannot fail, i.e. after the argument checks, so an early return cannot leave the shadow open.
   *
   * \param[out] state
   */
  void     saveMatrixIndexState           (matrixIndexState_t *state);

  /**
   * Closes the scratch index opened by saveMatrixIndexState().
   *
   * \param[in] state
   */
  void     restoreMatrixIndexState        (const matrixIndexState_t *state);

  bool_t   wrapIJ                         (uint16_t rows, uint16_t cols);

//  void displayVectorAngle(const real34Matrix_t *matrix, int j, int rows, int cols, uint8_t *toBeAngle);
//  void displayVectorElement(const real34Matrix_t *matrix, int j, int ii, int rows, int cols, real34_t *element, uint8_t *toBeAngle);


#endif // !MATRIXEDITOR_H
