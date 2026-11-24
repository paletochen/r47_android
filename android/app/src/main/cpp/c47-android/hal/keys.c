// SPDX-License-Identifier: GPL-3.0-only

/**
 * \file hal/keys.c
 * DMCP key FIFO queue, cooperative sleep, delay, timers, and core-shutdown synchronisation for the Android port.
 * JNI input entry points enqueue key codes when the core is busy or yielding, and active core routines drain them via key_pop while yielding
 * to Android in sys_sleep and sys_delay.
 */

#include "jni_bridge.h"

#include <errno.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>

#if !defined(DMCP_KEY_EXIT)
  #define DMCP_KEY_EXIT 33
#endif // !defined(DMCP_KEY_EXIT)

#define KEY_BUFFER_SIZE 64
#define SYS_TIMERS      4

uint32_t calc_state = 0;

// Lock acquisition order across the Android bridge is screenMutex -> keyMutex.
static pthread_mutex_t keyMutex                = PTHREAD_MUTEX_INITIALIZER;
static int             keyBuffer[KEY_BUFFER_SIZE];
static int             keyCount                = 0;
static bool_t          pushedKeyDown           = false;
static gint64          timerDeadline[SYS_TIMERS];
static gint64          lastMinute              = 0;
static int             g_inKeyPoll             = 0;
static uint32_t        g_lastKeyPollMs         = 0;
static atomic_int      g_inCoreYield           = 0;
static atomic_bool     g_coreShutdownRequested = false;

typedef enum {
  KEY_GESTURE_IDLE = 0,
  KEY_GESTURE_QUEUED,
  KEY_GESTURE_DIRECT
} keyGestureMode_t;

typedef struct {
  keyGestureMode_t mode;
  int dmcpCode;
} keyGestureState_t;

static keyGestureState_t touchGesture = {KEY_GESTURE_IDLE, 0};
static keyGestureState_t simGesture   = {KEY_GESTURE_IDLE, 0};

static bool_t pushLocked(int key) {
  // Reserve the final slot for a matching 0 release so every accepted press can record its release.
  const int limit = (key > 0) ? (KEY_BUFFER_SIZE - 1) : KEY_BUFFER_SIZE;
  if(keyCount >= limit) {
    return false;
  }
  keyBuffer[keyCount++] = key;
  return true;
}

static int popLocked(void) {
  if(keyCount == 0) {
    return -1;
  }
  const int key = keyBuffer[0];
  if(keyCount > 1) {
    memmove(&keyBuffer[0], &keyBuffer[1], (size_t)(keyCount - 1) * sizeof(int));
  }
  keyCount--;
  return key;
}

/**
 * Replaces any queued keys with a single EXIT press and release [DMCP_KEY_EXIT, 0] so active waits wake immediately
 * and post-shutdown drain iterations (such as C47PopKeyNoBuffer) finish in two pops.
 */
static void keyPushShutdownExit(void) {
  pthread_mutex_lock(&keyMutex);
  pushedKeyDown = false;
  keyBuffer[0] = DMCP_KEY_EXIT;
  keyBuffer[1] = 0;
  keyCount = 2;
  pthread_mutex_unlock(&keyMutex);
}

/**
 * Returns true when calculator input must be queued into the DMCP FIFO rather than dispatched directly to btnPressed.
 */
bool shouldQueueInputToDmcp(void) {
  return programRunStop == PGM_RUNNING || programRunStop == PGM_PAUSED || atomic_load(&g_inCoreYield) > 0;
}

/**
 * Applies pending shutdown state to core execution flags. Must run on the core thread or while screenMutex is locked.
 */
void applyPendingCoreShutdownOnCoreThread(void) {
  if(atomic_load(&g_coreShutdownRequested)) {
    SET_ST(STAT_PGM_END);
    if(programRunStop == PGM_RUNNING || programRunStop == PGM_PAUSED) {
      programRunStop = PGM_WAITING;
    }
  }
}

static void sleepMilliseconds(uint32_t ms) {
  struct timespec req = {
    .tv_sec  = (time_t)(ms / 1000u),
    .tv_nsec = (long)(ms % 1000u) * 1000000L
  };
  struct timespec rem = {0, 0};
  while(nanosleep(&req, &rem) == -1 && errno == EINTR && !atomic_load(&g_coreShutdownRequested)) {
    req = rem;
  }
}

/**
 * Unwinds recursive screenMutex ownership (relying on PTHREAD_MUTEX_RECURSIVE returning EPERM at lock count 0), drains queued core tasks,
 * optionally sleeps in <= 20 ms slices while polling tasks and shutdown state, relocks screenMutex to the prior depth, and applies shutdown.
 * Precondition: keyMutex is not locked by the calling thread.
 */
void unlockScreenMutexAndPump(uint32_t totalMs, bool doSleep) {
  int lockCount = 0;
  while(pthread_mutex_unlock(&screenMutex) == 0) {
    lockCount++;
  }

  atomic_fetch_add(&g_inCoreYield, 1);
  processCoreTasksNative();
  atomic_fetch_sub(&g_inCoreYield, 1);

  if(doSleep && !atomic_load(&g_coreShutdownRequested)) {
    if(totalMs == 0u) {
      sleepMilliseconds(1u);
    } else {
      const uint32_t startMs = sys_current_ms();
      // elapsedWallMs bounds slice count in unit tests where sys_current_ms() uses a deterministic manual clock.
      uint32_t elapsedWallMs = 0u;
      while(!atomic_load(&g_coreShutdownRequested)) {
        const uint32_t nowMs = sys_current_ms();
        const uint32_t elapsedMs = (nowMs - startMs > elapsedWallMs) ? (nowMs - startMs) : elapsedWallMs;
        if(elapsedMs >= totalMs) {
          break;
        }
        const uint32_t remMs = totalMs - elapsedMs;
        const uint32_t sliceMs = (remMs < 20u) ? remMs : 20u;
        sleepMilliseconds(sliceMs);
        elapsedWallMs += sliceMs;
        if(!atomic_load(&g_coreShutdownRequested) && elapsedWallMs < totalMs && (sys_current_ms() - startMs) < totalMs) {
          atomic_fetch_add(&g_inCoreYield, 1);
          processCoreTasksNative();
          atomic_fetch_sub(&g_inCoreYield, 1);
        }
      }
    }
  }

  while(lockCount > 0) {
    pthread_mutex_lock(&screenMutex);
    lockCount--;
  }
  applyPendingCoreShutdownOnCoreThread();
}

/**
 * Signals the native core to abort any active pause, delay, solver, or program for Activity teardown.
 */
void notifyCoreShutdownRequested(void) {
  atomic_store(&g_coreShutdownRequested, true);
  keyPushShutdownExit();
  if(pthread_mutex_trylock(&screenMutex) == 0) {
    applyPendingCoreShutdownOnCoreThread();
    pthread_mutex_unlock(&screenMutex);
  }
}

/**
 * Saves calculator state and requests app exit when an interactive or programmed OFF or SYSTEM command sets STAT_PGM_END or backToDMCP without an external
 * Activity teardown in progress. Must run while screenMutex is locked.
 */
bool handlePendingPowerOffLocked(void) {
  if(!isCoreShutdownRequested() && (ST(STAT_PGM_END) || backToDMCP)) {
    CLR_ST(STAT_PGM_END);
    CLR_ST(STAT_SUSPENDED);
    CLR_ST(STAT_OFF);
    backToDMCP = false;
    if(programRunStop == PGM_RUNNING || programRunStop == PGM_PAUSED) {
      programRunStop = PGM_WAITING;
    }
    key_pop_all();
    clearKeyBuffer();
    resetInputGestures();
    if(ram != NULL) {
      saveCalc();
    }
    lcd_refresh();
    triggerQuit();
    return true;
  }
  return false;
}

/**
 * Clears the shutdown latch and power-off flags when starting or re-attaching a fresh core session.
 */
void clearCoreShutdownRequested(void) {
  atomic_store(&g_coreShutdownRequested, false);
  CLR_ST(STAT_PGM_END);
  CLR_ST(STAT_SUSPENDED);
  CLR_ST(STAT_OFF);
  backToDMCP = false;
}

/**
 * Returns true if native runtime shutdown has been requested.
 */
bool isCoreShutdownRequested(void) {
  return atomic_load(&g_coreShutdownRequested);
}

/**
 * Encodes a 1-based DMCP key code (1..37 for main keys, 38..43 for softkeys) into the string form taken by btnPressed and btnFnPressed.
 */
bool formatCalculatorKeyId(int keyCode, char outKeyStr[4], bool *outIsFn) {
  if(outKeyStr == NULL || outIsFn == NULL) {
    return false;
  }
  if(keyCode >= 38 && keyCode <= 43) {
    outKeyStr[0] = (char)(keyCode - 38 + '1');
    outKeyStr[1] = '\0';
    *outIsFn = true;
    return true;
  }
  if(keyCode >= 1 && keyCode <= 37) {
    snprintf(outKeyStr, 4, "%02u", (unsigned int)(keyCode - 1));
    *outIsFn = false;
    return true;
  }
  outKeyStr[0] = '\0';
  *outIsFn = false;
  return false;
}

/**
 * Parses a button string ("00".."36" for main keys, "1".."6" for softkeys) into a 1-based DMCP key code, or returns 0 if invalid.
 */
int parseCalculatorKeyId(const char *data) {
  if(data == NULL || data[0] == '\0') {
    return 0;
  }
  if(data[1] == '\0' && data[0] >= '1' && data[0] <= '6') {
    return data[0] - '1' + 38;
  }
  if(data[0] >= '0' && data[0] <= '3' && data[1] >= '0' && data[1] <= '9' && data[2] == '\0') {
    const int code = (data[0] - '0') * 10 + (data[1] - '0') + 1;
    return (code >= 1 && code <= 37) ? code : 0;
  }
  return 0;
}

/**
 * Returns 1 when no DMCP key code is buffered, or 0 when at least one key code is buffered.
 * When empty on an active runtime, drains pending core tasks at most once per 25 ms with screenMutex unlocked so non-program operations
 * (SOLVE, INTEGRATE, DRAW, FACTOR) receive EXIT or R/S.
 */
int key_empty(void) {
  if(isCoreShutdownRequested()) {
    applyPendingCoreShutdownOnCoreThread();
    pthread_mutex_lock(&keyMutex);
    const bool_t empty = (keyCount == 0);
    pthread_mutex_unlock(&keyMutex);
    return empty ? 1 : 0;
  }

  bool_t shouldPoll = false;
  pthread_mutex_lock(&keyMutex);
  bool_t empty = (keyCount == 0);
  if(empty && isAndroidRuntimeReady() && g_inKeyPoll == 0) {
    const uint32_t now = sys_current_ms();
    if(now - g_lastKeyPollMs >= 25u) {
      g_lastKeyPollMs = now;
      g_inKeyPoll = 1;
      shouldPoll = true;
    }
  }
  pthread_mutex_unlock(&keyMutex);

  if(shouldPoll) {
    unlockScreenMutexAndPump(0u, false);
    pthread_mutex_lock(&keyMutex);
    g_inKeyPoll = 0;
    empty = (keyCount == 0);
    pthread_mutex_unlock(&keyMutex);
  }
  return empty ? 1 : 0;
}

/**
 * Appends a press (1 to 43) or release (0) code to the FIFO queue.
 */
int key_push(int k1) {
  pthread_mutex_lock(&keyMutex);
  // Drop any unmatched release whose press was never enqueued or ran interactively before program execution started.
  if(k1 == 0 && !pushedKeyDown) {
    pthread_mutex_unlock(&keyMutex);
    return 1;
  }
  if(pushLocked(k1)) {
    pushedKeyDown = (k1 > 0);
  }
  pthread_mutex_unlock(&keyMutex);
  return 1;
}

/**
 * Returns the oldest buffered key code without removing it, or -1 when empty.
 */
int key_tail(void) {
  pthread_mutex_lock(&keyMutex);
  const int key = (keyCount == 0 ? -1 : keyBuffer[0]);
  pthread_mutex_unlock(&keyMutex);
  return key;
}

/**
 * Pops one key for keyBuffer_pop when the core is idle; returns -1 while a program, yield, or shutdown is active so screen refreshes cannot
 * drain interrupt or teardown keys into kb_buffer_t.
 */
int keyPopForMainLoop(void) {
  if(shouldQueueInputToDmcp() || isCoreShutdownRequested()) {
    return -1;
  }
  pthread_mutex_lock(&keyMutex);
  const int key = popLocked();
  pthread_mutex_unlock(&keyMutex);
  return key;
}

/**
 * Pops the oldest key code from the FIFO queue, or returns -1 when empty.
 */
int key_pop(void) {
  pthread_mutex_lock(&keyMutex);
  const int key = popLocked();
  pthread_mutex_unlock(&keyMutex);
  return key;
}

/**
 * Clears all queued key codes and resets the pressed-key state.
 */
void key_pop_all(void) {
  pthread_mutex_lock(&keyMutex);
  keyCount = 0;
  pushedKeyDown = false;
  pthread_mutex_unlock(&keyMutex);
}

/**
 * Yields to Android in 10 ms slices until no queued key remains pressed, shutdown is requested, or the timeout expires.
 */
void wait_for_key_release(int tout) {
  if(!isAndroidRuntimeReady() || isCoreShutdownRequested()) {
    applyPendingCoreShutdownOnCoreThread();
    pthread_mutex_lock(&keyMutex);
    pushedKeyDown = false;
    pthread_mutex_unlock(&keyMutex);
    return;
  }

  const uint32_t startMs   = sys_current_ms();
  // Clamp tout <= 0 to 2000 ms so a lost Android touch-cancel UP event cannot block the single core thread indefinitely.
  const uint32_t maxWaitMs = (tout > 0) ? (uint32_t)tout : 2000u;

  for(;;) {
    if(isCoreShutdownRequested()) {
      applyPendingCoreShutdownOnCoreThread();
      pthread_mutex_lock(&keyMutex);
      pushedKeyDown = false;
      pthread_mutex_unlock(&keyMutex);
      break;
    }

    pthread_mutex_lock(&keyMutex);
    const bool_t down = pushedKeyDown;
    pthread_mutex_unlock(&keyMutex);

    if(!down) {
      break;
    }
    if(sys_current_ms() - startMs >= maxWaitMs) {
      pthread_mutex_lock(&keyMutex);
      pushedKeyDown = false;
      pthread_mutex_unlock(&keyMutex);
      break;
    }
    yieldToAndroidWithMs(10);
  }
}

/**
 * Yields to Android up to 20 ms or until the earliest active system timer deadline, clears expired timers, and updates the minute-wake flag.
 */
void sys_sleep(void) {
  if(!isAndroidRuntimeReady() || isCoreShutdownRequested()) {
    applyPendingCoreShutdownOnCoreThread();
    return;
  }

  int sleepMs = 20;
  pthread_mutex_lock(&keyMutex);
  const bool_t hasKeys = (keyCount > 0);
  if(!hasKeys) {
    const gint64 nowUs = g_get_monotonic_time();
    for(int i = 0; i < SYS_TIMERS; i++) {
      if(timerDeadline[i] != 0) {
        const gint64 remMs = (timerDeadline[i] - nowUs + 999LL) / 1000LL;
        if(remMs <= 0) {
          sleepMs = 0;
          break;
        }
        if(remMs < sleepMs) {
          sleepMs = (int)remMs;
        }
      }
    }
  }
  pthread_mutex_unlock(&keyMutex);

  if(!hasKeys && sleepMs > 0 && !isCoreShutdownRequested()) {
    yieldToAndroidWithMs(sleepMs);
  }

  pthread_mutex_lock(&keyMutex);
  const gint64 now = g_get_monotonic_time();
  for(int i = 0; i < SYS_TIMERS; i++) {
    if(timerDeadline[i] != 0 && timerDeadline[i] <= now) {
      timerDeadline[i] = 0;
    }
  }
  pthread_mutex_unlock(&keyMutex);

  if(ST(STAT_CLK_WKUP_ENABLE)) {
    const gint64 minute = (g_get_real_time() / 1000000LL) / 60LL;
    if(minute != lastMinute) {
      if(lastMinute != 0) {
        SET_ST(STAT_CLK_WKUP_FLAG);
      }
      lastMinute = minute;
    }
  }
}

/**
 * Yields to Android in <= 20 ms slices for ms_delay milliseconds while releasing screenMutex and draining core tasks.
 */
void sys_delay(uint32_t ms_delay) {
  if(!isAndroidRuntimeReady() || isCoreShutdownRequested()) {
    applyPendingCoreShutdownOnCoreThread();
    return;
  }
  uint32_t remaining = ms_delay;
  while(remaining > 0u && !isCoreShutdownRequested()) {
    const uint32_t chunk = (remaining > 20u) ? 20u : remaining;
    yieldToAndroidWithMs((int)chunk);
    remaining -= chunk;
  }
}

/**
 * Arms system timer timer_ix to expire after ms_value milliseconds.
 */
void sys_timer_start(int timer_ix, uint32_t ms_value) {
  if(timer_ix < 0 || timer_ix >= SYS_TIMERS) {
    return;
  }
  pthread_mutex_lock(&keyMutex);
  timerDeadline[timer_ix] = g_get_monotonic_time() + (gint64)ms_value * 1000LL;
  pthread_mutex_unlock(&keyMutex);
}

/**
 * Disables system timer timer_ix.
 */
void sys_timer_disable(int timer_ix) {
  if(timer_ix < 0 || timer_ix >= SYS_TIMERS) {
    return;
  }
  pthread_mutex_lock(&keyMutex);
  timerDeadline[timer_ix] = 0;
  pthread_mutex_unlock(&keyMutex);
}

/**
 * No-op on Android where OS screen-timeout policy manages display sleep.
 */
void reset_auto_off(void) {
}

/**
 * No-op on Android where querying the system clock needs no hardware settling delay.
 */
void rtc_wakeup_delay(void) {
}

/**
 * Returns false on Android because host keyboard events are translated in PhysicalKeyboardInput.kt.
 */
bool_t keyIsHostEvent(int key) {
  (void)key;
  return false;
}

/**
 * No-op on Android where no host-event codes are queued.
 */
void keyDispatchHostEvent(int key) {
  (void)key;
}

/**
 * Converts key string data ("00" to "36" or "1" to "6") and pushes a press or release code.
 */
void keyPushKey(const char *data, bool_t press, uint8_t button) {
  (void)button;
  const int code = parseCalculatorKeyId(data);
  if(code <= 0) {
    return;
  }
  key_push(press ? code : 0);
}

/**
 * Pushes a press followed by a release for key string data.
 */
void keyPushClick(const char *data, uint8_t button) {
  keyPushKey(data, true, button);
  keyPushKey(data, false, button);
}

/**
 * No-op on Android where GDK key events are not used.
 */
void keyPushPcEvent(const GdkEventKey *event, bool_t press) {
  (void)event;
  (void)press;
}

/**
 * Resets tracked touch and physical-keyboard gesture states on runtime startup or session re-attachment.
 */
void resetInputGestures(void) {
  touchGesture.mode = KEY_GESTURE_IDLE;
  touchGesture.dmcpCode = 0;
  simGesture.mode = KEY_GESTURE_IDLE;
  simGesture.dmcpCode = 0;
}

static void releaseKeyGestureLocked(keyGestureState_t *g) {
  const keyGestureMode_t prevMode = g->mode;
  g->mode = KEY_GESTURE_IDLE;
  g->dmcpCode = 0;
  if(prevMode == KEY_GESTURE_QUEUED) {
    key_push(0);
  } else if(prevMode == KEY_GESTURE_DIRECT && !shouldQueueInputToDmcp()) {
    processKeyCode(0);
  }
}

/**
 * Routes a press or release event symmetrically for one input source while screenMutex is locked. Presses arriving while the core is busy enqueue into
 * hal/keys.c; direct presses and releases execute processKeyCode so LCD refresh, TO_KB_ACTV timers, and PEM shift cleanup match mainLoop; overlapping rollover
 * presses complete the prior gesture first; and any pending OFF or SYSTEM state is handled.
 */
static void dispatchKeyGestureLocked(keyGestureState_t *g, keyGestureState_t *other, int dmcpCode, bool isRelease) {
  if(!isRelease) {
    releaseKeyGestureLocked(g);
    releaseKeyGestureLocked(other);
    if(dmcpCode < 1 || dmcpCode > 43) {
      (void)handlePendingPowerOffLocked();
      return;
    }
    if(shouldQueueInputToDmcp()) {
      g->mode = KEY_GESTURE_QUEUED;
      g->dmcpCode = dmcpCode;
      key_push(dmcpCode);
      return;
    }
    g->mode = KEY_GESTURE_DIRECT;
    g->dmcpCode = dmcpCode;
    processKeyCode(dmcpCode);
    (void)handlePendingPowerOffLocked();
    return;
  }

  if(dmcpCode > 0 && g->mode != KEY_GESTURE_IDLE && g->dmcpCode != dmcpCode) {
    return;
  }
  releaseKeyGestureLocked(g);
  (void)handlePendingPowerOffLocked();
}

/**
 * Routes a touch-keypad press or release event while screenMutex is locked.
 */
void dispatchTouchKeyGesture(int dmcpCode, bool isRelease) {
  dispatchKeyGestureLocked(&touchGesture, &simGesture, dmcpCode, isRelease);
}

/**
 * Routes a physical-keyboard press or release event while screenMutex is locked.
 */
void dispatchSimKeyGesture(int dmcpCode, bool isRelease) {
  dispatchKeyGestureLocked(&simGesture, &touchGesture, dmcpCode, isRelease);
}

/**
 * Releases any active touch-keypad gesture cleanly while screenMutex is locked.
 */
void releaseTouchKeyGesture(void) {
  releaseKeyGestureLocked(&touchGesture);
  (void)handlePendingPowerOffLocked();
}

/**
 * Moves idle queued keys into kb_buffer_t and dispatches each one via processKeyCode, stopping immediately if OFF or SYSTEM triggers exit.
 */
void keyDispatchNow(void) {
  uint8_t key;
  for(;;) {
    keyBuffer_pop();
    if(outKeyBuffer(&key) != BUFFER_SUCCESS) {
      (void)handlePendingPowerOffLocked();
      return;
    }
    processKeyCode(key);
    lcd_refresh();
    if(handlePendingPowerOffLocked()) {
      return;
    }
  }
}
