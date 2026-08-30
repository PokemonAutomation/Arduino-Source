/*  Audio Passthrough Pair
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioPassthroughPair_H
#define PokemonAutomation_AudioPipeline_AudioPassthroughPair_H

#include "AudioInfo.h"

namespace PokemonAutomation{

struct AudioFloatStreamListener;
struct FFTListener;


//  Represents an audio source/sink pair where audio is passed through from
//  source -> sink with minimal latency.
//
//  If desired, listeners can be attached to receive the FFT spectrums.
//
//  This class is fully thread-safe. Both source and sink can be changed
//  asynchronously. Listeners can be attached/detached asynchronously.
//
class AudioPassthroughPair{
public:
    virtual void add_listener(AudioFloatStreamListener& listener) {}
    virtual void remove_listener(AudioFloatStreamListener& listener) {}

    virtual void add_listener(FFTListener& listener) {}
    virtual void remove_listener(FFTListener& listener) {}

public:
    virtual ~AudioPassthroughPair() = default;

    virtual void reset(
        const std::string& file,
        const AudioDeviceInfo& output, double output_volume
    ) {}
    virtual void reset(
        const AudioDeviceInfo& input, AudioChannelFormat format,
        const AudioDeviceInfo& output, double output_volume
    ) {}

    virtual void clear_audio_source() {}
    virtual void set_audio_source(const std::string& file) {}
    virtual void set_audio_source(const AudioDeviceInfo& device, AudioChannelFormat format) {}

    virtual void clear_audio_sink() {}
    virtual void set_audio_sink(const AudioDeviceInfo& device, double volume) {}

    virtual void set_sink_volume(double volume) {}    //  Volume Range: [0, 1.0]
};



}
#endif
