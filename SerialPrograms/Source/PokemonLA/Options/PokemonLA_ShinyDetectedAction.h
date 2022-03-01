/*  Shiny Detected Action
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_ShinyDetectedAction_H
#define PokemonAutomation_PokemonLA_ShinyDetectedAction_H

#include "CommonFramework/Options/BatchOption/GroupOption.h"
//#include "CommonFramework/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"

namespace PokemonAutomation{
    class BotBaseContext;
    class EventNotificationOption;
    class StatsTracker;
    class ProgramEnvironment;
    class ConsoleHandle;
    class AsyncCommandSession;
namespace NintendoSwitch{
namespace PokemonLA{


enum class ShinyDetectedAction{
    IGNORE,
    STOP_PROGRAM,
    TAKE_VIDEO_STOP_PROGRAM,
};


class ShinyDetectedActionOption : public GroupOption{
public:
    ShinyDetectedActionOption();

    bool do_nothing() const;

//    BooleanCheckBoxOption STOP_PROGRAM;
//    BooleanCheckBoxOption TAKE_VIDEO;
    EnumDropdownOption ACTION;
    TimeExpressionOption<uint16_t> VIDEO_DELAY;
    EventNotificationOption NOTIFICATIONS;
};


//  Return true if program should stop.
bool run_on_shiny(
    const BotBaseContext& context,
    const ShinyDetectedActionOption& option
);

bool run_on_shiny(
    ProgramEnvironment& env, ConsoleHandle& console,
    AsyncCommandSession& command_session,
    ShinyDetectedActionOption& option,
    const StatsTracker* session_stats
);


}
}
}
#endif
