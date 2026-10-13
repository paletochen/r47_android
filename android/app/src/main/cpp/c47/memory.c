// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

uint32_t getFreeRamMemory(void) {
  uint32_t freeMem;
  int32_t i;

  freeMem = 0;
  for(i=0; i<numberOfFreeMemoryRegions; i++) {
    freeMem += freeMemoryRegions[i].sizeInBlocks;
  }

  return TO_BYTES(freeMem);
}

#if !defined(DMCP_BUILD)
  void debugMemory(const char *message) {
    printf("\n%s\nC47 owns %6" PRIu64 " bytes and GMP owns %6" PRIu64 " bytes (%" PRId32 " bytes free)\n", message, (uint64_t)TO_BYTES(c47MemInBlocks), (uint64_t)gmpMemInBytes, getFreeRamMemory());
    printf("    Addr   Size\n");
    for(int i=0; i<numberOfFreeMemoryRegions; i++) {
      printf("%2d%6u%7u\n", i, freeMemoryRegions[i].blockAddress, freeMemoryRegions[i].sizeInBlocks);
    }
    printf("\n");
  }
#endif // !DMCP_BUILD

bool_t isMemoryBlockAvailable(size_t sizeInBlocks, uint16_t numBlocks, float extraFraction) {
  int i;
  size_t extraSize              = (size_t)((float)sizeInBlocks * extraFraction);
  size_t requiredForNBlocks     = sizeInBlocks * numBlocks;
  size_t countOfBlocksOfSize    = 0;
  bool_t haveExtraBlock         = false;

  for(i=0; i<numberOfFreeMemoryRegions; i++) {
    const size_t thisBlockSize = freeMemoryRegions[i].sizeInBlocks;

    if(thisBlockSize >= requiredForNBlocks + extraSize) {
      // Total space fits in this block
      return true;
    }
    if(thisBlockSize >= sizeInBlocks) {
      // Count the number of blocks that would fit into this chunk of memory
      countOfBlocksOfSize += thisBlockSize / sizeInBlocks;
      const size_t residualSize = thisBlockSize % sizeInBlocks;

      if(residualSize >= extraSize) {
        // This block holds a number of block(s) plus the extra space
        haveExtraBlock = true;
      }
      if(countOfBlocksOfSize > numBlocks) {
        // We've got enough blocks and one over for the extra space
        return true;
      }
      if(countOfBlocksOfSize == numBlocks && haveExtraBlock) {
        // We've found enough large blocks and already have the extra space
        return true;
      }
    }
    else if(thisBlockSize >= extraSize) {
      haveExtraBlock = true;
      if(countOfBlocksOfSize >= numBlocks) {
        // We've found enough large blocks and now have the extra space
        return true;
      }
    }
  }

  return false;
}




void *allocC47Blocks(size_t sizeInBlocks) {
  //c47MemInBlocks += sizeInBlocks;

  //return freeListAlloc(sizeInBlocks);

  void *allocated = freeListAlloc(sizeInBlocks);
  if(allocated) {
    c47MemInBlocks += sizeInBlocks;
    return allocated;
  }
  else {
    return NULL;
  }
}

void *reallocC47Blocks(void *pcMemPtr, size_t oldSizeInBlocks, size_t newSizeInBlocks) {
  //c47MemInBlocks += newSizeInBlocks - oldSizeInBlocks;

  //return freeListRealloc(pcMemPtr, oldSizeInBlocks, newSizeInBlocks);

  void *allocated = freeListRealloc(pcMemPtr, oldSizeInBlocks, newSizeInBlocks);
  if(allocated) {
    c47MemInBlocks += newSizeInBlocks - oldSizeInBlocks;
    return allocated;
  }
  else {
    return NULL;
  }
}

void reduceC47Blocks(void *pcMemPtr, size_t oldSizeInBlocks, size_t newSizeInBlocks) {
  if(newSizeInBlocks == 0) {
    freeC47Blocks(pcMemPtr, oldSizeInBlocks);
    return;
  }

  freeListReduce(pcMemPtr, oldSizeInBlocks, newSizeInBlocks);
  c47MemInBlocks += newSizeInBlocks - oldSizeInBlocks;
}

void freeC47Blocks(void *pcMemPtr, size_t sizeInBlocks) {
  if(pcMemPtr == NULL) {
    return;
  }

  c47MemInBlocks -= sizeInBlocks;

  freeListFree(pcMemPtr, sizeInBlocks);
}





static void *gmpAllocFailed(void) {
  // GMP never checks what its allocator returns, so a NULL here becomes limb writes through a null pointer. TMP_ALLOC blocks over the 1 KB split of the gmp-impl.h
  // overlay reach these functions too, through __gmp_tmp_reentrant_alloc, so heap exhaustion stops here with a message instead of corrupting memory mid-arithmetic.
  displayBugScreen("In function allocGmp: GMP is out of memory");
  #if defined(DMCP_BUILD)
    lcd_refresh();
    wait_for_key_press();
    sys_reset();
  #else // !DMCP_BUILD
    exit(255);
  #endif // DMCP_BUILD
  return NULL;
}

void *allocGmp(size_t sizeInBytes) {
  sizeInBytes = TO_BYTES(TO_BLOCKS(sizeInBytes));
  gmpMemInBytes += sizeInBytes;

  //return freeListAlloc(TO_BLOCKS(sizeInBytes));
  void *pcMemPtr = malloc(sizeInBytes);
  if(pcMemPtr == NULL && sizeInBytes != 0) {
    return gmpAllocFailed();
  }
  return pcMemPtr;
}

void *reallocGmp(void *pcMemPtr, size_t oldSizeInBytes, size_t newSizeInBytes) {
  newSizeInBytes = TO_BYTES(TO_BLOCKS(newSizeInBytes));
  oldSizeInBytes = TO_BYTES(TO_BLOCKS(oldSizeInBytes));

  gmpMemInBytes += newSizeInBytes - oldSizeInBytes;

  //return freeListRealloc(pcMemPtr, TO_BLOCKS(oldSizeInBytes), TO_BLOCKS(newSizeInBytes));
  void *pcNewPtr = realloc(pcMemPtr, newSizeInBytes);
  if(pcNewPtr == NULL && newSizeInBytes != 0) {
    return gmpAllocFailed();
  }
  return pcNewPtr;
}

void freeGmp(void *pcMemPtr, size_t sizeInBytes) {
  sizeInBytes = TO_BYTES(TO_BLOCKS(sizeInBytes));
  gmpMemInBytes -= sizeInBytes;

  //freeListFree(pcMemPtr, TO_BLOCKS(sizeInBytes));
  free(pcMemPtr);
}



void resizeProgramMemory(uint16_t newSizeInBlocks) {
  // The program area always holds at least the block that carries the empty .END. - config.c reserves exactly that when it forms the pool. At zero blocks
  // beginOfProgramMemory sits one past the pool, and every reader that starts there, isAtEndOfPrograms() first, reads out of bounds.
  if(newSizeInBlocks == 0) {
    newSizeInBlocks = 1;
  }

  uint16_t currentSizeInBlocks = RAM_SIZE_IN_BLOCKS - TO_C47MEMPTR(beginOfProgramMemory);
  uint16_t deltaBlocks, blocksToMove = 0;
  uint8_t *newProgramMemoryPointer = NULL;

  #if !defined(DMCP_BUILD)
    //printf("currentSizeInBlocks = %u    newSizeInBlocks = %u\n", currentSizeInBlocks, newSizeInBlocks);
    //printf("currentAddress      = %u\n", TO_C47MEMPTR(beginOfProgramMemory));
  #endif // !DMCP_BUILD
  if(newSizeInBlocks == currentSizeInBlocks) { // Nothing to do
    return;
  }

  if(newSizeInBlocks > currentSizeInBlocks) { // Increase program memory size
    deltaBlocks = newSizeInBlocks - currentSizeInBlocks;
    if(deltaBlocks > freeMemoryRegions[numberOfFreeMemoryRegions - 1].sizeInBlocks) { // Out of memory
      #if defined(DMCP_BUILD)
        backToSystem(NOPARAM);
      #else // !DMCP_BUILD
        int32_t freeMemory = 0;
        for(int32_t i=0; i<numberOfFreeMemoryRegions; i++) {
          freeMemory += freeMemoryRegions[i].sizeInBlocks;
        }
        printf("\nOUT OF MEMORY\nMemory claimed: %" PRIu64 " bytes\nFragmented free memory: %" PRIu64 " bytes\n", (uint64_t)TO_BYTES(deltaBlocks), (uint64_t)TO_BYTES(freeMemory));
        exit(-3);
      #endif // DMCP_BUILD
    }
    else { // There is plenty of memory available
      blocksToMove = currentSizeInBlocks;
      newProgramMemoryPointer = beginOfProgramMemory - TO_BYTES(deltaBlocks);
      firstFreeProgramByte -= TO_BYTES(deltaBlocks);
      #if !defined(DMCP_BUILD)
        //printf("Increasing program memory by copying %u blocks from %u to %u\n", currentSizeInBlocks, TO_C47MEMPTR(beginOfProgramMemory), TO_C47MEMPTR(newProgramMemoryPointer));
      #endif // !DMCP_BUILD
      freeMemoryRegions[numberOfFreeMemoryRegions - 1].sizeInBlocks -= deltaBlocks;
    }
  }
  else { // Decrease program memory size
    deltaBlocks = currentSizeInBlocks - newSizeInBlocks;
    blocksToMove = newSizeInBlocks;
    newProgramMemoryPointer = beginOfProgramMemory + TO_BYTES(deltaBlocks);
    firstFreeProgramByte += TO_BYTES(deltaBlocks);
    #if !defined(DMCP_BUILD)
      //printf("Decreasing program memory by copying %u blocks from %u to %u\n", newSizeInBlocks, TO_C47MEMPTR(beginOfProgramMemory), TO_C47MEMPTR(newProgramMemoryPointer));
    #endif // !DMCP_BUILD
    freeMemoryRegions[numberOfFreeMemoryRegions - 1].sizeInBlocks += deltaBlocks;
  }

  xcopy(newProgramMemoryPointer, beginOfProgramMemory, TO_BYTES(blocksToMove));
  beginOfProgramMemory = newProgramMemoryPointer;
  //debugMemory("resizeProgramMemory : end");
}


#if defined(STACK_WATERMARK)
  // Stack high-water mark. A stretch of the unused stack is marked with a known value, the operation under test overwrites it as far down as it needs, and the
  // lowest word that no longer holds the marker is the deepest point reached. Reading the depth at a chosen moment instead would miss the peak between readings.
  // stackWatermarkAnchor fixes the reference point; every figure is bytes below it. On DMCP that is the head of program_main, so it is the shallowest frame of
  // the run. Elsewhere program_main does not exist and the first marking anchors itself, so simulator figures carry an unknown constant and compare only with
  // each other.
  //
  // How far down to mark cannot be derived, since neither DMCP nor either linker script says how far the stack extends. STCKSPN sets it and STACK_WATERMARK_SPAN
  // below is the default. Raise STCKSPN while cases come back STCKST 1. Too far writes outside the stack and the calculator faults on the next reset. The largest
  // depth confirmed as actually written is 16384, on the DM42n, where STCKSPU read back 16384 on all eight supplied cases; a DM42n run asking for 55000 crashed on
  // the next reset, with no capture showing how far it reached. Nothing bounds the DM42, which never got past a plot with an integral inside it. On the DM42 the block pool is in
  // the same memory, so the floor is raised to clear it whatever STCKSPN says; that has never happened in a run, since in the simulator the pool is elsewhere and
  // the test is always false there.
  //
  // To use it: define STACK_WATERMARK in defines.h and rebuild, then run a program shaped like the one below. A request left in STCKGO is carried out by the next
  // function the program runs, since the marking and the read sit either side of the dispatcher's call in items.c. 🖨xy writes register X then register Y as one tab
  // separated line to a dated DATA\*.REGS.TSV whenever the system flag 🖨ACT is clear, which it is unless the user sets it, so no printer is needed.
  //
  //   16384 STO 'STCKSPN' DROPX              how far down to mark, once, before the cases; leave it out to take the default
  //   1 STO 'STCKGO' DROPX                   lay down a fresh marker, carried out at the next function the program runs
  //   XEQ 'MYCASE'                           the operation under test
  //   2 STO 'STCKGO' DROPX                   read, carried out at the next function the program runs
  //   CLSTK                                  the function that carries it out; CLSTK also leaves the graph, which a plot case needs before a file can be written
  //   RCL 'STCKHI'  'MY CASE'  🖨xy  DROPX DROPX      the figure, labelled
  //   RCL 'STCKST'  's'        🖨xy  DROPX DROPX      what the figure is worth
  //   RCL 'STCKSPU' 'u'        🖨xy  DROPX DROPX      how far down was marked
  //
  // STCKHI is the figure and STCKST says what it is worth: 0 a depth the run reached, 1 the marker was gone to the bottom so the case went at least that far and
  // possibly much further, 2 nothing was disturbed so the figure is only where the tool itself sat, 3 no marker was laid so the figure repeats the reading before.
  // Only 0 is a measurement. STCKSPU reports how far down was marked, after any raising of the floor, so a figure below STCKSPN says the request never arrived.
  // STCKHWM holds the deepest since it was last cleared, and storing 0 into it starts a fresh session. STCKHI, STCKST, STCKHWM and STCKSPU are created by the tool
  // itself at the first dispatch and are ordinary named variables from then on. Worked example and captured results in tools/hwtest/stack-watermark.
  #define STACK_WATERMARK_PATTERN  0xA5C3A5C3u
  #define STACK_WATERMARK_MARK  1
  #define STACK_WATERMARK_READ     2
  // Default depth, per target, so the supplied eight cases need no STCKSPN stored.
  #if defined(OLD_HW)
    #define STACK_WATERMARK_SPAN   8088                      // arbitrary, and the only depth ever run on the DM42; it covers every case there that does not hang
  #elif defined(NEW_HW)
    #define STACK_WATERMARK_SPAN   16384                     // reaches the DM42n's deepest case at 14244, and is the largest depth confirmed written there
  #else // !OLD_HW && !NEW_HW
    #define STACK_WATERMARK_SPAN   65536                     // clears the deepest simulator case ever captured, 19152
  #endif // OLD_HW
  #define STACK_WATERMARK_SPAN_MAX 262144                      // a sanity limit against a wild store, not a safe depth: it is larger than either calculator's stack RAM
  #define STACK_WATERMARK_MEASURED 0                           // STCKST: the figure is a depth the run reached
  #define STACK_WATERMARK_FULL     1                           // the marker was gone to the bottom, so the figure is a floor
  #define STACK_WATERMARK_CLEAN    2                           // nothing was disturbed, so the figure is this function's own depth
  #define STACK_WATERMARK_STALE    3                           // no marker laid since the last reading, so the figure is that reading again

  static uint32_t *stackWatermarkFloor = NULL;                 // absolute, never written below
  static uintptr_t stackWatermarkTop = 0;                      // the anchor frame, the shallowest the stack ever is
  static uintptr_t stackWatermarkMarkedTo = 0;                // where the last marking stopped, always above the floor
  static bool_t    stackWatermarkActive = false;
  static bool_t    stackWatermarkMarkLaid = false;            // a marker has been laid since the last reading, so the next figure is that one's and not the one before
  static int32_t   stackWatermarkSpan = STACK_WATERMARK_SPAN;  // the span in force, set from STCKSPN where a marker is asked for, see _stackWatermarkReadSpan

  void stackWatermarkAnchor(void) {
    stackWatermarkTop = (uintptr_t)__builtin_frame_address(0) & ~(uintptr_t)3;
    stackWatermarkFloor = (uint32_t *)(stackWatermarkTop - STACK_WATERMARK_SPAN);   // a non-NULL sentinel; every marking recomputes this from the depth then in force
  }

  // STCKSPN into stackWatermarkSpan, the default until one is stored. Called only where STCKGO asks for a fresh marker, alongside the STCKGO read itself, because
  // that is a point the program chose. A marker is also laid unasked at any dispatch outside a run, and the span is a setting rather than something to look up
  // again at each of those.
  static void _stackWatermarkReadSpan(void) {
    calcRegister_t regist = findNamedVariable("STCKSPN");
    longInteger_t value;

    if(regist == INVALID_VARIABLE || getRegisterDataType(regist) != dtLongInteger) {
      return;
    }
    convertLongIntegerRegisterToLongInteger(regist, value);
    if(longIntegerCompareInt(value, 0) > 0 && longIntegerCompareInt(value, STACK_WATERMARK_SPAN_MAX) <= 0) {   // any depth up to the cap, above or below the default
      longIntegerToInt32(value, stackWatermarkSpan);
      stackWatermarkSpan &= ~3;                                // the floor is read a word at a time, so keep it aligned
    }
    longIntegerFree(value);
  }

  // A long integer, so the display format leaves the byte count alone.
  static void _stackWatermarkWrite(const char *variableName, int32_t used, bool_t raiseOnly) {
    calcRegister_t regist = findOrAllocateNamedVariable(variableName);
    longInteger_t value;

    if(regist == INVALID_VARIABLE) {
      return;
    }
    if(raiseOnly && getRegisterDataType(regist) == dtLongInteger) {
      convertLongIntegerRegisterToLongInteger(regist, value);   // initialises value, so it is not pre-inited here
      if(longIntegerCompareInt(value, used) >= 0) {
        longIntegerFree(value);
        return;
      }
      longIntegerFree(value);
    }
    longIntegerInit(value);
    int32ToLongInteger(used, value);
    convertLongIntegerToLongIntegerRegister(value, regist);
    longIntegerFree(value);
  }

  // What STCKGO is asking for, cleared once read so one request acts once. The trigger cannot be STCKHI or STCKHWM: both are written at every dispatch, so a request
  // parked in either is overwritten before the next dispatch can act on it.
  static int32_t _stackWatermarkRequest(void) {
    calcRegister_t regist = findNamedVariable("STCKGO");
    longInteger_t value;
    int32_t asked;

    if(regist == INVALID_VARIABLE || getRegisterDataType(regist) != dtLongInteger) {
      return 0;
    }
    convertLongIntegerRegisterToLongInteger(regist, value);
    asked = longIntegerCompareInt(value, 0) == 0 ? 0 : (longIntegerCompareInt(value, STACK_WATERMARK_READ) == 0 ? STACK_WATERMARK_READ : STACK_WATERMARK_MARK);
    longIntegerFree(value);
    if(asked == STACK_WATERMARK_MARK) {
      _stackWatermarkReadSpan();                               // the span the next marker uses, taken where the program asked for it
    }
    if(asked != 0) {
      _stackWatermarkWrite("STCKGO", 0, false);
    }
    return asked;
  }

  // The scan, from the floor up to where the marking stopped. It runs upward because a frame leaves words of itself unwritten, so a downward walk from the previous
  // peak would stop at the first of those holes and report too shallow. STCKST says which of four things the figure is, since all four otherwise look like a
  // plain number and only one of them is a measurement.
  static void _stackWatermarkRead(void) {
    uint32_t *p = stackWatermarkFloor;
    int32_t used;

    if(!stackWatermarkActive) {
      return;
    }
    while(p < (uint32_t *)stackWatermarkMarkedTo && *p == STACK_WATERMARK_PATTERN) {
      p++;
    }
    used = (int32_t)(stackWatermarkTop - (uintptr_t)p);        // bytes below the anchor, one origin for both figures
    if(!stackWatermarkMarkLaid) {
      _stackWatermarkWrite("STCKST", STACK_WATERMARK_STALE, false);    // no marker laid since the last reading, so this repeats what that one found
    }
    else if(p == stackWatermarkFloor) {
      _stackWatermarkWrite("STCKST", STACK_WATERMARK_FULL, false);     // disturbed at the floor: the peak is at or beyond the span, a floor not a figure
    }
    else if((uintptr_t)p >= stackWatermarkMarkedTo) {
      _stackWatermarkWrite("STCKST", STACK_WATERMARK_CLEAN, false);    // nothing disturbed anywhere: this is this function's own depth, not the run's
    }
    else {
      _stackWatermarkWrite("STCKST", STACK_WATERMARK_MEASURED, false);
    }
    stackWatermarkMarkLaid = false;
    _stackWatermarkWrite("STCKSPU", (int32_t)(stackWatermarkTop - (uintptr_t)stackWatermarkFloor), false);   // how far down was actually marked, after any clamp
    _stackWatermarkWrite("STCKHI",  used, false);
    _stackWatermarkWrite("STCKHWM", used, true);
  }

  void stackWatermarkBeforeDispatch(void) {
    uintptr_t sp;
    uint8_t savedErrorCode;
    int32_t asked;

    if(stackWatermarkFloor == NULL) {
      stackWatermarkAnchor();                                  // the simulator has no program_main
    }
    savedErrorCode = lastErrorCode;                            // a failure to allocate one of the variables must not be reported as the operation's error
    asked = (programRunStop == PGM_RUNNING) ? _stackWatermarkRequest() : STACK_WATERMARK_MARK;
    if(asked == STACK_WATERMARK_READ) {
      _stackWatermarkRead();
      lastErrorCode = savedErrorCode;
      return;
    }
    if(asked != STACK_WATERMARK_MARK) {
      lastErrorCode = savedErrorCode;                          // running, nothing asked: the standing marker stays
      return;
    }
    lastErrorCode = savedErrorCode;

    stackWatermarkFloor = (uint32_t *)(stackWatermarkTop - stackWatermarkSpan);
    // On the DM42 the block pool is malloc'd in the same SRAM as the stack, so a span reaching past it would mark the registers. Raise the floor to clear it. The
    // pool is checked for being below the anchor because on the DM42n and in the simulator it is not in the stack's memory at all, and there this does nothing.
    if(ram != NULL && ram + RAM_SIZE_IN_BLOCKS > stackWatermarkFloor && (uintptr_t)(ram + RAM_SIZE_IN_BLOCKS) < stackWatermarkTop) {
      stackWatermarkFloor = ram + RAM_SIZE_IN_BLOCKS;
    }
    // 256 clear of this frame, an arbitrary margin covering the marking's own locals and anything the compiler parks below the frame pointer.
    sp = ((uintptr_t)__builtin_frame_address(0) - 256) & ~(uintptr_t)3;
    if(sp <= (uintptr_t)stackWatermarkFloor) {                 // already below the floor: nothing here is stack
      return;
    }
    for(uint32_t *p = stackWatermarkFloor; p < (uint32_t *)sp; p++) {
      *p = STACK_WATERMARK_PATTERN;
    }
    stackWatermarkMarkedTo = sp;
    stackWatermarkMarkLaid = true;
    stackWatermarkActive = true;
  }

  // Called after every dispatch and again at the end of a top level program run. The reading is skipped while a program runs, where the readings come from STCKGO
  // instead, because each one scans the whole marked region.
  void stackWatermarkAfterDispatch(void) {
    uint8_t savedErrorCode;

    if(!stackWatermarkActive || programRunStop == PGM_RUNNING) {
      return;                                                  // inside a run the readings come from STCKGO
    }
    savedErrorCode = lastErrorCode;
    _stackWatermarkRead();
    lastErrorCode = savedErrorCode;
  }
#endif // STACK_WATERMARK
