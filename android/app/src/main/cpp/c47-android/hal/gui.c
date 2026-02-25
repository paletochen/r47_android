#include "c47.h"
#include <stdlib.h>
#include <string.h>
#include <android/log.h>

#define LOG_TAG "R47Gui"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

void calcModeNormalGui (void) {}
void calcModeAimGui    (void) {}
void calcModeTamGui    (void) {}

// Android specific implementation of setupUI
// This must handle the core allocations that GTK version does in setupUI.
void setupUI(void) {
    LOGI("setupUI: start");
    // screenData is now handled in init_lcd_buffers
    
    // We let doFnReset handle the main RAM and buffer allocations
    // because its logic for splitting errorMessage/aimBuffer is complex.
    LOGI("setupUI: done (buffers will be handled by doFnReset)");
}