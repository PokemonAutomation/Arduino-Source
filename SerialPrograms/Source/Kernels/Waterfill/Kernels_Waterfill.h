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



//  Find all the objects in the matrix. This will destroy "matrix".
std::vector<WaterfillObject> find_objects_inplace(PackedBinaryMatrix_IB& matrix, size_t min_area);




//  This one returns the objects one-by-one as they are found. This
//  avoids needing to keep them all in memory at once.
class WaterfillIterator{
public:
    virtual ~WaterfillIterator() = default;

    virtual bool find_next(WaterfillObject& object, bool keep_object) = 0;

};
std::unique_ptr<WaterfillIterator> make_WaterfillIterator(PackedBinaryMatrix_IB& matrix, size_t min_area);





}
}
}
#endif
