# Audio Pipeline — Working-Stream Path Hand-Off

Hand-off for the session taking over **PR #1563** (ffmpeg resampler for video output
encoding), from the session that did the audio pipeline migration.

## Read this first

- **Do not pull in an ffmpeg audio resampler.** The audio pipeline already has a
  kernel resampler (`Kernels::AudioResampler`, SIMD-dispatched, gate-tested) and a
  channel mixer. Use them; let ffmpeg do only what it does for video (decode/encode/
  mux).
- Audio is exposed to consumers through a **`WorkingAudioStreamListener` path**
  (added in PR #1671, reworked onto #1674's all-in-one node). Subscribe with
  `AudioSession::add_working_stream_listener(format, listener)` /
  `remove_working_stream_listener(listener)`, where `format` is the shape you
  want delivered (sample rate + channel count; the channel layout in
  `format.native_format()` drives the channel mixing). The pair builds a
  `WorkingAudio2WorkingAudio` node per listener (mix + resample, source shape
  -> your shape), so you get the converted stream for free.
- The legacy `AudioFloatStreamListener` path is gone (#1698): `AudioStream`
  and all the float listener plumbing were deleted, and `StreamHistorySession`
  consumes only the `WorkingAudioStreamListener` path.
- The fixed input-format presets (`AudioChannelFormat`) are being replaced by
  the device-driven `AudioInputChoice` in four steps: #1678 (Step A: the
  device's `supported_choices()` list) and #1707 (Step B: the pipeline opens
  the device through a choice) are in main; #1711 (Step C: session state,
  options/JSON persistence, and the format dropdown run on choices) is open;
  Step D deletes the preset enum, `set_format`, and `AUDIO_FORMAT_LABELS`.
  Note: no duplicate-device filtering in `all_input_devices()` is needed —
  the Windows winMM/MMDevice shadow entries were a Qt5 artifact (QTBUG-75781),
  fixed in Qt6, and no longer observed on Windows.

## What's already in main (the migration)

| PR | What |
|----|------|
| #1620 | Split `IO/AudioInputFile` / `IO/AudioInputDevice`; raw-listener roots on the reader. |
| #1623 | Pipeline tree built inside `AudioPassthroughPairQt` (decoder + sink/FFT branches). |
| #1647 / #1652 / #1654 | `WorkingAudioToFFT` node; FFT endpoint switched to the tree; legacy FFT path deleted. |
| #1655 | Sink gate tests (`SinkPipeline_Tests.cpp`): no-op bit-exactness, 1-LSB sint16, resample/mix invariants. |
| #1658 | Playback endpoint switched to the tree; `IO/AudioOutputDevice` (raw `RawAudioStreamListener` → `QAudioSink`); `AudioSink` deleted. Volume rides the sink encoder's multiplier (device opens at full volume). |
| #1665 | Sink opens in its **native** format (no more forced 44.1/48k presets); tree resamples/mixes/converts to it. |
| #1668 | Sink-side cleanups: dead per-output-device format enumeration removed (duplicate collapse now first-occurrence-wins per display name); `AudioFloatToStream` (float→raw) deleted; gate tests made self-contained. |
| #1674 | `AudioFormat` (Pimpl wrapper over `QAudioFormat`) and `WorkingAudio2WorkingAudio` (all-in-one mix + resample node); `ChannelMixer` / `AudioResampler` moved to the `WorkingAudioNode` base. |
| #1676 | Tree simplified onto the all-in-one node (one `WorkingAudio2WorkingAudio` per branch, direct connection when no conversion is needed). Also fixed a latent OOB write in `ChannelMixer::push_samples` for pushes larger than its buffer (it now chunks internally). |
| #1671 | **This path**: per-listener `add_listener(const AudioFormat&, WorkingAudioStreamListener&)` on the pair/session; `StreamHistorySession` now consumes it next to its legacy interface. |
| #1673 | **Native WAV file input**: `AudioSource` probes the WAV header (`probe_wav_native_format`), the loader streams the file's own rate / channels / sample type, and the tree converts to the sink's format. Gate tests in `IO/AudioFileLoader_Tests.cpp`. |
| #1680 | Multichannel WAV file input (up to 8 channels — the `fallback_channel_config` range); `channel_format(n)` no longer forces the stereo config on counts above 2, so `resolve_positions()` uses the standard layout (5.1 etc.) for the mix matrix. |
| #1683 | **Compressed file input at native format** (.mp3 etc. no longer forced to 48k/2ch float): `probe_decoded_native_format()` (throwaway `QAudioDecoder`, format read off the first decoded buffer); probe failure falls back to the legacy 48k stereo float target. `loadFullAudio()` (batch template loading) keeps its convert-to-requested-format behavior. Gate tests + their mp3 fixture live in the CommandLineTests repo (`CommonFramework/AudioPipeline/`, CLT PR #32), found via `UNIT_TEST_RESOURCE_PATH()`. |
| #1678 | Step A of the preset replacement: `AudioInputChoice` foundation — device-driven input choices (`AudioDeviceInfo::supported_choices()` / `preferred_choice_index()`) generalizing the 7 legacy presets. |
| #1695 / #1698 | `StreamHistorySession` moved onto the `WorkingAudioStreamListener` path; the legacy float stream path (and `AudioStream`) removed. |
| #1707 | Step B: the pipeline is choice-driven — the device source opens through a resolved `AudioInputChoice` (`AudioSource`, pair, session, history). |
| #1711 (open) | Step C: session state / options / JSON and the UI run on choices — `input_choice()` / `set_input_choice()`, `sanitize_choice()`, the format dropdown built from `supported_choices()` display names; `resolve_input_choice()` (the Step B preset→choice bridge) removed. |

## Architecture

```
AudioSession (UI thread)  ── audio() on ConsoleSystemSession
└─ AudioPassthroughPairQtThread → AudioPassthroughPairQt (event thread, SpinLock m_lock)
   ├─ m_reader: AudioSource
   │   ├─ raw stream  → m_source_decoder (RawAudio2WorkingAudio)   [tree root]
   │   │                ├─ sink branch:  [m_sink_conv: WorkingAudio2WorkingAudio] → m_sink_encoder → m_sink_device (QAudioSink)
   │   │                ├─ FFT branch:   [m_fft_conv: WorkingAudio2WorkingAudio] → m_fft_node (WorkingAudioToFFT) → InternalFFTListener
   │   │                └─ per working listener: [its WorkingAudio2WorkingAudio node] → internal forwarder → YOUR listener   ← THE NEW PATH
   └─ m_fft_listeners (AudioSession itself is an FFTListener)
```

- The tree is rebuilt under `m_lock` whenever the source/sink changes; producers push
  lock-free, and `ListenerSet::remove` blocks on in-flight pushes, so rebuilds never
  race a live push.
- Threading: file input produces on the event thread (QTimer); device input on the
  Qt audio backend thread. **Your listener callbacks run on whichever of those is
  producing** — no extra hop.

## The working-stream API (reference)

```cpp
//  AudioSession (thread-safe; safe to call from any thread)
void add_working_stream_listener(
    const AudioFormat& format,
    Kernels::AudioPipeline::WorkingAudioStreamListener& listener);
void remove_working_stream_listener(Kernels::AudioPipeline::WorkingAudioStreamListener&);
```

- Tap point: the source decoder's output — the tree's working float stream.
- The pair builds one `WorkingAudio2WorkingAudio` node per attached listener
  (source shape -> `format`), plus an internal forwarder carrying the
  requested shape as its nominal one (the node validates its listeners
  against that; **your listener's own nominal (rate, channels) is never
  validated** — it's metadata, pick anything sensible).
- Data layout: **split (planar) channels** in the requested shape.
  `push_samples(samples, stride, data)`: channel `c` starts at
  `data[c * stride]` and runs `samples` floats. `stride` is the
  per-channel buffer length (≥ `samples`); it is `0` for the
  single-channel float pass-through. Interleaved:
  `out[i*ch + c] = data[c*stride + i]`.
- The requested shape is **fixed at attach time**. If the shape you want
  follows the input preset (like the history session's), re-attach on input
  change: `post_input_change` -> remove + add with the new format. A
  consumer that requests one fixed shape (like a video encoder targeting
  48 kHz stereo) never has to.
- The input volume is already applied (it rides the source decoder).
- The capture-card `INTERLEAVE_RL_96000` channel swap **is applied** by the
  pair (literal swap matrix in the node, same as the sink branch), when the
  source is the RL preset and the requested shape is 2 channels.
- Re-adding a listener (e.g. with a new format) rebuilds its node; the
  remove quiesces in-flight pushes first, so no push can run through a node
  that is about to be destroyed. The entries survive source changes:
  `build_tree()` rebuilds each node for the new source shape, so a
  persistent listener keeps working across input changes.
- File input is native end-to-end: WAVs stream at the file's own rate / channels /
  sample type (up to 8ch, #1673 + #1680), and compressed formats are probed to
  their decoded native format (typically 44.1k stereo float for mp3, #1683).
  Either way the node adapts automatically.

## `StreamHistorySession`

It consumes the `WorkingAudioStreamListener` path only (the legacy float
interface was removed in #1698): attached via
`add_working_stream_listener(choice.format(), ...)` at `start()`, re-attached
in `post_input_change()` (the old-shape attach is removed first, so no push
in the old shape can race the shape update). It re-interleaves the planar
working stream at the active input choice's shape — the shape its tracker
records at (the tracker's audio shape was corrected in #1695: DUAL_44100 now
2 ch @ 44.1 kHz, MONO_96000 now 1 ch @ 96 kHz).

## The resampler & mixer kernels (if you ever own the chain yourself)

```cpp
//  Kernels/AudioResampler/Kernels_AudioResampler.h
AudioResampler(Logger&, size_t in_rate, size_t out_rate, size_t channels,
               size_t buffer_size = 4096 + 256, uint32_t taps = 32, double beta = 9.0);
//  Is itself a WorkingAudioStreamListener: add_listener(WorkingAudioStreamListener&).
//  validate_listener() matches (rate, channels) exactly.
```

- Window-sinc FIR, SIMD-dispatched; emits in output blocks. Gate-tested in
  `SinkPipeline_Tests.cpp` (resample-up 44.1k→48k, decimate 96k→48k: output length
  within ±2%/±1%, tone amplitudes at gain ≈1 bins).
- Caveats: block-buffered (first output lag is bounded), and input tail carry is
  bounded by `m_carry_capacity` (worst case one kernel block's input span; up to 128
  output samples on x86 AVX-512) — a stream that ends abruptly can lose a bounded
  tail. For continuous capture this is irrelevant; for file-ended streams pad the
  input or accept it.
- Channel conversion: `Kernels::AudioPipeline::ChannelMixer(rate, in_ch, out_ch,
  weight_matrix)`; `build_channel_mixer_matrix(QAudioFormat in, QAudioFormat out)`
  (`Tools/AudioFormatUtils.h`) gives the standard matrix (mono→stereo is gain-1.0
  dual-mono; stereo→mono is 0.5/0.5). `SinkPipeline_Tests.cpp::run_tree()` shows the
  exact construction/attachment pattern for chains of these nodes.

## Suggested shape for #1563

Just attach your encoder's listener to the requested shape — the pair does
the mixing and resampling for you:

```cpp
AudioFormat target;
target.native_format().setSampleRate(48000);
target.native_format().setChannelCount(2);
target.native_format().setChannelConfig(QAudioFormat::ChannelConfigStereo);
audio_session.add_working_stream_listener(target, my_encoder_listener);
```

Your `WorkingAudioStreamListener` then receives 48 kHz / 2 ch planar float
regardless of the input source, and can feed the ffmpeg audio encoder
directly. No manual `ChannelMixer` / `AudioResampler` chain needed (those
kernels are what you'd use to own the chain yourself, e.g. for a custom
target layout).

## Legacy float path — removed

Done in #1698 after #1695: `AudioStream.h/.cpp` is gone, as is the float
listener plumbing in `AudioSource`, the pair backends, and `AudioSession`
(`add_stream_listener` / `remove_stream_listener` no longer exist). Only the
raw stream and the working stream paths remain.

## Build & test

- `pa-build` (CMake + Ninja, Qt 6.10.2 aarch64, `-Wall -Wextra -Wpedantic -Werror`;
  incremental after the first build), then `pa-test .` (full command-line suite,
  ~18.2k tests; exit 0 on success).
- This aarch64 host has a partial gstreamer install; Qt 6.10 picks its
  QtMultimedia media backend at startup with no fallback, prefers the gstreamer
  plugin, and that plugin fails to load — so `QAudioDecoder` (the audio-file
  tests) is unsupported unless you run the suite with
  `QT_MEDIA_BACKEND=ffmpeg`.
- The command-line suite does no real audio I/O: device/sink behavior is verified
  `-Werror` + CI (x86) + live hardware. Gate tests
  (`SinkPipeline_Tests.cpp`, `Spectrum/FFTStreamer_Tests.cpp`,
  `IO/AudioFileLoader_Tests.cpp`) pin the kernel math.
- Audio-tree source files are CRLF; keep them that way when editing.
