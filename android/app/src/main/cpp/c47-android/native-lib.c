#include <string.h>
#include <unistd.h>

#include "jni_bridge.h"

JavaVM* g_jvm = NULL;
jobject g_mainActivityObj = NULL;
jmethodID g_requestFileId = NULL;
jmethodID g_playToneId = NULL;
jmethodID g_stopToneId = NULL;
jmethodID g_processCoreTasksId = NULL;
jmethodID g_setBeeperVolumeId = NULL;
jmethodID g_getBeeperVolumeId = NULL;
jmethodID g_getBatteryVoltageId = NULL;
jmethodID g_getStorageInfoId = NULL;

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

uint16_t getBeepVolume(void) {
  if (g_mainActivityObj && g_jvm && g_getBeeperVolumeId) {
    JNIEnv* env;
    if ((*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6) ==
        JNI_EDETACHED) {
      if ((*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL) != JNI_OK) {
        return 2;
      }
    }
    jint pct =
        (*env)->CallIntMethod(env, g_mainActivityObj, g_getBeeperVolumeId);
    int vol = (pct * 11 + 50) / 100;
    if (vol > 11) {
      vol = 11;
    }
    if (vol < 0) {
      vol = 0;
    }
    return (uint16_t)vol;
  }
  return 2;
}

int get_vbat(void) {
  if (g_mainActivityObj && g_jvm && g_getBatteryVoltageId) {
    JNIEnv* env;
    if ((*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6) ==
        JNI_EDETACHED) {
      if ((*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL) != JNI_OK) {
        return 3800;
      }
    }
    return (*env)->CallIntMethod(env, g_mainActivityObj, g_getBatteryVoltageId);
  }
  return 3800;
}

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
  if (getSystemFlag(FLAG_QUIET)) {
    return;
  }

  if (ms_delay > 0) {
    if (ms_delay > 2000) {
      ms_delay = 2000;
    }
    if (frequency != 0) {
      if (frequency > 20000) {
        frequency = 20000;
      }
      if (g_mainActivityObj && g_jvm && g_playToneId) {
        JNIEnv* env;
        if ((*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6) ==
            JNI_EDETACHED) {
          if ((*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL) != JNI_OK) {
            return;
          }
        }
        (*env)->CallVoidMethod(env, g_mainActivityObj, g_playToneId,
                               (jint)(frequency * 1000), (jint)ms_delay);
      }
    }
    yieldToAndroidWithMs((int)ms_delay);
  }
}

void audioTone(uint32_t frequency) {
  if (getSystemFlag(FLAG_QUIET)) {
    return;
  }

  if (g_mainActivityObj && g_jvm && g_playToneId) {
    JNIEnv* env;
    if ((*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6) ==
        JNI_EDETACHED) {
      if ((*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL) != JNI_OK) {
        return;
      }
    }
    (*env)->CallVoidMethod(env, g_mainActivityObj, g_playToneId,
                           (jint)frequency, (jint)200);
  }

  yieldToAndroidWithMs(210);
}

void processCoreTasksNative(void) {
  if (!g_mainActivityObj || !g_jvm || !g_processCoreTasksId) {
    return;
  }

  JNIEnv* env;
  if ((*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6) == JNI_EDETACHED) {
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

  uint32_t now = sys_current_ms();
  if (nextTimerRefresh <= now) {
    refreshTimer(NULL);
    nextTimerRefresh = now + 5;
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

void fnSetVolume(uint16_t volume) {
  if (volume > 11) {
    volume = 11;
  }
  if (g_mainActivityObj && g_jvm && g_setBeeperVolumeId) {
    JNIEnv* env;
    if ((*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6) ==
        JNI_EDETACHED) {
      if ((*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL) != JNI_OK) {
        return;
      }
    }
    jint pct = (volume * 100) / 11;
    (*env)->CallVoidMethod(env, g_mainActivityObj, g_setBeeperVolumeId, pct);
  }
}

void fnGetVolume(uint16_t unusedButMandatoryParameter) {
  (void)unusedButMandatoryParameter;
  longInteger_t volume;
  liftStack();
  longIntegerInit(volume);
  int32ToLongInteger((int32_t)getBeepVolume(), volume);
  convertLongIntegerToLongIntegerRegister(volume, REGISTER_X);
  longIntegerFree(volume);
}

void fnVolumeUp(uint16_t unusedButMandatoryParameter) {
  (void)unusedButMandatoryParameter;
  uint16_t vol = getBeepVolume();
  if (vol < 11) {
    vol++;
    fnSetVolume(vol);
  }
  audioTone(440000);
}

void fnVolumeDown(uint16_t unusedButMandatoryParameter) {
  (void)unusedButMandatoryParameter;
  uint16_t vol = getBeepVolume();
  if (vol > 0) {
    vol--;
    fnSetVolume(vol);
  }
  audioTone(440000);
}

static uint32_t _getValueFromRegister(calcRegister_t regist) {
  uint32_t value = 0;
  if (getRegisterDataType(regist) == dtReal34) {
    value = real34ToUInt32(REGISTER_REAL34_DATA(regist));
  } else if (getRegisterDataType(regist) == dtLongInteger) {
    longInteger_t lgInt;
    convertLongIntegerRegisterToLongInteger(regist, lgInt);
    longIntegerToUInt32(lgInt, value);
    longIntegerFree(lgInt);
  } else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE,
                            REGISTER_X);
    return (uint32_t)-1;
  }
  return value;
}

void fnBuzz(uint16_t unusedButMandatoryParameter) {
  (void)unusedButMandatoryParameter;
  if (!getSystemFlag(FLAG_QUIET)) {
    uint32_t frequency = _getValueFromRegister(REGISTER_Y);
    uint32_t ms_delay = _getValueFromRegister(REGISTER_X);
    _Buzz(frequency, ms_delay);
  }
}

void fnPlay(uint16_t regist) {
  if (getRegisterDataType(regist) == dtReal34Matrix) {
    real34Matrix_t m;
    if (!getSystemFlag(FLAG_QUIET)) {
      linkToRealMatrixRegister(regist, &m);
      uint16_t cols = m.header.matrixColumns;
      if ((cols != 2) && (cols != 3)) {
        displayCalcErrorMessage(ERROR_MATRIX_MISMATCH, ERR_REGISTER_LINE,
                                REGISTER_X);
        return;
      }
      screenUpdatingMode = SCRUPD_AUTO;
      screenUpdatingMode |= SCRUPD_SKIP_STATUSBAR_ONE_TIME;
      for (uint16_t i = 0; i < m.header.matrixRows; ++i) {
        uint32_t frequency = real34ToUInt32(&m.matrixElements[i * cols]);
        uint32_t ms_delay = real34ToUInt32(&m.matrixElements[i * cols + 1]);
        _Buzz(frequency, ms_delay);
        if (ms_delay > 0) {
          yieldToAndroidWithMs((int)(ms_delay / 8));
        }
        if (exitKeyWaiting()) {
          return;
        }
      }
    }
  } else {
    displayCalcErrorMessage(ERROR_INVALID_DATA_TYPE_FOR_OP, ERR_REGISTER_LINE,
                            NIM_REGISTER_LINE);
  }
}

void squeak(void) { _Buzz(1000, 10); }

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
  (void)reserved;
  g_jvm = vm;

  JNIEnv* env;
  if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK) {
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

void releaseNativeActivityReferences(JNIEnv* env) {
  if (g_mainActivityObj != NULL) {
    (*env)->DeleteGlobalRef(env, g_mainActivityObj);
    g_mainActivityObj = NULL;
  }

  g_requestFileId = NULL;
  g_playToneId = NULL;
  g_stopToneId = NULL;
  g_processCoreTasksId = NULL;
  g_setBeeperVolumeId = NULL;
  g_getBeeperVolumeId = NULL;
  g_getBatteryVoltageId = NULL;
  g_getStorageInfoId = NULL;
}

JNIEXPORT void JNICALL
Java_org_rpncalculators_r47_MainActivity_releaseNativeRuntime(JNIEnv* env,
                                                              jobject thiz) {
  (void)thiz;
  releaseNativeActivityReferences(env);
}

int register_main_activity_natives(JNIEnv* env) {
  static const JNINativeMethod methods[] = {
      {"updateNativeActivityRef", "()V",
       (void*)Java_org_rpncalculators_r47_MainActivity_updateNativeActivityRef},
      {"nativePreInit", "(Ljava/lang/String;)V",
       (void*)Java_org_rpncalculators_r47_MainActivity_nativePreInit},
      {"initNative", "(Ljava/lang/String;I)V",
       (void*)Java_org_rpncalculators_r47_MainActivity_initNative},
      {"tick", "()V", (void*)Java_org_rpncalculators_r47_MainActivity_tick},
      {"releaseNativeRuntime", "()V",
       (void*)Java_org_rpncalculators_r47_MainActivity_releaseNativeRuntime},
      {"sendKey", "(I)V",
       (void*)Java_org_rpncalculators_r47_MainActivity_sendKey},
      {"sendSimKeyNative", "(Ljava/lang/String;ZZ)V",
       (void*)Java_org_rpncalculators_r47_MainActivity_sendSimKeyNative},
      {"sendSimMenuNative", "(I)V",
       (void*)Java_org_rpncalculators_r47_MainActivity_sendSimMenuNative},
      {"sendSimFuncNative", "(I)V",
       (void*)Java_org_rpncalculators_r47_MainActivity_sendSimFuncNative},
      {"saveStateNative", "()V",
       (void*)Java_org_rpncalculators_r47_MainActivity_saveStateNative},
      {"loadStateNative", "()V",
       (void*)Java_org_rpncalculators_r47_MainActivity_loadStateNative},
      {"forceRefreshNative", "()V",
       (void*)Java_org_rpncalculators_r47_MainActivity_forceRefreshNative},
      {"setSlotNative", "(I)V",
       (void*)Java_org_rpncalculators_r47_MainActivity_setSlotNative},
      {"getXRegisterNative", "()Ljava/lang/String;",
       (void*)Java_org_rpncalculators_r47_MainActivity_getXRegisterNative},
      {"getDisplayPixels", "([I)V",
       (void*)Java_org_rpncalculators_r47_MainActivity_getDisplayPixels},
      {"setLcdColors", "(II)V",
       (void*)Java_org_rpncalculators_r47_MainActivity_setLcdColors},
      {"getButtonLabelNative", "(IIZ)Ljava/lang/String;",
       (void*)Java_org_rpncalculators_r47_MainActivity_getButtonLabelNative},
      {"getSoftkeyLabelNative", "(I)Ljava/lang/String;",
       (void*)Java_org_rpncalculators_r47_MainActivity_getSoftkeyLabelNative},
      {"getKeyboardStateNative", "()[I",
       (void*)Java_org_rpncalculators_r47_MainActivity_getKeyboardStateNative},
      {"getKeypadMetaNative", "(ZZ)[I",
       (void*)Java_org_rpncalculators_r47_MainActivity_getKeypadMetaNative},
      {"getKeypadLabelsNative", "(ZZ)[Ljava/lang/String;",
       (void*)Java_org_rpncalculators_r47_MainActivity_getKeypadLabelsNative},
      {"onFileSelectedNative", "(I)V",
       (void*)Java_org_rpncalculators_r47_MainActivity_onFileSelectedNative},
      {"onFileCancelledNative", "()V",
       (void*)Java_org_rpncalculators_r47_MainActivity_onFileCancelledNative},
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