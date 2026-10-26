/*  Audio Source
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      AudioSource represents a running audio input stream. Once
 *  constructed, it emits the raw frame-aligned stream to its
 *  RawAudioStreamListener children (root of the pipeline tree).
 *
 *      The concrete subclasses are AudioInputFile (a file, .wav / .mp3 /
 *  ...) and AudioInputDevice (a capture device). They are constructed from
 *  an AudioSourceDescriptor (the input's state) on the pair's event thread
 *  and owned by the pair.
 *
 */

#ifndef PokemonAutomation_AudioPipeline_AudioSource_H
#define PokemonAutomation_AudioPipeline_AudioSource_H

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/LifetimeSanitizer.h"
#include "CommonFramework/AudioPipeline/AudioInfo.h"
#include "Kernels/AudioPipeline/Kernels_AudioPipeline.h"

namespace PokemonAutomation{


class AudioSource{
public:
    virtual ~AudioSource() = default;

    //  Raw stream (pre-conversion) shape. The raw stream is the source's
    //  native frame-aligned output, before the pipeline tree applies any
    //  conversion (resample / mix / sample-type / volume).
    virtual AudioSampleFormat raw_sample_format() const = 0;
    virtual size_t raw_sample_rate() const = 0;
    virtual size_t raw_channels() const = 0;

    //  Raw stream access. The listener's nominal shape (sample type, rate,
    //  channel count) must match the raw stream's shape.
    virtual void add_raw_listener(Kernels::AudioPipeline::RawAudioStreamListener& listener){
        auto scope_check = m_sanitizer.check_scope();
        if (!listener_shape_matches(listener)){
            throw InternalProgramError(
                nullptr, PA_CURRENT_FUNCTION,
                "Listener shape doesn't match the raw stream's shape."
            );
        }
        m_listeners.add(listener);
    }
    virtual void remove_raw_listener(Kernels::AudioPipeline::RawAudioStreamListener& listener){
        auto scope_check = m_sanitizer.check_scope();
        m_listeners.remove(listener);
    }

protected:
    AudioSource() = default;

    //  Does the listener's nominal shape (sample type, rate, channel
    //  count) match this source's raw stream shape?
    virtual bool listener_shape_matches(const Kernels::AudioPipeline::RawAudioStreamListener& listener) const = 0;

    ListenerSet<Kernels::AudioPipeline::RawAudioStreamListener> m_listeners;
    LifetimeSanitizer m_sanitizer;
};



}
#endif
