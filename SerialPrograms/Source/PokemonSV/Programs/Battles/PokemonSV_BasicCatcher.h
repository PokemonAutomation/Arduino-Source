/*  Basic Catcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_BasicCatcher_H
#define PokemonAutomation_PokemonSV_BasicCatcher_H

#include <functional>
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSV/Inference/Battles/PokemonSV_BattleBallReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


//  Returns the quantity of the ball.
//  Returns -1 if unable to read.
int16_t move_to_ball(
    const BattleBallReader& reader, ConsoleHandle& console, ControllerContext& context,
    const std::string& ball_slug
);

//  Throw a ball. If error, throw an exception.
//  Returns the quantity prior to throwing the ball.
//  If ball is not found, returns zero.
int16_t throw_ball(
    ConsoleHandle& console, ControllerContext& context,
    Language language, const std::string& ball_slug
);



struct CatchResults{
    CatchResult result;
    uint16_t balls_used;
};
CatchResults basic_catcher(
    ConsoleHandle& console, ControllerContext& context,
    Language language,
    const std::string& ball_slug, uint16_t ball_limit,
    bool use_first_move_if_cant_throw,
    std::function<void()> on_throw_lambda = nullptr
);




}
}
}
#endif
