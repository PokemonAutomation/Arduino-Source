/*  Audio Passthrough Pair
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioPassthroughPair_H
#define PokemonAutomation_AudioPipeline_AudioPassthroughPair_H

#include "AudioInfo.h"

namespace PokemonAutomation{

struct FFTListener;

namespace Kernels{
namespace AudioPipeline{
class WorkingAudioStreamListener;
}
}


//  Represents an audio source/sink pair where audio is passed through from
//  source -> sink with minimal latency.
//
//  If desired, listeners can be attached to receive the FFT spectrums, or to
//  receive the working audio stream converted to a requested shape.
//
//  This class is fully thread-safe. Both source and sink can be changed
//  asynchronously. Listeners can be attached/detached asynchronously.
//
class AudioPassthroughPair{
public:
    virtual void add_listener(FFTListener& listener) = 0;
    virtual void remove_listener(FFTListener& listener) = 0;

    //  Attach a listener that receives the working audio stream converted to
    //  "format" (sample rate + channel count; the channel layout in
    //  format.native_format() drives the channel mixing). The pair builds a
    //  conversion node per listener; the listener receives planar float
    //  samples in the requested shape, with the input volume applied.
    //  Re-adding a listener (e.g. with a new format) rebuilds its node.
    virtual void add_listener(const AudioFormat& format, Kernels::AudioPipeline::WorkingAudioStreamListener& listener) = 0;
    virtual void remove_listener(Kernels::AudioPipeline::WorkingAudioStreamListener& listener) = 0;

public:
    virtual ~AudioPassthroughPair() = default;

    virtual void reset(
        const std::string& file,
        const AudioDeviceInfo& output, double output_volume
    ) = 0;
    virtual void reset(
        const AudioDeviceInfo& input, const AudioInputChoice& choice,
        const AudioDeviceInfo& output, double output_volume
    ) = 0;

    virtual void clear_audio_source() = 0;
    virtual void set_audio_source(const std::string& file) = 0;
    virtual void set_audio_source(const AudioDeviceInfo& device, const AudioInputChoice& choice) = 0;

    virtual void clear_audio_sink() = 0;
    virtual void set_audio_sink(const AudioDeviceInfo& device, double volume) = 0;

    virtual void set_sink_volume(double volume) = 0;    //  Volume Range: [0, 1.0]
};



}
#endif
