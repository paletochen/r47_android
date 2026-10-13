// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file hal/print_ir.h
 * Infrared interface related functions.
 */
#if !defined(PRINT_IR_H)
  #define PRINT_IR_H


  /**
   * Get print line delay
   * Output : delay in ticks
   */
   uint32_t getLineDelay();

  /**
   * Set print line delay
   * input : delay in ticks
   */
   void setLineDelay(uint16_t delay);

  /**
   * Send a byte over the IR interface.
   * Input : byte to send
   */
  void     sendByteIR( uint8_t byte );

#endif // !PRINT_IR_H
