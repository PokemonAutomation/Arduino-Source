/*  Shiny Hunt Autonomous - Berry Tree
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDateSpam.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "CommonFramework/Inference/VisualInferenceRoutines.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_EncounterHandler.h"
#include "PokemonSwSh_ShinyHuntAutonomous-BerryTree.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousBerryTree_Descriptor::ShinyHuntAutonomousBerryTree_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntAutonomousBerryTree",
        "Shiny Hunt Autonomous - Berry Tree",
        "SwSh-Arduino/wiki/Advanced:-ShinyHuntAutonomous-BerryTree",
        "Automatically hunt for shiny berry tree " + STRING_POKEMON + " using video feedback.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ShinyHuntAutonomousBerryTree::ShinyHuntAutonomousBerryTree(const ShinyHuntAutonomousBerryTree_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , ENCOUNTER_BOT_OPTIONS(false, true)
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , EXIT_BATTLE_TIMEOUT(
        "<b>Exit Battle Timeout:</b><br>After running, wait this long to return to overworld.",
        "10 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);

    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);

    PA_ADD_OPTION(m_advanced_options);
    PA_ADD_OPTION(EXIT_BATTLE_TIMEOUT);
}



std::unique_ptr<StatsTracker> ShinyHuntAutonomousBerryTree::make_stats() const{
    return std::unique_ptr<StatsTracker>(new ShinyHuntTracker(true));
}




void ShinyHuntAutonomousBerryTree::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_no_interact(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }

    ShinyHuntTracker& stats = env.stats<ShinyHuntTracker>();
    env.update_stats();

    StandardEncounterHandler handler(
        m_descriptor.display_name(),
        env, env.console,
        LANGUAGE,
        ENCOUNTER_BOT_OPTIONS,
        stats
    );

    uint8_t year = MAX_YEAR;
    while (true){
        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_FAST);
        home_roll_date_enter_game_autorollback(env.console, &year);
//        home_to_date_time(env.console, true, true);
//        neutral_date_skip(env.console);
//        settings_to_enter_game(env.console, true);
        pbf_mash_button(env.console, BUTTON_B, 90);
        env.console.botbase().wait_for_all_requests();

        {
            StandardBattleMenuDetector battle_menu_detector(false);
            StartBattleDetector start_battle_detector(env.console);

            int result = run_until(
                env, env.console,
                [](const BotBaseContext& context){
                    pbf_mash_button(context, BUTTON_A, 60 * TICKS_PER_SECOND);
                    context.botbase().wait_for_all_requests();
                },
                {
                    &battle_menu_detector,
                    &start_battle_detector,
                }
            );

            switch (result){
            case 0:
                env.log("Unexpected battle menu.", Qt::red);
                stats.add_error();
                env.update_stats();
                pbf_mash_button(env.console, BUTTON_B, TICKS_PER_SECOND);
                run_away(env, env.console, EXIT_BATTLE_TIMEOUT);
                continue;
            case 1:
                env.log("Battle started!");
                break;
            default:
                stats.add_error();
                env.update_stats();
                env.log("Timed out.");
                continue;
            }
        }

        //  Detect shiny.
        ShinyDetectionResult result = detect_shiny_battle(
            env.console,
            env, env.console, env.console,
            SHINY_BATTLE_REGULAR,
            std::chrono::seconds(30)
        );

        bool stop = handler.handle_standard_encounter_end_battle(result, EXIT_BATTLE_TIMEOUT);
        if (stop){
            break;
        }
//        pbf_mash_button(env.console, BUTTON_B, 10 * TICKS_PER_SECOND);
    }

    pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);

    home_to_date_time(env.console, false, false);
    pbf_press_button(env.console, BUTTON_A, 5, 5);
    pbf_press_button(env.console, BUTTON_A, 5, 10);
    pbf_press_button(env.console, BUTTON_HOME, 10, SETTINGS_TO_HOME_DELAY);

    if (!GO_HOME_WHEN_DONE){
        pbf_press_button(env.console, BUTTON_HOME, 10, HOME_TO_GAME_DELAY);
    }

    end_program_callback(env.console);
    end_program_loop(env.console);
}




}
}
}
