/*  Alpha Gallade Hunter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include "PokemonLA_GalladeFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;


GalladeFinder_Descriptor::GalladeFinder_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:GalladeFinder",
        STRING_POKEMON + " LA", "Alpha Gallade Hunter",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/AlphaGalladeHunter.md",
        "Constantly reset the Snowpoint Temple to find Shiny Alpha Gallade.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
class GalladeFinder_Descriptor::Stats : public StatsTracker, public ShinyStatIncrementer{
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
std::unique_ptr<StatsTracker> GalladeFinder_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


GalladeFinder::GalladeFinder()
    : SHINY_DETECTED_ENROUTE(
        "Enroute Shiny Action",
        "This applies if you are still traveling to the Gallade.",
        "0 * TICKS_PER_SECOND"
    )
    , SHINY_DETECTED_DESTINATION(
        "Destination Shiny Action",
        "This applies if you are near the Gallade.",
        "0 * TICKS_PER_SECOND"
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



void GalladeFinder::run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    // NOTE: there's no "stunned by alpha" detection in case the first spawn is an alpha!
    // NOTE: there is also no mitigation for if you get attacked by a Kirlia if it hates you
    GalladeFinder_Descriptor::Stats& stats = env.current_stats<GalladeFinder_Descriptor::Stats>();

    stats.attempts++;

    // program should be started right in front of the entrance
    // so enter the sub region
    env.console.log("Entering Snowpoint Temple...");
    mash_A_to_enter_sub_area(env, env.console, context);

    env.console.log("Beginning navigation to the Alpha Gallade...");
    // start the shiny detection, there's nothing initially
    env.console.log("Enabling Shiny Detection...");
    {
        float shiny_coefficient = 1.0;
        std::atomic<ShinyDetectedActionOption*> shiny_action(&SHINY_DETECTED_ENROUTE);

        ShinySoundDetector shiny_detector(env.console.logger(), env.console, [&](float error_coefficient) -> bool{
            //  Warning: This callback will be run from a different thread than this function.
            stats.shinies++;
            shiny_coefficient = error_coefficient;
            ShinyDetectedActionOption* action = shiny_action.load(std::memory_order_acquire);
            return on_shiny_callback(env, env.console, *action, error_coefficient);
        });

        int ret = run_until(
            env.console, context,
            [&](BotBaseContext& context){
                // forward portion
                pbf_controller_state(context, BUTTON_LCLICK, DPAD_NONE, 128, 0, 128, 128, (uint16_t)(6.8 * TICKS_PER_SECOND)); // forward while running until stairs, mash y a few times down the stairs
                pbf_mash_button(context, BUTTON_Y,(uint16_t)(2.8 * TICKS_PER_SECOND)); // roll down the stairs, recover stamina
                pbf_controller_state(context, BUTTON_LCLICK, DPAD_NONE, 128, 0, 128, 128, (uint16_t)(4.0 * TICKS_PER_SECOND)); // forward while sprinting again
                pbf_mash_button(context, BUTTON_Y,(uint16_t)(2.0 * TICKS_PER_SECOND)); // two mashes and then one y
                pbf_controller_state(context, BUTTON_LCLICK, DPAD_NONE, 128, 0, 128, 128, (uint16_t)(3.8 * TICKS_PER_SECOND)); // forward while sprinting again
                // basic map layout is walk forward for a while, move right, run back, then align camera, then walk left then forward to Gallade

                // right portion
                pbf_move_left_joystick(context, 255, 128, (uint16_t)(0.5 * TICKS_PER_SECOND), 0); // right alone
                pbf_controller_state(context, BUTTON_LCLICK, DPAD_NONE, 255, 128, 128, 128, (uint16_t)(2.4 * TICKS_PER_SECOND)); // forward while running until stairs
                pbf_mash_button(context, BUTTON_Y,(uint16_t)(1.8 * TICKS_PER_SECOND)); // roll down the stairs, recover stamina
                pbf_move_left_joystick(context, 255, 128, (uint16_t)(1.8 * TICKS_PER_SECOND), 20); // right alone

                // down portion
                // pbf_move_left_joystick(context, 128, 255, (uint16_t)(1.9 * TICKS_PER_SECOND), 20); // OLD down
                pbf_controller_state(context, BUTTON_LCLICK, DPAD_NONE, 128, 255, 128, 128, (uint16_t)(1.8 * TICKS_PER_SECOND));

                // camera align
                pbf_press_button(context, BUTTON_ZL, 20, 0); // camera align
                pbf_wait(context, 70);

                context.wait_for_all_requests();
                shiny_action.store(&SHINY_DETECTED_DESTINATION, std::memory_order_release);

                pbf_move_left_joystick(context, 0, 128, 2 * TICKS_PER_SECOND, 0); // left

                // then forward left
                pbf_move_left_joystick(context, 0, 0, (uint16_t)(1.1 * TICKS_PER_SECOND), 0);

                //pbf_move_left_joystick(context, 128, 0, 3.9 * TICKS_PER_SECOND, 0); // OLD forward
                pbf_controller_state(context, BUTTON_LCLICK, DPAD_NONE, 128, 0, 128, 128, (uint16_t)(3.5 * TICKS_PER_SECOND)); // forward while sprinting until stairs, mash y a few times down the stairs
                // we should easily be in range of gallade at this point, so if there's no shiny we're done
            },
            {{shiny_detector}}
        );
        shiny_detector.throw_if_no_sound();
        if (ret == 0){
            ShinyDetectedActionOption* action = shiny_action.load(std::memory_order_acquire);
            on_shiny_sound(env, env.console, context, *action, shiny_coefficient);
        }
    };

    // then reset
    env.console.log("No shiny detected, restarting the game!");

    pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
    reset_game_from_home(env, env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
}


void GalladeFinder::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    GalladeFinder_Descriptor::Stats& stats = env.current_stats<GalladeFinder_Descriptor::Stats>();

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

            pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
            reset_game_from_home(env, env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
        }
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}



}
}
}
