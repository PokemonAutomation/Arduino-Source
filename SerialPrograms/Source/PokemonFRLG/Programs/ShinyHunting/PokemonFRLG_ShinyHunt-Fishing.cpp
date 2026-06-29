/*  Shiny Hunt - Fishing
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Program Overview:
 *  This program automates fishing encounters in Pokemon FireRed/LeafGreen
 *  and stops when a shiny Pokemon is detected.
 *
 *  Assumptions:
 *  - Fishing rod is registered to the SELECT button
 *  - Player is facing fishable water
 *  - Lead Pokemon can always flee (Smoke Ball recommended)
 *  - Text speed is set to FAST
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_ShinyHunt-Fishing.h"

namespace PokemonAutomation {
namespace NintendoSwitch {
namespace PokemonFRLG {


ShinyHuntFishing_Descriptor::ShinyHuntFishing_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:ShinyHuntFishing",
        Pokemon::STRING_POKEMON + " FRLG",
        "Shiny Hunt - Fishing",
        "Programs/PokemonFRLG/ShinyHunt-Fishing.html",
        "Automated fishing shiny hunt.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct ShinyHuntFishing_Descriptor::Stats : public StatsTracker {
    Stats()
        : encounters(m_stats["Encounters"])
        , shinies(m_stats["Shinies"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Encounters");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& encounters;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker>
ShinyHuntFishing_Descriptor::make_stats() const {return std::make_unique<Stats>();}


ShinyHuntFishing::ShinyHuntFishing() : SingleSwitchProgramInstance()
    , GO_HOME_WHEN_DONE(true)
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATION_SHINY("Shiny found", true, true, ImageAttachmentMode::JPG, {"Notifs", "Showcase"})
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_SHINY,
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void ShinyHuntFishing::program(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context
){
    ShinyHuntFishing_Descriptor::Stats& stats = env.current_stats<ShinyHuntFishing_Descriptor::Stats>();

    home_black_border_check(env.console, context);

    /*
    Program setup requirements:
    - Player must be facing fishable water
    - Fishing rod must be registered
    - Text speed must be FAST
    */
    env.log("FRLG Fishing shiny hunt started.");

    while (true){
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);
        int fish_result = fish_encounter(env.console, context);

        switch (fish_result){
        case -1:
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "program(): No fish hooked after 5 minutes. Please ensure you are facing water with a rod registered.",
                env.console
            );
            break;
        case 0:
            stats.encounters++;
            env.update_stats();
            flee_battle(env.console, context);
            context.wait_for_all_requests();
            continue;
        case 1:
            stats.shinies++;
            env.update_stats();
            send_program_notification(
                env,
                NOTIFICATION_SHINY,
                COLOR_YELLOW,
                "Shiny found!",
                {},
                "",
                env.console.video().snapshot(),
                true
            );
            break;
        }
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}
}  // namespace PokemonFRLG
}  // namespace NintendoSwitch
}  // namespace PokemonAutomation
