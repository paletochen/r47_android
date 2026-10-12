#include "jni_bridge.h"

#include <stdio.h>
#include <string.h>

extern void btnFnPressed(GtkWidget *notUsed, GdkEvent *event, gpointer data);
extern void btnFnReleased(GtkWidget *notUsed, GdkEvent *event, gpointer data);
extern void btnPressed(GtkWidget *notUsed, GdkEvent *event, gpointer data);
extern void btnReleased(GtkWidget *notUsed, GdkEvent *event, gpointer data);

static char currentPressedKeyStr[4] = {0};
static int currentPressedKeyCode = 0;

JNIEXPORT void JNICALL
Java_org_rpncalculators_r47_MainActivity_sendSimFuncNative(
    JNIEnv *env, jobject thiz, jint funcId) {
  (void)env;
  (void)thiz;
  pthread_mutex_lock(&screenMutex);
  extern void runFunction(int16_t id);
  runFunction((int16_t)funcId);
  pthread_mutex_unlock(&screenMutex);
}

JNIEXPORT void JNICALL
Java_org_rpncalculators_r47_MainActivity_sendSimMenuNative(
    JNIEnv *env, jobject thiz, jint menuId) {
  (void)env;
  (void)thiz;
  pthread_mutex_lock(&screenMutex);
  extern void showSoftmenu(int16_t id);
  showSoftmenu((int16_t)menuId);
  refreshScreen(1);
  pthread_mutex_unlock(&screenMutex);
}

JNIEXPORT void JNICALL
Java_org_rpncalculators_r47_MainActivity_sendSimKeyNative(
    JNIEnv *env, jobject thiz, jstring keyId, jboolean isFn,
    jboolean isRelease) {
  (void)thiz;
  if (!ram || isCoreBlockingForIo) {
    return;
  }

  const char *nativeKeyId = (*env)->GetStringUTFChars(env, keyId, 0);
  if (!nativeKeyId) {
    return;
  }

  pthread_mutex_lock(&screenMutex);
  if (isFn) {
    if (isRelease) {
      extern void btnFnClickedR(void *w, void *data);
      btnFnClickedR(NULL, (void *)nativeKeyId);
    } else {
      extern void btnFnClickedP(void *w, void *data);
      btnFnClickedP(NULL, (void *)nativeKeyId);
    }
  } else {
    if (isRelease) {
      extern void btnClickedR(void *w, void *data);
      btnClickedR(NULL, (void *)nativeKeyId);
    } else {
      extern void btnClickedP(void *w, void *data);
      btnClickedP(NULL, (void *)nativeKeyId);
    }
  }
  pthread_mutex_unlock(&screenMutex);
  (*env)->ReleaseStringUTFChars(env, keyId, nativeKeyId);
}

JNIEXPORT void JNICALL Java_org_rpncalculators_r47_MainActivity_sendKey(
  JNIEnv *env, jobject thiz, jint keyCode) {
  (void)env;
  (void)thiz;
  if (!ram) {
    return;
  }

  char charKey[4];
  onUIActivity();
  if (keyCode > 0) {
    LOGI("sendKey: DOWN %d", keyCode);
    currentPressedKeyCode = keyCode;
    pthread_mutex_lock(&screenMutex);
    if (keyCode >= 38 && keyCode <= 43) {
      sprintf(charKey, "%c", keyCode - 38 + '1');
      strcpy(currentPressedKeyStr, charKey);
      btnFnPressed(NULL, &pressEvent, charKey);
    } else if (keyCode >= 1 && keyCode <= 37) {
      sprintf(charKey, "%02u", keyCode - 1);
      strcpy(currentPressedKeyStr, charKey);
      btnPressed(NULL, &pressEvent, charKey);
    }
    pthread_mutex_unlock(&screenMutex);
    return;
  }

  LOGI("sendKey: UP (last=%d)", currentPressedKeyCode);
  pthread_mutex_lock(&screenMutex);
  if (currentPressedKeyCode >= 38 && currentPressedKeyCode <= 43) {
    btnFnReleased(NULL, &releaseEvent, currentPressedKeyStr);
  } else if (currentPressedKeyCode >= 1 && currentPressedKeyCode <= 37) {
    btnReleased(NULL, &releaseEvent, currentPressedKeyStr);
  }
  pthread_mutex_unlock(&screenMutex);
  currentPressedKeyCode = 0;
  currentPressedKeyStr[0] = 0;
}

JNIEXPORT void JNICALL
Java_org_rpncalculators_r47_MainActivity_pasteTextNative(
    JNIEnv *env, jobject thiz, jstring text) {
  (void)thiz;
  if (!ram || isCoreBlockingForIo) {
    return;
  }

  const char *rawText = (*env)->GetStringUTFChars(env, text, 0);
  if (!rawText) {
    return;
  }

  char buf[512];
  size_t len = 0;
  const char *p = rawText;
  while (*p && len + 1 < sizeof(buf)) {
    unsigned char c = (unsigned char)*p;
    if (c == 0xC2 && (unsigned char)*(p + 1) == 0xB0) {
      buf[len++] = ':';
      buf[len++] = 'D';
      buf[len++] = 'E';
      buf[len++] = 'G';
      p += 2;
    } else if (c == 0x80 && (unsigned char)*(p + 1) == 0xB0) {
      buf[len++] = ':';
      buf[len++] = 'D';
      buf[len++] = 'E';
      buf[len++] = 'G';
      p += 2;
    } else {
      buf[len++] = *p++;
    }
  }
  buf[len] = '\0';
  (*env)->ReleaseStringUTFChars(env, text, rawText);

  char *s = buf;
  while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') {
    s++;
  }
  char *e = s + strlen(s);
  while (e > s && (*(e - 1) == ' ' || *(e - 1) == '\t' || *(e - 1) == '\r' || *(e - 1) == '\n')) {
    *(--e) = '\0';
  }
  if (!*s) {
    return;
  }

  char typeTag[32];
  strcpy(typeTag, "Real");

  char *degTag = strstr(s, ":DEG");
  if (degTag) {
    strcpy(typeTag, "Real:DEG");
    *degTag = '\0';
  } else {
    size_t slen = strlen(s);
    if (slen >= 3 && strcasecmp(s + slen - 3, "deg") == 0) {
      strcpy(typeTag, "Real:DEG");
      s[slen - 3] = '\0';
    } else if (slen >= 3 && strcasecmp(s + slen - 3, "rad") == 0) {
      strcpy(typeTag, "Real:RAD");
      s[slen - 3] = '\0';
    } else if (slen >= 4 && strcasecmp(s + slen - 4, "grad") == 0) {
      strcpy(typeTag, "Real:GRAD");
      s[slen - 4] = '\0';
    }
  }

  e = s + strlen(s);
  while (e > s && (*(e - 1) == ' ' || *(e - 1) == '\t' || *(e - 1) == '.' || *(e - 1) == ',')) {
    *(--e) = '\0';
  }

  pthread_mutex_lock(&screenMutex);
  if (calcMode == CM_NORMAL && getSystemFlag(FLAG_ASLIFT)) {
    liftStack();
  }

  restoreRegister(REGISTER_X, typeTag, s);
  setSystemFlag(FLAG_ASLIFT);

  extern bool_t reDraw;
  extern gboolean refreshLcd(gpointer unusedData);
  reDraw = true;
  screenUpdatingMode = SCRUPD_AUTO;
  refreshScreen(1);
  refreshLcd(NULL);
  lcd_refresh();
  pthread_mutex_unlock(&screenMutex);
}