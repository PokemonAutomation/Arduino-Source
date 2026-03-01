/*  Randomization
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <chrono>
#include "Common/CRC32/pabb_CRC32.h"
#include "Common/Cpp/Exceptions.h"
#include "Random.h"

namespace PokemonAutomation{



uint32_t random_u32(){
    uint64_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    uint32_t crc = 0;
    pabb_crc32_buffer(&crc, &seed, sizeof(seed));
    return crc;
}
uint32_t random_u32(uint32_t min, uint32_t max){
    if (min > max){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid random_u32() range.");
    }

    max++;
    uint32_t diff = max - min;

    //  Full range
    if (diff == 0){
        return random_u32();
    }

    //  Power-of-two
    if ((diff & (diff - 1)) == 0){
        return (random_u32() & (diff - 1)) + min;
    }

    uint32_t quo = 0xffffffff / diff;
    uint32_t retry_threshold = quo + diff;

    while (true){
        uint32_t u32 = random_u32();
        if (u32 < retry_threshold){
            return u32 % diff + min;
        }
    }
}



}
