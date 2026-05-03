#include "jni_bridge.h"

#include "saveRestoreCalcState.h"

#include <stdlib.h>
#include <string.h>

extern void refreshFn(uint16_t timerType);
extern void execFnTimeout(uint16_t timerType);
extern void shiftCutoff(uint16_t timerType);
extern void fnTimerDummy1(uint16_t timerType);
extern void execTimerApp(uint16_t timerType);

JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_updateNativeActivityRef(JNIEnv *env,
                                                                 jobject thiz) {
  LOGI("updateNativeActivityRef called");

  if (g_mainActivityObj) {
    (*env)->DeleteGlobalRef(env, g_mainActivityObj);
  }
  g_mainActivityObj = (*env)->NewGlobalRef(env, thiz);

  jclass clazz = (*env)->GetObjectClass(env, thiz);
  g_requestFileId =
      (*env)->GetMethodID(env, clazz, "requestFile", "(ZLjava/lang/String;I)V");
  g_playToneId = (*env)->GetMethodID(env, clazz, "playTone", "(II)V");
  g_stopToneId = (*env)->GetMethodID(env, clazz, "stopTone", "()V");
  g_processCoreTasksId =
      (*env)->GetMethodID(env, clazz, "processCoreTasks", "()V");
  (*env)->DeleteLocalRef(env, clazz);

  if (!ram) {
    return;
  }

  pthread_mutex_lock(&screenMutex);
  reDraw = true;
  refreshScreen(190);
  refreshLcd(NULL);
  lcd_refresh();
  pthread_mutex_unlock(&screenMutex);
}

JNIEXPORT void JNICALL Java_com_example_r47_MainActivity_nativePreInit(
    JNIEnv *env, jobject thiz, jstring path_obj) {
  (void)thiz;
  const char *path = (*env)->GetStringUTFChars(env, path_obj, 0);
  set_android_base_path(path);
  (*env)->ReleaseStringUTFChars(env, path_obj, path);

  extern void mp_set_memory_functions(
      void *(*alloc_func)(size_t),
      void *(*realloc_func)(void *, size_t, size_t),
      void (*free_func)(void *, size_t));
  extern void *allocGmp(size_t size);
  extern void *reallocGmp(void *ptr, size_t old_size, size_t new_size);
  extern void freeGmp(void *ptr, size_t size);

  mp_set_memory_functions(allocGmp, reallocGmp, freeGmp);
}

JNIEXPORT void JNICALL Java_com_example_r47_MainActivity_initNative(
    JNIEnv *env, jobject thiz, jstring pathObj, jint slotId) {
  (void)pathObj;
  Java_com_example_r47_MainActivity_updateNativeActivityRef(env, thiz);

  extern int current_slot_id;
  current_slot_id = slotId;

  setupUI();
  init_lcd_buffers();
  lcd_clear_buf();
  calcModel = USER_R47f_g;
  void doFnReset(uint16_t confirmation, bool_t autoSav);
  doFnReset(CONFIRMED, false);
  extern void restoreCalc(void);
  restoreCalc();
  fnRefreshState();
  nextScreenRefresh = sys_current_ms();
  nextTimerRefresh = sys_current_ms();
  fnTimerReset();
  fnTimerConfig(TO_FG_LONG, refreshFn, TO_FG_LONG);
  fnTimerConfig(TO_CL_LONG, refreshFn, TO_CL_LONG);
  fnTimerConfig(TO_FG_TIMR, refreshFn, TO_FG_TIMR);
  fnTimerConfig(TO_FN_LONG, refreshFn, TO_FN_LONG);
  fnTimerConfig(TO_FN_EXEC, execFnTimeout, 0);
  fnTimerConfig(TO_3S_CTFF, shiftCutoff, TO_3S_CTFF);
  fnTimerConfig(TO_CL_DROP, fnTimerDummy1, TO_CL_DROP);
  fnTimerConfig(TO_TIMER_APP, execTimerApp, 0);
  fnTimerConfig(TO_ASM_ACTIVE, refreshFn, TO_ASM_ACTIVE);

  pthread_mutex_lock(&screenMutex);
  reDraw = true;
  refreshScreen(190);
  refreshLcd(NULL);
  lcd_refresh();
  pthread_mutex_unlock(&screenMutex);
}

JNIEXPORT void JNICALL Java_com_example_r47_MainActivity_tick(
  JNIEnv *env, jobject thiz) {
  (void)env;
  (void)thiz;
  uint32_t now = sys_current_ms();
  if (pthread_mutex_trylock(&screenMutex) != 0) {
    return;
  }

  if (nextTimerRefresh <= now) {
    refreshTimer(NULL);
    nextTimerRefresh = now + 5;
  }
  if (nextScreenRefresh <= now) {
    refreshLcd(NULL);
    lcd_refresh();
    nextScreenRefresh = now + 100;
  }

  pthread_mutex_unlock(&screenMutex);
}

JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_saveStateNative(JNIEnv *env,
                                                         jobject thiz) {
  (void)env;
  (void)thiz;
  LOGI("saveStateNative triggered");
  if (!ram) {
    return;
  }

  pthread_mutex_lock(&screenMutex);

  size_t bufferSize = SCREEN_HEIGHT * 52;
  uint8_t *pixelBackup = malloc(bufferSize);
  if (pixelBackup) {
    memcpy(pixelBackup, lcd_buffer, bufferSize);
  }

  printStatus(0, errorMessages[101], 1);
  lcd_refresh();

  if (pixelBackup) {
    memcpy(lcd_buffer, pixelBackup, bufferSize);
    free(pixelBackup);
    for (int row = 0; row < SCREEN_HEIGHT; row++) {
      lcd_buffer[row * 52] = 1u;
    }
  }

  saveCalc();

  pthread_mutex_unlock(&screenMutex);
  LOGI("saveStateNative: Save complete.");
}

JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_loadStateNative(JNIEnv *env,
                                                         jobject thiz) {
  (void)env;
  (void)thiz;
  LOGI("loadStateNative triggered");
  if (!ram) {
    return;
  }

  pthread_mutex_lock(&screenMutex);
  extern void restoreCalc(void);
  restoreCalc();
  extern void scanLabelsAndPrograms(void);
  extern void defineCurrentProgramFromGlobalStepNumber(int16_t globalStepNumber);
  extern void defineCurrentStep(void);
  extern void defineFirstDisplayedStep(void);
  extern void defineCurrentProgramFromCurrentStep(void);
  extern void updateMatrixHeightCache(void);
  extern uint16_t currentLocalStepNumber;
  extern uint16_t currentProgramNumber;
  extern programList_t *programList;
  scanLabelsAndPrograms();
  if (currentProgramNumber > 0) {
    defineCurrentProgramFromGlobalStepNumber(
        currentLocalStepNumber +
        abs(programList[currentProgramNumber - 1].step) - 1);
  }
  defineCurrentStep();
  defineFirstDisplayedStep();
  defineCurrentProgramFromCurrentStep();
  updateMatrixHeightCache();
  refreshScreen(190);
  refreshLcd(NULL);
  lcd_refresh();
  pthread_mutex_unlock(&screenMutex);
}

JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_forceRefreshNative(
    JNIEnv *env, jobject thiz) {
  (void)env;
  (void)thiz;
  if (!ram) {
    return;
  }

  pthread_mutex_lock(&screenMutex);
  refreshScreen(190);
  refreshLcd(NULL);
  lcd_refresh();
  pthread_mutex_unlock(&screenMutex);
}

JNIEXPORT void JNICALL Java_com_example_r47_MainActivity_setSlotNative(
    JNIEnv *env, jobject thiz, jint slot) {
  (void)env;
  (void)thiz;
  extern int current_slot_id;
  current_slot_id = slot;
}