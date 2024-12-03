/*  Recent History
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <deque>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/VideoPipeline/Backends/VideoFrameQt.h"
#include "RecentHistory.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


struct AudioBlock{
    WallClock timestamp;
    std::vector<float> samples;

    AudioBlock(const AudioBlock&) = delete;
    void operator=(const AudioBlock&) = delete;

    AudioBlock(WallClock p_timestamp, const float* p_samples, size_t p_frames)
        : timestamp(p_timestamp)
        , samples(p_samples, p_samples + p_frames)
    {}
};


struct RecentHistoryTracker::Data{
    mutable SpinLock lock;
    std::chrono::seconds window;
    size_t audio_samples_per_second;
    double microseconds_per_sample;

    //  We use shared_ptr here so it's fast to snapshot when we need to copy
    //  everything asynchronously.
    std::deque<std::shared_ptr<AudioBlock>> audio;
    std::deque<std::shared_ptr<VideoFrame>> frames;
};



RecentHistoryTracker::~RecentHistoryTracker() = default;
RecentHistoryTracker::RecentHistoryTracker(
    size_t audio_samples_per_second,
    std::chrono::seconds window
)
    : m_data(CONSTRUCT_TOKEN)
{
    Data& data = *m_data;
    data.window = window;
    data.audio_samples_per_second = audio_samples_per_second;
    data.microseconds_per_sample = 1. / (audio_samples_per_second * 1000000.);
}

void RecentHistoryTracker::set_window(std::chrono::seconds window){
    Data& data = *m_data;
    WriteSpinLock lg(data.lock);
    data.window = window;
    clear_old();
}
void RecentHistoryTracker::on_samples(const float* samples, size_t frames){
    Data& data = *m_data;
    WallClock now = current_time();
    WriteSpinLock lg(data.lock);
//    cout << "on_samples() = " << data.audio.size() << endl;
    data.audio.emplace_back(std::make_shared<AudioBlock>(
        now, samples, frames
    ));
    clear_old();
}
void RecentHistoryTracker::on_frame(std::shared_ptr<VideoFrame> frame){
    //  TODO: Find a more efficient way to buffer the frames.
    //  It takes almost 10GB of memory to store 30 seconds of QVideoFrames
    //  due to them caching uncompressed bitmaps.
    return;

    Data& data = *m_data;
    WriteSpinLock lg(data.lock);
//    cout << "on_frame() = " << data.frames.size() << endl;
    data.frames.emplace_back(std::move(frame));
    clear_old();
}



void RecentHistoryTracker::clear_old(){
    //  Must call under lock.
    WallClock now = current_time();
    Data& data = *m_data;
    WallClock threshold = now - data.window;

//    WriteSpinLock lg(data.lock);
//    cout << "enter" << endl;
    while (!data.audio.empty()){
//        cout << "audio.size() = " << data.audio.size() << endl;
        AudioBlock& block = *data.audio.front();

        WallClock end_block = block.timestamp;
        end_block += std::chrono::microseconds(
            static_cast<std::chrono::microseconds::rep>((double)block.samples.size() * data.microseconds_per_sample)
        );

        if (end_block < threshold){
            data.audio.pop_front();
        }else{
            break;
        }
    }
//    cout << "exit" << endl;

    while (!data.frames.empty()){
        if (data.frames.front()->timestamp < threshold){
            data.frames.pop_front();
        }else{
            break;
        }
    }
}

void RecentHistoryTracker::save(Logger& logger, const std::string& filename) const{
    std::deque<std::shared_ptr<AudioBlock>> audio;
    std::deque<std::shared_ptr<VideoFrame>> frames;
    const Data& data = *m_data;
    {
        //  Fast copy the current state of the stream.
        WriteSpinLock lg(data.lock);
        if (audio.empty() && frames.empty()){
            return;
        }
        audio = data.audio;
        frames = data.frames;
    }

    //  Now that the lock is released, we can take our time encoding it.

    //  TODO
}








RecentHistorySession::RecentHistorySession(Logger& logger)
    : AudioFloatStreamListener(1)
    , m_logger(logger)
    , m_window(GlobalSettings::instance().VIDEO_HISTORY_SECONDS)
    , m_audio_format(AudioChannelFormat::NONE)
{}
void RecentHistorySession::start(){
    WriteSpinLock lg(m_lock);
    if (!m_current){
        initialize();
    }
}
void RecentHistorySession::save(const std::string& filename){
    //  Get an owning reference to the current tracker.
    //  This will allow us to promptly release the lock and unblock the UI from
    //  changing the streams (which will wipe the history).
    std::shared_ptr<RecentHistoryTracker> tracker;
    {
        WriteSpinLock lg(m_lock);
        tracker = m_current;
    }

    tracker->save(m_logger, filename);
}
void RecentHistorySession::on_samples(const float* data, size_t frames){
    WriteSpinLock lg(m_lock);
    if (m_current){
        m_current->on_samples(data, frames);
    }
}
void RecentHistorySession::on_frame(std::shared_ptr<VideoFrame> frame){
    WriteSpinLock lg(m_lock);
    if (m_current){
        m_current->on_frame(std::move(frame));
    }
}


void RecentHistorySession::clear(){
    //  Must call under lock.
    m_logger.log("Clearing stream history...", COLOR_ORANGE);
    m_current.reset();
    expected_samples_per_frame = 0;
    m_audio_format = AudioChannelFormat::NONE;
}
void RecentHistorySession::initialize(){
    //  Must call under lock.
    m_logger.log("Starting stream history...", COLOR_ORANGE);
    switch (m_audio_format){
    case AudioChannelFormat::NONE:
        expected_samples_per_frame = 0;
        m_current.reset(new RecentHistoryTracker(1, m_window));
        return;
    case AudioChannelFormat::MONO_48000:
        m_current.reset(new RecentHistoryTracker(48000 * 1, m_window));
        return;
    case AudioChannelFormat::DUAL_44100:
        m_current.reset(new RecentHistoryTracker(44100 * 1, m_window));
        return;
    case AudioChannelFormat::DUAL_48000:
    case AudioChannelFormat::MONO_96000:
    case AudioChannelFormat::INTERLEAVE_LR_96000:
    case AudioChannelFormat::INTERLEAVE_RL_96000:
        m_current.reset(new RecentHistoryTracker(48000 * 2, m_window));
        return;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid AudioFormat: " + std::to_string((size_t)m_audio_format));
    }
}
void RecentHistorySession::pre_input_change(){
    WriteSpinLock lg(m_lock);
    clear();
}
void RecentHistorySession::post_input_change(const std::string& file, const AudioDeviceInfo& device, AudioChannelFormat format){
    WriteSpinLock lg(m_lock);
    m_audio_format = format;
    initialize();
}
void RecentHistorySession::pre_shutdown(){
    WriteSpinLock lg(m_lock);
    clear();
}
void RecentHistorySession::post_new_source(const CameraInfo& device, Resolution resolution){
    WriteSpinLock lg(m_lock);
    initialize();
}
void RecentHistorySession::pre_resolution_change(Resolution resolution){
    WriteSpinLock lg(m_lock);
    clear();
}
void RecentHistorySession::post_resolution_change(Resolution resolution){
    WriteSpinLock lg(m_lock);
    initialize();
}







}
