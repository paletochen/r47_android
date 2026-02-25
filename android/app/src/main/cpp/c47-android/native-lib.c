#include <jni.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <android/log.h>
#include <unistd.h>
#include "c47.h"
#include "saveRestoreCalcState.h"
#include "keyboard.h"

#define LOG_TAG "R47Native"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

// Hardcoded Item IDs from items.h
#define ITM_SQUARE_ID       58
#define ITM_SQUAREROOTX_ID  61

// JNI Globals
static JavaVM *g_jvm = NULL;
static jobject g_mainActivityObj = NULL;
static jmethodID g_requestFileId = NULL;
static jmethodID g_playToneId = NULL;
static jmethodID g_stopToneId = NULL;
static jmethodID g_processCoreTasksId = NULL;

// File sync
static pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t fileCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t screenMutex; // Initialized in initNative as recursive
static int fileDescriptor = -1;
static bool fileReady = false;
static bool fileCancelled = false;
static bool isCoreBlockingForIo = false;

// Forward declarations
extern void set_android_base_path(const char* path);
extern void init_lcd_buffers();
extern void setupUI(void);
extern void lcd_clear_buf();
extern uint32_t *screenData;
extern uint32_t *ram;

// Globals from c47-gtk
int16_t debugWindow = 0;
int16_t screenStride = 400; 
bool_t  screenChange = FALSE;
int     currentBezel = 0;
calcKeyboard_t calcKeyboard[43];
gboolean ui_is_active = FALSE;

uint32_t nextTimerRefresh = 0;
uint32_t nextScreenRefresh = 0;

void onUIActivity(void) {
    ui_is_active = TRUE;
}

static GdkEvent pressEvent;
static GdkEvent releaseEvent;

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

uint32_t sys_current_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

// Core functions
extern void refreshFn(uint16_t timerType);
extern void execFnTimeout(uint16_t timerType);
extern void shiftCutoff(uint16_t timerType);
extern void fnTimerDummy1(uint16_t timerType);
extern void execTimerApp(uint16_t timerType);
extern void btnFnPressed(GtkWidget *notUsed, GdkEvent *event, gpointer data);
extern void btnFnReleased(GtkWidget *notUsed, GdkEvent *event, gpointer data);
extern void btnPressed(GtkWidget *notUsed, GdkEvent *event, gpointer data);
extern void btnReleased(GtkWidget *notUsed, GdkEvent *event, gpointer data);

// HAL implementation
uint16_t getBeepVolume(void) { return 80; }
void audioTone(uint32_t frequency) {
    _Buzz(frequency, 200); // 200ms default
}

void _Buzz(uint32_t frequency, uint32_t ms_delay) {
    if (!g_mainActivityObj || !g_jvm || !g_playToneId) return;
    
    // Frequency 0 means silence (rest), but we still need to block for the delay
    if (frequency > 0) {
        JNIEnv *env;
        if ((*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6) == JNI_EDETACHED) {
            if ((*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL) != JNI_OK) return;
        }
        (*env)->CallVoidMethod(env, g_mainActivityObj, g_playToneId, (jint)frequency, (jint)ms_delay);
    }
    
    // Block the core thread for the duration of the beep/rest.
    // Added 10ms cushion to slightly slow down melodies to match hardware speed.
    usleep((ms_delay + 10) * 1000);
}

void processCoreTasksNative() {
    if (!g_mainActivityObj || !g_jvm || !g_processCoreTasksId) return;
    JNIEnv *env;
    if ((*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6) == JNI_EDETACHED) {
        if ((*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL) != JNI_OK) return;
    }
    (*env)->CallVoidMethod(env, g_mainActivityObj, g_processCoreTasksId);
}

void yieldToAndroid() {
    if (!ram) return;
    
    // Force core to process internal display dirty flags before yielding
    refreshLcd(NULL);
    lcd_refresh(); 

    // 1. Fully unlock the mutex so UI thread can call getDisplayPixels
    int lockCount = 0;
    while (pthread_mutex_unlock(&screenMutex) == 0) {
        lockCount++;
    }
    
    // 2. Perform housekeeping while unlocked (JUST THE SLEEP)
    // Tiny sleep to ensure UI thread gets a chance to grab the mutex
    usleep(1000); 
    
    // 3. Re-acquire the mutex to previous recursion level
    while (lockCount > 0) {
        pthread_mutex_lock(&screenMutex);
        lockCount--;
    }
}

void fnSetVolume(uint16_t v) { (void)v; }
void fnGetVolume(uint16_t v) { (void)v; }
void fnVolumeUp(uint16_t v) { (void)v; }
void fnVolumeDown(uint16_t v) { (void)v; }
void fnBuzz(uint16_t v) { (void)v; }
void fnPlay(uint16_t v) { (void)v; }
void squeak() { _Buzz(1000, 10); }

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    g_jvm = vm;
    
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

    return JNI_VERSION_1_6;
}

int requestAndroidFile(int isSave, const char* defaultName, int fileType) {
    LOGI("requestAndroidFile(isSave=%d, defaultName=%s, fileType=%d)", isSave, defaultName, fileType);
    if (!g_requestFileId) {
        LOGE("requestAndroidFile: g_requestFileId is NULL!");
        return -1;
    }
    JNIEnv *env;
    if ((*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        (*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL);
    }

    // DEADLOCK PREVENTION: Release the core lock while waiting for SAF.
    int lockCount = 0;
    while (pthread_mutex_unlock(&screenMutex) == 0) {
        lockCount++;
    }
    LOGI("requestAndroidFile: Fully released screenMutex (lockCount=%d)", lockCount);
    
    pthread_mutex_lock(&fileMutex);
    isCoreBlockingForIo = true;
    fileReady = false; fileCancelled = false; fileDescriptor = -1;
    pthread_mutex_unlock(&fileMutex); 
    
    jstring nameObj = (*env)->NewStringUTF(env, defaultName ? defaultName : "");
    (*env)->CallVoidMethod(env, g_mainActivityObj, g_requestFileId, (jboolean)isSave, nameObj, (jint)fileType);
    (*env)->DeleteLocalRef(env, nameObj);

    pthread_mutex_lock(&fileMutex);
    LOGI("requestAndroidFile: Waiting for file result...");
    while (!fileReady && !fileCancelled) pthread_cond_wait(&fileCond, &fileMutex);
    int fd = fileDescriptor;
    isCoreBlockingForIo = false;
    LOGI("requestAndroidFile: Resumed, fd=%d, cancelled=%d", fd, fileCancelled);
    pthread_mutex_unlock(&fileMutex);

    // Re-acquire the core lock before returning.
    LOGI("requestAndroidFile: Re-acquiring screenMutex (%d times)", lockCount);
    for (int i = 0; i < lockCount; i++) {
        pthread_mutex_lock(&screenMutex);
    }

    return fd;
}

JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_onFileSelectedNative(JNIEnv* env, jobject thiz, jint fd) {
    LOGI("onFileSelectedNative(fd=%d)", fd);
    pthread_mutex_lock(&fileMutex);
    fileDescriptor = fd; fileReady = true;
    pthread_cond_signal(&fileCond);
    pthread_mutex_unlock(&fileMutex);
}

JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_onFileCancelledNative(JNIEnv* env, jobject thiz) {
    LOGI("onFileCancelledNative()");
    pthread_mutex_lock(&fileMutex);
    fileCancelled = true;
    pthread_cond_signal(&fileCond);
    pthread_mutex_unlock(&fileMutex);
}

JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_updateNativeActivityRef(JNIEnv* env, jobject thiz) {
    LOGI("updateNativeActivityRef called");
    if (g_mainActivityObj) {
        (*env)->DeleteGlobalRef(env, g_mainActivityObj);
    }
    g_mainActivityObj = (*env)->NewGlobalRef(env, thiz);
    jclass clazz = (*env)->GetObjectClass(env, thiz);
    g_requestFileId = (*env)->GetMethodID(env, clazz, "requestFile", "(ZLjava/lang/String;I)V");
    g_playToneId = (*env)->GetMethodID(env, clazz, "playTone", "(II)V");
    g_stopToneId = (*env)->GetMethodID(env, clazz, "stopTone", "()V");
    g_processCoreTasksId = (*env)->GetMethodID(env, clazz, "processCoreTasks", "()V");

    // Force a refresh based on restored state.
    if (ram) {
        pthread_mutex_lock(&screenMutex);
        reDraw = true;
        refreshScreen(190);
        refreshLcd(NULL);
        lcd_refresh();
        pthread_mutex_unlock(&screenMutex);
    }
}

JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_nativePreInit(JNIEnv* env, jobject thiz, jstring path_obj) {
    const char *path = (*env)->GetStringUTFChars(env, path_obj, 0);
    set_android_base_path(path);
    (*env)->ReleaseStringUTFChars(env, path_obj, path);
    
    // Setup memory functions early
    extern void mp_set_memory_functions(void *(*alloc_func)(size_t),
                                 void *(*realloc_func)(void *, size_t, size_t),
                                 void (*free_func)(void *, size_t));
    extern void *allocGmp(size_t size);
    extern void *reallocGmp(void *ptr, size_t old_size, size_t new_size);
    extern void freeGmp(void *ptr, size_t size);
    mp_set_memory_functions(allocGmp, reallocGmp, freeGmp);
}

JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_initNative(JNIEnv* env, jobject thiz, jstring pathObj, jint slotId) {
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
    
    // Core redraw based on restored state.
    pthread_mutex_lock(&screenMutex);
    reDraw = true;
    refreshScreen(190);
    refreshLcd(NULL);
    lcd_refresh();
    pthread_mutex_unlock(&screenMutex);
}

JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_tick(JNIEnv* env, jobject thiz) {
    uint32_t now = sys_current_ms();
    if (pthread_mutex_trylock(&screenMutex) == 0) {
        if(nextTimerRefresh <= now) { refreshTimer(NULL); nextTimerRefresh = now + 5; }
        if(nextScreenRefresh <= now) { 
            refreshLcd(NULL); 
            lcd_refresh(); 
            nextScreenRefresh = now + 100; 
        }
        pthread_mutex_unlock(&screenMutex);
    }
}

static char currentPressedKeyStr[4] = {0};
static int currentPressedKeyCode = 0;

JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_saveStateNative(JNIEnv* env, jobject thiz) {
    LOGI("saveStateNative triggered");
    if (ram) {
        pthread_mutex_lock(&screenMutex);
        
        // 1. BACKUP the clean LCD buffer (pixels only)
        // SCREEN_HEIGHT * 52 is the standard size for 240x400 (52 bytes per row)
        size_t buf_size = SCREEN_HEIGHT * 52;
        uint32_t* pixel_backup = malloc(buf_size);
        if (pixel_backup) {
            memcpy(pixel_backup, lcd_buffer, buf_size);
        }

        // 2. Show feedback to user on their PHYSICAL Android screen.
        printStatus(0, errorMessages[101], 1); // 101 = SAVING_STATE_FILE, 1 = force
        lcd_refresh(); // Push "Saving..." to Android display
        
        // 3. RESTORE the clean pixels to the INTERNAL buffer before saving to disk.
        // This ensures the saved file contains no "Saving..." text.
        if (pixel_backup) {
            memcpy(lcd_buffer, pixel_backup, buf_size);
            free(pixel_backup);
            // Mark all rows as dirty so that upon resume, lcd_refresh() 
            // pushes these clean pixels to the physical display.
            for (int r = 0; r < SCREEN_HEIGHT; r++) {
                lcd_buffer[r * 52] = 1u;
            }
        }
        
        // 4. Save the clean state (Logical Registers + Clean Pixels).
        saveCalc(); 
        
        pthread_mutex_unlock(&screenMutex);
        LOGI("saveStateNative: Save complete.");
    }
}

JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_forceRefreshNative(JNIEnv* env, jobject thiz) {
    if (ram) {
        pthread_mutex_lock(&screenMutex);
        refreshScreen(190);
        refreshLcd(NULL);
        lcd_refresh();
        pthread_mutex_unlock(&screenMutex);
    }
}

JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_sendSimFuncNative(JNIEnv* env, jobject thiz, jint funcId) {
    pthread_mutex_lock(&screenMutex);
    extern void runFunction(int16_t id);
    runFunction((int16_t)funcId);
    pthread_mutex_unlock(&screenMutex);
}

JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_sendSimMenuNative(JNIEnv* env, jobject thiz, jint menuId) {
    pthread_mutex_lock(&screenMutex);
    extern void showSoftmenu(int16_t id);
    extern uint8_t screenUpdatingMode;
    extern void refreshScreen(uint16_t reason);
    showSoftmenu((int16_t)menuId);
    refreshScreen(1);
    pthread_mutex_unlock(&screenMutex);
}

JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_sendSimKeyNative(JNIEnv* env, jobject thiz, jstring keyId, jboolean isFn, jboolean isRelease) {
    if (!ram || isCoreBlockingForIo) return;
    const char *nativeKeyId = (*env)->GetStringUTFChars(env, keyId, 0);
    if (nativeKeyId) {
        pthread_mutex_lock(&screenMutex);
        if (isFn) {
            if (isRelease) {
                extern void btnFnClickedR(void *w, void *data);
                btnFnClickedR(NULL, (void*)nativeKeyId);
            } else {
                extern void btnFnClickedP(void *w, void *data);
                btnFnClickedP(NULL, (void*)nativeKeyId);
            }
        } else {
            if (isRelease) {
                extern void btnClickedR(void *w, void *data);
                btnClickedR(NULL, (void*)nativeKeyId);
            } else {
                extern void btnClickedP(void *w, void *data);
                btnClickedP(NULL, (void*)nativeKeyId);
            }
        }
        pthread_mutex_unlock(&screenMutex);
        (*env)->ReleaseStringUTFChars(env, keyId, nativeKeyId);
    }
}

JNIEXPORT jstring JNICALL
Java_com_example_r47_MainActivity_getXRegisterNative(JNIEnv* env, jobject thiz) {
    if (!ram || isCoreBlockingForIo) return (*env)->NewStringUTF(env, "0");
    pthread_mutex_lock(&screenMutex);
    extern char* getXRegisterString();
    char* str = getXRegisterString();
    jstring result = (*env)->NewStringUTF(env, str ? str : "0");
    pthread_mutex_unlock(&screenMutex);
    return result;
}

JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_loadStateNative(JNIEnv* env, jobject thiz) {
    LOGI("loadStateNative triggered");
    if (ram) {
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
        #include "typeDefinitions.h"
        extern programList_t *programList;
        scanLabelsAndPrograms();
        if (currentProgramNumber > 0) {
            defineCurrentProgramFromGlobalStepNumber(currentLocalStepNumber + abs(programList[currentProgramNumber - 1].step) - 1);
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
}

JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_setSlotNative(JNIEnv* env, jobject thiz, jint slot) {
    extern int current_slot_id;
    current_slot_id = slot;
}

JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_sendKey(JNIEnv* env, jobject thiz, jint keyCode) {
    char charKey[4];
    onUIActivity();
    if (keyCode > 0) {
        LOGD("sendKey: DOWN %d", keyCode);
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
    } else {
        LOGD("sendKey: UP (last=%d)", currentPressedKeyCode);
        pthread_mutex_lock(&screenMutex);
        if (currentPressedKeyCode >= 38 && currentPressedKeyCode <= 43) btnFnReleased(NULL, &releaseEvent, currentPressedKeyStr);
        else if (currentPressedKeyCode >= 1 && currentPressedKeyCode <= 37) btnReleased(NULL, &releaseEvent, currentPressedKeyStr);
        pthread_mutex_unlock(&screenMutex);
        currentPressedKeyCode = 0; currentPressedKeyStr[0] = 0;
    }
}

JNIEXPORT jstring JNICALL
Java_com_example_r47_MainActivity_getButtonLabel(JNIEnv* env, jobject thiz, jint keyCode, jint type) {
    if (!ram) return (*env)->NewStringUTF(env, "");
    const calcKey_t *keys = getSystemFlag(FLAG_USER) ? kbd_usr : kbd_std;
    if (keyCode < 1 || keyCode > 37) return (*env)->NewStringUTF(env, "");
    const calcKey_t *key = &keys[keyCode - 1];
    int16_t item = 0;
    switch(type) {
        case 0: item = key->primary; break;
        case 1: item = key->fShifted; break;
        case 2: item = key->gShifted; break;
        case 3: item = key->primaryAim; break; 
    }
    if (item == 0) return (*env)->NewStringUTF(env, "");
    const char *name = indexOfItems[abs(item)].itemSoftmenuName;
    if (!name) return (*env)->NewStringUTF(env, "");
    uint8_t utf8[64]; memset(utf8, 0, sizeof(utf8)); stringToUtf8(name, utf8);
    return (*env)->NewStringUTF(env, (char*)utf8);
}

JNIEXPORT jstring JNICALL
Java_com_example_r47_MainActivity_getSoftkeyLabel(JNIEnv* env, jobject thiz, jint fnKeyIndex) {
    if (!ram || fnKeyIndex < 1 || fnKeyIndex > 6) return (*env)->NewStringUTF(env, "");
    int16_t m = softmenuStack[0].softmenuId;
    if (m < 0 || m >= NUMBER_OF_DYNAMIC_SOFTMENUS) return (*env)->NewStringUTF(env, "");
    int16_t firstItem = softmenuStack[0].firstItem;
    int16_t itemShift = (shiftF ? 6 : (shiftG ? 12 : 0));
    int16_t idx = firstItem + itemShift + (fnKeyIndex - 1);
    char *labelName = (char *)getNthString(dynamicSoftmenu[m].menuContent, idx);
    if (!labelName || labelName[0] == 0) return (*env)->NewStringUTF(env, "");
    uint8_t utf8[64]; memset(utf8, 0, sizeof(utf8)); stringToUtf8(labelName, utf8);
    return (*env)->NewStringUTF(env, (char*)utf8);
}

JNIEXPORT void JNICALL
Java_com_example_r47_MainActivity_getDisplayPixels(JNIEnv* env, jobject thiz, jintArray pixels) {
    if (!screenData) return;
    
    extern bool screenDataDirty;
    if (!screenDataDirty) return;

    if (pthread_mutex_trylock(&screenMutex) != 0) {
        // Core is busy (possibly in a PAUSE loop), skip this frame to keep UI responsive
        return;
    }
    (*env)->SetIntArrayRegion(env, pixels, 0, 400*240, (jint*)screenData);
    screenDataDirty = false;
    pthread_mutex_unlock(&screenMutex);
}

void dmcpResetAutoOff() {}
void rtc_wakeup_delay() {}
void LCD_power_on() {}

void triggerQuit() {
    LOGI("triggerQuit called");
    if (!g_mainActivityObj || !g_jvm) {
        LOGE("triggerQuit: MainActivity or JVM reference is NULL");
        return;
    }
    JNIEnv *env;
    jint res = (*g_jvm)->GetEnv(g_jvm, (void**)&env, JNI_VERSION_1_6);
    if (res == JNI_EDETACHED) {
        LOGI("triggerQuit: Attaching thread to JVM");
        if ((*g_jvm)->AttachCurrentThread(g_jvm, &env, NULL) != JNI_OK) {
            LOGE("triggerQuit: Failed to attach thread");
            return;
        }
    } else if (res != JNI_OK) {
        LOGE("triggerQuit: GetEnv failed");
        return;
    }
    jclass clazz = (*env)->GetObjectClass(env, g_mainActivityObj);
    jmethodID methodId = (*env)->GetMethodID(env, clazz, "quitApp", "()V");
    if (methodId) {
        LOGI("triggerQuit: Calling Java quitApp()");
        (*env)->CallVoidMethod(env, g_mainActivityObj, methodId);
    } else {
        LOGE("triggerQuit: Could not find quitApp method ID");
    }
}

void LCD_power_off(int mode) { 
    (void)mode;
    LOGI("LCD_power_off triggered");
    triggerQuit();
}

void draw_power_off_image(int mode) { (void)mode; }

void pgm_exit(void) {
    LOGI("pgm_exit triggered");
    triggerQuit();
}