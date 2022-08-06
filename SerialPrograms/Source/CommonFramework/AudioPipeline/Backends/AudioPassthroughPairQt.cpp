/*  Audio Passthrough Pair (Qt)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/AudioPipeline/AudioConstants.h"
#include "CommonFramework/AudioPipeline/Tools/AudioFormatUtils.h"
#include "CommonFramework/AudioPipeline/IO/AudioSource.h"
#include "CommonFramework/AudioPipeline/IO/AudioSink.h"
#include "CommonFramework/AudioPipeline/Spectrum/FFTStreamer.h"
#include "AudioPassthroughPairQt.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



void AudioPassthroughPairQt::add_listener(FFTListener& listener){
    SpinLockGuard lg(m_lock);
    m_listeners.insert(&listener);
}
void AudioPassthroughPairQt::remove_listener(FFTListener& listener){
    SpinLockGuard lg(m_lock);
    m_listeners.erase(&listener);
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
        AudioPassthroughPairQt& parent = m_parent;
        SpinLockGuard lg(parent.m_lock);
        if (parent.m_writer){
            parent.m_writer->operator AudioFloatStreamListener&().on_samples(data, frames);
        }
        if (parent.m_fft_runner){
            parent.m_fft_runner->on_samples(data, frames);
        }

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
    virtual void on_fft(size_t sample_rate, std::shared_ptr<AlignedVector<float>> fft_output) override{
        //  This is already inside the lock.
//        SpinLockGuard lg(m_parent.m_lock);
        for (FFTListener* listener : m_parent.m_listeners){
            listener->on_fft(sample_rate, fft_output);
        }
    }

private:
    AudioPassthroughPairQt& m_parent;
};





AudioPassthroughPairQt::~AudioPassthroughPairQt(){}

AudioPassthroughPairQt::AudioPassthroughPairQt(Logger& logger)
    : m_logger(logger)
{}

void AudioPassthroughPairQt::reset(
    const std::string& file,
    const AudioDeviceInfo& output, float volume
){
    QMetaObject::invokeMethod(this, [=]{
        SpinLockGuard lg(m_lock);
        if (m_reader){
            m_fft_listener.reset();
            m_fft_runner.reset();
            m_writer.reset();
            m_sample_listener.reset();
            m_reader.reset();
            m_input_format = AudioChannelFormat::NONE;
        }
        m_input_format = AudioChannelFormat::DUAL_48000;
        m_reader.reset(new AudioSource(m_logger, file, m_input_format));
        m_sample_listener.reset(new SampleListener(*this, m_reader->samples_per_frame()));
        m_output_device = output;
        m_volume = volume;
        init_audio_sink();
        m_fft_runner = make_FFT_streamer(m_input_format);
        m_fft_listener.reset(new InternalFFTListener(*this));
    });
}
void AudioPassthroughPairQt::reset(
    const AudioDeviceInfo& input, AudioChannelFormat format,
    const AudioDeviceInfo& output, float volume
){
    QMetaObject::invokeMethod(this, [=]{
        SpinLockGuard lg(m_lock);
        if (m_reader){
            m_fft_listener.reset();
            m_fft_runner.reset();
            m_writer.reset();
            m_sample_listener.reset();
            m_reader.reset();
            m_input_format = AudioChannelFormat::NONE;
        }
        if (input){
            m_input_format = format;
            m_reader.reset(new AudioSource(m_logger, input, m_input_format));
            m_sample_listener.reset(new SampleListener(*this, m_reader->samples_per_frame()));
            m_output_device = output;
            m_volume = volume;
            init_audio_sink();
            m_fft_runner = make_FFT_streamer(m_input_format);
            m_fft_listener.reset(new InternalFFTListener(*this));
        }
    });
}
void AudioPassthroughPairQt::clear_audio_source(){
    QMetaObject::invokeMethod(this, [=]{
        SpinLockGuard lg(m_lock);
        if (m_reader){
            m_fft_listener.reset();
            m_fft_runner.reset();
            m_sample_listener.reset();
            m_reader.reset();
            m_input_format = AudioChannelFormat::NONE;
        }
    });
}
void AudioPassthroughPairQt::set_audio_source(const std::string& file){
    QMetaObject::invokeMethod(this, [=]{
        SpinLockGuard lg(m_lock);
        if (m_reader){
            m_fft_listener.reset();
            m_fft_runner.reset();
            m_writer.reset();
            m_sample_listener.reset();
            m_reader.reset();
            m_input_format = AudioChannelFormat::NONE;
        }
        m_input_format = AudioChannelFormat::DUAL_48000;
        m_reader.reset(new AudioSource(m_logger, file, m_input_format));
        m_sample_listener.reset(new SampleListener(*this, m_reader->samples_per_frame()));
        init_audio_sink();
        m_fft_runner = make_FFT_streamer(m_input_format);
        m_fft_listener.reset(new InternalFFTListener(*this));
    });
}
void AudioPassthroughPairQt::set_audio_source(const AudioDeviceInfo& device, AudioChannelFormat format){
    QMetaObject::invokeMethod(this, [=]{
        SpinLockGuard lg(m_lock);
        if (m_reader){
            m_fft_listener.reset();
            m_fft_runner.reset();
            m_writer.reset();
            m_sample_listener.reset();
            m_reader.reset();
            m_input_format = AudioChannelFormat::NONE;
        }
        if (!device){
            return;
        }
        m_input_format = format;
        m_reader.reset(new AudioSource(m_logger, device, m_input_format));
        m_sample_listener.reset(new SampleListener(*this, m_reader->samples_per_frame()));
        init_audio_sink();
        m_fft_runner = make_FFT_streamer(m_input_format);
        m_fft_listener.reset(new InternalFFTListener(*this));
    });
}

void AudioPassthroughPairQt::clear_audio_sink(){
    QMetaObject::invokeMethod(this, [=]{
        SpinLockGuard lg(m_lock);
        m_writer.reset();
        m_output_device = AudioDeviceInfo();
    });
}
void AudioPassthroughPairQt::set_audio_sink(const AudioDeviceInfo& device, float volume){
    QMetaObject::invokeMethod(this, [=]{
        SpinLockGuard lg(m_lock);
        m_writer.reset();
        m_output_device = device;
        m_volume = volume;
        init_audio_sink();
    });
}
void AudioPassthroughPairQt::init_audio_sink(){
    QMetaObject::invokeMethod(this, [=]{
        if (!m_output_device){
            return;
        }
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
        m_writer.reset(new AudioSink(m_logger, m_output_device, output_format, m_volume));
    });
}


void AudioPassthroughPairQt::set_sink_volume(float volume){
    QMetaObject::invokeMethod(this, [=]{
        SpinLockGuard lg(m_lock);
        m_volume = volume;
        if (m_writer){
            m_writer->set_volume(volume);
        }
    });
}







}
