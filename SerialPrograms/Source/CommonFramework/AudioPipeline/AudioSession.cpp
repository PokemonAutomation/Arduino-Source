/*  Audio Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/AbstractLogger.h"
#include "CommonFramework/GlobalServices.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "Backends/AudioPassthroughPairQtThread.h"
#include "AudioSession.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{






void AudioSession::add_ui_listener(Listener& listener){
    std::lock_guard<std::mutex> lg(m_lock);
    m_listeners.insert(&listener);
}
void AudioSession::remove_ui_listener(Listener& listener){
    std::lock_guard<std::mutex> lg(m_lock);
    m_listeners.erase(&listener);
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
    global_watchdog().add(*this, std::chrono::seconds(5));
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
    std::lock_guard<std::mutex> lg(m_lock);
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

    push_input_changed();
    push_output_changed();
    for (Listener* listener : m_listeners){
        listener->volume_changed(m_option.volume());
        listener->display_changed(m_option.m_display_type);
    }
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
    m_spectrum_holder.clear();
    m_devices->clear_audio_source();
    m_option.m_input_file.clear();
    m_option.m_input_device = AudioDeviceInfo();
    push_input_changed();
}
void AudioSession::set_audio_input(std::string file){
    std::lock_guard<std::mutex> lg(m_lock);
    m_option.m_input_file = std::move(file);
    sanitize_format();
    m_devices->set_audio_source(m_option.m_input_file);
    push_input_changed();
}
void AudioSession::set_audio_input(AudioDeviceInfo info){
    std::lock_guard<std::mutex> lg(m_lock);
    m_logger.log("Setting audio input to: " + info.display_name());
    m_option.m_input_file.clear();
    m_option.m_input_device = std::move(info);
    m_option.m_input_format = AudioChannelFormat::NONE;
    if (sanitize_format()){
        m_devices->set_audio_source(info, m_option.m_input_format);
    }else{
        m_devices->clear_audio_source();
    }
    push_input_changed();
}
void AudioSession::set_format(AudioChannelFormat format){
    std::lock_guard<std::mutex> lg(m_lock);
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
    push_input_changed();
}
void AudioSession::clear_audio_output(){
    std::lock_guard<std::mutex> lg(m_lock);
    m_logger.log("Clearing audio output...");
    m_devices->clear_audio_sink();
    m_option.m_output_device = AudioDeviceInfo();
    push_output_changed();
}
void AudioSession::set_audio_output(AudioDeviceInfo info){
    std::lock_guard<std::mutex> lg(m_lock);
    m_logger.log("Setting audio output to: " + info.display_name());
    m_devices->set_audio_sink(info, m_option.m_volume);
    m_option.m_output_device = std::move(info);
    push_output_changed();
}
void AudioSession::set_volume(double volume){
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_option.m_volume == volume){
        return;
    }
    m_devices->set_sink_volume(volume);
    m_option.m_volume = volume;
    for (Listener* listener : m_listeners){
        listener->volume_changed(m_option.volume());
    }
}
void AudioSession::set_display(AudioOption::AudioDisplayType display){
    std::lock_guard<std::mutex> lg(m_lock);
    if (m_option.m_display_type == display){
        return;
    }
    m_option.m_display_type = display;
    for (Listener* listener : m_listeners){
        listener->display_changed(m_option.m_display_type);
    }
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
void AudioSession::push_input_changed(){
    for (Listener* listener : m_listeners){
        listener->input_changed(m_option.input_file(), m_option.input_device(), m_option.input_format());
    }
}
void AudioSession::push_output_changed(){
    for (Listener* listener : m_listeners){
        listener->output_changed(m_option.output_device());
    }
}



void AudioSession::reset(){
    std::lock_guard<std::mutex> lg(m_lock);
    m_logger.log("AudioSession::reset()");
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


void AudioSession::on_fft(size_t sample_rate, std::shared_ptr<AlignedVector<float>> fft_output){
    m_spectrum_holder.push_spectrum(sample_rate, std::move(fft_output));
    global_watchdog().delay(*this);
}
void AudioSession::on_watchdog_timeout(){
//    m_logger.log("AudioSession::on_watchdog_timeout()", COLOR_RED);
    if (m_option.m_input_file.empty() && !m_option.m_input_device){
        return;
    }
    m_logger.log("No audio detected for 5 seconds...", COLOR_RED);

    if (!GlobalSettings::instance().ENABLE_AUTO_RESET_AUDIO){
        return;
    }
    m_logger.log("Resetting the audio...", COLOR_GREEN);
    reset();
}








}
