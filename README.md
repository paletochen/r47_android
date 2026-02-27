# R47 Android Port: RPN Calculator

This repository provides a photographic Android port of the [SwissMicros C43/C47 project](https://gitlab.com/rpncalculators/c43.git). It runs the high-performance mathematical core of the C43 via a native JNI wrapper.

## 🎯 Features
- **Photographic Replica**: High-fidelity UI matching the physical hardware from SwissMicros (R47).
- **Native Performance**: Math engine runs in C via JNI for 100% parity with the hardware.
- **Modern Android Support**: Optimized for high-refresh screens and 16KB page sizes (Android 15+).
- **SAF Integration**: Full support for Android's Storage Access Framework for programs and state files.

## 🛠️ Linux Dependencies
To build the simulator or the Android APK on Linux (Ubuntu/Debian), you will need the following:

### 1. Build Tools & Math Libraries
```bash
sudo apt-get install git build-essential meson ninja-build libgmp-dev libgtk-3-dev libpulse-dev
```

**NOTE:** Please refer to the [Build instructions](https://gitlab.com/h2x/c47-wiki/-/wikis/Build-instructions) in the [C47-wiki](https://gitlab.com/h2x/c47-wiki/-/wikis/home) for further details on how to set up a development environment in Macos, Linux, Windows. 

### 2. Android Development
- **Java**: OpenJDK 17 or higher.
- **Android SDK & NDK**: Version 26.1.10909125 or newer recommended.

### 3. Firmware Cross-Compilation (Optional)
```bash
sudo apt-get install gcc-arm-none-eabi
```

## 🚀 Getting Started

### 1. Clone & Sync
The repository contains the Android shell and build blueprints. You must run the sync script to pull the mathematical core from C43 GitLab.
```bash
git clone https://github.com/paletochen/r47_android.git
cd r47_android
chmod +x sync_public.sh
./sync_public.sh
```

### 2. Build the Simulator (PC)
```bash
./dist.sh
```
This builds the GTK-based simulators for Linux/macOS and the firmware files for the physical DM42/DM42n hardware.

### 3. Build the Android App (APK)
Ensure `ANDROID_SDK_ROOT` is set in your environment.
```bash
./build_android.sh
```
The resulting APK will be located in `android/app/build/outputs/apk/debug/`.

### 🛠️ Advanced Build Configuration
If you have different versions of the Android SDK, NDK, or CMake installed (common on Windows), you can override the defaults. Depending on your operating system and terminal, some methods may be more reliable than others.

#### Option 1: Using gradle.properties (Most Reliable / Cross-Platform)
Create or edit `android/gradle.properties`. This method works on all systems (Linux, macOS, Windows) and is the recommended approach:
```properties
r47.compileSdk=35
r47.targetSdk=35
r47.ndkVersion=29.0.14206865
r47.cmakeVersion=3.28.1
```

#### Option 2: Environment Variables (Linux / macOS / WSL)
You can pass the NDK version as an environment variable before running the build script:
```bash
export R47_NDK_VERSION="29.0.14206865"
./build_android.sh
```

#### Option 3: PowerShell / CMD (Windows Native)
If you are running from a native Windows terminal without WSL or Git Bash:
**PowerShell:**
```powershell
$env:R47_NDK_VERSION="29.0.14206865"; ./build_android.sh
```
**CMD:**
```cmd
set R47_NDK_VERSION=29.0.14206865 && ./build_android.sh
```

#### Option 4: Manual local.properties (Alternative for Windows)
Some Windows users have reported success by manually appending the version to `local.properties`:
```cmd
echo ndkVersion=29.0.14206865 >> android/local.properties
```

## 📜 Acknowledgments
Based on the excellent work by the [WP43/C47 team](https://gitlab.com/rpncalculators/c43) and [SwissMicros](https://www.swissmicros.com/). This port is an independent community contribution.
