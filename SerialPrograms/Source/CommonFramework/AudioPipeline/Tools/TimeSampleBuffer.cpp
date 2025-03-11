/*  Time Sample Buffer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonFramework_AudioPipeline_TimeSampleBuffer_TPP
#define PokemonAutomation_CommonFramework_AudioPipeline_TimeSampleBuffer_TPP

#include "Common/Cpp/Exceptions.h"
#include "TimeSampleWriter.h"
#include "TimeSampleBuffer.h"

namespace PokemonAutomation{


template <typename Type>
TimeSampleBuffer<Type>::TimeSampleBuffer(
    size_t samples_per_second,
    Duration history,
    Duration gap_threshold
)
    : m_samples_per_second(samples_per_second)
    , m_sample_period(Duration(std::chrono::seconds(1)) / samples_per_second)
    , m_samples_to_buffer(samples_per_second * std::chrono::duration_cast<std::chrono::milliseconds>(history).count() / 1000)
    , m_duration_gap_threshold(gap_threshold)
    , m_sample_gap_threshold(samples_per_second * std::chrono::duration_cast<std::chrono::milliseconds>(gap_threshold).count() / 1000)
    , m_samples_stored(0)
{
    if (gap_threshold < m_sample_period){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Gap threshold cannot be smaller than sample period.");
    }
}

template <typename Type>
void TimeSampleBuffer<Type>::push_samples(
    const Type* samples, size_t count,
    WallClock timestamp
){
    std::vector<Type> block(count);
    memcpy(block.data(), samples, count * sizeof(Type));

    WriteSpinLock lg(m_lock);

#if 0
    auto iter = m_samples.find(timestamp);
    if (iter != m_samples.end()){
        timestamp++;
    }
#endif

    m_samples[timestamp] = std::move(block);
    m_samples_stored += count;

    //  Drop samples that are too old.
    while (!m_samples.empty()){
        auto iter = m_samples.begin();
        size_t samples_to_drop = iter->second.size();
        if (m_samples_stored < m_samples_to_buffer + samples_to_drop){
            break;
        }
        m_samples.erase(iter);
        m_samples_stored -= samples_to_drop;
    }
}

template <typename Type>
std::string TimeSampleBuffer<Type>::dump() const{
    ReadSpinLock lg(m_lock);

    std::string str;
    if (m_samples.empty()){
        str += "(buffer is empty)";
        return str;
    }
    auto iter = m_samples.rbegin();
    WallClock latest = iter->first;
    for (; iter != m_samples.rend(); ++iter){
        Duration last = iter->first - latest;
        Duration first = last - m_sample_period * iter->second.size();
        str += std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(last).count() / 1000.);
        str += " - ";
        str += std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(first).count() / 1000.);
        str += " : ";
        str += std::to_string(iter->second.size());
        str += "\n";
    }
    return str;
}


template <typename Type>
void TimeSampleBuffer<Type>::read_samples(
    Type* samples, size_t count,
    WallClock timestamp
) const{
    ReadSpinLock lg(m_lock);

    if (m_samples.empty()){
        memset(samples, 0, count * sizeof(Type));
        return;
    }

    //  Setup output state.
    WallClock requested_time = timestamp;
    TimeSampleWriterReverse output_buffer(samples, count);

    //  Jump to the latest block that's relevant to this request.
//    cout << requested - reference << endl;
//    cout << timestamp - reference << endl;
    auto current_block = m_samples.lower_bound(requested_time);
    if (current_block == m_samples.end()){
//        cout << "front gap" << endl;
        --current_block;
    }

    //  Setup input state.
    WallClock current_time = current_block->first;
    size_t current_index = current_block->second.size();

    //  State machine loop. Look at the current input and output states to
    //  decide on the next action. Stop when output is filled or we run out of
    //  blocks.
    while (output_buffer.samples_left() > 0){
//        cout << "Requested: " << requested_time - reference << endl;
//        cout << "Buffer   : " << current_time - reference << endl;

        //  Current block is empty. Move to previous block.
        if (current_index == 0){
            if (current_block == m_samples.begin()){
                output_buffer.fill_rest_with_zeros();
                return;
            }
            --current_block;
            current_time = current_block->first;
            current_index = current_block->second.size();
        }

        Duration output_ahead = requested_time - current_time;

        //  Requested is far ahead of what's next. Fill the gap with zeros.
        if (output_ahead > m_duration_gap_threshold){
//            cout << "Output Ahead" << endl;
            size_t block = output_ahead.count() / m_sample_period.count();
            output_buffer.push_zeros(block);
            requested_time -= block * m_sample_period;
            continue;
        }

        Duration input_ahead = current_time - requested_time;

        //  Requested is far behind what's next. Skip ahead.
        if (input_ahead > m_duration_gap_threshold){
//            cout << "Input Ahead" << endl;
            size_t block = input_ahead.count() / m_sample_period.count();
            block = std::min(block, current_index);
            current_index -= block;
            current_time -= block * m_sample_period;
            continue;
        }

        size_t block = output_buffer.push_block(current_block->second.data(), current_index);
//        cout << "Push block: " << block << endl;
        Duration block_time = block * m_sample_period;
        current_index -= block;
        current_time -= block_time;
        requested_time -= block_time;
    }
}



template class TimeSampleBuffer<uint8_t>;
template class TimeSampleBuffer<int8_t>;
template class TimeSampleBuffer<uint16_t>;
template class TimeSampleBuffer<int16_t>;
template class TimeSampleBuffer<uint32_t>;
template class TimeSampleBuffer<int32_t>;
template class TimeSampleBuffer<float>;



}
#endif
