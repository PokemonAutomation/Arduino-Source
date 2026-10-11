/*  Audio Pipeline - Kernels_WorkingAudio2RawAudio
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AudioPipeline_WorkingAudio2RawAudio_H
#define PokemonAutomation_Kernels_AudioPipeline_WorkingAudio2RawAudio_H

#include <functional>
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/Containers/AlignedVector.h"
#include "Kernels_AudioPipeline.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{



class WorkingAudio2RawAudio : public WorkingAudioStreamListener{
public:
    ~WorkingAudio2RawAudio();
    WorkingAudio2RawAudio(
        SampleFormat sample_type,
        size_t sample_rate,
        size_t channels,
        float multiplier,
        size_t buffer_size = 4096 + 256 //  4k + break super-alignment
    );

    void add_listener(RawAudioStreamListener& listener);
    void remove_listener(RawAudioStreamListener& listener);

    void set_multiplier(float multiplier);

    virtual void push_samples(size_t samples, size_t stride, const float* data) override;


private:
    template <typename Type>
    size_t set_converter();


private:
    SampleFormat m_sample_type;
    std::atomic<float> m_multiplier;
    size_t m_buffer_size;
    std::function<void(const void* data, size_t frames)> m_convert = nullptr;

    AlignedVector<char> m_interleaved_raw;
    AlignedVector<float> m_interleaved_floats;
    ListenerSet<RawAudioStreamListener> m_listeners;
};





}
}
}
#endif
