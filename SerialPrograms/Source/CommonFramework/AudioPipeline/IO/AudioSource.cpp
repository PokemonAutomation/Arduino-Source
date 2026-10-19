/*  Audio Source
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
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


AudioSource::~AudioSource(){}

AudioSource::AudioSource(Logger& logger, const std::string& file, AudioChannelFormat format, float volume_multiplier){
    QAudioFormat native_format;
    set_sample_format_to_float(native_format);
    set_format(native_format, format);
    init(format, AudioSampleFormat::FLOAT32, volume_multiplier);
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
    m_input_device = std::make_unique<AudioInputDevice>(logger, *m_reader, native_info, native_format);
}

void AudioSource::init(AudioChannelFormat format, AudioSampleFormat stream_format, float volume_multiplier){
    auto scope_check = m_sanitizer.check_scope();
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
