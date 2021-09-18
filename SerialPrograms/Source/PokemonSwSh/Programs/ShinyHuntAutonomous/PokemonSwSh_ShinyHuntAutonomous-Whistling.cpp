/*  Shiny Hunt Autonomous - Whistling
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
#include "PokemonSwSh_ShinyHuntAutonomous-Whistling.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousWhistling_Descriptor::ShinyHuntAutonomousWhistling_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntAutonomousWhistling",
        "Shiny Hunt Autonomous - Whistling",
        "SwSh-Arduino/wiki/Advanced:-ShinyHuntAutonomous-Whistling",
        "Stand in one place and whistle. Shiny hunt everything that attacks you using video feedback.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ShinyHuntAutonomousWhistling::ShinyHuntAutonomousWhistling(const ShinyHuntAutonomousWhistling_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , ENCOUNTER_BOT_OPTIONS(true, true)
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
    PA_ADD_OPTION(TIME_ROLLBACK_HOURS);

    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);

    PA_ADD_OPTION(m_advanced_options);
    PA_ADD_OPTION(EXIT_BATTLE_TIMEOUT);
}



struct ShinyHuntAutonomousWhistling::Stats : public ShinyHuntTracker{
    Stats()
        : ShinyHuntTracker(true)
        , m_unexpected_battles(m_stats["Unexpected Battles"])
    {
        m_display_order.insert(m_display_order.begin() + 2, Stat("Unexpected Battles"));
        m_aliases["Timeouts"] = "Errors";
    }
    uint64_t& m_unexpected_battles;
};
std::unique_ptr<StatsTracker> ShinyHuntAutonomousWhistling::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}





void ShinyHuntAutonomousWhistling::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_back_out(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }

    const uint32_t PERIOD = (uint32_t)TIME_ROLLBACK_HOURS * 3600 * TICKS_PER_SECOND;
    uint32_t last_touch = system_clock(env.console);

    Stats& stats = env.stats<Stats>();
    env.update_stats();

    StandardEncounterHandler handler(
        m_descriptor.display_name(),
        env, env.console,
        LANGUAGE,
        ENCOUNTER_BOT_OPTIONS,
        stats
    );

    while (true){
        //  Touch the date.
        if (TIME_ROLLBACK_HOURS > 0 && system_clock(env.console) - last_touch >= PERIOD){
            pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
            rollback_hours_from_home(env.console, TIME_ROLLBACK_HOURS, SETTINGS_TO_HOME_DELAY);
            resume_game_no_interact(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            last_touch += PERIOD;
        }

        env.console.botbase().wait_for_all_requests();
        {
            StandardBattleMenuDetector battle_menu_detector(false);
            StartBattleDetector start_battle_detector(env.console);

            int result = run_until(
                env, env.console,
                [](const BotBaseContext& context){
                    while (true){
                        pbf_mash_button(context, BUTTON_LCLICK, TICKS_PER_SECOND);
                        pbf_move_right_joystick(context, 192, 128, TICKS_PER_SECOND, 0);
                    }
                },
                {
                    &battle_menu_detector,
                    &start_battle_detector,
                }
            );

            switch (result){
            case 0:
                env.log("Unexpected battle menu.", Qt::red);
                stats.m_unexpected_battles++;
                env.update_stats();
                pbf_mash_button(env.console, BUTTON_B, TICKS_PER_SECOND);
                run_away(env, env.console, EXIT_BATTLE_TIMEOUT);
                continue;
            case 1:
                env.log("Battle started!");
                break;
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
    }

    env.update_stats();

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    }

    end_program_callback(env.console);
    end_program_loop(env.console);
}



}
}
}

