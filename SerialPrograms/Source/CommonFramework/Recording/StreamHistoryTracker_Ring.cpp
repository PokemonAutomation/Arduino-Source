/*  Stream History Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Implemented by keeping an in-memory ring of *encoded* video packets
 *  (hardware H.264, with ffmpeg's built-in LGPL MPEG-4 as a fallback)
 *  and AAC audio packets. Nothing is written to storage until save() is
 *  called, at which point the ring is muxed into a single MP4/MKV file.
 *
 *  Video is encoded on a dedicated worker thread (frames that arrive faster
 *  than the encoder can keep up are dropped, like the JPEG-based tracker).
 *  Audio arrives already mixed + resampled to 48 kHz stereo from the audio
 *  pipeline's working stream and is encoded inline in push_samples() (AAC
 *  encoding of 1024 samples takes well under a millisecond).
 *
 *  Only compiled when PA_STREAM_HISTORY_FFMPEG is defined (CMake option
 *  -DPA_STREAM_HISTORY_FFMPEG=ON).
 *
 */

#include <atomic>
#include <cerrno>
#include <cctype>
#include <cstring>
#include <deque>
#include <mutex>
#include <set>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/error.h>
#include <libavutil/version.h>
#include <libavutil/log.h>
#include <libavutil/pixdesc.h>
#include <libavutil/rational.h>
#include <libswscale/swscale.h>
}

//  MSVC's <cerrno> does not define EAGAIN; ffmpeg's send/receive API reports
//  "no data" as AVERROR(EAGAIN), which needs it. POSIX defines it as 11.
#ifndef EAGAIN
#define EAGAIN 11
#endif

#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/Recording/StreamHistoryOption.h"
#include "CommonFramework/VideoPipeline/Backends/VideoFrameQt.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "StreamHistory_H264.h"
#include "StreamHistoryTracker_Ring.h"

#include <QImage>
#include <QVideoFrame>


namespace PokemonAutomation{


namespace{


//  Hard cap on the ring. The time window usually evicts first; this exists so
//  that a very large history setting cannot consume all of RAM.
constexpr int64_t MAX_RING_BYTES = 90 * 1024 * 1024;

//  Audio is always resampled to 48 kHz and encoded as AAC (frame size 1024).
constexpr int AUDIO_OUTPUT_RATE = 48000;
constexpr int AUDIO_FRAME_SAMPLES = 1024;
constexpr int64_t AUDIO_FRAME_US = 1000000LL * AUDIO_FRAME_SAMPLES / AUDIO_OUTPUT_RATE;

//  The audio timeline follows the sample count, re-anchored to the wall clock
//  whenever the two disagree by more than this (a gap in the delivered audio,
//  e.g. a device underrun, or accumulated clock drift).
constexpr int64_t AUDIO_RESYNC_THRESHOLD_US = 100000;

//  Force a keyframe every 2 seconds so save() can rewind to a decodable point.
constexpr int64_t VIDEO_KEYFRAME_INTERVAL_US = 2000000;

//  Same bounded-queue / drop-oldest scheme as the JPEG-based tracker.
constexpr size_t MAX_PENDING_FRAMES = 10;


struct VideoRingPacket{
    WallClock timestamp;
    int flags = 0;
    std::vector<uint8_t> data;
};

struct AudioRingPacket{
    WallClock timestamp;
    std::vector<uint8_t> data;
};


//  Deleter for the codec parameter copies that save() hands to mux_to_file().
struct CodecParametersDeleter{
    void operator()(AVCodecParameters* params) const{
        avcodec_parameters_free(&params);
    }
};
using CodecParametersPtr = std::unique_ptr<AVCodecParameters, CodecParametersDeleter>;

//  Returns a copy of 'source', or null if 'source' is null or the copy fails.
CodecParametersPtr copy_codec_parameters(const AVCodecParameters* source){
    if (source == nullptr){
        return nullptr;
    }
    CodecParametersPtr copy(avcodec_parameters_alloc());
    if (copy == nullptr || avcodec_parameters_copy(copy.get(), source) < 0){
        return nullptr;
    }
    return copy;
}


//  ffmpeg logs to stderr at AV_LOG_INFO by default, and probing the encoder
//  candidates is noisy. The level is process-wide, so it is set once, when
//  the first tracker is built, rather than reset by every tracker.
void quiet_ffmpeg_logging_once(){
    static const bool done = []{
        av_log_set_level(AV_LOG_ERROR);
        return true;
    }();
    (void)done;
}


std::string ffmpeg_error(int errnum){
    char buffer[AV_ERROR_MAX_STRING_SIZE] = {};
    av_strerror(errnum, buffer, sizeof(buffer));
    return buffer;
}


//  Initializes the layout with the given channel count. av_channel_layout_default()
//  returns void (validate the result with av_channel_layout_check()).
static bool init_channel_layout(AVChannelLayout& layout, int channels){
    av_channel_layout_uninit(&layout);
    av_channel_layout_default(&layout, channels);
    return av_channel_layout_check(&layout) > 0;
}


//  The configured target FPS (falls back to 10 for unknown settings).
size_t get_target_fps(){
    const StreamHistoryOption& settings = GlobalSettings::instance().STREAM_HISTORY;
    switch (settings.VIDEO_FPS){
    case StreamHistoryOption::VideoFPS::FPS_30:  return 30;
    case StreamHistoryOption::VideoFPS::FPS_15:  return 15;
    case StreamHistoryOption::VideoFPS::FPS_10:  return 10;
    case StreamHistoryOption::VideoFPS::FPS_05:  return 5;
    case StreamHistoryOption::VideoFPS::FPS_01:  return 1;
    }
    return 10;
}


//  The target video bitrate in kbps: the configured fixed value (clamped to
//  100-8000) or the quality preset's value.
size_t get_target_bitrate_kbps(){
    const StreamHistoryOption& settings = GlobalSettings::instance().STREAM_HISTORY;
    //  Not reachable at the moment: ENCODING_MODE is not shown in the ffmpeg
    //  builds (StreamHistoryOption), so it stays at FIXED_QUALITY.
    if (settings.ENCODING_MODE == StreamHistoryOption::EncodingMode::FIXED_BITRATE){
        //  Clamp to a sane range. (The ring's memory cap still applies: at
        //  8000 kbps, MAX_RING_BYTES holds about 90 seconds.)
        uint32_t kbps = settings.VIDEO_BITRATE;
        if (kbps < 100) kbps = 100;
        if (kbps > 8000) kbps = 8000;
        return kbps;
    }
    switch (settings.VIDEO_QUALITY){
    case StreamHistoryOption::VideoQuality::VERY_LOW:  return 128;
    case StreamHistoryOption::VideoQuality::LOW:       return 512;
    case StreamHistoryOption::VideoQuality::NORMAL:    return 1024;
    case StreamHistoryOption::VideoQuality::HIGH:      return 2048;
    case StreamHistoryOption::VideoQuality::VERY_HIGH: return 4096;
    }
    return 512;
}


//  Resolves the configured output resolution (match input / 720p / 1080p)
//  to even H.264-compatible dimensions, preserving the input aspect ratio,
//  into out_width / out_height. Returns false if the size is unusable.
bool get_target_video_size(int input_width, int input_height, int& out_width, int& out_height){
    const StreamHistoryOption& settings = GlobalSettings::instance().STREAM_HISTORY;
    int target_width;
    switch (settings.RESOLUTION){
    case StreamHistoryOption::Resolution::MATCH_INPUT:
        target_width = input_width;
        break;
    case StreamHistoryOption::Resolution::FORCE_720p:
        target_width = 1280;
        break;
    case StreamHistoryOption::Resolution::FORCE_1080p:
        target_width = 1920;
        break;
    default:
        return false;
    }
    if (target_width <= 0 || input_width <= 0 || input_height <= 0){
        return false;
    }
    int target_height = input_height * target_width / input_width;
    //  H.264 (and the mpeg4 fallback) require even dimensions.
    out_width = target_width & ~1;
    out_height = target_height & ~1;
    return (out_width >= 16 && out_height >= 16);
}


//  Ordered by preference: the hardware encoders first, then (Windows) the
//  OS-native Media Foundation H.264 encoder, then ffmpeg's
//  built-in (LGPL) MPEG-4 Part 2 encoder as a last resort. Every candidate
//  in this list ships in an LGPL ffmpeg build; GPL components (libx264)
//  are deliberately not candidates.
const char* const VIDEO_ENCODER_CANDIDATES[] = {
#ifdef _WIN32
    //  h264_mf: the OS-native Media Foundation H.264 encoder (typically the
    //  built-in Microsoft encoder). Present on most Windows 8+ installs (not
    //  on N editions without the Media Feature Pack) and compiled into the
    //  vendored build; when none of the GPU vendors'
    //  encoders is available this yields real, universally playable H.264
    //  instead of MPEG-4 Part 2.
    "h264_qsv",           "h264_nvenc",           "h264_amf",           "h264_mf",
#elif defined(__APPLE__)
    "h264_videotoolbox",
#else
    "h264_v4l2m2m",
#endif
    "mpeg4",
};

//  A hardware encoder that hard-fails at runtime (avcodec_send_frame() or
//  avcodec_receive_packet() returns an error other than EAGAIN/EOF) has
//  likely broken its GPU driver: calling into the context again - even to
//  release it - can crash the process inside the driver (observed:
//  0xC0000005 access violation in atidxx64.dll during
//  avcodec_free_context() on a Ryzen 5 5625U iGPU). Such encoders are
//  "poisoned" for the rest of the process: the failed context is
//  deliberately leaked (at most one per encoder) and the encoder is never
//  initialized again, not even by later trackers.
Mutex poisoned_encoder_mutex;
std::set<std::string> poisoned_encoders;

//  Abandons 'ctx' after a runtime encoder failure (see above). The CPU
//  fallback (mpeg4) has no device behind it and is released normally; every
//  other candidate (the hardware wrappers and Media Foundation) is leaked and
//  poisoned. Contexts that merely fail to open are released by
//  init_video_encoder() instead: no device was exercised through them.
void abandon_video_encoder(Logger& logger, const char* name, AVCodecContext*& ctx){
    if (ctx == nullptr){
        return;
    }
    if (std::strcmp(name, "mpeg4") == 0){
        avcodec_free_context(&ctx);
    }else{
        {
            std::lock_guard<Mutex> lg(poisoned_encoder_mutex);
            poisoned_encoders.insert(name);
        }
        logger.log(
            "Stream history: leaking the failed '"
            + std::string(name) + "' video encoder context; releasing it can"
            + " crash the GPU driver, and it will not be used again in this"
            + " process.",
            COLOR_YELLOW
        );
    }
    ctx = nullptr;
}


}  // namespace




//  Threads:
//  - The encoder thread (worker_loop()) owns the video encoder and its state:
//    m_vctx, m_sws and the m_sws_* fields, m_video_width, m_video_height,
//    m_video_start, m_last_keyframe_us, m_last_pts_us,
//    m_logged_unsupported_format and m_video_encoder_index. Nothing else
//    touches them.
//  - The audio thread (push_samples()) owns the AAC encoder, the sample
//    accumulator and the audio clock: m_actx, m_audio_layout, m_have_audio,
//    m_audio_acc, m_audio_samples_total, m_audio_anchored,
//    m_audio_anchor_sample and m_audio_anchor_time.
//  - The video thread (on_frame()) throttles frames and queues them.
//  - save() and set_window() can come from any thread.
//  - m_lock guards what these threads share: the rings (m_video, m_audio,
//    m_ring_bytes), the codec parameter copies (m_video_params,
//    m_audio_params, m_video_time_base), m_window, m_next_frame_time, and
//    m_video_enabled (written by the encoder thread, read by on_frame()).
//  - m_queue_lock guards the frame queue (m_pending_frames), the encoder
//    thread's flags (m_stopping, m_encoding) and the dropped-frame report
//    (m_dropped_frames, m_last_drop_report).
//  - Fields set in the constructor and never changed afterwards (m_logger,
//    m_has_video, m_audio_channels, m_target_fps, m_frame_interval) need
//    no lock.
struct StreamHistoryTracker_Ring::Data{
    Logger& m_logger;
    mutable SpinLock m_lock;    //  See "Threads" above.
    std::chrono::seconds m_window;
    bool m_has_video;

    //  Audio configuration. The working stream is attached at a fixed
    //  48 kHz / stereo shape, so the audio is always 2 channels regardless
    //  of the input preset (the pipeline tree's mixer upmixes mono to
    //  dual-mono).
    size_t m_audio_channels = 2;
    bool m_have_audio;

    //  Video frame throttling (same scheme as StreamHistoryTracker_SaveFrames).
    size_t m_target_fps;
    std::chrono::microseconds m_frame_interval;
    WallClock m_next_frame_time = WallClock::min();

    //  The ring.
    std::deque<std::shared_ptr<const VideoRingPacket>> m_video;
    std::deque<std::shared_ptr<const AudioRingPacket>> m_audio;
    int64_t m_ring_bytes = 0;
    std::atomic<bool> m_reported_size_cap{false};

    //  Video encoder (initialized on the worker thread at the first frame).
    bool m_video_enabled;
    bool m_logged_unsupported_format = false;
    AVCodecContext* m_vctx = nullptr;
    SwsContext* m_sws = nullptr;
    int m_sws_in_format = AV_PIX_FMT_NONE;
    int m_sws_width = 0;
    int m_sws_height = 0;
    int m_sws_out_format = AV_PIX_FMT_NONE;
    int m_video_width = 0;
    int m_video_height = 0;
    int64_t m_last_keyframe_us = -VIDEO_KEYFRAME_INTERVAL_US;
    int64_t m_last_pts_us = -1;
    WallClock m_video_start = WallClock::min();

    //  Encoder candidate rotation. A hardware encoder can open successfully
    //  and still fail on every frame (e.g. its GPU device cannot allocate
    //  textures); after a hard failure from avcodec_send_frame() or
    //  avcodec_receive_packet() the current candidate is abandoned and the
    //  next one is tried, so the recording
    //  still gets video. Candidates that have failed (at open or at runtime)
    //  are not retried. Only the worker thread touches these.
    size_t m_video_encoder_index = 0;

    //  Codec parameter snapshots for muxing, owned by Data, guarded by
    //  `m_lock`. ffmpeg is not thread-safe on a live AVCodecContext, and
    //  mux_to_file() runs on the caller's thread while the video worker and
    //  the audio pipeline thread keep encoding into m_vctx/m_actx, so the muxer
    //  must never read those contexts directly (a torn read of, e.g., the
    //  lazily-populated hardware-encoder extradata crashes the mp4 muxer).
    //  Each snapshot is refreshed on its encoder's own thread when the first
    //  packet arrives and whenever the extradata size changes (the hardware
    //  encoders fill it in lazily).
    AVCodecParameters* m_video_params = nullptr;
    AVRational m_video_time_base = {1, 1000};
    AVCodecParameters* m_audio_params = nullptr;

    //  Audio encoder (used by push_samples).
    AVCodecContext* m_actx = nullptr;
    AVChannelLayout m_audio_layout = {};
    int64_t m_audio_samples_total = 0;    //  Samples (per channel) sent to the encoder.
    //  The audio sample clock: sample m_audio_anchor_sample was captured at
    //  m_audio_anchor_time. Re-anchored on gaps; see push_samples().
    bool m_audio_anchored = false;
    int64_t m_audio_anchor_sample = 0;
    WallClock m_audio_anchor_time;
    std::vector<float> m_audio_acc;    //  Interleaved float @ 48 kHz, accumulated from the planar working stream.

    //  Video worker thread.
    AsyncTask m_worker;
    Mutex m_queue_lock;
    bool m_stopping = false;    //  Guarded by m_queue_lock.
    bool m_encoding = false;    //  Guarded by m_queue_lock. A popped frame is being encoded.
    ConditionVariable m_cv;
    ConditionVariable m_idle_cv;    //  Notified after each frame is encoded (wait_until_idle()).
    std::deque<std::shared_ptr<const VideoFrame>> m_pending_frames;
    size_t m_dropped_frames = 0;    //  Guarded by m_queue_lock. Since the last report.
    WallClock m_last_drop_report = WallClock::min();    //  Guarded by m_queue_lock.

    //  Initializes the audio encoder (if enabled) and starts the video
    //  worker thread (if enabled); the video encoder itself is opened
    //  lazily on the first frame.
    Data(
        Logger& p_logger,
        std::chrono::seconds p_window,
        bool p_has_audio,
        bool p_has_video
    )
        : m_logger(p_logger)
        , m_window(p_window)
        , m_has_video(p_has_video)
        , m_have_audio(p_has_audio)
        , m_target_fps(get_target_fps())
        , m_frame_interval(std::chrono::microseconds(1000000 / m_target_fps))
        , m_video_enabled(p_has_video)
    {
        quiet_ffmpeg_logging_once();

        if (m_have_audio){
            init_audio_encoder();
        }
        if (m_has_video){
            m_worker = GlobalThreadPools::unlimited_normal().dispatch_now_blocking(
                [this]{ worker_loop(); }
            );
        }
    }

    //  Releases the ffmpeg resources (the worker thread is already stopped
    //  by ~StreamHistoryTracker_Ring()).
    ~Data(){
        if (m_sws != nullptr) sws_freeContext(m_sws);
        if (m_video_params != nullptr) avcodec_parameters_free(&m_video_params);
        if (m_audio_params != nullptr) avcodec_parameters_free(&m_audio_params);
        if (m_vctx != nullptr) avcodec_free_context(&m_vctx);
        if (m_actx != nullptr) avcodec_free_context(&m_actx);
    }


    //  Allocates + opens the AAC encoder (FLTP, 48 kHz, 1024-sample frames,
    //  96 kbps, stream config in the container header) at the tracker's
    //  fixed channel count; any failure disables audio (m_have_audio).
    void init_audio_encoder(){
        const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
        if (codec == nullptr){
            m_logger.log("Stream history: AAC encoder not found in ffmpeg build. Disabling audio.", COLOR_RED);
            m_have_audio = false;
            return;
        }

        // allocate an AAC encoder context
        m_actx = avcodec_alloc_context3(codec);
        if (m_actx == nullptr){
            m_logger.log("Stream history: Failed to allocate AAC encoder. Disabling audio.", COLOR_RED);
            m_have_audio = false;
            return;
        }
        m_actx->sample_rate = AUDIO_OUTPUT_RATE;
        m_actx->time_base = av_make_q(1, AUDIO_OUTPUT_RATE);
        m_actx->bit_rate = 96000;
        m_actx->frame_size = AUDIO_FRAME_SAMPLES;
        //  The AAC encoder requires the sample format to be set explicitly;
        //  avcodec_open2() fails with EINVAL ("Invalid audio sample format")
        //  if it is left at AV_SAMPLE_FMT_NONE. The encoder works in planar
        //  float; push_samples() accumulates the planar working stream
        //  packed/interleaved, and the encode loop de-interleaves it into
        //  the frame's planes.
        m_actx->sample_fmt = AV_SAMPLE_FMT_FLTP;
        //  The AAC stream configuration (AudioSpecificConfig) must land in the
        //  container header: with this flag the encoder emits raw AAC frames +
        //  extradata, which is what both the MP4 and MKV muxers need. Without
        //  it the packets would be ADTS and mux_to_file() would fail to set up
        //  the audio stream.
        m_actx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        // init AAC channel layout
        if (!init_channel_layout(m_actx->ch_layout, (int)m_audio_channels)){
            m_logger.log(
                "Stream history: Failed to initialize the AAC channel layout ("
                + std::to_string((int)m_audio_channels) + " channels). Disabling audio.",
                COLOR_RED
            );
            avcodec_free_context(&m_actx);
            m_actx = nullptr;
            m_have_audio = false;
            return;
        }

        // open the AAC encoder
        int open_err = avcodec_open2(m_actx, codec, nullptr);
        if (open_err < 0){
            m_logger.log(
                "Stream history: Failed to open AAC encoder: "
                + ffmpeg_error(open_err)
                + ". Disabling audio.",
                COLOR_RED
            );
            avcodec_free_context(&m_actx);
            m_actx = nullptr;
            m_have_audio = false;
            return;
        }
        //  The encoded frames use the same layout as the encoder (which
        //  just opened successfully with this channel count).
        init_channel_layout(m_audio_layout, (int)m_audio_channels);
        m_logger.log(
            "Stream history: audio = AAC " + std::to_string(m_audio_channels) + "ch @ 48 kHz ("
            + std::to_string(m_actx->bit_rate / 1000) + " kbps)."
        );
    }


    //  Tries the encoder candidates from m_video_encoder_index (skipping
    //  poisoned ones) and opens the first that works: sets m_vctx, resets
    //  the per-encoder state, and frees the old parameter snapshot. If none
    //  work, video is disabled (m_video_enabled).
    void init_video_encoder(int input_width, int input_height){
        constexpr size_t CANDIDATE_COUNT = sizeof(VIDEO_ENCODER_CANDIDATES)/sizeof(VIDEO_ENCODER_CANDIDATES[0]);
        //  Candidates before m_video_encoder_index already failed. Once past
        //  the last one (e.g. the last candidate opened but then failed at
        //  runtime), there is nothing left to try: disable video so the
        //  caller never proceeds with a null m_vctx.
        if (m_video_encoder_index >= CANDIDATE_COUNT){
            m_logger.log("Stream history: No usable video encoder left. Disabling video.", COLOR_RED);
            WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
            m_video_enabled = false;
            return;
        }

        int target_width = 0;
        int target_height = 0;
        if (!get_target_video_size(input_width, input_height, target_width, target_height)){
            m_logger.log("Stream history: Unsupported input resolution. Disabling video.", COLOR_RED);
            WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
            m_video_enabled = false;
            return;
        }

        size_t bitrate_kbps = get_target_bitrate_kbps();
        for (size_t i = m_video_encoder_index; i < CANDIDATE_COUNT; i++){
            const char* name = VIDEO_ENCODER_CANDIDATES[i];
            {
                std::lock_guard<Mutex> lg(poisoned_encoder_mutex);
                if (poisoned_encoders.count(name) != 0){
                    m_logger.log(
                        "Stream history: skipping video encoder '"
                        + std::string(name) + "' (it failed at runtime earlier"
                        + " in this process).",
                        COLOR_YELLOW
                    );
                    m_video_encoder_index = i + 1;
                    continue;
                }
            }
            const AVCodec* codec = avcodec_find_encoder_by_name(name);
            if (codec == nullptr){
                m_video_encoder_index = i + 1;
                continue;
            }
            AVCodecContext* ctx = avcodec_alloc_context3(codec);
            if (ctx == nullptr){
                m_video_encoder_index = i + 1;
                continue;
            }
            ctx->width = target_width;
            ctx->height = target_height;
            //  Milliseconds as the video timebase: the mpeg4 encoder rejects
            //  a timebase denominator above 65535 (the 16-bit
            //  time_incr_length of the MPEG-4 Part 2 standard) with EINVAL,
            //  and the hardware H.264 wrappers accept any timebase. The ring
            //  timestamps are rebuilt from the encoder's output pts (see
            //  drain_one_video_packet()), so the video timing in a saved file
            //  is millisecond-precise.
            ctx->time_base = av_make_q(1, 1000);
            //  The frame rate must be set explicitly: the h264_amf wrapper
            //  passes it to the AMD driver, which rejects - and tears down
            //  its D3D11 device for - a missing/zero frame rate ("unexpected
            //  frame rate" followed by AMF_NO_DEVICE).
            ctx->framerate = av_make_q((int)m_target_fps, 1);
            ctx->bit_rate = (int64_t)bitrate_kbps * 1000;
            ctx->gop_size = (int)(m_target_fps * 2);
            ctx->max_b_frames = 0;
            ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

            //  Pick the input pixel format: prefer NV12 (supported by all the
            //  hardware encoders and cheap to scale to), otherwise take the
            //  first format the encoder advertises. If no list can be
            //  determined at all, use YUV420P, which every encoder in the
            //  candidate list supports; some encoders (mpeg4) reject being
            //  opened without an explicit pixel format.
            AVPixelFormat preferred = AV_PIX_FMT_NONE;
            const enum AVPixelFormat* pix_fmts = nullptr;
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(61, 13, 100)
            //  avcodec_get_supported_config() was added in libavcodec 61.13
            //  (ffmpeg 7.1), which deprecated AVCodec.pix_fmts; ffmpeg 8.0
            //  removed it. ffmpeg 7.0 is libavcodec 61.3, so it takes the
            //  AVCodec.pix_fmts path below. This split is a preprocessor
            //  version gate (rather than compile-time detection) because the
            //  function is not declared in every header version this project
            //  supports (5.1 and later), and names in discarded if-constexpr
            //  branches are still looked up.
            //  (out_num_configs is optional; out_configs points at a list
            //  terminated by AV_PIX_FMT_NONE, or is NULL when every format is
            //  supported, in which case the YUV420P default below applies.)
            int ret = avcodec_get_supported_config(
                ctx, codec, AV_CODEC_CONFIG_PIX_FORMAT, 0, (const void**)&pix_fmts, nullptr
            );
            if (ret < 0){
                pix_fmts = nullptr;
            }
#else
            pix_fmts = codec->pix_fmts;
#endif
            if (pix_fmts != nullptr){
                for (size_t j = 0; pix_fmts[j] != AV_PIX_FMT_NONE; j++){
                    AVPixelFormat format = pix_fmts[j];
                    if (format == AV_PIX_FMT_NV12){
                        preferred = AV_PIX_FMT_NV12;
                        break;
                    }
                    if (preferred == AV_PIX_FMT_NONE){
                        preferred = format;
                    }
                }
            }
            if (preferred == AV_PIX_FMT_NONE){
                preferred = AV_PIX_FMT_YUV420P;
            }
            ctx->pix_fmt = preferred;
            int err = avcodec_open2(ctx, codec, nullptr);
            if (err == 0){
                m_vctx = ctx;
                m_video_encoder_index = i + 1;
                size_t dropped_packets;
                {
                    WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
                    //  New encoder: the old snapshot (if any) no longer
                    //  matches what the ring will hold going forward;
                    //  drain_one_video_packet() refreshes it on the first
                    //  encoded packet.
                    avcodec_parameters_free(&m_video_params);
                    //  The video packets already in the ring came from the
                    //  previous encoder (after a runtime fallback) and cannot
                    //  be muxed with the new encoder's parameters, so drop
                    //  them. (Empty on the first initialization.)
                    dropped_packets = m_video.size();
                    for (const std::shared_ptr<const VideoRingPacket>& packet : m_video){
                        m_ring_bytes -= (int64_t)packet->data.size();
                    }
                    m_video.clear();
                }
                if (dropped_packets != 0){
                    m_logger.log(
                        "Stream history: dropped " + std::to_string(dropped_packets)
                        + " video packets from the previous encoder.",
                        COLOR_YELLOW
                    );
                }
                m_video_width = target_width;
                m_video_height = target_height;
                m_video_start = WallClock::min();
                m_last_keyframe_us = -VIDEO_KEYFRAME_INTERVAL_US;
                m_last_pts_us = -1;
                m_logger.log(
                    "Stream history: video encoder = " + std::string(name)
                    + " ("
                    + std::string(av_get_pix_fmt_name(ctx->pix_fmt)) + ")"
                    + " at " + std::to_string(target_width) + "x" + std::to_string(target_height)
                    + ", " + std::to_string(bitrate_kbps) + " kbps."
                );
                return;
            }
            m_video_encoder_index = i + 1;
            m_logger.log(
                "Stream history: Video encoder '" + std::string(name) + "' failed to open: "
                + ffmpeg_error(err),
                COLOR_YELLOW
            );
            //  The encoder never opened, so no device was ever exercised;
            //  releasing the context is safe (unlike a context that failed at
            //  runtime - see abandon_video_encoder()).
            avcodec_free_context(&ctx);
        }
        m_logger.log("Stream history: No usable video encoder found. Disabling video.", COLOR_RED);
        WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
        m_video_enabled = false;
    }


    //  Receive and store a single encoded video packet.
    //  Returns 1 if a packet was stored, 0 if there is nothing to receive, or
    //  the (negative) ffmpeg error if the encoder failed.
    int drain_one_video_packet(){
        AVPacket* packet = av_packet_alloc();
        if (packet == nullptr){
            m_logger.log("Stream history: Failed to allocate video packet.", COLOR_RED);
            return 0;
        }
        int err = avcodec_receive_packet(m_vctx, packet);
        if (err == AVERROR(EAGAIN) || err == AVERROR_EOF){
            av_packet_free(&packet);
            return 0;
        }
        if (err < 0){
            av_packet_free(&packet);
            return err;
        }
        auto ring_packet = std::make_shared<VideoRingPacket>();
        //  Stamp the packet with the capture time of the frame it encodes.
        //  The encoder can hold several frames before emitting (the hardware
        //  encoders pipeline their input), so the frame most recently sent is
        //  not necessarily this packet's. The pts round-trips through the
        //  encoder: milliseconds since m_video_start (see encode_video_frame()).
        ring_packet->timestamp = m_video_start + std::chrono::milliseconds(
            packet->pts != AV_NOPTS_VALUE ? packet->pts : m_last_pts_us / 1000
        );
        ring_packet->flags = (packet->flags & AV_PKT_FLAG_KEY) ? AV_PKT_FLAG_KEY : 0;
        ring_packet->data.assign(packet->data, packet->data + packet->size);
        av_packet_free(&packet);

        int64_t added = (int64_t)ring_packet->data.size();
        bool capped;
        {
            WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
            //  Refresh the mux-time parameter snapshot on this (the encoder's)
            //  thread whenever the encoder state changes; the hardware encoders
            //  populate their extradata lazily, sometimes only after the first
            //  encoded packet. mux_to_file() must not read the live context.
            if (m_video_params == nullptr
                || m_video_params->extradata_size != m_vctx->extradata_size
            ){
                if (m_video_params == nullptr){
                    m_video_params = avcodec_parameters_alloc();
                }
                if (m_video_params != nullptr){
                    avcodec_parameters_from_context(m_video_params, m_vctx);
                    m_video_time_base = m_vctx->time_base;
                }
            }
            m_video.push_back(std::move(ring_packet));
            m_ring_bytes += added;
            capped = evict();
        }
        if (capped){
            report_size_cap();
        }
        return 1;
    }


    //  The worker thread: blocks on m_pending_frames, encodes each frame in
    //  order, and exits on shutdown (dropping any frames still queued).
    void worker_loop(){
        while (true){
            std::shared_ptr<const VideoFrame> frame;
            {
                std::unique_lock<Mutex> queue_guard(m_queue_lock);
                m_cv.wait(queue_guard, [this]{ return m_stopping || !m_pending_frames.empty(); });
                if (m_stopping){
                    //  Shutting down: drop any queued frames instead of
                    //  encoding them (the ring is about to be destroyed).
                    return;
                }
                frame = std::move(m_pending_frames.front());
                m_pending_frames.pop_front();
                m_encoding = true;
            }
            encode_video_frame(frame);
            {
                std::lock_guard<Mutex> queue_guard(m_queue_lock);
                m_encoding = false;
            }
            m_idle_cv.notify_all();
        }
    }


    //  Blocks until the queue is empty and no frame is being encoded.
    void wait_until_idle(){
        std::unique_lock<Mutex> queue_guard(m_queue_lock);
        m_idle_cv.wait(queue_guard, [this]{
            return m_stopping || (m_pending_frames.empty() && !m_encoding);
        });
    }


    //  Abandons the current encoder after a hard failure at runtime (from
    //  avcodec_send_frame() or avcodec_receive_packet()) and opens the next
    //  candidate. A hardware encoder that hard-fails at runtime has a broken
    //  GPU driver; every further call into it - including releasing the
    //  context - risks crashing the process inside the driver.
    void fall_back_video_encoder(int err, int input_width, int input_height){
        const char* failed = VIDEO_ENCODER_CANDIDATES[m_video_encoder_index - 1];
        m_logger.log(
            "Stream history: video encoder '" + std::string(failed)
            + "' failed at runtime ("
            + ffmpeg_error(err)
            + "); falling back to the next candidate.",
            COLOR_YELLOW
        );
        abandon_video_encoder(m_logger, failed, m_vctx);
        init_video_encoder(input_width, input_height);
    }


    //  Converts one frame to the encoder's pixel format (NV12 direct,
    //  everything else through RGB), (re)building the scaler as needed,
    //  then feeds it to the encoder: wall-clock-derived pts, a forced
    //  keyframe every VIDEO_KEYFRAME_INTERVAL_US, and an EAGAIN
    //  drain-and-retry. A hard send or receive failure abandons the
    //  (possibly broken) encoder and opens the next candidate.
    void encode_video_frame(const std::shared_ptr<const VideoFrame>& vf){
        if (!m_video_enabled || vf == nullptr || !vf->frame.isValid()){
            return;
        }

        QVideoFrame qframe = vf->frame;    //  Local copy (implicit sharing).
        if (!qframe.map(QVideoFrame::ReadOnly)){
            return;
        }
        bool still_mapped = true;
        auto unmap_if_needed = [&qframe, &still_mapped]{
            if (still_mapped){
                qframe.unmap();
                still_mapped = false;
            }
        };

        //  NV12 (the common capture card format) goes straight to the scaler;
        //  everything else is converted to RGB888 through Qt, like the
        //  JPEG-based tracker.
        int src_format = AV_PIX_FMT_NONE;
        int src_width = 0;
        int src_height = 0;
        const uint8_t* src_planes[4] = { nullptr, nullptr, nullptr, nullptr };
        int src_strides[4] = { 0, 0, 0, 0 };
        QImage rgb_image;
        if (qframe.pixelFormat() == QVideoFrameFormat::Format_NV12){
            src_format = AV_PIX_FMT_NV12;
            src_width = qframe.width();
            src_height = qframe.height();
            src_planes[0] = (const uint8_t*)qframe.bits(0);
            src_planes[1] = (const uint8_t*)qframe.bits(1); // Accurate way to get the UV plane
            src_strides[0] = qframe.bytesPerLine(0);
            src_strides[1] = qframe.bytesPerLine(1);
        }else{
            rgb_image = qframe.toImage().convertToFormat(QImage::Format_RGB888);
            unmap_if_needed();
            if (rgb_image.isNull()){
                if (!m_logged_unsupported_format){
                    m_logger.log("Stream history: Could not convert video frame to RGB. Skipping.", COLOR_RED);
                    m_logged_unsupported_format = true;
                }
                return;
            }
            src_format = AV_PIX_FMT_RGB24;
            src_width = rgb_image.width();
            src_height = rgb_image.height();
            src_planes[0] = (const uint8_t*)rgb_image.bits();
            src_strides[0] = (int)rgb_image.bytesPerLine();
        }

        if (m_vctx == nullptr){
            init_video_encoder(qframe.width(), qframe.height());
            if (!m_video_enabled){
                unmap_if_needed();
                return;
            }
        }

        //  (Re)build the scaler if the input changed, or if the encoder's
        //  preferred pixel format changed (e.g. after an encoder fallback).
        if (src_format != m_sws_in_format || src_width != m_sws_width || src_height != m_sws_height
            || m_vctx->pix_fmt != m_sws_out_format){
            if (m_sws != nullptr){
                sws_freeContext(m_sws);
                m_sws = nullptr;
            }
            m_sws = sws_getContext(
                src_width, src_height, (AVPixelFormat)src_format,
                m_video_width, m_video_height, m_vctx->pix_fmt,
                SWS_BILINEAR, nullptr, nullptr, nullptr
            );
            if (m_sws == nullptr){
                m_logger.log("Stream history: Failed to create video scaler.", COLOR_RED);
                unmap_if_needed();
                return;
            }
            m_sws_in_format = src_format;
            m_sws_width = src_width;
            m_sws_height = src_height;
            m_sws_out_format = m_vctx->pix_fmt;
        }

        AVFrame* frame = av_frame_alloc();
        if (frame == nullptr){
            unmap_if_needed();
            return;
        }
        frame->format = m_vctx->pix_fmt;
        frame->width = m_video_width;
        frame->height = m_video_height;
        if (av_frame_get_buffer(frame, 0) < 0){
            av_frame_free(&frame);
            unmap_if_needed();
            return;
        }

        int scaled = sws_scale(
            m_sws,
            src_planes, src_strides,
            0, src_height,
            frame->data, frame->linesize
        );
        unmap_if_needed();
        if (scaled != m_video_height){
            av_frame_free(&frame);
            return;
        }

        //  Timestamps + keyframe control.
        if (m_video_start == WallClock::min()){
            m_video_start = vf->timestamp;
        }
        int64_t pts_us = std::chrono::duration_cast<std::chrono::microseconds>(
            vf->timestamp - m_video_start
        ).count();
        //  The encoders need strictly increasing pts (mpeg4 rejects a repeat
        //  as a hard error, which would trigger an encoder fallback), and the
        //  pts is in milliseconds. on_frame()'s throttling already keeps the
        //  sent frames a frame interval apart; drop any frame that would still
        //  not advance the pts (e.g. a capture timestamp going backwards).
        if (m_last_pts_us >= 0 && pts_us / 1000 <= m_last_pts_us / 1000){
            av_frame_free(&frame);
            return;
        }
        m_last_pts_us = pts_us;
        bool force_keyframe = (pts_us - m_last_keyframe_us) >= VIDEO_KEYFRAME_INTERVAL_US;
        if (force_keyframe){
            m_last_keyframe_us = pts_us;
        }
        //  The video timebase is 1/1000 (milliseconds); see init_video_encoder.
        frame->pts = pts_us / 1000;
        if (force_keyframe){
            frame->pict_type = AV_PICTURE_TYPE_I;
        }

        //  Send the frame. Some encoders (notably hardware ones) buffer input,
        //  so a send can fail with EAGAIN while output packets are pending:
        //  drain one packet and retry instead of giving up.
        while (true){
            int err = avcodec_send_frame(m_vctx, frame);
            if (err == AVERROR(EAGAIN)){
                int drained = drain_one_video_packet();
                if (drained < 0){
                    av_frame_free(&frame);
                    fall_back_video_encoder(drained, qframe.width(), qframe.height());
                    return;
                }
                if (drained == 0){
                    av_frame_free(&frame);
                    return;
                }
                continue;
            }
            if (err < 0){
                av_frame_free(&frame);
                fall_back_video_encoder(err, qframe.width(), qframe.height());
                return;
            }
            av_frame_free(&frame);
            break;
        }

        while (true){
            int drained = drain_one_video_packet();
            if (drained < 0){
                fall_back_video_encoder(drained, qframe.width(), qframe.height());
                return;
            }
            if (drained == 0){
                break;
            }
        }
    }


    //  Drops the packets that fell out of the window (or past the memory
    //  cap). Returns true if the memory cap dropped packets the window would
    //  have kept. Must call under m_lock.
    bool evict(){
        WallClock latest = WallClock::min();
        if (!m_video.empty()){
            latest = m_video.back()->timestamp;
        }
        if (!m_audio.empty() && m_audio.back()->timestamp > latest){
            latest = m_audio.back()->timestamp;
        }
        if (latest == WallClock::min()){
            return false;
        }
        //  Keep one extra keyframe interval of headroom: save() starts the
        //  file at the first keyframe in the ring, and a keyframe is forced
        //  on the first frame at least VIDEO_KEYFRAME_INTERVAL_US after the
        //  previous one (so up to one frame interval later than that).
        //  Evicting at "latest - m_window" would let the first keyframe fall
        //  up to a full interval after the window's start point, shortening
        //  the saved file by as much. With the headroom, the first keyframe
        //  is at most one frame interval after "latest - m_window": the saved
        //  file covers the most recent "window" seconds give or take a frame,
        //  and may start up to one keyframe interval earlier. The memory cap
        //  (MAX_RING_BYTES) can still shorten it.
        WallClock threshold =
            latest - m_window - std::chrono::microseconds(VIDEO_KEYFRAME_INTERVAL_US);
        bool capped = false;
        while (!m_video.empty() && (m_video.front()->timestamp < threshold || m_ring_bytes > MAX_RING_BYTES)){
            capped |= m_video.front()->timestamp >= threshold;
            m_ring_bytes -= (int64_t)m_video.front()->data.size();
            m_video.pop_front();
        }
        while (!m_audio.empty() && (m_audio.front()->timestamp < threshold || m_ring_bytes > MAX_RING_BYTES)){
            capped |= m_audio.front()->timestamp >= threshold;
            m_ring_bytes -= (int64_t)m_audio.front()->data.size();
            m_audio.pop_front();
        }
        return capped;
    }

    //  Logs (once per tracker) that the memory cap is shortening the history.
    void report_size_cap(){
        if (m_reported_size_cap.exchange(true)){
            return;
        }
        m_logger.log(
            "Stream history: the history reached its "
            + std::to_string(MAX_RING_BYTES / (1024 * 1024))
            + " MB memory limit, so saved videos will be shorter than the"
            + " configured history length. Lower the video quality or the"
            + " history length to keep the full history.",
            COLOR_ORANGE
        );
    }


    //  Throttles the incoming frames to the target FPS and queues them for
    //  the worker thread, dropping the oldest queued frame when full.
    void on_frame(std::shared_ptr<const VideoFrame> frame){
        if (!m_has_video || frame == nullptr || frame->timestamp == WallClock::min()){
            return;
        }
        {
            WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
            if (!m_video_enabled){
                return;
            }

            //  Initialize on first frame.
            if (m_next_frame_time == WallClock::min()){
                m_next_frame_time = frame->timestamp;
            }

            //  Don't save every frame: only save frames as per m_target_fps.
            if (frame->timestamp < m_next_frame_time){
                return;
            }

            //  If the stream has been idle (e.g. a gap between frames), the while
            //  loop advances the schedule until it is once again ahead of the
            //  current timestamp. The resulting gap in the ring is fine because
            //  the muxer uses real timestamps.
            while (m_next_frame_time <= frame->timestamp){
                m_next_frame_time += m_frame_interval;
            }
        }

        std::string drop_report;
        {
            std::lock_guard<Mutex> queue_guard(m_queue_lock);

            //  Drop oldest if we are falling behind. Reported at most every
            //  10 seconds: when the encoder cannot keep up, this happens on
            //  every frame.
            if (m_pending_frames.size() >= MAX_PENDING_FRAMES){
                m_pending_frames.pop_front();
                m_dropped_frames++;
                WallClock now = current_time();
                if (m_last_drop_report == WallClock::min()
                    || now - m_last_drop_report >= std::chrono::seconds(10)
                ){
                    drop_report = "Stream history worker thread lagging: dropped "
                        + std::to_string(m_dropped_frames) + " frame(s)"
                        + (m_last_drop_report == WallClock::min() ? "." : " in the last 10+ seconds.");
                    m_dropped_frames = 0;
                    m_last_drop_report = now;
                }
            }
            m_pending_frames.emplace_back(std::move(frame));
        }
        m_cv.notify_one();
        if (!drop_report.empty()){
            m_logger.log(drop_report, COLOR_RED);
        }
    }


    //  The capture time of audio sample 'sample' (per-channel count) on the
    //  current sample clock.
    WallClock audio_sample_time(int64_t sample) const{
        return m_audio_anchor_time + std::chrono::duration_cast<WallClock::duration>(
            std::chrono::nanoseconds((sample - m_audio_anchor_sample) * 1000000000 / AUDIO_OUTPUT_RATE)
        );
    }


    //  Interleaves the planar 48 kHz input into m_audio_acc, then for every
    //  complete 1024-sample frame: encodes AAC, stores the packet in the
    //  ring (stamped with the capture time of its samples), and evicts.
    void push_samples(size_t samples, size_t stride, const float* data, WallClock now){
        if (!m_have_audio || samples == 0 || data == nullptr){
            return;
        }

        //  Keep the sample clock in line with the wall clock. The last sample
        //  of this delivery is taken to have been captured at "now"; if the
        //  sample count says otherwise by more than the threshold (audio went
        //  missing, or the device clock drifted), re-anchor so that later
        //  packets land at their real time instead of shifting all subsequent
        //  audio against the video.
        //  The working stream carries no capture times, so "now" is the
        //  delivery time: any steady pipeline latency shows up as audio
        //  slightly behind the video, and the first delivery's latency sets
        //  the anchor.
        int64_t end_sample = m_audio_samples_total
            + (int64_t)(m_audio_acc.size() / m_audio_channels)
            + (int64_t)samples;
        if (m_audio_anchored){
            int64_t error_us = std::chrono::duration_cast<std::chrono::microseconds>(
                now - audio_sample_time(end_sample)
            ).count();
            if (error_us > AUDIO_RESYNC_THRESHOLD_US || error_us < -AUDIO_RESYNC_THRESHOLD_US){
                m_audio_anchored = false;
            }
        }
        if (!m_audio_anchored){
            m_audio_anchor_sample = end_sample;
            m_audio_anchor_time = now;
            m_audio_anchored = true;
        }

        //  The audio pipeline's per-listener node delivers the working
        //  stream already mixed + resampled to this tracker's fixed
        //  48 kHz / stereo attach shape, with split (planar) channels:
        //  channel c starts at data[c * stride] and runs for samples
        //  (stride is the per-channel buffer length, 0 for the
        //  single-channel pass-through). Interleave into the accumulator
        //  for the encode loop below.
        m_audio_acc.reserve(m_audio_acc.size() + samples * m_audio_channels);
        for (size_t i = 0; i < samples; i++){
            for (size_t ch = 0; ch < m_audio_channels; ch++){
                m_audio_acc.push_back(data[ch * stride + i]);
            }
        }

        //  Encode each complete AAC frame.
        while (m_audio_acc.size() >= (size_t)AUDIO_FRAME_SAMPLES * m_audio_channels){
            AVFrame* frame = av_frame_alloc();
            if (frame == nullptr){
                m_logger.log("Stream history: Failed to allocate audio frame. Disabling audio.", COLOR_RED);
                m_have_audio = false;
                return;
            }
            frame->format = AV_SAMPLE_FMT_FLTP;
            frame->sample_rate = AUDIO_OUTPUT_RATE;
            frame->nb_samples = AUDIO_FRAME_SAMPLES;
            int ret = av_channel_layout_copy(&frame->ch_layout, &m_audio_layout);
            if (ret < 0){
                av_frame_free(&frame);
                m_logger.log("Stream history: Failed to copy audio channel layout. Disabling audio.", COLOR_RED);
                m_have_audio = false;
                return;
            }
            if (av_frame_get_buffer(frame, 0) < 0){
                av_frame_free(&frame);
                m_logger.log("Stream history: Failed to allocate audio frame buffer. Disabling audio.", COLOR_RED);
                m_have_audio = false;
                return;
            }
            //  The encoder works in planar float (one plane per channel), but
            //  the working stream was accumulated packed/interleaved, so
            //  de-interleave into the frame's planes.
            const float* acc = m_audio_acc.data();
            for (size_t ch = 0; ch < m_audio_channels; ch++){
                float* plane = (float*)frame->data[ch];
                for (size_t i = 0; i < AUDIO_FRAME_SAMPLES; i++){
                    plane[i] = acc[i * m_audio_channels + ch];
                }
            }
            frame->pts = m_audio_samples_total;
            m_audio_samples_total += AUDIO_FRAME_SAMPLES;
            {
                size_t chunk = (size_t)AUDIO_FRAME_SAMPLES * m_audio_channels;
                std::vector<float>::iterator erase_end = m_audio_acc.begin();
                std::advance(erase_end, (std::vector<float>::difference_type)chunk);
                m_audio_acc.erase(m_audio_acc.begin(), erase_end);
            }

            int err = avcodec_send_frame(m_actx, frame);
            av_frame_free(&frame);
            if (err < 0){
                m_logger.log("Stream history: Audio encode failed: " + ffmpeg_error(err), COLOR_RED);
                m_have_audio = false;
                return;
            }
            while (true){
                AVPacket* packet = av_packet_alloc();
                if (packet == nullptr){
                    return;
                }
                err = avcodec_receive_packet(m_actx, packet);
                if (err == AVERROR(EAGAIN)){
                    av_packet_free(&packet);
                    break;
                }
                if (err < 0){
                    av_packet_free(&packet);
                    m_logger.log("Stream history: Audio packet failed: " + ffmpeg_error(err), COLOR_RED);
                    m_have_audio = false;
                    break;
                }
                auto ring_packet = std::make_shared<AudioRingPacket>();
                //  Stamp with the capture time of the samples this packet
                //  presents. The AAC encoder emits each packet a frame late,
                //  and its pts (in samples, the encoder's 1/48000 timebase)
                //  already accounts for that and for the encoder's priming.
                //  This keeps audio aligned with video in the ring (eviction)
                //  and in save() (which trims the audio to the video's start
                //  point and muxes both streams by these timestamps).
                ring_packet->timestamp = audio_sample_time(
                    packet->pts != AV_NOPTS_VALUE
                        ? packet->pts
                        : m_audio_samples_total - AUDIO_FRAME_SAMPLES
                );
                ring_packet->data.assign(packet->data, packet->data + packet->size);
                av_packet_free(&packet);

                int64_t added = (int64_t)ring_packet->data.size();
                bool capped;
                {
                    WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
                    //  Refresh the audio parameter snapshot on this (the
                    //  encoder's) thread; see the video counterpart in
                    //  drain_one_video_packet().
                    if (m_audio_params == nullptr
                        || m_audio_params->extradata_size != m_actx->extradata_size
                    ){
                        if (m_audio_params == nullptr){
                            m_audio_params = avcodec_parameters_alloc();
                        }
                        if (m_audio_params != nullptr){
                            avcodec_parameters_from_context(m_audio_params, m_actx);
                        }
                    }
                    m_audio.push_back(std::move(ring_packet));
                    m_ring_bytes += added;
                    capped = evict();
                }
                if (capped){
                    report_size_cap();
                }
            }
        }
    }


    bool mux_to_file(
        const std::string& filename,
        const std::deque<std::shared_ptr<const VideoRingPacket>>& p_video,
        size_t p_video_start,
        const std::deque<std::shared_ptr<const AudioRingPacket>>& p_audio,
        size_t p_audio_start,
        WallClock start_ts,
        //  Copies of the codec parameter snapshots, taken by save() under the
        //  same lock as the packets so that they describe the same encoder
        //  (owned by the caller; the video extradata may be rewritten here).
        AVCodecParameters* vpar,
        AVRational vtb,
        const AVCodecParameters* apar
    ) const{
        //  Parameter names use the p_ prefix: the ring deques and start
        //  indices have the same names as Data members (C4458 under /W4).
        //  No live m_vctx/m_actx checks here: the codec state for muxing comes
        //  from the m_video_params/m_audio_params snapshots (captured on the
        //  encoder threads; ffmpeg is not thread-safe on a live context).
        bool have_video = (p_video_start < p_video.size());
        bool have_audio_stream = (p_audio_start < p_audio.size());

        //  MP4 unless the requested filename asks for Matroska.
        std::string extension;
        size_t dot = filename.find_last_of('.');
        if (dot != std::string::npos && dot + 1 < filename.size()){
            extension = filename.substr(dot + 1);
            for (char& c : extension){
                c = (char)std::tolower((unsigned char)c);
            }
        }
        const char* muxer = (extension == "mkv") ? "matroska" : "mp4";

        //  For MP4, put the moov atom (the file index) at the front so the
        //  file can start playing and seeking immediately instead of waiting
        //  for the reader to scan the whole file. The muxer does one fast
        //  rewrite pass at av_write_trailer() time; fine for local files.
        AVDictionary* muxer_options = nullptr;
        if (extension != "mkv"){
            av_dict_set(&muxer_options, "movflags", "+faststart", 0);
        }

        AVFormatContext* format = nullptr;
        if (avformat_alloc_output_context2(&format, nullptr, muxer, filename.c_str()) < 0
            || format == nullptr
        ){
            m_logger.log("Stream history: Failed to create output context for '" + filename + "'.", COLOR_RED);
            return false;
        }

        AVStream* video_stream = nullptr;
        AVStream* audio_stream = nullptr;
        bool video_annexb = false;
        {
            if (have_video){
                //  Only H.264 needs extradata (the avcC record); the other
                //  candidates (MPEG-4 Part 2) carry their stream
                //  configuration in-band.
                const bool needs_extradata = (vpar != nullptr
                    && vpar->codec_id == AV_CODEC_ID_H264);
                if (vpar == nullptr
                    || (needs_extradata && vpar->extradata == nullptr)
                ){
                    m_logger.log("Stream history: Failed to set up video stream (missing encoder state?).", COLOR_RED);
                    av_dict_free(&muxer_options);
                    avformat_free_context(format);
                    return false;
                }
                char hex[9] = {0};
                const char* digits = "0123456789abcdef";
                for (int b = 0; b < 4 && b < vpar->extradata_size; b++){
                    uint8_t byte = vpar->extradata[b];
                    hex[2 * b] = digits[byte >> 4];
                    hex[2 * b + 1] = digits[byte & 0xF];
                }
                m_logger.log(
                    "Stream history: muxing video (extradata "
                    + std::to_string(vpar->extradata_size) + " bytes, "
                    + std::string(hex) + ").",
                    COLOR_CYAN
                );
                //  Log the first bytes of the first ring packet too: the
                //  packet form must match the extradata form (Annex B for the
                //  QSV, NVENC, AMF and Media Foundation wrappers, AVCC for
                //  VideoToolbox, in-band for mpeg4). If an encoder ever mixed
                //  the forms, the MP4 save would be a silent blank file; the
                //  two logged prefixes make such a mismatch visible.
                for (size_t i = p_video_start; i < p_video.size(); i++){
                    const std::vector<uint8_t>& d = p_video[i]->data;
                    if (d.empty()){
                        continue;
                    }
                    char phex[9] = {0};
                    for (size_t b = 0; b < 4 && b < d.size(); b++){
                        uint8_t byte = d[b];
                        phex[2 * b] = digits[byte >> 4];
                        phex[2 * b + 1] = digits[byte & 0xF];
                    }
                    m_logger.log(
                        "Stream history: first video packet starts with "
                        + std::string(phex) + ".",
                        COLOR_CYAN
                    );
                    break;
                }
                //  MP4 stores H.264 as AVCC (avcC header + NAL units with
                //  4-byte length prefixes). The QSV, NVENC, AMF and Media
                //  Foundation wrappers emit Annex B instead, and the MP4 muxer
                //  of the vendored ffmpeg crashes on that form, so convert the
                //  extradata here and the ring packets below.
                //  Annex B starts with a 4-byte (00 00 00 01) or 3-byte
                //  (00 00 01) start code; AVCC starts with 01 (the avcC
                //  configurationVersion). Only H.264 is converted: MPEG-4
                //  Part 2 extradata also starts with a 00 00 01 start code.
                const uint8_t* extradata = vpar->extradata;
                int extradata_size = vpar->extradata_size;
                video_annexb = std::string(muxer) == "mp4"
                    && vpar->codec_id == AV_CODEC_ID_H264 && (
                    (extradata_size >= 4
                        && extradata[0] == 0 && extradata[1] == 0
                        && extradata[2] == 0 && extradata[3] == 1)
                    || (extradata_size >= 3
                        && extradata[0] == 0 && extradata[1] == 0
                        && extradata[2] == 1)
                );
                if (video_annexb){
                    std::vector<uint8_t> avcc;
                    if (!h264_avcc_from_annexb_extradata(
                        vpar->extradata, (size_t)vpar->extradata_size, avcc
                    )
                    ){
                        m_logger.log("Stream history: Failed to convert the H.264 extradata from Annex B to AVCC.", COLOR_RED);
                        av_dict_free(&muxer_options);
                        avformat_free_context(format);
                        return false;
                    }
                    uint8_t* new_extradata = (uint8_t*)av_mallocz(avcc.size() + AV_INPUT_BUFFER_PADDING_SIZE);
                    if (new_extradata == nullptr){
                        m_logger.log("Stream history: Failed to allocate the converted H.264 extradata.", COLOR_RED);
                        av_dict_free(&muxer_options);
                        avformat_free_context(format);
                        return false;
                    }
                    memcpy(new_extradata, avcc.data(), avcc.size());
                    av_free(vpar->extradata);
                    vpar->extradata = new_extradata;
                    vpar->extradata_size = (int)avcc.size();
                    m_logger.log("Stream history: converted H.264 Annex-B extradata to AVCC for MP4.", COLOR_CYAN);
                }
            }
            if (have_audio_stream){
                if (apar == nullptr || apar->extradata == nullptr){
                    m_logger.log("Stream history: Failed to set up audio stream (missing encoder state?).", COLOR_RED);
                    av_dict_free(&muxer_options);
                    avformat_free_context(format);
                    return false;
                }
            }
        }
        if (have_video){
            video_stream = avformat_new_stream(format, nullptr);
            if (video_stream == nullptr
                || avcodec_parameters_copy(video_stream->codecpar, vpar) < 0
            ){
                m_logger.log("Stream history: Failed to set up video stream (missing encoder state?).", COLOR_RED);
                av_dict_free(&muxer_options);
                avformat_free_context(format);
                return false;
            }
            //  Matroska's write_header() rewrites every stream timebase to 1/1000
            //  (its native TimestampScale is milliseconds) but leaves the packet
            //  timestamps submitted by the caller untouched, so .mkv packets must
            //  be stamped in milliseconds. The MP4 muxer rescales from the stream
            //  timebase itself, so the encoder's own timebase is kept there.
            video_stream->time_base = (std::string(muxer) == "matroska")
                ? av_make_q(1, 1000)
                : vtb;
        }
        if (have_audio_stream){
            audio_stream = avformat_new_stream(format, nullptr);
            if (audio_stream == nullptr
                || avcodec_parameters_copy(audio_stream->codecpar, apar) < 0
            ){
                m_logger.log("Stream history: Failed to set up audio stream (missing encoder state?).", COLOR_RED);
                av_dict_free(&muxer_options);
                avformat_free_context(format);
                return false;
            }
            //  Same as above: Matroska needs millisecond timestamps, while MP4
            //  handles the native 1/48000 (sample count) timebase just fine.
            audio_stream->time_base = (std::string(muxer) == "matroska")
                ? av_make_q(1, 1000)
                : av_make_q(1, AUDIO_OUTPUT_RATE);
        }

        //  The lavf encoding docs require the caller to provide an opened IO
        //  context (format->pb) before avformat_write_header() for muxers
        //  that aren't AVFMT_NOFILE: avformat_write_header() does not open
        //  format->url itself, and the muxer's write_header() dereferences
        //  s->pb (crashing on the NULL pointer). Opened here, just before the
        //  header, so the setup-failure paths above never leak a file handle;
        //  the success path closes it below. avio_open2() leaves the
        //  non-AVIO options (like movflags) in muxer_options for
        //  avformat_write_header().
        if (format->pb == nullptr && (format->flags & AVFMT_NOFILE) == 0){
            if (avio_open2(
                &format->pb,
                filename.c_str(),
                AVIO_FLAG_WRITE,
                &format->interrupt_callback,
                &muxer_options
            ) < 0 || format->pb == nullptr
            ){
                m_logger.log("Stream history: Failed to open '" + filename + "' for writing.", COLOR_RED);
                av_dict_free(&muxer_options);
                avformat_free_context(format);
                return false;
            }
        }
        if (avformat_write_header(format, &muxer_options) < 0){
            m_logger.log("Stream history: Failed to write file header for '" + filename + "'.", COLOR_RED);
            av_dict_free(&muxer_options);
            avio_closep(&format->pb);
            avformat_free_context(format);
            return false;
        }
        av_dict_free(&muxer_options);

        bool ok = true;
        AVPacket* packet = av_packet_alloc();
        if (packet == nullptr){
            ok = false;
        }else{
            //  When the ring holds Annex B H.264, each packet is rewritten
            //  with 4-byte NAL length prefixes; the converted buffer must
            //  outlive the write call.
            std::vector<uint8_t> converted_data;
            //  The muxers require strictly increasing dts per stream. The ring
            //  timestamps normally increase (video: from the encoder's
            //  millisecond pts; audio: from the sample clock), but rounding to
            //  the stream timebase can still make two equal. Bump a
            //  non-increasing dts to last+1 (one tick of the stream timebase:
            //  1 ms for MKV and MP4 video, 1/48000 s for MP4 audio) instead of
            //  failing the whole save. (Audio overlapping after a re-sync is
            //  dropped instead; see below.)
            int64_t last_video_dts = -1;
            int64_t last_audio_dts = -1;
            //  Where the last written audio packet ends (microseconds).
            int64_t audio_end_us = INT64_MIN;
            size_t dropped_audio = 0;
            size_t vi = p_video_start;
            size_t ai = p_audio_start;
            //  Submit the packets in timestamp order, alternating between the two
            //  streams. Writing one whole stream before the other would lay the
            //  Matroska clusters out of time order (all of the audio after all of
            //  the video), which breaks sequential players.
            for (;;){
                while (have_video && vi < p_video.size() && p_video[vi]->data.empty()){
                    vi++;    //  Nothing to write.
                }
                while (have_audio_stream && ai < p_audio.size() && p_audio[ai]->data.empty()){
                    ai++;
                }
                bool have_next_video = (have_video && vi < p_video.size());
                bool have_next_audio = (have_audio_stream && ai < p_audio.size());
                if (!have_next_video && !have_next_audio){
                    break;
                }
                //  Both timestamps in microseconds (the common comparison unit).
                int64_t video_us = INT64_MAX;
                int64_t audio_us = INT64_MAX;
                if (have_next_video){
                    video_us = std::chrono::duration_cast<std::chrono::microseconds>(
                        p_video[vi]->timestamp - start_ts
                    ).count();
                }
                if (have_next_audio){
                    audio_us = std::chrono::duration_cast<std::chrono::microseconds>(
                        p_audio[ai]->timestamp - start_ts
                    ).count();
                }
                if (have_next_video && video_us <= audio_us){
                    const VideoRingPacket& vp = *p_video[vi];
                    if (video_annexb){
                        if (!h264_annexb_to_avcc(vp.data.data(), vp.data.size(), converted_data)){
                            m_logger.log("Stream history: Failed to convert a video packet from Annex B to AVCC.", COLOR_RED);
                            ok = false;
                            break;
                        }
                        packet->data = converted_data.data();
                        packet->size = (int)converted_data.size();
                    }else{
                        //  The muxer only reads the packet data.
                        packet->data = const_cast<uint8_t*>(vp.data.data());
                        packet->size = (int)vp.data.size();
                    }
                    packet->stream_index = video_stream->index;
                    //  `video_us` counts microseconds (a 1/1000000 timebase);
                    //  convert to the stream timebase, keeping the dts strictly
                    //  increasing (see the note above the loop).
                    int64_t dts = av_rescale_q(video_us, av_make_q(1, 1000000), video_stream->time_base);
                    if (dts <= last_video_dts){
                        dts = last_video_dts + 1;
                    }
                    last_video_dts = dts;
                    packet->pts = dts;
                    packet->dts = dts;
                    packet->flags = vp.flags;
                    if (av_interleaved_write_frame(format, packet) < 0){
                        m_logger.log("Stream history: Failed writing video data.", COLOR_RED);
                        ok = false;
                        break;
                    }
                    vi++;
                }else{
                    //  A backwards re-sync of the audio clock (the device
                    //  clock running fast; see push_samples()) stamps the
                    //  following audio up to the re-sync threshold earlier,
                    //  overlapping audio already written. Drop the overlap:
                    //  bumping it to strictly increasing timestamps would
                    //  squeeze it into a burst of near-zero-length packets.
                    //  (1 ms of slack for the timestamp rounding.)
                    if (audio_end_us != INT64_MIN && audio_us < audio_end_us - 1000){
                        dropped_audio++;
                        ai++;
                        continue;
                    }
                    audio_end_us = audio_us + AUDIO_FRAME_US;
                    const AudioRingPacket& ap = *p_audio[ai];
                    //  The muxer only reads the packet data.
                    packet->data = const_cast<uint8_t*>(ap.data.data());
                    packet->size = (int)ap.data.size();
                    packet->stream_index = audio_stream->index;
                    //  The packet's capture time relative to the file start,
                    //  on the same clock as the video, so that a gap in the
                    //  audio does not shift the later audio against the
                    //  video. Rescale to the stream timebase (1/48000 for
                    //  MP4, 1/1000 for MKV), keeping the dts strictly
                    //  increasing.
                    int64_t dts = av_rescale_q(audio_us, av_make_q(1, 1000000), audio_stream->time_base);
                    if (dts <= last_audio_dts){
                        dts = last_audio_dts + 1;
                    }
                    last_audio_dts = dts;
                    packet->pts = dts;
                    packet->dts = dts;
                    packet->flags = 0;
                    if (av_interleaved_write_frame(format, packet) < 0){
                        m_logger.log("Stream history: Failed writing audio data.", COLOR_RED);
                        ok = false;
                        break;
                    }
                    ai++;
                }
            }
            av_packet_free(&packet);
            if (dropped_audio != 0){
                m_logger.log(
                    "Stream history: dropped " + std::to_string(dropped_audio)
                    + " audio packets that overlapped after an audio clock re-sync.",
                    COLOR_YELLOW
                );
            }
        }

        if (ok && av_write_trailer(format) < 0){
            m_logger.log("Stream history: Failed writing file trailer.", COLOR_RED);
            ok = false;
        }
        if (format->pb != nullptr){
            avio_closep(&format->pb);
        }
        avformat_free_context(format);
        return ok;
    }


    //  Snapshots the ring, rewinds the video to its first keyframe, trims
    //  the audio to the same start point, and muxes both to the file.
    bool save(const std::string& filename) const{
        m_logger.log("Saving stream history (in-memory encoded ring)...", COLOR_BLUE);

        //  Fast snapshot: the deques only hold shared_ptr's (the packet bytes are
        //  shared, not copied). Local names avoid C4458 (/W4) member shadowing.
        std::deque<std::shared_ptr<const VideoRingPacket>> video_snapshot;
        std::deque<std::shared_ptr<const AudioRingPacket>> audio_snapshot;
        //  The codec parameters are copied in the same critical section as
        //  the packets: an encoder fallback replaces both together (see
        //  init_video_encoder()), and muxing one encoder's packets with
        //  another's parameters produces a broken file.
        CodecParametersPtr video_params;
        CodecParametersPtr audio_params;
        AVRational video_time_base;
        {
            WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
            video_snapshot = this->m_video;
            audio_snapshot = this->m_audio;
            video_params = copy_codec_parameters(m_video_params);
            audio_params = copy_codec_parameters(m_audio_params);
            video_time_base = m_video_time_base;
        }
        if (video_snapshot.empty() && audio_snapshot.empty()){
            m_logger.log("Cannot save stream history: Nothing was recorded.", COLOR_RED);
            return false;
        }

        //  Rewind the video to the first keyframe (the oldest decodable start
        //  point) so the file decodes from the beginning. Because evict() keeps
        //  one extra keyframe interval of headroom, the file covers the most
        //  recent "window" seconds (give or take a frame, unless the memory
        //  cap evicted more), and may start up to one keyframe interval
        //  (~2 s) earlier than that.
        size_t video_start_index = 0;
        for (size_t i = 0; i < video_snapshot.size(); i++){
            if (video_snapshot[i]->flags & AV_PKT_FLAG_KEY){
                video_start_index = i;
                break;
            }
        }

        WallClock start_ts;
        if (!video_snapshot.empty()){
            start_ts = video_snapshot[video_start_index]->timestamp;
        }else{
            start_ts = audio_snapshot.front()->timestamp;
        }

        //  Trim the audio to the same start point.
        size_t audio_start = 0;
        if (!video_snapshot.empty() && !audio_snapshot.empty()){
            while (audio_start < audio_snapshot.size() && audio_snapshot[audio_start]->timestamp < start_ts){
                audio_start++;
            }
            if (audio_start >= audio_snapshot.size()){
                audio_snapshot.clear();
            }
        }

        bool ok = mux_to_file(
            filename,
            video_snapshot, video_start_index,
            audio_snapshot, audio_start,
            start_ts,
            video_params.get(), video_time_base, audio_params.get()
        );
        m_logger.log(
            ok ? "Stream history saved to '" + filename + "'."
               : "Failed to save stream history to '" + filename + "'.",
            ok ? COLOR_GREEN : COLOR_RED
        );
        return ok;
    }
};



//  Stops the video worker thread before Data (and its ffmpeg contexts) is
//  destroyed.
StreamHistoryTracker_Ring::~StreamHistoryTracker_Ring(){
    if (m_data->m_worker){
        {
            //  Set under the queue lock: otherwise the worker can check the
            //  predicate, miss this notification, and then block forever.
            std::lock_guard<Mutex> queue_guard(m_data->m_queue_lock);
            m_data->m_stopping = true;
        }
        m_data->m_cv.notify_all();
        m_data->m_worker.wait_and_ignore_exceptions();
    }
}

StreamHistoryTracker_Ring::StreamHistoryTracker_Ring(
    Logger& logger,
    std::chrono::seconds window,
    bool has_audio,
    bool has_video
)
    : m_data(
        CONSTRUCT_TOKEN,
        logger,
        window,
        has_audio,
        has_video
    )
{}

void StreamHistoryTracker_Ring::set_window(std::chrono::seconds window){
    WriteSpinLock lg(m_data->m_lock, PA_CURRENT_FUNCTION);
    m_data->m_window = window;
    (void)m_data->evict();
}

void StreamHistoryTracker_Ring::push_samples(size_t samples, size_t stride, const float* data){
    m_data->push_samples(samples, stride, data, current_time());
}
void StreamHistoryTracker_Ring::push_samples(size_t samples, size_t stride, const float* data, WallClock now){
    m_data->push_samples(samples, stride, data, now);
}
void StreamHistoryTracker_Ring::wait_until_idle(){
    m_data->wait_until_idle();
}

void StreamHistoryTracker_Ring::on_frame(std::shared_ptr<const VideoFrame> frame){
    m_data->on_frame(std::move(frame));
}

bool StreamHistoryTracker_Ring::save(const std::string& filename) const{
    return m_data->save(filename);
}



}
