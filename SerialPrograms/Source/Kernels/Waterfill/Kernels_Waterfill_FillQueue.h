/*  Fill Queue
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_WaterFill_FillQueue_H
#define PokemonAutomation_Kernels_WaterFill_FillQueue_H

#include <utility>
#include <vector>
#include "Common/Cpp/AlignedVector.h"

namespace PokemonAutomation{
namespace Kernels{
namespace WaterFill{


template <typename Intrinsics>
class FillQueue{
//    using Intrinsics = Intrinsics_u64;
    using Word = typename Intrinsics::Word;

public:
    struct Point{
        size_t x;
        size_t y;
        Word mask;
    };

public:
    FillQueue(size_t width, size_t height);

    void push_point(size_t x, size_t y, Word mask);
    void push_neighbors(size_t x, size_t y, Word mask);

    bool pop(Point& point);

private:
    size_t m_width;
    size_t m_height;
    std::vector<std::pair<size_t, size_t>> m_points;
    AlignedVector<Word> m_masks;
};




template <typename Intrinsics>
FillQueue<Intrinsics>::FillQueue(size_t width, size_t height)
    : m_width(width)
    , m_height(height)
{}
template <typename Intrinsics>
void FillQueue<Intrinsics>::push_point(size_t x, size_t y, Word mask){
    m_points.emplace_back(x, y);
    m_masks.emplace_back(mask);
}
template <typename Intrinsics>
void FillQueue<Intrinsics>::push_neighbors(size_t x, size_t y, Word mask){
    if (y > 0){
        push_point(x, y - 1, mask);
    }
    if (y + 1 < m_height){
        push_point(x, y + 1, mask);
    }
    if (x > 0 && Intrinsics::low_bit(mask)){
        push_point(x - 1, y, Intrinsics::HIGH_BIT());
    }
    if (x + 1 < m_width && Intrinsics::high_bit(mask)){
        push_point(x + 1, y, Intrinsics::LOW_BIT());
    }
}
template <typename Intrinsics>
bool FillQueue<Intrinsics>::pop(Point& point){
    if (m_points.empty()){
        return false;
    }
    std::pair<size_t, size_t> coord = m_points.back();
    point.x = coord.first;
    point.y = coord.second;
    m_points.pop_back();
    point.mask = m_masks.back();
    m_masks.pop_back();
    return true;
}




}
}
}
#endif
