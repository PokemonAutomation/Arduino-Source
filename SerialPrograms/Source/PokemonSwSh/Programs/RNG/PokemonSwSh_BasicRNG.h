/*  Basic RNG manipulation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "Pokemon/Pokemon_Xoroshiro128Plus.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;

// Performs 128 Orbeetle attack animations. 
// Returns the state after those animations.
Xoroshiro128PlusState find_rng_state(
    VideoStream& stream, ProControllerContext& context,
    bool save_screenshots, bool log_values
);

// Performs Orbeetle attack animations until only one possible state is left.
// Returns the state after those animations.
Xoroshiro128PlusState refind_rng_state(
    VideoStream& stream,
    ProControllerContext& context,
    Xoroshiro128PlusState last_known_state,
    size_t min_advances,
    size_t max_advances,
    bool save_screenshots,
    bool log_values
);

void do_rng_advances(
    VideoStream& stream, ProControllerContext& context,
    Xoroshiro128Plus& rng,
    size_t advances,
    Milliseconds press_duration,
    Milliseconds release_duration
);



}
}
}
