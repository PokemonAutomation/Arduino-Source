/*  Waterfill Types
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Types_H
#define PokemonAutomation_Kernels_Waterfill_Types_H

#include <stddef.h>
#include <stdint.h>
#include "Common/Cpp/Rectangle.h"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{


class WaterfillIterator;

// An object in the waterfill session.
// Objects are represented as non-zero bits on the size of an image.
class WaterfillObject{
public:
    WaterfillObject(WaterfillObject&& x) = default;
    WaterfillObject& operator=(WaterfillObject&& x) = default;
    WaterfillObject(const WaterfillObject& x){
        *this = x;
    }
    void operator=(const WaterfillObject& x){
        body_x = x.body_x;
        body_y = x.body_y;
        min_x = x.min_x;
        min_y = x.min_y;
        max_x = x.max_x;
        max_y = x.max_y;
        area = x.area;
        sum_x = x.sum_x;
        sum_y = x.sum_y;
        if (x.object){
            object = x.object->clone();
        }
    }

public:
    WaterfillObject() = default;

    size_t width() const{ return max_x - min_x; }
    size_t height() const{ return max_y - min_y; }

    double center_of_gravity_x() const{ return (double)sum_x / area; }
    double center_of_gravity_y() const{ return (double)sum_y / area; }

    double aspect_ratio() const{ return (double)width() / height(); }
    double area_ratio() const{ return (double)area / (width() * height()); }

    Rectangle<size_t> rectangle() const{
        return Rectangle<size_t>(min_x, min_y, max_x, max_y);
    }

    // Note: `object` must be constructed before calling this function.
    std::unique_ptr<PackedBinaryMatrix_IB> packed_matrix() const{
        return object->submatrix(min_x, min_y, max_x - min_x, max_y - min_y);
    }

    void merge_assume_no_overlap(const WaterfillObject& obj){
        if (obj.area == 0){
            return;
        }
        if (area == 0){
            *this = obj;
            return;
        }
        min_x = std::min(min_x, obj.min_x);
        min_y = std::min(min_y, obj.min_y);
        max_x = std::max(max_x, obj.max_x);
        max_y = std::max(max_y, obj.max_y);
        area += obj.area;
        sum_x += obj.sum_x;
        sum_y += obj.sum_y;
        if (object && obj.object){
            *object |= *obj.object;
        }
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

    std::unique_ptr<SparseBinaryMatrix_IB> object;
};




}
}
}
#endif
