/*  Alpha Crobat Hunter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
//#include "CommonFramework/Tools/InterruptableCommands.h"
//#include "CommonFramework/Tools/SuperControlSession.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/PokemonLA_MapDetector.h"
#include "PokemonLA/Inference/PokemonLA_DialogDetector.h"
#include "PokemonLA/Inference/PokemonLA_OverworldDetector.h"
#include "PokemonLA/Inference/PokemonLA_ShinySoundDetector.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include "PokemonLA_CrobatFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


CrobatFinder_Descriptor::CrobatFinder_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:CrobatFinder",
        STRING_POKEMON + " LA", "Alpha Crobat Hunter",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/AlphaCrobatHunter.md",
        "Constantly reset the cave to find Shiny Alpha Crobat.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


CrobatFinder::CrobatFinder(const CrobatFinder_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , SHINY_DETECTED("2 * TICKS_PER_SECOND")
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &SHINY_DETECTED.NOTIFICATIONS,
        &NOTIFICATION_PROGRAM_FINISH,
//        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(NOTIFICATIONS);
}


class CrobatFinder::Stats : public StatsTracker, public ShinyStatIncrementer{
public:
    Stats()
        : attempts(m_stats["Attempts"])
        , errors(m_stats["Errors"])
        , shinies(m_stats["Shinies"])
    {
        m_display_order.emplace_back("Attempts");
        m_display_order.emplace_back("Errors", true);
        m_display_order.emplace_back("Shinies", true);
    }
    virtual void add_shiny() override{
        shinies++;
    }

    std::atomic<uint64_t>& attempts;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& shinies;
};

std::unique_ptr<StatsTracker> CrobatFinder::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


void CrobatFinder::run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    // NOTE: there's no "stunned by alpha" detection in case any of the close ones are alphas!
    Stats& stats = env.stats<Stats>();

    stats.attempts++;

    // program should be started right in front of the entrance
    // so enter the sub region
    env.console.log("Entering Wayward Cave...");
    mash_A_to_enter_sub_area(env, context, env.console);

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
        throw OperationFailedException(env.console, "Unable to find Wyrdeer after 10 attempts.");
    }

    env.console.log("Beginning Shiny Detection...");
    // start the shiny detection, there's nothing initially
    {
        ShinySoundDetector shiny_detector(env.console, SHINY_DETECTED.stop_on_shiny());
        run_until(
            env, context, env.console,
            [](BotBaseContext& context){

                // FORWARD PORTION OF CAVE UNTIL LEDGE
                pbf_press_button(context, BUTTON_B, (uint16_t)(2.2 * TICKS_PER_SECOND), 80); // wyrdeer sprint
                pbf_move_left_joystick(context, 0, 128, 10, 20); // turn left
                pbf_press_button(context, BUTTON_ZL, 20, 50); // align camera

                // ASCEND THE LEDGE WITH BRAVIARY
                pbf_press_dpad(context, DPAD_RIGHT, 20, 50); // swap to braviary
                pbf_wait(context, (uint16_t)(0.6 * TICKS_PER_SECOND)); // wait for the ascent
                pbf_press_button(context, BUTTON_Y, (uint16_t)(2.4 * TICKS_PER_SECOND), 20); // descend to swap to Wyrdeer automatically

                // TO CROBAT PORTION
                pbf_press_button(context, BUTTON_B, (uint16_t)(1.05 * TICKS_PER_SECOND), 80); // sprint forward for a split second
                pbf_move_left_joystick(context, 255, 150, 10, 20); // rotate slightly right
                pbf_press_button(context, BUTTON_ZL, 20, 70); // align camera
                pbf_move_left_joystick(context, 128, 0, (uint16_t)(3.8 * TICKS_PER_SECOND), 0); // forward to crobat check

            },
            { &shiny_detector }
        );
        if (shiny_detector.detected()){
           stats.shinies++;
           on_shiny_sound(env, context, env.console, SHINY_DETECTED, shiny_detector.results());
        }
    };

    // then reset since no shiny was found
    env.console.log("No shiny detected, restarting the game!");

    pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
    reset_game_from_home(env, context, env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
}


void CrobatFinder::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    Stats& stats = env.stats<Stats>();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);

    while (true){
        env.update_stats();
        send_program_status_notification(
            env.logger(), NOTIFICATION_STATUS,
            env.program_info(),
            "",
            stats.to_str()
        );
        try{
            run_iteration(env, context);
        }catch (OperationFailedException&){
            stats.errors++;
            pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
            reset_game_from_home(env, context, env.console, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
        }
    }

    env.update_stats();
    send_program_finished_notification(
        env.logger(), NOTIFICATION_PROGRAM_FINISH,
        env.program_info(),
        "",
        stats.to_str()
    );
}



}
}
}
