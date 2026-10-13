// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file hal/gui.h
 */
#if !defined(GUI_H)
  #define GUI_H

  #if defined(DMCP_BUILD) || (SIMULATOR_ON_SCREEN_KEYBOARD == 0)
    #define calcModeNormalGui()
    #define calcModeAimGui()
    #define calcModeTamGui()
  #else // !defined(DMCP_BUILD) && SIMULATOR_ON_SCREEN_KEYBOARD != 0
    void calcModeNormalGui (void);
    void calcModeAimGui    (void);
    void calcModeTamGui    (void);
  #endif // defined(DMCP_BUILD) || SIMULATOR_ON_SCREEN_KEYBOARD == 0
#endif // !GUI_H
