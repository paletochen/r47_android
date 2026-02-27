#!/bin/bash

# =============================================================================
# R47 Android Build Script
# =============================================================================

# --- 1. Setup Variables ---
# Ensure Gradle is in path if installed via SDKMAN, but don't hardcode user home
if [ -d "$HOME/.sdkman/candidates/gradle/current/bin" ]; then
    export PATH="$HOME/.sdkman/candidates/gradle/current/bin:$PATH"
fi

if [ -n "$JAVA_HOME" ] && [ ! -d "$JAVA_HOME" ]; then
    unset JAVA_HOME
fi

if [ -z "$JAVA_HOME" ]; then
    for jvm in /usr/lib/jvm/java-17-openjdk-amd64 /usr/lib/jvm/java-21-openjdk-amd64 /usr/lib/jvm/default-java; do
        if [ -d "$jvm" ]; then
            export JAVA_HOME="$jvm"
            echo "Detected JAVA_HOME: $JAVA_HOME"
            break
        fi
    done
fi

PROJECT_ROOT="$(pwd)"
export ANDROID_SDK_ROOT=${ANDROID_SDK_ROOT:-$HOME/Android/Sdk}

# --- NDK Version Selection ---
# 1. Check for Environment Override
# 2. Check for Gradle Property in android/gradle.properties
# 3. Extract Default from build.gradle
# 4. Fallback to latest installed
IF_NDK_VERSION=${R47_NDK_VERSION}
if [ -z "$IF_NDK_VERSION" ]; then
    IF_NDK_VERSION=$(grep "r47.ndkVersion=" "$PROJECT_ROOT/android/gradle.properties" 2>/dev/null | cut -d'=' -f2)
fi
if [ -z "$IF_NDK_VERSION" ]; then
    IF_NDK_VERSION=$(grep "ndkVersion" "$PROJECT_ROOT/android/app/build.gradle" | grep -o '".*"' | sed 's/"//g')
fi

if [ -n "$IF_NDK_VERSION" ] && [ -d "$ANDROID_SDK_ROOT/ndk/$IF_NDK_VERSION" ]; then
    echo "Using detected NDK version: $IF_NDK_VERSION"
    export ANDROID_NDK_ROOT="$ANDROID_SDK_ROOT/ndk/$IF_NDK_VERSION"
else
    # Fallback to latest available NDK
    LATEST_NDK=$(ls -1 "$ANDROID_SDK_ROOT/ndk" 2>/dev/null | sort -V | tail -n1)
    if [ -n "$LATEST_NDK" ]; then
        echo "NDK $IF_NDK_VERSION not found. Falling back to latest: $LATEST_NDK"
        export ANDROID_NDK_ROOT="$ANDROID_SDK_ROOT/ndk/$LATEST_NDK"
    else
        echo "ERROR: No NDK found in $ANDROID_SDK_ROOT/ndk"
        exit 1
    fi
fi

export ANDROID_HOME=$ANDROID_SDK_ROOT
export ANDROID_NDK_HOME=$ANDROID_NDK_ROOT

ANDROID_PROJECT_DIR="$PROJECT_ROOT/android"
CPP_DIR="$ANDROID_PROJECT_DIR/app/src/main/cpp"

echo "======================================================="
echo "R47 Android Builder"
echo "SDK: $ANDROID_SDK_ROOT"
echo "NDK: $ANDROID_NDK_ROOT"
echo "======================================================="

# --- 2. Update Version from Upstream (c43 Core) ---
# We pull the hash from SwissMicros. 
# Public repo uses 'upstream', Private repo uses 'origin'.
COMMIT_HASH=$(git rev-parse --short upstream/master 2>/dev/null || git rev-parse --short origin/master 2>/dev/null || echo "unknown")
echo "--- SwissMicros Core Version: $COMMIT_HASH ---"
PREFS_FILE="$ANDROID_PROJECT_DIR/app/src/main/res/xml/root_preferences.xml"
if [ -f "$PREFS_FILE" ]; then
    # Update the summary for the about_version preference
    # We look for the block containing about_version and update the next app:summary line
    sed -i "/app:key=\"about_version\"/,/app:summary/ s/app:summary=\".*\"/app:summary=\"R47 Android Port ($COMMIT_HASH)\"/" "$PREFS_FILE"
else
    echo "WARNING: Could not find $PREFS_FILE to update version."
fi

# --- 3. Generate Assets (make sim) ---
echo "--- Generating Core Assets (running make sim) ---"
if [ -d "build.sim" ] && [ ! -f "build.sim/build.ninja" ]; then
    rm -rf build.sim
fi

make sim

if [ $? -ne 0 ]; then
    echo "ERROR: 'make sim' failed."
    exit 1
fi

# --- 3. Copy Source Code & Assets ---
echo "--- Copying Source Code ---"
mkdir -p "$CPP_DIR"

# Copy C47 Core
rm -rf "$CPP_DIR/c47"
mkdir -p "$CPP_DIR/c47"
cp -r src/c47/* "$CPP_DIR/c47/"

# PATCH Headers for Android Compatibility
echo "--- Patching headers for Android ---"

# 1. Disable GTK/Cairo blocks in all files
echo "--- Disabling GTK/Cairo blocks ---"
find "$CPP_DIR/c47" -type f \( -name "*.h" -o -name "*.c" \) -exec sed -i 's|#include <gtk/gtk.h>|#ifndef ANDROID_BUILD\n#include <gtk/gtk.h>\n#endif|g' {} + 
find "$CPP_DIR/c47" -type f \( -name "*.h" -o -name "*.c" \) -exec sed -i 's|#include <gdk/gdk.h>|#ifndef ANDROID_BUILD\n#include <gdk/gdk.h>\n#endif|g' {} + 
find "$CPP_DIR/c47" -type f \( -name "*.h" -o -name "*.c" \) -exec sed -i 's|#include <cairo.h>|#ifndef ANDROID_BUILD\n#include <cairo.h>\n#endif|g' {} + 

# 2. Resolve signature conflicts in headers
echo "--- Resolving signature conflicts in headers ---"
# Headers are now standardized in src/c47/*.h, no sed needed.

# 3. Resolve signature conflicts in implementations (timer.c, screen.c)
echo "--- Resolving signature conflicts in implementations ---"
# Handled by standardization in src/c47/*.c

# 4. Fix function call signatures
echo "--- Fixing function call signatures ---"
# Standardized to refreshLcd(NULL) / refreshTimer(NULL) in source, no sed needed.

# 5. Disable problematic GTK code in softmenus.c
sed -i 's|gtk_widget_queue_draw(screen);|// stub|g' "$CPP_DIR/c47/softmenus.c"
sed -i 's|while(gtk_events_pending())|if(0)|g' "$CPP_DIR/c47/softmenus.c"

# Copy decNumberICU
rm -rf "$CPP_DIR/decNumberICU"
cp -r dep/decNumberICU "$CPP_DIR/"

# Copy Generated Files
GENERATED_DEST="$CPP_DIR/generated"
mkdir -p "$GENERATED_DEST"
cp -v build.sim/src/generateCatalogs/softmenuCatalogs.h "$GENERATED_DEST/"
cp -v build.sim/src/generateConstants/constantPointers.h "$GENERATED_DEST/"
cp -v build.sim/src/generateConstants/constantPointers.c "$GENERATED_DEST/"
cp -v build.sim/src/ttf2RasterFonts/rasterFontsData.c    "$GENERATED_DEST/"
cp -v build.sim/src/c47/vcs.h     "$GENERATED_DEST/"
cp -v build.sim/src/c47/version.h "$GENERATED_DEST/"

# R47 Port: Overwrite vcs.h with the actual Core Hash to maintain SwissMicros identity
echo "--- Injecting Core Hash into vcs.h ---"
cat <<EOF > "$GENERATED_DEST/vcs.h"
#if !defined(VCS_H)
  #define VCS_H
  #define VCS_COMMIT_ID  "$COMMIT_HASH-mod"
#endif
EOF

# Copy Assets (Textures and Fonts)
echo "--- Copying Assets ---"
DRAWABLE_DIR="$ANDROID_PROJECT_DIR/app/src/main/res/drawable"
mkdir -p "$DRAWABLE_DIR"
# Note: r47_texture.png is managed within the Android project; do not overwrite from core.

ASSETS_FONTS_DIR="$ANDROID_PROJECT_DIR/app/src/main/assets/fonts"
mkdir -p "$ASSETS_FONTS_DIR"
cp -v res/fonts/*.ttf "$ASSETS_FONTS_DIR/"

# Copy GMP (mini-gmp)
echo "--- Setting up GMP ---"
mkdir -p "$CPP_DIR/gmp"
GMP_SRC_DIR="$PROJECT_ROOT/subprojects/gmp-6.2.1/mini-gmp"
if [ -d "$GMP_SRC_DIR" ]; then
    echo "Found mini-gmp at $GMP_SRC_DIR"
    cp "$GMP_SRC_DIR/mini-gmp.c" "$CPP_DIR/gmp/"
    cp "$GMP_SRC_DIR/mini-gmp.h" "$CPP_DIR/gmp/gmp.h"
    # Patch mini-gmp.c to include gmp.h instead of mini-gmp.h
    sed -i 's|#include "mini-gmp.h"|#include "gmp.h"|g' "$CPP_DIR/gmp/mini-gmp.c"
else
    echo "ERROR: Could not locate mini-gmp at $GMP_SRC_DIR. Build will likely fail."
fi

# Create local.properties
echo "sdk.dir=$ANDROID_SDK_ROOT" > "$ANDROID_PROJECT_DIR/local.properties"
# Note: ndk.dir is deprecated; we pass the version via Gradle property instead.

cd "$ANDROID_PROJECT_DIR"

# --- 4. Build APK ---
echo "--- Building APK ---"
GRADLE_CMD="./gradlew"

if [ ! -f "$GRADLE_CMD" ]; then
    gradle wrapper --gradle-version 8.2 --distribution-type bin
fi
chmod +x "$GRADLE_CMD"

# Pass detected NDK/SDK versions as Project Properties to override build.gradle defaults
GRADLE_PROPS="-Pr47.ndkVersion=$IF_NDK_VERSION"
if [ -n "$R47_COMPILE_SDK" ]; then GRADLE_PROPS="$GRADLE_PROPS -Pr47.compileSdk=$R47_COMPILE_SDK"; fi

# Clean cxx to ensure fresh cmake run
rm -rf app/.cxx
$GRADLE_CMD clean
$GRADLE_CMD assembleDebug $GRADLE_PROPS

APK_PATH="app/build/outputs/apk/debug/R47calculator-debug.apk"
if [ -f "$APK_PATH" ]; then
    echo "SUCCESS: APK created at: $ANDROID_PROJECT_DIR/$APK_PATH"
else
    echo "ERROR: APK build failed."
    exit 1
fi
