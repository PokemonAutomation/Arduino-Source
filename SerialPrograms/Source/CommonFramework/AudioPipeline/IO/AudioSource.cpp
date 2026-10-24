/*  Audio Source
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QAudioFormat>
#include "Common/Cpp/Exceptions.h"
#include "Common/Qt/StringToolsQt.h"
#include "CommonFramework/AudioPipeline/Tools/AudioFormatUtils.h"
#include "AudioInputDevice.h"
#include "AudioInputFile.h"
#include "AudioSource.h"

namespace PokemonAutomation{



size_t AudioSource::raw_sample_rate() const{
    if (m_input_file){
        return m_input_file->sample_rate();
    }
    if (m_input_device){
        return m_input_device->sample_rate();
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "No audio input.");
}
size_t AudioSource::raw_channels() const{
    if (m_input_file){
        return m_input_file->channels();
    }
    if (m_input_device){
        return m_input_device->channels();
    }
    throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "No audio input.");
}
void AudioSource::add_raw_listener(Kernels::AudioPipeline::RawAudioStreamListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    if (m_input_file){
        m_input_file->add_listener(listener);
    }else if (m_input_device){
        m_input_device->add_listener(listener);
    }else{
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "No audio input.");
    }
}
void AudioSource::remove_raw_listener(Kernels::AudioPipeline::RawAudioStreamListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    if (m_input_file){
        m_input_file->remove_listener(listener);
    }else if (m_input_device){
        m_input_device->remove_listener(listener);
    }else{
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "No audio input.");
    }
}


AudioSource::~AudioSource() = default;

AudioSource::AudioSource(
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
AudioSource::AudioSource(
    Logger& logger,
    const AudioDeviceInfo& device,
    const AudioInputChoice& choice
){
    if (!choice){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION, "Invalid audio input choice."
        );
    }
    NativeAudioInfo native_info = device.native_info();

    //  The device is opened in the choice's physical format: for the
    //  reinterpreted capture-card choices this is the physical 1ch x
    //  96000 stream; for native choices it is the choice itself.
    QAudioFormat native_format = choice.physical_format().native_format();

    AudioSampleFormat stream_format = choice.format().sample_format();
    if (stream_format == AudioSampleFormat::INVALID){
        //  Defensive: the choices built from input_choices() always carry
        //  a sample format the device supports.
        set_sample_format_to_float(native_format);
        stream_format = AudioSampleFormat::FLOAT32;
    }
    m_stream_format = stream_format;

    m_input_device = std::make_unique<AudioInputDevice>(
        logger,
        native_info,
        native_format,
        choice.is_reinterpreted()
    );
}









}
