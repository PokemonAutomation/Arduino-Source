/*  Audio Passthrough Pair (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/AudioPipeline/AudioPipelineOptions.h"
#include "CommonFramework/AudioPipeline/Tools/AudioFormatUtils.h"
#include "CommonFramework/AudioPipeline/AudioStream.h"
#include "CommonFramework/AudioPipeline/ChannelMixer.h"
#include "CommonFramework/AudioPipeline/IO/AudioSource.h"
#include "CommonFramework/AudioPipeline/IO/AudioOutputDevice.h"
#include "CommonFramework/AudioPipeline/Spectrum/FFTStreamer.h"
#include "Kernels/AudioPipeline/Kernels_RawAudio2WorkingAudio.h"
#include "Kernels/AudioPipeline/Kernels_WorkingAudio2RawAudio.h"
#include "Kernels/AudioPipeline/Kernels_WorkingAudio2WorkingAudio.h"
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


//
//  Per-listener working-stream conversion.
//
//  (Defined before the first use: remove_listener() destroys a
//  WorkingListenerEntry - and with it the unique_ptr<InternalWorkingListener>
//  in it - at its call site, so the type must be complete there.)
//  Each attached working-stream listener gets its own
//  WorkingAudio2WorkingAudio node (source shape -> the listener's requested
//  shape) attached to the source decoder, plus an InternalWorkingListener
//  forwarder that carries the requested shape as its nominal one (the node
//  validates its listeners against it; the user's listener may have any
//  nominal shape of its own).
//
//  The entries in m_working_entries survive tree rebuilds: when the input
//  changes, build_tree() rebuilds each node for the new source shape, and
//  teardown_tree() detaches them. All of these methods require the caller
//  to hold m_lock. Detaching blocks on in-flight pushes, so no push can
//  ever run through a node or forwarder that is about to be destroyed.
//

class AudioPassthroughPairQt::InternalWorkingListener final : public Kernels::AudioPipeline::WorkingAudioStreamListener{
public:
    InternalWorkingListener(
        Kernels::AudioPipeline::WorkingAudioStreamListener& target,
        size_t sample_rate, size_t channels
    )
        : Kernels::AudioPipeline::WorkingAudioStreamListener(sample_rate, channels)
        , m_target(target)
    {}
    //  The pair always detaches this from its node before destroying it
    //  (detach_working_entry). "m_target" is the user's listener, owned by
    //  the caller; it outlives the attach by contract.
    virtual void push_samples(size_t samples, size_t stride, const float* data) override{
        m_target.push_samples(samples, stride, data);
    }

private:
    Kernels::AudioPipeline::WorkingAudioStreamListener& m_target;
};


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
void AudioPassthroughPairQt::add_listener(
    const AudioFormat& format, Kernels::AudioPipeline::WorkingAudioStreamListener& listener
){
    auto scope_check = m_sanitizer.check_scope();
    WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
    WorkingListenerEntry& entry = m_working_entries[&listener];
    entry.m_format = format;
    build_working_entry(entry, listener);
}
void AudioPassthroughPairQt::remove_listener(Kernels::AudioPipeline::WorkingAudioStreamListener& listener){
    auto scope_check = m_sanitizer.check_scope();
    WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
    auto iter = m_working_entries.find(&listener);
    if (iter == m_working_entries.end()){
        return;
    }
    detach_working_entry(iter->second);
    m_working_entries.erase(iter);
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
        parent.m_fft_node->add_listener(*this);
    }
    ~InternalFFTListener(){
        m_parent.m_fft_node->remove_listener(*this);
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




void AudioPassthroughPairQt::build_working_entry(
    WorkingListenerEntry& entry, Kernels::AudioPipeline::WorkingAudioStreamListener& listener
){
    if (!m_source_decoder || m_input_format == AudioChannelFormat::NONE){
        return;
    }
    //  Re-adding with a new format: detach the old node first.
    if (entry.m_node){
        detach_working_entry(entry);
    }

    const size_t raw_rate = m_reader->raw_sample_rate();
    const size_t raw_channels = m_reader->raw_channels();
    const size_t out_rate = entry.m_format.sample_rate();
    const size_t out_channels = entry.m_format.channels();
    if (out_rate == 0 || out_channels == 0){
        m_logger.log("Working listener: invalid requested format; not attached.", COLOR_RED);
        return;
    }

    //  INTERLEAVE_RL_96000 plays the card's channel 1 (the odd physical
    //  samples) as the left channel; the input reinterpretation preserves
    //  the physical order, so the swap is applied to the matrix (the same
    //  special case the sink branch uses).
    std::vector<float> matrix;
    if (m_input_format == AudioChannelFormat::INTERLEAVE_RL_96000
        && raw_channels == 2 && out_channels == 2){
        //  out = [in 1, in 0]; weight(in i -> out o) at index i + in_ch * o.
        matrix = {0.0f, 1.0f, 1.0f, 0.0f};
    }else{
        matrix = build_channel_mixer_matrix(
            channel_format(raw_channels), entry.m_format.native_format()
        );
    }
    if (matrix.size() != raw_channels * out_channels){
        m_logger.log("Working listener: no channel mixer matrix for this conversion; not attached.", COLOR_RED);
        return;
    }

    entry.m_node.reset(new Kernels::AudioPipeline::WorkingAudio2WorkingAudio(
        m_logger, raw_rate, out_rate, raw_channels, out_channels, std::move(matrix)
    ));
    entry.m_forwarder.reset(new InternalWorkingListener(listener, out_rate, out_channels));
    entry.m_node->add_listener(*entry.m_forwarder);
    m_source_decoder->add_listener(*entry.m_node);
}


void AudioPassthroughPairQt::detach_working_entry(WorkingListenerEntry& entry){
    if (entry.m_forwarder && entry.m_node){
        entry.m_node->remove_listener(*entry.m_forwarder);
    }
    if (entry.m_node && m_source_decoder){
        m_source_decoder->remove_listener(*entry.m_node);
    }
    entry.m_forwarder.reset();
    entry.m_node.reset();
}



//
//  The new pipeline tree.
//
//  All three methods below require the caller to hold m_lock. Rebuilding
//  detaches the node heads from the decoder's listener set before the
//  nodes are destroyed; ListenerSet::remove blocks on in-flight pushes, so
//  no push can ever run through a node that is about to be destroyed.
//


void AudioPassthroughPairQt::teardown_tree(){
    //  Destroy the tree's leaves first and propagate toward the root:
    //  the FFT listener is registered on m_fft_node, and the branch
    //  heads are registered on the decoder. Detaching the decoder from
    //  the reader's raw stream first quiesces the tree, so none of the
    //  destructions below can ever race an in-flight push.
    if (m_source_decoder){
        m_reader->remove_raw_listener(*m_source_decoder);
    }
    for (auto& entry : m_working_entries){
        detach_working_entry(entry.second);
    }
    m_fft_listener.reset();
    m_fft_node.reset();
    m_fft_conv.reset();
    teardown_sink_branch();
    m_source_decoder.reset();
}


void AudioPassthroughPairQt::teardown_sink_branch(){
    if (m_source_decoder){
        if (m_sink_conv){
            m_source_decoder->remove_listener(*m_sink_conv);
        }else if (m_sink_encoder){
            m_source_decoder->remove_listener(*m_sink_encoder);
        }
    }
    //  Detach the device from the encoder's listener set before the
    //  encoder is destroyed; remove blocks on in-flight pushes.
    if (m_sink_encoder && m_sink_device){
        m_sink_encoder->remove_listener(*m_sink_device);
    }
    m_sink_conv.reset();
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

    //  Rebuild the per-listener working-stream conversion nodes for the new
    //  source shape (the entries themselves survive the rebuild).
    for (auto& entry : m_working_entries){
        build_working_entry(entry.second, *entry.first);
    }

    //  FFT branch: source channels -> 1 channel, source rate -> FFT rate.
    //  The tree runs the FFT at 48 kHz for all inputs (the post-migration
    //  standard); a 44.1 kHz source is resampled up here. The branch ends
    //  in m_fft_node, the tree's FFT endpoint.
    const size_t fft_rate = 48000;
    m_fft_node.reset(new WorkingAudioToFFT(fft_rate));
    if (raw_rate == fft_rate && raw_channels == 1){
        //  No conversion needed: feed the FFT node directly.
        m_source_decoder->add_listener(*m_fft_node);
    }else{
        std::vector<float> matrix = build_channel_mixer_matrix(
            channel_format(raw_channels), channel_format(1)
        );
        if (matrix.size() != raw_channels){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "No channel mixer matrix.");
        }
        m_fft_conv.reset(new Kernels::AudioPipeline::WorkingAudio2WorkingAudio(
            m_logger, raw_rate, fft_rate, raw_channels, 1, std::move(matrix)
        ));
        m_fft_conv->add_listener(*m_fft_node);
        m_source_decoder->add_listener(*m_fft_conv);
    }

    //  The sink branch may have been created by the init_audio_sink() call
    //  that ran earlier in the same locked scope (which found no decoder
    //  yet); build it now.
    rebuild_sink_branch();
}


void AudioPassthroughPairQt::rebuild_sink_branch(){
    if (!m_reader || !m_source_decoder){
        return;
    }
    //  The sink branch exists only when the output device is open; it
    //  plays in the device's native format (as opened by
    //  init_audio_sink), and this branch resamples / mixes / converts
    //  the working input to that shape.
    if (!m_sink_device){
        return;
    }

    const size_t sink_rate = m_sink_device->sample_rate();
    const size_t sink_channels = m_sink_device->channels();
    const Kernels::AudioPipeline::SampleFormat sink_sample_format = m_sink_device->sample_type();

    size_t raw_rate = m_reader->raw_sample_rate();
    size_t raw_channels = m_reader->raw_channels();

    //  Detach the old sink head; the FFT head stays attached, and the new
    //  sink head is pushed to the front of the decoder's listener set below
    //  so it runs before the FFT head.
    teardown_sink_branch();

    m_sink_encoder.reset(new Kernels::AudioPipeline::WorkingAudio2RawAudio(
        sink_sample_format,
        sink_rate, sink_channels,
        convert_audio_volume_from_slider(m_output_volume)
    ));
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
    if (raw_rate == sink_rate && raw_channels == sink_channels && !swap_channels){
        //  No conversion needed: feed the encoder directly.
        m_source_decoder->push_front_listener(*m_sink_encoder);
    }else{
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
        m_sink_conv.reset(new Kernels::AudioPipeline::WorkingAudio2WorkingAudio(
            m_logger, raw_rate, sink_rate, raw_channels, sink_channels, std::move(matrix)
        ));
        m_sink_conv->add_listener(*m_sink_encoder);
        m_source_decoder->push_front_listener(*m_sink_conv);
    }

    //  The branch's open tail: raw bytes to the output device.
    m_sink_encoder->add_listener(*m_sink_device);
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
            teardown_tree();
            m_sink_device.reset();
            m_sample_listener.reset();
            m_reader.reset();
            m_input_format = AudioChannelFormat::NONE;
        }
        m_input_format = AudioChannelFormat::DUAL_48000;
        m_reader.reset(new AudioSource(m_logger, file, m_input_format, m_file_input_multiplier));
        m_sample_listener.reset(new SampleListener(*this, m_reader->samples_per_frame()));
        m_output_device = output;
        m_output_volume = output_volume;
        init_audio_sink();
        build_tree(m_file_input_multiplier);
        m_fft_listener.reset(new InternalFFTListener(*this));
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
            teardown_tree();
            m_sink_device.reset();
            m_sample_listener.reset();
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
            build_tree(m_device_input_multiplier);
            m_fft_listener.reset(new InternalFFTListener(*this));
        }
    });
}
void AudioPassthroughPairQt::clear_audio_source(){
    auto scope_check = m_sanitizer.check_scope();
    QMetaObject::invokeMethod(this, [this]{
        auto scope_check = m_sanitizer.check_scope();
        WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
        if (m_reader){
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
            teardown_tree();
            m_sink_device.reset();
            m_sample_listener.reset();
            m_reader.reset();
            m_input_format = AudioChannelFormat::NONE;
        }
        m_input_format = AudioChannelFormat::DUAL_48000;
        m_reader.reset(new AudioSource(m_logger, file, m_input_format, m_file_input_multiplier));
        m_sample_listener.reset(new SampleListener(*this, m_reader->samples_per_frame()));
        init_audio_sink();
        build_tree(m_file_input_multiplier);
        m_fft_listener.reset(new InternalFFTListener(*this));
    });
}
void AudioPassthroughPairQt::set_audio_source(const AudioDeviceInfo& device, AudioChannelFormat format){
    auto scope_check = m_sanitizer.check_scope();
    QMetaObject::invokeMethod(this, [this, format, device]{
        auto scope_check = m_sanitizer.check_scope();
        WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
        if (m_reader){
            teardown_tree();
            m_sink_device.reset();
            m_sample_listener.reset();
            m_reader.reset();
            m_input_format = AudioChannelFormat::NONE;
        }
        m_input_format = format;
        if (device){
            m_reader.reset(new AudioSource(m_logger, device, m_input_format, m_device_input_multiplier));
            m_sample_listener.reset(new SampleListener(*this, m_reader->samples_per_frame()));
            init_audio_sink();
            build_tree(m_device_input_multiplier);
            m_fft_listener.reset(new InternalFFTListener(*this));
        }
    });
}

void AudioPassthroughPairQt::clear_audio_sink(){
    auto scope_check = m_sanitizer.check_scope();
    QMetaObject::invokeMethod(this, [this]{
        auto scope_check = m_sanitizer.check_scope();
        WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
        m_output_device = AudioDeviceInfo();
        teardown_sink_branch();
        m_sink_device.reset();
    });
}
void AudioPassthroughPairQt::set_audio_sink(const AudioDeviceInfo& device, double volume){
    auto scope_check = m_sanitizer.check_scope();
    QMetaObject::invokeMethod(this, [this, device, volume]{
        auto scope_check = m_sanitizer.check_scope();
        WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
        teardown_sink_branch();
        m_sink_device.reset();
        m_output_device = device;
        m_output_volume = volume;
        init_audio_sink();
    });
}
void AudioPassthroughPairQt::init_audio_sink(){
    //  Must be called under the lock.

//    cout << "AudioPassthroughPairQt::init_audio_sink()" << endl;
    auto scope_check = m_sanitizer.check_scope();
    if (m_input_format == AudioChannelFormat::NONE){
        return;
    }
    if (!m_output_device){
        return;
    }

    //  Open the device in its native format, as-is: the preferred rate,
    //  channel count, and sample format of the device. The tree's sink
    //  branch resamples / mixes / converts the working input to that
    //  shape. The output volume is applied by the branch's encoder, so
    //  the device stays at full volume.
    NativeAudioInfo native_info = m_output_device.native_info();
    QAudioFormat target_format = native_info.preferredFormat();
    AudioSampleFormat sample_format = get_sample_format(target_format);
    if (sample_format == AudioSampleFormat::INVALID){
        //  The native sample format isn't one we can convert to; fall
        //  back to a supported one we can.
        QAudioFormat float_format = target_format;
        set_sample_format_to_float(float_format);
        if (native_info.isFormatSupported(float_format)){
            target_format = float_format;
            sample_format = AudioSampleFormat::FLOAT32;
        }else{
            QAudioFormat sint16_format = target_format;
            sint16_format.setSampleFormat(QAudioFormat::SampleFormat::Int16);
            if (native_info.isFormatSupported(sint16_format)){
                target_format = sint16_format;
                sample_format = AudioSampleFormat::SINT16;
            }else{
                m_logger.log(
                    "Audio output device's native sample format is not supported, "
                    "and neither float nor 16-bit is. Audio output is disabled.",
                    COLOR_RED
                );
                return;
            }
        }
    }
    m_logger.log("AudioOutputDevice(): Target: " + dump_audio_format(target_format));
    m_logger.log("AudioOutputDevice(): Native: " + dump_audio_format(native_info.preferredFormat()));
    m_sink_device.reset(new AudioOutputDevice(
        m_logger, native_info, target_format,
        to_sample_format(sample_format),
        (size_t)target_format.sampleRate(),
        (size_t)target_format.channelCount()
    ));
    rebuild_sink_branch();
}


void AudioPassthroughPairQt::set_sink_volume(double volume){
    auto scope_check = m_sanitizer.check_scope();
    QMetaObject::invokeMethod(this, [this, volume]{
        auto scope_check = m_sanitizer.check_scope();
        WriteSpinLock lg(m_lock, PA_CURRENT_FUNCTION);
        m_output_volume = volume;
        if (m_sink_encoder){
            float absolute = convert_audio_volume_from_slider(volume);
            m_logger.log("Volume set to: Slider = " + tostr_default(volume) + " -> Absolute = " + tostr_default(absolute));
            m_sink_encoder->set_multiplier(absolute);
        }
    });
}







}
