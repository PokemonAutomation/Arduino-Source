/*  Audio Pipeline
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AudioPipeline_H
#define PokemonAutomation_Kernels_AudioPipeline_H

namespace PokemonAutomation{
namespace Kernels{
namespace AudioPipeline{



enum class SampleFormat{
    UINT8,
    SINT16,
    SINT32,
    FLOAT32,
};



class RawAudioStreamListener{
public:
    RawAudioStreamListener(
        SampleFormat sample_type,
        size_t sample_rate,
        size_t channels
    )
        : m_sample_type(sample_type)
        , m_channels(channels)
        , m_sample_rate(sample_rate)
    {}

    SampleFormat sample_type() const{
        return m_sample_type;
    }
    size_t channels() const{
        return m_channels;
    }
    size_t sample_rate() const{
        return m_sample_rate;
    }

    virtual void push_samples(size_t frames, const void* data) = 0;


private:
    const SampleFormat m_sample_type;
    const size_t m_channels;
    const size_t m_sample_rate;
};



class WorkingAudioStreamListener{
public:
    WorkingAudioStreamListener(
        size_t sample_rate,
        size_t channels
    )
        : m_channels(channels)
        , m_sample_rate(sample_rate)
    {}

    size_t channels() const{
        return m_channels;
    }
    size_t sample_rate() const{
        return m_sample_rate;
    }

    virtual void push_samples(size_t samples, size_t stride, const float* data) = 0;


private:
    const size_t m_channels;
    const size_t m_sample_rate;
};





}
}
}
#endif
