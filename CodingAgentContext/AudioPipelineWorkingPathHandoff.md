# Audio Pipeline — Working-Stream Path Hand-Off

Hand-off for the session taking over **PR #1563** (ffmpeg resampler for video output
encoding), from the session that did the audio pipeline migration.

## Read this first

- **Do not pull in an ffmpeg audio resampler.** The audio pipeline already has a
  kernel resampler (`Kernels::AudioResampler`, SIMD-dispatched, gate-tested) and a
  channel mixer. Use them; let ffmpeg do only what it does for video (decode/encode/
  mux).
- Audio is exposed to consumers through a **`WorkingAudioStreamListener` path**
  (added in PR #1671). Subscribe with
  `AudioSession::add_working_stream_listener(listener)` /
  `remove_working_stream_listener(listener)`.
- The legacy `AudioFloatStreamListener` path still exists but has **no consumers**
  left. It is being kept in place (punted) until #1563 moves off of it. Once that
  is done, remove it per the checklist at the bottom.

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
| #1671 | **This path**: `WorkingAudioStreamListener` listener API on the pair/session; `StreamHistorySession` now consumes it. |

## Architecture

```
AudioSession (UI thread)  ── audio() on ConsoleSystemSession
└─ AudioPassthroughPairQtThread → AudioPassthroughPairQt (event thread, SpinLock m_lock)
   ├─ m_reader: AudioSource
   │   ├─ raw stream  → m_source_decoder (RawAudio2WorkingAudio)   [tree root]
   │   │                ├─ sink branch:  [mixer/swap] → [resampler] → m_sink_encoder → m_sink_device (QAudioSink)
   │   │                ├─ FFT branch:   [mixer 0.5/0.5] → [resampler] → m_fft_node (WorkingAudioToFFT) → InternalFFTListener
   │   │                └─ working tap:  InternalWorkingListener → m_working_listeners   ← THE NEW PATH
   │   └─ float stream → SampleListener → m_stream_listeners (legacy, PUNTED, no consumers)
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
void add_working_stream_listener(Kernels::AudioPipeline::WorkingAudioStreamListener&);
void remove_working_stream_listener(Kernels::AudioPipeline::WorkingAudioStreamListener&);
```

- Tap point: the source decoder's output — the tree's working float stream.
- Data layout: **split (planar) channels**. `push_samples(samples, stride, data)`:
  channel `c` starts at `data[c * stride]` and runs `samples` floats. `stride` is the
  per-channel buffer length (≥ `samples`); it is `0` for the single-channel float
  pass-through. Interleaved: `out[i*ch + c] = data[c*stride + i]`.
- Shape = the source's **physical** rate/channels, with the **input volume already
  applied**. For device presets: MONO_48000 → 48k/1ch, DUAL_44100 → 44.1k/2ch,
  DUAL_48000 → 48k/2ch, MONO_96000 → 96k/1ch, INTERLEAVE_LR/RL_96000 → 48k/2ch
  (the card's 1ch×96k reinterpreted as two 48k channels). File input currently
  delivers 48k/2ch (the loader still forces it); a follow-up PR makes file input
  native rate/channels — this path adapts automatically (the tap is rebuilt with the
  tree), nothing for you to handle.
- The capture-card `INTERLEAVE_RL_96000` channel swap is **not** applied on this path
  (physical channel order); `StreamHistorySession` applies it itself.
- The pair's listener set does not validate shapes (the per-tree tap node does), so a
  persistent listener with a nominal `(rate, channels)` base is safe across input
  changes — derive the live shape from `AudioChannelFormat` state callbacks if you
  need it.

## The resampler (you already have one)

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

**Suggested shape for #1563:** own a small chain in the encoding component —
your `WorkingAudioStreamListener` → optional `ChannelMixer` → `AudioResampler`
(source rate → your target rate) → your ffmpeg audio encoder input. These are plain
thread-affine kernel objects (no Qt, no locks needed if only one producer feeds
them), exactly as the gate tests use them.

## Legacy float path — removal checklist (after #1563 is off it)

The legacy reader still runs with no consumers; remove all of it in one cleanup PR:

- `AudioStream.h/.cpp`: `AudioFloatStreamListener`, `AudioStreamToFloat`.
- `IO/AudioSource.{h,cpp}`: `m_reader`, `m_listeners`, `InternalListener`,
  `add_listener(AudioFloatStreamListener&)`, `samples_per_frame()`, `m_multiplier`,
  and the preset switch in `init()` (the raw-shape accessors
  `raw_sample_rate()/raw_channels()/raw_sample_format()` stay).
- `Backends/AudioPassthroughPairQt.{h,cpp}`: `SampleListener`, `m_sample_listener`,
  `m_stream_listeners`, the float `add/remove_listener` overloads (and every
  `m_sample_listener.reset()` in the lifecycle paths).
- `AudioPassthroughPair.h` + `AudioPassthroughPairQtThread.{h,cpp}`: the float
  listener overloads.
- `AudioSession.{h,cpp}`: `add_stream_listener` / `remove_stream_listener`.

## Build & test

- `pa-build` (CMake + Ninja, Qt 6.10.2 aarch64, `-Wall -Wextra -Wpedantic -Werror`;
  incremental after the first build), then `pa-test .` (full command-line suite,
  ~18.2k tests; exit 0 on success).
- The command-line suite does no real audio I/O: device/sink behavior is verified
  `-Werror` + CI (x86) + live hardware. Gate tests
  (`SinkPipeline_Tests.cpp`, `Spectrum/FFTStreamer_Tests.cpp`) pin the kernel math.
- Audio-tree source files are CRLF; keep them that way when editing.
