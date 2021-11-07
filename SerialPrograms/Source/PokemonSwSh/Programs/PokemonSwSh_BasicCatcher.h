/*  Basic Pokemon Catcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BasicCatcher_H
#define PokemonAutomation_PokemonSwSh_BasicCatcher_H

#include "CommonFramework/Language.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleBallReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


//  Returns the quantity of the ball.
//  Returns -1 if unable to read.
int16_t move_to_ball(
    const BattleBallReader& reader,
    ConsoleHandle& console,
    const std::string& ball_slug
);


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
    CANNOT_THROW_BALL,
    TIMEOUT,
};
struct CatchResults{
    CatchResult result;
    uint16_t balls_used;
};
CatchResults basic_catcher(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    Language language,
    const std::string& ball_slug
);



}
}
}
#endif
