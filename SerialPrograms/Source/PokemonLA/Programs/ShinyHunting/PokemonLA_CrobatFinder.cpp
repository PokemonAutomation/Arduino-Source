/*  Alpha Crobat Hunter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/PokemonLA_MountDetector.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include "PokemonLA_CrobatFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

using namespace Pokemon;


CrobatFinder_Descriptor::CrobatFinder_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:CrobatFinder",
        STRING_POKEMON + " LA", "Alpha Crobat Hunter",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/AlphaCrobatHunter.md",
        "Constantly reset the cave to find Shiny Alpha Crobat.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
class CrobatFinder_Descriptor::Stats : public StatsTracker, public ShinyStatIncrementer{
public:
    Stats()
        : attempts(m_stats["Attempts"])
        , errors(m_stats["Errors"])
        , shinies(m_stats["Shinies"])
    {
        m_display_order.emplace_back("Attempts");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Shinies", HIDDEN_IF_ZERO);
    }
    virtual void add_shiny() override{
        shinies++;
    }

    std::atomic<uint64_t>& attempts;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& shinies;
};
std::unique_ptr<StatsTracker> CrobatFinder_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


CrobatFinder::CrobatFinder()
    : SHINY_DETECTED_ENROUTE(
        "Enroute Shiny Action",
        "This applies if you are still traveling to the Crobat.",
        "2000 ms"
    )
    , SHINY_DETECTED_DESTINATION(
        "Destination Shiny Action",
        "This applies if you are near the Crobat.",
        "2000 ms"
    )
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &SHINY_DETECTED_ENROUTE.NOTIFICATIONS,
        &SHINY_DETECTED_DESTINATION.NOTIFICATIONS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_STATIC(SHINY_REQUIRES_AUDIO);
    PA_ADD_OPTION(SHINY_DETECTED_ENROUTE);
    PA_ADD_OPTION(SHINY_DETECTED_DESTINATION);
    PA_ADD_OPTION(NOTIFICATIONS);
}



void CrobatFinder::run_iteration(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // NOTE: there's no "stunned by alpha" detection in case any of the close ones are alphas!
    CrobatFinder_Descriptor::Stats& stats = env.current_stats<CrobatFinder_Descriptor::Stats>();

    stats.attempts++;

    // program should be started right in front of the entrance
    // so enter the sub region
    env.console.log("Entering Wayward Cave...");
    mash_A_to_enter_sub_area(env, env.console, context);

    env.console.log("Beginning navigation to the Alpha Crobat...");
    //  Switch to Wrydeer.
    bool error = true;
    MountDetector mount_detector;
    for (size_t c = 0; c < 10; c++){
        MountState mount = mount_detector.detect(env.console.video().snapshot());
        if (mount == MountState::WYRDEER_OFF){
            pbf_press_button(context, BUTTON_PLUS, 20, 105);
            error = false;
            break;
        }
        if (mount == MountState::WYRDEER_ON){
            pbf_wait(context, 5 * TICKS_PER_SECOND);
            error = false;
            break;
        }
        pbf_press_dpad(context, DPAD_LEFT, 20, 50);
        context.wait_for_all_requests();
    }
    if (error){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Unable to find Wyrdeer after 10 attempts.",
            env.console
        );
    }

    env.console.log("Beginning Shiny Detection...");
    // start the shiny detection, there's nothing initially
    {
        float shiny_coefficient = 1.0;
        std::atomic<OverworldShinyDetectedActionOption*> shiny_action(&SHINY_DETECTED_ENROUTE);
        WallClock destination_time = WallClock::max();

        ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool{
            //  Warning: This callback will be run from a different thread than this function.
            stats.shinies++;
            shiny_coefficient = error_coefficient;
            OverworldShinyDetectedActionOption* action = shiny_action.load(std::memory_order_acquire);
            return on_shiny_callback(env, env.console, *action, error_coefficient);
        });

        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){

                // FORWARD PORTION OF CAVE UNTIL LEDGE
                pbf_press_button(context, BUTTON_B, 2200ms, 640ms); // wyrdeer sprint
                pbf_move_left_joystick(context, 0, 128, 10, 20); // turn left
                pbf_press_button(context, BUTTON_ZL, 20, 50); // align camera

                // ASCEND THE LEDGE WITH BRAVIARY
                pbf_press_dpad(context, DPAD_RIGHT, 20, 50); // swap to braviary
                pbf_wait(context, 600ms); // wait for the ascent
                pbf_press_button(context, BUTTON_Y, 2400ms, 160ms); // descend to swap to Wyrdeer automatically

                // TO CROBAT PORTION
                pbf_press_button(context, BUTTON_B, 1050ms, 640ms); // sprint forward for a split second
                pbf_move_left_joystick(context, 255, 150, 10, 20); // rotate slightly right
                pbf_press_button(context, BUTTON_ZL, 20, 70); // align camera

                context.wait_for_all_requests();
                destination_time = current_time();
                shiny_action.store(&SHINY_DETECTED_DESTINATION, std::memory_order_release);

                pbf_move_left_joystick(context, 128, 0, (uint16_t)(3.8 * TICKS_PER_SECOND), 0); // forward to crobat check

            },
            {{shiny_detector}}
        );
        shiny_detector.throw_if_no_sound();
        if (ret == 0 || shiny_detector.last_detection() > destination_time){
            OverworldShinyDetectedActionOption* action = shiny_action.load(std::memory_order_acquire);
            on_shiny_sound(env, env.console, context, *action, shiny_coefficient);
        }
    };

    // then reset since no shiny was found
    env.console.log("No shiny detected, restarting the game!");

    pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY0);
    reset_game_from_home(env, env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
}


void CrobatFinder::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    CrobatFinder_Descriptor::Stats& stats = env.current_stats<CrobatFinder_Descriptor::Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    while (true){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS);
        try{
            run_iteration(env, context);
        }catch (OperationFailedException& e){
            stats.errors++;
            e.send_notification(env, NOTIFICATION_ERROR_RECOVERABLE);

            pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY0);
            reset_game_from_home(env, env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
        }
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}



}
}
}
