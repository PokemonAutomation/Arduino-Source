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
#include <functional>
#include <memory>
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

//  MSVC's <cerrno> may not define EAGAIN (see StreamHistoryTracker_Ring.cpp).
#ifndef EAGAIN
#define EAGAIN 11
#endif
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

//  The avcC record for SPS + PPS: version 1, profile/compatibility/level
//  from SPS bytes 1-3, 4-byte NAL lengths, one SPS, one PPS.
const Bytes SPS_PPS_AVCC = concat({
    {0x01, 0x64, 0x00, 0x1f, 0xff, 0xe1, 0x00, 0x07},
    SPS,
    {0x01, 0x00, 0x06},
    PPS,
});



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
constexpr size_t CHUNK_SAMPLES = (size_t)(AUDIO_RATE * CHUNK_MS / 1000);
constexpr int FRAME_WIDTH = 320;
constexpr int FRAME_HEIGHT = 180;

//  Allowed distance between the start of the tone and the first white frame
//  in a saved file. Covers the AAC frame granularity and the millisecond
//  rounding of the MKV timestamps.
constexpr double SYNC_TOLERANCE_S = 0.03;

//  Packets an encoder may still be holding when save() runs (the hardware
//  encoders pipeline several frames); those are not in the file.
constexpr double ENCODER_BACKLOG_S = 0.5;


//  The interval between the frames the tracker keeps: every fed frame
//  (FRAME_INTERVAL_MS) unless the configured FPS throttles below that.
double kept_frame_interval_s(){
    const StreamHistoryOption& settings = GlobalSettings::instance().STREAM_HISTORY;
    double fps = 10;
    switch (settings.VIDEO_FPS){
    case StreamHistoryOption::VideoFPS::FPS_30: fps = 30; break;
    case StreamHistoryOption::VideoFPS::FPS_15: fps = 15; break;
    case StreamHistoryOption::VideoFPS::FPS_10: fps = 10; break;
    case StreamHistoryOption::VideoFPS::FPS_05: fps = 5; break;
    case StreamHistoryOption::VideoFPS::FPS_01: fps = 1; break;
    }
    return std::max(FRAME_INTERVAL_MS / 1000., 1 / fps);
}

std::string output_path(const std::string& name){
    std::string directory = DEBUG_PATH() + "StreamHistoryTests/";
    Filesystem::create_directories(directory);
    return directory + name;
}



//  Feeds a tracker in 10 ms steps of simulated time: a frame every 100 ms
//  and a 10 ms chunk of 48 kHz stereo audio per step (stamped with the
//  time of its last sample). From "onset_ms" on, the frames are white
//  instead of black and the audio is a 1 kHz tone instead of silence.
struct FeedOptions{
    bool video = true;
    bool audio = true;
    int64_t onset_ms = -1;
    //  No audio is delivered in [gap_begin_ms, gap_end_ms).
    int64_t gap_begin_ms = -1;
    int64_t gap_end_ms = -1;
    //  Wait for the encoder after each frame (otherwise frames just queue).
    bool wait_for_encoder = true;
};

class Feeder{
public:
    Feeder(StreamHistoryTracker_Ring& tracker, FeedOptions options)
        : m_tracker(tracker)
        , m_options(options)
        , m_start(current_time())
        , m_black(FRAME_WIDTH, FRAME_HEIGHT, QImage::Format_RGB32)
        , m_white(FRAME_WIDTH, FRAME_HEIGHT, QImage::Format_RGB32)
        , m_audio(2 * CHUNK_SAMPLES)
    {
        m_black.fill(Qt::black);
        m_white.fill(Qt::white);
    }

    void run_until(int64_t end_ms){
        for (; m_time_ms < end_ms; m_time_ms += CHUNK_MS){
            bool onset = m_options.onset_ms >= 0 && m_time_ms >= m_options.onset_ms;
            if (m_options.video && m_time_ms % FRAME_INTERVAL_MS == 0){
                m_tracker.on_frame(std::make_shared<const VideoFrame>(
                    m_start + std::chrono::milliseconds(m_time_ms),
                    QVideoFrame(onset ? m_white : m_black)
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
    void push_audio_chunk(){
        int64_t first_sample = m_time_ms * AUDIO_RATE / 1000;
        int64_t onset_sample = m_options.onset_ms * AUDIO_RATE / 1000;
        for (size_t i = 0; i < CHUNK_SAMPLES; i++){
            int64_t sample = first_sample + (int64_t)i;
            float value = 0;
            if (m_options.onset_ms >= 0 && sample >= onset_sample){
                value = (float)(0.5 * std::sin(2 * 3.14159265358979 * 1000 * (double)sample / AUDIO_RATE));
            }
            m_audio[i] = value;                    //  Left.
            m_audio[CHUNK_SAMPLES + i] = value;    //  Right.
        }
        m_tracker.push_samples(
            CHUNK_SAMPLES, CHUNK_SAMPLES, m_audio.data(),
            m_start + std::chrono::milliseconds(m_time_ms + CHUNK_MS)
        );
    }

private:
    StreamHistoryTracker_Ring& m_tracker;
    FeedOptions m_options;
    WallClock m_start;
    int64_t m_time_ms = 0;
    QImage m_black;
    QImage m_white;
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
    size_t video_frames = 0;
    double video_onset_s = -1;      //  First decoded frame that is no longer black.

    bool audio_timestamps_increasing = true;
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
            if (ts <= last_audio_ts){
                file.audio_timestamps_increasing = false;
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



std::string test_save(Logger& logger, const std::string& filename, bool video, bool audio){
    StreamHistoryTracker_Ring tracker(logger, std::chrono::seconds(30), audio, video);
    FeedOptions options;
    options.video = video;
    options.audio = audio;
    Feeder feeder(tracker, options);
    feeder.run_until(5000);

    ProbedFile file;
    std::string error = save_and_probe(tracker, filename, video, audio, file);
    if (!error.empty() || !video){
        return error;
    }
    //  Nothing is evicted: the file runs from the first frame (0 s) to the
    //  last one kept (4.9 s, or earlier when the FPS setting throttles).
    double last_frame_s = 4.9;
    return check_range(
        "The video length",
        file.video_last_s - file.video_first_s,
        last_frame_s - kept_frame_interval_s() - ENCODER_BACKLOG_S, last_frame_s + 0.01
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
    //  The file starts at a keyframe at most one keyframe interval (2 s) plus
    //  one frame before the window, and covers the window up to the rounding
    //  of the keyframe spacing to whole frames.
    double interval = kept_frame_interval_s();
    return check_range(
        "The video length",
        file.video_last_s - file.video_first_s,
        WINDOW_S - 2 * interval - ENCODER_BACKLOG_S, WINDOW_S + 2.0 + interval + 0.01
    );
}

std::string test_sync(Logger& logger, const std::string& filename, bool audio_gap){
    StreamHistoryTracker_Ring tracker(logger, std::chrono::seconds(30), true, true);
    FeedOptions options;
    options.onset_ms = 3000;
    if (audio_gap){
        //  A second of audio never arrives (e.g. a device underrun). The audio
        //  after the gap must still line up with the video.
        options.gap_begin_ms = 1500;
        options.gap_end_ms = 2500;
    }
    Feeder feeder(tracker, options);
    feeder.run_until(5000);

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
    return check_range(
        "The audio onset minus the video onset",
        file.audio_onset_s - file.video_onset_s,
        -SYNC_TOLERANCE_S, SYNC_TOLERANCE_S
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
        })
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
    database.add<RingTest>("Save MP4", [](Logger& logger){ return test_save(logger, "Save.mp4", true, true); });
    database.add<RingTest>("Save MKV", [](Logger& logger){ return test_save(logger, "Save.mkv", true, true); });
    database.add<RingTest>("Save video only", [](Logger& logger){ return test_save(logger, "SaveVideoOnly.mp4", true, false); });
    database.add<RingTest>("Save audio only", [](Logger& logger){ return test_save(logger, "SaveAudioOnly.mp4", false, true); });
    database.add<RingTest>("Eviction", [](Logger& logger){ return test_eviction(logger); });
    database.add<RingTest>("Sync MP4", [](Logger& logger){ return test_sync(logger, "Sync.mp4", false); });
    database.add<RingTest>("Sync MKV", [](Logger& logger){ return test_sync(logger, "Sync.mkv", false); });
    database.add<RingTest>("Sync after an audio gap", [](Logger& logger){ return test_sync(logger, "SyncAudioGap.mp4", true); });
    database.add<RingTest>("Nothing recorded", [](Logger& logger){ return test_nothing_recorded(logger); });
    database.add<RingTest>("Unwritable path", [](Logger& logger){ return test_unwritable_path(logger); });
    database.add<RingTest>("Shutdown with queued frames", [](Logger& logger){ return test_shutdown(logger); });
    database.add<RingTest>("Save while recording", [](Logger& logger){ return test_save_while_recording(logger); });
#endif
}



}
