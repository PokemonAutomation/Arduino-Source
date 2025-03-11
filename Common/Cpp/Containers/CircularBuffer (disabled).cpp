/*  Circular Buffer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string.h>
#include <algorithm>
#include "Exceptions.h"
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
#ifdef PA_DEBUG_CircularBuffer
std::string CircularBuffer::dump() const{
    std::string ret;
    ret += "size   = " + std::to_string(m_size) + "\n";
    ret += "start  = " + std::to_string(m_start) + "\n";
    ret += "end    = " + std::to_string(m_end) + "\n";
    ret += "buffer = " + std::string(m_buffer.data(), m_buffer.size());
    return ret;
}
#endif





StreamReader::StreamReader(size_t initial_size){
    size_t buffer_size = 1;
    while (buffer_size < initial_size){
        buffer_size *= 2;
    }
    m_buffer = AlignedVector<char>(buffer_size);
#ifdef PA_DEBUG_CircularBuffer
    memset(m_buffer.data(), '-', m_buffer.size());
#endif
}
StreamReader::~StreamReader(){}
void StreamReader::push_back(const void* data, size_t bytes){
    size_t size = m_end - m_start;
    size_t new_size = size + bytes;
    if (new_size > m_buffer.size()){
        expand(new_size);
    }

    size_t mask = m_buffer.size() - 1;
//    size_t idx_s = (size_t)m_start & mask;
    size_t idx_e = (size_t)m_end & mask;

    //  Copy all the way to the end.
    size_t block = m_buffer.size() - std::max(size, idx_e);
    block = std::min(block, bytes);
    memcpy(m_buffer.data() + idx_e, data, block);
    m_end += block;
    idx_e = (size_t)m_end & mask;
    data = (char*)data + block;
    bytes -= block;


    //  Now copy the bottom.
    if (bytes > 0){
        memcpy(m_buffer.data() + idx_e, data, bytes);
        m_end += bytes;
    }
}
void StreamReader::read(void* data, uint64_t offset, size_t bytes) noexcept{
#ifdef PA_DEBUG_CircularBuffer
    constexpr char PADDING = '*';
#else
    constexpr char PADDING = 0;
#endif
    uint64_t read_end = offset + bytes;

    //  Completely before.
    if (read_end <= m_start){
        memset(data, PADDING, bytes);
        return;
    }

    //  Completely after.
    if (offset >= m_end){
        memset(data, PADDING, bytes);
        return;
    }

    //  Starts before.
    if (offset < m_start){
        size_t block = (size_t)(m_start - offset);
        memset(data, PADDING, block);
        data = (char*)data + block;
        offset += block;
        bytes -= block;
    }

    //  Ends after.
    if (read_end > m_end){
        size_t block = (size_t)(read_end - m_end);
        memset((char*)data + bytes - block, PADDING, block);
        bytes -= block;
    }

    size_t available = m_end - offset;
    size_t mask = m_buffer.size() - 1;
    size_t idx_s = (size_t)offset & mask;
//    size_t idx_e = (size_t)m_end & mask;

    //  Find out how many bytes we can read contiguously.
    size_t block = std::min(m_buffer.size() - idx_s, available);

    //  Don't read more bytes than is requested.
    block = std::min(block, bytes);

    //  Perform the read.
    memcpy(data, m_buffer.data() + idx_s, block);
    data = (char*)data + block;
    offset += block;
    bytes -= block;
    idx_s = (size_t)offset & mask;

    //  Wrap around and read the rest.
    if (bytes > 0){
        memcpy(data, m_buffer.data() + idx_s, bytes);
    }
}
void StreamReader::pop_to(uint64_t offset) noexcept{
    m_start = std::max(m_start, offset);
}
void StreamReader::expand(size_t min_size){
    size_t buffer_size = m_buffer.size();
    while (buffer_size < min_size){
        buffer_size *= 2;
    }

    //  TODO: Optimize out this allocation + copy.
    size_t size = (size_t)(m_end - m_start);
    AlignedVector<char> tmp(size);
    read(tmp.data(), m_start, size);

    m_buffer = AlignedVector<char>(buffer_size);
#ifdef PA_DEBUG_CircularBuffer
    memset(m_buffer.data(), '-', m_buffer.size());
#endif
    m_end = m_start;
    push_back(tmp.data(), size);
}
#ifdef PA_DEBUG_CircularBuffer
std::string StreamReader::dump() const{
    std::string ret;
    ret += "start  = " + std::to_string(m_start) + "\n";
    ret += "end    = " + std::to_string(m_end) + "\n";
    ret += "buffer = " + std::string(m_buffer.data(), m_buffer.size());
    return ret;
}
#endif




#if 0
ConvertedStreamReader::ConvertedStreamReader(
    size_t raw_object_size,
    size_t converted_object_size,
    size_t initial_objects  //  Will be rounded up to a power-of-two.
)
    : m_raw_object_size(raw_object_size)
    , m_converted_object_size(converted_object_size)
    , m_edge(raw_object_size)
{
    m_buffer_capacity = 1;
    while (m_buffer_capacity < initial_objects){
        m_buffer_capacity *= 2;
    }
    m_buffer = AlignedVector<char>(m_buffer_capacity * converted_object_size);
#ifdef PA_DEBUG_CircularBuffer
    memset(m_buffer.data(), '-', m_buffer.size());
#endif
}
ConvertedStreamReader::~ConvertedStreamReader(){}
void ConvertedStreamReader::push_back(const void* data, size_t bytes){
    size_t stored = m_end - m_start;
    size_t objects = (bytes + m_edge_size) / m_raw_object_size;

    //  Make sure there is enough space in the buffer.
    if (stored + objects > m_buffer_capacity){

    }

    //  Fill the edge block.
    if (m_edge_size > 0){
        size_t block = std::min(m_raw_object_size - m_edge_size, bytes);
        memcpy(m_edge.data() + m_edge_size, data, block);
        data = (char*)data + block;
        bytes -= block;
    }

    size_t mask = m_buffer_capacity - 1;
//    size_t index_s = (size_t)m_start & mask;
    size_t index_e = (size_t)m_end & mask;

    //  Process completed edge block.
    if (m_edge_size >= m_raw_object_size){
        convert(m_buffer.data() + index_e * m_converted_object_size, m_edge.data(), 1);
        m_edge_size = 0;
        m_end++;
        index_e = (size_t)m_end & mask;
        stored++;
        objects--;
    }

    {
        //  Write as far as we can.
        size_t block = m_buffer_capacity - std::max(stored, index_e);

        //  But don't write more than we have right now.
        block = std::min(block, objects);

        convert(m_buffer.data() + index_e * m_converted_object_size, data, block);
        size_t raw_bytes = block * m_raw_object_size;
        data = (char*)data + raw_bytes;
        bytes -= raw_bytes;
        objects -= block;
        m_end += block;
//        stored += block;
        index_e = (size_t)m_end & mask;
    }

    //  Now handle the wrap-around.
    if (objects > 0){
        size_t block = objects;
        convert(m_buffer.data() + index_e * m_converted_object_size, data, block);
        size_t raw_bytes = block * m_raw_object_size;
        data = (char*)data + raw_bytes;
        bytes -= raw_bytes;
//        objects -= block;
        m_end += block;
//        stored += block;
//        index_e = (size_t)m_end & mask;
    }

    //  Copy remaining bytes into edge buffer.
    memcpy(m_edge.data(), data, bytes);
    m_edge_size = bytes;
}




#ifdef PA_DEBUG_CircularBuffer
std::string ConvertedStreamReader::dump() const{
    std::string ret;
    ret += "start    = " + std::to_string(m_start) + "\n";
    ret += "end      = " + std::to_string(m_end) + "\n";
    ret += "capacity = " + std::to_string(m_buffer_capacity) + "\n";
    ret += "buffer   = " + std::string(m_buffer.data(), m_buffer.size());
    ret += "edge     = " + std::string(m_edge.data(), m_edge_size);
    return ret;
}
#endif

#endif











}
