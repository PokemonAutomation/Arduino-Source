/*  Audio Session
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QAudio>
#include "Backends/AudioPassthroughPairQtThread.h"
#include "AudioSession.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



// Slider bar volume: [0, 100], in log scale
// Volume value passed to AudioDisplayWidget (and the audio thread it manages): [0.f, 1.f], linear scale
float convertAudioVolumeFromSlider(int volume){
    volume = std::max(volume, 0);
    volume = std::min(volume, 100);
    // The slider bar value is in the log scale because log scale matches human sound
    // perception.
    float linearVolume = QAudio::convertVolume(
        volume / float(100.0f),
        QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale
    );
    return linearVolume;
}




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
}
AudioSession::~AudioSession(){
    m_devices->remove_listener(*this);
}

void AudioSession::clear_audio_input(){
    std::lock_guard<std::mutex> lg(m_lock);
    m_spectrum_holder.clear();
    m_devices->clear_audio_source();
    m_option.m_input_file.clear();
    m_option.m_input_device = AudioDeviceInfo();
}
void AudioSession::set_audio_input(std::string file){
    std::lock_guard<std::mutex> lg(m_lock);
    m_option.m_input_file = std::move(file);
    sanitize_format();
    m_devices->set_audio_source(m_option.m_input_file);
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
}
void AudioSession::clear_audio_output(){
    std::lock_guard<std::mutex> lg(m_lock);
    m_devices->clear_audio_sink();
    m_option.m_output_device = AudioDeviceInfo();
}
void AudioSession::set_audio_output(AudioDeviceInfo info){
    std::lock_guard<std::mutex> lg(m_lock);
    m_devices->set_audio_sink(info, m_option.m_volume);
    m_option.m_output_device = std::move(info);
}
void AudioSession::set_volume(int volume){
    std::lock_guard<std::mutex> lg(m_lock);
    m_devices->set_sink_volume(convertAudioVolumeFromSlider(volume));
    m_option.m_volume = volume;
}
void AudioSession::set_display(AudioOption::AudioDisplayType display){
    std::lock_guard<std::mutex> lg(m_lock);
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



void AudioSession::reset(){
    m_logger.log("AudioSession::reset()");
    std::lock_guard<std::mutex> lg(m_lock);
    if (!sanitize_format()){
        m_devices->clear_audio_source();
        return;
    }
    float volume = convertAudioVolumeFromSlider(m_option.m_volume);
    if (!m_option.m_input_file.empty()){
//        cout << "AudioSession::reset() - file: " << m_option.m_input_file << " - " << m_option.m_input_file.size() << endl;
        m_devices->reset(
            m_option.m_input_file,
            m_option.m_output_device, volume
        );
    }else{
//        cout << "AudioSession::reset() - device: " << m_option.m_inputDevice.display_name() << endl;
        m_devices->reset(
            m_option.m_input_device, m_option.m_input_format,
            m_option.m_output_device, volume
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
}








}
