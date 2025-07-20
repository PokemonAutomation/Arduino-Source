/*  Stream History Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QThread>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/VideoPipeline/Backends/VideoFrameQt.h"
#include "CommonFramework/Recording/StreamHistoryOption.h"

#if (QT_VERSION_MAJOR == 6) && (QT_VERSION_MINOR >= 8)
//#include "StreamHistoryTracker_SaveFrames.h"
//#include "StreamHistoryTracker_RecordOnTheFly.h"
#include "StreamHistoryTracker_ParallelStreams.h"
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
    bool m_has_video;
    std::shared_ptr<StreamHistoryTracker> m_current;

    Data(Logger& logger)
        : m_logger(logger)
        , m_window(GlobalSettings::instance().STREAM_HISTORY->HISTORY_SECONDS)
        , m_audio_format(AudioChannelFormat::NONE)
        , m_has_video(false)
    {}
};




StreamHistorySession::~StreamHistorySession() = default;
StreamHistorySession::StreamHistorySession(Logger& logger)
    : AudioFloatStreamListener(1)
    , m_data(CONSTRUCT_TOKEN, logger)
{}
void StreamHistorySession::start(AudioChannelFormat format, bool has_video){
    Data& data = *m_data;
    WriteSpinLock lg(data.m_lock);
    if (!data.m_current){
        data.m_audio_format = format;
        data.m_has_video = has_video;
        initialize();
    }
}


#if 0
class HistorySaverThread : public QThread{
public:
    HistorySaverThread(StreamHistoryTracker& tracker, const std::string& filename)
        : m_tracker(tracker)
        , m_filename(filename)
    {}
    ~HistorySaverThread(){
        quit();
        wait();
    }
    bool save(){
        start();
        quit();
        wait();
        return m_success;
    }
    virtual void run() override{
//        m_success = m_tracker.save(m_logger, m_filename);
        m_success = m_tracker.save(m_filename);
        exec();
    }

private:
    StreamHistoryTracker& m_tracker;
    const std::string& m_filename;
    bool m_success = false;
};
#endif

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
void StreamHistorySession::on_samples(const float* samples, size_t frames){
    Data& data = *m_data;
    WriteSpinLock lg(data.m_lock);
    if (data.m_current){
        data.m_current->on_samples(samples, frames);
    }
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
//    expected_samples_per_frame = 0;
//    data.m_audio_format = AudioChannelFormat::NONE;
//    data.m_has_video = false;
}
void StreamHistorySession::initialize(){
    if (!GlobalSettings::instance().STREAM_HISTORY->enabled()){
        return;
    }

    //  Must call under lock.
    Data& data = *m_data;
    data.m_logger.log("Starting stream history...", COLOR_ORANGE);

//    cout << "video = " << data.m_has_video << endl;

    switch (data.m_audio_format){
    case AudioChannelFormat::NONE:
        expected_samples_per_frame = 0;
        data.m_current.reset(new StreamHistoryTracker(data.m_logger, data.m_window, 0, 0, data.m_has_video));
        return;
    case AudioChannelFormat::MONO_48000:
        expected_samples_per_frame = 1;
        data.m_current.reset(new StreamHistoryTracker(data.m_logger, data.m_window, 1, 48000, data.m_has_video));
        return;
    case AudioChannelFormat::DUAL_44100:
        expected_samples_per_frame = 2;
        data.m_current.reset(new StreamHistoryTracker(data.m_logger, data.m_window, 1, 44100, data.m_has_video));
        return;
    case AudioChannelFormat::DUAL_48000:
    case AudioChannelFormat::MONO_96000:
    case AudioChannelFormat::INTERLEAVE_LR_96000:
    case AudioChannelFormat::INTERLEAVE_RL_96000:
        expected_samples_per_frame = 2;
        data.m_current.reset(new StreamHistoryTracker(data.m_logger, data.m_window, 2, 48000, data.m_has_video));
        return;
    default:
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Invalid AudioFormat: " + std::to_string((size_t)data.m_audio_format)
        );
    }
}
void StreamHistorySession::pre_input_change(){
//    cout << "pre_input_change()" << endl;
    Data& data = *m_data;
    WriteSpinLock lg(data.m_lock);
    clear();
}
void StreamHistorySession::post_input_change(const std::string& file, const AudioDeviceInfo& device, AudioChannelFormat format){
//    cout << "post_input_change()" << endl;
    Data& data = *m_data;
    WriteSpinLock lg(data.m_lock);
    if (device){
        data.m_audio_format = format;
    }else{
        data.m_audio_format = AudioChannelFormat::NONE;
    }
    initialize();
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
