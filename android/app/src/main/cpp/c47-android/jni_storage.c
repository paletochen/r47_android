#include "jni_bridge.h"

#include <string.h>

static int restoreScreenMutexAndReturn(int lockCount, int fd) {
  LOGI("requestAndroidFile: Re-acquiring screenMutex (%d times)", lockCount);
  for (int index = 0; index < lockCount; index++) {
    pthread_mutex_lock(&screenMutex);
  }

  return fd;
}

static int failAndroidFileRequest(int lockCount) {
  pthread_mutex_lock(&fileMutex);
  isCoreBlockingForIo = false;
  fileReady = false;
  fileCancelled = true;
  fileDescriptor = -1;
  pthread_mutex_unlock(&fileMutex);

  return restoreScreenMutexAndReturn(lockCount, -1);
}

int requestAndroidFile(int isSave, const char *defaultName, int fileType) {
  LOGI("requestAndroidFile(isSave=%d, defaultName=%s, fileType=%d)", isSave,
       defaultName, fileType);
  if (!g_requestFileId || !g_mainActivityObj) {
    LOGE("requestAndroidFile: requestFile bridge is not ready");
    return -1;
  }

  JNIEnv *env = NULL;
  if ((*g_jvm)->GetEnv(g_jvm, (void **)&env, JNI_VERSION_1_6) != JNI_OK) {
    jint attachResult = (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);
    if (attachResult != JNI_OK || !env) {
      LOGE("requestAndroidFile: AttachCurrentThread failed (%d)", attachResult);
      return -1;
    }
  }

  int lockCount = 0;
  while (pthread_mutex_unlock(&screenMutex) == 0) {
    lockCount++;
  }
  LOGI("requestAndroidFile: Fully released screenMutex (lockCount=%d)",
       lockCount);

  pthread_mutex_lock(&fileMutex);
  isCoreBlockingForIo = true;
  fileReady = false;
  fileCancelled = false;
  fileDescriptor = -1;
  pthread_mutex_unlock(&fileMutex);

  jstring nameObj = (*env)->NewStringUTF(env, defaultName ? defaultName : "");
  if ((*env)->ExceptionCheck(env)) {
    LOGE("requestAndroidFile: NewStringUTF threw before request dispatch");
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
    return failAndroidFileRequest(lockCount);
  }

  (*env)->CallVoidMethod(env, g_mainActivityObj, g_requestFileId,
                         (jboolean)isSave, nameObj, (jint)fileType);
  if (nameObj) {
    (*env)->DeleteLocalRef(env, nameObj);
  }

  if ((*env)->ExceptionCheck(env)) {
    LOGE("requestAndroidFile: requestFile threw before a file result was posted");
    (*env)->ExceptionDescribe(env);
    (*env)->ExceptionClear(env);
    return failAndroidFileRequest(lockCount);
  }

  pthread_mutex_lock(&fileMutex);
  LOGI("requestAndroidFile: Waiting for file result...");
  while (!fileReady && !fileCancelled) {
    pthread_cond_wait(&fileCond, &fileMutex);
  }
  int fd = fileDescriptor;
  isCoreBlockingForIo = false;
  LOGI("requestAndroidFile: Resumed, fd=%d, cancelled=%d", fd, fileCancelled);
  pthread_mutex_unlock(&fileMutex);

  return restoreScreenMutexAndReturn(lockCount, fd);
}

JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_onFileSelectedNative(
    JNIEnv *env, jobject thiz, jint fd) {
  (void)env;
  (void)thiz;
  LOGI("onFileSelectedNative(fd=%d)", fd);
  pthread_mutex_lock(&fileMutex);
  fileDescriptor = fd;
  fileReady = true;
  pthread_cond_signal(&fileCond);
  pthread_mutex_unlock(&fileMutex);
}

JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_onFileCancelledNative(
    JNIEnv *env, jobject thiz) {
  (void)env;
  (void)thiz;
  LOGI("onFileCancelledNative()");
  pthread_mutex_lock(&fileMutex);
  fileCancelled = true;
  pthread_cond_signal(&fileCond);
  pthread_mutex_unlock(&fileMutex);
}