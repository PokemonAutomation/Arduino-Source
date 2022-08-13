/*  Shiny Hunt - Custom Path
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_ShinyHuntCustomPath_H
#define PokemonAutomation_PokemonLA_ShinyHuntCustomPath_H

#include "Common/Qt/Options/BooleanCheckBoxOption.h"
#include "Common/Qt/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/Options/EnumDropdownOption.h"
#include "CommonFramework/Options/FloatingPointOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/PokemonLA_Locations.h"
#include "PokemonLA/PokemonLA_TravelLocations.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLA/Options/PokemonLA_TravelLocation.h"
#include "PokemonLA/Options/PokemonLA_CustomPathTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



class ShinyHuntCustomPath_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntCustomPath_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHuntCustomPath : public SingleSwitchProgramInstance{
public:
    ShinyHuntCustomPath();
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    // Run the custom path on overworld.
    void run_path(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    // Do one action (while ignoring listen-related actions)
    void do_non_listen_action(ConsoleHandle& console, BotBaseContext& context, size_t action_index);

private:
    ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;

//    TravelLocationOption TRAVEL_LOCATION;

    CustomPathTable PATH;

    EnumDropdownOption RESET_METHOD;

    BooleanCheckBoxOption TEST_PATH;

    ShinyDetectedActionOption SHINY_DETECTED_ENROUTE;
    ShinyDetectedActionOption SHINY_DETECTED_DESTINATION;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
