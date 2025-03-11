/*  Basic Pokemon Catcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BasicCatcher_H
#define PokemonAutomation_PokemonBDSP_BasicCatcher_H

#include "CommonFramework/Language.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "Pokemon/Pokemon_Notification.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;
    

struct CatchResults{
    CatchResult result = CatchResult::TIMED_OUT;
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
    VideoStream& stream, ProControllerContext& context,
    Language language,
    const std::string& ball_slug, uint16_t ball_limit
);



}
}
}
#endif
