/*  Audio Source
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      AudioSource represents an audio input stream. Once constructed,
 *  it spits out the float samples. Listeners can attach themselves to this
 *  class to receive these audio frames.
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioSource_H
#define PokemonAutomation_AudioPipeline_AudioSource_H

#include <memory>
#include <set>
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/LifetimeSanitizer.h"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "CommonFramework/AudioPipeline/AudioInfo.h"
#include "CommonFramework/AudioPipeline/AudioStream.h"

namespace PokemonAutomation{

class Logger;
class AudioStreamToFloat;
class AudioInputFile;
class AudioInputDevice;


class AudioSource{
public:
    void add_listener(AudioFloatStreamListener& listener);
    void remove_listener(AudioFloatStreamListener& listener);

public:
    ~AudioSource();

    //  Read from an audio file. (i.e. .wav or .mp3)
    AudioSource(Logger& logger, const std::string& file, AudioChannelFormat format, float volume_multiplier);

    //  Read from an audio input device. (i.e. capture card)
    AudioSource(Logger& logger, const AudioDeviceInfo& device, AudioChannelFormat format, float volume_multiplier);

    size_t sample_rate() const{ return m_sample_rate; }
    size_t channels() const{ return m_channels; }
    size_t samples_per_frame() const{ return m_channels * m_multiplier; }

private:
    void init(AudioChannelFormat format, AudioSampleFormat stream_format, float volume_multiplier);

private:
    class InternalListener;

    size_t m_sample_rate;
    size_t m_channels;
    size_t m_multiplier;

    SpinLock m_lock;

    std::unique_ptr<InternalListener> m_internal_listener;
    std::unique_ptr<AudioStreamToFloat> m_reader;

    std::unique_ptr<AudioInputFile> m_input_file;
    std::unique_ptr<AudioInputDevice> m_input_device;

    ListenerSet<AudioFloatStreamListener> m_listeners;

    LifetimeSanitizer m_sanitizer;
};



}
#endif
