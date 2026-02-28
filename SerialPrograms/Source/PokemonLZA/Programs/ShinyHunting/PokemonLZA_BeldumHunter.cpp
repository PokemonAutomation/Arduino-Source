/*  Beldum Hunter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/VideoResolutionCheck.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
//#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
//#include "PokemonLZA/PokemonLZA_Settings.h"
//#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
//#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA/Programs/PokemonLZA_GameEntry.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLZA_BeldumHunter.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;

BeldumHunter_Descriptor::BeldumHunter_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLZA:ShinyHunt-Beldum",
        STRING_POKEMON + " LZA", "Shiny Hunt - Beldum",
        "Programs/PokemonLZA/BeldumHunter.html",
        "Repeatedly enter Lysandre Labs to shiny hunt Beldum.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
class BeldumHunter_Descriptor::Stats : public StatsTracker, public PokemonLA::ShinyStatIncrementer{
public:
    Stats()
        : attempts(m_stats["Attempts"])
        , errors(m_stats["Errors"])
        , shinies(m_stats["Shinies"])
    {
        m_display_order.emplace_back("Attempts");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Shinies");
    }
    virtual void add_shiny() override{
        shinies++;
    }

    std::atomic<uint64_t>& attempts;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& shinies;
};
std::unique_ptr<StatsTracker> BeldumHunter_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


BeldumHunter::BeldumHunter()
    : TAKE_VIDEO("<b>Take Video:</b>", LockMode::UNLOCK_WHILE_RUNNING, true)
    , NOTIFICATION_SHINY(
        "Shiny Found",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_SHINY,
        &NOTIFICATION_STATUS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_STATIC(SHINY_REQUIRES_AUDIO);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(NOTIFICATIONS);
}

bool BeldumHunter::run_iteration(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    BeldumHunter_Descriptor::Stats& stats = env.current_stats<BeldumHunter_Descriptor::Stats>();
    stats.attempts++;
    {
        float shiny_coefficient = 1.0;
        PokemonLA::ShinySoundDetector shiny_detector(env.logger(), [&](float error_coefficient) -> bool {
            shiny_coefficient = error_coefficient;
            return true;
            });

        BlackScreenOverWatcher entered(COLOR_RED, { 0.074, 0.044, 0.826, 0.278 });

        env.log("Entering the lab.");
        context.wait_for_all_requests();
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context) {
                pbf_controller_state(context, BUTTON_B, DPAD_NONE, {0, +1}, {0, 0}, 800ms);
                pbf_wait(context, 5000ms);
            },
            { {entered} }
            );
        if (ret == 0) {
            env.log("Entered the lab.");
        }else{
            env.log("Failed to enter the lab.");
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to warp.",
                env.console
            );
        }
        pbf_wait(context, 1000ms);
        context.wait_for_all_requests();

        int res = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context) {

                env.log("Go straight toward the elevator.");
                pbf_controller_state(context, BUTTON_B, DPAD_NONE, {0, +1}, {0, 0}, 3680ms);

                env.log("Go left to where the Noivern spawns, then forward and then left.");
                pbf_controller_state(context, BUTTON_B, DPAD_NONE, {-1, 0}, {0, 0}, 2400ms);
                pbf_controller_state(context, BUTTON_B, DPAD_NONE, {0, +1}, {0, 0}, 640ms);
                pbf_controller_state(context, BUTTON_B, DPAD_NONE, {-1, 0}, {0, 0}, 1440ms);

                env.log("Through the Houndoom room and down the hallway.");
                pbf_controller_state(context, BUTTON_B, DPAD_NONE, {0, -1}, {0, 0}, 1120ms);
                pbf_controller_state(context, BUTTON_B, DPAD_NONE, {-1, 0}, {0, 0}, 1600ms);
                pbf_controller_state(context, BUTTON_B, DPAD_NONE, {0, +1}, {0, 0}, 2720ms);

                env.log("Final hallway to Beldum room.");
                pbf_controller_state(context, BUTTON_B, DPAD_NONE, {-1, 0}, {0, 0}, 3200ms);
                pbf_press_button(context, BUTTON_L, 40ms, 40ms);
                context.wait_for_all_requests();
            },
            { {shiny_detector} }
            );
        shiny_detector.throw_if_no_sound();

        if (res == 0) {
            env.log("Shiny detected!");
            if (TAKE_VIDEO) {
                pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 0ms);
            }

            return true;
        }
    }
    env.console.log("No shiny detected. Resetting.");

    pbf_press_button(context, BUTTON_HOME, 160ms, 3000ms);
    reset_game_from_home(env, env.console, context, false);

    return false;
}


void BeldumHunter::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    assert_16_9_720p_min(env.logger(), env.console);

    BeldumHunter_Descriptor::Stats& stats = env.current_stats<BeldumHunter_Descriptor::Stats>();

    /*
    * Setup: Face the opening to the labs. Save the game.
    * 
    * Program will enter the Labs and then run to the Beldum room.
    * No shiny, reset the game. Repeat.
    * Every time we enter the Beldum will reroll. We reset the game instead of running back
    * due to the wild Pokemon spawns.
    * This can also hunt Noivern, Houndour, and Houndoom.
    */

    while (true){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS);
        try{
            bool shiny_found = run_iteration(env, context);
            if (shiny_found) {
                stats.shinies++;
                env.update_stats();

                pbf_press_button(context, BUTTON_HOME, 160ms, 3000ms);

                send_program_notification(env, NOTIFICATION_SHINY,
                    COLOR_YELLOW, "Shiny sound detected!", {},
                    "", env.console.video().snapshot(), true);
                break;
            }
        }catch (OperationFailedException& e){
            stats.errors++;
            e.send_notification(env, NOTIFICATION_ERROR_RECOVERABLE);

            pbf_press_button(context, BUTTON_HOME, 160ms, 3000ms);
            reset_game_from_home(env, env.console, context, false);
        }
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}



}
}
}
