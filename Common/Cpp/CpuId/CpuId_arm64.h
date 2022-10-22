/*  CPU ID (arm64)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 */

#ifndef PokemonAutomation_CpuId_arm64_H
#define PokemonAutomation_CpuId_arm64_H

#include <stdint.h>

namespace PokemonAutomation{

// Currently we assume Apple M1 as the only arm64 we support, so not much CPU feature variety.
struct CPU_Features{
    CPU_Features& set_to_current();

    bool OK_M1 = false;
};






}
#endif
