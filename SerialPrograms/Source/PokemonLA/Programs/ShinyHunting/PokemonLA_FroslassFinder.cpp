/*  Alpha Froslass Finder
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
#include "PokemonLA/PokemonLA_TravelLocations.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLA/Programs/PokemonLA_MountChange.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include "PokemonLA/Programs/ShinyHunting/PokemonLA_FroslassFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;



FroslassFinder_Descriptor::FroslassFinder_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:AlphaFroslassFinder",
        STRING_POKEMON + " LA", "Alpha Froslass Hunter",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/AlphaFroslassHunter.md",
        "Constantly reset to find a Alpha Froslass or any Shiny in the path.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
class FroslassFinder_Descriptor::Stats : public StatsTracker, public ShinyStatIncrementer{
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
std::unique_ptr<StatsTracker> FroslassFinder_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



FroslassFinder::FroslassFinder()
    : DASH_DURATION(
        "<b>Braviary dash duration:</b><br>"
        "How many ticks for Braviary to dash to reach the hole.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "986"
    )
    , SHINY_DETECTED_ENROUTE(
        "Enroute Shiny Action",
        "This applies if a shiny is detected while enroute to the cave. (Does not ignore Misdreavus and Glalie)",
        "0 * TICKS_PER_SECOND"
    )
    , SHINY_DETECTED_DESTINATION(
        "Destination Shiny Action",
        "This applies if a shiny is detected at or near Froslass.",
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
    PA_ADD_OPTION(DASH_DURATION);
    PA_ADD_OPTION(SHINY_DETECTED_ENROUTE);
    PA_ADD_OPTION(SHINY_DETECTED_DESTINATION);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void FroslassFinder::run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    FroslassFinder_Descriptor::Stats& stats = env.current_stats<FroslassFinder_Descriptor::Stats>();

    stats.attempts++;

    goto_camp_from_jubilife(env, env.console, context, TravelLocations::instance().Icelands_Arena);

    //Start path
    env.console.log("Beginning Shiny Detection...");

    //Setup
    pbf_move_left_joystick(context, 108, 255, 20, 20);
    pbf_press_button(context, BUTTON_ZL, 10,10);
    pbf_wait(context, (uint16_t)(0.5 * TICKS_PER_SECOND));
    change_mount(env.console, context, MountState::BRAVIARY_ON);
    pbf_wait(context, (uint16_t)(0.5 * TICKS_PER_SECOND));

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
                //  Route to cave entrance
                pbf_press_button(context, BUTTON_B, (uint16_t)(2 * TICKS_PER_SECOND), 10);  //Get some distance from the moutain
                pbf_press_button(context, BUTTON_Y, (uint16_t)(4 * TICKS_PER_SECOND), 10);  //Descend
                pbf_press_button(context, BUTTON_B, DASH_DURATION, 10); //Reach to the cave entrance
                pbf_wait(context, (uint16_t)(0.5 * TICKS_PER_SECOND));
                pbf_press_button(context, BUTTON_PLUS, 10,10);
                pbf_wait(context, (uint16_t)(1.1 * TICKS_PER_SECOND));
                pbf_press_button(context, BUTTON_PLUS, 10,10);
                pbf_press_button(context, BUTTON_B, (uint16_t)(2.8 * TICKS_PER_SECOND), 10); // Braviary Second Push

                context.wait_for_all_requests();
                shiny_action.store(&SHINY_DETECTED_DESTINATION, std::memory_order_release);

                //  Move to Froslass
                pbf_press_dpad(context, DPAD_LEFT, 20, 20);
                pbf_press_button(context, BUTTON_B, (uint16_t)(4.5 * TICKS_PER_SECOND), 10);
            },
            {{shiny_detector}}
        );
        shiny_detector.throw_if_no_sound();
        if (ret == 0){
            ShinyDetectedActionOption* action = shiny_action.load(std::memory_order_acquire);
            on_shiny_sound(env, env.console, context, *action, shiny_coefficient);
        }
    }

    env.console.log("No shiny detected, Reset game!");
    pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
    reset_game_from_home(env, env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
}


void FroslassFinder::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    FroslassFinder_Descriptor::Stats& stats = env.current_stats<FroslassFinder_Descriptor::Stats>();

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
