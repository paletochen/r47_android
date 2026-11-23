// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file keyboard.h
 */
#if !defined(KEYBOARD_H)
#define KEYBOARD_H

int16_t determineFunctionKeyItem_C47(const char *data, bool_t shiftF, bool_t shiftG);
void    setLastKeyCode              (int key);
void    processAimInput             (int16_t item);


bool_t leavePem      (void);
void processKeyAction(int16_t item);

/**
 * Processing ENTER key.
 *
 * \param[in] unusedButMandatoryParameter
 */
void fnKeyEnter      (uint16_t unusedButMandatoryParameter);

/**
 * Processing EXIT key.
 *
 * \param[in] unusedButMandatoryParameter
 */
void fnKeyExit       (uint16_t unusedButMandatoryParameter);

/**
 * Processing CC key.
 *
 * \param[in] unusedButMandatoryParameter
 */
void fnKeyCC         (uint16_t unusedButMandatoryParameter);

/**
 * Processing BACKSPACE key.
 *
 * \param[in] unusedButMandatoryParameter
 */
void fnKeyBackspace  (uint16_t unusedButMandatoryParameter);

/**
 * Processing UP key.
 *
 * \param[in] unusedButMandatoryParameter
 */
void fnKeyUp         (uint16_t unusedButMandatoryParameter);

/**
 * Processing DOWN key.
 *
 * \param[in] unusedButMandatoryParameter
 */
void fnKeyDown       (uint16_t unusedButMandatoryParameter);

/**
 * Processing .d key.
 *
 * \param[in] unusedButMandatoryParameter
 */
void fnKeyDotD       (uint16_t unusedButMandatoryParameter);

#define ST_0_INIT     0     //STATE 0 INIT             //JM vv FN-DOUBLE
#define ST_1_PRESS1   1     //STATE 1 FIRST PRESS
#define ST_2_REL1     2     //STATE 2 FIRST RELEASE
#define ST_3_PRESS2   3     //STATE 3 SECOND PRESS     //Double click determination 2 to 3 < 75 ms.
#define ST_4_REL2     4     //STATE 4 SECOND RELEASE   //JM ^^


  /**
   * Simulates a function key click.
   *
   * \param unused
   * \param data String containing the key ID, "1" to "6"
   */
  void btnFnClicked (void *unused, void *data);

  /**
   * A calc function key was pressed.
   *
   * \param data pointer to a string containing the softkey number pressed: "1" to "6"
   */
  void btnFnPressed (void *data);

  /**
   * A calc function key was released.
   *
   * \param data pointer to a string containing the softkey number released: "1" to "6"
   */
  void btnFnReleased(void *data);

  /**
   * Simulates a button click.
   *
   * \param unused
   * \param data String containing the key ID
   */
  void btnClicked   (void *unused, void *data);

  /**
   * A calc button was pressed.
   *
   * \param data pointer to a string containing the key number pressed: 00=1/x, ..., 36=EXIT
   */
  void btnPressed   (void *data);

  /**
   * A calc button was released.
   *
   * \param data pointer to a string containing the key number released: 00=1/x, ..., 36=EXIT
   */
  void btnReleased  (void *data);

void execAutoRepeat(uint16_t key);

#endif // !KEYBOARD_H
