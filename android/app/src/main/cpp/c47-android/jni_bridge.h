#ifndef JNI_BRIDGE_H
#define JNI_BRIDGE_H

#include "c47.h"
#include <android/log.h>
#include <jni.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#define LOG_TAG "R47Native"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

#define MAIN_ACTIVITY_CLASS "com/example/r47/MainActivity"

extern JavaVM *g_jvm;
extern jobject g_mainActivityObj;
extern jmethodID g_requestFileId;
extern jmethodID g_playToneId;
extern jmethodID g_stopToneId;
extern jmethodID g_processCoreTasksId;

extern pthread_mutex_t fileMutex;
extern pthread_cond_t fileCond;
extern pthread_mutex_t screenMutex;
extern int fileDescriptor;
extern bool fileReady;
extern bool fileCancelled;
extern bool isCoreBlockingForIo;

extern gboolean ui_is_active;

extern uint32_t nextTimerRefresh;
extern uint32_t nextScreenRefresh;

extern GdkEvent pressEvent;
extern GdkEvent releaseEvent;

extern void set_android_base_path(const char *path);
extern void init_lcd_buffers(void);
extern void setupUI(void);
extern void lcd_clear_buf(void);
extern void lcd_refresh(void);
extern void refreshScreen(uint16_t reason);

extern void JNICALL Java_com_example_r47_MainActivity_setLcdColors(
    JNIEnv *env, jobject thiz, jint text, jint bg);

void onUIActivity(void);
gint64 g_get_monotonic_time(void);
gint64 g_get_real_time(void);
uint32_t sys_current_ms(void);
void processCoreTasksNative(void);
void yieldToAndroidWithMs(int ms);
void yieldToAndroid(void);
int requestAndroidFile(int isSave, const char *defaultName, int fileType);
void triggerQuit(void);
int register_main_activity_natives(JNIEnv *env);

JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_updateNativeActivityRef(JNIEnv *env,
                                                                 jobject thiz);
JNIEXPORT void JNICALL Java_com_example_r47_MainActivity_nativePreInit(
    JNIEnv *env, jobject thiz, jstring path_obj);
JNIEXPORT void JNICALL Java_com_example_r47_MainActivity_initNative(
    JNIEnv *env, jobject thiz, jstring pathObj, jint slotId);
JNIEXPORT void JNICALL Java_com_example_r47_MainActivity_tick(
    JNIEnv *env, jobject thiz);
JNIEXPORT void JNICALL Java_com_example_r47_MainActivity_sendKey(
    JNIEnv *env, jobject thiz, jint keyCode);
JNIEXPORT void JNICALL Java_com_example_r47_MainActivity_sendSimKeyNative(
    JNIEnv *env, jobject thiz, jstring keyId, jboolean isFn,
    jboolean isRelease);
JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_sendSimMenuNative(JNIEnv *env,
                                                           jobject thiz,
                                                           jint menuId);
JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_sendSimFuncNative(JNIEnv *env,
                                                           jobject thiz,
                                                           jint funcId);
JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_saveStateNative(JNIEnv *env,
                                                         jobject thiz);
JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_loadStateNative(JNIEnv *env,
                                                         jobject thiz);
JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_forceRefreshNative(JNIEnv *env,
                                                            jobject thiz);
JNIEXPORT void JNICALL Java_com_example_r47_MainActivity_setSlotNative(
    JNIEnv *env, jobject thiz, jint slot);
JNIEXPORT jstring JNICALL
Java_com_example_r47_MainActivity_getXRegisterNative(JNIEnv *env,
                                                            jobject thiz);
JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_getDisplayPixels(JNIEnv *env,
                                                          jobject thiz,
                                                          jintArray pixels);
JNIEXPORT jstring JNICALL
Java_com_example_r47_MainActivity_getButtonLabelNative(JNIEnv *env,
                                                              jobject thiz,
                                                              jint keyCode,
                                                              jint type,
                                                              jboolean isDynamic);
JNIEXPORT jstring JNICALL
Java_com_example_r47_MainActivity_getSoftkeyLabelNative(JNIEnv *env,
                                                               jobject thiz,
                                                               jint fnKeyIndex);
JNIEXPORT jintArray JNICALL
Java_com_example_r47_MainActivity_getKeyboardStateNative(JNIEnv *env,
                                                                jobject thiz);
JNIEXPORT jintArray JNICALL
Java_com_example_r47_MainActivity_getKeypadMetaNative(JNIEnv *env,
                                                             jobject thiz,
                                                             jboolean isDynamic);
JNIEXPORT jobjectArray JNICALL
Java_com_example_r47_MainActivity_getKeypadLabelsNative(JNIEnv *env,
                                                               jobject thiz,
                                                               jboolean isDynamic);
JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_onFileSelectedNative(JNIEnv *env,
                                                              jobject thiz,
                                                              jint fd);
JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_onFileCancelledNative(JNIEnv *env,
                                                               jobject thiz);

#endif