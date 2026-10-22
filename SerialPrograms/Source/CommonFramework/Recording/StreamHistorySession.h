/*  Stream History Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Capture the last X seconds of audio and video.
 *
 */

#ifndef PokemonAutomation_StreamHistorySession_H
#define PokemonAutomation_StreamHistorySession_H

#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/Containers/Pimpl.h"
#include "CommonFramework/AudioPipeline/AudioSession.h"
#include "CommonFramework/VideoPipeline/VideoSession.h"
#include "Kernels/AudioPipeline/Kernels_AudioPipeline.h"

namespace PokemonAutomation{


class StreamHistorySession
    : public AudioFloatStreamListener
    , public Kernels::AudioPipeline::WorkingAudioStreamListener
    , public VideoFrameListener
    , public AudioSession::StateListener
    , public VideoSession::StateListener
{
public:
    ~StreamHistorySession();
    StreamHistorySession(Logger& logger);
    void start(AudioChannelFormat format, bool has_video);
    bool save(const std::string& filename) const;

    //  The audio session this session's working-stream listener attaches to
    //  (set before start(); used to re-attach on input changes).
    void set_audio_session(AudioSession& session);

public:
    //  Legacy float stream (kept until the remaining float consumers
    //  migrate; see the audio pipeline hand-off doc).
    virtual void on_samples(const float* data, size_t frames) override;
    //  Working stream, delivered in the shape requested at attach time
    //  (the active preset's physical shape), split (planar) channels.
    virtual void push_samples(size_t samples, size_t stride, const float* data) override;
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

    //  Attaches this session to the audio session's working stream at the
    //  active preset's physical shape (removing any prior attach first).
    //  Must be called with no locks held.
    void attach_working_listener();

private:
    struct Data;
    Pimpl<Data> m_data;
};



}
#endif
