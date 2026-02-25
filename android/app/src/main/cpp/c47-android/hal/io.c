#include "c47.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <android/log.h>

#ifndef LOG_TAG
#define LOG_TAG "R47Io"
#endif
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

char android_base_path[512] = "/data/data/com.example.r47/files";
int current_slot_id = 0; // Linked via JNI

static FILE *openedFile = NULL;
extern int requestAndroidFile(int isSave, const char* defaultName, int fileType);
extern void onFileSelectedNative(int fd);
extern void onFileCancelledNative();

void set_android_base_path(const char* path) {
    strncpy(android_base_path, path, 511);
    char buf[1024];
    snprintf(buf, 1024, "%s/%s", path, STATE_DIR);
    mkdir(buf, 0777);
    snprintf(buf, 1024, "%s/%s", path, PROGRAMS_DIR);
    mkdir(buf, 0777);
}

int ioFileOpen(ioFilePath_t path, ioFileMode_t mode) {
    if (openedFile) {
        ioFileClose();
    }

    // Intercept State, Program and Manual Save File operations for Android SAF
    if (path == ioPathSaveStateFile || path == ioPathLoadStateFile ||
        path == ioPathSaveProgram || path == ioPathLoadProgram ||
        path == ioPathExportRTFProgram || path == ioPathSaveAllPrograms ||
        path == ioPathExportRTFAllPrograms || path == ioPathManualSave ||
        path == ioPathPgmFile) {
        
        int isSave = (mode == ioModeWrite);
        char defaultName[256];
        const char* ext = ".s47";
        
        if (path == ioPathSaveProgram || path == ioPathLoadProgram || path == ioPathSaveAllPrograms || path == ioPathPgmFile) {
            ext = ".p47";
        } else if (path == ioPathExportRTFProgram || path == ioPathExportRTFAllPrograms) {
            ext = ".rtf";
        } else if (path == ioPathManualSave) {
            ext = ".sav";
        }

        extern char *tmpStringLabelOrVariableName;
        extern void stringToASCII(const char *in, char *out);
        
        if (path == ioPathSaveStateFile || path == ioPathLoadStateFile) {
            strcpy(defaultName, "state.s47");
        } else if (path == ioPathManualSave) {
            #if (CALCMODEL == USER_R47)
                strcpy(defaultName, "R47.sav");
            #else
                strcpy(defaultName, "C47.sav");
            #endif
        } else {
            // For programs, try to use the current label name
            if (tmpStringLabelOrVariableName && tmpStringLabelOrVariableName[0] != 0) {
                char asciiName[256];
                stringToASCII(tmpStringLabelOrVariableName, asciiName);
                snprintf(defaultName, sizeof(defaultName), "%s%s", asciiName, ext);
            } else {
                snprintf(defaultName, sizeof(defaultName), "program%s", ext);
            }
        }
        
        LOGI("ioFileOpen SAF: path=%d, isSave=%d, defaultName=%s", path, isSave, defaultName);
        
        int category = 1; // Default to PROGRAMS
        if (path == ioPathSaveStateFile || path == ioPathLoadStateFile) {
            category = 0; // STATE
        } else if (path == ioPathManualSave) {
            category = 2; // SAVFILES
        }

        int fd = requestAndroidFile(isSave, defaultName, category);
        if (fd < 0) return FILE_CANCEL;
        
        openedFile = fdopen(fd, isSave ? "wb" : "rb");
        if (!openedFile) {
            close(fd);
            return FILE_ERROR;
        }
        return FILE_OK;
    }

    char fullpath[1024];
    const char* modeStr = (mode == ioModeRead) ? "rb" : ((mode == ioModeWrite) ? "wb" : "r+b");
    
    switch(path) {
        case ioPathManualSave:
            #if (CALCMODEL == USER_R47)
                snprintf(fullpath, 1024, "%s/%s/R47_%d.sav", android_base_path, SAVE_DIR, current_slot_id);
            #else
                snprintf(fullpath, 1024, "%s/%s/C47_%d.sav", android_base_path, SAVE_DIR, current_slot_id);
            #endif
            break;
        case ioPathAutoSave:
            #if (CALCMODEL == USER_R47)
                snprintf(fullpath, 1024, "%s/%s/R47auto_%d.sav", android_base_path, SAVE_DIR, current_slot_id);
            #else
                snprintf(fullpath, 1024, "%s/%s/C47auto_%d.sav", android_base_path, SAVE_DIR, current_slot_id);
            #endif
            break;
        case ioPathBackup:
            #if (CALCMODEL == USER_R47)
                snprintf(fullpath, 1024, "%s/backupR47_%d.cfg", android_base_path, current_slot_id);
            #else
                snprintf(fullpath, 1024, "%s/backup_%d.cfg", android_base_path, current_slot_id);
            #endif
            break;
        case ioPathPgmFile:
        case ioPathSaveProgram:
        case ioPathLoadProgram:
            snprintf(fullpath, 1024, "%s/%s/program.p47", android_base_path, PROGRAMS_DIR);
            break;
        default:
             snprintf(fullpath, 1024, "%s/default.dat", android_base_path);
             break;
    }
    
    if (mode != ioModeRead) {
        char dir[1024];
        if (path == ioPathManualSave || path == ioPathAutoSave) {
            snprintf(dir, 1024, "%s/%s", android_base_path, SAVE_DIR);
            mkdir(dir, 0777);
        } else if (path == ioPathPgmFile || path == ioPathSaveProgram || path == ioPathLoadProgram) {
            snprintf(dir, 1024, "%s/%s", android_base_path, PROGRAMS_DIR);
            mkdir(dir, 0777);
        }
    }

    openedFile = fopen(fullpath, modeStr);
    return openedFile ? FILE_OK : FILE_ERROR;
}

void ioFileWrite(const void *buffer, uint32_t size) {
    if (openedFile) fwrite(buffer, 1, size, openedFile);
}

uint32_t ioFileRead(void *buffer, uint32_t size) {
    if (openedFile) return fread(buffer, 1, size, openedFile);
    return 0;
}

void ioFileSeek(uint32_t position) {
    if (openedFile) fseek(openedFile, position, SEEK_SET);
}

void ioFileClose(void) {
    if (openedFile) {
        fflush(openedFile);
        // On Android, fsync ensures the SAF file is actually written
        fsync(fileno(openedFile));
        fclose(openedFile);
        openedFile = NULL;
    }
}

int ioEof(void) {
    return openedFile ? feof(openedFile) : 1;
}

int ioFileRemove(ioFilePath_t path, uint32_t *errorNumber) {
    return FILE_OK;
}

int save_statefile(const char * fpath, const char * fname, void * data) { return MRET_SAVESTATE; }
int load_statefile(const char * fpath, const char * fname, void * data) { return MRET_LOADSTATE; }
int save_programfile(const char * fpath, const char * fname, void * data) { return MRET_SAVESTATE; }
int load_programfile(const char * fpath, const char * fname, void * data) { return MRET_LOADSTATE; }
void show_warning(char *string) {}
void fnDiskInfo(uint16_t unused) {}