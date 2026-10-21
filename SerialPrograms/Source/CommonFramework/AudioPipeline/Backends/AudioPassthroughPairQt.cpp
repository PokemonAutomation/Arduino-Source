/*  Audio Passthrough Pair (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/AudioPipeline/AudioPipelineOptions.h"
#include "CommonFramework/AudioPipeline/Tools/AudioFormatUtils.h"
#include "CommonFramework/AudioPipeline/ChannelMixer.h"
#include "CommonFramework/AudioPipeline/IO/AudioSource.h"
#include "CommonFramework/AudioPipeline/IO/AudioSink.h"
#include "CommonFramework/AudioPipeline/Spectrum/FFTStreamer.h"
#include "Kernels/AudioPipeline/Kernels_RawAudio2WorkingAudio.h"
#include "Kernels/AudioPipeline/Kernels_WorkingAudio2RawAudio.h"
#include "Kernels/AudioPipeline/Kernels_ChannelMixer.h"
#include "Kernels/AudioResampler/Kernels_AudioResampler.h"
#include <QAudioFormat>
#if QT_VERSION_MAJOR == 5
#include <QAudioDeviceInfo>
#elif QT_VERSION_MAJOR == 6
#include <QAudioDevice>
#else
#error "Unsupported Qt version."
#endif
#include "AudioPassthroughPairQt.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


namespace{


Kernels::AudioPipeline::SampleFormat to_sample_format(AudioSampleFormat format){
    switch (format){
    case AudioSampleFormat::UINT8:   return Kernels::AudioPipeline::SampleFormat::UINT8;
    case AudioSampleFormat::SINT16:  return Kernels::AudioPipeline::SampleFormat::SINT16;
    case AudioSampleFormat::SINT32:  return Kernels::AudioPipeline::SampleFormat::SINT32;
    case AudioSampleFormat::FLOAT32: return Kernels::AudioPipeline::SampleFormat::FLOAT32;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unsupported audio sample format.");
    }
}


//  QAudioFormat with just a channel count and the standard layout for it
//  (1 = mono, 2 = stereo L+R). Enough for build_channel_mixer_matrix().
QAudioFormat channel_format(size_t channels){
    QAudioFormat format;
    format.setChannelCount((int)channels);
    format.setChannelConfig(
        channels == 1
            ? QAudioFormat::ChannelConfig::ChannelConfigMono
            : QAudioFormat::ChannelConfig::ChannelConfigStereo
    );
    return format;
}


}



void AudioPassthroughPairQt::add_listener(AudioFloatStreamListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    m_stream_listeners.add(listener);
}
void AudioPassthroughPairQt::remove_listener(AudioFloatStreamListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    m_stream_listeners.remove(listener);
}
void AudioPassthroughPairQt::add_listener(FFTListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
    m_fft_listeners.add(listener);
}
void AudioPassthroughPairQt::remove_listener(FFTListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
    m_fft_listeners.remove(listener);
}



class AudioPassthroughPairQt::SampleListener final : public AudioFloatStreamListener{
public:
    SampleListener(AudioPassthroughPairQt& parent, size_t samples_per_frame)
        : AudioFloatStreamListener(samples_per_frame)
        , m_parent(parent)
    {
        parent.m_reader->add_listener(*this);
    }
    ~SampleListener(){
        m_parent.m_reader->remove_listener(*this);
    }
    virtual void on_samples(const float* data, size_t frames) override{
        {
            AudioPassthroughPairQt& parent = m_parent;
            WriteSpinLock lg(parent.m_lock);
            if (parent.m_writer){
                AudioFloatStreamListener* listener = parent.m_writer->float_stream_listener();
                if (listener){
                    listener->on_samples(data, frames);
                }
            }
            if (parent.m_fft_runner){
                parent.m_fft_runner->on_samples(data, frames);
            }
        }
        m_parent.m_stream_listeners.run_method(
            &AudioFloatStreamListener::on_samples,
            data, frames
        );
    }

private:
    AudioPassthroughPairQt& m_parent;
};

class AudioPassthroughPairQt::InternalFFTListener final : public FFTListener{
public:
    InternalFFTListener(AudioPassthroughPairQt& parent)
        : m_parent(parent)
    {
        parent.m_fft_runner->add_listener(*this);
    }
    ~InternalFFTListener(){
        m_parent.m_fft_runner->remove_listener(*this);
    }
    virtual void on_fft(size_t sample_rate, std::shared_ptr<const AlignedVector<float>> fft_output) override{
        m_parent.m_fft_listeners.run_method(
            &FFTListener::on_fft,
            sample_rate, fft_output
        );
    }

private:
    AudioPassthroughPairQt& m_parent;
};


//
//  The new pipeline tree.
//
//  All three methods below require the caller to hold m_lock. Rebuilding
//  detaches the node heads from the decoder's listener set before the
//  nodes are destroyed; ListenerSet::remove blocks on in-flight pushes, so
//  no push can ever run through a node that is about to be destroyed.
//


void AudioPassthroughPairQt::teardown_tree(){
    if (m_source_decoder){
        m_reader->remove_raw_listener(*m_source_decoder);
    }
    m_sink_mixer.reset();
    m_sink_resampler.reset();
    m_sink_encoder.reset();
    m_fft_mixer.reset();
    m_fft_resampler.reset();
    m_source_decoder.reset();
}


void AudioPassthroughPairQt::teardown_sink_branch(){
    if (m_source_decoder){
        if (m_sink_mixer){
            m_source_decoder->remove_listener(*m_sink_mixer);
        }else if (m_sink_resampler){
            m_source_decoder->remove_listener(*m_sink_resampler);
        }else if (m_sink_encoder){
            m_source_decoder->remove_listener(*m_sink_encoder);
        }
    }
    m_sink_mixer.reset();
    m_sink_resampler.reset();
    m_sink_encoder.reset();
}


void AudioPassthroughPairQt::build_tree(float input_volume){
    if (!m_reader || m_input_format == AudioChannelFormat::NONE){
        return;
    }
    //  For the INTERLEAVE_96000 presets, m_reader reports the reinterpreted
    //  2ch x 48000 stream; MONO_96000 keeps the physical 1ch x 96000 shape
    //  (AudioInputFile / AudioInputDevice apply it).
    size_t raw_rate = m_reader->raw_sample_rate();
    size_t raw_channels = m_reader->raw_channels();

    m_source_decoder.reset(new Kernels::AudioPipeline::RawAudio2WorkingAudio(
        to_sample_format(m_reader->raw_sample_format()),
        raw_rate, raw_channels, input_volume
    ));
    m_reader->add_raw_listener(*m_source_decoder);

    //  FFT branch: source channels -> 1 channel, source rate -> FFT rate.
    //  The tree runs the FFT at 48 kHz for all inputs (the post-migration
    //  standard); a 44.1 kHz source is resampled up here.
    const size_t fft_rate = 48000;
    Kernels::AudioPipeline::WorkingAudioStreamListener* head = nullptr;
    if (raw_rate != fft_rate){
        m_fft_resampler.reset(new Kernels::AudioResampler::AudioResampler(raw_rate, fft_rate, 1));
        head = m_fft_resampler.get();
    }
    if (raw_channels != 1){
        auto matrix = build_channel_mixer_matrix(
            channel_format(raw_channels), channel_format(1)
        );
        if (matrix.empty()){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "No channel mixer matrix.");
        }
        m_fft_mixer.reset(new Kernels::AudioPipeline::ChannelMixer(
            raw_rate, raw_channels, 1, std::move(matrix)
        ));
        if (head){
            m_fft_mixer->add_listener(*head);
        }
        head = m_fft_mixer.get();
    }
    if (head){
        m_source_decoder->add_listener(*head);
    }

    //  The sink branch may have been created by the init_audio_sink() event
    //  queued before this one (which found no decoder yet); build it now.
    rebuild_sink_branch();
}


void AudioPassthroughPairQt::rebuild_sink_branch(){
    if (!m_reader || !m_source_decoder){
        return;
    }
    //  The sink branch mirrors the legacy writer: it exists only when the
    //  output device is open and actually accepts the target format.
    if (!m_writer || !m_writer->float_stream_listener()){
        return;
    }

    AudioChannelFormat output_format = m_input_format;
    switch (output_format){
    case AudioChannelFormat::INTERLEAVE_LR_96000:
    case AudioChannelFormat::INTERLEAVE_RL_96000:
        output_format = AudioChannelFormat::DUAL_48000;
        break;
    default:
        break;
    }
    size_t sink_rate;
    size_t sink_channels;
    switch (output_format){
    case AudioChannelFormat::MONO_48000:
        sink_rate = 48000;
        sink_channels = 1;
        break;
    case AudioChannelFormat::DUAL_44100:
        sink_rate = 44100;
        sink_channels = 2;
        break;
    case AudioChannelFormat::DUAL_48000:
        sink_rate = 48000;
        sink_channels = 2;
        break;
    case AudioChannelFormat::MONO_96000:
        sink_rate = 96000;
        sink_channels = 1;
        break;
    default:
        return;
    }

    NativeAudioInfo native_info = m_output_device.native_info();
    QAudioFormat target_format = native_info.preferredFormat();
    set_format(target_format, output_format);
    AudioSampleFormat sample_format = get_sample_format(target_format);
    if (sample_format == AudioSampleFormat::INVALID){
        sample_format = AudioSampleFormat::FLOAT32;
        set_sample_format_to_float(target_format);
    }

    size_t raw_rate = m_reader->raw_sample_rate();
    size_t raw_channels = m_reader->raw_channels();

    //  Detach the old sink head; the FFT head stays attached, and the new
    //  sink head is pushed to the front of the decoder's listener set below
    //  so it runs before the FFT head.
    teardown_sink_branch();

    Kernels::AudioPipeline::WorkingAudioStreamListener* head = nullptr;
    m_sink_encoder.reset(new Kernels::AudioPipeline::WorkingAudio2RawAudio(
        to_sample_format(sample_format),
        sink_rate, sink_channels,
        convert_audio_volume_from_slider(m_output_volume)
    ));
    head = m_sink_encoder.get();
    if (raw_rate != sink_rate){
        m_sink_resampler.reset(new Kernels::AudioResampler::AudioResampler(
            raw_rate, sink_rate, sink_channels
        ));
        m_sink_resampler->add_listener(*head);
        head = m_sink_resampler.get();
    }
    //  INTERLEAVE_RL_96000 plays the card's channel 1 (the odd physical
    //  samples) as the left channel; the input reinterpretation preserves
    //  the physical order, so the sink branch swaps the two channels.
    //  (The FFT pair-average is order-invariant.)
    //  Qt channel configs can't express a reversed channel order, so the
    //  swap matrix is literal for now; once the format encodes the
    //  card's channel mapping (dropdown rewrite),
    //  build_channel_mixer_matrix will derive it and this special case
    //  goes away.
    bool swap_channels =
        m_input_format == AudioChannelFormat::INTERLEAVE_RL_96000
        && raw_channels == 2 && sink_channels == 2;
    if (raw_channels != sink_channels || swap_channels){
        std::vector<float> matrix;
        if (swap_channels){
            //  out = [in 1, in 0]; weight(in i -> out o) at index i + in_ch * o.
            matrix = {0.0f, 1.0f, 1.0f, 0.0f};
        }else{
            matrix = build_channel_mixer_matrix(
                channel_format(raw_channels), channel_format(sink_channels)
            );
        }
        if (matrix.size() != raw_channels * sink_channels){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "No channel mixer matrix.");
        }
        m_sink_mixer.reset(new Kernels::AudioPipeline::ChannelMixer(
            raw_rate, raw_channels, sink_channels, std::move(matrix)
        ));
        m_sink_mixer->add_listener(*head);
        head = m_sink_mixer.get();
    }
    m_source_decoder->push_front_listener(*head);
}


AudioPassthroughPairQt::~AudioPassthroughPairQt(){
    auto scope_check = m_sanitizer.check_scope();
    WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
    teardown_tree();
}

AudioPassthroughPairQt::AudioPassthroughPairQt(Logger& logger)
    : m_logger(logger)
    , m_file_input_multiplier((float)GlobalSettings::instance().AUDIO_PIPELINE->FILE_VOLUME_SCALE)
    , m_device_input_multiplier((float)GlobalSettings::instance().AUDIO_PIPELINE->DEVICE_VOLUME_SCALE)
{}

void AudioPassthroughPairQt::reset(
    const std::string& file,
    const AudioDeviceInfo& output, double output_volume
){
    auto scope_check = m_sanitizer.check_scope();
    QMetaObject::invokeMethod(this, [this, file, output, output_volume]{
        auto scope_check = m_sanitizer.check_scope();
        WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
        if (m_reader){
            m_fft_listener.reset();
            m_fft_runner.reset();
            m_writer.reset();
            m_sample_listener.reset();
            teardown_tree();
            m_reader.reset();
            m_input_format = AudioChannelFormat::NONE;
        }
        m_input_format = AudioChannelFormat::DUAL_48000;
        m_reader.reset(new AudioSource(m_logger, file, m_input_format, m_file_input_multiplier));
        m_sample_listener.reset(new SampleListener(*this, m_reader->samples_per_frame()));
        m_output_device = output;
        m_output_volume = output_volume;
        init_audio_sink();
        m_fft_runner = make_FFT_streamer(m_input_format);
        m_fft_listener.reset(new InternalFFTListener(*this));
        build_tree(m_file_input_multiplier);
    });
}
void AudioPassthroughPairQt::reset(
    const AudioDeviceInfo& input, AudioChannelFormat format,
    const AudioDeviceInfo& output, double output_volume
){
    auto scope_check = m_sanitizer.check_scope();
//    cout << "AudioPassthroughPairQt::reset(): " << output.display_name() << endl;
    QMetaObject::invokeMethod(this, [this, format, output, output_volume, input]{
        auto scope_check = m_sanitizer.check_scope();
        WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
        if (m_reader){
            m_fft_listener.reset();
            m_fft_runner.reset();
            m_writer.reset();
            m_sample_listener.reset();
            teardown_tree();
            m_reader.reset();
            m_input_format = AudioChannelFormat::NONE;
        }
        m_input_format = format;
        m_output_device = output;
        m_output_volume = output_volume;
        if (input && format != AudioChannelFormat::NONE){
            m_reader.reset(new AudioSource(m_logger, input, m_input_format, m_device_input_multiplier));
            m_sample_listener.reset(new SampleListener(*this, m_reader->samples_per_frame()));
            init_audio_sink();
            m_fft_runner = make_FFT_streamer(m_input_format);
            m_fft_listener.reset(new InternalFFTListener(*this));
            build_tree(m_device_input_multiplier);
        }
    });
}
void AudioPassthroughPairQt::clear_audio_source(){
    auto scope_check = m_sanitizer.check_scope();
    QMetaObject::invokeMethod(this, [this]{
        auto scope_check = m_sanitizer.check_scope();
        WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
        if (m_reader){
            m_fft_listener.reset();
            m_fft_runner.reset();
            m_sample_listener.reset();
            teardown_tree();
            m_reader.reset();
            m_input_format = AudioChannelFormat::NONE;
        }
    });
}
void AudioPassthroughPairQt::set_audio_source(const std::string& file){
    auto scope_check = m_sanitizer.check_scope();
    QMetaObject::invokeMethod(this, [this, file]{
        auto scope_check = m_sanitizer.check_scope();
        WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
        if (m_reader){
            m_fft_listener.reset();
            m_fft_runner.reset();
            m_writer.reset();
            m_sample_listener.reset();
            teardown_tree();
            m_reader.reset();
            m_input_format = AudioChannelFormat::NONE;
        }
        m_input_format = AudioChannelFormat::DUAL_48000;
        m_reader.reset(new AudioSource(m_logger, file, m_input_format, m_file_input_multiplier));
        m_sample_listener.reset(new SampleListener(*this, m_reader->samples_per_frame()));
        init_audio_sink();
        m_fft_runner = make_FFT_streamer(m_input_format);
        m_fft_listener.reset(new InternalFFTListener(*this));
        build_tree(m_file_input_multiplier);
    });
}
void AudioPassthroughPairQt::set_audio_source(const AudioDeviceInfo& device, AudioChannelFormat format){
    auto scope_check = m_sanitizer.check_scope();
    QMetaObject::invokeMethod(this, [this, format, device]{
        auto scope_check = m_sanitizer.check_scope();
        WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
        if (m_reader){
            m_fft_listener.reset();
            m_fft_runner.reset();
            m_writer.reset();
            m_sample_listener.reset();
            teardown_tree();
            m_reader.reset();
            m_input_format = AudioChannelFormat::NONE;
        }
        m_input_format = format;
        if (device){
            m_reader.reset(new AudioSource(m_logger, device, m_input_format, m_device_input_multiplier));
            m_sample_listener.reset(new SampleListener(*this, m_reader->samples_per_frame()));
            init_audio_sink();
            m_fft_runner = make_FFT_streamer(m_input_format);
            m_fft_listener.reset(new InternalFFTListener(*this));
            build_tree(m_device_input_multiplier);
        }
    });
}

void AudioPassthroughPairQt::clear_audio_sink(){
    auto scope_check = m_sanitizer.check_scope();
    QMetaObject::invokeMethod(this, [this]{
        auto scope_check = m_sanitizer.check_scope();
        WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
        m_writer.reset();
        m_output_device = AudioDeviceInfo();
        teardown_sink_branch();
    });
}
void AudioPassthroughPairQt::set_audio_sink(const AudioDeviceInfo& device, double volume){
    auto scope_check = m_sanitizer.check_scope();
    QMetaObject::invokeMethod(this, [this, device, volume]{
        auto scope_check = m_sanitizer.check_scope();
        WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
        m_writer.reset();
        m_output_device = device;
        m_output_volume = volume;
        init_audio_sink();
    });
}
void AudioPassthroughPairQt::init_audio_sink(){
    //  Must be called under the lock.

//    cout << "AudioPassthroughPairQt::init_audio_sink()" << endl;
    auto scope_check = m_sanitizer.check_scope();
    AudioChannelFormat output_format = m_input_format;
    switch (output_format){
    case AudioChannelFormat::NONE:
        return;
    case AudioChannelFormat::MONO_48000:
    case AudioChannelFormat::MONO_96000:
    case AudioChannelFormat::DUAL_44100:
    case AudioChannelFormat::DUAL_48000:
        break;
    case AudioChannelFormat::INTERLEAVE_LR_96000:
    case AudioChannelFormat::INTERLEAVE_RL_96000:
        output_format = AudioChannelFormat::DUAL_48000;
        break;
    default:
        m_logger.log(std::string("Invalid AudioFormat: ") + AUDIO_FORMAT_LABELS[(size_t)output_format], COLOR_RED);
        return;
    }
    if (m_output_device){
        m_writer.reset(new AudioSink(m_logger, m_output_device, output_format, m_output_volume));
    }
    rebuild_sink_branch();
}


void AudioPassthroughPairQt::set_sink_volume(double volume){
    auto scope_check = m_sanitizer.check_scope();
    QMetaObject::invokeMethod(this, [this, volume]{
        auto scope_check = m_sanitizer.check_scope();
        WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
        m_output_volume = volume;
        if (m_writer){
            m_writer->set_volume(volume);
        }
        if (m_sink_encoder){
            m_sink_encoder->set_multiplier(convert_audio_volume_from_slider(volume));
        }
    });
}







}
