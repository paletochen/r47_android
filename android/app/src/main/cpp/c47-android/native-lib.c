#include "jni_bridge.h"

#include <string.h>
#include <unistd.h>

JavaVM *g_jvm = NULL;
jobject g_mainActivityObj = NULL;
jmethodID g_requestFileId = NULL;
jmethodID g_playToneId = NULL;
jmethodID g_stopToneId = NULL;
jmethodID g_processCoreTasksId = NULL;

pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t fileCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t screenMutex;
int fileDescriptor = -1;
bool fileReady = false;
bool fileCancelled = false;
bool isCoreBlockingForIo = false;

int16_t debugWindow = 0;
int16_t screenStride = 400;
bool_t screenChange = FALSE;
int currentBezel = 0;
calcKeyboard_t calcKeyboard[43];
gboolean ui_is_active = FALSE;

uint32_t nextTimerRefresh = 0;
uint32_t nextScreenRefresh = 0;

GdkEvent pressEvent;
GdkEvent releaseEvent;

uint16_t getBeepVolume(void) { return 80; }

void onUIActivity(void) { ui_is_active = TRUE; }

gint64 g_get_monotonic_time(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (gint64)ts.tv_sec * 1000000LL + ts.tv_nsec / 1000LL;
}

gint64 g_get_real_time(void) {
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return (gint64)ts.tv_sec * 1000000LL + ts.tv_nsec / 1000LL;
}

uint32_t sys_current_ms(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

void _Buzz(uint32_t frequency, uint32_t ms_delay) {
  if (!g_mainActivityObj || !g_jvm || !g_playToneId) {
    return;
  }

  if (frequency > 0) {
    JNIEnv *env;
    if ((*g_jvm)->GetEnv(g_jvm, (void **)&env, JNI_VERSION_1_6) ==
        JNI_EDETACHED) {
      if ((*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL) != JNI_OK) {
        return;
      }
    }
    (*env)->CallVoidMethod(env, g_mainActivityObj, g_playToneId,
                           (jint)frequency, (jint)ms_delay);
  }

  usleep((ms_delay + 10) * 1000);
}

void audioTone(uint32_t frequency) { _Buzz(frequency, 200); }

void processCoreTasksNative(void) {
  if (!g_mainActivityObj || !g_jvm || !g_processCoreTasksId) {
    return;
  }

  JNIEnv *env;
  if ((*g_jvm)->GetEnv(g_jvm, (void **)&env, JNI_VERSION_1_6) ==
      JNI_EDETACHED) {
    if ((*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL) != JNI_OK) {
      return;
    }
  }
  (*env)->CallVoidMethod(env, g_mainActivityObj, g_processCoreTasksId);
}

void yieldToAndroidWithMs(int ms) {
  if (!ram) {
    return;
  }

  refreshLcd(NULL);
  lcd_refresh();

  int lockCount = 0;
  while (pthread_mutex_unlock(&screenMutex) == 0) {
    lockCount++;
  }

  processCoreTasksNative();
  if (ms > 0) {
    usleep(ms * 1000);
  } else {
    usleep(1000);
  }

  while (lockCount > 0) {
    pthread_mutex_lock(&screenMutex);
    lockCount--;
  }
}

void yieldToAndroid(void) { yieldToAndroidWithMs(1); }

void fnSetVolume(uint16_t v) { (void)v; }
void fnGetVolume(uint16_t v) { (void)v; }
void fnVolumeUp(uint16_t v) { (void)v; }
void fnVolumeDown(uint16_t v) { (void)v; }
void fnBuzz(uint16_t v) { (void)v; }
void fnPlay(uint16_t v) { (void)v; }
void squeak(void) { _Buzz(1000, 10); }

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  (void)reserved;
  g_jvm = vm;

  JNIEnv *env;
  if ((*vm)->GetEnv(vm, (void **)&env, JNI_VERSION_1_6) != JNI_OK) {
    return JNI_ERR;
  }

  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&screenMutex, &attr);
  pthread_mutexattr_destroy(&attr);

  memset(&pressEvent, 0, sizeof(GdkEvent));
  pressEvent.type = GDK_BUTTON_PRESS;
  pressEvent.button.button = 1;

  memset(&releaseEvent, 0, sizeof(GdkEvent));
  releaseEvent.type = GDK_BUTTON_RELEASE;
  releaseEvent.button.button = 1;

  if (register_main_activity_natives(env) != JNI_OK) {
    return JNI_ERR;
  }

  return JNI_VERSION_1_6;
}

int register_main_activity_natives(JNIEnv *env) {
  static const JNINativeMethod methods[] = {
      {"updateNativeActivityRef", "()V",
        (void *)Java_com_example_r47_MainActivity_updateNativeActivityRef},
      {"nativePreInit", "(Ljava/lang/String;)V",
        (void *)Java_com_example_r47_MainActivity_nativePreInit},
      {"initNative", "(Ljava/lang/String;I)V",
        (void *)Java_com_example_r47_MainActivity_initNative},
       {"tick", "()V", (void *)Java_com_example_r47_MainActivity_tick},
       {"sendKey", "(I)V", (void *)Java_com_example_r47_MainActivity_sendKey},
      {"sendSimKeyNative", "(Ljava/lang/String;ZZ)V",
        (void *)Java_com_example_r47_MainActivity_sendSimKeyNative},
      {"sendSimMenuNative", "(I)V",
        (void *)Java_com_example_r47_MainActivity_sendSimMenuNative},
      {"sendSimFuncNative", "(I)V",
        (void *)Java_com_example_r47_MainActivity_sendSimFuncNative},
      {"saveStateNative", "()V",
        (void *)Java_com_example_r47_MainActivity_saveStateNative},
      {"loadStateNative", "()V",
        (void *)Java_com_example_r47_MainActivity_loadStateNative},
      {"forceRefreshNative", "()V",
        (void *)Java_com_example_r47_MainActivity_forceRefreshNative},
      {"setSlotNative", "(I)V",
        (void *)Java_com_example_r47_MainActivity_setSlotNative},
      {"getXRegisterNative", "()Ljava/lang/String;",
        (void *)Java_com_example_r47_MainActivity_getXRegisterNative},
      {"getDisplayPixels", "([I)V",
        (void *)Java_com_example_r47_MainActivity_getDisplayPixels},
      {"setLcdColors", "(II)V",
        (void *)Java_com_example_r47_MainActivity_setLcdColors},
      {"getButtonLabelNative", "(IIZ)Ljava/lang/String;",
        (void *)Java_com_example_r47_MainActivity_getButtonLabelNative},
      {"getSoftkeyLabelNative", "(I)Ljava/lang/String;",
        (void *)Java_com_example_r47_MainActivity_getSoftkeyLabelNative},
      {"getKeyboardStateNative", "()[I",
        (void *)Java_com_example_r47_MainActivity_getKeyboardStateNative},
      {"getKeypadMetaNative", "(Z)[I",
        (void *)Java_com_example_r47_MainActivity_getKeypadMetaNative},
      {"getKeypadLabelsNative", "(Z)[Ljava/lang/String;",
        (void *)Java_com_example_r47_MainActivity_getKeypadLabelsNative},
      {"onFileSelectedNative", "(I)V",
        (void *)Java_com_example_r47_MainActivity_onFileSelectedNative},
      {"onFileCancelledNative", "()V",
        (void *)Java_com_example_r47_MainActivity_onFileCancelledNative},
  };

  jclass clazz = (*env)->FindClass(env, MAIN_ACTIVITY_CLASS);
  if (clazz == NULL) {
    LOGE("Failed to find %s for RegisterNatives", MAIN_ACTIVITY_CLASS);
    return JNI_ERR;
  }

  if ((*env)->RegisterNatives(env, clazz, methods,
                              sizeof(methods) / sizeof(methods[0])) != 0) {
    LOGE("RegisterNatives failed for %s", MAIN_ACTIVITY_CLASS);
    (*env)->DeleteLocalRef(env, clazz);
    return JNI_ERR;
  }

  (*env)->DeleteLocalRef(env, clazz);
  return JNI_OK;
}