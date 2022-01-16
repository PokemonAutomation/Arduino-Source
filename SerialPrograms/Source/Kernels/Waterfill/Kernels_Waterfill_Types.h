/*  Waterfill Types
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Types_H
#define PokemonAutomation_Kernels_Waterfill_Types_H

#include <stddef.h>
#include <stdint.h>
#include <map>
#include <iostream>
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{



class WaterFillObject{
public:
    size_t width() const{ return max_x - min_x; }
    size_t height() const{ return max_y - min_y; }

    size_t center_x() const{ return sum_x / area; }
    size_t center_y() const{ return sum_y / area; }

    double aspect_ratio() const{ return (double)width() / height(); }
    double area_ratio() const{ return (double)area / (width() * height()); }

    PackedBinaryMatrix packed_matrix() const{
        return object.submatrix(min_x, min_y, max_x - min_x, max_y - min_y);
    }

    void merge_assume_no_overlap(const WaterFillObject& obj){
        if (obj.area == 0){
            return;
        }
        if (area == 0){
            *this = obj;
        }
        min_x = std::min(min_x, obj.min_x);
        min_y = std::min(min_y, obj.min_y);
        max_x = std::max(max_x, obj.max_x);
        max_y = std::max(max_y, obj.max_y);
        area += obj.area;
        sum_x += obj.sum_x;
        sum_y += obj.sum_y;
    }


public:
    void accumulate_body(
        size_t offset_x, size_t offset_y,
        size_t popcount,
        uint64_t tile_sum_x, uint64_t tile_sum_y
    ){
        area += popcount;
        sum_x += (uint64_t)offset_x * popcount + tile_sum_x;
        sum_y += (uint64_t)offset_y * popcount + tile_sum_y;
    }
    void accumulate_boundary(
        size_t offset_x, size_t offset_y,
        size_t tile_min_x, size_t tile_max_x,
        size_t tile_min_y, size_t tile_max_y
    ){
        min_x = std::min(min_x, offset_x + tile_min_x);
        min_y = std::min(min_y, offset_y + tile_min_y);
        max_x = std::max(max_x, offset_x + tile_max_x);
        max_y = std::max(max_y, offset_y + tile_max_y);
    }


public:
    //  The coordinates of any part of the body of this object.
    size_t body_x = 0;
    size_t body_y = 0;

    //  Enclosing rectangle. (max is one past the end)
    size_t min_x = (size_t)0 - 1;
    size_t min_y = (size_t)0 - 1;
    size_t max_x = 0;
    size_t max_y = 0;

    //  Number of bits in this object. (the popcount)
    size_t area = 0;

    //  These divided by "m_area" is the center of gravity.
    uint64_t sum_x = 0;
    uint64_t sum_y = 0;

    //  The object itself in the original image.
    SparseBinaryMatrix object;
};




}
}
}
#endif
