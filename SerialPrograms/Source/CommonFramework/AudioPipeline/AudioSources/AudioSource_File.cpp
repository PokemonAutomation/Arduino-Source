/*  Audio Source (File)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QAudioFormat>
#include <QFileDialog>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/StringToolsQt.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/AudioPipeline/AudioPipelineOptions.h"
#include "CommonFramework/AudioPipeline/Tools/AudioFormatUtils.h"
#include "AudioSource_File.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{




std::string AudioSourceDescriptor_File::path() const{
    ReadSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
    return m_path;
}
void AudioSourceDescriptor_File::set_path(std::string path){
    WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
    m_path = std::move(path);
}

bool AudioSourceDescriptor_File::operator==(const AudioSourceDescriptor& x) const{
    if (typeid(*this) != typeid(x)){
        return false;
    }

    std::string other_path = static_cast<const AudioSourceDescriptor_File&>(x).m_path;

    ReadSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
    return m_path == other_path;
}

std::vector<AudioInputChoice> AudioSourceDescriptor_File::supported_formats() const{
    return {};
}
bool AudioSourceDescriptor_File::sanitize_desired_format(Logger& logger, AudioInputChoice& format) const{
    format = AudioInputChoice();
    return true;
}

void AudioSourceDescriptor_File::run_post_select(){
//    if (!m_path.empty()){
//        return;
//    }
    std::string path = QFileDialog::getOpenFileName(
        nullptr, "Open audio file", ".", "*.wav *.mp3"
    ).toStdString();
    set_path(std::move(path));
}
JsonValue AudioSourceDescriptor_File::to_json() const{
    ReadSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
    return m_path;
}
void AudioSourceDescriptor_File::load_json(const JsonValue& json){
//    cout << "load_json: " << m_path << endl;
    const std::string* name = json.to_string();
    if (name != nullptr){
        WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
        m_path = *name;
    }
}

std::unique_ptr<AudioSource> AudioSourceDescriptor_File::make_AudioSource(
    Logger& logger,
    const AudioInputChoice& format
) const{
//    cout << "make_AudioSource: " << m_path << endl;
    if (m_path.empty()){
        //  E.g. the user cancelled the file dialog: no source.
        return nullptr;
    }
    return std::make_unique<AudioSource_File>(logger, m_path);
}




AudioSource_File::AudioSource_File(
    Logger& logger,
    const std::string& file
){
    QAudioFormat native_format;
    if (has_extension(file, "wav") && probe_wav_native_format(file, native_format)){
        //  Native WAV: the stream is the file's own format (rate, channels,
        //  sample type). The tree's decoder / resampler / mixer convert it to
        //  the sink's format; no 96k reinterpretation for files.
        m_stream_format = get_sample_format(native_format);
    }else{
        //  Compressed formats: probe the decoder's native output format
        //  and stream the file at its own rate / channels / sample type,
        //  like the native WAV path (the tree's decoder / resampler /
        //  mixer convert it to the sink's format). If the probe fails
        //  (no decode backend, unreadable file), fall back to the legacy
        //  48k stereo float target; the loader logs the decode failure as
        //  before. An unreadable .wav falls through here too.
        if (probe_decoded_native_format(file, native_format)){
            m_stream_format = get_sample_format(native_format);
        }else{
            //  The channel count must be set before the channel config:
            //  QAudioFormat::setChannelCount() resets the config.
            native_format.setChannelCount(2);
            native_format.setChannelConfig(QAudioFormat::ChannelConfig::ChannelConfigStereo);
            native_format.setSampleRate(48000);
            set_sample_format_to_float(native_format);
            m_stream_format = AudioSampleFormat::FLOAT32;
        }
    }
    m_input_file = std::make_unique<AudioInputFile>(logger, file, native_format);
}

void AudioSource_File::add_raw_listener(RawAudioStreamListener& listener){
    m_input_file->add_listener(listener);
}
void AudioSource_File::remove_raw_listener(RawAudioStreamListener& listener){
    m_input_file->remove_listener(listener);
}

size_t AudioSource_File::raw_sample_rate() const{
    return m_input_file->sample_rate();
}
size_t AudioSource_File::raw_channels() const{
    return m_input_file->channels();
}

float AudioSource_File::volume_multiplier() const{
    return (float)GlobalSettings::instance().AUDIO_PIPELINE->FILE_VOLUME_SCALE;
}















}
