/*  Recent History
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *  Capture the last X seconds of audio and video.
 *
 */

#ifndef PokemonAutomation_RecentHistory_H
#define PokemonAutomation_RecentHistory_H

#include <chrono>
#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Containers/Pimpl.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/AudioPipeline/AudioSession.h"
#include "CommonFramework/VideoPipeline/CameraSession.h"

namespace PokemonAutomation{


class RecentHistoryTracker{
public:
    ~RecentHistoryTracker();
    RecentHistoryTracker(
        size_t audio_samples_per_second,
        std::chrono::seconds window
    );
    void set_window(std::chrono::seconds window);

    void save(Logger& logger, const std::string& filename) const;

public:
//    void push_frame(QVideoFrame frame);

    void on_samples(const float* data, size_t frames);
    void on_frame(std::shared_ptr<VideoFrame> frame);

private:
    void clear_old();

private:
    struct Data;
    Pimpl<Data> m_data;
};


class RecentHistorySession
    : public AudioFloatStreamListener
    , public VideoFrameListener
    , public AudioSession::StateListener
    , public CameraSession::StateListener
{
public:
    RecentHistorySession(Logger& logger);
    void start();
    void save(const std::string& filename);

public:
    virtual void on_samples(const float* data, size_t frames) override;
    virtual void on_frame(std::shared_ptr<VideoFrame> frame) override;

public:
    virtual void pre_input_change() override;
    virtual void post_input_change(const std::string& file, const AudioDeviceInfo& device, AudioChannelFormat format) override;

    virtual void pre_shutdown() override;
    virtual void post_new_source(const CameraInfo& device, Resolution resolution) override;
    virtual void pre_resolution_change(Resolution resolution) override;
    virtual void post_resolution_change(Resolution resolution) override;

private:
    void clear();
    void initialize();

private:
    Logger& m_logger;
    SpinLock m_lock;
    std::chrono::seconds m_window;
    AudioChannelFormat m_audio_format;
    std::shared_ptr<RecentHistoryTracker> m_current;
};



}
#endif
