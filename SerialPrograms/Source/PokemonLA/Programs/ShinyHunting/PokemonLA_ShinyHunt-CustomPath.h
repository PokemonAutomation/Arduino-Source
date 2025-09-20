/*  Shiny Hunt - Custom Path
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_ShinyHuntCustomPath_H
#define PokemonAutomation_PokemonLA_ShinyHuntCustomPath_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_MiscOptions.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
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
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    // Run the custom path on overworld.
    void run_path(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    // Do one action (while ignoring listen-related actions)
    void do_non_listen_action(
        VideoStream& stream, ProControllerContext& context,
        const CustomPathTableRow& row
    );

private:
    ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;

    CustomPathTableFromJubilife PATH;

    ResetMethodOption RESET_METHOD;

    TimeOfDayOption TIME_OF_DAY;
    SimpleIntegerOption<uint32_t> RUNS_PER_TIME_RESET;

    BooleanCheckBoxOption TEST_PATH;

    OverworldShinyDetectedActionOption SHINY_DETECTED_ENROUTE;
    OverworldShinyDetectedActionOption SHINY_DETECTED_DESTINATION;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
