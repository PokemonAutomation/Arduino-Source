/*  Max Lair AI
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/CRC32.h"
//#include "CommonFramework/Environment/Environment.h"
#include "PokemonSwSh_MaxLair_AI.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


int random(int min, int max){
//    uint64_t seed = x86_rdtsc();
    uint64_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    seed = pabb_crc32(0, &seed, sizeof(seed));
    seed %= (max - min + 1);
    return (int)seed + min;
}


}
}
}
}
