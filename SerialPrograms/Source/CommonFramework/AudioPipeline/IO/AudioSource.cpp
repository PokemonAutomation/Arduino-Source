/*  Audio Source
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Qt/StringToolsQt.h"
#include "CommonFramework/AudioPipeline/AudioStream.h"
#include "CommonFramework/AudioPipeline/Tools/AudioFormatUtils.h"
#include "AudioInputDevice.h"
#include "AudioInputFile.h"
#include "AudioSource.h"

namespace PokemonAutomation{



class AudioSource::InternalListener : public AudioFloatStreamListener{
public:
    InternalListener(AudioSource& parent)
        : AudioFloatStreamListener(parent.m_channels * parent.m_multiplier)
        , m_parent(parent)
    {}

private:
    virtual void on_samples(const float* data, size_t objects) override{
//        cout << "objects = " << objects << endl;
        m_parent.m_listeners.run_method(
            &AudioFloatStreamListener::on_samples,
            data, objects
        );
    }

    AudioSource& m_parent;
};




void AudioSource::add_listener(AudioFloatStreamListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    m_listeners.add(listener);
}
void AudioSource::remove_listener(AudioFloatStreamListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    m_listeners.remove(listener);
}
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

AudioSource::AudioSource(Logger& logger, const std::string& file, AudioChannelFormat format, float volume_multiplier){
    QAudioFormat native_format;
    if (has_extension(file, "wav") && probe_wav_native_format(file, native_format)){
        //  Native WAV: the stream is the file's own format (rate, channels,
        //  sample type). The tree's decoder / resampler / mixer convert it to
        //  the sink's format; no 96k reinterpretation for files.
        m_stream_format = get_sample_format(native_format);
    }else{
        //  Compressed formats are decoded by QAudioDecoder to a fixed target:
        //  float @ 48k stereo (per the preset). An unreadable .wav falls
        //  through here too; the loader logs the open failure.
        set_sample_format_to_float(native_format);
        set_format(native_format, format);
        m_stream_format = AudioSampleFormat::FLOAT32;
    }
    m_sample_rate = (size_t)native_format.sampleRate();
    m_channels = (size_t)native_format.channelCount();
    m_multiplier = 1;
    m_reader.reset(new AudioStreamToFloat(m_stream_format, m_channels, volume_multiplier, false));
    m_internal_listener = std::make_unique<InternalListener>(*this);
    m_reader->add_listener(*m_internal_listener);
    m_input_file = std::make_unique<AudioInputFile>(logger, *m_reader, file, native_format);
}
AudioSource::AudioSource(Logger& logger, const AudioDeviceInfo& device, AudioChannelFormat format, float volume_multiplier){
    NativeAudioInfo native_info = device.native_info();
    QAudioFormat native_format = native_info.preferredFormat();

    set_format(native_format, format);

    AudioSampleFormat stream_format = get_sample_format(native_format);
    if (stream_format == AudioSampleFormat::INVALID){
        stream_format = AudioSampleFormat::FLOAT32;
        set_sample_format_to_float(native_format);
    }

    init(format, stream_format, volume_multiplier);
    //  Only the interleaved presets misreport the card's two 48 kHz
    //  channels as a 1ch x 96000 stream; MONO_96000 is a true 96 kHz mono
    //  stream and keeps that shape. This is a capture-card hardware quirk,
    //  so it applies to device input only (file input is loaded in the
    //  requested format as-is).
    const bool reinterpret_96k =
        format == AudioChannelFormat::INTERLEAVE_LR_96000
        || format == AudioChannelFormat::INTERLEAVE_RL_96000;
    m_input_device = std::make_unique<AudioInputDevice>(logger, *m_reader, native_info, native_format, reinterpret_96k);
}

void AudioSource::init(AudioChannelFormat format, AudioSampleFormat stream_format, float volume_multiplier){
    auto scope_check = m_sanitizer.check_scope();
    m_stream_format = stream_format;
    switch (format){
    case AudioChannelFormat::MONO_48000:
        m_sample_rate = 48000;
        m_channels = 1;
        m_multiplier = 1;
        m_reader.reset(new AudioStreamToFloat(stream_format, 1, volume_multiplier, false));
        break;
    case AudioChannelFormat::DUAL_44100:
        m_sample_rate = 44100;
        m_channels = 2;
        m_multiplier = 1;
        m_reader.reset(new AudioStreamToFloat(stream_format, 2, volume_multiplier, false));
        break;
    case AudioChannelFormat::DUAL_48000:
        m_sample_rate = 48000;
        m_channels = 2;
        m_multiplier = 1;
        m_reader.reset(new AudioStreamToFloat(stream_format, 2, volume_multiplier, false));
        break;
    case AudioChannelFormat::MONO_96000:
        //  Treat mono-96000 as 2-sample frames.
        //  The FFT will then average each pair to produce 48000Hz.
        //  The output will push the same stream at the original 4 bytes * 96000Hz.
        m_sample_rate = 96000;
        m_channels = 1;
        m_multiplier = 2;
        m_reader.reset(new AudioStreamToFloat(stream_format, 2, volume_multiplier, false));
        break;
    case AudioChannelFormat::INTERLEAVE_LR_96000:
        m_sample_rate = 48000;
        m_channels = 2;
        m_multiplier = 1;
        m_reader.reset(new AudioStreamToFloat(stream_format, 2, volume_multiplier, false));
        break;
    case AudioChannelFormat::INTERLEAVE_RL_96000:
        m_sample_rate = 48000;
        m_channels = 2;
        m_multiplier = 1;
        m_reader.reset(new AudioStreamToFloat(stream_format, 2, volume_multiplier, true));
        break;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid AudioFormat: " + std::to_string((size_t)format));
    }

    m_internal_listener = std::make_unique<InternalListener>(*this);
    m_reader->add_listener(*m_internal_listener);
}









}
