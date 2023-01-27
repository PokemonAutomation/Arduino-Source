/*  Tera Battler
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraBattler_H
#define PokemonAutomation_PokemonSV_TeraBattler_H

#include "ClientSource/Connection/BotBase.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "PokemonSV/Options/PokemonSV_TeraAIOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


//  Run a tera battle until you either win or lose.
bool run_tera_battle(
    ProgramEnvironment& env,
    ConsoleHandle& console, BotBaseContext& context,
    EventNotificationOption& error_notification,
    TeraAIOption& battle_AI
);




}
}
}
#endif
