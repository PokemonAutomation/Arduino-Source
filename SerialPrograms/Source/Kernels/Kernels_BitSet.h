/*  Bit Scanning
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_BitSet_H
#define PokemonAutomation_Kernels_BitSet_H

#include <stdint.h>
#include <string.h>
#include <vector>
#include "Common/Compiler.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Kernels{


class BitSet{
    static constexpr size_t WORD_BITS = 32;
    using vtype = uint32_t;

public:
    BitSet() = default;
    BitSet(size_t bits)
        : m_dense((bits + WORD_BITS - 1) / WORD_BITS)
    {
        memset(m_dense.data(), 0, m_dense.size() * sizeof(uint32_t));
    }

    PA_FORCE_INLINE void clear(){
        for (uint64_t index : m_sparse){
            m_dense[index / WORD_BITS] = 0;
        }
        m_sparse.clear();
    }

    PA_FORCE_INLINE bool get(size_t index) const{
        size_t word = index / WORD_BITS;
        size_t shift = index % WORD_BITS;
        uint32_t mask = (uint32_t)1 << shift;
        return m_dense[word] & mask;
    }
    PA_FORCE_INLINE bool set(size_t index){
        size_t word = index / WORD_BITS;
        size_t shift = index % WORD_BITS;
        uint32_t& ref = m_dense[word];
        uint32_t value = ref;
        uint32_t mask = (uint32_t)1 << shift;
        if (value & mask){
            return true;
        }else{
            ref = value | mask;
            m_sparse.emplace_back(index);
            return false;
        }
    }
    PA_FORCE_INLINE bool pop(size_t& index){
        while (!m_sparse.empty()){
            size_t current = m_sparse.back();
            m_sparse.pop_back();
            size_t word = current / WORD_BITS;
            size_t shift = current % WORD_BITS;
            uint32_t& ref = m_dense[word];
            uint32_t value = ref;
            uint32_t mask = (uint32_t)1 << shift;
            if (value & mask){
                ref = value & ~mask;
                index = current;
                return true;
            }
        }
        return false;
    }

private:
    std::vector<vtype> m_dense;
    std::vector<size_t> m_sparse;
};




// Used in Waterfill/Kernels_Waterfill_Session.tpp to record the tile traversal during waterfill
class BitSet2D{
    static constexpr size_t WORD_BITS = 32;
    using vtype = uint32_t;

public:
    BitSet2D() = default;
    BitSet2D(size_t width, size_t height)
        : m_width(width)
        , m_dense((width*height + WORD_BITS - 1) / WORD_BITS)
    {
//        cout << width << " x " << height << endl;
        memset(m_dense.data(), 0, m_dense.size() * sizeof(uint32_t));
    }

    PA_FORCE_INLINE void clear(){
        for (uint64_t i : m_sparse){
            size_t current_x = (uint32_t)i;
            size_t current_y = (size_t)(i >> 32);
            size_t index = current_x + current_y * m_width;
            m_dense[index / WORD_BITS] = 0;
        }
        m_sparse.clear();
    }

    PA_FORCE_INLINE bool get(size_t x, size_t y) const{
        size_t index = x + y * m_width;
        size_t word = index / WORD_BITS;
        size_t shift = index % WORD_BITS;
        uint32_t mask = (uint32_t)1 << shift;
        return m_dense[word] & mask;
    }
    PA_FORCE_INLINE bool set(size_t x, size_t y){
        size_t index = x + y * m_width;
        size_t word = index / WORD_BITS;
        size_t shift = index % WORD_BITS;
        uint32_t& ref = m_dense[word];
        uint32_t value = ref;
        uint32_t mask = (uint32_t)1 << shift;
        if (value & mask){
            return true;
        }else{
            ref = value | mask;
            m_sparse.emplace_back(x | ((uint64_t)y << 32));
            return false;
        }
    }
    PA_FORCE_INLINE bool pop(size_t& x, size_t& y){
        while (!m_sparse.empty()){
            size_t current = m_sparse.back();
            m_sparse.pop_back();
            size_t current_x = (uint32_t)current;
            size_t current_y = (size_t)(current >> 32);
            size_t index = current_x + current_y * m_width;
            size_t word = index / WORD_BITS;
            size_t shift = index % WORD_BITS;
            uint32_t& ref = m_dense[word];
            uint32_t value = ref;
            uint32_t mask = (uint32_t)1 << shift;
            if (value & mask){
                ref = value & ~mask;
                x = current_x;
                y = current_y;
                return true;
            }
        }
        return false;
    }


private:
    size_t m_width;
    std::vector<vtype> m_dense;
    std::vector<uint64_t> m_sparse;
};






}
}
#endif
