/*  Audio Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/GlobalServices.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "Backends/AudioPassthroughPairQtThread.h"
#include "AudioPipelineOptions.h"
#include "AudioSession.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{






void AudioSession::add_state_listener(StateListener& listener){
    m_listeners.add(listener);
}
void AudioSession::remove_state_listener(StateListener& listener){
    m_listeners.remove(listener);
}
void AudioSession::add_stream_listener(AudioFloatStreamListener& listener){
    m_devices->add_listener(listener);
}
void AudioSession::remove_stream_listener(AudioFloatStreamListener& listener){
    m_devices->remove_listener(listener);
}
void AudioSession::add_spectrum_listener(AudioSpectrumHolder::Listener& listener){
    m_spectrum_holder.add_listener(listener);
}
void AudioSession::remove_spectrum_listener(AudioSpectrumHolder::Listener& listener){
    m_spectrum_holder.remove_listener(listener);
}



AudioSession::AudioSession(Logger& logger, AudioOption& option)
     : m_logger(logger)
     , m_option(option)
     , m_devices(new AudioPassthroughPairQtThread(logger))
{
    AudioSession::reset();
    m_devices->add_listener(*this);

    uint8_t watchdog_timeout = GlobalSettings::instance().AUDIO_PIPELINE->AUTO_RESET_SECONDS;
    if (watchdog_timeout != 0){
        global_watchdog().add(*this, std::chrono::seconds(watchdog_timeout));
    }
}
AudioSession::~AudioSession(){
    global_watchdog().remove(*this);
    m_devices->remove_listener(*this);
}


void AudioSession::get(AudioOption& option){
    std::lock_guard<std::mutex> lg(m_lock);
    option.m_input_file     = m_option.m_input_file;
    option.m_input_device   = m_option.m_input_device;
    option.m_input_format   = m_option.m_input_format;
    option.m_output_device  = m_option.m_output_device;
    option.m_volume         = m_option.m_volume;
    option.m_display_type   = m_option.m_display_type;
}
void AudioSession::set(const AudioOption& option){
    {
        std::lock_guard<std::mutex> lg(m_lock);
        signal_pre_input_change();

        m_option.m_input_file       = option.m_input_file;
        m_option.m_input_device     = option.m_input_device;
        m_option.m_input_format     = option.m_input_format;
        m_option.m_output_device    = option.m_output_device;
        m_option.m_volume           = option.m_volume;
        m_option.m_display_type     = option.m_display_type;

        if (!m_option.m_input_file.empty()){
            sanitize_format();
            m_devices->set_audio_source(m_option.m_input_file);
        }else if (sanitize_format()){
            m_devices->set_audio_source(m_option.m_input_device, m_option.m_input_format);
        }else{
            m_devices->clear_audio_source();
        }

        m_devices->set_audio_sink(m_option.m_output_device, m_option.m_volume);
    }

    signal_post_input_change();
    signal_post_output_change();
    m_listeners.run_method_unique(&StateListener::post_volume_change, m_option.volume());
    m_listeners.run_method_unique(&StateListener::post_display_change, m_option.m_display_type);
}
std::pair<std::string, AudioDeviceInfo> AudioSession::input_device() const{
    std::lock_guard<std::mutex> lg(m_lock);
    return {m_option.input_file(), m_option.m_input_device};
}
AudioChannelFormat AudioSession::input_format() const{
    std::lock_guard<std::mutex> lg(m_lock);
    return m_option.m_input_format;
}
AudioDeviceInfo AudioSession::output_device() const{
    std::lock_guard<std::mutex> lg(m_lock);
    return m_option.m_output_device;
}
double AudioSession::output_volume() const{
    std::lock_guard<std::mutex> lg(m_lock);
    return m_option.m_volume;
}
AudioOption::AudioDisplayType AudioSession::display_type() const{
    std::lock_guard<std::mutex> lg(m_lock);
    return m_option.m_display_type;
}


void AudioSession::clear_audio_input(){
    std::lock_guard<std::mutex> lg(m_lock);
    m_logger.log("Clearing audio input...");
    signal_pre_input_change();
    m_devices->clear_audio_source();
    m_option.m_input_file.clear();
    m_option.m_input_device = AudioDeviceInfo();
    signal_post_input_change();

    //  We need to do this at the end.
    //  The previous line "signal_post_input_change()" is what will shut off the
    //  audio stream. If we clear the history before that, a race condition can
    //  add another spectrum to the history before the stream actually stops.
    //  When this happens, we will be left with a non-empty history which will
    //  be displayed as a non-moving freq-bars or spectrum instead of black.
    m_spectrum_holder.clear();
}
void AudioSession::set_audio_input(std::string file){
    std::lock_guard<std::mutex> lg(m_lock);
    signal_pre_input_change();
    m_option.m_input_file = std::move(file);
    sanitize_format();
    m_devices->set_audio_source(m_option.m_input_file);
    signal_post_input_change();
}
void AudioSession::set_audio_input(AudioDeviceInfo info){
    std::lock_guard<std::mutex> lg(m_lock);
    m_logger.log("Setting audio input to: " + info.display_name());
    signal_pre_input_change();
    m_option.m_input_file.clear();
    m_option.m_input_device = std::move(info);
    m_option.m_input_format = AudioChannelFormat::NONE;
    if (sanitize_format()){
        m_devices->set_audio_source(info, m_option.m_input_format);
    }else{
        m_devices->clear_audio_source();
    }
    signal_post_input_change();
}
void AudioSession::set_format(AudioChannelFormat format){
    std::lock_guard<std::mutex> lg(m_lock);
    signal_pre_input_change();
    m_option.m_input_format = format;
    if (sanitize_format()){
        if (!m_option.m_input_file.empty()){
            m_devices->set_audio_source(m_option.m_input_file);
        }else{
            m_devices->set_audio_source(m_option.m_input_device, m_option.m_input_format);
        }
    }else{
        m_devices->clear_audio_source();
    }
    signal_post_input_change();
}
void AudioSession::clear_audio_output(){
    std::lock_guard<std::mutex> lg(m_lock);
    m_logger.log("Clearing audio output...");
    m_devices->clear_audio_sink();
    m_option.m_output_device = AudioDeviceInfo();
    signal_post_output_change();
}
void AudioSession::set_audio_output(AudioDeviceInfo info){
    std::lock_guard<std::mutex> lg(m_lock);
    m_logger.log("Setting audio output to: " + info.display_name());
    m_devices->set_audio_sink(info, m_option.m_volume);
    m_option.m_output_device = std::move(info);
    signal_post_output_change();
}
void AudioSession::set_volume(double volume){
    {
        std::lock_guard<std::mutex> lg(m_lock);
        if (m_option.m_volume == volume){
            return;
        }
        m_devices->set_sink_volume(volume);
        m_option.m_volume = volume;
    }
    m_listeners.run_method_unique(&StateListener::post_volume_change, m_option.volume());
}
void AudioSession::set_display(AudioOption::AudioDisplayType display){
    {
        std::lock_guard<std::mutex> lg(m_lock);
        if (m_option.m_display_type == display){
            return;
        }
        m_option.m_display_type = display;
    }
    m_listeners.run_method_unique(&StateListener::post_display_change, m_option.m_display_type);
}

bool AudioSession::sanitize_format(){
    if (!m_option.m_input_file.empty()){
        m_option.m_input_format = AudioChannelFormat::NONE;
        return true;
    }
    AudioDeviceInfo& info = m_option.m_input_device;
    const std::vector<AudioChannelFormat>& supported_formats = info.supported_formats();
    int preferred_index = info.preferred_format_index();
    if (supported_formats.empty() || preferred_index < 0){
//        cout << "supported_formats = " << supported_formats.size() << endl;
//        cout << "preferred_index = " << preferred_index << endl;
        m_logger.log("No supported formats for this input device.", COLOR_RED);
        m_option.m_input_format = AudioChannelFormat::NONE;
        return false;
    }
    if (m_option.m_input_format == AudioChannelFormat::NONE){
        m_logger.log("No format set. Resetting to default...", COLOR_ORANGE);
    }else{
        for (AudioChannelFormat supported_format : supported_formats){
            if (m_option.m_input_format == supported_format){
                return true;
            }
        }
        m_logger.log("Desired format not supported. Resetting to default...", COLOR_RED);
    }
    m_option.m_input_format = supported_formats[preferred_index];
    return true;
}
void AudioSession::signal_pre_input_change(){
    m_listeners.run_method_unique(&StateListener::pre_input_change);
}
void AudioSession::signal_post_input_change(){
    m_listeners.run_method_unique(
        &StateListener::post_input_change,
        m_option.input_file(),
        m_option.input_device(),
        m_option.input_format()
    );
}
void AudioSession::signal_post_output_change(){
    m_listeners.run_method_unique(&StateListener::post_output_change, m_option.output_device());
}



void AudioSession::reset(){
    std::lock_guard<std::mutex> lg(m_lock);
    m_logger.log("AudioSession::reset()");
    signal_pre_input_change();
    if (!m_option.m_input_file.empty()){
//        cout << "AudioSession::reset() - file: " << m_option.m_input_file << " - " << m_option.m_input_file.size() << endl;
        m_devices->reset(
            m_option.m_input_file,
            m_option.m_output_device, m_option.m_volume
        );
    }else{
//        cout << "AudioSession::reset() - device: " << m_option.m_inputDevice.display_name() << endl;
        m_devices->reset(
            m_option.m_input_device, m_option.m_input_format,
            m_option.m_output_device, m_option.m_volume
        );
    }
    signal_post_input_change();
}
std::vector<AudioSpectrum> AudioSession::spectrums_since(uint64_t starting_seqnum){
    return m_spectrum_holder.spectrums_since(starting_seqnum);
}
std::vector<AudioSpectrum> AudioSession::spectrums_latest(size_t num_last_spectrums){
    return m_spectrum_holder.spectrums_latest(num_last_spectrums);
}
void AudioSession::add_overlay(uint64_t starting_seqnum, size_t end_seqnum, Color color){
    m_spectrum_holder.add_overlay(starting_seqnum, end_seqnum, color);
}


void AudioSession::on_fft(size_t sample_rate, std::shared_ptr<const AlignedVector<float>> fft_output){
    m_spectrum_holder.push_spectrum(sample_rate, std::move(fft_output));
    global_watchdog().delay(*this);
}
void AudioSession::on_watchdog_timeout(){
//    m_logger.log("AudioSession::on_watchdog_timeout()", COLOR_RED);
    if (m_option.m_input_file.empty() && !m_option.m_input_device){
        return;
    }

    uint8_t watchdog_timeout = GlobalSettings::instance().AUDIO_PIPELINE->AUTO_RESET_SECONDS;
    m_logger.log("No audio detected for " + std::to_string(watchdog_timeout) + " seconds...", COLOR_RED);

    if (watchdog_timeout == 0){
        return;
    }
    m_logger.log("Resetting the audio...", COLOR_GREEN);
    reset();
}








}
