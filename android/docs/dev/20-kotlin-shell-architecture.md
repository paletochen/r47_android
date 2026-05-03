# Kotlin Shell Architecture

## Top-level model

The Android app is a single-activity, view-based shell around a long-lived
native core. `MainActivity` coordinates Android lifecycle, preferences, slot
selection, settings, and external integrations. It does not own the calculator
engine loop.

## Ownership by class

- `MainActivity`: loads `c47-android`, binds the layout, wires helper types,
  forwards input to the core runtime, and exposes the native methods used by
  JNI.
- `NativeCoreRuntime`: owns the persistent core thread, the shared task queue,
  frame-driven LCD refresh, and keypad snapshot refresh.
- `SlotStore`: owns slot metadata persistence and the current-slot preference.
- `AudioEngine`: owns Android-side tone playback and runtime beeper settings.
- `StorageAccessCoordinator`: owns SAF create and open launchers, native file
  request handoff, and work-directory validation on resume.
- `WorkDirectory`: stores the persisted tree URI and resolves the `STATE`,
  `PROGRAMS`, `SAVFILES`, and `SCREENS` subfolders.
- `DisplayActionController`: owns display long-press actions such as copy X
  register, paste number, and entry into Picture-in-Picture.
- `ReplicaOverlay` and `ReplicaKeypadLayout`: host the shell chrome modes, the
  normalized shared keypad touch grid, LCD projection, the classic
  `r47_texture` image-backed shell, the full scene-driven key views used by
  `native`, the scene-driven label overlay used by `r47_background`, the
  shared settings-entry strip, and the PiP interaction surface.
- `KeypadTopology`: owns the Android-local 43-key row, lane, and family
  contract used by layout, touch-grid row membership, and per-key family
  policy.
- `SettingsActivity`: owns the settings UI and preference-driven Android shell
  options.

## Model boundary

- `KeypadSnapshot` is the Kotlin-side projection of the native keypad scene.
- `KeypadTopology` is the Android-local keypad topology contract for key-code
  row order, family, and touch-grid lane membership. It is consumed by
  `ReplicaKeypadLayout` and `CalculatorKeyView`, not exported by the native
  calculator core.
- slot metadata is an Android model owned by `SlotStore`, not by the native
  calculator core.
- preference state controls Android shell behavior such as fullscreen mode,
  chrome mode, scaling mode, haptics, beeper volume, and touch-zone debugging.
  `chrome_mode` now chooses between the default `r47_texture` shell, the
  `r47_background` background-backed shell, and the native-drawn shell. All
  three modes share the same logical touch grid, the same texture-derived LCD
  placement, and the same adaptive visible-frame crop in `full_width`.

## Runtime and event flow

Calculator state does not live in `Activity` fields alone. The durable state
owner is the native core plus `NativeCoreRuntime`'s shared thread and task queue.

Main flow:

1. Android touch, keyboard, or menu events call `offerCoreTask(...)` or direct
   bridge methods exposed by `MainActivity`.
2. `NativeCoreRuntime` serializes native work on the core thread.
3. The frame callback pulls LCD pixels and keypad metadata from native code.
4. `MainActivity.currentKeypadSnapshot()` converts native keypad arrays into a
   `KeypadSnapshot`.
5. `ReplicaOverlay` and `CalculatorKeyView` redraw from that snapshot.

This keeps the UI reactive while the engine remains on one native execution path.

`NativeCoreRuntime` is also the place where LCD refresh cadence and keypad
metadata polling stay coordinated, so do not duplicate refresh loops elsewhere
in the shell.

## Lifecycle contract

- `onCreate()` wires the overlay, helpers, keypad, and native runtime, then
  attaches the core thread.
- `onResume()` requests a native refresh and revalidates the work-directory
  contract.
- `onPause()` performs a synchronous native save when auto-save on minimize is
  enabled and the app is not moving into PiP.
- `onDestroy()` only stops the shared runtime when the activity is actually
  finishing.
- `onPictureInPictureModeChanged()` switches the overlay between normal shell
  mode and PiP mode.

## Input surfaces

The Kotlin shell currently accepts input from four paths:

- on-screen keys built by `ReplicaKeypadLayout`
- physical keyboard mappings handled in `MainActivity`
- display long-press actions coordinated by `DisplayActionController`
- PiP touch mapping handled by `ReplicaOverlay`

Each path ultimately resolves to core-thread work or a small Android-side action.

## Persistence and slot model

- slot metadata lives in `SlotStore`
- the active slot ID is mirrored into native code through `setSlotNative(...)`
- slot switching follows one ordered background flow: save current state, update
  slot ID, load target state
- SAF-backed user files remain outside the internal app directory contract
- the work-directory contract is a user-facing SAF tree, not a replacement for
  the app-internal native base path

## Current platform shape

- the app is view-based and uses view binding
- `MainActivity` is portrait-only in the manifest
- Picture-in-Picture is enabled
- settings live in a separate `SettingsActivity`
- haptics, audio, fullscreen state, scaling mode, and touch-zone overlays are
  preference-driven Android concerns

## Kotlin-side change rules

- Keep `MainActivity` as a coordinator, not as a second calculator engine.
- Keep blocking work off the main thread.
- Add new Android integrations through focused helper types when they do not
  belong in `MainActivity`.
- When adding a native call, update the Kotlin external declaration and the JNI
  registration table together.
- When a feature needs both Android lifecycle logic and native execution,
  prefer a small Kotlin coordinator plus core-thread work instead of splitting
  state across both sides.
- If a new feature needs durable calculator state, decide first whether the true
  owner is the native core, Android preferences, or Android-side slot metadata.
