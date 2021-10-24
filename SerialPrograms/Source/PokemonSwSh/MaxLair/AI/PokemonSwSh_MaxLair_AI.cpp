/*  Max Lair AI
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <immintrin.h>
#include "PokemonSwSh_MaxLair_AI.h"

#if _WIN32
#include <intrin.h>
#endif

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{



#if _WIN32
uint64_t x86_rdtscp(){
    unsigned int dummy;
    return __rdtscp(&dummy);
}
#else
uint64_t x86_rdtscp(){
    unsigned int lo, hi;
    __asm__ volatile (
        "rdtscp"
        : "=a" (lo), "=d" (hi)
        :
        : "ecx"
    );
    return ((uint64_t)hi << 32) | lo;
}
#endif

int random(int min, int max){
    uint64_t seed = _mm_crc32_u64(x86_rdtscp(), 0);
    seed %= (max - min + 1);
    return (int)seed + min;
}







}
}
}
}
