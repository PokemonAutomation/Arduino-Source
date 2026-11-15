/*  Stream History Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Tests for the H.264 bitstream helpers (StreamHistory_H264.h), which check
 *  the exact output bytes, and (when PA_STREAM_HISTORY_FFMPEG is defined)
 *  end-to-end tests for StreamHistoryTracker_Ring: synthetic frames and
 *  audio on a simulated timeline, saved to MP4/MKV under DEBUG_PATH() and
 *  read back with ffmpeg. A passing test removes its file.
 *
 */

#include <stdint.h>
#include <string>
#include <vector>
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "StreamHistory_H264.h"
#include "StreamHistory_Tests.h"

#if defined(PA_STREAM_HISTORY_FFMPEG)
#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstring>
#include <functional>
#include <memory>
#include <random>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
}
#include <QImage>
#include <QVideoFrame>
#include "Common/Cpp/Time.h"
#include "Common/Cpp/Concurrency/AsyncTask.h"
#include "Common/Cpp/Filesystem/Filesystem.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Recording/StreamHistoryOption.h"
#include "CommonFramework/Tools/GlobalThreadPools.h"
#include "CommonFramework/VideoPipeline/Backends/VideoFrameQt.h"
#include "StreamHistoryTracker_Ring.h"
#endif

namespace PokemonAutomation{

namespace{



using Bytes = std::vector<uint8_t>;

Bytes concat(std::initializer_list<Bytes> parts){
    Bytes ret;
    for (const Bytes& part : parts){
        ret.insert(ret.end(), part.begin(), part.end());
    }
    return ret;
}

std::string to_hex(const Bytes& bytes){
    const char* digits = "0123456789abcdef";
    std::string ret;
    for (size_t i = 0; i < bytes.size(); i++){
        if (i != 0){
            ret += ' ';
        }
        ret += digits[bytes[i] >> 4];
        ret += digits[bytes[i] & 0xF];
    }
    return ret.empty() ? "(empty)" : ret;
}

//  Runs one helper and compares its return value and output bytes. The
//  output vector starts out non-empty, so the helper must replace it rather
//  than append to it.
template <typename Function>
UnitTestResult check_conversion(
    Function function,
    const Bytes& input,
    bool expected_ok,
    const Bytes& expected
){
    Bytes out{0xee, 0xee};
    bool ok = function(input.data(), input.size(), out);
    if (ok != expected_ok){
        return std::string("Returned ") + (ok ? "true" : "false")
            + ", expected " + (expected_ok ? "true" : "false")
            + ". Output: " + to_hex(out);
    }
    if (!expected_ok){
        return true;
    }
    if (out != expected){
        return "Output: " + to_hex(out) + "\nExpected: " + to_hex(expected);
    }
    return true;
}


const Bytes START3{0x00, 0x00, 0x01};
const Bytes START4{0x00, 0x00, 0x00, 0x01};

//  Sample parameter sets (High profile, level 3.1) and NAL units.
const Bytes SPS{0x67, 0x64, 0x00, 0x1f, 0xac, 0xd9, 0x40};
const Bytes PPS{0x68, 0xeb, 0xe3, 0xcb, 0x22, 0xc0};
const Bytes SEI{0x06, 0x05, 0xff};
const Bytes AUD{0x09, 0xf0};
const Bytes IDR{0x65, 0x88, 0x84};

//  The avcC High profile extension for SPS (ISO/IEC 14496-15): 4:2:0
//  chroma (0xfc | 1), 8-bit luma and chroma (0xf8 | 0), no SPS extensions.
const Bytes SPS_AVCC_EXTENSION{0xfd, 0xf8, 0xf8, 0x00};

//  The avcC record for SPS + PPS: version 1, profile/compatibility/level
//  from SPS bytes 1-3, 4-byte NAL lengths, one SPS, one PPS, and the High
//  profile extension.
const Bytes SPS_PPS_AVCC = concat({
    {0x01, 0x64, 0x00, 0x1f, 0xff, 0xe1, 0x00, 0x07},
    SPS,
    {0x01, 0x00, 0x06},
    PPS,
    SPS_AVCC_EXTENSION,
});

//  Main profile (77) parameter set: the avcC record has no extension.
const Bytes SPS_MAIN{0x67, 0x4d, 0x40, 0x1f, 0x96, 0x54};

//  High 4:2:2 profile (122), seq_parameter_set_id 0, chroma_format_idc 2,
//  10-bit luma and chroma (bit_depth_*_minus8 = 2).
const Bytes SPS_HIGH_422_10BIT{0x67, 0x7a, 0x00, 0x1f, 0xb6, 0xc8};

//  High 4:4:4 Predictive profile (244), chroma_format_idc 3 (followed by
//  separate_colour_plane_flag 0), 8-bit luma, 9-bit chroma.
const Bytes SPS_HIGH_444{0x67, 0xf4, 0x00, 0x1f, 0x91, 0x50};

//  SPS with level_idc 0: the 00 00 in front of the exp-Golomb fields takes
//  an emulation prevention byte (03), which the parser must skip. The avcC
//  record keeps the SPS as is.
const Bytes SPS_EMULATION_PREVENTION{0x67, 0x64, 0x00, 0x00, 0x03, 0xac, 0xd9, 0x40};

//  The SPS and PPS that libx264 writes for a 320x240 High profile stream,
//  and the avcC record that ffmpeg's own MP4 muxer builds from them.
const Bytes X264_SPS{
    0x67, 0x64, 0x00, 0x0c, 0xac, 0xd9, 0x41, 0x41, 0xfb, 0x01, 0x10, 0x00, 0x00,
    0x03, 0x00, 0x10, 0x00, 0x00, 0x03, 0x01, 0x40, 0xf1, 0x42, 0x99, 0x60,
};
const Bytes X264_PPS{0x68, 0xeb, 0xe3, 0xcb, 0x22, 0xc0};
const Bytes X264_FFMPEG_AVCC{
    0x01, 0x64, 0x00, 0x0c, 0xff, 0xe1, 0x00, 0x19, 0x67, 0x64, 0x00, 0x0c, 0xac,
    0xd9, 0x41, 0x41, 0xfb, 0x01, 0x10, 0x00, 0x00, 0x03, 0x00, 0x10, 0x00, 0x00,
    0x03, 0x01, 0x40, 0xf1, 0x42, 0x99, 0x60, 0x01, 0x00, 0x06, 0x68, 0xeb, 0xe3,
    0xcb, 0x22, 0xc0, 0xfd, 0xf8, 0xf8, 0x00,
};



class AnnexBToAvccTest : public UnitTest{
public:
    AnnexBToAvccTest(const std::string& name, Bytes input, bool expected_ok, Bytes expected)
        : UnitTest("StreamHistory - H264 AnnexB to AVCC - " + name)
        , m_input(std::move(input))
        , m_expected_ok(expected_ok)
        , m_expected(std::move(expected))
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        return check_conversion(h264_annexb_to_avcc, m_input, m_expected_ok, m_expected);
    }

private:
    Bytes m_input;
    bool m_expected_ok;
    Bytes m_expected;
};


class AvccFromExtradataTest : public UnitTest{
public:
    AvccFromExtradataTest(const std::string& name, Bytes input, bool expected_ok, Bytes expected)
        : UnitTest("StreamHistory - H264 avcC from extradata - " + name)
        , m_input(std::move(input))
        , m_expected_ok(expected_ok)
        , m_expected(std::move(expected))
    {}

    virtual UnitTestResult run(Logger&, CancellableScope&) const override{
        return check_conversion(h264_avcc_from_annexb_extradata, m_input, m_expected_ok, m_expected);
    }

private:
    Bytes m_input;
    bool m_expected_ok;
    Bytes m_expected;
};



#if defined(PA_STREAM_HISTORY_FFMPEG)

//
//  StreamHistoryTracker_Ring, end to end: synthetic frames and audio on a
//  simulated timeline, saved to a file, then read back and decoded.
//


constexpr int64_t CHUNK_MS = 10;
constexpr int64_t FRAME_INTERVAL_MS = 100;
constexpr int AUDIO_RATE = 48000;
constexpr int FRAME_WIDTH = 320;
constexpr int FRAME_HEIGHT = 180;

//  Allowed distance between the start of the tone and the first white frame
//  in a saved file. Covers AAC pre-echo (the encoder smears some energy up
//  to one frame, about 21 ms, ahead of the onset) and the millisecond
//  rounding of the MKV timestamps.
constexpr double SYNC_TOLERANCE_S = 0.03;

//  Largest allowed deviation of the spacing between consecutive audio
//  packets from one AAC frame (1024 samples), when the audio timeline must
//  be continuous (no re-sync). Covers the millisecond rounding of MKV.
constexpr double AUDIO_SPACING_TOLERANCE_S = 0.0011;

//  The tracker re-syncs the audio to the clock past this much drift
//  (AUDIO_RESYNC_THRESHOLD_US in StreamHistoryTracker_Ring.cpp).
constexpr double AUDIO_RESYNC_THRESHOLD_S = 0.1;

//  The tracker forces a keyframe on the first frame at least this long
//  after the previous one (VIDEO_KEYFRAME_INTERVAL_US).
constexpr double VIDEO_KEYFRAME_INTERVAL_S = 2.0;

//  Video trailing the audio by more than this has stopped, and is evicted
//  by the audio's clock (MAX_VIDEO_LAG_US).
constexpr int64_t MAX_VIDEO_LAG_MS = 5000;

//  Frames fed after the point a test checks, so that the encoder emits the
//  frames before it. The encoder holds the newest frames for a while: the
//  hardware encoders pipeline several, more when many tests share the GPU
//  (over a second has been seen with h264_amf). The frames still held at
//  save() are not in the file.
constexpr int64_t ENCODER_TAIL_MS = 3000;


//  The interval between the frames the tracker keeps: every fed frame
//  (frame_interval_ms) unless the configured FPS throttles below that.
double kept_frame_interval_s(int64_t frame_interval_ms = FRAME_INTERVAL_MS){
    double fps = (double)GlobalSettings::instance().STREAM_HISTORY->target_fps();
    return std::max(frame_interval_ms / 1000., 1 / fps);
}

std::string output_path(const std::string& name){
    std::string directory = DEBUG_PATH() + "StreamHistoryTests/";
    Filesystem::create_directories(directory);
    return directory + name;
}



//  Feeds a tracker in 10 ms steps of simulated time: by default a frame
//  every 100 ms and a 10 ms chunk of 48 kHz stereo audio per step. Each chunk
//  is 480 samples, delivered at the time of its last sample; with
//  audio_clock_scale a chunk is a sample longer or shorter now and then,
//  and with audio_jitter_ms it is delivered later. From "onset_ms" on, the
//  frames are white instead of black and the audio is a 1 kHz tone instead
//  of silence.
enum class FrameFormat{
    RGB32,  //  Converted through QImage by the tracker.
    NV12,   //  The common capture card format; scaled directly from its planes.
};

struct FeedOptions{
    bool video = true;
    bool audio = true;
    FrameFormat frame_format = FrameFormat::RGB32;
    //  The time between frames. A multiple of CHUNK_MS.
    int64_t frame_interval_ms = FRAME_INTERVAL_MS;
    //  No frames are delivered from this time on.
    int64_t video_end_ms = -1;
    int64_t onset_ms = -1;
    //  No audio is delivered in [gap_begin_ms, gap_end_ms).
    int64_t gap_begin_ms = -1;
    int64_t gap_end_ms = -1;
    //  From this time on, the frames are twice as large in each dimension.
    int64_t resize_ms = -1;
    //  Each audio chunk is delivered a random 0 - audio_jitter_ms after its
    //  last sample is captured (chunks stay in order).
    int64_t audio_jitter_ms = 0;
    //  Samples the audio device produces per second of real time, relative
    //  to 48 kHz (1.001 = a device clock running 0.1% fast).
    double audio_clock_scale = 1.0;
    //  Wait for the encoder after each frame (otherwise frames just queue).
    bool wait_for_encoder = true;
    //  Each channel's buffer is this many samples longer than the samples
    //  delivered (stride = samples + padding), as when the pipeline reuses
    //  a larger buffer. The padding is full scale, so audio read from it
    //  shows up as a tone.
    size_t audio_stride_padding = 0;
};

class Feeder{
public:
    Feeder(StreamHistoryTracker_Ring& tracker, FeedOptions options)
        : m_tracker(tracker)
        , m_options(options)
        , m_start(current_time())
        , m_last_delivery(m_start)
        , m_random(12345)
    {}

    void run_until(int64_t end_ms){
        for (; m_time_ms < end_ms; m_time_ms += CHUNK_MS){
            bool video_ended = m_options.video_end_ms >= 0 && m_time_ms >= m_options.video_end_ms;
            if (m_options.video && !video_ended && m_time_ms % m_options.frame_interval_ms == 0){
                m_tracker.on_frame(std::make_shared<const VideoFrame>(
                    m_start + std::chrono::milliseconds(m_time_ms),
                    make_frame()
                ));
                if (m_options.wait_for_encoder){
                    m_tracker.wait_until_idle();
                }
            }
            bool in_gap = m_time_ms >= m_options.gap_begin_ms && m_time_ms < m_options.gap_end_ms;
            if (m_options.audio && !in_gap){
                push_audio_chunk();
            }
        }
    }

private:
    bool onset() const{
        return m_options.onset_ms >= 0 && m_time_ms >= m_options.onset_ms;
    }

    QVideoFrame make_frame() const{
        bool large = m_options.resize_ms >= 0 && m_time_ms >= m_options.resize_ms;
        int width = large ? 2 * FRAME_WIDTH : FRAME_WIDTH;
        int height = large ? 2 * FRAME_HEIGHT : FRAME_HEIGHT;
        if (m_options.frame_format == FrameFormat::RGB32){
            QImage image(width, height, QImage::Format_RGB32);
            image.fill(onset() ? Qt::white : Qt::black);
            return QVideoFrame(image);
        }

        //  NV12: a full-resolution luma plane, then a half-resolution plane
        //  of interleaved chroma. Limited-range black or white, no color.
        QVideoFrame frame(QVideoFrameFormat(QSize(width, height), QVideoFrameFormat::Format_NV12));
        if (!frame.map(QVideoFrame::WriteOnly)){
            return QVideoFrame();
        }
        for (int y = 0; y < height; y++){
            std::memset(frame.bits(0) + (ptrdiff_t)y * frame.bytesPerLine(0), onset() ? 235 : 16, (size_t)width);
        }
        for (int y = 0; y < height / 2; y++){
            std::memset(frame.bits(1) + (ptrdiff_t)y * frame.bytesPerLine(1), 128, (size_t)width);
        }
        frame.unmap();
        return frame;
    }

    //  The device's sample count at a point in real time.
    int64_t device_samples(int64_t time_ms) const{
        return std::llround((double)time_ms * AUDIO_RATE / 1000 * m_options.audio_clock_scale);
    }

    void push_audio_chunk(){
        //  The samples the device captures during this step. With a drifting
        //  device clock, that is a sample more or less than 480 now
        //  and then.
        int64_t first_sample = device_samples(m_time_ms);
        size_t samples = (size_t)(device_samples(m_time_ms + CHUNK_MS) - first_sample);
        int64_t onset_sample = device_samples(m_options.onset_ms);
        size_t stride = samples + m_options.audio_stride_padding;
        m_audio.assign(2 * stride, 1.0f);
        for (size_t i = 0; i < samples; i++){
            int64_t sample = first_sample + (int64_t)i;
            float value = 0;
            if (m_options.onset_ms >= 0 && sample >= onset_sample){
                value = (float)(0.5 * std::sin(2 * 3.14159265358979 * 1000 * (double)sample / AUDIO_RATE));
            }
            m_audio[i] = value;             //  Left.
            m_audio[stride + i] = value;    //  Right.
        }

        WallClock delivery = m_start + std::chrono::milliseconds(m_time_ms + CHUNK_MS);
        if (m_options.audio_jitter_ms > 0){
            std::uniform_int_distribution<int64_t> latency(0, m_options.audio_jitter_ms);
            delivery += std::chrono::milliseconds(latency(m_random));
        }
        m_last_delivery = std::max(m_last_delivery, delivery);
        m_tracker.push_samples(samples, stride, m_audio.data(), m_last_delivery);
    }

private:
    StreamHistoryTracker_Ring& m_tracker;
    FeedOptions m_options;
    WallClock m_start;
    WallClock m_last_delivery;
    std::mt19937 m_random;
    int64_t m_time_ms = 0;
    std::vector<float> m_audio;
};



//  What a saved file contains, as read back by ffmpeg. Times are seconds
//  on the file's timeline.
struct ProbedFile{
    bool has_video = false;
    bool has_audio = false;

    bool first_video_packet_is_key = false;
    bool video_timestamps_increasing = true;
    double video_first_s = 0;
    double video_last_s = 0;
    size_t video_packets = 0;
    //  The duration of the video stream as the container states it (-1 if
    //  it states none). For MP4 this ends with the last sample's duration,
    //  which the file only has if the muxer was given it.
    double video_stream_duration_s = -1;
    size_t video_frames = 0;
    //  Largest distance between consecutive keyframes (0 with fewer than two).
    double video_max_keyframe_gap_s = 0;
    size_t video_keyframes = 0;
    double video_onset_s = -1;      //  First decoded frame that is no longer black.

    bool audio_timestamps_increasing = true;
    double audio_first_s = 0;
    //  Largest deviation of the spacing between consecutive audio packets
    //  from one AAC frame.
    double audio_max_spacing_error_s = 0;
    //  Smallest spacing between consecutive audio packets.
    double audio_min_spacing_s = 1e9;
    size_t audio_packets = 0;
    size_t audio_frames = 0;
    double audio_onset_s = -1;      //  First decoded sample of the tone.
};

struct ProbeContext{
    AVFormatContext* format = nullptr;
    AVCodecContext* video = nullptr;
    AVCodecContext* audio = nullptr;
    AVPacket* packet = nullptr;
    AVFrame* frame = nullptr;
    ~ProbeContext(){
        av_frame_free(&frame);
        av_packet_free(&packet);
        avcodec_free_context(&video);
        avcodec_free_context(&audio);
        avformat_close_input(&format);
    }
};

std::string open_decoder(const AVStream& stream, AVCodecContext*& context){
    const AVCodec* codec = avcodec_find_decoder(stream.codecpar->codec_id);
    if (codec == nullptr){
        return std::string("No decoder for ") + avcodec_get_name(stream.codecpar->codec_id) + ".";
    }
    context = avcodec_alloc_context3(codec);
    if (context == nullptr
        || avcodec_parameters_to_context(context, stream.codecpar) < 0
    ){
        return "Failed to set up the decoder.";
    }
    context->pkt_timebase = stream.time_base;
    if (avcodec_open2(context, codec, nullptr) < 0){
        return std::string("Failed to open the ") + codec->name + " decoder.";
    }
    return "";
}

//  Average of the luma plane (16 = black, 235 = white for limited range).
std::string mean_luma(const AVFrame& frame, double& luma){
    const AVPixFmtDescriptor* descriptor = av_pix_fmt_desc_get((AVPixelFormat)frame.format);
    if (descriptor == nullptr
        || (descriptor->flags & AV_PIX_FMT_FLAG_RGB)
        || descriptor->comp[0].depth != 8
    ){
        return "Unexpected decoded pixel format.";
    }
    int step = descriptor->comp[0].step;
    int offset = descriptor->comp[0].offset;
    uint64_t sum = 0;
    for (int y = 0; y < frame.height; y++){
        const uint8_t* row = frame.data[0] + (ptrdiff_t)y * frame.linesize[0] + offset;
        for (int x = 0; x < frame.width; x++){
            sum += row[x * step];
        }
    }
    luma = (double)sum / ((double)frame.width * frame.height);
    return "";
}

std::string probe_file(const std::string& path, ProbedFile& file){
    ProbeContext context;
    if (avformat_open_input(&context.format, path.c_str(), nullptr, nullptr) < 0){
        return "Failed to open the saved file.";
    }
    if (avformat_find_stream_info(context.format, nullptr) < 0){
        return "Failed to read the saved file's stream info.";
    }
    int video_index = av_find_best_stream(context.format, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    int audio_index = av_find_best_stream(context.format, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    file.has_video = video_index >= 0;
    file.has_audio = audio_index >= 0;
    AVRational video_time_base{1, 1};
    AVRational audio_time_base{1, 1};
    std::string error;
    if (file.has_video){
        const AVStream& stream = *context.format->streams[video_index];
        video_time_base = stream.time_base;
        if (stream.duration != AV_NOPTS_VALUE){
            file.video_stream_duration_s = stream.duration * av_q2d(video_time_base);
        }
        error = open_decoder(stream, context.video);
        if (!error.empty()){
            return error;
        }
    }
    if (file.has_audio){
        const AVStream& stream = *context.format->streams[audio_index];
        audio_time_base = stream.time_base;
        error = open_decoder(stream, context.audio);
        if (!error.empty()){
            return error;
        }
    }
    context.packet = av_packet_alloc();
    context.frame = av_frame_alloc();
    if (context.packet == nullptr || context.frame == nullptr){
        return "Out of memory.";
    }

    auto receive_video = [&]() -> std::string{
        while (true){
            int ret = avcodec_receive_frame(context.video, context.frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
                return "";
            }
            if (ret < 0){
                return "Failed to decode a video frame.";
            }
            file.video_frames++;
            if (file.video_onset_s < 0){
                double luma = 0;
                std::string luma_error = mean_luma(*context.frame, luma);
                if (!luma_error.empty()){
                    return luma_error;
                }
                if (luma > 64){
                    file.video_onset_s = context.frame->best_effort_timestamp * av_q2d(video_time_base);
                }
            }
            av_frame_unref(context.frame);
        }
    };
    auto receive_audio = [&]() -> std::string{
        while (true){
            int ret = avcodec_receive_frame(context.audio, context.frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
                return "";
            }
            if (ret < 0){
                return "Failed to decode an audio frame.";
            }
            file.audio_frames++;
            if (file.audio_onset_s < 0){
                AVSampleFormat format = (AVSampleFormat)context.frame->format;
                if (format != AV_SAMPLE_FMT_FLTP && format != AV_SAMPLE_FMT_FLT){
                    return "Unexpected decoded sample format.";
                }
                //  Left channel: plane 0 (planar) or every other sample (packed).
                int step = format == AV_SAMPLE_FMT_FLT ? context.frame->ch_layout.nb_channels : 1;
                const float* samples = (const float*)context.frame->data[0];
                for (int i = 0; i < context.frame->nb_samples; i++){
                    if (std::fabs(samples[i * step]) > 0.1f){
                        file.audio_onset_s = context.frame->best_effort_timestamp * av_q2d(audio_time_base)
                            + (double)i / context.frame->sample_rate;
                        break;
                    }
                }
            }
            av_frame_unref(context.frame);
        }
    };

    bool first_video_packet = true;
    double last_keyframe_s = -1;
    int64_t last_video_ts = INT64_MIN;
    int64_t last_audio_ts = INT64_MIN;
    while (av_read_frame(context.format, context.packet) >= 0){
        AVPacket& packet = *context.packet;
        int64_t ts = packet.dts != AV_NOPTS_VALUE ? packet.dts : packet.pts;
        if (packet.stream_index == video_index){
            if (first_video_packet){
                file.first_video_packet_is_key = (packet.flags & AV_PKT_FLAG_KEY) != 0;
                file.video_first_s = packet.pts * av_q2d(video_time_base);
                first_video_packet = false;
            }
            file.video_last_s = packet.pts * av_q2d(video_time_base);
            file.video_packets++;
            if (packet.flags & AV_PKT_FLAG_KEY){
                file.video_keyframes++;
                if (last_keyframe_s >= 0){
                    file.video_max_keyframe_gap_s = std::max(
                        file.video_max_keyframe_gap_s, file.video_last_s - last_keyframe_s
                    );
                }
                last_keyframe_s = file.video_last_s;
            }
            if (ts <= last_video_ts){
                file.video_timestamps_increasing = false;
            }
            last_video_ts = ts;
            if (avcodec_send_packet(context.video, &packet) < 0){
                error = "Failed to decode a video packet.";
            }else{
                error = receive_video();
            }
        }else if (packet.stream_index == audio_index){
            file.audio_packets++;
            if (ts <= last_audio_ts){
                file.audio_timestamps_increasing = false;
            }
            if (last_audio_ts == INT64_MIN){
                file.audio_first_s = ts * av_q2d(audio_time_base);
            }else{
                double spacing_s = (double)(ts - last_audio_ts) * av_q2d(audio_time_base);
                file.audio_max_spacing_error_s = std::max(
                    file.audio_max_spacing_error_s,
                    std::fabs(spacing_s - 1024. / AUDIO_RATE)
                );
                file.audio_min_spacing_s = std::min(file.audio_min_spacing_s, spacing_s);
            }
            last_audio_ts = ts;
            if (avcodec_send_packet(context.audio, &packet) < 0){
                error = "Failed to decode an audio packet.";
            }else{
                error = receive_audio();
            }
        }
        av_packet_unref(context.packet);
        if (!error.empty()){
            return error;
        }
    }
    if (context.video != nullptr && avcodec_send_packet(context.video, nullptr) >= 0){
        error = receive_video();
        if (!error.empty()){
            return error;
        }
    }
    if (context.audio != nullptr && avcodec_send_packet(context.audio, nullptr) >= 0){
        error = receive_audio();
        if (!error.empty()){
            return error;
        }
    }
    return "";
}

//  The checks every saved file must pass.
std::string check_file(const ProbedFile& file, bool video, bool audio){
    if (file.has_video != video){
        return video ? "The file has no video stream." : "The file has an unexpected video stream.";
    }
    if (file.has_audio != audio){
        return audio ? "The file has no audio stream." : "The file has an unexpected audio stream.";
    }
    if (video){
        if (!file.first_video_packet_is_key){
            return "The first video packet is not a keyframe.";
        }
        if (!file.video_timestamps_increasing){
            return "The video timestamps are not strictly increasing.";
        }
        if (file.video_frames == 0){
            return "No video frame decoded.";
        }
        //  A keyframe at least every keyframe interval, give or take a frame:
        //  save() can only start the file at one. (The hardware encoders
        //  need forced_idr for this.)
        double max_gap_s = VIDEO_KEYFRAME_INTERVAL_S + kept_frame_interval_s() + 0.01;
        if (file.video_max_keyframe_gap_s > max_gap_s){
            return "Keyframes are up to " + std::to_string(file.video_max_keyframe_gap_s)
                + " s apart, expected at most " + std::to_string(max_gap_s) + " s.";
        }
        //  Keyframes are forced every keyframe interval, not on every frame:
        //  a frame must not stay marked as a keyframe once it was forced
        //  (the tracker reuses its frames). Scene changes may add a few
        //  more, so only a file that is mostly keyframes fails.
        if (file.video_packets >= 10 && 2 * file.video_keyframes > file.video_packets){
            return std::to_string(file.video_keyframes) + " of the "
                + std::to_string(file.video_packets) + " video packets are keyframes.";
        }
    }
    if (audio){
        if (!file.audio_timestamps_increasing){
            return "The audio timestamps are not strictly increasing.";
        }
        if (file.audio_frames == 0){
            return "No audio frame decoded.";
        }
    }
    return "";
}

//  Saves, reads back, and checks. The file is removed if everything passes
//  (and kept for inspection otherwise).
std::string save_and_probe(
    StreamHistoryTracker_Ring& tracker, const std::string& filename,
    bool video, bool audio, ProbedFile& file
){
    std::string path = output_path(filename);
    if (!tracker.save(path)){
        return "save() failed.";
    }
    std::string error = probe_file(path, file);
    if (error.empty()){
        error = check_file(file, video, audio);
    }
    if (!error.empty()){
        return error + " (" + path + ")";
    }
    Filesystem::remove(path);
    return "";
}

std::string check_range(const char* what, double value, double low, double high){
    if (low <= value && value <= high){
        return "";
    }
    return std::string(what) + " is " + std::to_string(value)
        + " s, expected " + std::to_string(low) + " - " + std::to_string(high) + " s.";
}



std::string test_save(Logger& logger, const std::string& filename, FeedOptions options){
    constexpr int64_t END_MS = 5000;
    StreamHistoryTracker_Ring tracker(logger, std::chrono::seconds(30), options.audio, options.video);
    Feeder feeder(tracker, options);
    feeder.run_until(END_MS + ENCODER_TAIL_MS);

    ProbedFile file;
    std::string error = save_and_probe(tracker, filename, options.video, options.audio, file);
    if (!error.empty() || !options.video){
        return error;
    }
    //  Nothing is evicted: the file starts at the first frame (0 s) and
    //  reaches at least the last frame kept before END_MS (4.9 s, or earlier
    //  when the FPS setting throttles). The tail may or may not be in it.
    double end_s = END_MS / 1000.;
    return check_range(
        "The video length",
        file.video_last_s - file.video_first_s,
        end_s - kept_frame_interval_s() - 0.01, end_s + ENCODER_TAIL_MS / 1000. + 0.01
    );
}

//  Checks a file saved from a ring that evicted to "window_s" seconds.
std::string check_evicted_file(const ProbedFile& file, int64_t window_s){
    //  The window is measured back from the newest video packet in the ring,
    //  however far the encoder trails. The file starts at a keyframe at most
    //  one keyframe interval (2 s) plus one frame before the window (the
    //  upper bound), and covers the window up to the rounding of the
    //  keyframe spacing to whole frames (the lower bound).
    double interval = kept_frame_interval_s();
    std::string error = check_range(
        "The video length",
        file.video_last_s - file.video_first_s,
        (double)window_s - 2 * interval, (double)window_s + VIDEO_KEYFRAME_INTERVAL_S + interval + 0.01
    );
    if (!error.empty()){
        return error;
    }
    //  save() drops the audio from before the first keyframe, and the audio
    //  is continuous, so it starts within one AAC frame after the video.
    return check_range(
        "The audio start minus the video start",
        file.audio_first_s - file.video_first_s,
        -AUDIO_SPACING_TOLERANCE_S, 1024. / AUDIO_RATE + AUDIO_SPACING_TOLERANCE_S
    );
}

std::string test_eviction(Logger& logger){
    constexpr int64_t WINDOW_S = 3;
    StreamHistoryTracker_Ring tracker(logger, std::chrono::seconds(WINDOW_S), true, true);
    Feeder feeder(tracker, FeedOptions());
    feeder.run_until(12000);

    ProbedFile file;
    std::string error = save_and_probe(tracker, "Eviction.mp4", true, true, file);
    if (!error.empty()){
        return error;
    }
    return check_evicted_file(file, WINDOW_S);
}

std::string test_shrink_window(Logger& logger){
    //  Shrinking the history length evicts down to the new window at once,
    //  before any more packets arrive.
    constexpr int64_t WINDOW_S = 3;
    StreamHistoryTracker_Ring tracker(logger, std::chrono::seconds(30), true, true);
    Feeder feeder(tracker, FeedOptions());
    feeder.run_until(12000);
    tracker.set_window(std::chrono::seconds(WINDOW_S));

    ProbedFile file;
    std::string error = save_and_probe(tracker, "ShrinkWindow.mp4", true, true, file);
    if (!error.empty()){
        return error;
    }
    return check_evicted_file(file, WINDOW_S);
}

std::string test_video_stops(Logger& logger){
    //  The video stops (e.g. the capture card is unplugged) while the audio
    //  continues. The old video must still be evicted: once the audio is
    //  more than MAX_VIDEO_LAG_MS plus the window and the keyframe headroom
    //  past the last frame, the file is audio only.
    constexpr int64_t WINDOW_S = 3;
    constexpr int64_t VIDEO_END_MS = 2000;
    FeedOptions options;
    options.video_end_ms = VIDEO_END_MS;
    StreamHistoryTracker_Ring tracker(logger, std::chrono::seconds(WINDOW_S), true, true);
    Feeder feeder(tracker, options);
    feeder.run_until(
        VIDEO_END_MS + MAX_VIDEO_LAG_MS + WINDOW_S * 1000
        + (int64_t)(VIDEO_KEYFRAME_INTERVAL_S * 1000) + 2000
    );

    ProbedFile file;
    return save_and_probe(tracker, "VideoStops.mp4", false, true, file);
}

std::string test_throttle(Logger& logger){
    //  Frames arrive at 50 fps, faster than any FPS setting: the tracker
    //  keeps one per target frame interval.
    constexpr int64_t END_MS = 5000;
    FeedOptions options;
    options.frame_interval_ms = 20;
    StreamHistoryTracker_Ring tracker(logger, std::chrono::seconds(30), true, true);
    Feeder feeder(tracker, options);
    feeder.run_until(END_MS + ENCODER_TAIL_MS);

    ProbedFile file;
    std::string error = save_and_probe(tracker, "Throttle.mp4", true, true, file);
    if (!error.empty()){
        return error;
    }
    if (file.video_packets < 2){
        return "Fewer than two video packets in the file.";
    }
    double interval = kept_frame_interval_s(options.frame_interval_ms);
    double spacing = (file.video_last_s - file.video_first_s) / (double)(file.video_packets - 1);
    return check_range("The average frame spacing", spacing, 0.95 * interval, 1.05 * interval);
}

//  Records until "end_ms" with the onset at "options.onset_ms", then checks
//  that the tone starts within "tolerance_s" of the first white frame, and
//  that no audio packets are squeezed closer than one AAC frame. With
//  "continuous_audio", also checks that the audio packets are evenly spaced
//  (the tracker never re-synced the audio to the clock).
std::string test_sync(
    Logger& logger, const std::string& filename,
    FeedOptions options, int64_t end_ms,
    double tolerance_s, bool continuous_audio
){
    StreamHistoryTracker_Ring tracker(logger, std::chrono::seconds(30), true, true);
    Feeder feeder(tracker, options);
    feeder.run_until(end_ms);

    ProbedFile file;
    std::string error = save_and_probe(tracker, filename, true, true, file);
    if (!error.empty()){
        return error;
    }
    if (file.video_onset_s < 0){
        return "The video never turns white.";
    }
    if (file.audio_onset_s < 0){
        return "The tone is not in the audio.";
    }
    error = check_range(
        "The audio onset minus the video onset",
        file.audio_onset_s - file.video_onset_s,
        -tolerance_s, tolerance_s
    );
    if (error.empty()){
        error = check_range(
            "The smallest audio packet spacing",
            file.audio_min_spacing_s,
            1024. / AUDIO_RATE - AUDIO_SPACING_TOLERANCE_S, 1e9
        );
    }
    if (error.empty() && continuous_audio){
        error = check_range(
            "The largest audio packet spacing error",
            file.audio_max_spacing_error_s,
            0, AUDIO_SPACING_TOLERANCE_S
        );
    }
    return error;
}

FeedOptions sync_options(){
    FeedOptions options;
    options.onset_ms = 3000;
    return options;
}

std::string test_mp4_matches_mkv(Logger& logger){
    //  The same ring saved twice holds the same packets in both containers.
    //  An MP4 sample's duration is the gap to the next sample, so the last
    //  one's duration comes from the muxer's packet duration alone: without
    //  it the newest frame is stored 0 long, the video stream ends a frame
    //  early, and some readers drop that frame. The recording is stopped
    //  before saving, so both saves see the same ring.
    constexpr int64_t END_MS = 5000;
    StreamHistoryTracker_Ring tracker(logger, std::chrono::seconds(30), true, true);
    Feeder feeder(tracker, FeedOptions());
    feeder.run_until(END_MS + ENCODER_TAIL_MS);

    ProbedFile mp4;
    std::string error = save_and_probe(tracker, "SameRing.mp4", true, true, mp4);
    if (!error.empty()){
        return "MP4: " + error;
    }
    ProbedFile mkv;
    error = save_and_probe(tracker, "SameRing.mkv", true, true, mkv);
    if (!error.empty()){
        return "MKV: " + error;
    }
    if (mp4.video_packets != mkv.video_packets){
        return "The MP4 has " + std::to_string(mp4.video_packets) + " video packets, the MKV "
            + std::to_string(mkv.video_packets) + ".";
    }
    if (mp4.audio_packets != mkv.audio_packets){
        return "The MP4 has " + std::to_string(mp4.audio_packets) + " audio packets, the MKV "
            + std::to_string(mkv.audio_packets) + ".";
    }
    //  The stream runs from the first frame to the end of the last one.
    double interval = kept_frame_interval_s();
    double frames_s = mp4.video_last_s - mp4.video_first_s;
    return check_range(
        "The MP4 video stream duration",
        mp4.video_stream_duration_s, frames_s + 0.95 * interval, frames_s + 1.05 * interval
    );
}

std::string test_nothing_recorded(Logger& logger){
    StreamHistoryTracker_Ring tracker(logger, std::chrono::seconds(30), true, true);
    std::string path = output_path("NothingRecorded.mp4");
    if (tracker.save(path)){
        Filesystem::remove(path);
        return "save() succeeded with nothing recorded.";
    }
    return "";
}

std::string test_unwritable_path(Logger& logger){
    StreamHistoryTracker_Ring tracker(logger, std::chrono::seconds(30), true, true);
    Feeder feeder(tracker, FeedOptions());
    feeder.run_until(1000);
    if (tracker.save(output_path("missing-directory/Unwritable.mp4"))){
        return "save() succeeded into a directory that does not exist.";
    }
    return "";
}

std::string test_shutdown(Logger& logger){
    //  Destroy the tracker while frames are still queued for the encoder: the
    //  destructor must drop them and return promptly.
    WallClock start = current_time();
    {
        StreamHistoryTracker_Ring tracker(logger, std::chrono::seconds(30), true, true);
        FeedOptions options;
        options.wait_for_encoder = false;
        Feeder feeder(tracker, options);
        feeder.run_until(1000);
    }
    double elapsed_s = std::chrono::duration<double>(current_time() - start).count();
    return check_range("Construction, feeding and destruction", elapsed_s, 0, 10);
}

std::string test_save_while_recording(Logger& logger){
    StreamHistoryTracker_Ring tracker(logger, std::chrono::seconds(30), true, true);
    Feeder feeder(tracker, FeedOptions());
    feeder.run_until(1000);

    //  Save repeatedly on another thread while the recording continues.
    std::string saver_error;
    AsyncTask saver = GlobalThreadPools::unlimited_normal().dispatch_now_blocking([&]{
        for (int c = 0; c < 5 && saver_error.empty(); c++){
            ProbedFile file;
            saver_error = save_and_probe(
                tracker, "SaveWhileRecording-" + std::to_string(c) + ".mp4", true, true, file
            );
        }
    });
    feeder.run_until(6000);
    saver.wait_and_rethrow_exceptions();
    return saver_error;
}



class RingTest : public UnitTest{
public:
    RingTest(const std::string& name, std::function<std::string(Logger&)> body)
        : UnitTest("StreamHistory - Ring - " + name)
        , m_body(std::move(body))
    {
        m_threads = 2;    //  The tracker's encoder thread.
    }

    virtual UnitTestResult run(Logger& logger, CancellableScope&) const override{
        std::string error = m_body(logger);
        if (error.empty()){
            return true;
        }
        return error;
    }

private:
    std::function<std::string(Logger&)> m_body;
};

#endif



}



void add_tests_StreamHistory(UnitTestDatabase& database){
    //  Annex B packets to AVCC (4-byte big-endian length prefixes).
    database.add<AnnexBToAvccTest>(
        "4-byte start code",
        concat({START4, IDR}),
        true, concat({{0x00, 0x00, 0x00, 0x03}, IDR})
    );
    database.add<AnnexBToAvccTest>(
        "3-byte start code",
        concat({START3, IDR}),
        true, concat({{0x00, 0x00, 0x00, 0x03}, IDR})
    );
    database.add<AnnexBToAvccTest>(
        "Several NALs, 3-byte separators",
        concat({START4, AUD, START3, SPS, START3, PPS, START3, IDR}),
        true, concat({
            {0x00, 0x00, 0x00, 0x02}, AUD,
            {0x00, 0x00, 0x00, 0x07}, SPS,
            {0x00, 0x00, 0x00, 0x06}, PPS,
            {0x00, 0x00, 0x00, 0x03}, IDR,
        })
    );
    //  A NAL unit never ends in a zero byte, so the zero in front of a
    //  following 4-byte start code is part of the start code, not the NAL.
    database.add<AnnexBToAvccTest>(
        "Several NALs, 4-byte separators",
        concat({START4, AUD, START4, SPS, START4, PPS, START4, IDR}),
        true, concat({
            {0x00, 0x00, 0x00, 0x02}, AUD,
            {0x00, 0x00, 0x00, 0x07}, SPS,
            {0x00, 0x00, 0x00, 0x06}, PPS,
            {0x00, 0x00, 0x00, 0x03}, IDR,
        })
    );
    database.add<AnnexBToAvccTest>(
        "Bytes before the first start code are skipped",
        concat({{0xab, 0xcd}, START3, IDR}),
        true, concat({{0x00, 0x00, 0x00, 0x03}, IDR})
    );
    database.add<AnnexBToAvccTest>(
        "No start code",
        Bytes{0x12, 0x34, 0x56},
        false, Bytes{}
    );
    database.add<AnnexBToAvccTest>(
        "Empty input",
        Bytes{},
        false, Bytes{}
    );
    database.add<AnnexBToAvccTest>(
        "Start code with no payload",
        START4,
        false, Bytes{}
    );
    //  An empty NAL unit (two start codes in a row) is skipped; the NAL
    //  units after it are still converted.
    database.add<AnnexBToAvccTest>(
        "Empty NAL between 3-byte start codes",
        concat({START3, START3, AUD, START3, IDR}),
        true, concat({
            {0x00, 0x00, 0x00, 0x02}, AUD,
            {0x00, 0x00, 0x00, 0x03}, IDR,
        })
    );
    database.add<AnnexBToAvccTest>(
        "3-byte start code right before a 4-byte one",
        concat({START3, START4, AUD, START4, IDR}),
        true, concat({
            {0x00, 0x00, 0x00, 0x02}, AUD,
            {0x00, 0x00, 0x00, 0x03}, IDR,
        })
    );
    database.add<AnnexBToAvccTest>(
        "Trailing zero padding",
        concat({START4, IDR, {0x00, 0x00}}),
        true, concat({{0x00, 0x00, 0x00, 0x03}, IDR})
    );

    //  avcC configuration record from Annex B extradata.
    database.add<AvccFromExtradataTest>(
        "SPS + PPS, 3-byte start codes",
        concat({START3, SPS, START3, PPS}),
        true, SPS_PPS_AVCC
    );
    database.add<AvccFromExtradataTest>(
        "SPS + PPS, 4-byte start codes",
        concat({START4, SPS, START4, PPS}),
        true, SPS_PPS_AVCC
    );
    database.add<AvccFromExtradataTest>(
        "Other NALs and repeated parameter sets are ignored",
        concat({
            START3, SEI,
            START3, SPS,
            START3, PPS,
            START3, Bytes{0x67, 0x4d, 0x00, 0x2a, 0x99},
            START3, Bytes{0x68, 0x01},
        }),
        true, SPS_PPS_AVCC
    );
    database.add<AvccFromExtradataTest>(
        "SPS without PPS",
        concat({START3, SPS}),
        true, concat({
            {0x01, 0x64, 0x00, 0x1f, 0xff, 0xe1, 0x00, 0x07},
            SPS,
            {0x00},     //  No picture parameter sets.
            SPS_AVCC_EXTENSION,
        })
    );
    database.add<AvccFromExtradataTest>(
        "Empty NAL before the SPS",
        concat({START3, START4, SPS, START4, PPS}),
        true, SPS_PPS_AVCC
    );
    database.add<AvccFromExtradataTest>(
        "Main profile has no extension",
        concat({START4, SPS_MAIN, START4, PPS}),
        true, concat({
            {0x01, 0x4d, 0x40, 0x1f, 0xff, 0xe1, 0x00, 0x06},
            SPS_MAIN,
            {0x01, 0x00, 0x06},
            PPS,
        })
    );
    database.add<AvccFromExtradataTest>(
        "High 4:2:2 10-bit extension",
        concat({START4, SPS_HIGH_422_10BIT, START4, PPS}),
        true, concat({
            {0x01, 0x7a, 0x00, 0x1f, 0xff, 0xe1, 0x00, 0x06},
            SPS_HIGH_422_10BIT,
            {0x01, 0x00, 0x06},
            PPS,
            {0xfe, 0xfa, 0xfa, 0x00},
        })
    );
    database.add<AvccFromExtradataTest>(
        "High 4:4:4 extension skips separate_colour_plane_flag",
        concat({START4, SPS_HIGH_444, START4, PPS}),
        true, concat({
            {0x01, 0xf4, 0x00, 0x1f, 0xff, 0xe1, 0x00, 0x06},
            SPS_HIGH_444,
            {0x01, 0x00, 0x06},
            PPS,
            {0xff, 0xf8, 0xf9, 0x00},
        })
    );
    database.add<AvccFromExtradataTest>(
        "Emulation prevention byte in the SPS",
        concat({START4, SPS_EMULATION_PREVENTION, START4, PPS}),
        true, concat({
            {0x01, 0x64, 0x00, 0x00, 0xff, 0xe1, 0x00, 0x08},
            SPS_EMULATION_PREVENTION,
            {0x01, 0x00, 0x06},
            PPS,
            SPS_AVCC_EXTENSION,
        })
    );
    database.add<AvccFromExtradataTest>(
        "libx264 High profile, same as ffmpeg's avcC",
        concat({START4, X264_SPS, START4, X264_PPS}),
        true, X264_FFMPEG_AVCC
    );
    database.add<AvccFromExtradataTest>(
        "High profile SPS without the chroma format",
        concat({START4, Bytes{0x67, 0x64, 0x00, 0x1f}, START4, PPS}),
        false, Bytes{}
    );
    database.add<AvccFromExtradataTest>(
        "No SPS",
        concat({START3, PPS}),
        false, Bytes{}
    );
    database.add<AvccFromExtradataTest>(
        "SPS too short",
        concat({START3, Bytes{0x67, 0x64, 0x00}, START3, PPS}),
        false, Bytes{}
    );
    database.add<AvccFromExtradataTest>(
        "Empty input",
        Bytes{},
        false, Bytes{}
    );

#if defined(PA_STREAM_HISTORY_FFMPEG)
    //  StreamHistoryTracker_Ring, end to end.
    database.add<RingTest>("Save MP4", [](Logger& logger){
        return test_save(logger, "Save.mp4", FeedOptions());
    });
    database.add<RingTest>("Save MKV", [](Logger& logger){
        return test_save(logger, "Save.mkv", FeedOptions());
    });
    database.add<RingTest>("Save MP4 from NV12 frames", [](Logger& logger){
        FeedOptions options;
        options.frame_format = FrameFormat::NV12;
        return test_save(logger, "SaveNV12.mp4", options);
    });
    database.add<RingTest>("Save video only", [](Logger& logger){
        FeedOptions options;
        options.audio = false;
        return test_save(logger, "SaveVideoOnly.mp4", options);
    });
    database.add<RingTest>("Save audio only", [](Logger& logger){
        FeedOptions options;
        options.video = false;
        return test_save(logger, "SaveAudioOnly.mp4", options);
    });
    database.add<RingTest>("Resolution change while recording", [](Logger& logger){
        //  The scaler is rebuilt for the new input size; the encoder keeps
        //  its size.
        FeedOptions options;
        options.resize_ms = 2500;
        return test_save(logger, "ResolutionChange.mp4", options);
    });
    database.add<RingTest>("Resolution change while recording, NV12", [](Logger& logger){
        FeedOptions options;
        options.frame_format = FrameFormat::NV12;
        options.resize_ms = 2500;
        return test_save(logger, "ResolutionChangeNV12.mp4", options);
    });
    database.add<RingTest>("Eviction", [](Logger& logger){ return test_eviction(logger); });
    database.add<RingTest>("Shrink the window while recording", [](Logger& logger){ return test_shrink_window(logger); });
    database.add<RingTest>("Video stops while the audio continues", [](Logger& logger){ return test_video_stops(logger); });
    database.add<RingTest>("Frames faster than the target FPS", [](Logger& logger){ return test_throttle(logger); });
    database.add<RingTest>("Sync MP4", [](Logger& logger){
        return test_sync(logger, "Sync.mp4", sync_options(), 5000, SYNC_TOLERANCE_S, true);
    });
    database.add<RingTest>("Sync MKV", [](Logger& logger){
        return test_sync(logger, "Sync.mkv", sync_options(), 5000, SYNC_TOLERANCE_S, true);
    });
    database.add<RingTest>("Sync MP4 from NV12 frames", [](Logger& logger){
        FeedOptions options = sync_options();
        options.frame_format = FrameFormat::NV12;
        return test_sync(logger, "SyncNV12.mp4", options, 5000, SYNC_TOLERANCE_S, true);
    });
    database.add<RingTest>("Sync after an audio gap", [](Logger& logger){
        //  A second of audio never arrives (e.g. a device underrun). The
        //  audio after the gap must still line up with the video.
        FeedOptions options = sync_options();
        options.gap_begin_ms = 1500;
        options.gap_end_ms = 2500;
        return test_sync(logger, "SyncAudioGap.mp4", options, 5000, SYNC_TOLERANCE_S, false);
    });
    database.add<RingTest>("Sync with audio delivery jitter", [](Logger& logger){
        //  Audio arriving 0 - 50 ms late must not trigger a re-sync. Audio is
        //  stamped by arrival, so the first chunk's latency offsets it.
        FeedOptions options = sync_options();
        options.audio_jitter_ms = 50;
        return test_sync(logger, "SyncJitter.mp4", options, 5000, SYNC_TOLERANCE_S + 0.05, true);
    });
    database.add<RingTest>("Sync with a fast audio clock", [](Logger& logger){
        //  A device clock 0.1% fast drifts 100 ms in 100 s. By 190 s the
        //  audio would be 190 ms off without the re-sync.
        FeedOptions options;
        options.onset_ms = 190000;
        options.audio_clock_scale = 1.001;
        return test_sync(
            logger, "SyncFastClock.mp4", options, 200000,
            AUDIO_RESYNC_THRESHOLD_S + SYNC_TOLERANCE_S, false
        );
    });
    database.add<RingTest>("Sync across a backwards audio re-sync", [](Logger& logger){
        //  With the device clock 0.1% fast, the re-sync at about 100 s moves
        //  the audio timeline 100 ms back, overlapping audio already in the
        //  ring. The saved window (from about 80 s) includes it: the overlap
        //  must be dropped, not squeezed into near-zero-length packets.
        FeedOptions options;
        options.onset_ms = 105000;
        options.audio_clock_scale = 1.001;
        return test_sync(
            logger, "SyncBackwardsResync.mp4", options, 110000,
            SYNC_TOLERANCE_S, false
        );
    });
    database.add<RingTest>("Sync with a slow audio clock", [](Logger& logger){
        FeedOptions options;
        options.onset_ms = 190000;
        options.audio_clock_scale = 0.999;
        return test_sync(
            logger, "SyncSlowClock.mp4", options, 200000,
            AUDIO_RESYNC_THRESHOLD_S + SYNC_TOLERANCE_S, false
        );
    });
    database.add<RingTest>("Sync with padded channel buffers", [](Logger& logger){
        //  The audio pipeline may hand over channel buffers longer than the
        //  samples delivered. Reading past the samples would put the
        //  full-scale padding into the audio long before the onset.
        FeedOptions options = sync_options();
        options.audio_stride_padding = 37;
        return test_sync(logger, "SyncPaddedChannels.mp4", options, 5000, SYNC_TOLERANCE_S, true);
    });
    database.add<RingTest>("MP4 and MKV of the same ring match", [](Logger& logger){ return test_mp4_matches_mkv(logger); });
    database.add<RingTest>("Nothing recorded", [](Logger& logger){ return test_nothing_recorded(logger); });
    database.add<RingTest>("Unwritable path", [](Logger& logger){ return test_unwritable_path(logger); });
    database.add<RingTest>("Shutdown with queued frames", [](Logger& logger){ return test_shutdown(logger); });
    database.add<RingTest>("Save while recording", [](Logger& logger){ return test_save_while_recording(logger); });
#endif
}



}
