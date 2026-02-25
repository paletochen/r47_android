#ifndef ANDROID_MOCKS_H
#define ANDROID_MOCKS_H

#if defined(ANDROID_BUILD)

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <pthread.h>
#include <android/log.h>

#define LOG_TAG "R47Native"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

// GMP needs to be here for stubs to be visible
#include "gmp.h"

// GMP Stubs
#ifndef mpz_div_2exp
#define mpz_div_2exp mpz_tdiv_q_2exp
#endif
#ifndef mpz_fits_uint_p
#define mpz_fits_uint_p mpz_fits_ulong_p
#endif

static inline void mpz_nextprime(mpz_ptr r, mpz_srcptr n) {
    mpz_add_ui(r, n, 1);
    while (mpz_probab_prime_p(r, 10) == 0) mpz_add_ui(r, r, 1);
}
static inline void mpz_fib_ui(mpz_ptr r, unsigned long n) {
    if (n == 0) { mpz_set_ui(r, 0); return; }
    if (n == 1) { mpz_set_ui(r, 1); return; }
    mpz_t a, b;
    mpz_init_set_ui(a, 0); mpz_init_set_ui(b, 1);
    for (unsigned long i = 2; i <= n; i++) { mpz_add(a, a, b); mpz_swap(a, b); }
    mpz_set(r, b); mpz_clear(a); mpz_clear(b);
}

// GTK/GDK/Cairo Mocks
typedef void GtkWidget;
typedef struct {
    int type;
    struct {
        int button;
        double x; 
        double y; 
    } button;
} GdkEvent;
typedef void cairo_t;
typedef void cairo_surface_t;
typedef void GtkClipboard;
typedef void* gpointer;
typedef int gboolean;
typedef long long gint64;
typedef void* GSourceFunc;
typedef void* GMainContext;

#ifndef TRUE
  #define TRUE 1
#endif
#ifndef FALSE
  #define FALSE 0
#endif

// GDK Enums
#define GDK_BUTTON_PRESS 4
#define GDK_2BUTTON_PRESS 5
#define GDK_3BUTTON_PRESS 6
#define GDK_BUTTON_RELEASE 7
#define GDK_DOUBLE_BUTTON_PRESS GDK_2BUTTON_PRESS
#define GDK_TRIPLE_BUTTON_PRESS GDK_3BUTTON_PRESS
#define GDK_SELECTION_CLIPBOARD 0
#define CAIRO_FORMAT_RGB24 0

// GTK/GDK/Cairo Function Mocks
extern void triggerQuit(void);
#define gtk_main_quit() triggerQuit()
#define gtk_widget_queue_draw(...) (void)0
#define gtk_events_pending(...) 0
#define gtk_main_iteration(...) (void)0
#define gtk_clipboard_get(...) NULL
#define gtk_clipboard_set_text(...) (void)0
#define gtk_clipboard_clear(...) (void)0
#define gtk_clipboard_set_image(...) (void)0
#define gdk_pixbuf_get_from_surface(...) NULL
#define cairo_image_surface_create_for_data(...) NULL
#define cairo_set_source_surface(...) (void)0
#define cairo_surface_mark_dirty(...) (void)0
#define cairo_paint(...) (void)0
#define cairo_surface_destroy(...) (void)0
#define cairo_scale(...) (void)0
#define cairo_pattern_set_filter(...) (void)0
#define cairo_get_source(...) NULL

#define GTK_TOGGLE_BUTTON(...) NULL
#define GTK_LABEL(...) NULL
#define gtk_label_set_label(...) (void)0
#define gtk_toggle_button_set_active(...) (void)0
#define gtk_toggle_button_get_active(...) 0
#define gtk_widget_show(...) (void)0
#define gtk_widget_hide(...) (void)0
#define gdk_threads_add_timeout(...) 0

#define g_timeout_add(...) 0
#define g_main_context_iteration(...) 0
#define g_main_context_default(...) NULL

extern void processCoreTasksNative(void);
extern void yieldToAndroid(void);
extern uint32_t sys_current_ms(void);
extern gint64 g_get_monotonic_time(void);
extern gint64 g_get_real_time(void);
extern int requestAndroidFile(int isSave, const char* defaultName, int fileType);
extern pthread_mutex_t coreMutex;
extern pthread_mutex_t screenMutex;

// Globals needed by core - MATCHING TYPES IN c47.h
extern int16_t debugWindow;
extern int16_t screenStride;
extern bool    debugMemAllocation;
extern bool    forceTamAlpha;
extern uint32_t deadKey;
extern uint32_t *screenData;

#endif // ANDROID_BUILD

#endif // ANDROID_MOCKS_H