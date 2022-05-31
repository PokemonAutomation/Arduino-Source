/*  Circular Buffer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CircularBuffer_H
#define PokemonAutomation_CircularBuffer_H

#include "AlignedVector.h"

//#define PA_DEBUG_CircularBuffer

#ifdef PA_DEBUG_CircularBuffer
#include <string>
#endif


namespace PokemonAutomation{


class CircularBuffer{
public:
    CircularBuffer(size_t initial_size);
    ~CircularBuffer();

    size_t size() const{ return m_size; }

    void push_back(const void* data, size_t bytes);
    size_t pop_front(void* data, size_t bytes) noexcept;

#ifdef PA_DEBUG_CircularBuffer
    std::string dump() const{
        std::string ret;
        ret += "size   = " + std::to_string(m_size) + "\n";
        ret += "start  = " + std::to_string(m_start) + "\n";
        ret += "end    = " + std::to_string(m_end) + "\n";
        ret += "buffer = " + std::string(m_buffer.data(), m_buffer.size());
        return ret;
    }
#endif

private:
    void expand(size_t min_size);

private:
    AlignedVector<char> m_buffer;
    size_t m_size = 0;
    size_t m_start = 0;
    size_t m_end = 0;
};



}
#endif
