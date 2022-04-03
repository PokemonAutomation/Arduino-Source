/*  Basic Pokemon Catcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BasicCatcher_H
#define PokemonAutomation_PokemonBDSP_BasicCatcher_H

#include "CommonFramework/Language.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "PokemonBDSP/Inference/Battles/PokemonBDSP_BattleBallReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
    

//  Throws the specified ball until:
//      1.  The Pokemon is caught. Returns # of balls used.
//      2.  The Pokemon faints. Returns -# of balls used.
//      3.  Your Pokemon faints. Throws an error. (will change in the future)
//      4.  You run of the ball. Throws an error.
enum class CatchResult{
    POKEMON_CAUGHT,
    POKEMON_FAINTED,
    OWN_FAINTED,    //  Not implemented yet. Will show up as TIMEOUT for now.
    OUT_OF_BALLS,
    CANNOT_THROW_BALL,  // The Pokemon is in Fly or Underground, or it's a story pokemon
                        // that cannot be caught.
    TIMEOUT,
};

struct CatchResults{
    CatchResult result = CatchResult::TIMEOUT;
    uint16_t balls_used = 0;
};

// Throw balls repeatedly to catch wild pokemon.
// It can detect whether the wild pokemon is caught, the wild pokemon is fainted,
// cannot throw balls, own pokemon fainted, out of balls or timeout.
// If both own and wild pokemon fainted and not blackout, count as wild pokemon fainted.
// If own pokemon level up and want to learn new moves, choose to not learn them.
// If the wild pokemon is caught or fainted, the game returns to the overworld when
// basic_catcher() returns.
//
// Don't handle the case that own pokemon evolving or black out to Pokecenter.
CatchResults basic_catcher(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    Language language,
    const std::string& ball_slug
);



}
}
}
#endif
