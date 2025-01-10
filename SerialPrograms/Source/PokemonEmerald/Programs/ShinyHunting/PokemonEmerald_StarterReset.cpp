/*  Emerald Starter Reset
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonEmerald_StarterReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonEmerald{

StarterReset_Descriptor::StarterReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonEmerald:StarterReset",
        "Pokemon Emerald", "Starter Reset",
        "ComputerControl/blob/master/Wiki/Programs/PokemonEmerald/StarterReset.md",
        "Soft reset for a shiny starter. Battery cannot be dry.",
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

struct StarterReset_Descriptor::Stats : public StatsTracker{
    Stats()
        : dupe_attempts(m_stats["Dupe Attempts"])
    {
        m_display_order.emplace_back("Dupe Attempts");
    }
    std::atomic<uint64_t>& dupe_attempts;
};
std::unique_ptr<StatsTracker> StarterReset_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

StarterReset::StarterReset()
    : ATTEMPTS(
        "<b>Duplication Attempts:</b><br>The number of times you wish to run this routine.",
        LockMode::UNLOCK_WHILE_RUNNING,
        100
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        // &NOTIFICATION_ERROR_FATAL,
        })
{
    PA_ADD_OPTION(ATTEMPTS);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void StarterReset::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    StarterReset_Descriptor::Stats& stats = env.current_stats<StarterReset_Descriptor::Stats>();

    /*
    * Stand in front of birch's bag.
    * 
    * text speed fast
    * 
    * have to do the SR method instead of run away
    * 
    * cannot have dry battery
    */

    uint32_t c = 0;


    /*
    start at birch bag

    starter selection

    wild zig appears, shiny check (audio?)

    go starter

    now shiny check

    track starter shiny, zig shiny, number of attempts, errors

    if not shiny, soft reset

    soft reset checks for dry battery and returns true or false!
    
    */



    while (c < ATTEMPTS){
        env.log("Current Attempts: " + tostr_u_commas(c));

        // increment counter, increment stats
        c++;
        stats.dupe_attempts++;
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

    }

    GO_HOME_WHEN_DONE.run_end_of_program(context);
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}

