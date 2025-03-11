/*  Basic Pokemon Catcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BasicCatcher_H
#define PokemonAutomation_PokemonSwSh_BasicCatcher_H

#include "CommonFramework/Language.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleBallReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Pokemon;


//  Returns the quantity of the ball.
//  Returns -1 if unable to read.
int16_t move_to_ball(
    const BattleBallReader& reader, VideoStream& stream, ProControllerContext& context,
    const std::string& ball_slug
);


struct CatchResults{
    CatchResult result;
    uint16_t balls_used;
};
CatchResults basic_catcher(
    VideoStream& stream, ProControllerContext& context,
    Language language,
    const std::string& ball_slug, uint16_t ball_limit
);



}
}
}
#endif
