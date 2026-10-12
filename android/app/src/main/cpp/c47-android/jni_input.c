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
    } else if (c == 0xE2 && (unsigned char)*(p + 1) == 0x80 && ((unsigned char)*(p + 2) >= 0x80 && (unsigned char)*(p + 2) <= 0x8B)) {
      // UTF-8 U+2000..U+200B (punctuation spaces, thin space, hair space, zero-width space)
      buf[len++] = ' ';
      p += 3;
    } else if (c == 0xC2 && (unsigned char)*(p + 1) == 0xA0) {
      // UTF-8 U+00A0 (non-breaking space)
      buf[len++] = ' ';
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
  char *radTag = strstr(s, ":RAD");
  char *gradTag = strstr(s, ":GRAD");
  if (degTag) {
    strcpy(typeTag, "Real:DEG");
    *degTag = '\0';
  } else if (radTag) {
    strcpy(typeTag, "Real:RAD");
    *radTag = '\0';
  } else if (gradTag) {
    strcpy(typeTag, "Real:GRAD");
    *gradTag = '\0';
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
    } else if (slen >= 1 && (s[slen - 1] == 'r' || s[slen - 1] == 'R')) {
      strcpy(typeTag, "Real:RAD");
      s[slen - 1] = '\0';
    } else if (slen >= 1 && (s[slen - 1] == 'g' || s[slen - 1] == 'G')) {
      strcpy(typeTag, "Real:GRAD");
      s[slen - 1] = '\0';
    } else if (slen >= 1 && (s[slen - 1] == 'd' || s[slen - 1] == 'D')) {
      strcpy(typeTag, "Real:DEG");
      s[slen - 1] = '\0';
    }
  }

  char timeSecBuf[64];
  char dateFmtBuf[64];

  if (strchr(s, ':')) {
    bool_t isNeg = false;
    char *pTime = s;
    if (*pTime == '-') {
      isNeg = true;
      pTime++;
    } else if (*pTime == '+') {
      pTime++;
    }

    char *firstColon = strchr(pTime, ':');
    if (firstColon) {
      char *secondColon = strchr(firstColon + 1, ':');
      char part1[32], part2[32], part3[32];

      size_t l1 = firstColon - pTime;
      if (l1 >= sizeof(part1)) {
        l1 = sizeof(part1) - 1;
      }
      strncpy(part1, pTime, l1);
      part1[l1] = '\0';

      if (secondColon) {
        size_t l2 = secondColon - (firstColon + 1);
        if (l2 >= sizeof(part2)) {
          l2 = sizeof(part2) - 1;
        }
        strncpy(part2, firstColon + 1, l2);
        part2[l2] = '\0';

        strncpy(part3, secondColon + 1, sizeof(part3) - 1);
        part3[sizeof(part3) - 1] = '\0';
      } else {
        strncpy(part2, firstColon + 1, sizeof(part2) - 1);
        part2[sizeof(part2) - 1] = '\0';
        strcpy(part3, "0");
      }

      // Trim any whitespace inside extracted parts and remove internal spaces
      char *t1 = part1;
      while (*t1 == ' ' || *t1 == '\t') {
        t1++;
      }
      char *te1 = t1 + strlen(t1);
      while (te1 > t1 && (*(te1 - 1) == ' ' || *(te1 - 1) == '\t')) {
        *(--te1) = '\0';
      }
      char cleanP1[32], cleanP2[32], cleanP3[32];
      char *w = cleanP1;
      for (char *r = t1; *r; r++) {
        if (*r != ' ' && *r != '\t') {
          *w++ = *r;
        }
      }
      *w = '\0';

      char *t2 = part2;
      while (*t2 == ' ' || *t2 == '\t') {
        t2++;
      }
      char *te2 = t2 + strlen(t2);
      while (te2 > t2 && (*(te2 - 1) == ' ' || *(te2 - 1) == '\t')) {
        *(--te2) = '\0';
      }
      w = cleanP2;
      for (char *r = t2; *r; r++) {
        if (*r != ' ' && *r != '\t') {
          *w++ = *r;
        }
      }
      *w = '\0';

      char *t3 = part3;
      while (*t3 == ' ' || *t3 == '\t') {
        t3++;
      }
      char *te3 = t3 + strlen(t3);
      while (te3 > t3 && (*(te3 - 1) == ' ' || *(te3 - 1) == '\t')) {
        *(--te3) = '\0';
      }
      w = cleanP3;
      for (char *r = t3; *r; r++) {
        if (*r != ' ' && *r != '\t') {
          *w++ = *r;
        }
      }
      *w = '\0';

      real34_t rH, rM, rS, totalSec;
      stringToReal34(cleanP1, &rH);
      stringToReal34(cleanP2, &rM);
      stringToReal34(cleanP3, &rS);

      if (!real34IsNaN(&rH) && !real34IsNaN(&rM) && !real34IsNaN(&rS)) {
        real34Multiply(&rH, const34_3600, &totalSec);
        real34Multiply(&rM, const34_60, &rM);
        real34Add(&totalSec, &rM, &totalSec);
        real34Add(&totalSec, &rS, &totalSec);
        if (isNeg) {
          real34SetNegativeSign(&totalSec);
        }
        real34ToString(&totalSec, timeSecBuf);
        strcpy(typeTag, "Time");
        s = timeSecBuf;
      }
    }
  } else if (strchr(s, '-') || strchr(s, '/')) {
    char sep = strchr(s, '-') ? '-' : '/';
    char *p1 = s;
    char *firstSep = strchr(p1, sep);
    char *secondSep = firstSep ? strchr(firstSep + 1, sep) : NULL;

    if (firstSep && secondSep) {
      char part1[32], part2[32], part3[32];
      size_t l1 = firstSep - p1;
      size_t l2 = secondSep - (firstSep + 1);

      if (l1 < sizeof(part1) && l2 < sizeof(part2)) {
        strncpy(part1, p1, l1);
        part1[l1] = '\0';
        strncpy(part2, firstSep + 1, l2);
        part2[l2] = '\0';
        strncpy(part3, secondSep + 1, sizeof(part3) - 1);
        part3[sizeof(part3) - 1] = '\0';

        uint32_t n1 = (uint32_t)strtoul(part1, NULL, 10);
        uint32_t n2 = (uint32_t)strtoul(part2, NULL, 10);
        uint32_t n3 = (uint32_t)strtoul(part3, NULL, 10);

        if (n1 > 31) {
          // YYYY-MM-DD
          snprintf(dateFmtBuf, sizeof(dateFmtBuf), "%u.%02u%02u", n1, n2, n3);
          strcpy(typeTag, "DYMD");
          s = dateFmtBuf;
        } else if (n3 > 31) {
          if (sep == '/' || getSystemFlag(FLAG_MDY)) {
            // MM/DD/YYYY
            snprintf(dateFmtBuf, sizeof(dateFmtBuf), "%u.%02u%04u", n1, n2, n3);
            strcpy(typeTag, "DMDY");
            s = dateFmtBuf;
          } else {
            // DD.MM.YYYY
            snprintf(dateFmtBuf, sizeof(dateFmtBuf), "%u.%02u%04u", n1, n2, n3);
            strcpy(typeTag, "DDMY");
            s = dateFmtBuf;
          }
        }
      }
    }
  }

  e = s + strlen(s);
  while (e > s && (*(e - 1) == ' ' || *(e - 1) == '\t' || *(e - 1) == '.' || *(e - 1) == ',')) {
    *(--e) = '\0';
  }

  // Sanitize Real numbers: strip any internal spaces (e.g. from digit grouping) and normalize radix
  if (strncmp(typeTag, "Real", 4) == 0) {
    char cleanBuf[256];
    char *r = s;
    char *w = cleanBuf;
    while (*r && (size_t)(w - cleanBuf) < sizeof(cleanBuf) - 1) {
      if (*r != ' ' && *r != '\t') {
        *w++ = *r;
      }
      r++;
    }
    *w = '\0';

    char *lastDot = strrchr(cleanBuf, '.');
    char *lastComma = strrchr(cleanBuf, ',');
    if (lastComma != NULL && lastDot == NULL) {
      // European format with comma as radix mark (e.g. "1234,56")
      for (char *p = cleanBuf; *p; p++) {
        if (*p == ',') {
          *p = '.';
        }
      }
    } else if (lastDot != NULL && lastComma != NULL) {
      if (lastComma > lastDot) {
        // European format with thousand dots and comma radix (e.g. "1.234,56")
        char *src = cleanBuf;
        char *dst = cleanBuf;
        while (*src) {
          if (*src == '.') {
            src++;
          } else if (*src == ',') {
            *dst++ = '.';
            src++;
          } else {
            *dst++ = *src++;
          }
        }
        *dst = '\0';
      } else {
        // Standard format with thousand commas and dot radix (e.g. "1,234.56")
        char *src = cleanBuf;
        char *dst = cleanBuf;
        while (*src) {
          if (*src == ',') {
            src++;
          } else {
            *dst++ = *src++;
          }
        }
        *dst = '\0';
      }
    }

    // Re-trim any trailing radix marks from the cleaned string
    size_t clen = strlen(cleanBuf);
    while (clen > 0 && (cleanBuf[clen - 1] == '.' || cleanBuf[clen - 1] == ',')) {
      cleanBuf[--clen] = '\0';
    }

    strcpy(buf, cleanBuf);
    s = buf;
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