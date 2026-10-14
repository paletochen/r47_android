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


void leavePem        (void);
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


  #if defined(PC_BUILD)
/**
   * Simulate a function key click.
   *
   * \param notUsed The button to pass to btnFnPressed and btnFnReleased
   * \param data String containing the key ID
   */
  void btnFnClicked  (GtkWidget *notUsed, gpointer data);
  void btnFnClickedP (GtkWidget *notUsed, gpointer data);                     //JM
  void btnFnClickedR (GtkWidget *notUsed, gpointer data);                     //JM
  /**
   * A calc function key was pressed.
   *
   * \param notUsed
   * \param data pointer to a string containing the key number pressed: 00=1/x, ..., 36=EXIT
   */
  void btnFnPressed  (GtkWidget *notUsed, GdkEvent *event, gpointer data);

  /**
   * A calc function key was released.
   *
   * \param notUsed
   * \param data pointer to a string containing the key number pressed: 00=1/x, ..., 36=EXIT
   */
  void btnFnReleased (GtkWidget *notUsed, GdkEvent *event, gpointer data);

  /**
   * Simulate a button click.
   *
   * \param notUsed The button to pass to btnPressed and btnReleased
   * \param data String containing the key ID
   */
  void btnClicked    (GtkWidget *notUsed, gpointer data);
  void btnClickedP   (GtkWidget *notUsed, gpointer data);                     //JM
  void btnClickedR   (GtkWidget *notUsed, gpointer data);                     //JM
  /**
   * A calc button was pressed.
   *
   * \param notUsed
   * \param data pointer to a string containing the key number pressed: 00=1/x, ..., 36=EXIT
   */
  void btnPressed    (GtkWidget *notUsed, GdkEvent *event, gpointer data);

  /**
   * A calc button was released.
   *
   * \param notUsed
   * \param data pointer to a string containing the key number pressed: 00=1/x, ..., 36=EXIT
   */
  void frmCalcMouseButtonPressed(GtkWidget *notUsed, GdkEvent *event, gpointer data);

  /**
   * A calc button was pressed.
   *
   * \param notUsed
   * \param data pointer to a string containing the key number pressed: 00=1/x, ..., 36=EXIT
   */
  void frmCalcMouseButtonReleased(GtkWidget *notUsed, GdkEvent *event, gpointer data);

  /**
   * A calc button was released.
   *
   * \param notUsed
   * \param data pointer to a string containing the key number pressed: 00=1/x, ..., 36=EXIT
   */
  void btnReleased   (GtkWidget *notUsed, GdkEvent *event, gpointer data);
#endif // PC_BUILD

void execAutoRepeat(uint16_t key);

  #if defined(DMCP_BUILD)
  void btnFnClicked (void *w, void *data);
  void btnFnPressed (void *data);
  void btnFnReleased(void *data);
  void btnClicked   (void *w, void *data);
  void btnPressed   (void *data);
  void btnReleased  (void *data);
  #endif // DMCP_BUILD

#endif // !KEYBOARD_H
