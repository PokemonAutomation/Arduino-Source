/*  Stream History Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Capture the last X seconds of audio and video.
 *
 */

#ifndef PokemonAutomation_StreamHistorySession_H
#define PokemonAutomation_StreamHistorySession_H

#include "Common/Cpp/AbstractLogger.h"
#include "Common/Cpp/Containers/Pimpl.h"
#include "CommonFramework/AudioPipeline/AudioSession.h"
#include "CommonFramework/VideoPipeline/VideoSession.h"

namespace PokemonAutomation{


class StreamHistorySession
    : public AudioFloatStreamListener
    , public VideoFrameListener
    , public AudioSession::StateListener
    , public VideoSession::StateListener
{
public:
    ~StreamHistorySession();
    StreamHistorySession(Logger& logger);
    void start(AudioChannelFormat format, bool has_video);
    bool save(const std::string& filename) const;

public:
    virtual void on_samples(const float* data, size_t frames) override;
    virtual void on_frame(std::shared_ptr<const VideoFrame> frame) override;

public:
    virtual void pre_input_change() override;
    virtual void post_input_change(const std::string& file, const AudioDeviceInfo& device, AudioChannelFormat format) override;

    virtual void pre_shutdown() override;
    virtual void post_shutdown() override;
    virtual void post_startup(VideoSource* source) override;
    virtual void pre_resolution_change(Resolution resolution) override;
    virtual void post_resolution_change(Resolution resolution) override;

private:
    void clear();
    void initialize();

private:
    struct Data;
    Pimpl<Data> m_data;
};



}
#endif
