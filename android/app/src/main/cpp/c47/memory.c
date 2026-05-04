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
  bool_t haveOneBigBlockForAllN = false;
  bool_t haveExtraBlock         = false;

  const size_t spaciousBlockSize = sizeInBlocks + extraSize;
  for(i=0; i<numberOfFreeMemoryRegions; i++) {
    const size_t thisBlockSize = freeMemoryRegions[i].sizeInBlocks;

    if(thisBlockSize >= requiredForNBlocks + extraSize) {
      return true;
    }
    if(thisBlockSize >= sizeInBlocks) {
      countOfBlocksOfSize++;
      if(thisBlockSize >= spaciousBlockSize) {
        // This block holds the desired block plus the extra space
        haveExtraBlock = true;
      }
      if (countOfBlocksOfSize > numBlocks) {
        // We've got enough blocks and one over for the extra space
        return true;
      }
      if (countOfBlocksOfSize == numBlocks && haveExtraBlock) {
        // We've found enough large blocks and already have the extra space
        return true;
      }
    }
    else if(thisBlockSize >= extraSize) {
      haveExtraBlock = true;
      if (countOfBlocksOfSize >= numBlocks) {
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





void *allocGmp(size_t sizeInBytes) {
  sizeInBytes = TO_BYTES(TO_BLOCKS(sizeInBytes));
  gmpMemInBytes += sizeInBytes;

  //return freeListAlloc(TO_BLOCKS(sizeInBytes));
  return malloc(sizeInBytes);
}

void *reallocGmp(void *pcMemPtr, size_t oldSizeInBytes, size_t newSizeInBytes) {
  newSizeInBytes = TO_BYTES(TO_BLOCKS(newSizeInBytes));
  oldSizeInBytes = TO_BYTES(TO_BLOCKS(oldSizeInBytes));

  gmpMemInBytes += newSizeInBytes - oldSizeInBytes;

  //return freeListRealloc(pcMemPtr, TO_BLOCKS(oldSizeInBytes), TO_BLOCKS(newSizeInBytes));
  return realloc(pcMemPtr, newSizeInBytes);
}

void freeGmp(void *pcMemPtr, size_t sizeInBytes) {
  sizeInBytes = TO_BYTES(TO_BLOCKS(sizeInBytes));
  gmpMemInBytes -= sizeInBytes;

  //freeListFree(pcMemPtr, TO_BLOCKS(sizeInBytes));
  free(pcMemPtr);
}



void resizeProgramMemory(uint16_t newSizeInBlocks) {
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
