/*  Audio IO Device
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioIODevice_H
#define PokemonAutomation_AudioPipeline_AudioIODevice_H

#include <memory>
#include <set>
#include "Common/Cpp/SpinLock.h"
#include "AudioInfo.h"
#include "AudioPassthroughPair.h"

namespace PokemonAutomation{

class Logger;
class AudioSource;
class AudioSink;
class AudioFloatToFFT;
//struct AudioFloatStreamListener;
struct FFTListener;



//  A fully thread-safe class that holds the audio input and output devices.
//  You can asynchronously change the input and output devices.
//  This class will also broadcast FFT spectrums.
class AudioIODevice{
public:
    void add_listener(FFTListener& listener);
    void remove_listener(FFTListener& listener);


public:
    virtual ~AudioIODevice();
    AudioIODevice(Logger& logger);

    void clear_audio_source();
    void set_audio_source(const std::string& file);
    void set_audio_source(const AudioDeviceInfo& device, AudioChannelFormat format);

    void clear_audio_sink();
    void set_audio_sink(const AudioDeviceInfo& device, float volume);

    void set_sink_volume(float volume);


private:
    class SampleListener;
    class InternalFFTListener;

    void init_audio_sink();


private:
    Logger& m_logger;

    mutable SpinLock m_lock;

    AudioChannelFormat m_input_format;
    std::unique_ptr<AudioSource> m_reader;
    std::unique_ptr<SampleListener> m_sample_listener;

    AudioDeviceInfo m_output_device;
    float m_volume = 1.0;
    std::unique_ptr<AudioSink> m_writer;

    std::unique_ptr<AudioFloatToFFT> m_fft_runner;
    std::unique_ptr<InternalFFTListener> m_fft_listener;

    std::set<FFTListener*> m_listeners;
};



}
#endif
