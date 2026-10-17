/*  Stream History Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Implement by keeping an in-memory ring of *encoded* video packets
 *  (hardware H.264, with ffmpeg's built-in LGPL MPEG-4 as a fallback)
 *  and AAC audio packets. Nothing is written to storage until save() is
 *  called, at which point the ring is muxed into a single MP4/MKV file.
 *
 *  Video is encoded on a dedicated worker thread (frames that arrive faster
 *  than the encoder can keep up are dropped, like the JPEG-based tracker).
 *  Audio is resampled to 48 kHz and encoded inline in on_samples() (AAC
 *  encoding of 1024 samples takes well under a millisecond).
 *
 *  Only compiled when PA_STREAM_HISTORY_FFMPEG is defined (CMake option
 *  -DPA_STREAM_HISTORY_FFMPEG=ON).
 *
 */

#include <cerrno>
#include <cctype>
#include <cstring>
#include <deque>
#include <mutex>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/error.h>
#include <libavutil/version.h>
#include <libavutil/log.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libavutil/rational.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

//  MSVC's <cerrno> does not define EAGAIN; ffmpeg's send/receive API reports
//  "no data" as AVERROR(EAGAIN), which needs it. POSIX defines it as 11.
#ifndef EAGAIN
#define EAGAIN 11
#endif

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/Recording/StreamHistoryOption.h"
#include "CommonFramework/VideoPipeline/Backends/VideoFrameQt.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
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
    int64_t first_sample;    //  Cumulative samples (per channel) at the start of this frame.
    std::vector<uint8_t> data;
};


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

//  Creates the 48 kHz resampler. swr_alloc_set_opts2() takes a SwrContext**
//  and returns 0 on success.
static bool make_audio_resampler(SwrContext*& swr,
                                 const AVChannelLayout& out_layout, int out_rate,
                                 const AVChannelLayout& in_layout, int in_rate){
    int ret = swr_alloc_set_opts2(
        &swr,
        &out_layout, AV_SAMPLE_FMT_FLT, out_rate,
        &in_layout,  AV_SAMPLE_FMT_FLT, in_rate,
        0, nullptr
    );
    return ret == 0 && swr != nullptr;
}


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


size_t get_target_bitrate_kbps(){
    const StreamHistoryOption& settings = GlobalSettings::instance().STREAM_HISTORY;
    if (settings.ENCODING_MODE == StreamHistoryOption::EncodingMode::FIXED_BITRATE){
        //  Clamp so that a pathological setting cannot blow past the ring cap.
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


int qt_pixel_format_to_av_pix_fmt(QVideoFrameFormat::PixelFormat format){
    switch (format){
    case QVideoFrameFormat::Format_NV12:     return AV_PIX_FMT_NV12;
    case QVideoFrameFormat::Format_YUV420P:  return AV_PIX_FMT_YUV420P;
    case QVideoFrameFormat::Format_YUYV:     return AV_PIX_FMT_YUYV422;
    case QVideoFrameFormat::Format_P010:     return AV_PIX_FMT_P010;
    case QVideoFrameFormat::Format_XRGB8888:    return AV_PIX_FMT_BGRA;
    case QVideoFrameFormat::Format_ARGB8888:   return AV_PIX_FMT_BGRA;
    default:                                 return AV_PIX_FMT_NONE;
    }
}


//  Ordered by preference: hardware encoders first, then ffmpeg's built-in
//  (LGPL) MPEG-4 Part 2 encoder as a last resort. Every candidate in this
//  list ships in an LGPL ffmpeg build; GPL components (libx264) are
//  deliberately not candidates.
const char* const VIDEO_ENCODER_CANDIDATES[] = {
#ifdef _WIN32
    "h264_qsv",           "h264_nvenc",           "h264_amf",
#elif defined(__APPLE__)
    "h264_videotoolbox",
#else
    "h264_v4l2m2m",
#endif
    "mpeg4",
};

//  A hardware encoder whose avcodec_send_frame() hard-failed at runtime has
//  broken its GPU driver: calling into the context again - even to release
//  it - can crash the driver (observed: 0xC0000005 access violation in
//  atidxx64.dll during avcodec_free_context() on a Ryzen 5 5625U iGPU).
//  Such encoders are "poisoned" for the rest of the process: the failed
//  context is deliberately leaked (a bounded, one-time leak) and the encoder
//  is never initialized again.
std::mutex poisoned_encoder_mutex;
std::string poisoned_encoder;

//  Abandons 'ctx' after a runtime encoder failure (the context has already
//  pushed frames into a possibly-broken GPU driver). Hardware encoder
//  contexts are leaked, not freed, because releasing a context whose driver
//  is already broken can crash the process inside the driver (observed:
//  0xC0000005 access violation in atidxx64.dll on a Ryzen 5 5625U iGPU); the
//  CPU fallback (mpeg4) is released normally. The encoder is remembered as
//  poisoned so that later sessions do not re-initialize it (which would
//  re-break the driver and leak another device). Encoder contexts that
//  merely fail to open are released normally - the driver was never
//  exercised through them.
void abandon_video_encoder(Logger& logger, const char* name, AVCodecContext*& ctx){
    if (ctx == nullptr){
        return;
    }
    if (std::strcmp(name, "mpeg4") == 0){
        avcodec_free_context(&ctx);
    }else{
        {
            std::lock_guard<std::mutex> lg(poisoned_encoder_mutex);
            poisoned_encoder = name;
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

//  MP4 (ISO BMFF) stores H.264 as AVCC: an avcC configuration record in the
//  stream header and NAL units with 4-byte big-endian length prefixes in the
//  samples. The h264_amf wrapper (with AV_CODEC_FLAG_GLOBAL_HEADER) instead
//  produces Annex B (00 00 00 01 start codes) for both, and the MP4 muxer
//  shipped with the vendored ffmpeg crashes on that form; the other
//  hardware encoders (nvenc, qsv) produce AVCC natively. These two
//  conversions turn the AMF output into the standard AVCC form so that the
//  muxer always sees what it expects.

//  Walks the NAL units of an Annex B H.264 byte stream and re-emits them
//  with 4-byte big-endian length prefixes. Returns false if no NAL unit is
//  found.
bool h264_annexb_to_avcc(const uint8_t* in, size_t in_size, std::vector<uint8_t>& out){
    out.clear();
    const uint8_t* p = in;
    const uint8_t* end = in + in_size;
    bool found_any = false;
    while (p < end){
        const uint8_t* nal_start = nullptr;
        for (const uint8_t* q = p; q + 2 < end; q++){
            if (q[0] == 0 && q[1] == 0 && q[2] == 1){
                nal_start = q + 3;
                break;
            }
        }
        if (nal_start == nullptr){
            break;    //  No start code left; stop at any trailing garbage.
        }
        const uint8_t* nal_end = nal_start;
        for (const uint8_t* q = nal_start; q + 2 < end; q++){
            if (q[0] == 0 && q[1] == 0 && q[2] == 1){
                nal_end = q;
                break;
            }
        }
        if (nal_end == nal_start){
            nal_end = end;    //  The last NAL unit runs to the end of the buffer.
        }
        size_t nal_size = (size_t)(nal_end - nal_start);
        if (nal_size == 0){
            break;
        }
        found_any = true;
        out.push_back((uint8_t)(nal_size >> 24));
        out.push_back((uint8_t)(nal_size >> 16));
        out.push_back((uint8_t)(nal_size >> 8));
        out.push_back((uint8_t)(nal_size));
        out.insert(out.end(), nal_start, nal_end);
        p = nal_end;
    }
    return found_any;
}

//  Builds the MP4 avcC configuration record from an Annex B H.264 extradata
//  (SPS/PPS NAL units with start codes). Returns false if no SPS is found.
bool h264_avcc_from_annexb_extradata(const uint8_t* in, size_t in_size, std::vector<uint8_t>& out){
    out.clear();
    std::vector<uint8_t> sps;
    std::vector<uint8_t> pps;
    const uint8_t* p = in;
    const uint8_t* end = in + in_size;
    while (p < end){
        const uint8_t* nal_start = nullptr;
        for (const uint8_t* q = p; q + 2 < end; q++){
            if (q[0] == 0 && q[1] == 0 && q[2] == 1){
                nal_start = q + 3;
                break;
            }
        }
        if (nal_start == nullptr){
            break;
        }
        const uint8_t* nal_end = nal_start;
        for (const uint8_t* q = nal_start; q + 2 < end; q++){
            if (q[0] == 0 && q[1] == 0 && q[2] == 1){
                nal_end = q;
                break;
            }
        }
        if (nal_end == nal_start){
            nal_end = end;
        }
        if (nal_start < nal_end){
            int type = nal_start[0] & 0x1F;
            if (type == 7 && sps.empty()){
                sps.assign(nal_start, nal_end);    //  SPS NAL unit.
            }else if (type == 8 && pps.empty()){
                pps.assign(nal_start, nal_end);    //  PPS NAL unit.
            }
        }
        p = nal_end;
    }
    if (sps.size() < 4){
        return false;
    }
    //  avcC: configurationVersion=1, profile_idc, profile_compatibility,
    //  level_idc, 6 reserved bits + lengthSizeMinusOne (3 => 4-byte NAL
    //  unit lengths), 3 reserved bits + numOfSequenceParameterSets (1),
    //  then the SPS with its 16-bit length, numOfPictureParameterSets (1),
    //  and the PPS with its 16-bit length.
    out.push_back(0x01);
    out.push_back(sps[1]);
    out.push_back(sps[2]);
    out.push_back(sps[3]);
    out.push_back(0xFF);
    out.push_back(0xE1);
    out.push_back((uint8_t)(sps.size() >> 8));
    out.push_back((uint8_t)(sps.size() & 0xFF));
    out.insert(out.end(), sps.begin(), sps.end());
    out.push_back(0x01);
    out.push_back((uint8_t)(pps.size() >> 8));
    out.push_back((uint8_t)(pps.size() & 0xFF));
    out.insert(out.end(), pps.begin(), pps.end());
    return true;
}


}  // namespace




struct StreamHistoryTracker_Ring::Impl{
    Logger& logger;
    mutable SpinLock lock;    //  Protects the rings and the encoder state.
    std::chrono::seconds window;
    bool has_video;

    //  Audio configuration, as declared by the session.
    size_t audio_channels;
    int audio_input_rate;
    bool have_audio;

    //  Video frame throttling (same scheme as StreamHistoryTracker_SaveFrames).
    size_t target_fps;
    std::chrono::microseconds frame_interval;
    WallClock next_frame_time = WallClock::min();

    //  The ring.
    std::deque<std::shared_ptr<const VideoRingPacket>> video;
    std::deque<std::shared_ptr<const AudioRingPacket>> audio;
    int64_t ring_bytes = 0;

    //  Video encoder (initialized on the worker thread at the first frame).
    bool video_enabled;
    bool logged_unsupported_format = false;
    AVCodecContext* vctx = nullptr;
    SwsContext* sws = nullptr;
    int sws_in_format = AV_PIX_FMT_NONE;
    int sws_width = 0;
    int sws_height = 0;
    int sws_out_format = AV_PIX_FMT_NONE;
    int video_width = 0;
    int video_height = 0;
    int64_t last_keyframe_us = -VIDEO_KEYFRAME_INTERVAL_US;
    int64_t last_pts_us = -1;
    WallClock video_start = WallClock::min();

    //  Encoder candidate rotation. A hardware encoder can open successfully
    //  and still fail on every frame (e.g. its GPU device cannot allocate
    //  textures); after a hard failure from avcodec_send_frame() the current
    //  candidate is abandoned and the next one is tried, so the recording
    //  still gets video. Candidates that have failed (at open or at runtime)
    //  are not retried. Only the worker thread touches these.
    size_t video_encoder_index = 0;

    //  Codec parameter snapshots for muxing, owned by Impl, guarded by
    //  `lock`. ffmpeg is not thread-safe on a live AVCodecContext, and
    //  mux_to_file() runs on the caller's thread while the video worker and
    //  the audio pipeline thread keep encoding into vctx/actx, so the muxer
    //  must never read those contexts directly (a torn read of, e.g., the
    //  lazily-populated hardware-encoder extradata crashes the mp4 muxer).
    //  Each snapshot is refreshed on its encoder's own thread whenever the
    //  encoder state changes.
    AVCodecParameters* video_params = nullptr;
    AVRational video_time_base = {1, 1000};
    AVCodecParameters* audio_params = nullptr;

    //  Audio encoder + resampler (used by on_samples).
    AVCodecContext* actx = nullptr;
    SwrContext* swr = nullptr;
    AVChannelLayout audio_layout = {};
    int64_t audio_samples_total = 0;
    std::vector<float> audio_acc;    //  Resampled interleaved float @ 48 kHz.

    //  Video worker thread.
    AsyncTask worker;
    std::atomic<bool> stopping{false};
    Mutex queue_lock;
    ConditionVariable cv;
    std::deque<std::shared_ptr<const VideoFrame>> pending_frames;

    Impl(
        Logger& p_logger,
        std::chrono::seconds p_window,
        size_t p_audio_samples_per_frame,
        size_t p_audio_frames_per_second,
        bool p_has_video
    )
        : logger(p_logger)
        , window(p_window)
        , has_video(p_has_video)
        , audio_channels(p_audio_samples_per_frame)
        , audio_input_rate((int)p_audio_frames_per_second)
        , have_audio(p_audio_samples_per_frame > 0 && p_audio_frames_per_second > 0)
        , target_fps(get_target_fps())
        , frame_interval(std::chrono::microseconds(1000000 / target_fps))
        , video_enabled(p_has_video)
    {
        av_log_set_level(AV_LOG_ERROR);

        if (have_audio){
            init_audio_encoder();
        }
        if (has_video){
            worker = GlobalThreadPools::unlimited_normal().dispatch_now_blocking(
                [this]{ worker_loop(); }
            );
        }
    }

    ~Impl(){
        if (sws != nullptr) sws_freeContext(sws);
        if (swr != nullptr) swr_free(&swr);
        if (video_params != nullptr) avcodec_parameters_free(&video_params);
        if (audio_params != nullptr) avcodec_parameters_free(&audio_params);
        if (vctx != nullptr) avcodec_free_context(&vctx);
        if (actx != nullptr) avcodec_free_context(&actx);
    }


    void init_audio_encoder(){
        const AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
        if (codec == nullptr){
            logger.log("Stream history: AAC encoder not found in ffmpeg build. Disabling audio.", COLOR_RED);
            have_audio = false;
            return;
        }
        actx = avcodec_alloc_context3(codec);
        if (actx == nullptr){
            logger.log("Stream history: Failed to allocate AAC encoder. Disabling audio.", COLOR_RED);
            have_audio = false;
            return;
        }
        actx->sample_rate = AUDIO_OUTPUT_RATE;
        actx->time_base = av_make_q(1, AUDIO_OUTPUT_RATE);
        actx->bit_rate = (audio_channels >= 2 ? 96000 : 64000);
        actx->frame_size = AUDIO_FRAME_SAMPLES;
        //  The AAC encoder requires the sample format to be set explicitly;
        //  avcodec_open2() fails with EINVAL ("Invalid audio sample format")
        //  if it is left at AV_SAMPLE_FMT_NONE. The encoder works in planar
        //  float, so on_samples() de-interleaves the accumulated (packed)
        //  resampler output into the frame's planes.
        actx->sample_fmt = AV_SAMPLE_FMT_FLTP;
        //  The AAC stream configuration (AudioSpecificConfig) must land in the
        //  container header: with this flag the encoder emits raw AAC frames +
        //  extradata, which is what both the MP4 and MKV muxers need. Without
        //  it the packets would be ADTS and mux_to_file() would fail to set up
        //  the audio stream.
        actx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        if (!init_channel_layout(actx->ch_layout, (int)audio_channels)){
            logger.log(
                "Stream history: Failed to initialize the AAC channel layout ("
                + std::to_string((int)audio_channels) + " channels). Disabling audio.",
                COLOR_RED
            );
            avcodec_free_context(&actx);
            actx = nullptr;
            have_audio = false;
            return;
        }
        int open_err = avcodec_open2(actx, codec, nullptr);
        if (open_err < 0){
            logger.log(
                "Stream history: Failed to open AAC encoder: "
                + ffmpeg_error(open_err)
                + ". Disabling audio.",
                COLOR_RED
            );
            avcodec_free_context(&actx);
            actx = nullptr;
            have_audio = false;
            return;
        }
        //  The resampler and encoded frames use the same layout as the encoder
        //  (which just opened successfully with this channel count).
        init_channel_layout(audio_layout, (int)audio_channels);
        logger.log(
            "Stream history: audio = AAC " + std::to_string(audio_channels) + "ch @ 48 kHz ("
            + std::to_string(actx->bit_rate / 1000) + " kbps)."
        );
    }


    void init_video_encoder(int input_width, int input_height){
        constexpr size_t CANDIDATE_COUNT = sizeof(VIDEO_ENCODER_CANDIDATES)/sizeof(VIDEO_ENCODER_CANDIDATES[0]);
        //  Candidates before video_encoder_index already failed; once past
        //  the last one, video was disabled when it did.
        if (video_encoder_index >= CANDIDATE_COUNT){
            return;
        }

        int target_width = 0;
        int target_height = 0;
        if (!get_target_video_size(input_width, input_height, target_width, target_height)){
            logger.log("Stream history: Unsupported input resolution. Disabling video.", COLOR_RED);
            WriteSpinLock lg(lock, PA_CURRENT_FUNCTION);
            video_enabled = false;
            return;
        }

        size_t bitrate_kbps = get_target_bitrate_kbps();
        for (size_t i = video_encoder_index; i < CANDIDATE_COUNT; i++){
            const char* name = VIDEO_ENCODER_CANDIDATES[i];
            {
                std::lock_guard<std::mutex> lg(poisoned_encoder_mutex);
                if (poisoned_encoder == name){
                    logger.log(
                        "Stream history: skipping video encoder '"
                        + std::string(name) + "' (it failed at runtime earlier"
                        + " in this process).",
                        COLOR_YELLOW
                    );
                    video_encoder_index = i + 1;
                    continue;
                }
            }
            const AVCodec* codec = avcodec_find_encoder_by_name(name);
            if (codec == nullptr){
                video_encoder_index = i + 1;
                continue;
            }
            AVCodecContext* ctx = avcodec_alloc_context3(codec);
            if (ctx == nullptr){
                video_encoder_index = i + 1;
                continue;
            }
            ctx->width = target_width;
            ctx->height = target_height;
            //  Milliseconds as the video timebase: the mpeg4 encoder rejects
            //  a timebase denominator above 65535 (the 16-bit
            //  time_incr_length of the MPEG-4 Part 2 standard) with EINVAL,
            //  and the hardware H.264 wrappers accept any timebase; the
            //  muxer derives packet timestamps from the ring's microsecond
            //  WallClock timestamps anyway, so this choice only affects
            //  what the encoders see.
            ctx->time_base = av_make_q(1, 1000);
            //  The frame rate must be set explicitly: the h264_amf wrapper
            //  passes it to the AMD driver, which rejects - and tears down
            //  its D3D11 device for - a missing/zero frame rate ("unexpected
            //  frame rate" followed by AMF_NO_DEVICE).
            ctx->framerate = av_make_q((int)target_fps, 1);
            ctx->bit_rate = (int64_t)bitrate_kbps * 1000;
            ctx->gop_size = (int)(target_fps * 2);
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
#if LIBAVCODEC_VERSION_MAJOR >= 61
            //  ffmpeg 7.0+ (libavcodec 61+) removed AVCodec.pix_fmts; query the
            //  encoder instead. This split is a preprocessor version gate
            //  (rather than compile-time detection) because
            //  avcodec_get_supported_config() is not declared in every header
            //  version this project supports (5.1-7.0), and names in discarded
            //  if-constexpr branches are still looked up.
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
                vctx = ctx;
                video_encoder_index = i + 1;
                {
                    WriteSpinLock lg(lock, PA_CURRENT_FUNCTION);
                    //  New encoder: the old snapshot (if any) no longer
                    //  matches what the ring will hold going forward;
                    //  drain_one_video_packet() refreshes it on the first
                    //  encoded packet.
                    avcodec_parameters_free(&video_params);
                }
                video_width = target_width;
                video_height = target_height;
                video_start = WallClock::min();
                last_keyframe_us = -VIDEO_KEYFRAME_INTERVAL_US;
                last_pts_us = -1;
                logger.log(
                    "Stream history: video encoder = " + std::string(name)
                    + " ("
                    + std::string(av_get_pix_fmt_name(ctx->pix_fmt)) + ")"
                    + " at " + std::to_string(target_width) + "x" + std::to_string(target_height)
                    + ", " + std::to_string(bitrate_kbps) + " kbps."
                );
                return;
            }
            video_encoder_index = i + 1;
            logger.log(
                "Stream history: Video encoder '" + std::string(name) + "' failed to open: "
                + ffmpeg_error(err),
                COLOR_YELLOW
            );
            //  The encoder never opened, so no device was ever exercised;
            //  releasing the context is safe (unlike a context that failed at
            //  runtime - see abandon_video_encoder()).
            avcodec_free_context(&ctx);
        }
        logger.log("Stream history: No usable video encoder found. Disabling video.", COLOR_RED);
        WriteSpinLock lg(lock, PA_CURRENT_FUNCTION);
        video_enabled = false;
    }


    //  Receive and store a single encoded video packet.
    //  Returns 1 if a packet was stored, 0 if there is nothing to receive, -1 on
    //  error (already logged).
    int drain_one_video_packet(WallClock timestamp){
        AVPacket* packet = av_packet_alloc();
        if (packet == nullptr){
            logger.log("Stream history: Failed to allocate video packet.", COLOR_RED);
            return -1;
        }
        int err = avcodec_receive_packet(vctx, packet);
        if (err == AVERROR(EAGAIN) || err == AVERROR_EOF){
            av_packet_free(&packet);
            return 0;
        }
        if (err < 0){
            av_packet_free(&packet);
            logger.log("Stream history: Video packet failed: " + ffmpeg_error(err), COLOR_RED);
            return -1;
        }
        auto ring_packet = std::make_shared<VideoRingPacket>();
        ring_packet->timestamp = timestamp;
        ring_packet->flags = (packet->flags & AV_PKT_FLAG_KEY) ? AV_PKT_FLAG_KEY : 0;
        ring_packet->data.assign(packet->data, packet->data + packet->size);
        av_packet_free(&packet);

        int64_t added = (int64_t)ring_packet->data.size();
        WriteSpinLock lg(lock, PA_CURRENT_FUNCTION);
        //  Refresh the mux-time parameter snapshot on this (the encoder's)
        //  thread whenever the encoder state changes; the hardware encoders
        //  populate their extradata lazily, sometimes only after the first
        //  encoded packet. mux_to_file() must not read the live context.
        if (video_params == nullptr
            || video_params->extradata_size != vctx->extradata_size
        ){
            if (video_params == nullptr){
                video_params = avcodec_parameters_alloc();
            }
            if (video_params != nullptr){
                avcodec_parameters_from_context(video_params, vctx);
                video_time_base = vctx->time_base;
            }
        }
        video.push_back(std::move(ring_packet));
        ring_bytes += added;
        evict();
        return 1;
    }


    void worker_loop(){
        while (true){
            std::shared_ptr<const VideoFrame> frame;
            {
                std::unique_lock<Mutex> queue_guard(queue_lock);
                cv.wait(queue_guard, [this]{ return stopping.load() || !pending_frames.empty(); });
                if (stopping.load() && pending_frames.empty()){
                    return;
                }
                frame = std::move(pending_frames.front());
                pending_frames.pop_front();
            }
            encode_video_frame(frame);
        }
    }


    void encode_video_frame(const std::shared_ptr<const VideoFrame>& vf){
        if (!video_enabled || vf == nullptr || !vf->frame.isValid()){
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
        if (qt_pixel_format_to_av_pix_fmt(qframe.pixelFormat()) == AV_PIX_FMT_NV12){
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
                if (!logged_unsupported_format){
                    logger.log("Stream history: Could not convert video frame to RGB. Skipping.", COLOR_RED);
                    logged_unsupported_format = true;
                }
                return;
            }
            src_format = AV_PIX_FMT_RGB24;
            src_width = rgb_image.width();
            src_height = rgb_image.height();
            src_planes[0] = (const uint8_t*)rgb_image.bits();
            src_strides[0] = (int)rgb_image.bytesPerLine();
        }

        if (vctx == nullptr){
            init_video_encoder(qframe.width(), qframe.height());
            if (!video_enabled){
                unmap_if_needed();
                return;
            }
        }

        //  (Re)build the scaler if the input changed, or if the encoder's
        //  preferred pixel format changed (e.g. after an encoder fallback).
        if (src_format != sws_in_format || src_width != sws_width || src_height != sws_height
            || vctx->pix_fmt != sws_out_format){
            if (sws != nullptr){
                sws_freeContext(sws);
                sws = nullptr;
            }
            sws = sws_getContext(
                src_width, src_height, (AVPixelFormat)src_format,
                video_width, video_height, vctx->pix_fmt,
                SWS_BILINEAR, nullptr, nullptr, nullptr
            );
            if (sws == nullptr){
                logger.log("Stream history: Failed to create video scaler.", COLOR_RED);
                unmap_if_needed();
                return;
            }
            sws_in_format = src_format;
            sws_width = src_width;
            sws_height = src_height;
            sws_out_format = vctx->pix_fmt;
        }

        AVFrame* frame = av_frame_alloc();
        if (frame == nullptr){
            unmap_if_needed();
            return;
        }
        frame->format = vctx->pix_fmt;
        frame->width = video_width;
        frame->height = video_height;
        if (av_frame_get_buffer(frame, 0) < 0){
            av_frame_free(&frame);
            unmap_if_needed();
            return;
        }

        int scaled = sws_scale(
            sws,
            src_planes, src_strides,
            0, src_height,
            frame->data, frame->linesize
        );
        unmap_if_needed();
        if (scaled != video_height){
            av_frame_free(&frame);
            return;
        }

        //  Timestamps + keyframe control.
        if (video_start == WallClock::min()){
            video_start = vf->timestamp;
        }
        int64_t pts_us = std::chrono::duration_cast<std::chrono::microseconds>(
            vf->timestamp - video_start
        ).count();
        if (pts_us < last_pts_us){
            pts_us = last_pts_us;
        }
        last_pts_us = pts_us;
        bool force_keyframe = (pts_us - last_keyframe_us) >= VIDEO_KEYFRAME_INTERVAL_US;
        if (force_keyframe){
            last_keyframe_us = pts_us;
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
            int err = avcodec_send_frame(vctx, frame);
            if (err == AVERROR(EAGAIN)){
                if (drain_one_video_packet(vf->timestamp) <= 0){
                    av_frame_free(&frame);
                    return;
                }
                continue;
            }
            if (err < 0){
                av_frame_free(&frame);
                //  Fall back immediately. A hardware encoder that hard-fails
                //  at runtime has a broken GPU driver; every further call
                //  into it - including releasing the context - risks crashing
                //  the process inside the driver.
                const char* failed = VIDEO_ENCODER_CANDIDATES[video_encoder_index - 1];
                logger.log(
                    "Stream history: video encoder '" + std::string(failed)
                    + "' failed at runtime ("
                    + ffmpeg_error(err)
                    + "); falling back to the next candidate.",
                    COLOR_YELLOW
                );
                abandon_video_encoder(logger, failed, vctx);
                init_video_encoder(qframe.width(), qframe.height());
                return;
            }
            av_frame_free(&frame);
            break;
        }

        while (drain_one_video_packet(vf->timestamp) > 0){}
    }


    void evict(){
        //  Must call under lock.
        WallClock latest = WallClock::min();
        if (!video.empty()){
            latest = video.back()->timestamp;
        }
        if (!audio.empty() && audio.back()->timestamp > latest){
            latest = audio.back()->timestamp;
        }
        if (latest == WallClock::min()){
            return;
        }
        WallClock threshold = latest - window;
        while (!video.empty() && (video.front()->timestamp < threshold || ring_bytes > MAX_RING_BYTES)){
            ring_bytes -= (int64_t)video.front()->data.size();
            video.pop_front();
        }
        while (!audio.empty() && (audio.front()->timestamp < threshold || ring_bytes > MAX_RING_BYTES)){
            ring_bytes -= (int64_t)audio.front()->data.size();
            audio.pop_front();
        }
    }


    void on_frame(std::shared_ptr<const VideoFrame> frame){
        if (!has_video || frame == nullptr || frame->timestamp == WallClock::min()){
            return;
        }
        {
            WriteSpinLock lg(lock, PA_CURRENT_FUNCTION);
            if (!video_enabled){
                return;
            }

            //  Initialize on first frame.
            if (next_frame_time == WallClock::min()){
                next_frame_time = frame->timestamp;
            }

            //  Don't save every frame: only save frames as per target_fps.
            if (frame->timestamp < next_frame_time){
                return;
            }

            //  If the stream has been idle (e.g. a gap between frames), the while
            //  loop advances the schedule until it is once again ahead of the
            //  current timestamp. The resulting gap in the ring is fine because
            //  the muxer uses real timestamps.
            while (next_frame_time <= frame->timestamp){
                next_frame_time += frame_interval;
            }
        }

        {
            std::lock_guard<Mutex> queue_guard(queue_lock);

            //  Drop oldest if we are falling behind.
            if (pending_frames.size() >= MAX_PENDING_FRAMES){
                pending_frames.pop_front();
                logger.log("Stream history worker thread lagging: Frame dropped.", COLOR_RED);
            }
            pending_frames.emplace_back(std::move(frame));
        }
        cv.notify_one();
    }


    void on_samples(const float* data, size_t frames){
        if (!have_audio || frames == 0 || data == nullptr){
            return;
        }
        WallClock now = current_time();

        //  DUAL_44100 is declared to us as 1ch @ 44100 Hz, but the pipeline
        //  delivers interleaved stereo frames (2 samples/frame). Take the left
        //  channel so we actually resample a mono stream. (All the other formats
        //  have stride == channel count.)
        std::vector<float> mono_scratch;
        const float* in_data = data;
        if (audio_channels == 1 && audio_input_rate == 44100){
            mono_scratch.resize(frames);
            for (size_t i = 0; i < frames; i++){
                mono_scratch[i] = data[i * 2];
            }
            in_data = mono_scratch.data();
        }

        //  1. Resample to 48 kHz (same channel count) and accumulate.
        if (swr == nullptr){
            AVChannelLayout in_layout = {};
            if (!init_channel_layout(in_layout, (int)audio_channels)
                || !init_channel_layout(audio_layout, (int)audio_channels)
            ){
                logger.log("Stream history: Failed to set audio channel layout. Disabling audio.", COLOR_RED);
                have_audio = false;
                return;
            }
            bool swr_created = make_audio_resampler(swr, audio_layout, AUDIO_OUTPUT_RATE, in_layout, audio_input_rate);
            av_channel_layout_uninit(&in_layout);
            if (!swr_created){
                logger.log("Stream history: Failed to create audio resampler. Disabling audio.", COLOR_RED);
                have_audio = false;
                return;
            }
        }

        const uint8_t* in_planes[1] = { (const uint8_t*)in_data };
        int in_samples = (int)frames;
        int out_capacity = in_samples * 5 / 4 + AUDIO_FRAME_SAMPLES + 64;    //  per channel
        std::vector<uint8_t> out_buffer((size_t)out_capacity * audio_channels * sizeof(float));
        uint8_t* out_planes[1] = { out_buffer.data() };
        int out_samples = swr_convert(swr, out_planes, out_capacity, in_planes, in_samples);
        if (out_samples > 0){
            const float* out_floats = (const float*)out_planes[0];
            audio_acc.insert(
                audio_acc.end(),
                out_floats,
                out_floats + (size_t)out_samples * audio_channels
            );
        }

        //  2. Encode each complete AAC frame.
        while (audio_acc.size() >= (size_t)AUDIO_FRAME_SAMPLES * audio_channels){
            AVFrame* frame = av_frame_alloc();
            if (frame == nullptr){
                logger.log("Stream history: Failed to allocate audio frame. Disabling audio.", COLOR_RED);
                have_audio = false;
                return;
            }
            frame->format = AV_SAMPLE_FMT_FLTP;
            frame->sample_rate = AUDIO_OUTPUT_RATE;
            frame->nb_samples = AUDIO_FRAME_SAMPLES;
            int ret = av_channel_layout_copy(&frame->ch_layout, &audio_layout);
            if (ret < 0){
                av_frame_free(&frame);
                logger.log("Stream history: Failed to copy audio channel layout. Disabling audio.", COLOR_RED);
                have_audio = false;
                return;
            }
            if (av_frame_get_buffer(frame, 0) < 0){
                av_frame_free(&frame);
                logger.log("Stream history: Failed to allocate audio frame buffer. Disabling audio.", COLOR_RED);
                have_audio = false;
                return;
            }
            //  The encoder works in planar float (one plane per channel), but
            //  the resampler output was accumulated packed/interleaved, so
            //  de-interleave into the frame's planes.
            const float* acc = audio_acc.data();
            for (size_t ch = 0; ch < audio_channels; ch++){
                float* plane = (float*)frame->data[ch];
                for (size_t i = 0; i < AUDIO_FRAME_SAMPLES; i++){
                    plane[i] = acc[i * audio_channels + ch];
                }
            }
            frame->pts = audio_samples_total;
            audio_samples_total += AUDIO_FRAME_SAMPLES;
            {
                size_t chunk = (size_t)AUDIO_FRAME_SAMPLES * audio_channels;
                std::vector<float>::iterator erase_end = audio_acc.begin();
                std::advance(erase_end, (std::vector<float>::difference_type)chunk);
                audio_acc.erase(audio_acc.begin(), erase_end);
            }

            int err = avcodec_send_frame(actx, frame);
            av_frame_free(&frame);
            if (err < 0){
                logger.log("Stream history: Audio encode failed: " + ffmpeg_error(err), COLOR_RED);
                have_audio = false;
                return;
            }
            while (true){
                AVPacket* packet = av_packet_alloc();
                if (packet == nullptr){
                    return;
                }
                err = avcodec_receive_packet(actx, packet);
                if (err == AVERROR(EAGAIN)){
                    av_packet_free(&packet);
                    break;
                }
                if (err < 0){
                    av_packet_free(&packet);
                    logger.log("Stream history: Audio packet failed: " + ffmpeg_error(err), COLOR_RED);
                    have_audio = false;
                    break;
                }
                auto ring_packet = std::make_shared<AudioRingPacket>();
                ring_packet->timestamp = now - std::chrono::microseconds(AUDIO_FRAME_US);
                ring_packet->first_sample = audio_samples_total - AUDIO_FRAME_SAMPLES;
                ring_packet->data.assign(packet->data, packet->data + packet->size);
                av_packet_free(&packet);

                int64_t added = (int64_t)ring_packet->data.size();
                {
                    WriteSpinLock lg(lock, PA_CURRENT_FUNCTION);
                    //  Refresh the audio parameter snapshot on this (the
                    //  encoder's) thread; see the video counterpart in
                    //  drain_one_video_packet().
                    if (audio_params == nullptr
                        || audio_params->extradata_size != actx->extradata_size
                    ){
                        if (audio_params == nullptr){
                            audio_params = avcodec_parameters_alloc();
                        }
                        if (audio_params != nullptr){
                            avcodec_parameters_from_context(audio_params, actx);
                        }
                    }
                    audio.push_back(std::move(ring_packet));
                    ring_bytes += added;
                    evict();
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
        WallClock start_ts
    ){
        //  Parameter names use the p_ prefix: the ring deques and start
        //  indices have the same names as Impl members (C4458 under /W4).
        //  No live vctx/actx checks here: the codec state for muxing comes
        //  from the video_params/audio_params snapshots (captured on the
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
            logger.log("Stream history: Failed to create output context for '" + filename + "'.", COLOR_RED);
            return false;
        }

        AVStream* video_stream = nullptr;
        AVStream* audio_stream = nullptr;
        AVCodecParameters* vpar = nullptr;
        AVCodecParameters* apar = nullptr;
        AVRational vtb = {1, 1000};
        bool video_annexb = false;
        {
            ReadSpinLock lg(lock, PA_CURRENT_FUNCTION);
            if (have_video){
                //  Only H.264 needs extradata (the avcC record); the other
                //  candidates (MPEG-4 Part 2) carry their stream
                //  configuration in-band.
                const bool needs_extradata = (video_params != nullptr
                    && video_params->codec_id == AV_CODEC_ID_H264);
                if (video_params == nullptr
                    || (needs_extradata && video_params->extradata == nullptr)
                ){
                    logger.log("Stream history: Failed to set up video stream (missing encoder state?).", COLOR_RED);
                    avformat_free_context(format);
                    return false;
                }
                vpar = avcodec_parameters_alloc();
                if (vpar == nullptr
                    || avcodec_parameters_copy(vpar, video_params) < 0
                ){
                    logger.log("Stream history: Failed to copy the video codec parameters.", COLOR_RED);
                    avcodec_parameters_free(&vpar);
                    avformat_free_context(format);
                    return false;
                }
                if (needs_extradata && vpar->extradata == nullptr){
                    logger.log("Stream history: Failed to set up video stream (missing encoder state?).", COLOR_RED);
                    avcodec_parameters_free(&vpar);
                    avformat_free_context(format);
                    return false;
                }
                vtb = video_time_base;
                char hex[9] = {0};
                const char* digits = "0123456789abcdef";
                for (int b = 0; b < 4 && b < video_params->extradata_size; b++){
                    uint8_t byte = video_params->extradata[b];
                    hex[2 * b] = digits[byte >> 4];
                    hex[2 * b + 1] = digits[byte & 0xF];
                }
                logger.log(
                    "Stream history: muxing video (extradata "
                    + std::to_string(video_params->extradata_size) + " bytes, "
                    + std::string(hex) + ").",
                    COLOR_CYAN
                );
                //  MP4 stores H.264 as AVCC (avcC header + NAL units with
                //  4-byte length prefixes). The h264_amf wrapper emits
                //  Annex B instead, and the MP4 muxer of the vendored ffmpeg
                //  crashes on that form, so convert the extradata here and
                //  the ring packets below.
                video_annexb = (std::string(muxer) == "mp4"
                    && vpar->extradata_size >= 4
                    && vpar->extradata[0] == 0 && vpar->extradata[1] == 0
                    && vpar->extradata[2] == 0 && vpar->extradata[3] == 1);
                if (video_annexb){
                    std::vector<uint8_t> avcc;
                    if (!h264_avcc_from_annexb_extradata(
                        vpar->extradata, (size_t)vpar->extradata_size, avcc
                    )
                    ){
                        logger.log("Stream history: Failed to convert the H.264 extradata from Annex B to AVCC.", COLOR_RED);
                        avcodec_parameters_free(&vpar);
                        avformat_free_context(format);
                        return false;
                    }
                    uint8_t* new_extradata = (uint8_t*)av_mallocz(avcc.size() + AV_INPUT_BUFFER_PADDING_SIZE);
                    if (new_extradata == nullptr){
                        logger.log("Stream history: Failed to allocate the converted H.264 extradata.", COLOR_RED);
                        avcodec_parameters_free(&vpar);
                        avformat_free_context(format);
                        return false;
                    }
                    memcpy(new_extradata, avcc.data(), avcc.size());
                    av_free(vpar->extradata);
                    vpar->extradata = new_extradata;
                    vpar->extradata_size = (int)avcc.size();
                    logger.log("Stream history: converted H.264 Annex-B extradata to AVCC for MP4.", COLOR_CYAN);
                }
            }
            if (have_audio_stream){
                if (audio_params == nullptr || audio_params->extradata == nullptr){
                    logger.log("Stream history: Failed to set up audio stream (missing encoder state?).", COLOR_RED);
                    avcodec_parameters_free(&vpar);
                    avformat_free_context(format);
                    return false;
                }
                apar = avcodec_parameters_alloc();
                if (apar == nullptr
                    || avcodec_parameters_copy(apar, audio_params) < 0
                    || apar->extradata == nullptr
                ){
                    logger.log("Stream history: Failed to copy the audio codec parameters.", COLOR_RED);
                    avcodec_parameters_free(&vpar);
                    avcodec_parameters_free(&apar);
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
                logger.log("Stream history: Failed to set up video stream (missing encoder state?).", COLOR_RED);
                avcodec_parameters_free(&vpar);
                avcodec_parameters_free(&apar);
                avformat_free_context(format);
                return false;
            }
            video_stream->time_base = vtb;
        }
        if (have_audio_stream){
            audio_stream = avformat_new_stream(format, nullptr);
            if (audio_stream == nullptr
                || avcodec_parameters_copy(audio_stream->codecpar, apar) < 0
            ){
                logger.log("Stream history: Failed to set up audio stream (missing encoder state?).", COLOR_RED);
                avcodec_parameters_free(&vpar);
                avcodec_parameters_free(&apar);
                avformat_free_context(format);
                return false;
            }
            audio_stream->time_base = av_make_q(1, AUDIO_OUTPUT_RATE);
        }
        avcodec_parameters_free(&vpar);
        avcodec_parameters_free(&apar);

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
                logger.log("Stream history: Failed to open '" + filename + "' for writing.", COLOR_RED);
                av_dict_free(&muxer_options);
                avformat_free_context(format);
                return false;
            }
        }
        if (avformat_write_header(format, &muxer_options) < 0){
            logger.log("Stream history: Failed to write file header for '" + filename + "'.", COLOR_RED);
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
            if (have_video){
                //  When the ring holds Annex B H.264 (the h264_amf wrapper),
                //  each packet is rewritten with 4-byte NAL length prefixes;
                //  the converted buffer must outlive the write call.
                std::vector<uint8_t> converted_data;
                //  The MP4 muxer requires strictly increasing dts per stream, but the
                //  ring packets are stamped with the wall-clock time of the frame they
                //  were drained from, and a hardware encoder can emit several packets
                //  for one frame (or flush two or more at once), so consecutive
                //  packets can share a timestamp. Bump a non-increasing dts to
                //  last+1 (1 ms at the 1/1000 video timebase) instead of failing the
                //  whole save.
                int64_t last_video_dts = -1;
                for (size_t i = p_video_start; i < p_video.size() && ok; i++){
                    const VideoRingPacket& vp = *p_video[i];
                    if (vp.data.empty()){
                        continue;    //  Nothing to write.
                    }
                    int64_t us = std::chrono::duration_cast<std::chrono::microseconds>(
                        vp.timestamp - start_ts
                    ).count();
                    if (video_annexb){
                        if (!h264_annexb_to_avcc(vp.data.data(), vp.data.size(), converted_data)){
                            logger.log("Stream history: Failed to convert a video packet from Annex B to AVCC.", COLOR_RED);
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
                    //  `us` counts microseconds (a 1/1000000 timebase); convert to
                    //  the stream timebase, keeping the dts strictly increasing
                    //  (see the note above the loop).
                    int64_t dts = av_rescale_q(us, av_make_q(1, 1000000), video_stream->time_base);
                    if (dts <= last_video_dts){
                        dts = last_video_dts + 1;
                    }
                    last_video_dts = dts;
                    packet->pts = dts;
                    packet->dts = dts;
                    packet->flags = vp.flags;
                    if (av_interleaved_write_frame(format, packet) < 0){
                        logger.log("Stream history: Failed writing video data.", COLOR_RED);
                        ok = false;
                    }
                }
            }
            if (have_audio_stream){
                int64_t base_samples = p_audio[p_audio_start]->first_sample;
                for (size_t i = p_audio_start; i < p_audio.size() && ok; i++){
                    const AudioRingPacket& ap = *p_audio[i];
                    //  The muxer only reads the packet data.
                    packet->data = const_cast<uint8_t*>(ap.data.data());
                    packet->size = (int)ap.data.size();
                    packet->stream_index = audio_stream->index;
                    packet->pts = ap.first_sample - base_samples;
                    packet->dts = packet->pts;
                    packet->flags = 0;
                    if (av_interleaved_write_frame(format, packet) < 0){
                        logger.log("Stream history: Failed writing audio data.", COLOR_RED);
                        ok = false;
                    }
                }
            }
            av_packet_free(&packet);
        }

        if (ok && av_write_trailer(format) < 0){
            logger.log("Stream history: Failed writing file trailer.", COLOR_RED);
            ok = false;
        }
        if (format->pb != nullptr){
            avio_closep(&format->pb);
        }
        avformat_free_context(format);
        return ok;
    }


    bool save(const std::string& filename){
        logger.log("Saving stream history (in-memory encoded ring)...", COLOR_BLUE);

        //  Fast snapshot: the deques only hold shared_ptr's (the packet bytes are
        //  shared, not copied). Local names avoid C4458 (/W4) member shadowing.
        std::deque<std::shared_ptr<const VideoRingPacket>> video_snapshot;
        std::deque<std::shared_ptr<const AudioRingPacket>> audio_snapshot;
        {
            WriteSpinLock lg(lock, PA_CURRENT_FUNCTION);
            video_snapshot = this->video;
            audio_snapshot = this->audio;
        }
        if (video_snapshot.empty() && audio_snapshot.empty()){
            logger.log("Cannot save stream history: Nothing was recorded.", COLOR_RED);
            return false;
        }

        //  Rewind the video to the first keyframe (the oldest decodable start
        //  point) so the file decodes from the beginning. This loses at most one
        //  keyframe interval (~2 s) from the head of the window; rewinding to
        //  the *last* keyframe instead would discard almost the entire window.
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

        bool ok = mux_to_file(filename, video_snapshot, video_start_index, audio_snapshot, audio_start, start_ts);
        logger.log(
            ok ? "Stream history saved to '" + filename + "'."
               : "Failed to save stream history to '" + filename + "'.",
            ok ? COLOR_GREEN : COLOR_RED
        );
        return ok;
    }
};



StreamHistoryTracker_Ring::~StreamHistoryTracker_Ring(){
    Impl& data = *m_impl;
    if (data.worker){
        data.stopping = true;
        data.cv.notify_all();
        data.worker.wait_and_ignore_exceptions();
    }
}

StreamHistoryTracker_Ring::StreamHistoryTracker_Ring(
    Logger& logger,
    std::chrono::seconds window,
    size_t audio_samples_per_frame,
    size_t audio_frames_per_second,
    bool has_video
)
    : m_impl(std::make_unique<Impl>(
        logger,
        window,
        audio_samples_per_frame,
        audio_frames_per_second,
        has_video
    ))
{}

void StreamHistoryTracker_Ring::set_window(std::chrono::seconds window){
    Impl& data = *m_impl;
    WriteSpinLock lg(data.lock, PA_CURRENT_FUNCTION);
    data.window = window;
    data.evict();
}

void StreamHistoryTracker_Ring::on_samples(const float* data, size_t frames){
    m_impl->on_samples(data, frames);
}

void StreamHistoryTracker_Ring::on_frame(std::shared_ptr<const VideoFrame> frame){
    m_impl->on_frame(std::move(frame));
}

bool StreamHistoryTracker_Ring::save(const std::string& filename) const{
    return m_impl->save(filename);
}



}
