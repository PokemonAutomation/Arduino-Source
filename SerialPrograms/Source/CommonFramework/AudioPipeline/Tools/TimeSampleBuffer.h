/*  Time Sample Buffer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 *  This is a buffer for raw audio samples taken from an audio input.
 *
 *  It will store at least "history" worth of samples before dropping old data.
 *
 *  This class isn't just a trivial circular buffer. It allows random write
 *  and random read access. Reads will intelligently try to construct a
 *  contiguous audio stream. If it can't, it will insert zeros or drop samples
 *  where appropriate to correct it.
 *
 *  This class in effect, is fault-tolerant to interruptions by both the writer
 *  and reader(s). Gaps or delays will self-correct to avoid a situation where
 *  the reader(s) perpetually fall behind or get ahead of the writers. Drifts
 *  where the writer/reader(s) are of slightly different speeds will also be
 *  corrected.
 *
 *
 *  Both read and write access to the buffer is done by blocks with a timestamp
 *  indicating the approximate time of the latest sample in the block.
 *
 *  If you need to read data contiguously, you need to use TimeSampleBufferReader
 *  to read it as it will ensure that the samples are contiguous across
 *  successive read calls.
 *
 */

#ifndef PokemonAutomation_CommonFramework_AudioPipeline_TimeSampleBuffer_H
#define PokemonAutomation_CommonFramework_AudioPipeline_TimeSampleBuffer_H

#include <vector>
#include <map>
#include <string>
#include "Common/Cpp/Time.h"
#include "Common/Cpp/Concurrency/SpinLock.h"

namespace PokemonAutomation{


template <typename Type>
class TimeSampleBufferReader;


template <typename Type>
class TimeSampleBuffer{
    using Duration = std::chrono::system_clock::duration;

public:
    TimeSampleBuffer(
        size_t samples_per_second,
        Duration history,
        Duration gap_threshold = std::chrono::milliseconds(100)
    );

    //  Write "count" samples ending on "timestamp".
    void push_samples(
        const Type* samples, size_t count,
        WallClock timestamp = current_time()
    );

    //  Read "count" samples ending on "timestamp".
    void read_samples(
        Type* samples, size_t count,
        WallClock timestamp = current_time()
    ) const;

    std::string dump() const;

private:
    friend class TimeSampleBufferReader<Type>;
    using MapType = std::map<WallClock, std::vector<Type>>;

    const size_t m_samples_per_second;
    const Duration m_sample_period;     //  Time between adjacent samples.

    //  Minimum # of samples to keep.
    const size_t m_samples_to_buffer;

    //  # of samples in a gap or an overlap before we can no longer stretch.
    const Duration m_duration_gap_threshold;
    const size_t m_sample_gap_threshold;

    mutable SpinLock m_lock;
    MapType m_samples;
    size_t m_samples_stored;
};



}
#endif
