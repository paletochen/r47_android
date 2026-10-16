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

static bool_t tryPasteMatrix(const char *str) {
  if(!str || !*str) {
    return false;
  }

  const char *p = str;
  while(*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') {
    p++;
  }

  // Check for optional register prefix e.g. "X = ", "x = ", "R01 = ", "RegX = "
  const char *lineEnd = strpbrk(p, "\r\n");
  const char *eq = strchr(p, '=');
  if(eq != NULL && (lineEnd == NULL || eq < lineEnd)) {
    p = eq + 1;
    while(*p == ' ' || *p == '\t') {
      p++;
    }
  }

  // Check for optional '[' e.g. "[2x2]"
  if(*p == '[') {
    p++;
    while(*p == ' ' || *p == '\t') {
      p++;
    }
  }

  // Read rows
  char *endRows = NULL;
  unsigned long rows = strtoul(p, &endRows, 10);
  if(rows == 0 || endRows == NULL || endRows == p) {
    return false;
  }
  while(*endRows == ' ' || *endRows == '\t') {
    endRows++;
  }

  // Dimension separator must be 'x' or 'X'
  if(*endRows != 'x' && *endRows != 'X') {
    return false;
  }
  endRows++; // skip 'x'
  while(*endRows == ' ' || *endRows == '\t') {
    endRows++;
  }

  // Read cols
  char *endCols = NULL;
  unsigned long cols = strtoul(endRows, &endCols, 10);
  if(cols == 0 || endCols == NULL || endCols == endRows) {
    return false;
  }

  while(*endCols == ' ' || *endCols == '\t' || *endCols == ']') {
    endCols++;
  }

  // Must be followed by newline, whitespace or end of string
  if(*endCols != '\r' && *endCols != '\n' && *endCols != ' ' && *endCols != '\t' && *endCols != '\0') {
    return false;
  }

  // Dimension and storage overflow sanity check
  if((uint64_t)rows * cols * REAL34_SIZE_IN_BLOCKS + TO_BLOCKS(sizeof(matrixHeader_t)) > 0xFFFFu) {
    return false;
  }

  uint32_t totalElements = (uint32_t)(rows * cols);
  const char *curr = endCols;

  // Count non-empty lines following the header to choose reading strategy
  const char *scan = curr;
  uint32_t lineCount = 0;
  while(*scan) {
    while(*scan == ' ' || *scan == '\t') {
      scan++;
    }
    if(*scan == '\r' || *scan == '\n') {
      while(*scan == '\r' || *scan == '\n') {
        scan++;
      }
      continue;
    }
    if(*scan == '\0') {
      break;
    }
    lineCount++;
    while(*scan && *scan != '\r' && *scan != '\n') {
      scan++;
    }
  }

  bool_t lineByLine = (lineCount >= totalElements);

  // Allocate temporary element buffers
  real34_t *realElements = (real34_t *)malloc(totalElements * sizeof(real34_t));
  real34_t *imagElements = (real34_t *)calloc(totalElements, sizeof(real34_t));
  if(!realElements || !imagElements) {
    if(realElements) {
      free(realElements);
    }
    if(imagElements) {
      free(imagElements);
    }
    return false;
  }

  bool_t isComplex = false;

  for(uint32_t i = 0; i < totalElements; i++) {
    char elemBuf[256];
    elemBuf[0] = '\0';

    // Skip leading delimiters/whitespace
    while(*curr == ' ' || *curr == '\t' || *curr == '\r' || *curr == '\n' || *curr == ';' || *curr == ',') {
      curr++;
    }

    if(*curr == '\0') {
      real34SetZero(&realElements[i]);
      real34SetZero(&imagElements[i]);
      continue;
    }

    if(lineByLine) {
      const char *nl = strpbrk(curr, "\r\n");
      size_t elen = nl ? (size_t)(nl - curr) : strlen(curr);
      if(elen >= sizeof(elemBuf)) {
        elen = sizeof(elemBuf) - 1;
      }
      strncpy(elemBuf, curr, elen);
      elemBuf[elen] = '\0';
      curr += elen;
    }
    else {
      const char *delim = curr;
      while(*delim && *delim != ' ' && *delim != '\t' && *delim != '\r' && *delim != '\n' && *delim != ';' && *delim != ',') {
        delim++;
      }
      size_t elen = (size_t)(delim - curr);
      if(elen >= sizeof(elemBuf)) {
        elen = sizeof(elemBuf) - 1;
      }
      strncpy(elemBuf, curr, elen);
      elemBuf[elen] = '\0';
      curr = delim;
    }

    // Trim trailing whitespace
    char *e = elemBuf + strlen(elemBuf);
    while(e > elemBuf && (*(e - 1) == ' ' || *(e - 1) == '\t')) {
      *(--e) = '\0';
    }

    // Check if element contains a complex unit
    char *cpxUnit = strstr(elemBuf, "jx");
    if(!cpxUnit) {
      cpxUnit = strstr(elemBuf, "ix");
    }
    if(!cpxUnit) {
      cpxUnit = strstr(elemBuf, "\x83\xc1");
    }

    if(cpxUnit != NULL) {
      isComplex = true;
      // Search backwards from cpxUnit for '+' or '-'
      char *signPtr = NULL;
      for(char *c = cpxUnit - 1; c >= elemBuf; c--) {
        if(*c == '+' || *c == '-') {
          signPtr = c;
          break;
        }
      }

      if(signPtr != NULL) {
        char realPart[128];
        size_t rLen = signPtr - elemBuf;
        if(rLen >= sizeof(realPart)) {
          rLen = sizeof(realPart) - 1;
        }
        strncpy(realPart, elemBuf, rLen);
        realPart[rLen] = '\0';

        char *reEnd = realPart + strlen(realPart);
        while(reEnd > realPart && (*(reEnd - 1) == ' ' || *(reEnd - 1) == '\t')) {
          *(--reEnd) = '\0';
        }
        stringToReal34(realPart, &realElements[i]);

        char *imagVal = cpxUnit + (cpxUnit[1] == 'x' || cpxUnit[1] == 'X' ? 2 : 1);
        while(*imagVal == ' ' || *imagVal == '\t') {
          imagVal++;
        }
        if(*imagVal == '\0') {
          real34Copy(const34_1, &imagElements[i]);
          if(*signPtr == '-') {
            real34SetNegativeSign(&imagElements[i]);
          }
        }
        else {
          stringToReal34(imagVal, &imagElements[i]);
          if(*signPtr == '-') {
            real34SetNegativeSign(&imagElements[i]);
          }
        }
      }
      else {
        // Pure imaginary element e.g. "jx2" or "-jx3"
        real34SetZero(&realElements[i]);
        char *imagVal = cpxUnit + (cpxUnit[1] == 'x' || cpxUnit[1] == 'X' ? 2 : 1);
        while(*imagVal == ' ' || *imagVal == '\t') {
          imagVal++;
        }
        if(*imagVal == '\0') {
          real34Copy(const34_1, &imagElements[i]);
        }
        else {
          stringToReal34(imagVal, &imagElements[i]);
        }
        if(elemBuf[0] == '-') {
          real34SetNegativeSign(&imagElements[i]);
        }
      }
    }
    else {
      // Real element: normalize comma radix if needed
      char *lastDot = strrchr(elemBuf, '.');
      char *lastComma = strrchr(elemBuf, ',');
      if(lastComma != NULL && lastDot == NULL) {
        for(char *c = elemBuf; *c; c++) {
          if(*c == ',') {
            *c = '.';
          }
        }
      }
      stringToReal34(elemBuf, &realElements[i]);
      real34SetZero(&imagElements[i]);
    }
  }

  pthread_mutex_lock(&screenMutex);
  if(calcMode == CM_NORMAL && getSystemFlag(FLAG_ASLIFT)) {
    liftStack();
  }

  if(isComplex) {
    reallocateRegister(REGISTER_X, dtComplex34Matrix, COMPLEX34_SIZE_IN_BLOCKS * rows * cols, amNone);
    REGISTER_MATRIX_HEADER(REGISTER_X)->matrixRows = (uint16_t)rows;
    REGISTER_MATRIX_HEADER(REGISTER_X)->matrixColumns = (uint16_t)cols;
    complex34_t *dst = REGISTER_COMPLEX34_MATRIX_ELEMENTS(REGISTER_X);
    for(uint32_t i = 0; i < totalElements; i++) {
      real34Copy(&realElements[i], VARIABLE_REAL34_DATA(dst + i));
      real34Copy(&imagElements[i], VARIABLE_IMAG34_DATA(dst + i));
    }
  }
  else {
    reallocateRegister(REGISTER_X, dtReal34Matrix, REAL34_SIZE_IN_BLOCKS * rows * cols, amNone);
    REGISTER_MATRIX_HEADER(REGISTER_X)->matrixRows = (uint16_t)rows;
    REGISTER_MATRIX_HEADER(REGISTER_X)->matrixColumns = (uint16_t)cols;
    real34_t *dst = REGISTER_REAL34_MATRIX_ELEMENTS(REGISTER_X);
    for(uint32_t i = 0; i < totalElements; i++) {
      real34Copy(&realElements[i], dst + i);
    }
  }

  free(realElements);
  free(imagElements);

  setSystemFlag(FLAG_ASLIFT);

  extern bool_t reDraw;
  extern gboolean refreshLcd(gpointer unusedData);
  reDraw = true;
  screenUpdatingMode = SCRUPD_AUTO;
  refreshScreen(1);
  refreshLcd(NULL);
  lcd_refresh();
  pthread_mutex_unlock(&screenMutex);

  return true;
}

// Helper to normalize a numeric token: remove thousand separators, convert European decimal comma to dot,
// strip spaces, trim trailing radix marks.
static bool_t normalizeNumericToken(const char *src, char *dst, size_t dstSize) {
  if(!src || !*src || dstSize < 2) {
    return false;
  }

  char buf[256];
  const char *r = src;
  char *w = buf;
  while(*r && (size_t)(w - buf) < sizeof(buf) - 1) {
    if(*r != ' ' && *r != '\t' && *r != '\r' && *r != '\n') {
      *w++ = *r;
    }
    r++;
  }
  *w = '\0';

  if(buf[0] == '\0') {
    return false;
  }

  char *lastDot = strrchr(buf, '.');
  char *lastComma = strrchr(buf, ',');

  if(lastComma != NULL && lastDot == NULL) {
    // Lone comma radix (e.g. "1234,56") -> convert to '.'
    for(char *p = buf; *p; p++) {
      if(*p == ',') {
        *p = '.';
      }
    }
  }
  else if(lastDot != NULL && lastComma != NULL) {
    if(lastComma > lastDot) {
      // European format: "1.234,56" -> thousand dots stripped, comma to '.'
      char *s = buf;
      char *d = buf;
      while(*s) {
        if(*s == '.') {
          s++;
        }
        else if(*s == ',') {
          *d++ = '.';
          s++;
        }
        else {
          *d++ = *s++;
        }
      }
      *d = '\0';
    }
    else {
      // US format: "1,234.56" -> thousand commas stripped
      char *s = buf;
      char *d = buf;
      while(*s) {
        if(*s == ',') {
          s++;
        }
        else {
          *d++ = *s++;
        }
      }
      *d = '\0';
    }
  }

  // Trim trailing radix marks unless string is just "."
  size_t blen = strlen(buf);
  while(blen > 1 && (buf[blen - 1] == '.' || buf[blen - 1] == ',')) {
    buf[--blen] = '\0';
  }

  if(blen >= dstSize) {
    return false;
  }
  strcpy(dst, buf);
  return true;
}

// Find imaginary unit ('i', 'j', 'I', 'J', STD_op_i, STD_op_j)
// Returns pointer to unit, or NULL if none.
// Ensures unit is not part of "pi", "PI", "pir", or ":MULTPI".
static const char *findImaginaryUnit(const char *s) {
  const char *p = s;
  while(*p) {
    if((unsigned char)p[0] == 0x83 && ((unsigned char)p[1] == 0xC1 || (unsigned char)p[1] == 0xC2)) {
      return p;
    }
    char c = *p;
    if(c == 'i' || c == 'I' || c == 'j' || c == 'J') {
      if(p == s || (p[-1] != 'p' && p[-1] != 'P')) {
        if(p < s + 5 || strncmp(p - 5, ":MULT", 5) != 0) {
          return p;
        }
      }
    }
    p++;
  }
  return NULL;
}

// Find '+' or '-' separating Real and Imaginary parts (index > 0, not after 'e' or 'E').
static const char *findRealImagSeparator(const char *s) {
  size_t len = strlen(s);
  for(size_t i = 1; i < len; i++) {
    char c = s[i];
    if(c == '+' || c == '-') {
      size_t prev = i - 1;
      while(prev > 0 && (s[prev] == ' ' || s[prev] == '\t')) {
        prev--;
      }
      char prevChar = s[prev];
      if(prevChar == 'e' || prevChar == 'E') {
        continue;
      }
      size_t next = i + 1;
      while(next < len && (s[next] == ' ' || s[next] == '\t')) {
        next++;
      }
      if(next < len) {
        return s + i;
      }
    }
  }
  return NULL;
}

static bool_t parseRealToken(const char *tok, real34_t *valOut) {
  if(!tok) {
    return false;
  }
  char cleanBuf[256];
  if(!normalizeNumericToken(tok, cleanBuf, sizeof(cleanBuf))) {
    return false;
  }
  stringToReal34(cleanBuf, valOut);
  return !real34IsNaN(valOut);
}

static bool_t parseImaginaryToken(const char *tok, real34_t *valOut) {
  if(!tok) {
    return false;
  }

  bool_t isNeg = false;
  const char *p = tok;
  while(*p == ' ' || *p == '\t') {
    p++;
  }
  if(*p == '-') {
    isNeg = true;
    p++;
  }
  else if(*p == '+') {
    p++;
  }

  char magBuf[256];
  char *w = magBuf;
  while(*p && (size_t)(w - magBuf) < sizeof(magBuf) - 1) {
    if((unsigned char)p[0] == 0x83 && ((unsigned char)p[1] == 0xC1 || (unsigned char)p[1] == 0xC2)) {
      p += 2;
      continue;
    }
    char c = *p;
    if(c == 'i' || c == 'I' || c == 'j' || c == 'J' || c == '*' || c == ' ' || c == '\t') {
      p++;
      continue;
    }
    if((c == 'x' || c == 'X') && (w == magBuf)) {
      p++;
      continue;
    }
    *w++ = *p++;
  }
  *w = '\0';

  if(magBuf[0] == '-') {
    isNeg = !isNeg;
    memmove(magBuf, magBuf + 1, strlen(magBuf));
  }
  else if(magBuf[0] == '+') {
    memmove(magBuf, magBuf + 1, strlen(magBuf));
  }

  if(magBuf[0] == '\0') {
    real34Copy(const34_1, valOut);
  }
  else {
    char cleanBuf[256];
    if(!normalizeNumericToken(magBuf, cleanBuf, sizeof(cleanBuf))) {
      return false;
    }
    stringToReal34(cleanBuf, valOut);
    if(real34IsNaN(valOut)) {
      return false;
    }
  }

  if(isNeg) {
    real34SetNegativeSign(valOut);
  }
  else {
    real34SetPositiveSign(valOut);
  }
  return true;
}

static bool_t tryPasteComplex(const char *rawInput) {
  if(!rawInput || !*rawInput) {
    return false;
  }

  char buf[512];
  const char *start = rawInput;
  while(*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n') {
    start++;
  }
  if(!*start) {
    return false;
  }

  size_t inLen = strlen(start);
  if(inLen >= sizeof(buf)) {
    return false;
  }
  strcpy(buf, start);
  char *end = buf + strlen(buf);
  while(end > buf && (*(end - 1) == ' ' || *(end - 1) == '\t' || *(end - 1) == '\r' || *(end - 1) == '\n')) {
    *(--end) = '\0';
  }

  bool_t hasOuterParens = false;
  size_t len = strlen(buf);
  if(buf[0] == '(' && len > 2 && buf[len - 1] == ')') {
    hasOuterParens = true;
    buf[len - 1] = '\0';
    memmove(buf, buf + 1, len - 1);
    char *s = buf;
    while(*s == ' ' || *s == '\t') {
      s++;
    }
    if(s > buf) {
      memmove(buf, s, strlen(s) + 1);
    }
    end = buf + strlen(buf);
    while(end > buf && (*(end - 1) == ' ' || *(end - 1) == '\t')) {
      *(--end) = '\0';
    }
  }

  real34_t reVal, imVal;
  uint32_t tag = amNone;
  bool_t parsed = false;

  // 1. Polar notation: angle indicators \xe2\x88\xa1 (∡), \xe2\x88\xa0 (∠), /_, or ' < '
  const char *polarSep = NULL;
  size_t polarSepLen = 0;
  if((polarSep = strstr(buf, "\xe2\x88\xa1")) != NULL) {
    polarSepLen = 3;
  }
  else if((polarSep = strstr(buf, "\xe2\x88\xa0")) != NULL) {
    polarSepLen = 3;
  }
  else if((polarSep = strstr(buf, "/_")) != NULL) {
    polarSepLen = 2;
  }
  else if((polarSep = strstr(buf, " < ")) != NULL) {
    polarSepLen = 3;
  }

  if(polarSep != NULL) {
    char magStr[256];
    char angStr[256];
    size_t mlen = (size_t)(polarSep - buf);
    if(mlen < sizeof(magStr)) {
      strncpy(magStr, buf, mlen);
      magStr[mlen] = '\0';
      strncpy(angStr, polarSep + polarSepLen, sizeof(angStr) - 1);
      angStr[sizeof(angStr) - 1] = '\0';

      angularMode_t angMode = currentAngularMode;
      char *degP = strstr(angStr, ":DEG");
      char *radP = strstr(angStr, ":RAD");
      char *gradP = strstr(angStr, ":GRAD");
      char *multpiP = strstr(angStr, ":MULTPI");
      char *piUtf8 = strstr(angStr, "\xcf\x80");
      if(!piUtf8) {
        piUtf8 = strstr(angStr, "\xce\xa0");
      }

      if(multpiP) {
        angMode = amMultPi;
        *multpiP = '\0';
      }
      else if(piUtf8) {
        angMode = amMultPi;
        *piUtf8 = '\0';
      }
      else if(degP) {
        angMode = amDegree;
        *degP = '\0';
      }
      else if(radP) {
        angMode = amRadian;
        *radP = '\0';
      }
      else if(gradP) {
        angMode = amGrad;
        *gradP = '\0';
      }
      else {
        size_t alen = strlen(angStr);
        while(alen > 0 && (angStr[alen - 1] == ' ' || angStr[alen - 1] == '\t')) {
          angStr[--alen] = '\0';
        }
        if(alen >= 3 && strcasecmp(angStr + alen - 3, "deg") == 0) {
          angMode = amDegree;
          angStr[alen - 3] = '\0';
        }
        else if(alen >= 3 && strcasecmp(angStr + alen - 3, "rad") == 0) {
          angMode = amRadian;
          angStr[alen - 3] = '\0';
        }
        else if(alen >= 4 && strcasecmp(angStr + alen - 4, "grad") == 0) {
          angMode = amGrad;
          angStr[alen - 4] = '\0';
        }
        else if(alen >= 3 && strcasecmp(angStr + alen - 3, "pir") == 0) {
          angMode = amMultPi;
          angStr[alen - 3] = '\0';
        }
        else if(alen >= 2 && strcasecmp(angStr + alen - 2, "pi") == 0) {
          angMode = amMultPi;
          angStr[alen - 2] = '\0';
        }
        else if(alen >= 1 && (angStr[alen - 1] == 'r' || angStr[alen - 1] == 'R')) {
          angMode = amRadian;
          angStr[alen - 1] = '\0';
        }
        else if(alen >= 1 && (angStr[alen - 1] == 'g' || angStr[alen - 1] == 'G')) {
          angMode = amGrad;
          angStr[alen - 1] = '\0';
        }
        else if(alen >= 1 && (angStr[alen - 1] == 'd' || angStr[alen - 1] == 'D')) {
          angMode = amDegree;
          angStr[alen - 1] = '\0';
        }
      }

      if(angMode == amMultPi) {
        char *ae = angStr + strlen(angStr);
        while(ae > angStr && (*(ae - 1) == ' ' || *(ae - 1) == '\t' || *(ae - 1) == '*')) {
          *(--ae) = '\0';
        }
        char *as = angStr;
        while(*as == ' ' || *as == '\t') {
          as++;
        }
        if(*as == '\0' || strcmp(as, "+") == 0) {
          strcpy(angStr, "1.");
        }
        else if(strcmp(as, "-") == 0) {
          strcpy(angStr, "-1.");
        }
      }

      real34_t mag34, ang34;
      if(parseRealToken(magStr, &mag34) && parseRealToken(angStr, &ang34)) {
        convertAngle34FromTo(&ang34, angMode, amRadian);
        real_t rMag, rAng, rSin, rCos, rRe, rIm;
        real34ToReal(&mag34, &rMag);
        real34ToReal(&ang34, &rAng);
        C47_WP34S_Cvt2RadSinCosTan(&rAng, amRadian, &rSin, &rCos, NULL, &ctxtReal39);
        realMultiply(&rMag, &rCos, &rRe, &ctxtReal39);
        realMultiply(&rMag, &rSin, &rIm, &ctxtReal39);
        realToReal34(&rRe, &reVal);
        realToReal34(&rIm, &imVal);
        tag = (angMode & amAngleMask) | amPolar;
        parsed = true;
      }
    }
  }

  // 2. Coordinate / Tuple format: "(re, im)" or "re; im"
  if(!parsed && (hasOuterParens || strchr(buf, ';') != NULL)) {
    char *coordSep = strchr(buf, ';');
    if(!coordSep && hasOuterParens) {
      coordSep = strchr(buf, ',');
    }
    if(coordSep != NULL) {
      char part1[256], part2[256];
      size_t l1 = (size_t)(coordSep - buf);
      if(l1 < sizeof(part1)) {
        strncpy(part1, buf, l1);
        part1[l1] = '\0';
        strncpy(part2, coordSep + 1, sizeof(part2) - 1);
        part2[sizeof(part2) - 1] = '\0';

        if(parseRealToken(part1, &reVal) && parseRealToken(part2, &imVal)) {
          tag = amNone;
          parsed = true;
        }
      }
    }
  }

  // 3. Cartesian notation with imaginary unit ('i', 'j', STD_op_i, STD_op_j)
  if(!parsed) {
    const char *unitPtr = findImaginaryUnit(buf);
    if(unitPtr != NULL) {
      const char *sep = findRealImagSeparator(buf);
      if(sep == NULL) {
        real34SetZero(&reVal);
        if(parseImaginaryToken(buf, &imVal)) {
          tag = amNone;
          parsed = true;
        }
      }
      else {
        char part1[256], part2[256];
        size_t l1 = (size_t)(sep - buf);
        if(l1 < sizeof(part1)) {
          strncpy(part1, buf, l1);
          part1[l1] = '\0';
          strncpy(part2, sep, sizeof(part2) - 1);
          part2[sizeof(part2) - 1] = '\0';

          const char *u1 = findImaginaryUnit(part1);
          const char *u2 = findImaginaryUnit(part2);

          if(u2 != NULL && u1 == NULL) {
            if(parseRealToken(part1, &reVal) && parseImaginaryToken(part2, &imVal)) {
              tag = amNone;
              parsed = true;
            }
          }
          else if(u1 != NULL && u2 == NULL) {
            if(parseRealToken(part2, &reVal) && parseImaginaryToken(part1, &imVal)) {
              tag = amNone;
              parsed = true;
            }
          }
        }
      }
    }
  }

  if(!parsed) {
    return false;
  }

  pthread_mutex_lock(&screenMutex);
  if(calcMode == CM_NORMAL && getSystemFlag(FLAG_ASLIFT)) {
    liftStack();
  }

  reallocateRegister(REGISTER_X, dtComplex34, 0, tag);
  real34Copy(&reVal, REGISTER_REAL34_DATA(REGISTER_X));
  real34Copy(&imVal, REGISTER_IMAG34_DATA(REGISTER_X));

  setSystemFlag(FLAG_ASLIFT);

  extern bool_t reDraw;
  extern gboolean refreshLcd(gpointer unusedData);
  reDraw = true;
  screenUpdatingMode = SCRUPD_AUTO;
  refreshScreen(1);
  refreshLcd(NULL);
  lcd_refresh();
  pthread_mutex_unlock(&screenMutex);

  return true;
}

JNIEXPORT void JNICALL
Java_org_rpncalculators_r47_MainActivity_pasteTextNative(
    JNIEnv *env, jobject thiz, jstring text) {
  (void)thiz;
  if(!ram || isCoreBlockingForIo) {
    return;
  }

  const char *rawText = (*env)->GetStringUTFChars(env, text, 0);
  if(!rawText) {
    return;
  }

  // Trim leading/trailing whitespace from rawText
  const char *rawStart = rawText;
  while(*rawStart == ' ' || *rawStart == '\t' || *rawStart == '\r' || *rawStart == '\n') {
    rawStart++;
  }
  size_t rawTrimLen = strlen(rawStart);
  while(rawTrimLen > 0 && (rawStart[rawTrimLen - 1] == ' ' || rawStart[rawTrimLen - 1] == '\t' ||
                           rawStart[rawTrimLen - 1] == '\r' || rawStart[rawTrimLen - 1] == '\n')) {
    rawTrimLen--;
  }

  if(rawTrimLen == 0) {
    (*env)->ReleaseStringUTFChars(env, text, rawText);
    return;
  }

  // 1. Explicitly quoted string ("..." or '...'): strip enclosing quotes and paste directly as String
  if(rawTrimLen >= 2 && ((rawStart[0] == '"' && rawStart[rawTrimLen - 1] == '"') ||
                         (rawStart[0] == '\'' && rawStart[rawTrimLen - 1] == '\''))) {
    char *quotedStr = (char *)malloc(rawTrimLen);
    if(quotedStr) {
      memcpy(quotedStr, rawStart + 1, rawTrimLen - 2);
      quotedStr[rawTrimLen - 2] = '\0';

      pthread_mutex_lock(&screenMutex);
      if(calcMode == CM_NORMAL && getSystemFlag(FLAG_ASLIFT)) {
        liftStack();
      }
      restoreRegister(REGISTER_X, "Stri", quotedStr);
      setSystemFlag(FLAG_ASLIFT);

      extern bool_t reDraw;
      extern gboolean refreshLcd(gpointer unusedData);
      reDraw = true;
      screenUpdatingMode = SCRUPD_AUTO;
      refreshScreen(1);
      refreshLcd(NULL);
      lcd_refresh();
      pthread_mutex_unlock(&screenMutex);

      free(quotedStr);
    }
    (*env)->ReleaseStringUTFChars(env, text, rawText);
    return;
  }

  // Save copy of unquoted trimmed input for string fallback (preserving spaces/punctuation)
  char *origTrimmed = (char *)malloc(rawTrimLen + 1);
  if(!origTrimmed) {
    (*env)->ReleaseStringUTFChars(env, text, rawText);
    return;
  }
  memcpy(origTrimmed, rawStart, rawTrimLen);
  origTrimmed[rawTrimLen] = '\0';

  size_t rawLen = strlen(rawText);
  char *buf = (char *)malloc(rawLen * 2 + 64);
  if(!buf) {
    free(origTrimmed);
    (*env)->ReleaseStringUTFChars(env, text, rawText);
    return;
  }

  size_t len = 0;
  const char *p = rawText;
  while(*p) {
    unsigned char c = (unsigned char)*p;
    if(c == 0xC2 && (unsigned char)*(p + 1) == 0xB0) {
      buf[len++] = ':';
      buf[len++] = 'D';
      buf[len++] = 'E';
      buf[len++] = 'G';
      p += 2;
    } else if(c == 0x80 && (unsigned char)*(p + 1) == 0xB0) {
      buf[len++] = ':';
      buf[len++] = 'D';
      buf[len++] = 'E';
      buf[len++] = 'G';
      p += 2;
    } else if(c == 0xAC && (unsigned char)*(p + 1) == 0x66) { // STD_SUP_pir
      buf[len++] = ':';
      buf[len++] = 'M';
      buf[len++] = 'U';
      buf[len++] = 'L';
      buf[len++] = 'T';
      buf[len++] = 'P';
      buf[len++] = 'I';
      p += 2;
    } else if(c == 0x83 && (unsigned char)*(p + 1) == 0xC0) { // STD_pi
      buf[len++] = ':';
      buf[len++] = 'M';
      buf[len++] = 'U';
      buf[len++] = 'L';
      buf[len++] = 'T';
      buf[len++] = 'P';
      buf[len++] = 'I';
      p += 2;
      if(*p == 'r' || *p == 'R') {
        p++;
      }
    } else if(c == 0xE2 && (unsigned char)*(p + 1) == 0x80 && ((unsigned char)*(p + 2) >= 0x80 && (unsigned char)*(p + 2) <= 0x8B)) {
      // UTF-8 U+2000..U+200B (punctuation spaces, thin space, hair space, zero-width space)
      buf[len++] = ' ';
      p += 3;
    } else if(c == 0xC2 && (unsigned char)*(p + 1) == 0xA0) {
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
  while(*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') {
    s++;
  }
  char *e = s + strlen(s);
  while(e > s && (*(e - 1) == ' ' || *(e - 1) == '\t' || *(e - 1) == '\r' || *(e - 1) == '\n')) {
    *(--e) = '\0';
  }
  if(!*s) {
    free(origTrimmed);
    free(buf);
    return;
  }

  // Strip leading register identifier (e.g. "X = ..." or "X=...")
  if((s[0] == 'X' || s[0] == 'x') && (s[1] == '=' || (s[1] == ' ' && s[2] == '='))) {
    char *afterEq = strchr(s, '=');
    if(afterEq) {
      s = afterEq + 1;
      while(*s == ' ' || *s == '\t') {
        s++;
      }
    }
    char *origAfterEq = strchr(origTrimmed, '=');
    if(origAfterEq) {
      char *newOrig = origAfterEq + 1;
      while(*newOrig == ' ' || *newOrig == '\t') {
        newOrig++;
      }
      memmove(origTrimmed, newOrig, strlen(newOrig) + 1);
    }
  }

  if(tryPasteMatrix(s)) {
    free(origTrimmed);
    free(buf);
    return;
  }

  if(tryPasteComplex(s)) {
    free(origTrimmed);
    free(buf);
    return;
  }

  char typeTag[32];
  strcpy(typeTag, "Real");

  char multpiDefaultBuf[8] = "";

  char *degTag = strstr(s, ":DEG");
  char *radTag = strstr(s, ":RAD");
  char *gradTag = strstr(s, ":GRAD");
  char *multpiTag = strstr(s, ":MULTPI");

  char *piUtf8 = strstr(s, "\xcf\x80");     // UTF-8 lowercase pi 'π'
  if(!piUtf8) {
    piUtf8 = strstr(s, "\xce\xa0");         // UTF-8 uppercase pi 'Π'
  }

  if(multpiTag) {
    strcpy(typeTag, "Real:MULTPI");
    *multpiTag = '\0';
  } else if(piUtf8) {
    strcpy(typeTag, "Real:MULTPI");
    *piUtf8 = '\0';
  } else if(degTag) {
    strcpy(typeTag, "Real:DEG");
    *degTag = '\0';
  } else if(radTag) {
    strcpy(typeTag, "Real:RAD");
    *radTag = '\0';
  } else if(gradTag) {
    strcpy(typeTag, "Real:GRAD");
    *gradTag = '\0';
  } else {
    size_t slen = strlen(s);
    if(slen >= 3 && strcasecmp(s + slen - 3, "pir") == 0) {
      strcpy(typeTag, "Real:MULTPI");
      s[slen - 3] = '\0';
    } else if(slen >= 2 && strcasecmp(s + slen - 2, "pi") == 0) {
      strcpy(typeTag, "Real:MULTPI");
      s[slen - 2] = '\0';
    } else if(slen >= 3 && strcasecmp(s + slen - 3, "deg") == 0) {
      strcpy(typeTag, "Real:DEG");
      s[slen - 3] = '\0';
    } else if(slen >= 3 && strcasecmp(s + slen - 3, "rad") == 0) {
      strcpy(typeTag, "Real:RAD");
      s[slen - 3] = '\0';
    } else if(slen >= 4 && strcasecmp(s + slen - 4, "grad") == 0) {
      strcpy(typeTag, "Real:GRAD");
      s[slen - 4] = '\0';
    } else if(slen >= 2 && (s[slen - 1] == 'r' || s[slen - 1] == 'R') &&
              (isdigit((unsigned char)s[slen - 2]) || s[slen - 2] == '.' || s[slen - 2] == ',')) {
      strcpy(typeTag, "Real:RAD");
      s[slen - 1] = '\0';
    } else if(slen >= 2 && (s[slen - 1] == 'g' || s[slen - 1] == 'G') &&
              (isdigit((unsigned char)s[slen - 2]) || s[slen - 2] == '.' || s[slen - 2] == ',')) {
      strcpy(typeTag, "Real:GRAD");
      s[slen - 1] = '\0';
    } else if(slen >= 2 && (s[slen - 1] == 'd' || s[slen - 1] == 'D') &&
              (isdigit((unsigned char)s[slen - 2]) || s[slen - 2] == '.' || s[slen - 2] == ',')) {
      strcpy(typeTag, "Real:DEG");
      s[slen - 1] = '\0';
    }
  }

  if(strcmp(typeTag, "Real:MULTPI") == 0) {
    char *end = s + strlen(s);
    while(end > s && (*(end - 1) == ' ' || *(end - 1) == '\t' || *(end - 1) == '*')) {
      *(--end) = '\0';
    }
    while(*s == ' ' || *s == '\t') {
      s++;
    }
    if(*s == '\0' || strcmp(s, "+") == 0) {
      strcpy(multpiDefaultBuf, "1.");
      s = multpiDefaultBuf;
    } else if(strcmp(s, "-") == 0) {
      strcpy(multpiDefaultBuf, "-1.");
      s = multpiDefaultBuf;
    }
  }

  char timeSecBuf[64];
  char dateFmtBuf[64];
  bool_t isTime = false;
  bool_t isDate = false;

  if(strchr(s, ':')) {
    bool_t isNeg = false;
    char *pTime = s;
    if(*pTime == '-') {
      isNeg = true;
      pTime++;
    } else if(*pTime == '+') {
      pTime++;
    }

    char *firstColon = strchr(pTime, ':');
    if(firstColon) {
      char *secondColon = strchr(firstColon + 1, ':');
      char part1[32], part2[32], part3[32];

      size_t l1 = firstColon - pTime;
      if(l1 >= sizeof(part1)) {
        l1 = sizeof(part1) - 1;
      }
      strncpy(part1, pTime, l1);
      part1[l1] = '\0';

      if(secondColon) {
        size_t l2 = secondColon - (firstColon + 1);
        if(l2 >= sizeof(part2)) {
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
      while(*t1 == ' ' || *t1 == '\t') {
        t1++;
      }
      char *te1 = t1 + strlen(t1);
      while(te1 > t1 && (*(te1 - 1) == ' ' || *(te1 - 1) == '\t')) {
        *(--te1) = '\0';
      }
      char cleanP1[32], cleanP2[32], cleanP3[32];
      char *w = cleanP1;
      for(char *r = t1; *r; r++) {
        if(*r != ' ' && *r != '\t') {
          *w++ = *r;
        }
      }
      *w = '\0';

      char *t2 = part2;
      while(*t2 == ' ' || *t2 == '\t') {
        t2++;
      }
      char *te2 = t2 + strlen(t2);
      while(te2 > t2 && (*(te2 - 1) == ' ' || *(te2 - 1) == '\t')) {
        *(--te2) = '\0';
      }
      w = cleanP2;
      for(char *r = t2; *r; r++) {
        if(*r != ' ' && *r != '\t') {
          *w++ = *r;
        }
      }
      *w = '\0';

      char *t3 = part3;
      while(*t3 == ' ' || *t3 == '\t') {
        t3++;
      }
      char *te3 = t3 + strlen(t3);
      while(te3 > t3 && (*(te3 - 1) == ' ' || *(te3 - 1) == '\t')) {
        *(--te3) = '\0';
      }
      w = cleanP3;
      for(char *r = t3; *r; r++) {
        if(*r != ' ' && *r != '\t') {
          *w++ = *r;
        }
      }
      *w = '\0';

      if(cleanP1[0] && cleanP2[0] && cleanP3[0]) {
        real34_t rH, rM, rS, totalSec;
        stringToReal34(cleanP1, &rH);
        stringToReal34(cleanP2, &rM);
        stringToReal34(cleanP3, &rS);

        if(!real34IsNaN(&rH) && !real34IsNaN(&rM) && !real34IsNaN(&rS)) {
          real34Multiply(&rH, const34_3600, &totalSec);
          real34Multiply(&rM, const34_60, &rM);
          real34Add(&totalSec, &rM, &totalSec);
          real34Add(&totalSec, &rS, &totalSec);
          if(isNeg) {
            real34SetNegativeSign(&totalSec);
          }
          real34ToString(&totalSec, timeSecBuf);
          strcpy(typeTag, "Time");
          s = timeSecBuf;
          isTime = true;
        }
      }
    }
  } else if(strchr(s, '-') || strchr(s, '/')) {
    char sep = strchr(s, '-') ? '-' : '/';
    char *p1 = s;
    char *firstSep = strchr(p1, sep);
    char *secondSep = firstSep ? strchr(firstSep + 1, sep) : NULL;

    if(firstSep && secondSep) {
      char part1[32], part2[32], part3[32];
      size_t l1 = firstSep - p1;
      size_t l2 = secondSep - (firstSep + 1);

      if(l1 < sizeof(part1) && l2 < sizeof(part2)) {
        strncpy(part1, p1, l1);
        part1[l1] = '\0';
        strncpy(part2, firstSep + 1, l2);
        part2[l2] = '\0';
        strncpy(part3, secondSep + 1, sizeof(part3) - 1);
        part3[sizeof(part3) - 1] = '\0';

        char *endp1 = NULL, *endp2 = NULL, *endp3 = NULL;
        uint32_t n1 = (uint32_t)strtoul(part1, &endp1, 10);
        uint32_t n2 = (uint32_t)strtoul(part2, &endp2, 10);
        uint32_t n3 = (uint32_t)strtoul(part3, &endp3, 10);

        if(endp1 && *endp1 == '\0' && endp2 && *endp2 == '\0' && endp3 && *endp3 == '\0') {
          if(n1 > 31 && n2 >= 1 && n2 <= 12 && n3 >= 1 && n3 <= 31) {
            // YYYY-MM-DD
            snprintf(dateFmtBuf, sizeof(dateFmtBuf), "%u.%02u%02u", n1, n2, n3);
            strcpy(typeTag, "DYMD");
            s = dateFmtBuf;
            isDate = true;
          } else if(n3 > 31) {
            if(sep == '/' || getSystemFlag(FLAG_MDY)) {
              // MM/DD/YYYY
              snprintf(dateFmtBuf, sizeof(dateFmtBuf), "%u.%02u%04u", n1, n2, n3);
              strcpy(typeTag, "DMDY");
              s = dateFmtBuf;
              isDate = true;
            } else {
              // DD.MM.YYYY
              snprintf(dateFmtBuf, sizeof(dateFmtBuf), "%u.%02u%04u", n1, n2, n3);
              strcpy(typeTag, "DDMY");
              s = dateFmtBuf;
              isDate = true;
            }
          }
        }
      }
    }
  }

  bool_t isReal = false;
  char cleanBuf[256];

  if(!isDate && !isTime) {
    e = s + strlen(s);
    while(e > s && (*(e - 1) == ' ' || *(e - 1) == '\t' || *(e - 1) == '.' || *(e - 1) == ',')) {
      *(--e) = '\0';
    }

    // Sanitize Real numbers: strip any internal spaces (e.g. from digit grouping) and normalize radix
    char *r = s;
    char *w = cleanBuf;
    while(*r && (size_t)(w - cleanBuf) < sizeof(cleanBuf) - 1) {
      if(*r != ' ' && *r != '\t') {
        *w++ = *r;
      }
      r++;
    }
    *w = '\0';

    char *lastDot = strrchr(cleanBuf, '.');
    char *lastComma = strrchr(cleanBuf, ',');
    if(lastComma != NULL && lastDot == NULL) {
      // European format with comma as radix mark (e.g. "1234,56")
      for(char *pClean = cleanBuf; *pClean; pClean++) {
        if(*pClean == ',') {
          *pClean = '.';
        }
      }
    } else if(lastDot != NULL && lastComma != NULL) {
      if(lastComma > lastDot) {
        // European format with thousand dots and comma radix (e.g. "1.234,56")
        char *src = cleanBuf;
        char *dst = cleanBuf;
        while(*src) {
          if(*src == '.') {
            src++;
          } else if(*src == ',') {
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
        while(*src) {
          if(*src == ',') {
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
    while(clen > 0 && (cleanBuf[clen - 1] == '.' || cleanBuf[clen - 1] == ',')) {
      cleanBuf[--clen] = '\0';
    }

    if(cleanBuf[0] != '\0') {
      real34_t testReal;
      stringToReal34(cleanBuf, &testReal);
      if(!real34IsNaN(&testReal)) {
        isReal = true;
      }
    }
  }

  pthread_mutex_lock(&screenMutex);
  if(calcMode == CM_NORMAL && getSystemFlag(FLAG_ASLIFT)) {
    liftStack();
  }

  if(isDate || isTime) {
    restoreRegister(REGISTER_X, typeTag, s);
  } else if(isReal) {
    restoreRegister(REGISTER_X, typeTag, cleanBuf);
  } else {
    restoreRegister(REGISTER_X, "Stri", origTrimmed);
  }

  setSystemFlag(FLAG_ASLIFT);

  extern bool_t reDraw;
  extern gboolean refreshLcd(gpointer unusedData);
  reDraw = true;
  screenUpdatingMode = SCRUPD_AUTO;
  refreshScreen(1);
  refreshLcd(NULL);
  lcd_refresh();
  pthread_mutex_unlock(&screenMutex);

  free(origTrimmed);
  free(buf);
}