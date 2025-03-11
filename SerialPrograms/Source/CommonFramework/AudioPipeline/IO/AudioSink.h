/*  Audio Sink
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      AudioSink represents an audio output stream. Once constructed,
 *  you can push float audio samples to it and it will play it from the
 *  respective speaker or whatever.
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioSink_H
#define PokemonAutomation_AudioPipeline_AudioSink_H

#include <memory>
#include "Common/Cpp/LifetimeSanitizer.h"
#include "CommonFramework/AudioPipeline/AudioInfo.h"
#include "CommonFramework/AudioPipeline/AudioStream.h"

namespace PokemonAutomation{

class Logger;
class AudioOutputDevice;


class AudioSink{
public:
    ~AudioSink();
    AudioSink(Logger& logger, const AudioDeviceInfo& device, AudioChannelFormat format, double volume);

    size_t sample_rate() const{ return m_sample_rate; }
    size_t channels() const{ return m_channels; }
    size_t samples_per_frame() const{ return m_channels * m_multiplier; }

    AudioFloatStreamListener* float_stream_listener();

    void set_volume(double volume);


private:
    size_t m_sample_rate;
    size_t m_channels;
    size_t m_multiplier;

    std::unique_ptr<AudioOutputDevice> m_writer;

    LifetimeSanitizer m_sanitizer;
};



}
#endif
