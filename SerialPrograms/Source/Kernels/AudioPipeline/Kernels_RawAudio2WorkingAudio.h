/*  Audio Pipeline - RawAudio2WorkingAudio
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AudioPipeline_RawAudio2WorkingAudio_H
#define PokemonAutomation_Kernels_AudioPipeline_RawAudio2WorkingAudio_H

#include <atomic>
#include <functional>
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/Containers/AlignedVector.h"
#include "Kernels_AudioPipeline.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{



class RawAudio2WorkingAudio : public RawAudioStreamListener{
public:
    ~RawAudio2WorkingAudio();
    RawAudio2WorkingAudio(
        SampleFormat sample_type,
        size_t sample_rate,
        size_t channels,
        float multiplier,
        size_t buffer_size = 4096 + 256 //  4k + break super-alignment
    );

    void add_listener(WorkingAudioStreamListener& listener);
    //  Attaches "listener" before all existing listeners (call order).
    void push_front_listener(WorkingAudioStreamListener& listener);
    void remove_listener(WorkingAudioStreamListener& listener);

    void set_multiplier(float multiplier);

    virtual void push_samples(size_t frames, const void* data) override;


private:
    void validate_listener(const WorkingAudioStreamListener& listener) const;

    template <typename Type>
    void set_converter();


private:
    std::atomic<float> m_multiplier;
    size_t m_buffer_size;
    std::function<void(const void* data, size_t frames)> m_convert = nullptr;

    AlignedVector<float> m_interleaved_floats;
    AlignedVector<float> m_split_floats;
    ListenerSet<WorkingAudioStreamListener> m_listeners;
};




}
}
}
#endif
