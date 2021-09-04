/*  Shiny Hunt Autonomous - Regi
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonArray>
#include "Common/Cpp/PrettyPrint.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDateSpam.h"
#include "CommonFramework/PersistentSettings.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_EncounterHandler.h"
#include "PokemonSwSh/Programs/ShinyHuntUnattended/PokemonSwSh_ShinyHunt-Regi.h"
#include "PokemonSwSh_ShinyHuntAutonomous-Regi.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousRegi_Descriptor::ShinyHuntAutonomousRegi_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntAutonomousRegi",
        "Shiny Hunt Autonomous - Regi",
        "SwSh-Arduino/wiki/Advanced:-ShinyHuntAutonomous-Regi",
        "Automatically hunt for shiny Regi using video feedback.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ShinyHuntAutonomousRegi::ShinyHuntAutonomousRegi(const ShinyHuntAutonomousRegi_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , ENCOUNTER_BOT_OPTIONS(false, false)
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , EXIT_BATTLE_TIMEOUT(
        "<b>Exit Battle Timeout:</b><br>After running, wait this long to return to overworld.",
        "10 * TICKS_PER_SECOND"
    )
    , POST_BATTLE_MASH_TIME(
        "<b>Post-Battle Mash:</b><br>After each battle, mash B for this long to clear the dialogs.",
        "1 * TICKS_PER_SECOND"
    )
    , TRANSITION_DELAY(
        "<b>Transition Delay:</b><br>Time to enter/exit the building.",
        "5 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(TOUCH_DATE_INTERVAL);

    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(REGI_NAME);

    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);

    PA_ADD_OPTION(m_advanced_options);
    PA_ADD_OPTION(EXIT_BATTLE_TIMEOUT);
    PA_ADD_OPTION(POST_BATTLE_MASH_TIME);
    PA_ADD_OPTION(TRANSITION_DELAY);
}




std::unique_ptr<StatsTracker> ShinyHuntAutonomousRegi::make_stats() const{
    return std::unique_ptr<StatsTracker>(
        new ShinyHuntTracker(
            true,
            {{"Light Resets", "Errors"}}
        )
    );
}




void ShinyHuntAutonomousRegi::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_back_out(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }

    ShinyHuntTracker& stats = env.stats<ShinyHuntTracker>();
    env.update_stats();
//    DiscordWebHook::send_message_old(true, "Starting a new run for regi", stats.make_discord_stats());

    StandardEncounterHandler handler(
        m_descriptor.display_name(),
        env, env.console,
        LANGUAGE,
        ENCOUNTER_BOT_OPTIONS,
        stats
    );

    uint32_t last_touch = system_clock(env.console) - TOUCH_DATE_INTERVAL;
    bool error = false;
    while (true){
        pbf_mash_button(env.console, BUTTON_B, POST_BATTLE_MASH_TIME);
        move_to_corner(env, error, TRANSITION_DELAY);
        if (error){
            env.update_stats();
            error = false;
        }
//        DiscordWebHook::send_message_old(false, "Regi wasn't shiny this time", stats.make_discord_stats());

        //  Touch the date.
        if (TOUCH_DATE_INTERVAL > 0 && system_clock(env.console) - last_touch >= TOUCH_DATE_INTERVAL){
            env.log("Touching date to prevent rollover.");
            pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
            touch_date_from_home(env.console, SETTINGS_TO_HOME_DELAY);
            resume_game_no_interact(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            last_touch += TOUCH_DATE_INTERVAL;
        }

        //  Do the light puzzle.
        run_regi_light_puzzle(env, REGI_NAME, stats.encounters());

        //  Start the encounter.
        pbf_mash_button(env.console, BUTTON_A, 5 * TICKS_PER_SECOND);
        env.console.botbase().wait_for_all_requests();

        //  Detect shiny.
        ShinyDetectionResult result = detect_shiny_battle(
            env.console,
            env, env.console, env.console,
            SHINY_BATTLE_REGULAR,
            std::chrono::seconds(30)
        );
//        shininess = ShinyDetection::SQUARE_SHINY;

        bool stop = handler.handle_standard_encounter_end_battle(result, EXIT_BATTLE_TIMEOUT);
        if (stop){
            break;
        }
    }

//    DiscordWebHook::send_message_old(true, "Regi was shiny this time", stats.make_discord_stats());

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    }

    end_program_callback(env.console);
    end_program_loop(env.console);
}




}
}
}


