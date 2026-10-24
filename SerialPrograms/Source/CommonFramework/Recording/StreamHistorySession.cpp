/*  Stream History Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <vector>
#include <QAudioFormat>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/Pimpl.tpp"   //  Pimpl<QAudioFormat> (AudioFormat) instantiations
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/AudioPipeline/AudioInfo.h"
#include "CommonFramework/VideoPipeline/Backends/VideoFrameQt.h"
#include "CommonFramework/Recording/StreamHistoryOption.h"

#if (QT_VERSION_MAJOR == 6) && (QT_VERSION_MINOR >= 8)
#include "StreamHistoryTracker_SaveFrames.h"
//#include "StreamHistoryTracker_RecordOnTheFly.h"
//#include "StreamHistoryTracker_ParallelStreams.h"
#else
#include "StreamHistoryTracker_Null.h"
#endif


#include "StreamHistorySession.h"



namespace PokemonAutomation{





struct StreamHistorySession::Data{
    Logger& m_logger;
    mutable SpinLock m_lock;
    std::chrono::seconds m_window;
    //  The resolved input choice for the active device input (invalid
    //  when there is no device input). Drives both the tracker's shape
    //  and the working-stream attach shape.
    AudioInputChoice m_audio_choice;
    bool m_has_video;
    //  The active input's physical working-stream shape (what the tree
    //  delivers through the working listener, and what the tracker records
    //  at). Set in initialize().
    size_t m_active_channels = 0;
    size_t m_active_rate = 0;
    std::vector<float> m_interleaved;
    //  The audio session the working listener attaches to (set_audio_session).
    AudioSession* m_audio_session = nullptr;
    std::shared_ptr<StreamHistoryTracker> m_current;

    Data(Logger& logger)
        : m_logger(logger)
        , m_window(GlobalSettings::instance().STREAM_HISTORY->HISTORY_SECONDS)
        , m_has_video(false)
    {}
};




StreamHistorySession::~StreamHistorySession() = default;
StreamHistorySession::StreamHistorySession(Logger& logger)
    //  The working-stream nominal shape is metadata only: the pair's
    //  per-listener forwarder carries the shape actually requested, and
    //  nothing validates this one.
    : Kernels::AudioPipeline::WorkingAudioStreamListener(48000, 2)
    , m_data(CONSTRUCT_TOKEN, logger)
{}
void StreamHistorySession::start(
    const AudioInputChoice& choice, bool has_video
){
    Data& data = *m_data;
    {
        WriteSpinLock lg(data.m_lock);
        if (!data.m_current){
            data.m_audio_choice = choice;
            data.m_has_video = has_video;
            initialize();
        }
    }
    attach_working_listener();
}
void StreamHistorySession::set_audio_session(AudioSession& session){
    Data& data = *m_data;
    WriteSpinLock lg(data.m_lock);
    data.m_audio_session = &session;
}



bool StreamHistorySession::save(const std::string& filename) const{
    //  This will be coming in from random threads. It will block until the save
    //  is finished or failed.

    const Data& data = *m_data;

    //  Get an owning reference to the current tracker.
    //  This will allow us to promptly release the lock and unblock the UI from
    //  changing the streams (which will wipe the history).
    std::shared_ptr<StreamHistoryTracker> tracker;
    {
        WriteSpinLock lg(data.m_lock);
        if (!data.m_current){
            data.m_logger.log("Cannot save stream history: Stream history is not enabled.", COLOR_RED);
            return false;
        }
        tracker = data.m_current;
    }

//    tracker->save(m_logger, filename);
//    HistorySaverThread saver(*tracker, filename);
//    return saver.save();

    return tracker->save(filename);
}
void StreamHistorySession::push_samples(size_t samples, size_t stride, const float* data){
    Data& state = *m_data;
    WriteSpinLock lg(state.m_lock);
    if (!state.m_current || samples == 0 || state.m_active_channels == 0){
        return;
    }
    //  The working stream is split (planar) channels: channel "c" starts at
    //  "data[c * stride]" and runs for "samples" samples. (stride may be 0
    //  for the single-channel pass-through.) Re-interleave for the tracker.
    //  The capture card's RL channel swap (INTERLEAVE_RL_96000) is already
    //  applied by the pair's conversion node matrix, the same way as for
    //  the sink branch.
    const size_t channels = state.m_active_channels;
    state.m_interleaved.resize(samples * channels);
    for (size_t i = 0; i < samples; i++){
        for (size_t c = 0; c < channels; c++){
            state.m_interleaved[i * channels + c] = data[c * stride + i];
        }
    }
    state.m_current->on_samples(state.m_interleaved.data(), samples);
}
void StreamHistorySession::on_frame(std::shared_ptr<const VideoFrame> frame){
    Data& data = *m_data;
    WriteSpinLock lg(data.m_lock);
    if (data.m_current){
        data.m_current->on_frame(std::move(frame));
    }
}


void StreamHistorySession::clear(){
//    cout << "clear()" << endl;

    //  Must call under lock.
    Data& data = *m_data;
    data.m_logger.log("Clearing stream history...", COLOR_ORANGE);
    data.m_current.reset();
}
void StreamHistorySession::initialize(){
    if (!GlobalSettings::instance().STREAM_HISTORY->enabled()){
        return;
    }

    //  Must call under lock.
    Data& data = *m_data;
    data.m_logger.log("Starting stream history...", COLOR_ORANGE);

//    cout << "video = " << data.m_has_video << endl;

    //  The tracker's audio shape is the input source's working-stream
    //  shape (what the tree delivers through the working listener), i.e.
    //  the resolved input choice's logical shape: the preset's framing,
    //  with the capture-card 96 kHz reinterpretation applied for the
    //  reinterpreted choices. No choice (file input or no input) records
    //  at no shape.
    const AudioInputChoice& choice = data.m_audio_choice;
    const size_t channels = choice ? choice.format().channels() : 0;
    const size_t rate = choice ? choice.format().sample_rate() : 0;
    data.m_active_channels = channels;
    data.m_active_rate = rate;
    data.m_current.reset(new StreamHistoryTracker(data.m_logger, data.m_window, channels, rate, data.m_has_video));
}
void StreamHistorySession::pre_input_change(){
//    cout << "pre_input_change()" << endl;
    Data& data = *m_data;
    WriteSpinLock lg(data.m_lock);
    clear();
}
void StreamHistorySession::post_input_change(const std::string& file, const AudioDeviceInfo& device, const AudioInputChoice& choice){
//    cout << "post_input_change()" << endl;
    Data& data = *m_data;
    AudioSession* audio_session;
    {
        WriteSpinLock lg(data.m_lock);
        audio_session = data.m_audio_session;
    }
    //  Detach the working listener first: after this returns, no pushes in
    //  the old shape can reach push_samples() (the pair quiesces in-flight
    //  pushes). Then update the shapes, then attach for the new shape.
    if (audio_session){
        audio_session->remove_working_stream_listener(*this);
    }
    {
        WriteSpinLock lg(data.m_lock);
        data.m_audio_choice = choice;
        initialize();
    }
    attach_working_listener();
}
void StreamHistorySession::attach_working_listener(){
    Data& data = *m_data;
    AudioSession* audio_session;
    AudioInputChoice target;
    {
        WriteSpinLock lg(data.m_lock);
        audio_session = data.m_audio_session;
        target = data.m_audio_choice;
    }
    if (!audio_session || !target){
        return;
    }
    //  The remove quiesces any in-flight pushes in the old shape (and is a
    //  no-op if not attached yet); the node is then built for the new one.
    audio_session->remove_working_stream_listener(*this);
    audio_session->add_working_stream_listener(
        AudioFormat(target.format().native_format()), *this
    );
}
void StreamHistorySession::pre_shutdown(){
//    cout << "pre_shutdown()" << endl;
    Data& data = *m_data;
    WriteSpinLock lg(data.m_lock);
    clear();
}
void StreamHistorySession::post_shutdown(){
//    cout << "post_shutdown()" << endl;
    Data& data = *m_data;
    WriteSpinLock lg(data.m_lock);
    data.m_has_video = false;
    initialize();
}
void StreamHistorySession::post_startup(VideoSource* source){
    Data& data = *m_data;
    WriteSpinLock lg(data.m_lock);
    data.m_has_video = source != nullptr;
    initialize();
}
void StreamHistorySession::pre_resolution_change(Resolution resolution){
//    cout << "pre_resolution_change()" << endl;
    Data& data = *m_data;
    WriteSpinLock lg(data.m_lock);
    clear();
}
void StreamHistorySession::post_resolution_change(Resolution resolution){
//    cout << "post_resolution_change()" << endl;
    Data& data = *m_data;
    WriteSpinLock lg(data.m_lock);
    initialize();
}







}
