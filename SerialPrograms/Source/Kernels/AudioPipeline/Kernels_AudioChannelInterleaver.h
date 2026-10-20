/*  Audio Pipeline - AudioChannelInterleaver
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AudioPipeline_AudioChannelInterleaver_H
#define PokemonAutomation_Kernels_AudioPipeline_AudioChannelInterleaver_H

#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/Containers/AlignedVector.h"
#include "Kernels_AudioPipeline.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{



//
//  A streaming node that interleaves two working-format (float,
//  channel-strided) channels at R Hz into a single channel at 2R Hz:
//  output[2i] = channel 0[i], output[2i+1] = channel 1[i].
//
//  This is the exact inverse of the 1ch x 2R -> 2ch x R reinterpretation
//  (deinterleave) applied by the audio input nodes for the 96 kHz
//  capture-card presets, where a physical 1ch x 96000 stream is the two
//  48 kHz channels of a capture card. Used to restore the physical
//  1ch x 96000 stream for the MONO_96000 output device, which is still
//  opened in that shape by the legacy AudioSink.
//
class AudioChannelInterleaver : public WorkingAudioStreamListener{
public:
    ~AudioChannelInterleaver();
    AudioChannelInterleaver(
        size_t in_sample_rate,
        size_t buffer_size = 4096 + 256 //  4k + break super-alignment
    );

    size_t out_sample_rate() const{
        return 2 * m_in_rate;
    }

    void add_listener(WorkingAudioStreamListener& listener);
    void remove_listener(WorkingAudioStreamListener& listener);

    virtual void push_samples(size_t samples, size_t stride, const float* data) override;


private:
    const size_t m_in_rate;
    const size_t m_buffer_size;
    AlignedVector<float> m_buffer;
    ListenerSet<WorkingAudioStreamListener> m_listeners;
};



}
}
}
#endif
