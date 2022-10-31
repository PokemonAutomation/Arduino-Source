/*  Basic RNG manipulation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/ConsoleHandle.h"
#include "PokemonSwSh/Inference/RNG/PokemonSwSh_OrbeetleAttackAnimationDetector.h"
#include "PokemonSwSh/Programs/RNG/PokemonSwSh_Xoroshiro128Plus.h"

namespace PokemonAutomation {
class BotBaseContext;
namespace NintendoSwitch {
namespace PokemonSwSh {

// Performs 128 Orbeetle attack animations. 
// Returns the state after those animations.
Xoroshiro128PlusState find_rng_state(
    ConsoleHandle& console, BotBaseContext& context,
    bool save_screenshots, bool log_values
);

// Performs Orbeetle attack animations until only one possible state is left.
// Returns the state after those animations.
Xoroshiro128PlusState refind_rng_state(
    ConsoleHandle& console,
    BotBaseContext& context,
    Xoroshiro128PlusState last_known_state,
    size_t min_advances,
    size_t max_advances,
    bool save_screenshots,
    bool log_values
);

void do_rng_advances(ConsoleHandle& env, BotBaseContext& context, Xoroshiro128Plus& rng, size_t advances, uint16_t press_duration = 10, uint16_t release_duration = 10);



}
}
}
