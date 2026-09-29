// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The C47 Authors

/**
 * \file selftest.c
 * A program that calls the file system entry points the way the firmware does.
 *
 * It runs inside the emulated machine and reaches hostfs.py through the library table, so what it exercises is the whole route: the svc trap, the argument registers, the
 * FIL in emulated memory and the two fields the firmware takes from it with the f_size and f_tell macros of ff_ifc.h.
 *
 * Every entry offset arrives as a define from stubs.py, which takes them from the parsed SDK header, so nothing here repeats a number the header states.
 *
 * The result is a bit per check, and zero means every check succeeded.
 */

#define FA_READ          0x01
#define FA_WRITE         0x02
#define FA_CREATE_ALWAYS 0x08
#define FR_OK 0

#define PROBE_SIZE 256
#define PROBE_AT   128
#define PROBE_READ  64

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;

typedef int (*f_open_t)(void *fp, const char *path, uint8_t mode);
typedef int (*f_close_t)(void *fp);
typedef int (*f_read_t)(void *fp, void *buf, uint32_t count, uint32_t *read);
typedef int (*f_write_t)(void *fp, const void *buf, uint32_t count, uint32_t *written);
typedef int (*f_lseek_t)(void *fp, uint32_t offset);
typedef int (*check_create_dir_t)(const char *dir);

#define ENTRY(offset) (LIBRARY_FN_BASE + (offset))

// The two fields the firmware takes straight from the FIL, at the offsets the structure in ff_ifc.h gives them.
#define FIL_OBJSIZE(fp) (*(volatile uint32_t *)((uint8_t *)(fp) + 12))
#define FIL_FPTR(fp)    (*(volatile uint32_t *)((uint8_t *)(fp) + 24))

static const char probe_dir[] = "PGEMU";
static const char probe_path[] = "PGEMU/probe.bin";
static const char big_path[] = "PGEMU/big.bin";

#define BIG_SIZE (3 * READ_BLOCK + 123)                                               // past two block boundaries, so a read has to refill and a seek has to leave the block
#define CHUNK 256
#define WRITE_AT (BIG_SIZE / 2)                                                       // inside the file, so a write there does not change its size

/** The byte the file is expected to contain at one offset. */
static uint8_t pattern(uint32_t offset) {
  return (uint8_t)(offset * 7 + 3);
}

/** \param[in] fp a FIL the emulator provides \return one bit per failed check, zero where every check succeeded */
int pgemu_selftest(void *fp) {
  const f_open_t open_file = (f_open_t)ENTRY(OFF_F_OPEN);
  const f_close_t close_file = (f_close_t)ENTRY(OFF_F_CLOSE);
  const f_read_t read_file = (f_read_t)ENTRY(OFF_F_READ);
  const f_write_t write_file = (f_write_t)ENTRY(OFF_F_WRITE);
  const f_lseek_t seek_file = (f_lseek_t)ENTRY(OFF_F_LSEEK);
  const check_create_dir_t create_dir = (check_create_dir_t)ENTRY(OFF_CHECK_CREATE_DIR);

  uint8_t buffer[PROBE_SIZE];
  uint32_t moved = 0;
  int fail = 0;

  for(uint32_t i = 0; i < PROBE_SIZE; i++) {
    buffer[i] = pattern(i);
  }

  if(create_dir(probe_dir) != FR_OK) {
    return 512;
  }
  if(open_file(fp, probe_path, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
    return 1;
  }
  if(write_file(fp, buffer, PROBE_SIZE, &moved) != FR_OK || moved != PROBE_SIZE) {
    fail |= 2;
  }
  if(FIL_OBJSIZE(fp) != PROBE_SIZE || FIL_FPTR(fp) != PROBE_SIZE) {
    fail |= 4;                                                                        // f_size and f_tell take these two straight out of emulated memory
  }
  close_file(fp);

  if(open_file(fp, probe_path, FA_READ) != FR_OK) {
    return fail | 8;
  }
  if(FIL_OBJSIZE(fp) != PROBE_SIZE) {
    fail |= 16;
  }
  if(seek_file(fp, PROBE_AT) != FR_OK || FIL_FPTR(fp) != PROBE_AT) {
    fail |= 32;
  }
  for(uint32_t i = 0; i < PROBE_READ; i++) {
    buffer[i] = 0;
  }
  if(read_file(fp, buffer, PROBE_READ, &moved) != FR_OK || moved != PROBE_READ) {
    fail |= 64;
  }
  for(uint32_t i = 0; i < PROBE_READ; i++) {
    if(buffer[i] != pattern(PROBE_AT + i)) {
      fail |= 128;
      break;
    }
  }
  if(FIL_FPTR(fp) != PROBE_AT + PROBE_READ) {
    fail |= 256;
  }
  close_file(fp);

  // A file past several read blocks, so a read has to refill and a seek has to leave the block behind.
  if(open_file(fp, big_path, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
    return fail | 1024;
  }
  for(uint32_t at = 0; at < BIG_SIZE; at += CHUNK) {
    uint32_t span = BIG_SIZE - at;
    if(span > CHUNK) {
      span = CHUNK;
    }
    for(uint32_t i = 0; i < span; i++) {
      buffer[i] = pattern(at + i);
    }
    if(write_file(fp, buffer, span, &moved) != FR_OK || moved != span) {
      fail |= 2048;
      break;
    }
  }
  if(FIL_OBJSIZE(fp) != BIG_SIZE) {
    fail |= 4096;
  }
  close_file(fp);

  if(open_file(fp, big_path, FA_READ) != FR_OK) {
    return fail | 8192;
  }
  if(FIL_OBJSIZE(fp) != BIG_SIZE) {
    fail |= 16384;
  }
  const uint32_t sequential = BIG_SIZE < 300 ? BIG_SIZE : 300;
  for(uint32_t i = 0; i < sequential; i++) {                                          // one byte at a time, which is how the firmware loads a state file
    if(read_file(fp, buffer, 1, &moved) != FR_OK || moved != 1 || buffer[0] != pattern(i)) {
      fail |= 32768;
      break;
    }
  }
  const uint32_t crossings[3] = { READ_BLOCK - 4, 2 * READ_BLOCK + 50, 100 };         // over a boundary, well past the block, then backwards inside it
  for(uint32_t c = 0; c < 3; c++) {
    if(seek_file(fp, crossings[c]) != FR_OK || FIL_FPTR(fp) != crossings[c]) {
      fail |= 65536;
      break;
    }
    if(read_file(fp, buffer, 16, &moved) != FR_OK || moved != 16) {
      fail |= 131072;
      break;
    }
    for(uint32_t i = 0; i < 16; i++) {
      if(buffer[i] != pattern(crossings[c] + i)) {
        fail |= 262144;
        break;
      }
    }
  }
  if(seek_file(fp, BIG_SIZE - 4) != FR_OK || read_file(fp, buffer, 16, &moved) != FR_OK || moved != 4) {
    fail |= 524288;                                                                   // a read past the end returns what there is and no more
  }
  close_file(fp);

  // A write has to leave no stale block behind. The read comes first so that a block covering the region is in hand when the write lands on it, which is the only
  // order in which a block kept across a write shows up.
  if(open_file(fp, big_path, FA_READ | FA_WRITE) != FR_OK) {
    return fail | 1048576;
  }
  if(seek_file(fp, WRITE_AT) != FR_OK || read_file(fp, buffer, 8, &moved) != FR_OK || moved != 8) {
    fail |= 2097152;
  }
  for(uint32_t i = 0; i < 8; i++) {
    buffer[i] = (uint8_t)(0xA0 + i);
  }
  if(seek_file(fp, WRITE_AT) != FR_OK || write_file(fp, buffer, 8, &moved) != FR_OK || moved != 8) {
    fail |= 2097152;
  }
  for(uint32_t i = 0; i < 8; i++) {
    buffer[i] = 0;
  }
  if(seek_file(fp, WRITE_AT) != FR_OK || read_file(fp, buffer, 8, &moved) != FR_OK || moved != 8) {
    fail |= 4194304;
  }
  for(uint32_t i = 0; i < 8; i++) {
    if(buffer[i] != (uint8_t)(0xA0 + i)) {
      fail |= 8388608;
      break;
    }
  }
  if(FIL_OBJSIZE(fp) != BIG_SIZE) {
    fail |= 16777216;                                                                 // a write inside the file does not change its size
  }
  close_file(fp);
  return fail;
}
