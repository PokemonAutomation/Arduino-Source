# Audio Pipeline Redesign — Design Review Handoff

Status: **Design review in progress (no implementation yet).** This document is a
handoff so a new session can pick up the audio pipeline redesign discussion where
the previous one left off. All paths are relative to the `Arduino-Source` repo root;
the audio code lives under `SerialPrograms/Source/`.

- Discussion channel: `#Audio Pipeline Redesign`
- Related PRs:
  - #1558 — the audio pipeline effort this belongs to
  - #1572 — `AudioResampler` streaming node (open, branch `audio-resampler-streaming-node`; the last kernel piece)
  - #1596 — resampler weight table built from block sizes (merged)
  - #1600 — channel mixer matrix builder for `QAudioFormat` (merged)

---

## 1. The Problem (today)

The pipeline in `SerialPrograms/Source/CommonFramework/AudioPipeline/` consumes audio
from a source, forwards it to a sink for playback, and passes the same stream to an FFT
pipeline for inference. It is flaky due to audio-format discrepancies:

1. Only a few preset formats are recognized: mono 48k, stereo 44.1k, stereo 48k, and
   "mono 96k" which is really **stereo 48k misreported by the capture card**
   (`AudioChannelFormat` in `CommonFramework/AudioPipeline/AudioInfo.h`).
2. The sink is forced to the source's format. If the sink rejects it, no audio plays
   (`IO/AudioSink.cpp:153` logs "Audio output is disabled").
3. Stereo is combined into 1 channel for the FFT (pair averaging in
   `Spectrum/FFTStreamer.cpp:102`).
4. Only 44.1k/48k are supported end-to-end; the FFT and inference support both.

Key facts about today's behavior:

- `AudioPassthroughPairQt` (`Backends/AudioPassthroughPairQt.cpp`) drives everything
  from the Qt audio callback thread: the source's float samples are written to the sink
  *and* fed to the FFT runner, all under one spin lock.
- Source volume: `DEVICE_VOLUME_SCALE` / `FILE_VOLUME_SCALE`
  (`AudioPipelineOptions.h`, both `UNLOCK_WHILE_RUNNING`) are multiplied into the
  source float stream by `AudioStreamToFloat`, so they scale **both** the sink path
  and the FFT path.
- Input format selection is preset-enum driven: `AudioSession::sanitize_format()`,
  `AudioInfo.cpp supported_input_formats()` (includes the capture-card presets
  `MONO_96000` and `INTERLEAVE_LR_96000`/`INTERLEAVE_RL_96000`, and a MiraBox
  preferred-index hack at `AudioInfo.cpp:248`).
- `AudioOption` serializes the `AudioChannelFormat` enum (saved settings).
- Downstream consumers keyed on the enum / preset rates:
  - `CommonFramework/Recording/StreamHistorySession.cpp:132` (stream history
    recording switches on the preset enum).
  - `CommonTools/Audio/AudioPerSpectrumDetectorBase.cpp:104-108` — the spectrogram
    matcher is keyed on the spectrum's `sample_rate`.
  - `CommonTools/Audio/AudioTemplateCache.cpp:56` — audio templates are loaded as
    `<path>-<sample_rate>` resource files (e.g. `ShinySound-48000.wav` vs
    `ShinySound-44100.wav`).

## 2. The Plan (agreed direction)

Accept **any** input format (any channel count, any sample rate, any sample format)
and convert any input to any output, by chaining "processing nodes" into a tree:

```
source (device or file, format F_in)
  └─ RawAudio2WorkingAudio (F_in → float32, channel-strided)
       ├─ sink branch:  [ChannelMixer → device layout] → [AudioResampler → device rate]
       │                 → WorkingAudio2RawAudio (device sample format, playback volume)
       │                 → QAudioSink (device's native format)
       └─ FFT branch:   [ChannelMixer → mono] → [AudioResampler → 48000]
                         → FFT pipeline (48 kHz only)
```

- The FFT branch is **48 kHz only**; the 44.1 kHz inference path is removed.
- Canonical node order per branch: mix **before** resample (mixer is rate-independent
  and zero-latency; fewer channels through the resampler is cheaper). Skip no-op nodes.
- The capture-card case: the user's dropdown choice is applied to the `QAudioFormat`
  itself *before* it enters the pipeline, so the whole tree sees e.g. 2×48k instead of
  1×96k (see §4, item 1).

### Building blocks (what exists)

| Node | Path | Role |
|---|---|---|
| `RawAudio2WorkingAudio` | `Kernels/AudioPipeline/Kernels_RawAudio2WorkingAudio.h/.cpp` | raw (UInt8/Int16/Int32/Float32, any ch/rate, interleaved) → float32 channel-strided "working" format. Base class constructed with the **input** format; listeners must match it. |
| `ChannelMixer` | `Kernels/AudioPipeline/Kernels_ChannelMixer.h/.cpp` | working → working, N→M channels via a float weight matrix (size `in×out`, weight at `i + in*out` = input channel i → output channel o). Rate unchanged. |
| matrix builder | `CommonFramework/AudioPipeline/ChannelMixer.h/.cpp` | `build_channel_mixer_matrix(QAudioFormat in, QAudioFormat out)` — ITU-R BS.1116-style downmix/upmix coefficients (0.5/0.5 L/R→mono, 0.7 surround fills, 0.35 center upmix, mono upmix = 1.0 to front channels, LFE passes through, top/back-center dropped). Returns an **empty** vector if either side has a non-positive channel count or `ChannelConfigUnknown`. **No renormalization** (documented: full-scale multichannel can exceed full-scale in narrow downmixes). Unit-tested (`add_tests_ChannelMixerMatrix`). |
| `WorkingAudio2RawAudio` | `Kernels/AudioPipeline/Kernels_WorkingAudio2RawAudio.h/.cpp` | working → raw, target sample type/channels, with runtime `set_multiplier()` (atomic) — intended for **playback volume**. |
| `AudioResampler` | `Kernels/AudioResampler/Kernels_AudioResampler.h/.cpp` (PR #1572) | streaming resampler, working → working, rate in→out, channels unchanged. Kaiser-window sinc; weight table indexed by the **output** period; per-ISA block kernels (Default, SSE2 tap2, AVX512 block128; FMA3 tap4 known-broken and unwired). Per-channel carry buffers; flushes its remainder on every `push_samples()` return, so added latency ≈ carry only (~150 samples ≈ 3ms at 44.1k↔48k), not the 4352-sample buffer. |

Working-format contract: `WorkingAudioStreamListener::push_samples(samples, stride,
data)` — float32, channel-strided (channel c at `c*stride + n`); the raw side is
interleaved.

Also relevant:

- `Spectrum/FFTStreamer.*` — `AudioFloatToFFT` today takes an **interleaved** float
  stream of 1–2 channels at 44.1/48k and pair-averages; 2048-sample window, 512 step
  (`AudioConstants.h`). Needs to move to a working-format **mono 48k** input (see §5).
- `Kernels/AudioPipeline/Kernels_AudioPipeline_RoundTrip_TestInfra.*` — round-trip
  test pattern for the raw↔working conversions.

## 3. Decisions made during the review

1. **Capture-card "96k mono" is a reinterpretation, not a format.** The card actually
   emits 48k stereo (`L0 R0 L1 R1 …`). Today's `MONO_96000` preset works because the
   FFT *averages consecutive pairs* — a stereo downmix, **not** a resample. Interpreting
   the stream as true 96k mono and resampling 96k→48k would keep only even samples =
   left channel only.
   **Decision:** the user's dropdown choice rewrites the `QAudioFormat` (rate, channel
   count, channel config) *before* it enters the pipeline; the whole tree sees e.g.
   2×48k. This works because the capture-card driver exposes both views (which is why
   `INTERLEAVE_LR_96000` and `MONO_96000` both exist in `supported_formats`).
   Follow-ups still to fold in:
   - Keep the sample type from the device's preferred format when rewriting
     rate/channels (don't assume float).
   - Robustness: not every device exposes both views. If a card only claims 96k mono
     (`isFormatSupported(48k stereo)` false), the rewritten format can't be opened.
     The general mechanism is: open the device with *request format* A, interpret the
     byte stream as *interpretation format* B (invariant: `ch × rate × sampleSize`
     equal, i.e. same frame byte size). The rewrite approach is the common case where
     A == B, but keep A ≠ B as an option (`QAudioSource` takes A,
     `RawAudio2WorkingAudio` takes B) — cheap insurance against driver quirks like the
     MiraBox hack (`AudioInfo.cpp:248`).
2. **Channel assignments when rewriting the `QAudioFormat`.** In Qt6,
   `setChannelConfig(Stereo)` sets both channel count and positions. The rewritten
   format must never end up `ChannelConfigUnknown`: `build_channel_mixer_matrix`
   returns an empty matrix for unknown layouts, and positions resolve via
   `channelOffset()`. Use `ChannelConfigMono` for the 1-channel case — the mixer
   builder's mono detection keys off `FrontCenter`. Also implement the tree-builder
   fallback for genuinely unknown layouts (see §5, item 4).
3. **Input volume goes on `RawAudio2WorkingAudio`.** A `multiplier` parameter will be
   added to `RawAudio2WorkingAudio` (input volume — originally meant to normalize file
   inputs, which tend to be louder than capture cards). `WorkingAudio2RawAudio`'s
   multiplier stays the playback-volume control. Follow-ups:
   - Give it a `set_multiplier()` + `std::atomic<float>` (both volume options are
     `UNLOCK_WHILE_RUNNING`, so it must apply live).
   - Both nodes have a zero-copy fast path for FLOAT32+1ch that bypasses everything
     (`Kernels_RawAudio2WorkingAudio.cpp:57,102`,
     `Kernels_WorkingAudio2RawAudio.cpp:44,96`). The Raw fast path would silently drop
     the gain — restrict it to `multiplier == 1.0f`. Note `WorkingAudio2RawAudio`'s
     fast path *already* ignores its multiplier today (a float32-mono sink would skip
     playback volume) — fix both.
   - Apply the gain in the float domain after type conversion, matching today's
     `AudioStreamToFloat` behavior, so levels reaching the FFT are unchanged.
4. **Resampler taps: default 32, not 16.** 32 taps matches ffmpeg's default; the 16
   that landed in the node was wrong. Keep taps/beta as per-edge parameters in the tree
   builder so e.g. a 44.1k sink edge can use more taps than the 48k FFT edge.
5. **Resampler weight-table size is fine (and smaller than first feared).** The table
   is indexed on the **output** period only: `m_data` is `taps_allocated × tap_stride`
   with `tap_stride ≈ period_prime + sample_block`, and `period_prime` is a multiple of
   `period_out = out/gcd(in,out)` (`Kernels/AudioResampler/Kernels_AudioResampler_WeightTable.cpp:57-60`).
   `period_in` never affects allocation. So size ≈ `taps × (out_rate/gcd + block) × 4B`
   (+ ≥4KB cache-way padding per tap row).
   - Real device rates are 44.1k-family or 48k-family, so gcds stay large. Worst
     realistic pair 44100↔48000 → gcd 300, period_out ≤ 160 → ~150KB at 32 taps. Even
     44100→192000 is ~150KB.
   - Theoretical bound: `taps × out_rate × 4B` at gcd = 1 → ~25MB at a 192k output,
     ~6.2MB against the 48k FFT edge regardless of input rate. Real hardware won't hit
     this; a sanity cap on accepted rates (≤384k) is a one-liner if desired.
   - Build time is a non-issue: `taps × (period_out + block)` sin+bessel_i0 evals —
     ~10K in the realistic case, ~1.5M even at the 6MB bound (tens of ms).
   - **Still worth doing:** cache tables per `(in, out, taps, beta, block)` — the
     watchdog (`AudioSession::on_watchdog_timeout`, default 5s silence) rebuilds the
     whole tree, and both branches often want the *same* conversion
     (e.g. 44.1k→48k for sink and FFT), so without a cache the same table is built
     twice per reset.

## 4. Open design issues (not yet resolved)

1. **Requested format ≠ delivered format (both ends).** On Windows, WASAPI shared mode
   / the winMM shadow device can silently resample (project's own comment:
   `AudioInfo.cpp:323-324` "shadow device (winMM) that seems to be able to resample to
   all formats"). On Qt6, `preferredFormat()` is unreliable
   (`AudioInfo.cpp:193` "always returns stereo 44.1 kHz"). So "match the device's
   native format" can't just read `preferredFormat()`. Plan: after `start()`, read the
   **actual** format (`QAudioSource::format()`) and build the tree from that; for the
   sink, use a candidate ladder (preferred → 48k/44.1k × S16/F32 × device channel
   count) validated by `isFormatSupported` **and** successful `QAudioSink::start()`
   (OpenError → next candidate), instead of today's "Audio output is disabled" dead
   end (`IO/AudioSink.cpp:153`). Also pin `isInterleaved()` — all kernels assume
   interleaved.
2. **Threading rules for the graph.** `push_samples()` is not thread-safe anywhere
   (ChannelMixer writes `m_buffer`; the resampler writes carry/buffer). The fan-out is
   only safe because both branches run synchronously in the source audio callback today
   (`Backends/AudioPassthroughPairQt.cpp`, `SampleListener`). Document the invariant:
   **one thread per node; shared nodes only on a single thread.** Sharing one resampler
   between both branches is legal today but breaks the moment the FFT moves to a
   worker thread — and there's no queue node in `Kernels/AudioPipeline` to decouple
   branches. Also note the pre-existing coupling stays: blocking `QAudioSink` writes in
   the callback delay the FFT and vice versa (underruns).
3. **48k-only FFT ripples downstream.** `AudioSpectrum::sample_rate` becomes constant
   48000; the detector keys the matcher on spectrum rate
   (`CommonTools/Audio/AudioPerSpectrumDetectorBase.cpp:104-108`) and
   `AudioTemplateCache` loads `<name>-<rate>` resources. **Action:** audit that every
   audio template has a `-48000` variant and re-render 44100-only ones — bin spacing
   changes (44.1k: 21.53 Hz/bin → 48k: 23.44 Hz/bin over the 2048-point window), so
   44.1k templates are *not* interchangeable. Then delete the 44100 resources and code
   paths.
4. **Mixer fallbacks for unknown channel layouts.** `build_channel_mixer_matrix`
   returns empty if either side's `channelConfig` is unknown; some backends report only
   a channel count. The tree builder needs defined fallbacks: N→1 = `[0.5, 0.5]`
   (N=2) or `1/N` for N≥3; sink = identity when counts match, else first-M-channels.
   Otherwise tree construction fails on some devices.
5. **Other preset-enum-tied consumers + settings migration.**
   `StreamHistorySession::initialize()` (`CommonFramework/Recording/StreamHistorySession.cpp:132`)
   and `AudioFloatStreamListener::expected_samples_per_frame` assume the 48/44.1k
   presets; with arbitrary rates, consumers need the stream's actual (rate, channels).
   `AudioOption` serializes the old enum — saved settings need migration
   (`INTERLEAVE_RL` is just a 2×2 permutation matrix).
6. **Clipping.** No renormalization in the mixer → int16 clip on full-scale multichannel
   downmix (sink path). Stereo→mono (0.5/0.5) is bounded, so the FFT path is fine.
   Consider an optional soft-clip/pre-scale on the mix node; at minimum document it.
7. **`AudioFloatToFFT` API.** Today: interleaved 1–2ch float with pair-averaging
   (`Spectrum/FFTStreamer.h/.cpp`). Move it to a working-format mono-48k input and
   remove the pair-averaging (the mixer does it).
8. **File source.** The WAV loader hard-fails on rate mismatch today
   (`IO/AudioFileLoader.cpp:67` "we don't interpolate"); the tree fixes that for free —
   read the file's native format and convert in the pipeline. For compressed files,
   check `QAudioDecoder::format()` after start rather than assuming the requested
   format.
9. **Qt5.** `channelOffset()` is Qt6-only, so "any layout" is effectively Qt6-only.
   The codebase still has `#if QT_VERSION_MAJOR == 5` paths; fine if Qt5 is being
   dropped, but state it explicitly.

## 5. Resampler PR #1572 — known issues (for awareness)

From the PR description:

- The FMA3 `run_block16_tap4_x64_FMA3` kernel has a broken 8×4 transpose and is
  deliberately **not wired up** (falls through to the SSE2 tap2 kernel).
- Open question (to be settled by CI hardware verdict, not paper trace): the SSE2 tap2
  and AVX512 POC block kernels' `shuffle_ps` transposes may also misroute samples.
  The new direct block-kernel tests run every kernel against a scalar reference in CI
  (x86 runners), so check those results when the PR is reviewed.
- Stream tests push random input through the node in chunks and compare every output
  against a scalar reference computed from the weight table; 9 registered rate/channel
  combos including 44100↔48000.

## 6. Test plan (agreed)

1. **Graph-level round-trip** through both branches vs a scalar reference (use the
   `Kernels_AudioPipeline_RoundTrip_TestInfra` pattern): arbitrary in-format →
   (mix → resample → raw-out) → compare to a direct offline conversion.
2. **Capture-card regression:** 48k-stereo bytes *declared* 96k-mono → resample gives
   L-only; *declared* 48k-stereo → mix matches the old FFT pair-average. Pin both
   behaviors explicitly.
3. **48k mono input → FFT output bit-identical to today** (all-no-op path: no mixer,
   no resampler).

## 7. Suggested next steps

1. Land PR #1572 (AudioResampler) and fix its default `taps` 16 → 32.
2. Add `multiplier`/`set_multiplier()` to `RawAudio2WorkingAudio`; fix the two
   FLOAT32+1ch fast paths (§3, item 3).
3. Define the "format" value type for the new options (rate, channels, channel config,
   sample format) + the dropdown entries for the capture-card interpretations, and the
   tree-builder (including the §4 fallbacks and the A≠B request/interpretation option).
4. Rework `AudioPassthroughPairQt` (or replace it) around the node tree: build from the
   actual source format after `start()`, sink candidate ladder, atomic tree teardown
   (reverse construction order; keep the `LifetimeSanitizer` discipline), volume wiring
   per §3 item 3, and per-edge taps/beta.
5. Move `AudioFloatToFFT` to working-format mono-48k; do the template resource audit
   (§4 item 3); migrate `AudioOption`/`StreamHistorySession` off the preset enum.
6. Add the tests from §6, including a weight-table cache test (same table not built
   twice across a reset).

## 8. Key file map

```
SerialPrograms/Source/
├── Kernels/AudioPipeline/
│   ├── Kernels_AudioPipeline.h              # SampleFormat enum, Raw/Working stream listener interfaces
│   ├── Kernels_RawAudio2WorkingAudio.{h,cpp}
│   ├── Kernels_ChannelMixer.{h,cpp}
│   ├── Kernels_WorkingAudio2RawAudio.{h,cpp}
│   ├── Kernels_AudioPipeline_Tests.cpp
│   └── Kernels_AudioPipeline_RoundTrip_TestInfra.{h,cpp}
├── Kernels/AudioResampler/                  # merged pieces + PR #1572 (Kernels_AudioResampler.*)
├── Kernels/AudioStreamConverter/            # raw <-> float sample conversion kernels
├── Kernels/AudioChannelInterleave/          # interleave / deinterleave kernels
├── Kernels/AudioChannelMixer/               # mix_channels kernel
├── Kernels/KaiserWindow/                    # bessel_i0, kaiser window (used by resampler + FFT)
├── CommonFramework/AudioPipeline/
│   ├── AudioInfo.{h,cpp}                    # AudioChannelFormat presets, device enumeration, set_format()
│   ├── AudioSession.{h,cpp}                 # session, watchdog reset, options persistence
│   ├── AudioStream.{h,cpp}                  # today's AudioStreamToFloat / AudioFloatToStream
│   ├── ChannelMixer.{h,cpp}                 # build_channel_mixer_matrix (Qt)
│   ├── AudioPipelineOptions.h               # FILE/DEVICE_VOLUME_SCALE, USE_KAISER_WINDOW, AUTO_RESET_SECONDS
│   ├── Backends/AudioPassthroughPairQt*.cpp # today's glue (source callback drives sink + FFT)
│   ├── IO/AudioSource.{h,cpp}               # device/file input, format forcing
│   ├── IO/AudioSink.{h,cpp}                 # output device, "audio disabled" failure path
│   ├── IO/AudioFileLoader.{h,cpp}           # wav (WavFile) + QAudioDecoder paths
│   └── Spectrum/FFTStreamer.{h,cpp}         # AudioFloatToFFT (2048-pt window, 512 step, pair avg)
└── CommonTools/
    ├── Audio/                               # SpectrogramMatcher, AudioTemplateCache (rate-suffixed resources)
    └── InferencePivots/AudioInferencePivot.*
```
