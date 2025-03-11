/*  Alignment Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AlignmentTools_H
#define PokemonAutomation_Kernels_AlignmentTools_H

#include <cstddef>
#include <type_traits>
#include "Common/Compiler.h"

namespace PokemonAutomation{
namespace Kernels{


template <typename T>
struct SizeOf{
    static const size_t value = sizeof(T);
};
template <>
struct SizeOf<const void>{
    static const size_t value = 1;
};
template <>
struct SizeOf<void>{
    static const size_t value = 1;
};



template <size_t alignment, typename LengthType>
PA_FORCE_INLINE LengthType align_int_up(LengthType x){
    static_assert(std::is_unsigned<LengthType>::value, "Length must be an unsigned integer.");
    static_assert((alignment & (alignment - 1)) == 0, "Alignment must be a power-of-two.");
    constexpr LengthType MASK = alignment - 1;
    return (x + MASK) & ~MASK;
}


}
}
#endif
