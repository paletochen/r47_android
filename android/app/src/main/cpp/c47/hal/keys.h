// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file hal/keys.h
 * Key buffer, sleep, delay, timers and the calc_state bits.
 * DMCP functions should follow specification at
 * https://technical.swissmicros.com/dmcp/doc/DMCP-ifc-html/
 */
#if !defined(KEYS_H)
  #define KEYS_H

#if defined(DMCP_BUILD)
  /**
   * The key keyBuffer_pop moves to the ring buffer the main loop dispatches from.
   */
  static inline int keyPopForMainLoop(void) {
    return key_pop();
  }

  /**
   * The calculator has no host events: every key code is a key.
   */
  static inline bool_t keyIsHostEvent(int key) {
    return false;
  }

  static inline void keyDispatchHostEvent(int key) {
  }

#else // !DMCP_BUILD
  // the calc_state bits the shared code sets and tests, at their DMCP positions
  #define STAT_RUNNING          0x00000002u
  #define STAT_SUSPENDED        0x00000004u
  #define STAT_KEYUP_WAIT       0x00000008u
  #define STAT_OFF              0x00000010u
  #define STAT_PGM_END          0x00000200u
  #define STAT_CLK_WKUP_ENABLE  0x00000400u
  #define STAT_CLK_WKUP_FLAG    0x00001000u
  #define STAT_POWER_CHANGE     0x00008000u

  extern uint32_t calc_state;
  #define ST(x)     ((x) & calc_state)
  #define SET_ST(x) (calc_state |= (x))
  #define CLR_ST(x) (calc_state &= ~(x))

  // mimic DMCP functions
  /**
   * Key buffer. A press is pushed as its key code, 1 to 37 for the keys and 38 to 43 for the softkeys, and a release as 0.
   * key_pop and key_tail return -1 when the buffer is empty, key_tail without taking the key.
   */
  int      key_empty           (void);
  int      key_push            (int k1);
  int      key_tail            (void);
  int      key_pop             (void);
  void     key_pop_all         (void);

  /**
   * Returns when no pushed key is down, or after tout ms when tout is above 0.
   */
  void     wait_for_key_release(int tout);

  /**
   * Returns after the next event: a key, an expired timer or the minute wake.
   */
  void     sys_sleep           (void);
  void     sys_delay           (uint32_t ms_delay);
  uint32_t sys_current_ms      (void);
  void     sys_timer_start     (int timer_ix, uint32_t ms_value);
  void     sys_timer_disable   (int timer_ix);

  /**
   * The simulator has no auto off, no battery and no real time clock to wait for: reset_auto_off does nothing, get_vbat returns a healthy
   * 3100 mV and rtc_wakeup_delay returns at once.
   */
  void     reset_auto_off      (void);
  int      get_vbat            (void);
  void     rtc_wakeup_delay    (void);

  /**
   * The simulator pushes the C47 key numbers themselves, so there is no physical layout to convert.
   */
  static inline int convertKeyCode(int key) {
    return key;
  }

  /**
   * A host event is an input the calculator keyboard does not have: a PC key, or a mouse click that selects the f or the g shift. It takes a key
   * code from 100 up in the key buffer. key_pop translates it into the calculator keys it maps to, as a PC key acts in CM_NORMAL, and
   * keyPopForMainLoop hands it to the main loop whole, which processes it with keyDispatchHostEvent.
   */
  #define KEY_HOST_EVENT_FIRST 100

  int      keyPopForMainLoop        (void);
  bool_t   keyIsHostEvent           (int key);
  void     keyDispatchHostEvent     (int key);

  /**
   * Pushes a press or a release of a calculator key.
   *
   * \param[in] data   the key string of btnPressed, "00" to "36", or of btnFnPressed, "1" to "6"
   * \param[in] press  true for a press, false for a release
   * \param[in] button mouse button: 1 plain, 2 with the f shift, 3 with the g shift
   */
  void     keyPushKey               (const char *data, bool_t press, uint8_t button);

  /**
   * Pushes a press and a release of a calculator key, as a click gives them.
   */
  void     keyPushClick             (const char *data, uint8_t button);

  /**
   * Pushes a PC keyboard event as a host event.
   */
  void     keyPushPcEvent           (const GdkEventKey *event, bool_t press);

  /**
   * Processes the keys in the buffer now, in the order they were pushed, as the main loop does. A script runs this after each key it pushes.
   */
  void     keyDispatchNow           (void);
#endif // DMCP_BUILD
#endif // !KEYS_H
