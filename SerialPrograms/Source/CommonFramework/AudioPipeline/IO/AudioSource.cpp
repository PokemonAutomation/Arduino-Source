/*  Audio Source
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

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


AudioSource::~AudioSource(){}

AudioSource::AudioSource(
    Logger& logger,
    const std::string& file,
    AudioChannelFormat format
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
        //  48k stereo float target per the preset; the loader logs the
        //  decode failure as before. An unreadable .wav falls through
        //  here too.
        if (probe_decoded_native_format(file, native_format)){
            m_stream_format = get_sample_format(native_format);
        }else{
            set_sample_format_to_float(native_format);
            set_format(native_format, format);
            m_stream_format = AudioSampleFormat::FLOAT32;
        }
    }
    m_input_file = std::make_unique<AudioInputFile>(logger, file, native_format);
}
AudioSource::AudioSource(
    Logger& logger,
    const AudioDeviceInfo& device,
    AudioChannelFormat format
){
    NativeAudioInfo native_info = device.native_info();
    QAudioFormat native_format = native_info.preferredFormat();

    set_format(native_format, format);

    AudioSampleFormat stream_format = get_sample_format(native_format);
    if (stream_format == AudioSampleFormat::INVALID){
        set_sample_format_to_float(native_format);
        stream_format = AudioSampleFormat::FLOAT32;
    }
    m_stream_format = stream_format;
    //  Only the interleaved presets misreport the card's two 48 kHz
    //  channels as a 1ch x 96000 stream; MONO_96000 is a true 96 kHz mono
    //  stream and keeps that shape. This is a capture-card hardware quirk,
    //  so it applies to device input only (file input is loaded in the
    //  requested format as-is).
    const bool reinterpret_96k =
        format == AudioChannelFormat::INTERLEAVE_LR_96000
        || format == AudioChannelFormat::INTERLEAVE_RL_96000;
    m_input_device = std::make_unique<AudioInputDevice>(logger, native_info, native_format, reinterpret_96k);
}









}
