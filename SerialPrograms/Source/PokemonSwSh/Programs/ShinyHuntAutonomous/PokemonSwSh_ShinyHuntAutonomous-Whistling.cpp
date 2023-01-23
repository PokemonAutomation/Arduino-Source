/*  Shiny Hunt Autonomous - Whistling
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Device.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh/Programs/PokemonSwSh_EncounterHandler.h"
#include "PokemonSwSh_ShinyHuntAutonomous-Whistling.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousWhistling_Descriptor::ShinyHuntAutonomousWhistling_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntAutonomousWhistling",
        STRING_POKEMON + " SwSh", "Shiny Hunt Autonomous - Whistling",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/ShinyHuntAutonomous-Whistling.md",
        "Stand in one place and whistle. Shiny hunt everything that attacks you using video feedback.",
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct ShinyHuntAutonomousWhistling_Descriptor::Stats : public ShinyHuntTracker{
    Stats()
        : ShinyHuntTracker(true)
        , m_unexpected_battles(m_stats["Unexpected Battles"])
    {
        m_display_order.insert(m_display_order.begin() + 2, Stat("Unexpected Battles"));
        m_aliases["Timeouts"] = "Errors";
    }
    std::atomic<uint64_t>& m_unexpected_battles;
};
std::unique_ptr<StatsTracker> ShinyHuntAutonomousWhistling_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



ShinyHuntAutonomousWhistling::ShinyHuntAutonomousWhistling()
    : GO_HOME_WHEN_DONE(false)
    , ENCOUNTER_BOT_OPTIONS(true, true)
    , NOTIFICATIONS({
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_NONSHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_SHINY,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_SUCCESS,
        &ENCOUNTER_BOT_OPTIONS.NOTIFICATION_CATCH_FAILED,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , EXIT_BATTLE_TIMEOUT(
        "<b>Exit Battle Timeout:</b><br>After running, wait this long to return to overworld.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "10 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(TIME_ROLLBACK_HOURS);

    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(EXIT_BATTLE_TIMEOUT);
}



void ShinyHuntAutonomousWhistling::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_back_out(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    const uint32_t PERIOD = (uint32_t)TIME_ROLLBACK_HOURS * 3600 * TICKS_PER_SECOND;
    uint32_t last_touch = system_clock(context);

    ShinyHuntAutonomousWhistling_Descriptor::Stats& stats = env.current_stats<ShinyHuntAutonomousWhistling_Descriptor::Stats>();
    env.update_stats();

    StandardEncounterHandler handler(
        env, env.console, context,
        LANGUAGE,
        ENCOUNTER_BOT_OPTIONS,
        stats
    );

    while (true){
        //  Touch the date.
        if (TIME_ROLLBACK_HOURS > 0 && system_clock(context) - last_touch >= PERIOD){
            pbf_press_button(context, BUTTON_HOME, 10, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE);
            rollback_hours_from_home(context, TIME_ROLLBACK_HOURS, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY);
            resume_game_no_interact(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            last_touch += PERIOD;
        }

        context.wait_for_all_requests();
        {
            StandardBattleMenuWatcher battle_menu_detector(false);
            StartBattleWatcher start_battle_detector;

            int result = run_until(
                env.console, context,
                [](BotBaseContext& context){
                    while (true){
                        pbf_mash_button(context, BUTTON_LCLICK, TICKS_PER_SECOND);
                        pbf_move_right_joystick(context, 192, 128, TICKS_PER_SECOND, 0);
                    }
                },
                {
                    {battle_menu_detector},
                    {start_battle_detector},
                }
            );

            switch (result){
            case 0:
                env.log("Unexpected battle menu.", COLOR_RED);
                stats.m_unexpected_battles++;
                env.update_stats();
                pbf_mash_button(context, BUTTON_B, TICKS_PER_SECOND);
                run_away(env.console, context, EXIT_BATTLE_TIMEOUT);
                continue;
            case 1:
                env.log("Battle started!");
                break;
            }
        }

        //  Detect shiny.
        ShinyDetectionResult result = detect_shiny_battle(
            env.console, context,
            SHINY_BATTLE_REGULAR,
            std::chrono::seconds(30)
        );

        bool stop = handler.handle_standard_encounter_end_battle(result, EXIT_BATTLE_TIMEOUT);
        if (stop){
            break;
        }
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}



}
}
}

