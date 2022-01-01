/*  Water Fill Mask (64-bit integer)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_WaterFill_Mask64_H
#define PokemonAutomation_Kernels_WaterFill_Mask64_H

#include <ostream>
#include "Common/Compiler.h"
#include "Kernels/Kernels_BitReverse_x64.h"

namespace PokemonAutomation{
namespace Kernels{
namespace WaterFill{


struct Mask64{
    uint64_t forward;
    uint64_t reverse;

    PA_FORCE_INLINE Mask64() = default;
    PA_FORCE_INLINE Mask64(const Mask64& x)
        : forward(x.forward)
        , reverse(x.reverse)
    {}
    PA_FORCE_INLINE Mask64(uint64_t x)
        : forward(x)
        , reverse(bit_reverse64_x64(x))
    {}
    PA_FORCE_INLINE Mask64(const uint64_t& f, const uint64_t& r)
        : forward(f)
        , reverse(r)
    {}

    PA_FORCE_INLINE operator bool() const{
        return forward != 0;
    }

    PA_FORCE_INLINE void operator&=(const Mask64& x){
        forward &= x.forward;
        reverse &= x.reverse;
    }

    friend std::ostream& operator<<(std::ostream& stream, const Mask64& x){
        for (size_t c = 0; c < 64; c++){
            stream << ((x.forward >> c) & 1);
        }
        return stream;
    }
};




}
}
}
#endif
