/*  BDSP Blink Model
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BlinkModel_H
#define PokemonAutomation_PokemonBDSP_BlinkModel_H

#include <stddef.h>
#include <stdint.h>
#include <vector>
#include "Pokemon/Pokemon_Xorshift128.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


const double BDSP_NPC_TICK_SECONDS = 1.017;
const double BDSP_POKEMON_BLINK_MIN_SECONDS = 3.0;
const double BDSP_POKEMON_BLINK_MAX_SECONDS = 12.0;
const double BDSP_POKEMON_BLINK_OFFSET_SECONDS = 0.285;


enum class BlinkType : uint8_t{
    Single = 0,
    Double = 1,
};
const char* blink_type_name(BlinkType type);


//  An NPC blinks when bits 1-3 of its roll are all zero: about a 1 in 8 chance.
inline bool npc_blinks(uint32_t roll){
    return (roll & 0x0e) == 0;
}
//  Bit 0 decides whether it is a single or a double blink.
inline BlinkType NPC_blink_type(uint32_t roll){
    return (BlinkType)(roll & 1);
}


//  The game's float generator.
//  Note that it runs backwards: a roll of zero gives the maximum,
//  and a roll of all-ones gives the minimum
double bdsp_range_float(uint32_t roll, double minimum, double maximum);

//  Seconds until a Pokemon model's next blink.
double bdsp_pokemon_blink_interval(uint32_t roll);


const size_t BDSP_POKEMON_BLINK_KNOWN_BITS = 4;

//  Recover the 23-bit fraction from a measured interval.
//  Returns false if no roll could have produced this interval at all.
bool bdsp_pokemon_blink_fraction(double interval_seconds, uint32_t& fraction);

//  Recover just the top BDSP_POKEMON_BLINK_KNOWN_BITS of the fraction, along with
//  how much timing error that reading could absorb before it would flip to the
//  neighbouring value.
//  Returns false only if no roll could have produced this interval.
bool bdsp_pokemon_blink_bucket_with_margin(
    double interval_seconds, uint32_t& bucket, double& margin_seconds
);

struct BlinkTick{
    bool blinked = false;
    BlinkType type = BlinkType::Single;   //  Only meaningful when "blinked".
};

// Predict what one NPC would do over the given number of ticks.
std::vector<BlinkTick> generate_blink_ticks(
    Pokemon::Xorshift128 rng,
    size_t ticks,
    uint8_t npcs = 1,
    uint8_t slot = 0
);


}
}
}
#endif
