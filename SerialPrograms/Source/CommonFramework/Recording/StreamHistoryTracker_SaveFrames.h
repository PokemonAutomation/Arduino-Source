/*  Stream History Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Implement by saving the last X seconds of frames. This is currently not
 *  viable because the QVideoFrames are uncompressed.
 *
 */

#ifndef PokemonAutomation_StreamHistoryTracker_SaveFrames_H
#define PokemonAutomation_StreamHistoryTracker_SaveFrames_H

// #include <opencv2/opencv.hpp>
#include <deque>
// #include <QCoreApplication>
// #include <QFileInfo>
// #include <QUrl>
// #include <QAudioBufferInput>
// #include <QVideoFrameInput>
// #include <QMediaFormat>
// #include <QMediaRecorder>
// #include <QMediaCaptureSession>
// #include <QScopeGuard>
// #include "Common/Cpp/Logging/AbstractLogger.h"
// #include "Common/Cpp/Concurrency/SpinLock.h"
// #include "CommonFramework/VideoPipeline/Backends/VideoFrameQt.h"


// #include <iostream>
// using std::cout;
// using std::endl;


namespace PokemonAutomation{


struct AudioBlock{
    WallClock timestamp;
    std::vector<float> samples;

    AudioBlock(const AudioBlock&) = delete;
    void operator=(const AudioBlock&) = delete;

    AudioBlock(WallClock p_timestamp, const float* p_samples, size_t p_count)
        : timestamp(p_timestamp)
        , samples(p_samples, p_samples + p_count)
    {}
};

struct CompressedVideoFrame{
    WallClock timestamp;
    std::vector<unsigned char> compressed_frame;
};

QImage decompress_video_frame(const std::vector<uchar> &compressed_buffer);
std::vector<uchar> compress_video_frame(const QVideoFrame& const_frame);

class StreamHistoryTracker{
public:
    StreamHistoryTracker(
        Logger& logger,
        std::chrono::seconds window,
        size_t audio_samples_per_frame,
        size_t audio_frames_per_second,
        bool has_video
    );
    void set_window(std::chrono::seconds window);

    bool save(const std::string& filename) const;

public:
    void on_samples(const float* data, size_t frames);
    void on_frame(std::shared_ptr<const VideoFrame> frame);

private:
    void clear_old();

private:
    Logger& m_logger;
    mutable SpinLock m_lock;
    std::chrono::seconds m_window;

    const size_t m_audio_samples_per_frame;
    const size_t m_audio_frames_per_second;
    const size_t m_audio_samples_per_second;
    const double m_microseconds_per_sample;
    const bool m_has_video;

    //  We use shared_ptr here so it's fast to snapshot when we need to copy
    //  everything asynchronously.
    std::deque<std::shared_ptr<AudioBlock>> m_audio;
    std::deque<std::shared_ptr<const VideoFrame>> m_frames;
    std::deque<CompressedVideoFrame> m_compressed_frames;
};






}
#endif
