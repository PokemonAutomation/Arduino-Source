/*  RS Starter Reset
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
#include "PokemonRSE/Inference/Dialogs/PokemonRSE_DialogDetector.h"
#include "PokemonRSE/Inference/PokemonRSE_ShinyNumberDetector.h"
#include "PokemonRSE/PokemonRSE_Navigation.h"
#include "PokemonRSE_StarterReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

StarterReset_Descriptor::StarterReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonRSE:StarterReset",
        Pokemon::STRING_POKEMON + " RSE", "[RS]Starter Reset - Video",
        "ComputerControl/blob/master/Wiki/Programs/PokemonRSE/StarterReset.md",
        "Soft reset for a shiny starter. Ruby and Sapphire only.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}

struct StarterReset_Descriptor::Stats : public StatsTracker{
    Stats()
        : resets(m_stats["Resets"])
        , shinystarter(m_stats["Shiny Starter"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shiny Starter");
    }
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinystarter;
};
std::unique_ptr<StatsTracker> StarterReset_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

StarterReset::StarterReset()
    : TARGET(
        "<b>Starter:</b><br>",
        {
            {Target::treecko, "treecko", "Treecko"},
            {Target::torchic, "torchic", "Torchic"},
            {Target::mudkip, "mudkip", "Mudkip"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Target::treecko
    )
//    , STARTER_WAIT0(
//        "<b>Send out starter wait:</b><br>After pressing A to send out your selected starter, wait this long for the animation. Make sure to add extra time in case it is shiny.",
//        LockMode::LOCK_WHILE_RUNNING,
//        "6000 ms"
//    )
    , NOTIFICATION_SHINY_STARTER(
        "Shiny Starter",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_SHINY_STARTER,
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(TARGET);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void StarterReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    StarterReset_Descriptor::Stats& stats = env.current_stats<StarterReset_Descriptor::Stats>();

    /*
    * Settings: Text Speed fast.
    * Full screen, no filter? The device I'm using to test has similar looking output, but I don't have switch online+.
    * If on a retro handheld, make sure the screen matches that of NSO+.
    * 
    * Setup: Stand in front of the Professor's bag and save the game.
    * 
    * Required to fight, so have to do the SR method instead of run away
    * Soft reset programs are only for Ruby/Sapphire, as Emerald has the 0 seed issue.
    * 
    * This also assumes no dry battery.
    * 
    * WARNING: Timings in Emerald for the battle menu are slightly different. This won't work with Emerald at all.
    */

    bool shiny_starter = false;
    while (!shiny_starter) {
        env.log("Opening bag and selecting starter.");
        pbf_press_button(context, BUTTON_A, 40, 180);

        switch (TARGET) {
        case Target::treecko:
            pbf_press_dpad(context, DPAD_LEFT, 40, 100);
            break;
        case Target::torchic:
            //Default cursor position, do nothing.
            break;
        case Target::mudkip:
            pbf_press_dpad(context, DPAD_RIGHT, 40, 100);
            break;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "StarterReset: Invalid target.",
                env.console
            );
            break;
        }
        pbf_mash_button(context, BUTTON_A, 540);
        context.wait_for_all_requests();

        env.log("Starting battle.");

        //Now mash B until the battle menu appears
        BattleMenuWatcher battle_menu(COLOR_RED);
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                pbf_mash_button(context, BUTTON_B, 1000);
            },
            {battle_menu}
        );
        context.wait_for_all_requests();
        if (ret != 0){
            env.console.log("Failed to detect battle menu.", COLOR_RED);
        }
        else {
            env.log("Battle menu detected.");
        }

        //Open the summary and check the color of the number
        pbf_press_dpad(context, DPAD_DOWN, 40, 80);
        pbf_press_button(context, BUTTON_A, 40, 80);

        BlackScreenOverWatcher detector(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
        int ret2 = wait_until(
            env.console, context,
            std::chrono::milliseconds(3000),
            {{detector}}
        );
        if (ret2 == 0){
            env.log("Entered party menu.");
        }else{
            env.log("Timed out waiting to enter party menu.", COLOR_RED);
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "StarterReset: Timed out waiting to enter party menu.",
                env.console
            );
        }

        pbf_press_button(context, BUTTON_A, 20, 180);
        pbf_press_dpad(context, DPAD_DOWN, 40, 80);
        pbf_press_button(context, BUTTON_A, 40, 80);

        //Check second party member - used for testing with hacked in shiny starter
        //pbf_press_dpad(context, DPAD_DOWN, 40, 80);

        pbf_wait(context, 125);
        context.wait_for_all_requests();

        VideoSnapshot screen = env.console.video().snapshot();
        ShinyNumberDetector shiny_checker(COLOR_YELLOW);
        shiny_starter = shiny_checker.read(env.console.logger(), screen);

        if (shiny_starter) {
            env.log("Shiny starter detected!");
            stats.shinystarter++;
            send_program_status_notification(env, NOTIFICATION_SHINY_STARTER, "Shiny starter found!", screen, true);
        }
        else {
            env.log("Starter is not shiny.");
            env.log("Soft resetting.");
            send_program_status_notification(
                env, NOTIFICATION_STATUS_UPDATE,
                "Soft resetting."
            );
            soft_reset(env.program_info(), env.console, context);
            stats.resets++;
        }
    }

    //if system set to nintendo switch, have go home when done option?

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}

