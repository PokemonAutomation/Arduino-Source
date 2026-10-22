/*  Audio Pipeline - ChannelMixer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AudioPipeline_ChannelMixer_H
#define PokemonAutomation_Kernels_AudioPipeline_ChannelMixer_H

#include <vector>
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/Containers/AlignedVector.h"
#include "Kernels_AudioPipeline.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{



class ChannelMixer : public WorkingAudioNode{
public:
    ~ChannelMixer();
    ChannelMixer(
        size_t sample_rate,
        size_t in_channels,
        size_t out_channels,
        std::vector<float> weight_matrix,
        size_t buffer_size = 4096 + 256 //  4k + break super-alignment
    );

    virtual void add_listener(WorkingAudioStreamListener& listener) override;
    virtual void remove_listener(WorkingAudioStreamListener& listener) override;

    virtual void push_samples(size_t samples, size_t stride, const float* data) override;


private:
    size_t m_buffer_size;
    std::vector<float> m_weight_matrix;
    AlignedVector<float> m_buffer;
    ListenerSet<WorkingAudioStreamListener> m_listeners;
};




}
}
}
#endif
