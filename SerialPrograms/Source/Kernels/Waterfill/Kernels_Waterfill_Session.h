/*  Waterfill Session
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Session_H
#define PokemonAutomation_Kernels_Waterfill_Session_H

#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix.h"
#include "Kernels_Waterfill_Types.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{


class WaterfillIterator;

// Abstract base class for doing waterfill operations
// Waterfill is finding connected components on in a binary matrix.
// The implementation is in template derived class `WaterfillSession_t`.
// It is templated to allow different implementations of SIMD code.
class WaterfillSession{
public:
    virtual ~WaterfillSession() = default;
    virtual void set_source(PackedBinaryMatrix_IB& source) = 0;

    virtual std::unique_ptr<WaterfillIterator> make_iterator(size_t min_area) = 0;

    //  Get the object at the specific bit position.
    //  The object will be removed from the input matrix.
    //  Return true if there is an object at the bit (x, y); false otherwise.
    //  If keep_object is true, object.object is constructed.
    virtual bool find_object_on_bit(
        WaterfillObject& object, bool keep_object,
        size_t x, size_t y
    ) = 0;

};
std::unique_ptr<WaterfillSession> make_WaterfillSession();
std::unique_ptr<WaterfillSession> make_WaterfillSession(PackedBinaryMatrix_IB& matrix);



class WaterfillIterator{
public:
    virtual ~WaterfillIterator() = default;

    //  Returns false is nothing is left.
    virtual bool find_next(WaterfillObject& object, bool keep_object) = 0;
};




}
}
}
#endif
