/*  Waterfill Core (AVX512-GF)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Core_x64_AVX512GF_H
#define PokemonAutomation_Kernels_Waterfill_Core_x64_AVX512GF_H

#include "Kernels_Waterfill_Core_x64_AVX512.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{
struct Waterfill_x64_AVX512GF : public Waterfill_x64_AVX512{



//  Run Waterfill algorithm on mask "m" with starting point "x".
//  Save result back into "x".
static void Waterfill_expand(const BinaryTile_AVX512& m, BinaryTile_AVX512& x);




};
}
}
}
#endif
