# AGENTS.md

Guidance for AI coding agents working in this repository.
For how to write a new automation program, see also `CodingAgentContext/AutomationProgramPatterns.md`.

## What this project is

This is the Pokémon Automation Computer Control (CC) program. It is an open-source
application that automates Pokémon (and other) games on Nintendo Switch using microcontroller
controller hardware, computer vision, OCR, and machine learning.

## Repo layout

- `Common/` — shared code. `Common/Cpp` is Qt-free standard C++ (the foundation of the
  project; keep it Qt-free except documented seams). `Common/Qt` holds Qt-specific helpers.
- `SerialPrograms/` — the application: `SerialPrograms/Source` (code), `SerialPrograms/cmake`
  (CMake modules), `SerialPrograms/bin` (build output).
- `Packages/` — **not part of this repo**: the separate `PokemonAutomation/Packages` repo
  (downloadable resources in `Packages/DownloadedResources`, firmware, tooling). CMake clones
  it into `Packages/` automatically at configure time if it is missing (`clone_packages`).
- `3rdParty/`, `3rdParty-Core/` — vendored dependencies (compiled as external includes; MSVC
  builds use `/external:W0`).
- `CodingAgentContext/` — agent-facing notes on writing new automation programs.
- Test images/fixtures live in a **separate repo**: `PokemonAutomation/CommandLineTests`.
  At test time it is checked out next to the program; `GlobalAutoPaths` detects the
  `CommandLineTests/` (or `UnitTestResources/`) folder and loads test files relative to it.

## Programs vs. infra

The codebase splits into **programs** (individual automation programs) and **infra**
(everything else: `CommonFramework`, `CommonTools`, `Common/Cpp`, `Controllers`, ...).
Programs live in:

- `SerialPrograms/Source/VideoGames/`
- `SerialPrograms/Source/Pokemon/` (not yet moved into `VideoGames/`)
- `SerialPrograms/Source/PokemonBDSP/` (not yet moved into `VideoGames/`)
- Console-generic (non-game-specific) programs under `SerialPrograms/Source/NintendoSwitch/`:
  its `Inference/`, `Options/`, `Programs/`, and `DevPrograms/` subfolders are programs;
  the rest of that folder (`Commands/`, `Controllers/`, `Framework/`, console state/handles, ...)
  is infra.

Infra has a much higher quality standard than programs because of its large blast radius,
and it requires strict human review. New AI-generated code must meet that bar; filler and
low-quality "AI slop" are not acceptable.

## Hard rules

### Qt

The long-term goal is to reduce the Qt footprint in preparation for a future migration away
from Qt.

- Qt is **forbidden** in program code (the three program folders above).
- Qt is **strongly discouraged** in non-UI infra. Use it only when the alternative is a lot
  of new code or would pull in a library the project doesn't already use.
- Qt is generally **unavoidable** for UI code (widgets, panels, windows).

### Filesystem

Direct use of `std::filesystem` is **banned**: its native encoding is ambiguous (UTF-8 bytes
on Linux, UTF-16 on Windows), while this project defines `char*`/`std::string` as UTF-8.
Use the wrappers in `Common/Cpp/Filesystem/` instead: `Filesystem::` functions, `Path`,
`FileIO`, `file_to_string()`. Rationale is documented in the header comment of
`Common/Cpp/Filesystem/FilePath.h`.

### Concurrency (Qt thread-adoption bug on Windows)

Qt > 6.8.3 on Windows can hang when raw `std::thread`s coexist with Qt event loops
("thread adoption" bug: QTBUG-131892, see project issue #570; see
`Common/Cpp/Concurrency/Qt6.9ThreadBugWorkaround.h`). Therefore:

- Don't use `std::thread` manually, and don't use `Common/Cpp/Concurrency/Thread`
  directly either: `Thread` leaks the thread (a consequence of the Qt adoption-bug
  workaround), so only the thread pool implementations are allowed to use it. Pools
  reuse their worker threads, which keeps the leaked-thread count down to the peak
  number of concurrent tasks (the pools still leak all of their threads on exit).
  Run all background work on the app-level pools in
  `SerialPrograms/Source/CommonFramework/Tools/GlobalThreadPools.h`.
- Don't use `std::mutex` / `std::condition_variable` directly. Use the
  `PokemonAutomation::Mutex` / `ConditionVariable` wrappers in `Common/Cpp/Concurrency/`.
- `std::async` is banned (it may spawn a new thread under the hood).

Some legacy files still use `std::thread`/`std::mutex` directly; don't add new uses, and
migrate a file to the wrappers when you're already touching it and it's straightforward.

### UI / state separation

- UI state lives in a *session* (or `UiState`) class — e.g. `PanelSession` (a program
  instance), `VideoOverlaySession`, or the `UiState<...>` option classes. These hold **all**
  program data and logic and are fully thread-safe/asynchronous.
- The actual UI lives in `Widget` classes that attach to the session. The session always
  outlives the UI. Keep state in the session; widgets never own program state.
- The indirection is `Common/Cpp/UiWrapper.h`: state classes inherit `UiState<T>` and the UI
  is produced by a factory registered in `UiState<T>::m_ui_factory` (called via
  `make_ui_component()`). This lets state compile and run with no UI at all (headless) and
  keeps the door open for migrating to a different UI library.

## Building and testing

- CMake >= 3.24, C++23. Dependencies: Qt 6.10.2 (with `qtmultimedia` + `qtserialport`),
  OpenCV 4, ONNX Runtime, Tesseract. Linux CI builds with Ninja; Windows with MSVC.
- Configure/build from `SerialPrograms/` with the CMake presets in
  `SerialPrograms/CMakePresets.json` (Ninja, C++23, automoc on), e.g. `cmake --preset default`
  then `cmake --build --preset default`; platform-specific setup is documented in
  `SerialPrograms/BuildInstructions/`.
- `-DPA_CORE_ONLY=ON` builds only the GUI-free core (no Qt/OpenCV) — a quick way to
  compile-check `Common/Cpp` changes.
- New source files must be added to the explicit list in
  `SerialPrograms/cmake/SourceFiles.cmake`; they are not picked up automatically.
- MSVC builds pass `/W4 /WX` — **every compiler warning is an error** (clang-cl, GCC, and
  clang also build in CI with strict flags).
- No tab characters in source (CI has a "Check for Tabs" job).
- Run the test suite by launching the built binary with `--command-line-test-mode` from
  `SerialPrograms/bin` (no GUI). Requirements (see the test steps in
  `.github/workflows/cpp-ci-serial-programs-base.yml` and `SerialPrograms/Source/Tests/CommandLineTests.h`):
  - `PokemonAutomation/CommandLineTests` checked out next to the program (test images).
  - `PaddleOCRDetection` resources unzipped from `Packages/DownloadedResources/` into the
    program's downloaded-resources folder (`SerialPrograms/bin/DownloadedResources/` on
    Windows/Linux; `~/Library/Application Support/SerialPrograms/DownloadedResources` on macOS).
  - Headless Linux: `QT_QPA_PLATFORM=offscreen` and
    `TESSDATA_PREFIX=<bin>/Resources/Tesseract`.
- Test framework: test classes derive `UnitTest` (`Common/Cpp/TestRunners/UnitTest.h`) and are
  collected via per-module `add_tests(UnitTestDatabase&)` — see
  `SerialPrograms/Source/Tests/CommandLineTests.h`.

## Conventions

- `char*` / `std::string` are UTF-8. Use `Common/Cpp/Strings/` (`StringTools`, `Unicode`)
  for text processing.
- File headers: `/*  Title \n * \n *  From: https://github.com/PokemonAutomation/ \n * \n */`;
  include guards are `PokemonAutomation_<Name>_H`.
- **Line endings: the codebase is overwhelmingly CRLF** (there is no `.gitattributes`
  normalization; `* -text`). Preserve each file's existing line endings when editing — do
  not re-normalize a file to LF, it produces a whole-file diff.
- Errors/cancellation: use `CommonFramework/Exceptions/` (`OperationFailedException`,
  `ProgramFinishedException`, ... — typically via `throw_and_log`) and
  `Common/Cpp/CancellableScope.h` (`CancellableScope::throw_if_cancelled()` in loops).
- File paths: use the macros from `CommonFramework/GlobalAutoPaths.h` (`RESOURCE_PATH()`,
  `USER_FILE_PATH()`, `DOWNLOADED_RESOURCE_PATH()`, ...) rather than hardcoding locations.

### Control flow formatting

- Put the opening brace on the same line as `if`, `else if`, `for`, `while`, or `switch`,
  with **no space between the closing `)` and `{`**: `if (condition){`, not `if (condition) {`.
  Write `}else if (condition){` and `}else{` without spaces around `else`'s braces.
- Always use braces and multiline bodies for conditionals and loops, even for a single
  statement. Do not write `if (condition) return;` or `if (condition){ return; }`.
  Put the body on its own indented line and the closing brace on its own line (except when
  followed by `else` / `else if`). The same rule applies to one-statement loop bodies.
- Follow these rules in new or edited control flow even when nearby legacy code differs.
  Avoid reformatting unrelated code.

```cpp
if (condition){
    do_something();
}else if (other_condition){
    do_something_else();
}else{
    handle_default();
}

for (size_t i = 0; i < count; i++){
    process(i);
}
```