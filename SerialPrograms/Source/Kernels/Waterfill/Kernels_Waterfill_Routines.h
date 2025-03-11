/*  Waterfill Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Routines_H
#define PokemonAutomation_Kernels_Waterfill_Routines_H

#include <vector>
#include <set>
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix_t.h"
#include "Kernels/BinaryMatrix/Kernels_PackedBinaryMatrixCore.h"
#include "Kernels/BinaryMatrix/Kernels_SparseBinaryMatrixCore.h"
#include "Kernels_Waterfill_Types.h"
#include "Kernels_Waterfill_Session.tpp"
#include "Kernels_Waterfill.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{




//  Find all the objects in the matrix. This will destroy "matrix".
template <typename Tile, typename TileRoutines>
std::vector<WaterfillObject> find_objects_inplace(PackedBinaryMatrixCore<Tile>& matrix, size_t min_area){
    std::vector<WaterfillObject> ret;
    WaterfillSession_t<Tile, TileRoutines> session(matrix);
    for (size_t r = 0; r < matrix.tile_height(); r++){
        for (size_t c = 0; c < matrix.tile_width(); c++){
            while (true){
                WaterfillObject object;
                if (!session.find_object_in_tile(object, false, c, r)){
                    break;
                }
                object.object.reset();
                if (object.area >= min_area){
                    ret.emplace_back(object);
                }
            }
        }
    }
    return ret;
}







}
}
}
#endif
