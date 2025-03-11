/*  Time Sample Writer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonFramework_AudioPipeline_TimeSampleWriter_H
#define PokemonAutomation_CommonFramework_AudioPipeline_TimeSampleWriter_H

#include <string.h>
#include <algorithm>

namespace PokemonAutomation{


template <typename Type>
class TimeSampleWriterForward{
public:
    TimeSampleWriterForward(Type* output, size_t count)
        : m_output(output)
        , m_samples_left(count)
    {}

    //  # of samples still left to fill before the output buffer is full.
    size_t samples_left() const{ return m_samples_left; }

    //  Push the current block of samples into the output buffer.
    //  Returns the # of samples actually pushed.
    size_t push_block(const Type* samples, size_t count){
        size_t block = std::min(m_samples_left, count);
        memcpy(m_output, samples, block * sizeof(Type));
        m_output += block;
        m_samples_left -= block;
        return block;
    }

    //  Push zeros into the output buffer.
    //  Returns the # of samples actually pushed.
    size_t push_zeros(size_t count){
        size_t block = std::min(m_samples_left, count);
        memset(m_output, 0, block * sizeof(Type));
        m_output += block;
        m_samples_left -= block;
        return block;
    }

    void fill_rest_with_zeros(){
        memset(m_output, 0, m_samples_left * sizeof(Type));
        m_output += m_samples_left;
        m_samples_left = 0;
    }


private:
    Type* m_output;
    size_t m_samples_left;
};



template <typename Type>
class TimeSampleWriterReverse{
public:
    TimeSampleWriterReverse(Type* output, size_t count)
        : m_output(output + count)
        , m_samples_left(count)
    {}

    //  # of samples still left to fill before the output buffer is full.
    size_t samples_left() const{ return m_samples_left; }

    //  Push the current block of samples into the output buffer.
    //  Returns the # of samples actually pushed.
    size_t push_block(const Type* samples, size_t count){
        size_t block = std::min(m_samples_left, count);
        memcpy(m_output - block, samples + count - block, block * sizeof(Type));
        m_output -= block;
        m_samples_left -= block;
        return block;
    }

    //  Push zeros into the output buffer.
    //  Returns the # of samples actually pushed.
    size_t push_zeros(size_t count){
        size_t block = std::min(m_samples_left, count);
        memset(m_output - block, 0, block * sizeof(Type));
        m_output -= block;
        m_samples_left -= block;
        return block;
    }

    void fill_rest_with_zeros(){
        m_output -= m_samples_left;
        memset(m_output, 0, m_samples_left * sizeof(Type));
        m_samples_left = 0;
    }


private:
    Type* m_output;
    size_t m_samples_left;
};



}
#endif
