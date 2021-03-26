/*  ShinyHuntAutonomous-Fishing
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Clientside/PrettyPrint.h"
#include "Common/SwitchFramework/FrameworkSettings.h"
#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "Common/PokemonSwSh/PokemonSettings.h"
#include "Common/PokemonSwSh/PokemonSwShGameEntry.h"
#include "Common/PokemonSwSh/PokemonSwShDateSpam.h"
#include "PokemonSwSh/Inference/PokemonSwSh_FishingDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh_EncounterTracker.h"
#include "PokemonSwSh_ShinyHuntAutonomous-Fishing.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousFishing::ShinyHuntAutonomousFishing()
    : SingleSwitchProgram(
        FeedbackType::REQUIRED, PABotBaseLevel::PABOTBASE_12KB,
        "Shiny Hunt Autonomous - Fishing",
        "SerialPrograms/ShinyHuntAutonomous-Fishing.md",
        "Automatically hunt for shiny fishing " + STRING_POKEMON + " using video feedback."
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , EXIT_BATTLE_MASH_TIME(
        "<b>Exit Battle Time:</b><br>After running, wait this long to return to overworld and for the fish to reappear.",
        "6 * TICKS_PER_SECOND"
    )
    , FISH_RESPAWN_TIME(
        "<b>Fish Respawn Time:</b><br>Wait this long for fish to respawn.",
        "4 * TICKS_PER_SECOND"
    )
    , TIME_ROLLBACK_HOURS(
        "<b>Time Rollback (in hours):</b><br>Periodically roll back the time to keep the weather the same. If set to zero, this feature is disabled.",
        1, 0, 11
    )
{
    m_options.emplace_back(&m_advanced_options, "");
    m_options.emplace_back(&EXIT_BATTLE_MASH_TIME, "EXIT_BATTLE_MASH_TIME");
    m_options.emplace_back(&FISH_RESPAWN_TIME, "FISH_RESPAWN_TIME");
    m_options.emplace_back(&TIME_ROLLBACK_HOURS, "TIME_ROLLBACK_HOURS");
}




std::string ShinyHuntAutonomousFishing::Stats::stats() const{
    std::string str;
    str += str_encounters();
    str += " - Misses: " + tostr_u_commas(m_misses);
    str += " - Timeouts: " + tostr_u_commas(m_timeouts);
    str += " - Unexpected Battles: " + tostr_u_commas(m_unexpected_battles);
    str += str_shinies();
    return str;
}

void ShinyHuntAutonomousFishing::program(SingleSwitchProgramEnvironment& env) const{
    grip_menu_connect_go_home();
    resume_game_no_interact(TOLERATE_SYSTEM_UPDATE_MENU_FAST);

    const uint32_t PERIOD = (uint32_t)TIME_ROLLBACK_HOURS * 3600 * TICKS_PER_SECOND;
    uint32_t last_touch = system_clock();

    Stats stats;
    StandardEncounterTracker tracker(stats, env.console, false, EXIT_BATTLE_MASH_TIME);

    while (true){
        stats.log_stats(env, env.logger);

        //  Touch the date.
        if (TIME_ROLLBACK_HOURS > 0 && system_clock() - last_touch >= PERIOD){
            pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
            rollback_hours_from_home(TIME_ROLLBACK_HOURS, SETTINGS_TO_HOME_DELAY);
            resume_game_no_interact(TOLERATE_SYSTEM_UPDATE_MENU_FAST);
            last_touch += PERIOD;
        }

        pbf_wait(FISH_RESPAWN_TIME);
        env.console.botbase().wait_for_all_requests();

        //  Trigger encounter.
        {
            FishingDetector detector(env.console);
            pbf_press_button(BUTTON_A, 10, 10);
            pbf_mash_button(BUTTON_B, TICKS_PER_SECOND);
            env.console.botbase().wait_for_all_requests();
            FishingDetector::Detection detection = detector.wait_for_detection(env, env.logger);
            switch (detection){
            case FishingDetector::NO_DETECTION:
                stats.m_timeouts++;
                pbf_mash_button(BUTTON_B, 2 * TICKS_PER_SECOND);
                continue;
            case FishingDetector::HOOKED:
                pbf_press_button(BUTTON_A, 10, 0);
                break;
            case FishingDetector::MISSED:
                stats.m_misses++;
                pbf_mash_button(BUTTON_B, 2 * TICKS_PER_SECOND);
                continue;
            case FishingDetector::BATTLE_MENU:
                stats.m_unexpected_battles++;
                tracker.run_away();
                continue;
            }
            env.wait(std::chrono::seconds(3));
            detection = detector.detect_now();
            if (detection == FishingDetector::MISSED){
                stats.m_misses++;
                pbf_mash_button(BUTTON_B, 2 * TICKS_PER_SECOND);
                continue;
            }
        }

        //  Detect shiny.
        ShinyEncounterDetector::Detection detection;
        {
            ShinyEncounterDetector detector(
                env.console, env.logger,
                ShinyEncounterDetector::REGULAR_BATTLE,
                std::chrono::seconds(30)
            );
            detection = detector.detect(env);
        }

        if (tracker.process_result(detection)){
            break;
        }
        if (detection == ShinyEncounterDetector::NO_BATTLE_MENU){
            stats.m_timeouts++;
            pbf_mash_button(BUTTON_B, TICKS_PER_SECOND);
            tracker.run_away();
        }
    }

    stats.log_stats(env, env.logger);

    pbf_press_button(BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);

    end_program_callback();
    end_program_loop();
}



}
}
}

