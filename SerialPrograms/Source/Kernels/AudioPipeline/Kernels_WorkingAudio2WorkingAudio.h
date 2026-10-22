/*  Audio Stream Converters
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_AudioStreamConverters_H
#define PokemonAutomation_AudioStreamConverters_H

#include <vector>
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/Containers/Pimpl.h"
#include "Kernels_AudioPipeline.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{



class WorkingAudio2WorkingAudio : public WorkingAudioNode{
public:
    ~WorkingAudio2WorkingAudio();
    WorkingAudio2WorkingAudio(
        Logger& logger,
        size_t in_sample_rate, size_t out_sample_rate,
        size_t in_channels, size_t out_channels,
        std::vector<float> weight_matrix,
        size_t buffer_size = 4096 + 256 //  4k + break super-alignment
    );

    virtual void add_listener(WorkingAudioStreamListener& listener) override;
    virtual void remove_listener(WorkingAudioStreamListener& listener) override;

    virtual void push_samples(size_t samples, size_t stride, const float* data) override;


private:
    struct Data;
    Pimpl<Data> m_data;
};



}
}
}
#endif
