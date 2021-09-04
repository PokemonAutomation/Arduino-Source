/*  Shiny Hunt Autonomous - Fishing
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
#include "PokemonSwSh/ShinyHuntTracker.h"
#include "PokemonSwSh/Inference/PokemonSwSh_FishingDetector.h"
#include "PokemonSwSh/Inference/ShinyDetection/PokemonSwSh_ShinyEncounterDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_EncounterHandler.h"
#include "PokemonSwSh_ShinyHuntAutonomous-Fishing.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyHuntAutonomousFishing_Descriptor::ShinyHuntAutonomousFishing_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonSwSh:ShinyHuntAutonomousFishing",
        "Shiny Hunt Autonomous - Fishing",
        "SwSh-Arduino/wiki/Advanced:-ShinyHuntAutonomous-Fishing",
        "Automatically hunt for shiny fishing " + STRING_POKEMON + " using video feedback.",
        FeedbackType::REQUIRED,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}



ShinyHuntAutonomousFishing::ShinyHuntAutonomousFishing(const ShinyHuntAutonomousFishing_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , GO_HOME_WHEN_DONE(false)
    , ENCOUNTER_BOT_OPTIONS(true, true)
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , EXIT_BATTLE_TIMEOUT(
        "<b>Exit Battle Timeout:</b><br>After running, wait this long to return to overworld and for the fish to reappear.",
        "10 * TICKS_PER_SECOND"
    )
    , FISH_RESPAWN_TIME(
        "<b>Fish Respawn Time:</b><br>Wait this long for fish to respawn.",
        "5 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(START_IN_GRIP_MENU);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(TIME_ROLLBACK_HOURS);

    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);

    PA_ADD_OPTION(m_advanced_options);
    PA_ADD_OPTION(EXIT_BATTLE_TIMEOUT);
    PA_ADD_OPTION(FISH_RESPAWN_TIME);
}



struct ShinyHuntAutonomousFishing::Stats : public ShinyHuntTracker{
    Stats()
        : ShinyHuntTracker(true)
        , m_misses(m_stats["Misses"])
    {
        m_display_order.insert(m_display_order.begin() + 1, Stat("Misses"));
        m_aliases["Timeouts"] = "Errors";
        m_aliases["Unexpected Battles"] = "Errors";
    }
    uint64_t& m_misses;
};
std::unique_ptr<StatsTracker> ShinyHuntAutonomousFishing::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



void ShinyHuntAutonomousFishing::program(SingleSwitchProgramEnvironment& env){
    if (START_IN_GRIP_MENU){
        grip_menu_connect_go_home(env.console);
        resume_game_no_interact(env.console, TOLERATE_SYSTEM_UPDATE_MENU_FAST);
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

        pbf_wait(env.console, FISH_RESPAWN_TIME);
        env.console.botbase().wait_for_all_requests();

        //  Trigger encounter.
        {
            FishingDetector detector(env.console);
            pbf_press_button(env.console, BUTTON_A, 10, 10);
            pbf_mash_button(env.console, BUTTON_B, TICKS_PER_SECOND);
            env.console.botbase().wait_for_all_requests();
            FishingDetector::Detection detection = detector.wait_for_detection(env, env.console);
            switch (detection){
            case FishingDetector::NO_DETECTION:
                stats.add_error();
                env.update_stats();
                pbf_mash_button(env.console, BUTTON_B, 2 * TICKS_PER_SECOND);
                continue;
            case FishingDetector::HOOKED:
                pbf_press_button(env.console, BUTTON_A, 10, 0);
                break;
            case FishingDetector::MISSED:
                stats.m_misses++;
                pbf_mash_button(env.console, BUTTON_B, 2 * TICKS_PER_SECOND);
                continue;
            case FishingDetector::BATTLE_MENU:
                stats.add_error();
                env.update_stats();
                run_away(env, env.console, EXIT_BATTLE_TIMEOUT);
                continue;
            }
            env.wait(std::chrono::seconds(3));
            detection = detector.detect_now(env.console.video().snapshot());
            if (detection == FishingDetector::MISSED){
                stats.m_misses++;
                pbf_mash_button(env.console, BUTTON_B, 2 * TICKS_PER_SECOND);
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
    }

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(env.console, BUTTON_HOME, 10, GAME_TO_HOME_DELAY_SAFE);
    }

    end_program_callback(env.console);
    end_program_loop(env.console);
}



}
}
}

