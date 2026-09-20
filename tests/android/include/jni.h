#ifndef _JNI_H
#define _JNI_H

#include <stdint.h>
#include <stddef.h>

typedef uint8_t  jboolean;
typedef int8_t   jbyte;
typedef uint16_t jchar;
typedef int16_t  jshort;
typedef int32_t  jint;
typedef int64_t  jlong;
typedef float    jfloat;
typedef double   jdouble;
typedef jint     jsize;

typedef void* jobject;
typedef jobject jclass;
typedef jobject jstring;
typedef jobject jarray;
typedef jobject jintArray;
typedef jobject jobjectArray;
typedef jobject jthrowable;
typedef void* jmethodID;
typedef void* jfieldID;

#define JNI_FALSE 0
#define JNI_TRUE  1
#define JNI_OK    0
#define JNI_ERR   (-1)
#define JNI_EDETACHED (-2)
#define JNI_VERSION_1_6 0x00010006

typedef struct JNINativeMethod {
    const char* name;
    const char* signature;
    void*       fnPtr;
} JNINativeMethod;

struct JNIInvokeInterface;
struct JNINativeInterface;

typedef const struct JNINativeInterface* JNIEnv;
typedef const struct JNIInvokeInterface* JavaVM;

struct JNIInvokeInterface {
    void* reserved0;
    void* reserved1;
    void* reserved2;
    jint (*DestroyJavaVM)(JavaVM*);
    jint (*AttachCurrentThread)(JavaVM*, JNIEnv**, void*);
    jint (*DetachCurrentThread)(JavaVM*);
    jint (*GetEnv)(JavaVM*, void**, jint);
    jint (*AttachCurrentThreadAsDaemon)(JavaVM*, JNIEnv**, void*);
};

struct JNINativeInterface {
    void* reserved0;
    void* reserved1;
    void* reserved2;
    void* reserved3;
    jint (*GetVersion)(JNIEnv *);
    jclass (*FindClass)(JNIEnv *, const char *);
    jint (*RegisterNatives)(JNIEnv *, jclass, const JNINativeMethod *, jint);
    jint (*UnregisterNatives)(JNIEnv *, jclass);
    jstring (*NewStringUTF)(JNIEnv *, const char *);
    const char* (*GetStringUTFChars)(JNIEnv *, jstring, jboolean *);
    void (*ReleaseStringUTFChars)(JNIEnv *, jstring, const char *);
    jobject (*NewGlobalRef)(JNIEnv *, jobject);
    void (*DeleteGlobalRef)(JNIEnv *, jobject);
    void (*DeleteLocalRef)(JNIEnv *, jobject);
    void (*CallVoidMethod)(JNIEnv *, jobject, jmethodID, ...);
    jint (*CallIntMethod)(JNIEnv *, jobject, jmethodID, ...);
    jobject (*CallObjectMethod)(JNIEnv *, jobject, jmethodID, ...);
};

#define JNIEXPORT
#define JNICALL

#endif // _JNI_H
