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

#include <deque>
#include "Common/Cpp/Concurrency/Mutex.h"
#include "Common/Cpp/Concurrency/ConditionVariable.h"


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
    ~StreamHistoryTracker();
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
    void worker_loop(); // The function that runs in the thread

private:
    static constexpr size_t MAX_PENDING_FRAMES = 10;
    Logger& m_logger;
    mutable SpinLock m_lock;
    std::chrono::seconds m_window;

    const size_t m_audio_samples_per_frame;
    const size_t m_audio_frames_per_second;
    const size_t m_audio_samples_per_second;
    const double m_microseconds_per_sample;
    const bool m_has_video;
    size_t m_target_fps;
    std::chrono::microseconds m_frame_interval;

    //  We use shared_ptr here so it's fast to snapshot when we need to copy
    //  everything asynchronously.
    std::deque<std::shared_ptr<AudioBlock>> m_audio;
    std::deque<std::shared_ptr<const VideoFrame>> m_frames;
    std::deque<CompressedVideoFrame> m_compressed_frames;
    WallClock m_next_frame_time;

    std::thread m_worker;
    std::atomic<bool> m_stopping{false};
    
    // Queue for the worker thread
    Mutex m_queue_lock;
    ConditionVariable m_cv;
    std::deque<std::shared_ptr<const VideoFrame>> m_pending_frames;

    

    
};






}
#endif
