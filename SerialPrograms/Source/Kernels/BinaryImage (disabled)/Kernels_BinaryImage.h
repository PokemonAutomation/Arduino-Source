/*  Binary Image
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryImage_H
#define PokemonAutomation_Kernels_BinaryImage_H

#include "Kernels/Kernels_Arch.h"

#if 0
#elif defined PA_Arch_x64_SSE42
#include "Kernels_BinaryImage_x64_SSE42.h"
#else
#include "Kernels_BinaryImage_Default.h"
#endif


namespace PokemonAutomation{
namespace Kernels{


#if 0
#elif defined PA_Arch_x64_SSE42
using BinaryImage = BinaryImage_x64_SSE42;
#else
using BinaryImage = BinaryImage_Default;
#endif


}
}
#endif
