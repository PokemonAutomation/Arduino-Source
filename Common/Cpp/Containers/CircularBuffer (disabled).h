/*  Circular Buffer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CircularBuffer_H
#define PokemonAutomation_CircularBuffer_H

#include <stdint.h>
#include "AlignedVector.h"

#define PA_DEBUG_CircularBuffer

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
    std::string dump() const;
#endif

private:
    void expand(size_t min_size);

private:
    AlignedVector<char> m_buffer;
    size_t m_size = 0;
    size_t m_start = 0;
    size_t m_end = 0;
};



class StreamReader{
public:
    StreamReader(size_t initial_size);  //  Will be rounded up to a power-of-two.
    ~StreamReader();

    //  Return the offset of the oldest byte that is still stored.
    size_t oldest() const{ return m_start; }

    //  Return the 1 past the newest byte that is still stored.
    size_t end() const{ return m_end; }

    //  Return the # of bytes stored.
    size_t bytes_stored() const{ return m_end - m_start; }

    //  Push the specified buffer into the end of this stream.
    void push_back(const void* data, size_t bytes);

    //  Read "bytes" bytes starting from the specified offset.
    //  Bytes that are not stored are set to zero.
    void read(void* data, uint64_t offset, size_t bytes) noexcept;

    //  Evict all bytes before the specified offset from this stream.
    void pop_to(uint64_t offset) noexcept;

#ifdef PA_DEBUG_CircularBuffer
    std::string dump() const;
#endif

private:
    void expand(size_t min_size);

private:
    AlignedVector<char> m_buffer;
    uint64_t m_start = 0;
    uint64_t m_end = 0;
};



#if 0
class ConvertedStreamReader{
public:
    ConvertedStreamReader(
        size_t raw_object_size,         //  Size of each object in the raw input stream.
        size_t converted_object_size,   //  Size of each object after conversion to preferred format.
        size_t initial_objects          //  Will be rounded up to a power-of-two.
    );
    ~ConvertedStreamReader();

    //  Return the offset of the oldest byte that is still stored.
    size_t oldest() const{ return m_start; }

    //  Return the 1 past the newest byte that is still stored.
    size_t end() const{ return m_end; }

    //  Return the # of bytes stored.
    size_t objects_stored() const{ return m_end - m_start; }

    //  Push the specified buffer into the end of this stream.
    void push_back(const void* data, size_t bytes);

    //  Evict all objects before the specified offset from this stream.
    void pop_to(uint64_t offset) noexcept;

#ifdef PA_DEBUG_CircularBuffer
    std::string dump() const;
#endif


protected:
    virtual void convert(void* object, const void* raw, size_t count) = 0;


private:
    size_t m_raw_object_size;
    size_t m_converted_object_size;

    AlignedVector<char> m_edge;
    size_t m_edge_size = 0;

    AlignedVector<char> m_buffer;
    size_t m_buffer_capacity = 0;
    uint64_t m_start = 0;
    uint64_t m_end = 0;
};
#endif










}
#endif
