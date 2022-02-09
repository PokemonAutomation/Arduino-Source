/*  Time Sample Buffer Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_AudioPipeline_TimeSampleBufferReader_H
#define PokemonAutomation_CommonFramework_AudioPipeline_TimeSampleBufferReader_H

#include "TimeSampleBuffer.h"

namespace PokemonAutomation{


template <typename Type>
class TimeSampleBufferReader{
    using TimePoint = std::chrono::system_clock::time_point;
    using Duration = std::chrono::system_clock::duration;

public:
    TimeSampleBufferReader(TimeSampleBuffer<Type>& buffer);

    void set_to_timestamp(TimePoint timestamp = std::chrono::system_clock::now());

    void read_samples(
        Type* samples, size_t count,
        TimePoint timestamp = std::chrono::system_clock::now()
    );

private:
    void set_to_timestamp_unprotected(TimePoint timestamp = std::chrono::system_clock::now());

public:
    TimeSampleBuffer<Type>& m_buffer;

//    TimePoint m_last_timestamp;

    //  Last read sample.
    TimePoint m_current_block;
    size_t m_current_index;
};



}
#endif
