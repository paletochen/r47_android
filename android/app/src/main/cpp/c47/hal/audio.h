// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file hal/audio.h
 */
#if !defined(AUDIO_H)
  #define AUDIO_H

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
