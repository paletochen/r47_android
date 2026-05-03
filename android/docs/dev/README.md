# Android Development Docs

These pages describe the current Android development contract for the checked-in
R47 shell.

They are code-facing development notes, not end-user usage docs.

Read in this order:

- [10-build-and-source-layout.md](10-build-and-source-layout.md): toolchain,
  source ownership, build entry points, and CI.
- [20-kotlin-shell-architecture.md](20-kotlin-shell-architecture.md):
  lifecycle, runtime ownership, input flow, storage, and Kotlin-side structure.
- [30-native-core-and-jni.md](30-native-core-and-jni.md): CMake, JNI,
  synchronization, HAL I/O, and packaging constraints.
- [40-ui-rendering-and-gtk-mapping.md](40-ui-rendering-and-gtk-mapping.md):
  shell projection, keypad scene data, and GTK-derived rendering rules.
- [90-official-references.md](90-official-references.md): official Android,
  NDK, Kotlin, storage, and view-system references.

The CI lane follows the same ownership model as the local build: resolve one
authoritative upstream core revision, run the root simulator tests, build the
debug APK, then publish a main-branch snapshot prerelease from the generated
artifact bundle.

Two rules govern most Android work in this repository:

1. The preferred source of truth for shared calculator behavior is the synced
   root tree plus generated outputs from `build.sim`.
2. `android/app/src/main/cpp` is the staged Android native input tree that CMake
   builds. Refresh it through `android/stage_native_sources.sh`.

If a change crosses both Kotlin and native boundaries, read the build page and
the JNI page before editing.
