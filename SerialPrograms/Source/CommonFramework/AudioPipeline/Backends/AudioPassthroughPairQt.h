/*  Audio Passthrough Pair (Qt)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioPassthroughPairQt_H
#define PokemonAutomation_AudioPipeline_AudioPassthroughPairQt_H

#include <memory>
#include <set>
#include <QObject>
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/LifetimeSanitizer.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/AudioPipeline/AudioPassthroughPair.h"

namespace PokemonAutomation{

class Logger;
class AudioSource;
class AudioSink;
class AudioFloatToFFT;


class AudioPassthroughPairQt final : public QObject, public AudioPassthroughPair{
public:
    virtual void add_listener(AudioFloatStreamListener& listener) override;
    virtual void remove_listener(AudioFloatStreamListener& listener) override;

    virtual void add_listener(FFTListener& listener) override;
    virtual void remove_listener(FFTListener& listener) override;


public:
    virtual ~AudioPassthroughPairQt();
    AudioPassthroughPairQt(Logger& logger);

    virtual void reset(
        const std::string& file,
        const AudioDeviceInfo& output, double output_volume
    ) override;
    virtual void reset(
        const AudioDeviceInfo& input, AudioChannelFormat format,
        const AudioDeviceInfo& output, double output_volume
    ) override;

    virtual void clear_audio_source() override;
    virtual void set_audio_source(const std::string& file) override;
    virtual void set_audio_source(const AudioDeviceInfo& device, AudioChannelFormat format) override;

    virtual void clear_audio_sink() override;
    virtual void set_audio_sink(const AudioDeviceInfo& device, double volume) override;

    virtual void set_sink_volume(double volume) override;


private:
    class SampleListener;
    class InternalFFTListener;

    void init_audio_sink();


private:
    Logger& m_logger;

    mutable SpinLock m_lock;

    //  The order of these parameters is important due to destruction order.
    //  Objects lower on this list attach to and hold references to those
    //  higher on the list.

    AudioChannelFormat m_input_format;
    std::unique_ptr<AudioSource> m_reader;
    std::unique_ptr<SampleListener> m_sample_listener;      //  Attaches to "m_reader".

    AudioDeviceInfo m_output_device;
    float m_file_input_multiplier = 1.0;
    float m_device_input_multiplier = 1.0;
    double m_output_volume = 1.0;
    std::unique_ptr<AudioSink> m_writer;

    std::unique_ptr<AudioFloatToFFT> m_fft_runner;
    std::unique_ptr<InternalFFTListener> m_fft_listener;    //  Attaches to m_fft_runner"".

    ListenerSet<AudioFloatStreamListener> m_stream_listeners;
    ListenerSet<FFTListener> m_fft_listeners;

    LifetimeSanitizer m_sanitizer;
};



}
#endif
