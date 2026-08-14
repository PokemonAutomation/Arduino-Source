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
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonRSE/Inference/Dialogs/PokemonRSE_DialogDetector.h"
#include "PokemonRSE/PokemonRSE_Navigation.h"
#include "PokemonRSE_LegendaryReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

LegendaryReset_Descriptor::LegendaryReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonRSE:LegendaryReset",
        Pokemon::STRING_POKEMON + " RSE", "Legendary Reset",
        "Programs/PokemonRSE/LegendaryReset.html",
        "Shiny hunt legendary Pokemon using soft resets.",
        ProgramControllerClass::StandardController_NoRestrictions,
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
    : TARGET(
        "<b>Target:</b><br>",
        {
            {Target::press_a,       "press_a",      "Press A: Rayquaza, Regis, Latios/Latias (Southern Island), Voltorb, Electrode, Kecleon"},
            {Target::walk_left,     "walk_left",    "Walk Left: Kyogre, Groudon"}, //if emerald is fixed, +deoxys hooh lugia
            {Target::walk_right,    "walk_right",   "Walk Right: Kyogre, Groudon"},//tested groudon, regi, ray, kecleon
        },
        LockMode::LOCK_WHILE_RUNNING,
        Target::press_a
    )
    , TAKE_VIDEO("<b>Take Video:</b><br>Record a video when the shiny is found.", LockMode::UNLOCK_WHILE_RUNNING, true)
    , GO_HOME_WHEN_DONE(true)
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
    PA_ADD_STATIC(SHINY_REQUIRES_AUDIO);
    PA_ADD_OPTION(TARGET);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void LegendaryReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    LegendaryReset_Descriptor::Stats& stats = env.current_stats<LegendaryReset_Descriptor::Stats>();

    home_black_border_check(env.console, context);

    while (true){
        //Mash B until black screen detected but not over (entered battle)
        BlackScreenWatcher battle_entered(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                switch (TARGET){
                case Target::press_a:
                    env.log("Target: Press A.");
                    //Mash A as Kecleon has a Yes/No dialog
                    pbf_mash_button(context, BUTTON_A, 3000ms);
                    break;
                case Target::walk_left:
                    env.log("Target: Walk Left.");
                    //Step forward to start the encounter.
                    pbf_press_dpad(context, DPAD_LEFT, 320ms, 100ms);
                    pbf_press_dpad(context, DPAD_LEFT, 320ms, 100ms);
                    break;
                case Target::walk_right:
                    env.log("Target: Walk Right.");
                    pbf_press_dpad(context, DPAD_RIGHT, 320ms, 100ms);
                    pbf_press_dpad(context, DPAD_RIGHT, 320ms, 100ms);
                    break;
                }
                context.wait_for_all_requests();

                //Long mash to clear dialog+wait for those long animations
                env.log("Mashing B through animation.");
                pbf_mash_button(context, BUTTON_B, 120s);
                context.wait_for_all_requests();
            },
            {battle_entered}
        );
        context.wait_for_all_requests();
        if (ret != 0){
            stats.errors++;
            env.update_stats();
            env.log("Failed to enter battle.", COLOR_RED);
            stats.errors += soft_reset(env.console, context);
            continue;
        }else{
            env.log("Battle started.");
        }

        bool legendary_shiny = handle_encounter(env.console, context, false);
        if (legendary_shiny){
            stats.shinies++;
            env.update_stats();
            send_program_notification(
                env,
                NOTIFICATION_SHINY,
                COLOR_YELLOW,
                "Shiny found!",
                {}, "",
                env.console.video().snapshot(),
                true
            );
            if (TAKE_VIDEO){
                pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 0ms);
            }
            break;
        }

        //No shiny found
        env.log("Soft resetting.");
        send_program_status_notification(
            env, NOTIFICATION_STATUS_UPDATE,
            "Soft resetting."
        );

        stats.errors += soft_reset(env.console, context);
        stats.resets++;
        env.update_stats();
        context.wait_for_all_requests();
    }

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}

