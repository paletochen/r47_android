// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file bufferize.h
 */
#if !defined(BUFFERIZE_H)
#define BUFFERIZE_H

  extern bool_t delayCloseNim;

  void     fnAim                    (uint16_t unusedButMandatoryParameter);
  void     insertAlphaCharacter     (uint16_t item, int16_t *currentCursor);
  void     deleteAlphaCharacter     (int16_t *currentCursor);
  void     fnAlphaCursorLeft        (uint16_t unusedButMandatoryParameter);
  void     fnAlphaCursorRight       (uint16_t unusedButMandatoryParameter);
  void     fnAlphaCursorHome        (uint16_t unusedButMandatoryParameter);
  void     fnAlphaCursorEnd         (uint16_t unusedButMandatoryParameter);
  void     resetAlphaSelectionBuffer(void);
  uint16_t convertItemToSubOrSup    (uint16_t item, int16_t subOrSup);
  void     light_ASB_icon(void);                        //JM
  void     kill_ASB_icon(void);                         //JM

  /**
   * Adds an item to the alpha buffer.
   *
   * \param[in] item item to add to the buffer
   */
  void    addItemToBuffer          (uint16_t item);

  void    addItemToNimBuffer       (int16_t item);
  void    closeNimWithFraction     (real34_t *dest);
  void    nimRealValue             (real34_t *dest);
  bool_t  nimPiAfterI              (void);
  bool_t  nimShortInteger          (char *buffer, calcRegister_t regist);
  bool_t  nimFractionEntered       (void);
  void    nimLiteralValue          (real34_t *re, real34_t *im);
  void    nimComplexValue          (real34_t *dest_r, real34_t *dest_i);
  void    nimStartNumber           (int16_t item);
  void    nimCompleteNumber        (void);
  void    nimHint                  (char *hint);
  void    closeNimWithComplex      (real34_t *dest_r, real34_t *dest_i);
  void    closeNim                 (void);
  bool_t  nimPolarEntry            (void);
  bool_t  nimComplexPart           (void);
  #if defined(OPTION_DRG_CFG)
    bool_t  nimRealOrFractionPart    (void);
    bool_t  nimDrgChoosesUnit        (void);
    const char *nimDmsMark           (void);
  #endif // OPTION_DRG_CFG
  void    closeAim                 (void);
  void    nimBufferToDisplayBuffer (const char *buffer, char *displayBuffer);
  bool_t  checkShortIntegerDigits  (char *buffer);
#endif // !BUFFERIZE_H
