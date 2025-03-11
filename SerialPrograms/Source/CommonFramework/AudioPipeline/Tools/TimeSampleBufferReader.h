/*  Time Sample Buffer Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonFramework_AudioPipeline_TimeSampleBufferReader_H
#define PokemonAutomation_CommonFramework_AudioPipeline_TimeSampleBufferReader_H

#include "TimeSampleBuffer.h"

namespace PokemonAutomation{


template <typename Type>
class TimeSampleBufferReader{
    using Duration = std::chrono::system_clock::duration;

public:
    TimeSampleBufferReader(TimeSampleBuffer<Type>& buffer);

    void set_to_timestamp(WallClock timestamp = current_time());

    void read_samples(
        Type* samples, size_t count,
        WallClock timestamp = current_time()
    );

private:
    void set_to_timestamp_unprotected(WallClock timestamp = current_time());

public:
    TimeSampleBuffer<Type>& m_buffer;

//    TimePoint m_last_timestamp;

    //  Last read sample.
    WallClock m_current_block;
    size_t m_current_index;
};



}
#endif
