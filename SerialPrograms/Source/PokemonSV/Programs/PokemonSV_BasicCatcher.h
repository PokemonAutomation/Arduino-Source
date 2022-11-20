/*  Basic Catcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_BasicCatcher_H
#define PokemonAutomation_PokemonSV_BasicCatcher_H

#include "ClientSource/Connection/BotBase.h"
#include "PokemonSV/Inference/PokemonSV_BattleBallReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


//  Returns the quantity of the ball.
//  Returns -1 if unable to read.
int16_t move_to_ball(
    const BattleBallReader& reader, ConsoleHandle& console, BotBaseContext& context,
    const std::string& ball_slug
);







}
}
}
#endif
