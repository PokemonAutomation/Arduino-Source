/*  Alpha Froslass Finder
*
*  From: https://github.com/PokemonAutomation/Arduino-Source
*
*/

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Inference/BlackScreenDetector.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ButtonDetector.h"
#include "PokemonLA/Inference/PokemonLA_MapDetector.h"
#include "PokemonLA/Inference/PokemonLA_DialogDetector.h"
#include "PokemonLA/Inference/PokemonLA_OverworldDetector.h"
#include "PokemonLA/Inference/PokemonLA_UnderAttackDetector.h"
#include "PokemonLA/Inference/Sounds/PokemonLA_ShinySoundDetector.h"
#include "PokemonLA/Programs/PokemonLA_MountChange.h"
#include "PokemonLA/Programs/PokemonLA_GameEntry.h"
#include "PokemonLA/Programs/PokemonLA_RegionNavigation.h"
#include "PokemonLA/Programs/ShinyHunting/PokemonLA_FroslassFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

FroslassFinder_Descriptor::FroslassFinder_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:AlphaFroslassFinder",
        STRING_POKEMON + " LA", "Alpha Froslass Hunter",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/AlphaFroslassHunter.md",
        "Constantly reset to find a Alpha Froslass or any Shiny in the path.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

FroslassFinder::FroslassFinder(const FroslassFinder_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , SHINY_DETECTED("Shiny Detected Action", "", "0 * TICKS_PER_SECOND")
    , SKIP_PATH_SHINY("<b>Skip any Shines on the route to the cave.</b><br>Only care about shines inside the cave.", false)
    , NOTIFICATION_STATUS("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS,
        &SHINY_DETECTED.NOTIFICATIONS,
        &NOTIFICATION_PROGRAM_FINISH,
//        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_STATIC(SHINY_REQUIRES_AUDIO);
    PA_ADD_OPTION(SHINY_DETECTED);
    PA_ADD_OPTION(SKIP_PATH_SHINY);
    PA_ADD_OPTION(NOTIFICATIONS);
}

class FroslassFinder::Stats : public StatsTracker, public ShinyStatIncrementer{
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

std::unique_ptr<StatsTracker> FroslassFinder::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


void FroslassFinder::run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    Stats& stats = env.stats<Stats>();

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

    //Route to cave entrance
    {
        ShinyDetectedActionOption SHINY_DETECTED_ON_ROUTE("Shiny Detected Action", "", QString::number(SHINY_DETECTED.SCREENSHOT_DELAY));
        SHINY_DETECTED_ON_ROUTE.NOTIFICATIONS = SHINY_DETECTED.NOTIFICATIONS;
        SHINY_DETECTED_ON_ROUTE.ACTION.set(!SKIP_PATH_SHINY);

        float shiny_coefficient = 1.0;
        ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool{
            //  Warning: This callback will be run from a different thread than this function.
            stats.shinies++;
            shiny_coefficient = error_coefficient;
            return on_shiny_callback(env, env.console, SHINY_DETECTED, error_coefficient);
        });

        int ret = run_until(
            env.console, context,
                [](BotBaseContext& context){
                pbf_press_button(context, BUTTON_B, (uint16_t)(2 * TICKS_PER_SECOND), 10);  //Get some distance from the moutain
                pbf_press_button(context, BUTTON_Y, (uint16_t)(4 * TICKS_PER_SECOND), 10);  //Descend
                pbf_press_button(context, BUTTON_B, (uint16_t)(7.89 * TICKS_PER_SECOND), 10); //Reach to the cave entrance
                pbf_wait(context, (uint16_t)(0.5 * TICKS_PER_SECOND));
                pbf_press_button(context, BUTTON_PLUS, 10,10);
                pbf_wait(context, (uint16_t)(1.1 * TICKS_PER_SECOND));
                pbf_press_button(context, BUTTON_PLUS, 10,10);
                pbf_press_button(context, BUTTON_B, (uint16_t)(2.8 * TICKS_PER_SECOND), 10); // Braviary Second Push
            },
            {{shiny_detector}}
        );
        if (ret == 0){
            on_shiny_sound(env, env.console, context, SHINY_DETECTED, shiny_coefficient);
        }
    }

    //Move to Froslass
    {
        float shiny_coefficient = 1.0;
        ShinySoundDetector shiny_detector(env.console, [&](float error_coefficient) -> bool{
            //  Warning: This callback will be run from a different thread than this function.
            stats.shinies++;
            shiny_coefficient = error_coefficient;
            return on_shiny_callback(env, env.console, SHINY_DETECTED, error_coefficient);
        });

        int ret = run_until(
                env.console, context,
                [](BotBaseContext& context){
                pbf_press_dpad(context, DPAD_LEFT, 20, 20);
                pbf_press_button(context, BUTTON_B, (uint16_t)(4.5 * TICKS_PER_SECOND), 10);
            },
            {{shiny_detector}}
        );
        if (ret == 0){
            on_shiny_sound(env, env.console, context, SHINY_DETECTED, shiny_coefficient);
        }
    }

    env.console.log("No shiny detected, returning to Jubilife!");
    pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
    reset_game_from_home(env, env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
}


void FroslassFinder::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
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
            reset_game_from_home(env, env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
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
