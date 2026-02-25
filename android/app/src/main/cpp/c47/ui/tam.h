// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file ui/tam.h
 */
#if !defined(TAM_H)
  #define TAM_H

  /**
   * Enters TAM mode.
   * This initialises TAM entry for the given command and sets the `tamBuffer`
   * to the appropriate text. In GUI mode the keyboard is set-up. Once this
   * function has been called TAM mode can be left by input processed by
   * ::tamProcessInput or by calling ::leaveTamModeIfEnabled. If TAM is left the command
   * that triggered TAM is implicitly cancelled (no further action is needed).
   * For the command to be executed the input must be processed by
   * ::tamProcessInput.
   *
   * This function should be called instead of the command that requires TAM
   * input. The command that requires TAM should be passed as a parameter.
   *
   * \param[in] func the `indexOfItems` index for the command that
   *                 requires TAM mode
   */
  void tamEnterMode    (int16_t func);

  /**
   * Leaves TAM mode. TAM mode is closed and the pending operation is cancelled.
   */
  void leaveTamModeIfEnabled(void);

  /**
   * Processes input for the TAM buffer.
   * Almost all input is handled by this function. The exceptions are:
   * - alpha input when in 'alpha' mode (tamState_t::alpha), where input of alpha
   *   characters must be put into the AIM buffer before calling this function with
   *   the input item
   * - `EXIT` and other external functions where TAM should be closed externally
   *   using ::leaveTamModeIfEnabled
   *
   * After calling this function the ::tamBuffer will be updated and it should be
   * redrawn to the relevant part of the display.
   *
   * \param[in] item the ITM value to process
   */
  void tamProcessInput (uint16_t item);


  /**
   * Returns actual function for current TAM.
   *
   * \return operation code
   */
  int16_t tamOperation (void);
#endif // !TAM_H
