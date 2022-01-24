/*  Waterfill Algorithm
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_H
#define PokemonAutomation_Kernels_Waterfill_H

#include <vector>
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix.h"
#include "Kernels_Waterfill_Types.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{



//  Gets an object sitting on the specified tile.
//  If an object is found, it is stored into "object". Returns true.
//  If tile is empty, returns false.
bool find_object_in_tile(
    PackedBinaryMatrix& matrix,
    WaterfillObject& object,
    size_t tile_x, size_t tile_y
);

//  Gets an object sitting on the specified bit.
//  If an object is found, it is stored into "object". Returns true.
//  If bit is 0, returns false.
bool find_object_on_bit(
    PackedBinaryMatrix& matrix,
    WaterfillObject& object,
    size_t x, size_t y
);



//  Find all the objects in the matrix.
//  "keep_objects" will keep the object inside the "object" field, but
//  the cost potentially a lot of memory.
std::vector<WaterfillObject> find_objects(const PackedBinaryMatrix& matrix, size_t min_area, bool keep_objects);

//  This one is faster (avoids a matrix copy), but will zero out the
//  matrix as part of the algorithm. So make sure you copy it if you
//  want to keep it.
std::vector<WaterfillObject> find_objects_inplace(PackedBinaryMatrix& matrix, size_t min_area, bool keep_objects);


//  This one returns the objects one-by-one as they are found. This
//  avoids needing to keep them all in memory at once.
class WaterFillIterator{
public:
    WaterFillIterator(PackedBinaryMatrix& matrix, size_t min_area);

    bool find_next(WaterfillObject& object);

private:
    PackedBinaryMatrix& m_matrix;
    size_t m_min_area;
    size_t m_tile_row;
    size_t m_tile_col;
};



}
}
}
#endif
