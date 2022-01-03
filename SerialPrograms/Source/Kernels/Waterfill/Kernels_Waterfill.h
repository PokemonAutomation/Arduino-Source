/*  Waterfill Algorithm
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_H
#define PokemonAutomation_Kernels_Waterfill_H

#include <vector>
#include "Kernels/BinaryMatrix/Kernels_PackedBinaryMatrix.h"
#include "Kernels_Waterfill_Types.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{



//  Gets an object sitting on the specified tile.
//  If an object is found, it is stored into "object". Returns true.
//  If tile is empty, returns false.
bool find_object(
    PackedBinaryMatrix& matrix,
    WaterFillObject& object,
    size_t tile_x, size_t tile_y
);



//  Find all the objects in the matrix.
//  "keep_objects" will keep the object inside the "object" field, but
//  the cost potentially a lot of memory.
std::vector<WaterFillObject> find_objects(const PackedBinaryMatrix& matrix, size_t min_area, bool keep_objects);

//  This one is faster (avoids a matrix copy), but will zero out the
//  matrix as part of the algorithm. So make sure you copy it if you
//  want to keep it.
std::vector<WaterFillObject> find_objects_inplace(PackedBinaryMatrix& matrix, size_t min_area, bool keep_objects);





}
}
}
#endif
