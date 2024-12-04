/*  Stream History Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/VideoPipeline/Backends/VideoFrameQt.h"

#if (QT_VERSION_MAJOR == 6) && (QT_VERSION_MINOR >= 8)
#include "StreamHistoryTracker_SaveFrames.h"
//#include "StreamHistoryTracker_RecordOnTheFly.h"
#else
#include "StreamHistoryTracker_Null.h"
#endif


#include "StreamHistorySession.h"



namespace PokemonAutomation{





struct StreamHistorySession::Data{
    Logger& m_logger;
    mutable SpinLock m_lock;
    std::chrono::seconds m_window;
    AudioChannelFormat m_audio_format;
    std::shared_ptr<StreamHistoryTracker> m_current;

    Data(Logger& logger)
        : m_logger(logger)
        , m_window(GlobalSettings::instance().STREAM_HISTORY.VIDEO_HISTORY_SECONDS)
        , m_audio_format(AudioChannelFormat::NONE)
    {}
};




StreamHistorySession::StreamHistorySession(Logger& logger)
    : AudioFloatStreamListener(1)
    , m_data(CONSTRUCT_TOKEN, logger)
{}
void StreamHistorySession::start(AudioChannelFormat format){
    Data& data = *m_data;
    WriteSpinLock lg(data.m_lock);
    if (!data.m_current){
        data.m_audio_format = format;
        initialize();
    }
}


class HistorySaverThread : public QThread{
public:
    HistorySaverThread(Logger& logger, const StreamHistoryTracker& tracker, const std::string& filename)
        : m_logger(logger)
        , m_tracker(tracker)
        , m_filename(filename)
    {
        start();
    }
    ~HistorySaverThread(){
        quit();
        wait();
    }
    virtual void run() override{
        m_tracker.save(m_logger, m_filename);
    }

private:
    Logger& m_logger;
    const StreamHistoryTracker& m_tracker;
    const std::string& m_filename;
};

void StreamHistorySession::save(const std::string& filename) const{
    const Data& data = *m_data;

    //  Get an owning reference to the current tracker.
    //  This will allow us to promptly release the lock and unblock the UI from
    //  changing the streams (which will wipe the history).
    std::shared_ptr<StreamHistoryTracker> tracker;
    {
        WriteSpinLock lg(data.m_lock);
        if (!data.m_current){
            data.m_logger.log("Cannot save stream history: Stream history is not enabled.", COLOR_RED);
            return;
        }
        tracker = data.m_current;
    }

//    tracker->save(m_logger, filename);
    HistorySaverThread saver(data.m_logger, *tracker, filename);
}
void StreamHistorySession::on_samples(const float* samples, size_t frames){
    Data& data = *m_data;
    WriteSpinLock lg(data.m_lock);
    if (data.m_current){
        data.m_current->on_samples(samples, frames);
    }
}
void StreamHistorySession::on_frame(std::shared_ptr<VideoFrame> frame){
    Data& data = *m_data;
    WriteSpinLock lg(data.m_lock);
    if (data.m_current){
        data.m_current->on_frame(std::move(frame));
    }
}


void StreamHistorySession::clear(){
    //  Must call under lock.
    Data& data = *m_data;
    data.m_logger.log("Clearing stream history...", COLOR_ORANGE);
    data.m_current.reset();
    expected_samples_per_frame = 0;
    data.m_audio_format = AudioChannelFormat::NONE;
}
void StreamHistorySession::initialize(){
    if (!GlobalSettings::instance().STREAM_HISTORY.enabled()){
        return;
    }

    //  Must call under lock.
    Data& data = *m_data;
    data.m_logger.log("Starting stream history...", COLOR_ORANGE);
    switch (data.m_audio_format){
    case AudioChannelFormat::NONE:
        expected_samples_per_frame = 0;
        data.m_current.reset(new StreamHistoryTracker(0, 0, data.m_window));
        return;
    case AudioChannelFormat::MONO_48000:
        data.m_current.reset(new StreamHistoryTracker(1, 48000, data.m_window));
        return;
    case AudioChannelFormat::DUAL_44100:
        data.m_current.reset(new StreamHistoryTracker(1, 44100, data.m_window));
        return;
    case AudioChannelFormat::DUAL_48000:
    case AudioChannelFormat::MONO_96000:
    case AudioChannelFormat::INTERLEAVE_LR_96000:
    case AudioChannelFormat::INTERLEAVE_RL_96000:
        data.m_current.reset(new StreamHistoryTracker(2, 48000, data.m_window));
        return;
    default:
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Invalid AudioFormat: " + std::to_string((size_t)data.m_audio_format)
        );
    }
}
void StreamHistorySession::pre_input_change(){
    Data& data = *m_data;
    WriteSpinLock lg(data.m_lock);
    clear();
}
void StreamHistorySession::post_input_change(const std::string& file, const AudioDeviceInfo& device, AudioChannelFormat format){
    Data& data = *m_data;
    WriteSpinLock lg(data.m_lock);
    data.m_audio_format = format;
    initialize();
}
void StreamHistorySession::pre_shutdown(){
    Data& data = *m_data;
    WriteSpinLock lg(data.m_lock);
    clear();
}
void StreamHistorySession::post_new_source(const CameraInfo& device, Resolution resolution){
    Data& data = *m_data;
    WriteSpinLock lg(data.m_lock);
    initialize();
}
void StreamHistorySession::pre_resolution_change(Resolution resolution){
    Data& data = *m_data;
    WriteSpinLock lg(data.m_lock);
    clear();
}
void StreamHistorySession::post_resolution_change(Resolution resolution){
    Data& data = *m_data;
    WriteSpinLock lg(data.m_lock);
    initialize();
}







}
