// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file hal/audio.h
 */
#if !defined(AUDIO_H)
  #define AUDIO_H

  /* Every command that makes a sound, with its items[] entry and what it does to the buzzer volume.
   *
   * System flag QUIET, item 488, silences the calculator completely. Nothing below makes a sound while it is set. Five places start the buzzer and every one tests the
   * flag first: audioTone, dm42_squeak and _Buzz here, the beep() macro in defines.h, and _keyClick in c47Extensions/keyboardTweak.c.
   *
   * VOL takes 0 to 11, the TAM maximum on item 2198. The DMCP SDK documents no range for get_beep_volume, so 11 comes from our own item table, not from DMCP.
   *
   * OUT_VOL_MAX in defines.h decides whether a sound may borrow the volume. Defined, the three marked "raises to 11" do so and put the user's setting back. Undefined,
   * none of them touch it and every sound plays at whatever the user set.
   *
   * Commands the user can run:
   *
   *   command   item  function        input                        volume
   *   -------   ----  --------------  ---------------------------  ------------------------------------------------------------------------------
   *   VOL       2198  fnSetVolume     TM_VALUE, 0 to 11            sets it. Both the reading and the requested value are clamped to 0 to 11
   *   VOL#      2199  fnGetVolume     NOPARAM, result in X         reads it. Makes no sound
   *   VOL^      2200  fnVolumeUp      NOPARAM                      one step up, then a tone so the new setting can be heard
   *   VOLv      2201  fnVolumeDown    NOPARAM                      one step down, then a tone so the new setting can be heard
   *   TONE      1624  fnTone          TM_VALUE, 0 to 9             plays at the user's setting, leaves it alone
   *   BUZZ      2202  fnBuzz          NOPARAM, Hz in Y, ms in X    plays at the user's setting, leaves it alone
   *   PLAY      2203  fnPlay          TM_REGISTER, 0 to 99         Nx2 plays at the user's setting. Nx3 takes a volume per note from column 2 and
   *                                                                puts the user's setting back, both at the end and when EXIT stops it
   *   SNAP      1405  fnSNAP          NOPARAM                      two marks around the capture. Raises to 11 and puts the setting back, unless
   *                                                                the setting is already at its lowest, which is left alone
   *   Pa        2682  fnP_Alpha       TM_REGISTER, 0 to 99         the printing refusal below
   *   Pr        1714  fnP_Regs        TM_REGISTER, 0 to 99         the printing refusal below
   *   Px        1676  fnP_All_Regs    NOPARAM                      the printing refusal below
   *   Pxy       1707  fnP_All_Regs    NOPARAM                      the printing refusal below
   *   PREGS     1715  fnP_All_Regs    NOPARAM                      the printing refusal below
   *   PSTK      1716  fnP_All_Regs    NOPARAM                      the printing refusal below
   *   PALLr     1799  fnP_All_Regs    NOPARAM                      the printing refusal below
   *
   * The printing refusal: with the printer flag clear these write to file, and file output is only allowed from certain calculator modes. Asked from any other mode they
   * refuse and say so with three beeps, low, high, low. Each beep raises the volume to 11 and puts the user's setting back. Pa needs CM_AIM, Pr needs CM_NORMAL, and the
   * five fnP_All_Regs names need CM_NORMAL or CM_NO_UNDO.
   *
   * Sound with no command behind it, all of it switch gated and none of it compiled today, since DM42_KEYCLICK, DM42_POWERMARKS, DM42_POWERMARK_KEYPRESS and
   * CLICK_REFRESHSCR in defines.h are all undefined:
   *
   *   source        driven by                                      volume
   *   ------------  ---------------------------------------------  ----------------------------------------------------------------------------
   *   key click     keyClick() from six sites in c47.c, on key      raises to 11 for the click and puts the user's setting back
   *                 press and release edges
   *   power marks   powerMarkerMsF() from sixteen sites in c47.c,   the same. Marks the power state on a scope, so length and pitch carry the
   *                 timer.c and screen.c, marking sleep, wake,      meaning: 1, 3, 5, 7, 10 and 15 ms at 1, 4, 8, 10 and 15 kHz, plus one whose
   *                 refresh and key press                           length is the sleep time itself
   *
   * Sound helpers with no command of their own: audioTone plays one 250 ms note; dm42_squeak one fixed 125 ms note; _Buzz one note of a given length, capped at 2 s.
   * All three play at the user's setting.
   */

  /**
   * Plays a tone.
   * Each hardware platform that supports playing audio should implement
   * this method to play a short tone of a given frequency. This is the
   * only supported audio playback. Configuration for a "silent mode" is
   * not covered by this function and should be checked by the caller.
   *
   * \param[in] frequency the frequency of the note to play in mHz
   */
  void audioTone(uint32_t frequency);

  /**
   * Set Buzzer volume on the calculator.
   * Only relevant for the DMCP version, not used for the simulator
   * Input : volume level from 0 to 11
   */
  void fnSetVolume(uint16_t volume);

  /**
   * Get Buzzer volume on the calculator.
   * Only relevant for the DMCP version, not used for the simulator
   * Output : volume level from 0 to 11
   */
  void fnGetVolume(uint16_t volume);

  /**
   * Increase Buzzer volume on the calculator.
   * Only relevant for the DMCP version, not used for the simulator
   */
  void fnVolumeUp(uint16_t unusedButMandatoryParameter);

  /**
   * Decrease Buzzer volume on the calculator.
   * Only relevant for the DMCP version, not used for the simulator
   */
  void fnVolumeDown(uint16_t unusedButMandatoryParameter);

  /**
   * DM42 squeak sound
   * Only relevant for the DMCP version, not used for the simulator
   */
  void squeak();

  /**
   * Play a sound on the buzzer whose frequency is in Y and duration in X.
   * Only relevant for the DMCP version, not used for the simulator
   */
  void _Buzz(uint32_t frequency, uint32_t ms_delay);
  void fnBuzz(uint16_t unusedButMandatoryParameter);

  /**
   * Play a melody on the buzzer whose notes frequency and durations are in a Nx2 matrix.
   * Only relevant for the DMCP version, not used for the simulator
   */
  void fnPlay(uint16_t regist);

  uint16_t getBeepVolume(void);

#endif // !AUDIO_H
