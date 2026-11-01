# Handoff: PaddleOCR crash in the Mac Intel CI (x86_64 macOS)

Status: OPEN investigation. This document is a handoff from a different agent
session that studied the crash while doing unrelated TCP socket work
(PRs #1787 / #1793 / #1794). That work is separate from this crash and is
continuing; everything below is self-contained.

## TL;DR

- The `C++ CI Serial Programs Mac Intel` job (`runs-on: macos-15-intel`,
  GitHub-hosted ephemeral runners) segfaults (exit code 139) while running
  `./SerialPrograms.app/Contents/MacOS/SerialPrograms --command-line-test-mode`.
- Every crash so far happens inside the **first PaddleOCR ONNX session
  creation of the run**, ~0.1s after the stdout line
  `Set potential model cache path in session options: .../ModelCache/PaddleOCRPipeline/`
  (`SerialPrograms/Source/ML/Models/ML_ONNXRuntimeHelpers.cpp:28`).
- The sessions are **CPU-only** (CI default `USE_GPU = false`; no CoreML /
  GPU provider is ever engaged in these runs).
- It is probabilistic: it has not happened on `main`, on Mac arm64
  (`macos-15`), or in the same code trees with slightly different test
  scheduling. x86_64 Mac only so far.
- It is **not caused by the socket PRs** (evidence in "Ruled out" below).

## Crash and control runs (all 2026-11-01)

PaddleOCR crash:

| CI run | branch / commit | job | note |
| --- | --- | --- | --- |
| 41266441051 | #1787 `sockets-server-loopback` @ 544b9fb28 | Mac Intel | failed on BOTH attempts of the job (attempt=2) |
| 41275162708 | #1793 `sockets-server-posix` @ 77e2f4f10 | Mac Intel | 13/14 socket tests had passed before the crash |

Related run that failed for a DIFFERENT reason (a socket bug, since fixed;
PaddleOCR did NOT crash in it - useful as evidence the crash is not
deterministic on this tree):

| CI run | branch / commit | job | note |
| --- | --- | --- | --- |
| 41268990238 | #1793 @ e7cd4206a | Mac Intel | failed on `Sockets (POSIX) - Large transfer` (client partial-send bug, fixed in 77e2f4f10); PaddleOCR passed |

Controls that PASSED in the same window:

| CI run | ref | job |
| --- | --- | --- |
| 41271119786 | main 159a7e992 | Mac Intel |
| 41273317855 | main 147c0dbd8 | Mac Intel |
| 41277122162 / 41277122200 | main e85275f86 | Mac / Mac Intel |
| 41275186453 | #1794 `sockets-server-winsocket` @ e46a26040 | Mac Intel |
| 41275186384 | #1794 @ e46a26040 | Mac |
| 41275162746 | #1793 @ 77e2f4f10 | Mac |
| 41274799422 | #1787 `sockets-server-loopback` @ 20140b763 (rebased) | Mac |
| - | #1787 @ 544b9fb28 (the 02:55 run, pre-rebase) | Mac |

Fetch logs with `gh run view <runId> --log-failed` (failed runs) or
`gh run view <runId> --log`.

## Pattern in the crash logs

- Three `PokemonSV::Test_SandwichRecipeDetector` tests start within ~2ms of
  each other (Recipe_7_8_9_10_11_12, Recipe_75_76_77_78_79_80,
  Recipe_41_42_43_44_45_46 - the first macOS images in registration order),
  and the first PaddleOCR session of the run starts right after. The last
  stdout line before the segfault is always the "Set potential model cache
  path" line from `create_session_options()`.
- In both crash runs, the socket test `Sockets (Qt) - Rapid reconnect`
  (50 connect/send/echo/close cycles on the Qt event thread pool, part of
  the socket PRs' test harness) was still running when the crash happened.
- Run 41268990238 had the SAME overlap and survived, so the overlap is
  necessary-but-not-sufficient: a probabilistic timing window.
- On arm64 (macos-15) the identical three sandwich tests start ~2ms apart
  and the session init takes ~2.4s to the first OCR result without crashing.

## Ruled out

1. **The socket PRs' code.** In every crash the socket tests that completed
   had PASSED; the crash is in the PaddleOCR/ONNX init window. The socket
   implementation was diffed byte-identical to a revision that previously
   passed both Mac CIs. #1794 (same 7 Qt socket tests; WinSocket/POSIX code
   is `#ifdef`'d out of the Mac build) passed Mac Intel; main passed Mac
   Intel repeatedly in the same window.
2. **GPU/CoreML path.** CI is CPU-only: `OnnxOptions::USE_GPU` defaults to
   `false` and no `Using CoreML execution provider` line appears in any log.
3. **Model-cache writes.** On CPU, ONNX Runtime does not write to
   `ModelCacheDirectory`; the `PaddleOCRPipeline/` cache directory is never
   created in these runs.
4. **A main regression.** main's Mac Intel was green at ce55a34aa /
   9783a7ec2 / 159a7e992 / 147c0dbd8 / e85275f86 today.

## Code map

- `SerialPrograms/Source/ML/Models/ML_ONNXRuntimeHelpers.cpp`
  - `create_session_options()` :28 - the last line printed before the crash.
  - `clean_up_old_model_cache()` :126 - `remove_all` on the shared cache dir.
  - `write_cache_flag_file()` :155.
  - `create_session()` :165 - GPU try, then CPU fallback; both catch and
    throw `MLModelSessionCreationError` (a segfault bypasses these).
- `SerialPrograms/Source/ML/Models/ML_OrtEnv.cpp`
  - `make_ort_threading_options()`: `SetGlobalIntraOpNumThreads`
    (default `hardware_concurrency() - 2`), `SetGlobalInterOpNumThreads(0)`,
    `SetGlobalSpinControl(0)`, `SetGlobalDenormalAsZero()` (**x86-only** -
    sets MXCSR DAZ/FTZ on ORT threads).
  - `global_ort_env()`: single `Ort::Env`, function-local static.
- `SerialPrograms/Source/ML/Inference/ML_PaddleOCRPipeline.cpp`
  - `PaddleOCRPipeline` ctor: `create_session(rec.onnx,
    ML_MODEL_CACHE_PATH() + "PaddleOCRPipeline/", USE_GPU)`, then
    `GetInputNameAllocated` / `GetOutputNameAllocated`, `load_dictionary`.
    The crash could be anywhere in this window (only the `create_session_options`
    line goes to stdout; the "Ort::Session created" log goes to the file
    logger and is NOT in the CI log).
  - `detector()`: lazily creates the *detection* session (different cache
    dir, `PaddleOCRDetector/`) on first `recognize_multiline()`.
- `SerialPrograms/Source/CommonTools/OCR/OCR_PaddleOCR.cpp`
  - `PaddleOcrGlobals`: one `PaddleOCRPipeline` per language group;
    `ensure_paddle_ocr_instance()` constructs the pipeline (i.e. creates the
    ONNX session) **while holding the pool's write spinlock**. Same-language
    creation is serialized; different language groups are created in
    parallel. See also the "PaddleOCR with Onnx is threadsafe" comment on
    `paddle_ocr_read()`.
  - Precedent note: the `USE_GPU` option text documents a prior
    PaddleOCR-multithreading crash (DirectML on Windows) - there is history
    of threading hazards in this stack.
- `SerialPrograms/Source/VideoGames/PokemonSV/Inference/Picnics/
  PokemonSV_SandwichRecipeDetector.cpp` - `Test_SandwichRecipeDetector`
  builds a `SandwichRecipeNumberDetector` per test and calls
  `OCR::read_number()` (single-line path, recognition model only).
- ONNX Runtime: 1.23.2, committed prebuilt binary in
  `3rdPartyBinaries/onnxruntime-osx-x86_64-1.23.2.tgz` (used for symbol
  matching). No dSYM ships with it, but the dylib retains mangled C++
  symbols (e.g. `FinalizeSessionStateImpl`, `PartitionOrtFormatModel` -
  both in the session-creation path), so crash-report frames should mostly
  symbolicate; verify against the committed dylib's UUID.
- CI wiring: `.github/workflows/cpp-ci-serial-programs-mac-intel.yml`
  (`os: macos-15-intel`) and `.../cpp-ci-serial-programs-mac.yml`
  (`os: macos-15`) -> `.../cpp-ci-serial-programs-base.yml`, step
  `Run tests (Mac)` (~line 292).

## Hypotheses (roughly ranked)

H1. **Timing-sensitive race in ORT's x86_64 session initialization**,
triggered by concurrent load on the shared runner (the Qt rapid-reconnect
test hammering sockets/threads at the same moment). Needs a stack trace to
confirm or kill.

H2. **ORT 1.23.2 x86_64 binary-level issue** sensitive to CPU state
(candidate: `SetGlobalDenormalAsZero()`'s MXCSR DAZ/FTZ handling, thread
pool init on many-core Xeon, or an issue specific to the macOS 15 Intel
runner image). Search onnxruntime issues for 1.23.x segfaults during
`Ort::Session` creation on macOS x86_64; optionally test 1.23.0 (the Linux
version in use) or a newer release on a scratch branch.

H3. **Resource exhaustion** on `macos-15-intel` (memory pressure causing a
bad allocation during model load). A crash report with the faulting address
would distinguish this.

H4. **Real latent bug, probably unrelated to these crashes**: cross-language
model-cache race. All PaddleOCR recognition models (english/chinese/latin/
korean) share ONE cache directory and ONE `HASH.txt`.
`clean_up_old_model_cache()` runs `Filesystem::remove_all(model_cache_path)`
on any hash mismatch, `create_session()` has no synchronization, and
`ensure_paddle_ocr_instance()` only serializes creation per language group.
So two different languages initializing close together while the cache is
actually being written (the GPU/CoreML path - i.e. user machines with
"Use GPU Acceleration" enabled) can have one thread delete the cache
directory while another thread's ONNX Runtime is writing into it, producing
a corrupt `.mlmodel` and a later crash. Fix options: per-model cache
subdirectories, or a global mutex around `create_session` plus an atomic
directory swap. Worth a small standalone PR either way.

## Next steps

1. **Get a stack trace** (the blocker). Two CI options:

   a. On-failure crash-report dump (cheap; macOS writes `.ips` crash
   reports automatically to `~/Library/Logs/DiagnosticReports/`). Add after
   the `Run tests (Mac)` step in `cpp-ci-serial-programs-base.yml`:

   ```yaml
   - name: Dump macOS crash reports (on failure)
     if: startsWith(inputs.os, 'macOS') && failure()
     run: |
       echo "=== DiagnosticReports ==="
       ls -la "$HOME/Library/Logs/DiagnosticReports/" || true
       for f in "$HOME/Library/Logs/DiagnosticReports/"*.ips; do
         [ -e "$f" ] || continue
         echo "=== $f ==="
         cat "$f"
       done
   ```

   The `.ips` is JSON: the crashed thread's `frames` carry symbol names and
   `imageOffset`; `usedImages` maps image indices to paths and UUIDs. Match
   `libonnxruntime.dylib`'s UUID against
   `3rdPartyBinaries/onnxruntime-osx-x86_64-1.23.2.tgz`.

   b. Rerun the suite under lldb (full backtrace straight into the log, but
   it doubles the failing run's time):

   ```yaml
   - name: Rerun tests under lldb for a backtrace (on failure)
     if: startsWith(inputs.os, 'macOS') && failure()
     run: |
       cd Arduino-Source/SerialPrograms/bin
       lldb -b -o run -o "thread backtrace all" -o quit \
         ./SerialPrograms.app/Contents/MacOS/SerialPrograms \
         --command-line-test-mode
   ```

   lldb ships with the GitHub macOS images. This step is expected to "fail"
   (nonzero exit on crash); the value is the log.

2. **More data points**: rerun the failed Mac Intel job of PR #1793
   (run 41275162708) via the Actions UI "Re-run failed jobs" (the agent
   token used so far has no permission to trigger reruns via API).

3. **If the backtrace is in `libonnxruntime`**: search onnxruntime issues
   for the 1.23.x version; try a different ORT version on a scratch branch
   to see if the crash follows the binary.

4. **If the backtrace is in our code**: use the code map above; look first
   at the `PaddleOCRPipeline` ctor window and the ORT threading options
   (`SetGlobalDenormalAsZero` is the most x86-specific knob in play).

5. **Either way**: fix H4 (the shared model-cache race) as a small
   separate PR.

## Pointers

- Socket PRs (separate effort, do not conflate with this crash):
  #1787 `sockets-server-loopback`, #1793 `sockets-server-posix`,
  #1794 `sockets-server-winsocket`. All CI-green as of this writing except
  #1793's Mac Intel job, which hit this crash.
- Key files: `SerialPrograms/Source/ML/Models/ML_ONNXRuntimeHelpers.cpp`,
  `SerialPrograms/Source/ML/Models/ML_OrtEnv.cpp`,
  `SerialPrograms/Source/ML/Inference/ML_PaddleOCRPipeline.cpp`,
  `SerialPrograms/Source/CommonTools/OCR/OCR_PaddleOCR.cpp`,
  `.github/workflows/cpp-ci-serial-programs-base.yml`.
