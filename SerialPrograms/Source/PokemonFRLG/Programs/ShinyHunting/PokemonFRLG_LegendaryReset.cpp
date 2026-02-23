/*  Legendary Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Pokemon/Pokemon_Strings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/Inference/Sounds/PokemonFRLG_ShinySoundDetector.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_LegendaryReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

LegendaryReset_Descriptor::LegendaryReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:LegendaryReset",
        Pokemon::STRING_POKEMON + " FRLG", "Legendary Reset",
        "Programs/PokemonFRLG/LegendaryReset.html",
        "Shiny hunt legendary Pokemon using soft resets.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct LegendaryReset_Descriptor::Stats : public StatsTracker{
    Stats()
        : resets(m_stats["Resets"])
        , shinies(m_stats["Shinies"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> LegendaryReset_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

LegendaryReset::LegendaryReset()
    : GO_HOME_WHEN_DONE(true)
    , NOTIFICATION_SHINY(
        "Shiny found",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_SHINY,
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void LegendaryReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    LegendaryReset_Descriptor::Stats& stats = env.current_stats<LegendaryReset_Descriptor::Stats>();

    /*
    * Settings: Text Speed fast. Default borders. Audio required.
    * Setup: Stand in front of target. Save the game.
    * Lead can be shiny, reset is before lead is even sent out.
    * This is the same as RSE resets.
    */

    while (true) {
        //Talk to target
        pbf_press_button(context, BUTTON_A, 320ms, 320ms);

        //Mash B until black screen detected but not over (entered battle)
        BlackScreenWatcher battle_entered(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 10000ms);
                context.wait_for_all_requests();
            },
            {battle_entered}
        );
        context.wait_for_all_requests();
        if (ret != 0){
            stats.errors++;
            env.update_stats();
            env.log("Failed to enter battle.", COLOR_RED);
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to enter battle.",
                env.console
            );
        }
        else {
            env.log("Battle started.");
        }

        //handle_encounter will wait for "POKEMON appeared!"
        bool legendary_shiny = handle_encounter(env.console, context, true);
        if (legendary_shiny) {
            stats.shinies++;

            send_program_notification(env, NOTIFICATION_SHINY, COLOR_YELLOW, "Shiny found!", {}, "", env.console.video().snapshot(), true);
            break;
        }

        //No shiny found
        env.log("Soft resetting.");
        send_program_status_notification(
            env, NOTIFICATION_STATUS_UPDATE,
            "Soft resetting."
        );

        soft_reset(env.program_info(), env.console, context);
        stats.resets++;
        env.update_stats();
        context.wait_for_all_requests();
    }

    if (GO_HOME_WHEN_DONE) {
        pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}

