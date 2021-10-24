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


template <typename vtype>
class BinaryImageBase{
    static constexpr size_t BITS_PER_WORD = sizeof(vtype) * CHAR_BIT;

public:
    BinaryImageBase(BinaryImageBase&& x);
    void operator=(BinaryImageBase&& x);


public:
    BinaryImageBase(size_t width, size_t height);

    size_t width() const{ return m_width; }
    size_t height() const{ return m_height; }

    bool pixel(size_t x, size_t y) const;
    void set_pixel(size_t x, size_t y, bool set);

    uint64_t word64(size_t x, size_t y) const;
    uint64_t& word64(size_t x, size_t y);

    vtype wordV(size_t x, size_t y) const;
    vtype& wordV(size_t x, size_t y);

    const vtype* row(size_t y) const;
    vtype* row(size_t y);

    std::string dump() const;


protected:
    size_t m_width;
    size_t m_height;
    size_t m_words_per_row;
    size_t m_bytes_per_row;
    AlignedVector<vtype> m_data;
};




template <typename vtype>
BinaryImageBase<vtype>::BinaryImageBase(BinaryImageBase&& x)
    : m_width(x.m_width)
    , m_height(x.m_height)
    , m_words_per_row(0)
    , m_bytes_per_row(x.m_bytes_per_row)
    , m_data(std::move(x.m_data))
{
    x.m_width = 0;
    x.m_height = 0;
    x.m_bytes_per_row = 0;
}
template <typename vtype>
void BinaryImageBase<vtype>::operator=(BinaryImageBase&& x){
    m_width = x.m_width;
    m_height = x.m_height;
    m_words_per_row = x.m_words_per_row;
    m_bytes_per_row = x.m_bytes_per_row;
    m_data = std::move(x.m_data);
    x.m_width = 0;
    x.m_height = 0;
    x.m_bytes_per_row = 0;
}

template <typename vtype>
BinaryImageBase<vtype>::BinaryImageBase(size_t width, size_t height)
    : m_width(width)
    , m_height(height)
    , m_words_per_row(Kernels::align_int_up<BITS_PER_WORD>(width) / BITS_PER_WORD)
    , m_bytes_per_row(m_words_per_row * sizeof(vtype))
    , m_data(m_words_per_row * height)
{}


template <typename vtype> PA_FORCE_INLINE
bool BinaryImageBase<vtype>::pixel(size_t x, size_t y) const{
    uint64_t word = *((const uint64_t*)row(y) + x / 64);
    return (word >> (x % 64)) & 1;
}
template <typename vtype> PA_FORCE_INLINE
void BinaryImageBase<vtype>::set_pixel(size_t x, size_t y, bool set){
    uint64_t& word = *((uint64_t*)row(y) + x / 64);
    uint64_t shift = x % 64;
    uint64_t bit = set ? 1 : 0;
    bit <<= shift;
    uint64_t mask = ~(1 << shift);
    word = (word & mask) | bit;
}



template <typename vtype> PA_FORCE_INLINE
uint64_t BinaryImageBase<vtype>::word64(size_t x, size_t y) const{
    return ((const uint64_t*)row(y))[x];

}
template <typename vtype> PA_FORCE_INLINE
uint64_t& BinaryImageBase<vtype>::word64(size_t x, size_t y){
    return ((uint64_t*)row(y))[x];
}
template <typename vtype> PA_FORCE_INLINE
vtype BinaryImageBase<vtype>::wordV(size_t x, size_t y) const{
    return row(y)[x];
}
template <typename vtype> PA_FORCE_INLINE
vtype& BinaryImageBase<vtype>::wordV(size_t x, size_t y){
    return row(y)[x];
}

template <typename vtype> PA_FORCE_INLINE
const vtype* BinaryImageBase<vtype>::row(size_t y) const{
    return (const vtype*)((const char*)m_data.data() + m_bytes_per_row * y);
}
template <typename vtype> PA_FORCE_INLINE
vtype* BinaryImageBase<vtype>::row(size_t y){
    return (vtype*)((char*)m_data.data() + m_bytes_per_row * y);
}

template <typename vtype>
std::string BinaryImageBase<vtype>::dump() const{
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
