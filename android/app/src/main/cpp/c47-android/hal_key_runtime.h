// SPDX-License-Identifier: GPL-3.0-only

#if !defined(HAL_KEY_RUNTIME_H)
#define HAL_KEY_RUNTIME_H

#include <stdbool.h>
#include <stdint.h>

// Core-thread yield, key-ID translation, and shutdown synchronisation (implemented in hal/keys.c).
// Lock acquisition order: screenMutex (recursive, core/JNI) -> keyMutex (non-recursive, hal/keys.c).
// Never acquire screenMutex while keyMutex is locked.
bool isAndroidRuntimeReady(void);
bool shouldQueueInputToDmcp(void);
void unlockScreenMutexAndPump(uint32_t totalMs, bool doSleep);
void applyPendingCoreShutdownOnCoreThread(void);
void notifyCoreShutdownRequested(void);
void clearCoreShutdownRequested(void);
bool isCoreShutdownRequested(void);
bool formatCalculatorKeyId(int keyCode, char outKeyStr[4], bool *outIsFn);
int parseCalculatorKeyId(const char *data);
void resetInputGestures(void);

#endif // !defined(HAL_KEY_RUNTIME_H)
