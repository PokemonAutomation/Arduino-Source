/*  Time Sample Buffer Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "TimeSampleWriter.h"
#include "TimeSampleBufferReader.h"

namespace PokemonAutomation{



template <typename Type>
TimeSampleBufferReader<Type>::TimeSampleBufferReader(TimeSampleBuffer<Type>& buffer)
    : m_buffer(buffer)
//    , m_last_timestamp(TimePoint::min())
    , m_current_block(WallClock::min())
    , m_current_index(0)
{}

template <typename Type>
void TimeSampleBufferReader<Type>::set_to_timestamp(WallClock timestamp){
    WriteSpinLock lg(m_buffer.m_lock);
    set_to_timestamp_unprotected(timestamp);
}

template <typename Type>
void TimeSampleBufferReader<Type>::set_to_timestamp_unprotected(WallClock timestamp){
    m_current_block = WallClock::min();
    m_current_index = 0;

    const typename TimeSampleBuffer<Type>::MapType& buffer = m_buffer.m_samples;
    if (buffer.empty()){
        return;
    }

    auto current_block = buffer.upper_bound(timestamp);
    if (current_block == buffer.end()){
//        cout << "front gap" << endl;
        --current_block;
    }

    WallClock end = current_block->first;
    WallClock start = end - current_block->second.size() * m_buffer.m_sample_period;

//    cout << start - REFERENCE << " - " << end - REFERENCE << endl;

    //  Way ahead of the latest sample.
    if (timestamp - end > m_buffer.m_duration_gap_threshold){
//        cout << "way ahead" << endl;
        return;
    }

    //  Way before the earliest sample.
    if (start - timestamp > m_buffer.m_duration_gap_threshold){
//        cout << "way behind" << endl;
        return;
    }

    size_t block_size = current_block->second.size();
    m_current_block = current_block->first;

    //  Slightly ahead of latest sample. Clip to latest.
    if (timestamp >= end){
//        cout << "slightly ahead" << endl;
        m_current_index = block_size;
        return;
    }

    //  Slightly behind oldest sample. Clip to oldest.
    if (timestamp <= start){
//        cout << "slightly behind" << endl;
        m_current_index = 0;
        return;
    }

    //  Somewhere inside the block.
    size_t block = (end - timestamp).count() / m_buffer.m_sample_period.count();
    block = std::min(block, block_size);
    m_current_index = block_size - block;
}

template <typename Type>
void TimeSampleBufferReader<Type>::read_samples(
    Type* samples, size_t count,
    WallClock timestamp
){
    const typename TimeSampleBuffer<Type>::MapType& buffer = m_buffer.m_samples;

    WriteSpinLock lg(m_buffer.m_lock);

    if (buffer.empty()){
        memset(samples, 0, count * sizeof(Type));
        return;
    }

    //  Setup output state.
    WallClock requested_time = timestamp - count * m_buffer.m_sample_period;
    TimeSampleWriterForward output_buffer(samples, count);

    auto current_block = buffer.lower_bound(m_current_block);
    if (current_block == buffer.end()){
//        cout << "front gap" << endl;
        --current_block;
    }

    //  If the block no longer exists, jump to whatever is best block for the requested timestamp.
    if (current_block->first != m_current_block || current_block->second.size() <= m_current_index){
//        cout << "resetting state" << endl;
        current_block = buffer.lower_bound(requested_time);
        if (current_block == buffer.end()){
            --current_block;
        }
        m_current_block = current_block->first;
        m_current_index = 0;
    }

    //  Setup input state.
    WallClock current_time = current_block->first - current_block->second.size() * m_buffer.m_sample_period;

    while (output_buffer.samples_left() > 0){
        //  Current block is empty. Move to next block.
        if (m_current_index >= current_block->second.size()){
            ++current_block;
            if (current_block == buffer.end()){
                output_buffer.fill_rest_with_zeros();
                return;
            }
            m_current_block = current_block->first;
            m_current_index = 0;
            current_time = current_block->first - current_block->second.size() * m_buffer.m_sample_period;
        }

        const std::vector<Type>& data = current_block->second;
        size_t samples_remaining_in_block = data.size() - m_current_index;

        //  Requested is far ahead of what's next. Skip ahead.
        Duration output_ahead = requested_time - current_time;
        if (output_ahead > m_buffer.m_duration_gap_threshold){
//            cout << "Output Ahead" << endl;
            size_t block = output_ahead.count() / m_buffer.m_sample_period.count();
            block = std::min(block, samples_remaining_in_block);
            m_current_index += block;
            current_time += block * m_buffer.m_sample_period;
            continue;
        }

        //  Requested is far behind what's next. Fill the gap with zeros.
        Duration input_ahead = current_time - requested_time;
        if (input_ahead > m_buffer.m_duration_gap_threshold){
//            cout << "Input Ahead" << endl;
            size_t block = input_ahead.count() / m_buffer.m_sample_period.count();
            output_buffer.push_zeros(block);
            requested_time += block * m_buffer.m_sample_period;
            continue;
        }

        size_t block = output_buffer.push_block(data.data() + m_current_index, samples_remaining_in_block);
        Duration block_time = block * m_buffer.m_sample_period;
        m_current_index += block;
        current_time += block_time;
        requested_time += block_time;
    }
}



template class TimeSampleBufferReader<uint8_t>;
template class TimeSampleBufferReader<int8_t>;
template class TimeSampleBufferReader<uint16_t>;
template class TimeSampleBufferReader<int16_t>;
template class TimeSampleBufferReader<uint32_t>;
template class TimeSampleBufferReader<int32_t>;
template class TimeSampleBufferReader<float>;




}
