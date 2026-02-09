/*  Audio Passthrough Pair (Qt separate thread)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "Common/Cpp/Concurrency/SpinPause.h"
//#include "CommonFramework/GlobalSettingsPanel.h"
//#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "CommonFramework/Tools/GlobalThreadPoolsQt.h"
#include "AudioPassthroughPairQt.h"
#include "AudioPassthroughPairQtThread.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


void AudioPassthroughPairQtThread::add_listener(AudioFloatStreamListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    AudioPassthroughPairQt* body = m_body;
    body->add_listener(listener);
}
void AudioPassthroughPairQtThread::remove_listener(AudioFloatStreamListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    AudioPassthroughPairQt* body = m_body;
    body->remove_listener(listener);
}
void AudioPassthroughPairQtThread::add_listener(FFTListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    AudioPassthroughPairQt* body = m_body;
    body->add_listener(listener);
}
void AudioPassthroughPairQtThread::remove_listener(FFTListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    AudioPassthroughPairQt* body = m_body;
    body->remove_listener(listener);
}


AudioPassthroughPairQtThread::AudioPassthroughPairQtThread(Logger& logger)
    : m_logger(logger)
    , m_body(static_cast<AudioPassthroughPairQt*>(
        GlobalThreadPools::qt_event_threadpool().add_object(
            [this]{
                return std::make_unique<AudioPassthroughPairQt>(m_logger);
            })
        )
    )
{}
AudioPassthroughPairQtThread::~AudioPassthroughPairQtThread(){
    GlobalThreadPools::qt_event_threadpool().remove_object(m_body);
}


void AudioPassthroughPairQtThread::reset(
    const std::string& file,
    const AudioDeviceInfo& output, double output_volume
){
    auto scope_check = m_sanitizer.check_scope();
    AudioPassthroughPairQt* body = m_body;
    body->reset(file, output, output_volume);
}
void AudioPassthroughPairQtThread::reset(
    const AudioDeviceInfo& input, AudioChannelFormat format,
    const AudioDeviceInfo& output, double output_volume
){
    auto scope_check = m_sanitizer.check_scope();
    AudioPassthroughPairQt* body = m_body;
    body->reset(input, format, output, output_volume);
}
void AudioPassthroughPairQtThread::clear_audio_source(){
    auto scope_check = m_sanitizer.check_scope();
    AudioPassthroughPairQt* body = m_body;
    body->clear_audio_source();
}
void AudioPassthroughPairQtThread::set_audio_source(const std::string& file){
    auto scope_check = m_sanitizer.check_scope();
    AudioPassthroughPairQt* body = m_body;
    body->set_audio_source(file);
}
void AudioPassthroughPairQtThread::set_audio_source(const AudioDeviceInfo& device, AudioChannelFormat format){
    auto scope_check = m_sanitizer.check_scope();
    AudioPassthroughPairQt* body = m_body;
    body->set_audio_source(device, format);
}
void AudioPassthroughPairQtThread::clear_audio_sink(){
    auto scope_check = m_sanitizer.check_scope();
    AudioPassthroughPairQt* body = m_body;
    body->clear_audio_sink();
}
void AudioPassthroughPairQtThread::set_audio_sink(const AudioDeviceInfo& device, double volume){
    auto scope_check = m_sanitizer.check_scope();
    AudioPassthroughPairQt* body = m_body;
    body->set_audio_sink(device, volume);
}
void AudioPassthroughPairQtThread::set_sink_volume(double volume){
    auto scope_check = m_sanitizer.check_scope();
    AudioPassthroughPairQt* body = m_body;
    body->set_sink_volume(volume);
}




}
