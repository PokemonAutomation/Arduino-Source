/*  Waterfill Process
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include "Common/Cpp/Color.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "WaterfillUtilities.h"

namespace PokemonAutomation{


std::pair<PackedBinaryMatrix2, size_t> remove_center_pixels(
    const Kernels::Waterfill::WaterfillObject& object,
    size_t num_pixels_to_remove
){
    PackedBinaryMatrix2 matrix = object.packed_matrix();
    size_t width = matrix.width();
    size_t height = matrix.height();
//    cout << matrix.dump() << endl;

    //  Sort all pixels by distance from center.
    size_t center_x = object.center_x() - object.min_x;
    size_t center_y = object.center_y() - object.min_y;
    std::map<uint64_t, size_t> distances;
    for (size_t r = 0; r < height; r++){
        for (size_t c = 0; c < width; c++){
            if (matrix.get(c, r)){
                size_t dist_x = c - center_x;
                size_t dist_y = r - center_y;
                uint64_t distance_sqr = (uint64_t)dist_x*dist_x + (uint64_t)dist_y*dist_y;
                distances[distance_sqr]++;
            }
        }
    }

    //  Filter out pixels close to center
    size_t count = 0;
    uint64_t distance_sqr_th = 0;
    for (auto& item : distances){
        count += item.second;
        if (count >= num_pixels_to_remove){
            distance_sqr_th = item.first;
            break;
        }
    }
    for (size_t r = 0; r < height; r++){
        for (size_t c = 0; c < width; c++){
            size_t dist_x = c - center_x;
            size_t dist_y = r - center_y;
            uint64_t distance_sqr = (uint64_t)dist_x*dist_x + (uint64_t)dist_y*dist_y;
            if (distance_sqr < distance_sqr_th){
                matrix.set(c, r, false);
            }
        }
    }

    return std::pair<PackedBinaryMatrix2, size_t>(std::move(matrix), distance_sqr_th);
}


void draw_matrix_on_image(
    const PackedBinaryMatrix2& matrix,
    uint32_t color, ImageRGB32& image, size_t offset_x, size_t offset_y
){
    for (size_t x = 0; x < matrix.width(); x++){
        for (size_t y = 0; y < matrix.height(); y++){
            if (matrix.get(x, y)){
                image.pixel(offset_x + x, offset_y + y) = (uint32_t)color;
            }
        }
    }
}


void draw_object_on_image(
    const Kernels::Waterfill::WaterfillObject& obj,
    const uint32_t& color, ImageRGB32& image, size_t offset_x, size_t offset_y
){
    for (size_t x = 0; x < obj.width(); x++){
        for (size_t y = 0; y < obj.height(); y++){
            if (obj.object->get(obj.min_x + x, obj.min_y + y)){
                image.pixel(offset_x + obj.min_x + x, offset_y + obj.min_y + y) = color;
                // cout << "Set color at " << offset_x + object.min_x + obj.min_x + x << ", " << offset_y + object.min_y + obj.min_y + y << endl;
            }
        }
    }
}




}
