/*  Audio Source
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      AudioSource represents an audio input stream. Once constructed, it
 *  emits the raw frame-aligned stream to its RawAudioStreamListener
 *  children (root of the pipeline tree).
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioSource_H
#define PokemonAutomation_AudioPipeline_AudioSource_H

#include "CommonFramework/AudioPipeline/AudioFormat.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{
    class RawAudioStreamListener;
    class WorkingAudioStreamListener;
}
}


class AudioSource{
public:
    virtual ~AudioSource() = default;


public:
    using RawAudioStreamListener = Kernels::AudioPipeline::RawAudioStreamListener;
    using WorkingAudioStreamListener = Kernels::AudioPipeline::WorkingAudioStreamListener;

    virtual void add_raw_listener(RawAudioStreamListener& listener) = 0;
    virtual void remove_raw_listener(RawAudioStreamListener& listener) = 0;


public:
    //  Raw stream (pre-conversion) access. The raw stream is the source's
    //  native frame-aligned output, before the pipeline tree applies any
    //  conversion (resample / mix / sample-type / volume).
    virtual AudioSampleFormat raw_sample_format() const = 0;

    virtual size_t raw_sample_rate() const = 0;
    virtual size_t raw_channels() const = 0;

    virtual float volume_multiplier() const = 0;
};



}
#endif
