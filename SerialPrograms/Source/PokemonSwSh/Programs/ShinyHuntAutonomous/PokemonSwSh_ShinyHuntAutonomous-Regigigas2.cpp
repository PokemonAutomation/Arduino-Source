/*  Shiny Hunt Autonomous - Regigigas2
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
#include "PokemonSwSh/Inference/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_RaidCatchDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_StartGame.h"
#include "PokemonSwSh/Programs/PokemonSwSh_EncounterHandler.h"
#include "PokemonSwSh_ShinyHuntAutonomous-Regigigas2.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousRegigigas2_Descriptor::ShinyHuntAutonomousRegigigas2_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntAutonomousRegigigas2",
        "Shiny Hunt Autonomous - Regigigas2",
        "SwSh-Arduino/wiki/Advanced:-ShinyHuntAutonomous-Regigigas2",
        "Automatically hunt for shiny Regigigas using video feedback.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ShinyHuntAutonomousRegigigas2::ShinyHuntAutonomousRegigigas2(const ShinyHuntAutonomousRegigigas2_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , REVERSAL_PP(
        "<b>Reversal PP:</b><br>The amount of Reversal PP you are saved with.",
        24
    )
    , FILTER(false, false)
    , TOUCH_DATE_INTERVAL(
        "<b>Rollover Prevention:</b><br>Prevent a den from rolling over by periodically touching the date. If set to zero, this feature is disabled.",
        "4 * 3600 * TICKS_PER_SECOND"
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , CATCH_TO_OVERWORLD_DELAY(
        "<b>Catch to Overworld Delay:</b>",
        "8 * TICKS_PER_SECOND"
    )
    , VIDEO_ON_SHINY(
        "<b>Video Capture:</b><br>Take a video of the encounter if it is shiny.",
        true
    )
{
    m_options.emplace_back(&START_IN_GRIP_MENU, "START_IN_GRIP_MENU");
    m_options.emplace_back(&GO_HOME_WHEN_DONE, "GO_HOME_WHEN_DONE");

    m_options.emplace_back(&REVERSAL_PP, "REVERSAL_PP");
    m_options.emplace_back(&FILTER, "FILTER");

    m_options.emplace_back(&TOUCH_DATE_INTERVAL, "TOUCH_DATE_INTERVAL");
    m_options.emplace_back(&NOTIFICATION_LEVEL, "NOTIFICATION_LEVEL");

    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&CATCH_TO_OVERWORLD_DELAY, "CATCH_TO_OVERWORLD_DELAY");
    if (PERSISTENT_SETTINGS().developer_mode){
        m_options.emplace_back(&VIDEO_ON_SHINY, "VIDEO_ON_SHINY");
    }
}




std::unique_ptr<StatsTracker> ShinyHuntAutonomousRegigigas2::make_stats() const{
    return std::unique_ptr<StatsTracker>(
        new ShinyHuntTracker(
            true,
            {{"Timeouts", "Errors"}}
        )
    );
}



bool ShinyHuntAutonomousRegigigas2::kill_and_return(SingleSwitchProgramEnvironment& env) const{
    RaidCatchDetector detector(env.console, std::chrono::seconds(30));
    pbf_mash_button(env.console, BUTTON_A, 4 * TICKS_PER_SECOND);

    if (!detector.wait(env)){
        env.log("Raid Catch Menu not found.", Qt::red);
        return false;
    }

    pbf_press_dpad(env.console, DPAD_DOWN, 10, 0);
    pbf_press_button(env.console, BUTTON_A, 10, CATCH_TO_OVERWORLD_DELAY);
    return true;
}
void ShinyHuntAutonomousRegigigas2::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_back_out(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST, 500);
    }else{
        pbf_press_button(env.console, BUTTON_B, 5, 5);
    }

    uint32_t last_touch = system_clock(env.console);

    ShinyHuntTracker& stats = env.stats<ShinyHuntTracker>();
    env.update_stats();

    StandardEncounterHandler handler(
        m_descriptor.display_name(),
        env, env.console,
        nullptr, Language::None,
        stats,
        FILTER,
        VIDEO_ON_SHINY,
        NOTIFICATION_LEVEL
    );

    while (true){
        for (uint8_t pp = REVERSAL_PP; pp > 0; pp--){
            env.log("Starting Regigigas Encounter: " + tostr_u_commas(stats.encounters() + 1));

            pbf_mash_button(env.console, BUTTON_A, 18 * TICKS_PER_SECOND);
            env.console.botbase().wait_for_all_requests();

            if (!wait_for_start_battle(env, env.console, std::chrono::seconds(30))){
                stats.add_error();
                env.update_stats();
                break;
            }

            ShinyType shininess = detect_shiny_battle(
                env, env.console,
                SHINY_BATTLE_RAID,
                std::chrono::seconds(30)
            );
//            shininess = ShinyDetection::STAR_SHINY;

            bool stop = handler.handle_standard_encounter(shininess);
            if (stop){
                goto StopProgram;
            }

            if (shininess == ShinyType::UNKNOWN){
                stats.add_error();
                env.update_stats();
                break;
            }

            kill_and_return(env);
        }

        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
        if (TOUCH_DATE_INTERVAL > 0 && system_clock(env.console) - last_touch >= TOUCH_DATE_INTERVAL){
            touch_date_from_home(env.console, SETTINGS_TO_HOME_DELAY);
            last_touch += TOUCH_DATE_INTERVAL;
        }
        reset_game_from_home_with_inference(
            env, env.console,
            TOLERATE_SYSTEM_UPDATE_MENU_FAST
        );
    }


StopProgram:
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


