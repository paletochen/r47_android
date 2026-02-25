// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file calcMode.h
 */
#if !defined(CALCMODE_H)
  #define CALCMODE_H

  void btn_Clicked_Gen             (bool_t shF, bool_t shG, char *st);
  void fnOff                       (uint16_t unsuedParamButMandatory);

  /**
   * Sets the calc mode to normal.
   */
  void calcModeNormal              (void);

  /**
   * Sets the calc mode to alpha input mode.
   *
   * \param[in] unusedButMandatoryParameter
   */
  void calcModeAim                 (uint16_t unusedButMandatoryParameter);

  /**
   * Sets the calc mode to number input mode.
   *
   * \param[in] unusedButMandatoryParameter
   */
  void calcModeNim                 (uint16_t unusedButMandatoryParameter);

  /**
   * Sets the calc mode to alpha selection menu if needed.
   */
  void enterAsmModeIfMenuIsACatalog(int16_t id);

  /**
   * Leaves the alpha selection mode.
   */
  void leaveAsmMode                (void);
#endif // !CALCMODE_H
