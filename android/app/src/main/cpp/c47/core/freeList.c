// SPDX-License-Identifier: GPL-3.0-only
// SPDX-FileCopyrightText: Copyright The WP43 and C47 Authors

#include "c47.h"

  #if !defined(DMCP_BUILD) && !defined(WIN32)
    #include <execinfo.h>
  #endif // !DMCP_BUILD

void *freeListAlloc(size_t sizeInBlocks) {
  uint16_t minSizeInBlocks = 65535u, minBlock = C47_NULL;
  int i;
  void *pcMemPtr;

  if(sizeInBlocks == 0) {
    sizeInBlocks = 1;
  }

  #if !defined(DMCP_BUILD)
    //if(debugMemAllocation) {
    //  printf("Allocating %" PRIu64 " bytes (%" PRIu16 " blocks)\n", (uint64_t)TO_BYTES(sizeInBlocks), sizeInBlocks);
    //}
  #endif // !DMCP_BUILD

  // Search the smalest hole where the claimed block fits
  //debugMemory();
  for(i=0; i<numberOfFreeMemoryRegions; i++) {
    if(freeMemoryRegions[i].sizeInBlocks == sizeInBlocks) {
      #if !defined(DMCP_BUILD)
        //if(debugMemAllocation) {
        //  printf("The block found is the size of the one claimed at address %u\n", freeMemoryRegions[i].blockAddress);
        //}
      #endif // !DMCP_BUILD
      pcMemPtr = TO_PCMEMPTR(freeMemoryRegions[i].blockAddress);
      xcopy(freeMemoryRegions + i, freeMemoryRegions + i + 1, (numberOfFreeMemoryRegions - i - 1) * sizeof(freeMemoryRegion_t));
      numberOfFreeMemoryRegions--;
      //debugMemory("freeListAlloc: found a memory region with the exact requested size!");
      #if !defined(DMCP_BUILD)
        allocatedMemoryRegions[numberOfAllocatedMemoryRegions].blockAddress = TO_C47MEMPTR(pcMemPtr);
        allocatedMemoryRegions[numberOfAllocatedMemoryRegions].sizeInBlocks = sizeInBlocks;
        //printf("Memory allocation: %5zd blocks at address %5u     (number of allocated regions = %4d)\n", sizeInBlocks, TO_C47MEMPTR(pcMemPtr), numberOfAllocatedMemoryRegions + 1);
        //fflush(stderr);
        numberOfAllocatedMemoryRegions++;
        if(numberOfAllocatedMemoryRegions >= MAX_ALLOCATED_REGIONS) {
          errorf("numberOfAllocatedMemoryRegions is >= MAX_ALLOCATED_REGIONS, increase MAX_ALLOCATED_REGIONS in defines.h (this affects only the PC simulator not the HW firmware)");
        }
      #endif // !DMCP_BUILD
      return pcMemPtr;
    }
    else if(freeMemoryRegions[i].sizeInBlocks > sizeInBlocks && freeMemoryRegions[i].sizeInBlocks < minSizeInBlocks) {
      minSizeInBlocks = freeMemoryRegions[i].sizeInBlocks;
      minBlock = i;
    }
  }

  if(minBlock == C47_NULL) {
    #if defined(DMCP_BUILD)
      //backToSystem(NOPARAM);
    #else // !DMCP_BUILD
      minSizeInBlocks = 0;
      for(i=0; i<numberOfFreeMemoryRegions; i++) {
        minSizeInBlocks += freeMemoryRegions[i].sizeInBlocks;
      }
      printf("\nOUT OF MEMORY\nMemory claimed: %" PRIu64 " bytes\nFragmented free memory: %u bytes\n", (uint64_t)TO_BYTES(sizeInBlocks), TO_BYTES(minSizeInBlocks));
      //exit(-3);
    #endif // DMCP_BUILD
    return NULL;
  }

  #if !defined(DMCP_BUILD)
    //if(debugMemAllocation) {
    //  printf("The block found is larger than the one claimed\n");
    //}
  #endif // !DMCP_BUILD
  pcMemPtr = TO_PCMEMPTR(freeMemoryRegions[minBlock].blockAddress);
  freeMemoryRegions[minBlock].blockAddress += sizeInBlocks;
  freeMemoryRegions[minBlock].sizeInBlocks -= sizeInBlocks;

  //debugMemory("freeListAlloc: allocated within the smalest memory region found that is large enough.");
  #if !defined(DMCP_BUILD)
    allocatedMemoryRegions[numberOfAllocatedMemoryRegions].blockAddress = TO_C47MEMPTR(pcMemPtr);
    allocatedMemoryRegions[numberOfAllocatedMemoryRegions].sizeInBlocks = sizeInBlocks;
    //printf("Memory allocation: %5zd blocks at address %5u     (number of allocated regions = %4d)\n", sizeInBlocks, TO_C47MEMPTR(pcMemPtr), numberOfAllocatedMemoryRegions + 1);
    //fflush(stderr);
    numberOfAllocatedMemoryRegions++;
    if(numberOfAllocatedMemoryRegions >= MAX_ALLOCATED_REGIONS) {
      errorf("numberOfAllocatedMemoryRegions is >= MAX_ALLOCATED_REGIONS, increase MAX_ALLOCATED_REGIONS in defines.h (this affects only the PC simulator not the HW firmware)");
    }
  #endif // !DMCP_BUILD
  return pcMemPtr;
}

void *freeListRealloc(void *pcMemPtr, size_t oldSizeInBlocks, size_t newSizeInBlocks) {
  void *newMemPtr;

  // GMP never calls realloc with pcMemPtr beeing NULL
  if(oldSizeInBlocks == 0) {
    oldSizeInBlocks = 1;
  }

  if(newSizeInBlocks == 0) {
    newSizeInBlocks = 1;
  }

  #if !defined(DMCP_BUILD)
    //printf("Allocating %zd bytes and freeing %zd blocks\n", newSizeInBlocks, oldSizeInBlocks);
  #endif // !DMCP_BUILD

  if((newMemPtr = freeListAlloc(newSizeInBlocks))) {
    xcopy(newMemPtr, pcMemPtr, TO_BYTES(min(newSizeInBlocks, oldSizeInBlocks)));
    freeListFree(pcMemPtr, oldSizeInBlocks);

    return newMemPtr;
  }
  else { // not enough memory!
    return NULL;
  }
}

void freeListReduce(void *pcMemPtr, size_t oldSizeInBlocks, size_t newSizeInBlocks) {
  // GMP never calls realloc with pcMemPtr beeing NULL
  if(oldSizeInBlocks == 0) {
    oldSizeInBlocks = 1;
  }

  uint16_t C47RamPtr = TO_C47MEMPTR(pcMemPtr);

  #if !defined(DMCP_BUILD)
    //printf("Reducing at %zd from %zd blocks to %zd blocks\n", TO_C47MEMPTR(pcMemPtr), oldSizeInBlocks, newSizeInBlocks);
    int region;
    for(region=0; region<numberOfAllocatedMemoryRegions; region++) {
      if(allocatedMemoryRegions[region].blockAddress == C47RamPtr) {
        //printf("Memory reducing: from %05zd blocks to %5zd blocks at address %5u     (number of allocated regions = %4d)\n", oldSizeInBlocks, newSizeInBlocks, C47RamPtr, numberOfAllocatedMemoryRegions - 1);
        if(allocatedMemoryRegions[region].sizeInBlocks != oldSizeInBlocks) {
          errorf("---->Memory reducing:");
          fprintf(stderr, "%zd blocks at address %" PRIu16 ", but %" PRIu16 " were allocated\n", oldSizeInBlocks, C47RamPtr, allocatedMemoryRegions[region].sizeInBlocks);
          fflush(stderr);
        }

        allocatedMemoryRegions[region].sizeInBlocks -= oldSizeInBlocks - newSizeInBlocks;

        region = -1;
        break;
      }
    }
    if(region >= numberOfAllocatedMemoryRegions) {
      errorf("---->Memory reducing:");
      fprintf(stderr, "%5zd blocks at address %5u never allocated at this address     (number of allocated regions = %4d)\n", oldSizeInBlocks, C47RamPtr, numberOfAllocatedMemoryRegions);
      fflush(stderr);
    }
  #endif // !DMCP_BUILD

  // is the freed block just before an other free block?
  bool_t done = false;
  uint16_t addr = C47RamPtr + oldSizeInBlocks; // Address of the 1st bloc after the blocks to be freed
  C47RamPtr += newSizeInBlocks; // Address of the block to be freed
  int32_t i;
  for(i=0; i<numberOfFreeMemoryRegions && freeMemoryRegions[i].blockAddress<=addr; i++) {
    if(freeMemoryRegions[i].blockAddress == addr) {
      freeMemoryRegions[i].blockAddress = C47RamPtr;
      freeMemoryRegions[i].sizeInBlocks += oldSizeInBlocks - newSizeInBlocks;
      done = true;
      break;
    }
  }

  #if !defined(DMCP_BUILD)
    // check for overlap
    for(i=1; i<numberOfFreeMemoryRegions; i++) {
      if((freeMemoryRegions[i-1].blockAddress + freeMemoryRegions[i-1].sizeInBlocks) >= freeMemoryRegions[i].blockAddress) {
        printf("\n*** Free memory regions overlap discovered in freeListReduce()!\n");
        printf("*** This suggests there was double-free!\n");
        //printf("Free blocks (%" PRId32 "):\n", numberOfFreeMemoryRegions);
        //for(int32_t j=0; j<numberOfFreeMemoryRegions; j++) {
        //  printf("  %2" PRId32 " starting at %5" PRIu16 ": %5" PRIu16 " blocks = %6" PRIu32 " bytes\n", j, freeMemoryRegions[j].blockAddress, freeMemoryRegions[j].sizeInBlocks, TO_BYTES((uint32_t)freeMemoryRegions[j].sizeInBlocks));
        //}
        break;
      }
    }
  #endif // !DMCP_BUILD

  // new free block
  if(!done) {
    if(numberOfFreeMemoryRegions == MAX_FREE_REGIONS) {
      #if defined(DMCP_BUILD)
        backToSystem(NOPARAM);
      #else // !DMCP_BUILD
        printf("\n**********************************************************************\n");
        printf("* The maximum number of free memory blocks has been exceeded!        *\n");
        printf("* This number must be increased or the compaction function improved. *\n");
        printf("**********************************************************************\n");
        exit(-2);
      #endif // DMCP_BUILD
    }

    i = 0;
    while(i<numberOfFreeMemoryRegions && freeMemoryRegions[i].blockAddress < C47RamPtr) {
      i++;
    }

    if(i < numberOfFreeMemoryRegions) {
      xcopy(freeMemoryRegions + i + 1, freeMemoryRegions + i, (numberOfFreeMemoryRegions - i) * sizeof(freeMemoryRegion_t));
    }

    freeMemoryRegions[i].blockAddress = C47RamPtr;
    freeMemoryRegions[i].sizeInBlocks = oldSizeInBlocks - newSizeInBlocks;
    numberOfFreeMemoryRegions++;
  }
}

void freeListFree(void *pcMemPtr, size_t sizeInBlocks) {
  uint16_t C47RamPtr, addr;
  int32_t i, j;
  bool_t done;

  // GMP never calls free with pcMemPtr beeing NULL
  if(pcMemPtr == NULL) {
    return;
  }

  if(sizeInBlocks == 0) {
    sizeInBlocks = 1;
  }
  C47RamPtr = TO_C47MEMPTR(pcMemPtr);
  #if !defined(DMCP_BUILD)
    //printf("Freeing %zd bytes\n", TO_BYTES(sizeInBlocks));
    int region;
    for(region=0; region<numberOfAllocatedMemoryRegions; region++) {
      if(allocatedMemoryRegions[region].blockAddress == C47RamPtr) {
        //printf("Memory freeing: %5zd blocks at address %5u     (number of allocated regions = %4d)\n", sizeInBlocks, C47RamPtr, numberOfAllocatedMemoryRegions - 1);
        if(allocatedMemoryRegions[region].sizeInBlocks != sizeInBlocks) {
          errorf("---->Memory freeing A (regions):");
          fprintf(stderr, "%zd blocks at address %" PRIu16 ", but %" PRIu16 " were allocated\n", sizeInBlocks, C47RamPtr, allocatedMemoryRegions[region].sizeInBlocks);
                                        #if !defined(WIN32)
                                          void *callstack[128];
                                          int frames = backtrace(callstack, 128);
                                          char **strs = backtrace_symbols(callstack, frames);
                                          printf("%30s%42s%s\n\n\n", "", "freeListFree called from: ", strs[1]);
                                          printf("%30s%42s%s\n\n\n", "", "backtrace: ", "");
                                          for (int i = 1; i < frames; i++) {
                                              printf("%30s%42d: %s\n", "", i, strs[i]);
                                          }
                                          free(strs);
                                        #endif 
          fflush(stderr);
        }
        if(numberOfAllocatedMemoryRegions - region - 1) {
          xcopy(allocatedMemoryRegions + region, allocatedMemoryRegions + region + 1, (numberOfAllocatedMemoryRegions - region - 1) * sizeof(freeMemoryRegion_t));
        }
        numberOfAllocatedMemoryRegions--;
        region = -1;
        break;
      }
    }
    if(region >= numberOfAllocatedMemoryRegions) {
      errorf("---->Memory freeing B (number of regions):");
      fprintf(stderr, "%5zd blocks at address %5u never allocated at this address     (number of allocated regions = %4d)\n", sizeInBlocks, C47RamPtr, numberOfAllocatedMemoryRegions);
                                        #if !defined(WIN32)
                                          void *callstack[128];
                                          int frames = backtrace(callstack, 128);
                                          char **strs = backtrace_symbols(callstack, frames);
                                          printf("%30s%42s%s\n\n\n", "", "freeListFree called from: ", strs[1]);
                                          printf("%30s%42s%s\n\n\n", "", "backtrace: ", "");
                                          for (int i = 1; i < frames; i++) {
                                              printf("%30s%42d: %s\n", "", i, strs[i]);
                                          }
                                          free(strs);
                                        #endif
      fflush(stderr);
    }
  #endif // !DMCP_BUILD

  done = false;

  // is the freed block just before an other free block?
  addr = C47RamPtr + sizeInBlocks; // Address of the 1st bloc after the blocks to be freed;
  for(i=0; i<numberOfFreeMemoryRegions && freeMemoryRegions[i].blockAddress<=addr; i++) {
    if(freeMemoryRegions[i].blockAddress == addr) {
      freeMemoryRegions[i].blockAddress = C47RamPtr;
      freeMemoryRegions[i].sizeInBlocks += sizeInBlocks;
      sizeInBlocks = freeMemoryRegions[i].sizeInBlocks;
      j = i;
      done = true;
      break;
    }
  }

  // is the freed block just after an other free block?
  for(i=0; i<numberOfFreeMemoryRegions && freeMemoryRegions[i].blockAddress+freeMemoryRegions[i].sizeInBlocks<=C47RamPtr; i++) {
    if(freeMemoryRegions[i].blockAddress + freeMemoryRegions[i].sizeInBlocks == C47RamPtr) {
      freeMemoryRegions[i].sizeInBlocks += sizeInBlocks;
      if(done) {
        xcopy(freeMemoryRegions + j, freeMemoryRegions + j + 1, (numberOfFreeMemoryRegions - j - 1) * sizeof(freeMemoryRegion_t));
        numberOfFreeMemoryRegions--;
      }
      else {
        done = true;
      }
      break;
    }
  }

  #if !defined(DMCP_BUILD)
    // check for overlap
    for(i=1; i<numberOfFreeMemoryRegions; i++) {
      if((freeMemoryRegions[i-1].blockAddress + freeMemoryRegions[i-1].sizeInBlocks) >= freeMemoryRegions[i].blockAddress) {
        printf("\n*** Free memory regions overlap discovered in freeListFree()!\n");
        printf("*** This suggests there was double-free!\n");
        //printf("Free blocks (%" PRId32 "):\n", numberOfFreeMemoryRegions);
        //for(j=0; j<numberOfFreeMemoryRegions; j++) {
        //  printf("  %2" PRId32 " starting at %5" PRIu16 ": %5" PRIu16 " blocks = %6" PRIu32 " bytes\n", j, freeMemoryRegions[j].blockAddress, freeMemoryRegions[j].sizeInBlocks, TO_BYTES((uint32_t)freeMemoryRegions[j].sizeInBlocks));
        //}
        break;
      }
    }
  #endif // !DMCP_BUILD

  // new free block
  if(!done) {
    if(numberOfFreeMemoryRegions == MAX_FREE_REGIONS) {
      #if defined(DMCP_BUILD)
        backToSystem(NOPARAM);
      #else // !DMCP_BUILD
        printf("\n**********************************************************************\n");
        printf("* The maximum number of free memory blocks has been exceeded!        *\n");
        printf("* This number must be increased or the compaction function improved. *\n");
        printf("**********************************************************************\n");
        exit(-2);
      #endif // DMCP_BUILD
    }

    i = 0;
    while(i<numberOfFreeMemoryRegions && freeMemoryRegions[i].blockAddress < C47RamPtr) {
      i++;
    }

    if(i < numberOfFreeMemoryRegions) {
      xcopy(freeMemoryRegions + i + 1, freeMemoryRegions + i, (numberOfFreeMemoryRegions - i) * sizeof(freeMemoryRegion_t));
    }

    freeMemoryRegions[i].blockAddress = C47RamPtr;
    freeMemoryRegions[i].sizeInBlocks = sizeInBlocks;
    numberOfFreeMemoryRegions++;
  }

  //debugMemory("freeListFree : end");
}
