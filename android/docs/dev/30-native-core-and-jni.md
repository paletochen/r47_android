# Native Core And JNI

## Native library shape

The Android module builds one shared library: `c47-android`. `MainActivity`
loads it from a static initializer via `System.loadLibrary("c47-android")`.

CMake builds the library from:

- staged core sources under `android/app/src/main/cpp/c47`
- staged decNumber sources under `android/app/src/main/cpp/decNumberICU`
- staged generated sources under `android/app/src/main/cpp/generated`
- Android-specific bridge and HAL files under
  `android/app/src/main/cpp/c47-android`
- staged mini-gmp sources under `android/app/src/main/cpp/gmp`

Tracked Android stub headers under `c47-android/stubs` and the forced include
of `android_mocks.h` let the Android build satisfy upstream GTK, GDK, and Cairo
includes without rewriting staged source files during the Gradle build.

The Android bridge code is intentionally split by responsibility:

- `jni_lifecycle.c` for init, tick, refresh, and slot-state lifecycle work
- `jni_input.c` for key and menu dispatch
- `jni_display.c` for LCD pixels, keypad snapshots, and X-register queries
- `jni_storage.c` for SAF-backed blocking file handoff
- `native-lib.c` for shared JNI bootstrap, registration, and bridge globals

## JNI contract

JNI registration is explicit. `JNI_OnLoad()` initializes the JVM handle, the
recursive `screenMutex`, and calls `register_main_activity_natives(...)`. The
bridge does not rely on name-based native lookup.

The registered native surface includes:

- activity reattachment
- native pre-init, init, and tick
- key, menu, and function dispatch
- state save, load, and force refresh
- LCD pixel transfer
- keypad metadata and label snapshots, including per-label roles such as
  underlined menu-opening faceplate legends
- slot selection and X-register fetch
- SAF file selection callbacks

Development rule:

- Keep the Kotlin external declarations, `JNINativeMethod` table, signatures,
  and implementations aligned in one change.
- Keep app-class lookups and registration failures early in `JNI_OnLoad()` so a
  broken bridge fails at library load time rather than on first use.

## Threading and synchronization

`NativeCoreRuntime` runs the engine loop on a background thread. The JNI bridge
supports that model by keeping shared synchronization in native code:

- `screenMutex` is recursive
- `yieldToAndroidWithMs()` refreshes the LCD, releases the recursive screen
  lock, lets Android process queued work, sleeps briefly, and then reacquires
  the lock
- the bridge can update the current activity reference when the activity is
  recreated
- file I/O handoff uses a condition-based native wait path so the calculator
  core can request a file without inventing a second storage protocol

Practical rule:

- when a native change can block on Android UI or storage, make the lock
  boundaries explicit before changing behavior

## File I/O boundary

`hal/io.c` uses two Android-specific paths:

- a runtime base path set by `set_android_base_path(...)` for app-internal files
  and subdirectories
- SAF handoff for state, program, RTF export, manual save, and related
  user-facing file operations

The SAF path works as follows:

1. Native code calls `requestAndroidFile(...)` with save or load mode, default
   name, and category.
2. Kotlin launches the correct SAF intent through `StorageAccessCoordinator`.
3. The selected file descriptor is detached from the
   `ParcelFileDescriptor` and returned to native code.
4. Native code wraps the descriptor with `fdopen(...)` and continues using
   standard file I/O.

The runtime base path is separate from the user-selected work directory. The
base path supports internal files; the work-directory contract supports user
data organized through SAF.

## JNI change checklist

1. Update the Kotlin external declaration.
2. Update the registered method table.
3. Update the bridge header and the owning C implementation.
4. Recheck thread and lock behavior if the call can touch UI, storage, or long
  native work.

## Change ownership

- For shared calculator behavior, change the canonical root core and restage it
  into the Android tree.
- Change `android/app/src/main/cpp/c47-android` directly only for Android
  bridge, HAL, or stub behavior.
- Do not patch staged upstream C files in place when a tracked Android stub or
  bridge-layer fix can own the compatibility rule.

## 16 KB and packaging contract

The checked-in Android build uses the supported NDK flexible-page-size path:

- `android/app/build.gradle` passes
  `-DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON` to CMake
- the checked-in NDK pin is `29.0.14206865`
- the checked-in AGP version is `8.7.3`

The current checked-in APK target is `arm64-v8a`. Any added prebuilt native
dependency must also satisfy the 16 KB requirements for ELF and APK alignment.

The CI lane verifies that contract by checking zip alignment and native library
`LOAD` segment alignment in the built debug APK.

That artifact verification is the reason packaging changes should be documented
alongside the workflow and Gradle files, not only in the CMake layer.
