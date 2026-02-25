// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

/**
 * \file memory.h
 */

/****************************************************************************************************************
 *        Old hardware (DMCP)                               new hardware (DMCP5)
 *
 * CPU    STM32L476ZG                                       CPU    STM32U575
 *
 * FLASH  1024Kb from 0x08000000  code of                   FLASH  2048Kb from 0x08000000  code of
 *                 to 0x080FFFFF  functions                                 to 0x081FFFFF  functions
 *
 * SRAM2    32Kb from 0x10000000  global                    SRAM1   192Kb from 0x20000000  local variables
 *                 to 0x10007FFF  variables                                 to 0x2002FFFF  in functions
 *
 * SRAM1    96Kb from 0x20000000  local variables in        SRAM2    64Kb from 0x20030000  ?
 *                 to 0x20017FFF  functions and malloc                      to 0x2003FFFF
 *
 * QSPI   2048kB from 0x90000000                            SRAM3   512Kb from 0x20040000  malloc
 *                 to 0x901FFFFF                                            to 0x200BFFFF
 *
 *                                                          SRAM4    16Kb from 0x28000000  global
 *                                                                          to 0x28003FFF  variables
 *
 *                                                          BKPSRAM   2Kb from 0x38800000  backup? Not sure of
 *                                                                          to 0x388007FF  address or size !
 *
 *                                                          QSPI   2048Kb from 0x90000000
 *                                                                          to 0x901FFFFF
 ****************************************************************************************************************/

#if !defined(MEMORY_H)
  #define MEMORY_H

  // The 6 followoing functions are only there to know who allocates and frees memory
  void    *allocC47Blocks        (size_t sizeInBlocks);
  void    *reallocC47Blocks      (void *pcMemPtr, size_t oldSizeInBlocks, size_t newSizeInBlocks);
  void     reduceC47Blocks       (void *pcMemPtr, size_t oldSizeInBlocks, size_t newSizeInBlocks);
  void     freeC47Blocks         (void *pcMemPtr, size_t sizeInBlocks);

  void    *allocGmp              (size_t sizeInBytes);
  void    *reallocGmp            (void *pcMemPtr, size_t oldSizeInBytes, size_t newSizeInBytes);
  void     freeGmp               (void *pcMemPtr, size_t sizeInBytes);

  uint32_t getFreeRamMemory      (void);
  void     resizeProgramMemory   (uint16_t newSizeInBlocks);
  bool_t   isMemoryBlockAvailable(size_t sizeInBlocks);
  void     debugMemory           (const char *message);
  void     ramDump               (void);

  // The following macros are for avoid crash in case that the memory is full. The corresponding label `cleanup_***` is needed AFTER freeing the memory.
  #define checkedAllocate2(var, size, label) do { var = allocC47Blocks(size); if(!var) {displayCalcErrorMessage(ERROR_RAM_FULL, ERR_REGISTER_LINE, NIM_REGISTER_LINE); goto label; } } while(0)
  #define checkedAllocate(var, size) checkedAllocate2(var, size, cleanup_##var)
#endif // !MEMORY_H
