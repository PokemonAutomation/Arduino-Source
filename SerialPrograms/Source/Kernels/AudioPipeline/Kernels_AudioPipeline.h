/*  Audio Pipeline
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AudioPipeline_H
#define PokemonAutomation_Kernels_AudioPipeline_H

#include <cstddef>

namespace PokemonAutomation{

class UnitTestDatabase;

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
    virtual ~RawAudioStreamListener() = default;

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
    virtual ~WorkingAudioStreamListener() = default;

    WorkingAudioStreamListener(
        size_t sample_rate,
        size_t channels
    )
        : m_in_sample_rate(sample_rate)
        , m_in_channels(channels)
    {}

    size_t sample_rate() const{
        return m_in_sample_rate;
    }
    size_t channels() const{
        return m_in_channels;
    }

    virtual void push_samples(size_t samples, size_t stride, const float* data) = 0;


private:
    const size_t m_in_sample_rate;
    const size_t m_in_channels;
};


class WorkingAudioNode : public WorkingAudioStreamListener{
public:
    virtual ~WorkingAudioNode() = default;
    WorkingAudioNode(
        size_t in_sample_rate, size_t out_sample_rate,
        size_t in_channels, size_t out_channels
    )
        : WorkingAudioStreamListener(in_sample_rate, in_channels)
        , m_out_sample_rate(out_sample_rate)
        , m_out_channels(out_channels)
    {}

    virtual void add_listener(WorkingAudioStreamListener& listener) = 0;
    virtual void remove_listener(WorkingAudioStreamListener& listener) = 0;

    size_t in_sample_rate() const{
        return WorkingAudioStreamListener::sample_rate();
    }
    size_t out_sample_rate() const{
        return m_out_sample_rate;
    }
    size_t in_channels() const{
        return WorkingAudioStreamListener::channels();
    }
    size_t out_channels() const{
        return m_out_channels;
    }


private:
    const size_t m_out_sample_rate;
    const size_t m_out_channels;
};





void add_tests(UnitTestDatabase& database);




}
}
}
#endif
