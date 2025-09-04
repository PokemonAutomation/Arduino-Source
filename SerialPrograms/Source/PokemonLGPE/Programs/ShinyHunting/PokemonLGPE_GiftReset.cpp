/*  LGPE Gift Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Joycon.h"
#include "NintendoSwitch/Programs/NintendoSwitch_GameEntry.h"
#include "Pokemon/Pokemon_Strings.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "PokemonLGPE/Inference/PokemonLGPE_ShinySymbolDetector.h"
#include "PokemonLGPE/Programs/PokemonLGPE_GameEntry.h"
#include "PokemonLGPE_GiftReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

GiftReset_Descriptor::GiftReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLGPE:GiftReset",
        Pokemon::STRING_POKEMON + " LGPE", "Gift Reset",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLGPE/GiftReset.md",
        "Shiny hunt gift Pokemon by resetting the game.",
        ProgramControllerClass::SpecializedController,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct GiftReset_Descriptor::Stats : public StatsTracker{
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
std::unique_ptr<StatsTracker> GiftReset_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

GiftReset::GiftReset()
    : EXTRA_DIALOG(
        "<b>Persian/Arcanine:</b><br>Check this if the gift Pokemon is Persian or Arcanine.",
        LockMode::UNLOCK_WHILE_RUNNING, false
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_SHINY(
        "Shiny Found",
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
    PA_ADD_OPTION(EXTRA_DIALOG);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void GiftReset::program(SingleSwitchProgramEnvironment& env, CancellableScope& scope){
    JoyconContext context(scope, env.console.controller<RightJoycon>());
    assert_16_9_720p_min(env.logger(), env.console);
    GiftReset_Descriptor::Stats& stats = env.current_stats<GiftReset_Descriptor::Stats>();

    /*
    Setup:
    Stand in front of trade NPC.
    Start the program in-game.

    Must have 500yen for magikarp.
    Gift Pokemon: https://www.serebii.net/letsgopikachueevee/gift.shtml
    Tested with Magikarp on a new save. Should work with most of the others.
    Can always add a dropdown with target if it doesn't.
    Fossils will be handled in a different program.
    */

    bool shiny_found = false;
    while (!shiny_found) {
        //Purchase Magikarp
        BlackScreenOverWatcher gift_obtained(COLOR_RED);
        int ret = run_until<JoyconContext>(
            env.console, context,
            [](JoyconContext& context){
                pbf_mash_button(context, BUTTON_A, 20000ms);
            },
            {gift_obtained}
        );
        context.wait_for_all_requests();
        if (ret != 0){
            stats.errors++;
            env.update_stats();
            env.log("Failed to receive gift Pokemon.", COLOR_RED);
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to receive gift Pokemon.",
                env.console
            );
        }
        else {
            env.log("Received gift Pokemon.");
        }
        send_program_status_notification(
            env, NOTIFICATION_STATUS_UPDATE,
            "Received gift Pokemon."
        );

        if (EXTRA_DIALOG){
            env.log("Persian/Arcanine selected. Mashing B to exit dialog.");
            pbf_mash_button(context, BUTTON_B, 4100ms);
            context.wait_for_all_requests();
        }

        //Wait a bit.
        pbf_wait(context, 2500ms);
        context.wait_for_all_requests();

        //Open menu, open party, open boxes
        env.log("Opening boxes.");
        pbf_press_button(context, BUTTON_X, 200ms, 500ms);
        pbf_press_button(context, BUTTON_A, 200ms, 1500ms);
        pbf_press_button(context, BUTTON_Y, 200ms, 2000ms);
        context.wait_for_all_requests();

        //Sort by order caught
        env.log("Sorting by order caught.");
        pbf_press_button(context, BUTTON_Y, 200ms, 1000ms);
        pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
        pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
        context.wait_for_all_requests();

        //Press left to go to last (most recent) Pokemon
        env.log("Opening summary of most recent Pokemon.");
        pbf_move_joystick(context, 0, 128, 100ms, 100ms);
        context.wait_for_all_requests();

        //View summary - it takes a moment to load
        env.log("Viewing summary.");
        pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
        pbf_move_joystick(context, 128, 255, 100ms, 100ms);
        pbf_move_joystick(context, 128, 255, 100ms, 100ms);
        pbf_press_button(context, BUTTON_A, 200ms, 100ms);
        context.wait_for_all_requests();

        pbf_wait(context, 5000ms);
        context.wait_for_all_requests();

        //Now check for shinies. Check everything that was traded.
        VideoSnapshot screen = env.console.video().snapshot();
        ShinySymbolDetector shiny_checker(COLOR_YELLOW);
        bool check = shiny_checker.read(env.console.logger(), screen);

        if (check) {
            env.log("Shiny detected!");
            stats.shinies++;
            env.update_stats();
            send_program_notification(env, NOTIFICATION_SHINY, COLOR_YELLOW, "Shiny found!", {}, "", screen, true);
            shiny_found = true;
        }
        else {
            env.log("Not shiny. Resetting game.");
            send_program_status_notification(
                env, NOTIFICATION_STATUS_UPDATE,
                "Not shiny. Resetting game."
            );

            reset_game_from_game(env, env.console, context, &stats.errors, 3000ms);
            stats.resets++;
            env.update_stats();
        }
    }

    if (GO_HOME_WHEN_DONE) {
        pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}
