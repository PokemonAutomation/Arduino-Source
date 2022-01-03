/*  Binary Image Base
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryImageBase_H
#define PokemonAutomation_Kernels_BinaryImageBase_H

#include <limits.h>
#include <string>
#include "Kernels/Kernels_Alignment.h"
#include "Common/Cpp/AlignedVector.h"

namespace PokemonAutomation{
namespace Kernels{


template <typename Context>
class BinaryImageBase{
    using vtype = typename Context::vtype;
    static constexpr size_t BITS_PER_WORD = sizeof(vtype) * CHAR_BIT;

public:
    BinaryImageBase(BinaryImageBase&& x);
    void operator=(BinaryImageBase&& x);

public:
    BinaryImageBase(size_t width, size_t height);

    size_t width() const{ return m_width; }
    size_t height() const{ return m_height; }

    //  These are slow.
    bool pixel(size_t x, size_t y) const;
    void set_pixel(size_t x, size_t y, bool set);

public:
    //  These are fast.
    uint64_t word64(size_t x, size_t y) const;
    uint64_t& word64(size_t x, size_t y);

    vtype wordV(size_t x, size_t y) const;
    vtype& wordV(size_t x, size_t y);

    const vtype* row(size_t y) const;
    vtype* row(size_t y);

public:
    //  Debugging
    std::string dump() const;


protected:
    size_t m_width;
    size_t m_height;
    size_t m_words_per_row;
    size_t m_bytes_per_row;
    AlignedVector<vtype> m_data;
};




template <typename Context>
BinaryImageBase<Context>::BinaryImageBase(BinaryImageBase&& x)
    : m_width(x.m_width)
    , m_height(x.m_height)
    , m_words_per_row(x.m_words_per_row)
    , m_bytes_per_row(x.m_bytes_per_row)
    , m_data(std::move(x.m_data))
{
    x.m_width = 0;
    x.m_height = 0;
    x.m_bytes_per_row = 0;
}
template <typename Context>
void BinaryImageBase<Context>::operator=(BinaryImageBase&& x){
    m_width = x.m_width;
    m_height = x.m_height;
    m_words_per_row = x.m_words_per_row;
    m_bytes_per_row = x.m_bytes_per_row;
    m_data = std::move(x.m_data);
    x.m_width = 0;
    x.m_height = 0;
    x.m_bytes_per_row = 0;
}

template <typename Context>
BinaryImageBase<Context>::BinaryImageBase(size_t width, size_t height)
    : m_width(width)
    , m_height(height)
    , m_words_per_row(Kernels::align_int_up<BITS_PER_WORD>(width) / BITS_PER_WORD)
    , m_bytes_per_row(m_words_per_row * sizeof(vtype))
    , m_data(m_words_per_row * height)
{}


template <typename Context> PA_FORCE_INLINE
bool BinaryImageBase<Context>::pixel(size_t x, size_t y) const{
    uint64_t word = *((const uint64_t*)row(y) + x / 64);
    return (word >> (x % 64)) & 1;
}
template <typename Context> PA_FORCE_INLINE
void BinaryImageBase<Context>::set_pixel(size_t x, size_t y, bool set){
    uint64_t& word = *((uint64_t*)row(y) + x / 64);
    uint64_t shift = x % 64;
    uint64_t bit = set ? 1 : 0;
    bit <<= shift;
    uint64_t mask = ~(1 << shift);
    word = (word & mask) | bit;
}



template <typename Context> PA_FORCE_INLINE
uint64_t BinaryImageBase<Context>::word64(size_t x, size_t y) const{
    return ((const uint64_t*)row(y))[x];

}
template <typename Context> PA_FORCE_INLINE
uint64_t& BinaryImageBase<Context>::word64(size_t x, size_t y){
    return ((uint64_t*)row(y))[x];
}
template <typename Context> PA_FORCE_INLINE
typename Context::vtype BinaryImageBase<Context>::wordV(size_t x, size_t y) const{
    return row(y)[x];
}
template <typename Context> PA_FORCE_INLINE
typename Context::vtype& BinaryImageBase<Context>::wordV(size_t x, size_t y){
    return row(y)[x];
}

template <typename Context> PA_FORCE_INLINE
const typename Context::vtype* BinaryImageBase<Context>::row(size_t y) const{
    return (const vtype*)((const char*)m_data.data() + m_bytes_per_row * y);
}
template <typename Context> PA_FORCE_INLINE
typename Context::vtype* BinaryImageBase<Context>::row(size_t y){
    return (vtype*)((char*)m_data.data() + m_bytes_per_row * y);
}

template <typename Context>
std::string BinaryImageBase<Context>::dump() const{
    std::string str;
    for (size_t r = 0; r < m_height; r++){
        for (size_t c = 0; c < m_width; c++){
            str += pixel(c, r) ? "1" : "0";
        }
        str += "\n";
    }
    return str;
}



}
}
#endif
