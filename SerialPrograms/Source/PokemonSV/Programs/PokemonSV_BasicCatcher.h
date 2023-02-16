/*  Basic Catcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_BasicCatcher_H
#define PokemonAutomation_PokemonSV_BasicCatcher_H

#include "ClientSource/Connection/BotBase.h"
#include "PokemonSV/Inference/Battles/PokemonSV_BattleBallReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


//  Returns the quantity of the ball.
//  Returns -1 if unable to read.
int16_t move_to_ball(
    const BattleBallReader& reader, ConsoleHandle& console, BotBaseContext& context,
    const std::string& ball_slug
);



enum class CatchResult{
    POKEMON_CAUGHT,
    POKEMON_FAINTED,
    OWN_FAINTED,    //  Not implemented yet. Will show up as TIMEOUT for now.
    OUT_OF_BALLS,
    CANNOT_THROW_BALL,
//    TIMEOUT,
};
struct CatchResults{
    CatchResult result;
    uint16_t balls_used;
};
CatchResults basic_catcher(
    ConsoleHandle& console, BotBaseContext& context,
    Language language,
    const std::string& ball_slug
);




}
}
}
#endif
