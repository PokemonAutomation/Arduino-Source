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
#include <type_traits>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/version.h>
#if __has_include(<libavutil/error.h>)
#include <libavutil/error.h>    //  ffmpeg 8.0 renamed err.h to error.h
#else
#include <libavutil/err.h>
#endif
#include <libavutil/log.h>
#include <libavutil/opt.h>
#include <libavutil/rational.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>

    //  Probe types for the ffmpeg API-generation checks below. They must be
    //  declared with C language linkage (i.e. inside this extern "C" block)
    //  to match decltype() of the functions declared above - C++ language
    //  linkage would make std::is_same fail even for identical signatures.
    using PaSwrAllocSetOpts2NewT = int (*)(SwrContext**, const AVChannelLayout*, AVSampleFormat, int,
                                           const AVChannelLayout*, AVSampleFormat, int, int, void*);
    using PaChannelLayoutDefaultNewT = void (*)(AVChannelLayout*, int);
}

//  ffmpeg has changed several API signatures between releases (5.1 - 8.x).
//  Which generation of the API the installed headers declare cannot be
//  inferred reliably from the LIBAV*_VERSION_MAJOR macros - distributions
//  ship different ffmpeg versions (with different major numbers and
//  different API generations), so the ring detects the actual API at
//  compile time from the declarations above. Each generation-specific call
//  is routed through a function template that is only instantiated when it
//  is actually taken, because non-dependent names/expressions in a
//  discarded if-constexpr branch are still looked up and checked.
//  (The one exception, avcodec_get_supported_config(), is not declared at
//  all in older headers, so that split is a preprocessor version gate that
//  removes the code entirely instead.)

//  Newer ffmpeg: av_channel_layout_default() returns void (validate the
//  result with av_channel_layout_check() instead).
static constexpr bool kChannelLayoutDefaultReturnsVoid =
    std::is_same_v<decltype(&av_channel_layout_default), PaChannelLayoutDefaultNewT>;
//  Newer ffmpeg: swr_alloc_set_opts2() takes a pointer to the context and
//  returns 0 on success, instead of returning the context directly.
static constexpr bool kSwrAllocSetOpts2TakesContextPtr =
    std::is_same_v<decltype(&swr_alloc_set_opts2), PaSwrAllocSetOpts2NewT>;

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


//  Initializes the layout with the given channel count, working against
//  both API generations. The generation-specific calls go through
//  function templates (non-type template parameters) so that they are
//  only instantiated when their branch is actually taken.
template <auto ChannelLayoutDefault, auto ChannelLayoutCheck>
static bool init_channel_layout_new_api(AVChannelLayout& layout, int channels){
    av_channel_layout_uninit(&layout);
    ChannelLayoutDefault(&layout, channels);
    return ChannelLayoutCheck(&layout) > 0;
}
template <auto ChannelLayoutDefault>
static bool init_channel_layout_old_api(AVChannelLayout& layout, int channels){
    av_channel_layout_uninit(&layout);
    return ChannelLayoutDefault(&layout, channels) >= 0;
}
static bool init_channel_layout(AVChannelLayout& layout, int channels){
    if constexpr (kChannelLayoutDefaultReturnsVoid){
        return init_channel_layout_new_api<av_channel_layout_default, av_channel_layout_check>(layout, channels);
    } else {
        return init_channel_layout_old_api<av_channel_layout_default>(layout, channels);
    }
}

//  Creates the 48 kHz resampler against whichever swresample generation is
//  installed. (Older releases: swr_alloc_set_opts2() takes an AVDictionary**
//  and returns the context; newer ones take a SwrContext** and return 0 on
//  success.)
template <auto SwrAllocSetOpts2>
static bool make_audio_resampler_new_api(SwrContext*& swr,
                                         const AVChannelLayout& out_layout, int out_rate,
                                         const AVChannelLayout& in_layout, int in_rate){
    int ret = SwrAllocSetOpts2(
        &swr,
        &out_layout, AV_SAMPLE_FMT_FLT, out_rate,
        &in_layout,  AV_SAMPLE_FMT_FLT, in_rate,
        0, nullptr
    );
    return ret == 0 && swr != nullptr;
}
template <auto SwrAllocSetOpts2>
static bool make_audio_resampler_old_api(SwrContext*& swr,
                                         const AVChannelLayout& out_layout, int out_rate,
                                         const AVChannelLayout& in_layout, int in_rate){
    AVDictionary* options = nullptr;    //  consumed (freed) by the call on both paths
    swr = SwrAllocSetOpts2(
        &options,
        &out_layout, AV_SAMPLE_FMT_FLT, out_rate,
        &in_layout,  AV_SAMPLE_FMT_FLT, in_rate,
        0, nullptr
    );
    return swr != nullptr;
}
static bool make_audio_resampler(SwrContext*& swr,
                                 const AVChannelLayout& out_layout, int out_rate,
                                 const AVChannelLayout& in_layout, int in_rate){
    if constexpr (kSwrAllocSetOpts2TakesContextPtr){
        return make_audio_resampler_new_api<swr_alloc_set_opts2>(swr, out_layout, out_rate, in_layout, in_rate);
    } else {
        return make_audio_resampler_old_api<swr_alloc_set_opts2>(swr, out_layout, out_rate, in_layout, in_rate);
    }
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
    int video_width = 0;
    int video_height = 0;
    int64_t last_keyframe_us = -VIDEO_KEYFRAME_INTERVAL_US;
    int64_t last_pts_us = -1;
    WallClock video_start = WallClock::min();

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
        //  The AAC stream configuration (AudioSpecificConfig) must land in the
        //  container header: with this flag the encoder emits raw AAC frames +
        //  extradata, which is what both the MP4 and MKV muxers need. Without
        //  it the packets would be ADTS and mux_to_file() would fail to set up
        //  the audio stream.
        actx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        if (!init_channel_layout(actx->ch_layout, (int)audio_channels)
            || avcodec_open2(actx, codec, nullptr) < 0
        ){
            logger.log("Stream history: Failed to open AAC encoder. Disabling audio.", COLOR_RED);
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
        int target_width = 0;
        int target_height = 0;
        if (!get_target_video_size(input_width, input_height, target_width, target_height)){
            logger.log("Stream history: Unsupported input resolution. Disabling video.", COLOR_RED);
            WriteSpinLock lg(lock, PA_CURRENT_FUNCTION);
            video_enabled = false;
            return;
        }

        size_t bitrate_kbps = get_target_bitrate_kbps();
        for (const char* name : VIDEO_ENCODER_CANDIDATES){
            const AVCodec* codec = avcodec_find_encoder_by_name(name);
            if (codec == nullptr){
                continue;
            }
            AVCodecContext* ctx = avcodec_alloc_context3(codec);
            if (ctx == nullptr){
                continue;
            }
            ctx->width = target_width;
            ctx->height = target_height;
            ctx->time_base = av_make_q(1, 1000000);
            ctx->bit_rate = (int64_t)bitrate_kbps * 1000;
            ctx->gop_size = (int)(target_fps * 2);
            ctx->max_b_frames = 0;
            ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

            //  Prefer NV12; otherwise take the first format the encoder supports.
#if LIBAVCODEC_VERSION_MAJOR >= 61
            //  ffmpeg 7.0+ (libavcodec 61+) removed AVCodec.pix_fmts; query the
            //  encoder instead. This split is a preprocessor version gate
            //  (rather than compile-time detection) because
            //  avcodec_get_supported_config() is not declared at all in older
            //  headers, and names in discarded if-constexpr branches are still
            //  looked up.
            //  (out_num_configs is optional; out_configs points at a
            //  list terminated by AV_PIX_FMT_NONE, or is NULL when every
            //  format is supported.)
            const enum AVPixelFormat* pix_fmts = nullptr;
            int ret = avcodec_get_supported_config(ctx, codec, AV_CODEC_CONFIG_PIX_FORMAT, 0, (const void**)&pix_fmts, nullptr);
            if (ret >= 0 && pix_fmts != nullptr){
                AVPixelFormat preferred = AV_PIX_FMT_NONE;
                for (int i = 0; pix_fmts[i] != AV_PIX_FMT_NONE; i++){
                    AVPixelFormat format = pix_fmts[i];
                    if (format == AV_PIX_FMT_NV12){
                        preferred = AV_PIX_FMT_NV12;
                        break;
                    }
                    if (preferred == AV_PIX_FMT_NONE){
                        preferred = format;
                    }
                }
                ctx->pix_fmt = preferred;
            }
#else
            if (codec->pix_fmts != nullptr){
                AVPixelFormat preferred = AV_PIX_FMT_NONE;
                for (int i = 0; codec->pix_fmts[i] != AV_PIX_FMT_NONE; i++){
                    AVPixelFormat format = (AVPixelFormat)codec->pix_fmts[i];
                    if (format == AV_PIX_FMT_NV12){
                        preferred = AV_PIX_FMT_NV12;
                        break;
                    }
                    if (preferred == AV_PIX_FMT_NONE){
                        preferred = format;
                    }
                }
                ctx->pix_fmt = preferred;
            }
#endif
            int err = avcodec_open2(ctx, codec, nullptr);
            if (err == 0){
                vctx = ctx;
                video_width = target_width;
                video_height = target_height;
                video_start = WallClock::min();
                last_keyframe_us = -VIDEO_KEYFRAME_INTERVAL_US;
                last_pts_us = -1;
                logger.log(
                    "Stream history: video encoder = " + std::string(name)
                    + " at " + std::to_string(target_width) + "x" + std::to_string(target_height)
                    + ", " + std::to_string(bitrate_kbps) + " kbps."
                );
                return;
            }
            logger.log(
                "Stream history: Video encoder '" + std::string(name) + "' failed to open: "
                + ffmpeg_error(err),
                COLOR_YELLOW
            );
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

        //  (Re)build the scaler if the input changed.
        if (src_format != sws_in_format || src_width != sws_width || src_height != sws_height){
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
        frame->pts = pts_us;
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
                logger.log("Stream history: Video encode failed: " + ffmpeg_error(err), COLOR_RED);
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
            frame->format = AV_SAMPLE_FMT_FLT;
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
            memcpy(
                frame->data[0],
                audio_acc.data(),
                (size_t)AUDIO_FRAME_SAMPLES * audio_channels * sizeof(float)
            );
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
        bool have_video = (p_video_start < p_video.size()) && (vctx != nullptr);
        bool have_audio_stream = (p_audio_start < p_audio.size()) && (actx != nullptr);

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
        if (have_video){
            video_stream = avformat_new_stream(format, nullptr);
            if (video_stream == nullptr
                || avcodec_parameters_from_context(video_stream->codecpar, vctx) < 0
                || vctx->extradata == nullptr
            ){
                logger.log("Stream history: Failed to set up video stream (missing encoder state?).", COLOR_RED);
                avformat_free_context(format);
                return false;
            }
            video_stream->time_base = vctx->time_base;
        }
        if (have_audio_stream){
            audio_stream = avformat_new_stream(format, nullptr);
            if (audio_stream == nullptr
                || avcodec_parameters_from_context(audio_stream->codecpar, actx) < 0
                || actx->extradata == nullptr
            ){
                logger.log("Stream history: Failed to set up audio stream (missing encoder state?).", COLOR_RED);
                avformat_free_context(format);
                return false;
            }
            audio_stream->time_base = av_make_q(1, AUDIO_OUTPUT_RATE);
        }

        if (avformat_write_header(format, &muxer_options) < 0){
            logger.log("Stream history: Failed to write file header for '" + filename + "'.", COLOR_RED);
            av_dict_free(&muxer_options);
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
                for (size_t i = p_video_start; i < p_video.size() && ok; i++){
                    const VideoRingPacket& vp = *p_video[i];
                    int64_t us = std::chrono::duration_cast<std::chrono::microseconds>(
                        vp.timestamp - start_ts
                    ).count();
                    //  The muxer only reads the packet data.
                    packet->data = const_cast<uint8_t*>(vp.data.data());
                    packet->size = (int)vp.data.size();
                    packet->stream_index = video_stream->index;
                    //  `us` counts microseconds (a 1/1000000 timebase); convert to
                    //  the stream timebase.
                    packet->pts = av_rescale_q(us, av_make_q(1, 1000000), video_stream->time_base);
                    packet->dts = packet->pts;
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
