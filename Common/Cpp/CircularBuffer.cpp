/*  Circular Buffer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <string.h>
#include <algorithm>
#include "AlignedVector.tpp"
#include "CircularBuffer.h"

namespace PokemonAutomation{



CircularBuffer::CircularBuffer(size_t initial_size)
    : m_buffer(initial_size)
{
#ifdef PA_DEBUG_CircularBuffer
    memset(m_buffer.data(), '-', m_buffer.size());
#endif
}
CircularBuffer::~CircularBuffer(){}

void CircularBuffer::push_back(const void* data, size_t bytes){
    size_t new_size = m_size + bytes;
    if (new_size > m_buffer.size()){
        expand(new_size);
    }

    if (m_size == 0){
        m_start = 0;
        m_end = 0;
    }

    //  Copy all the way to the end.
    size_t block = m_buffer.size() - std::max(m_size, m_end);
    block = std::min(block, bytes);
    memcpy(m_buffer.data() + m_end, data, block);
    m_size += block;
    m_end += block;
    data = (char*)data + block;
    bytes -= block;

    //  Rewind the pointer if necessary.
    if (m_end == m_buffer.size()){
        m_end = 0;
    }

    //  Now copy the bottom.
    if (bytes > 0){
        memcpy(m_buffer.data() + m_end, data, bytes);
        m_size += bytes;
        m_end += bytes;
    }
}

size_t CircularBuffer::pop_front(void* data, size_t bytes) noexcept{
    bytes = std::min(bytes, m_size);

    size_t remaining = bytes;

    //  Copy all the way to the end.
    size_t block = std::min(remaining, m_buffer.size() - m_start);
    memcpy(data, m_buffer.data() + m_start, block);
    m_size -= block;
    m_start += block;
    data = (char*)data + block;
    remaining -= block;

    //  Rewind the pointer if necessary.
    if (m_start == m_buffer.size()){
        m_start = 0;
    }

    //  Now copy the bottom.
    if (remaining > 0){
        memcpy(data, m_buffer.data() + m_start, remaining);
        m_size -= remaining;
        m_start += remaining;
    }

    return bytes;
}


void CircularBuffer::expand(size_t min_size){
    size_t buffer_size = m_buffer.size();
    while (buffer_size < min_size){
        buffer_size *= 2;
    }
    AlignedVector<char> new_buffer(buffer_size);
#ifdef PA_DEBUG_CircularBuffer
    memset(new_buffer.data(), '-', new_buffer.size());
#endif
    size_t size = m_size;
    pop_front(new_buffer.data(), size);
    m_buffer = std::move(new_buffer);
    m_size = size;
    m_start = 0;
    m_end = size;
}



}
