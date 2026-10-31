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
#include "Kernels/AudioPipeline/Kernels_AudioPipeline.h"
#include "CommonFramework/AudioPipeline/AudioSession.h"
#include "CommonFramework/VideoPipeline/VideoSession.h"

namespace PokemonAutomation{


class StreamHistorySession
    : public Kernels::AudioPipeline::WorkingAudioStreamListener
    , public VideoFrameListener
    , public AudioSession::StateListener
    , public VideoSession::StateListener
{
public:
    ~StreamHistorySession();
    StreamHistorySession(Logger& logger);
    //  Starts recording (if no tracker exists yet) and attaches to the audio
    //  session's working stream. "choice" is the resolved input choice of the
    //  audio session's active input (invalid when there is no device input,
    //  i.e. file input or no input at all): with the ffmpeg ring it only
    //  decides whether audio is recorded; otherwise it also sets the shape
    //  the tracker records and the working stream is attached at.
    void start(const AudioInputChoice& choice, bool has_video);
    bool save(const std::string& filename) const;

    //  The audio session this session's working-stream listener attaches to
    //  (set before start(); used to re-attach on input changes).
    void set_audio_session(AudioSession& session);

public:
    //  Working stream, delivered in the shape requested at attach time
    //  (48 kHz stereo for the ffmpeg ring, the active preset's physical
    //  shape otherwise), split (planar) channels.
    virtual void push_samples(size_t samples, size_t stride, const float* data) override;
    virtual void on_frame(std::shared_ptr<const VideoFrame> frame) override;

public:
    virtual void pre_input_change() override;
    virtual void post_input_change(
        const std::shared_ptr<const AudioSourceDescriptor>& source,
        const AudioInputChoice& choice
    ) override;

    virtual void pre_shutdown() override;
    virtual void post_shutdown() override;
    virtual void post_startup(VideoSource* source) override;
    virtual void pre_resolution_change(Resolution resolution) override;
    virtual void post_resolution_change(Resolution resolution) override;

private:
    //  Drop the current tracker / build a new one for the current settings.
    //  Must be called without the lock: building and destroying a tracker
    //  opens encoders and starts or joins threads, and the audio and video
    //  threads spin on the lock (push_samples(), on_frame()) meanwhile.
    void clear();
    void initialize();

    //  Attaches this session to the audio session's working stream
    //  (removing any prior attach first): at a fixed 48 kHz stereo for the
    //  ffmpeg ring, at the active preset's physical shape otherwise.
    //  Must be called with no locks held.
    void attach_working_listener();

private:
    struct Data;
    Pimpl<Data> m_data;
};



}
#endif
