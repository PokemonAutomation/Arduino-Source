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



//  Gets an object sitting on the specified bit.
//  If an object is found, it is stored into "object". Returns true.
//  If bit is 0, returns false.
bool find_object_on_bit(PackedBinaryMatrix_IB& matrix, WaterfillObject& object, size_t x, size_t y);



//  Find all the objects in the matrix.
//  "keep_objects" will keep the object inside the "object" field, but
//  the cost potentially a lot of memory.
std::vector<WaterfillObject> find_objects(const PackedBinaryMatrix_IB& matrix, size_t min_area, bool keep_objects);

//  This one is faster (avoids a matrix copy), but will zero out the
//  matrix as part of the algorithm. So make sure you copy it if you
//  want to keep it.
std::vector<WaterfillObject> find_objects_inplace(PackedBinaryMatrix_IB& matrix, size_t min_area, bool keep_objects);


//  This one returns the objects one-by-one as they are found. This
//  avoids needing to keep them all in memory at once.
class WaterfillIterator2{
public:
    virtual ~WaterfillIterator2() = default;
    WaterfillIterator2(size_t min_area) : m_min_area(min_area) {}

    virtual bool find_next(WaterfillObject& object) = 0;

protected:
    size_t m_min_area;
    size_t m_tile_row = 0;
    size_t m_tile_col = 0;
};
std::unique_ptr<WaterfillIterator2> make_WaterfillIterator(PackedBinaryMatrix_IB& matrix, size_t min_area);





}
}
}
#endif
