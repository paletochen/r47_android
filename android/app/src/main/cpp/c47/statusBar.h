// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
   * \file statusBar.h
   * Status bar management.
   */

#if !defined(STATUSBAR_H)
  #define STATUSBAR_H

   #if defined (PC_BUILD)
    void mockupSB(void);
  #endif //PC_BUILD

  #if !defined(TESTSUITE_BUILD)
  /**
   * Refreshes the status bar.
   */
  void forceSBupdate          (void);
  void refreshStatusBar       (void);

  /**
   * Displays date and time in the status bar.
   */
  bool_t timeChanged          (void);
  bool_t showDateTime         (void);

  /**
   * Displays the complex result mode C or R in the status bar.
   */
//  void showRealComplexResult  (void);

  /**
   * Displays the complex mode rectangular or polar in the status bar.
   */
//  void showComplexMode        (void);

  /**
   * Displays the angular mode in the status bar.
   */
//  void showAngularMode        (void);

  /**
   * Displays the faction mode in the status bar.
   */
  void showFracMode           (void);

  /**
   * Displays the integer mode icon in the status bar.
   */
//  void showIntegerMode        (void);

  /**
   * Displays the matrix mode icon in the status bar.
   */
//  void showMatrixMode         (void);

  /**
   * Displays the TVM mode icon in the status bar.
   */
//  void showTvmMode            (void);

  /**
   * Displays the overflow flag and the carry flag.
   */
//  void showOverflowCarry      (void);

  /**
   * Shows or hides the alpha mode.
   */
  void showHideAlphaMode      (void);

  /**
   * Shows or hides the hourglass icon in the status bar.
   */
  void showHideHourGlass      (void);

  /**
   * Shows ASB icon in the status bar.
   */
//  void light_ASB_icon         (void);

  /**
   * Hides ASB icon in the status bar.
   */
//  void kill_ASB_icon          (void);

  /**
   * Shows or hides the watch icon in the status bar.
   */
//  void showHideWatch          (void);

  /**
   * Shows or hides the serial I/O icon in the status bar.
   */
//  void showHideSerialIO       (void);

  /**
   * Shows or hides the printer icon in the status bar.
   */
//  void showHidePrinter        (void);

  /**
   * Shows or hides the user mode icon in the status bar.
   */
//  void showHideUserMode       (void);

  /**
   * Shows or hides the USB or low battery icon in the status bar.
   */
  void showHideUsbLowBattery  (void);

  /**
   * Shows or hides the USB icon in the status bar.
   */
  void showHideStackLift      (void);

//  void showHideASB            (void);       //JM

  #endif // !TESTSUITE_BUILD
#endif // !STATUSBAR_H
