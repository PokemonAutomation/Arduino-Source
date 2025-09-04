/*  Berry Farmer 2
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "Common/Cpp/Time.h"
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleMenuDetector.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh/Inference/Sounds/PokemonSwSh_BerryTreeRustlingSoundDetector.h"
#include "PokemonSwSh/Programs/PokemonSwSh_EncounterHandler.h"
#include "PokemonSwSh_DateSpam-BerryFarmer2.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;


BerryFarmer2_Descriptor::BerryFarmer2_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:BerryFarmer2",
        STRING_POKEMON + " SwSh", "Date Spam - Berry Farmer 2",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/DateSpam-BerryFarmer2.md",
        "Farm berries using Feedback.",
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

class BerryFarmer2_Descriptor::Stats : public ShinyHuntTracker{
public:
    Stats()
        : ShinyHuntTracker(true)
        , days(m_stats["Days"])
        , shakes(m_stats["Shakes"])
    {
        m_display_order.insert(m_display_order.begin() + 0, Stat("Days"));
        m_display_order.insert(m_display_order.begin() + 1, Stat("Shakes"));
    }

public:
    std::atomic<uint64_t>& days;
    std::atomic<uint64_t>& shakes;
};
std::unique_ptr<StatsTracker> BerryFarmer2_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



BerryFarmer2::BerryFarmer2()
    : REQUIRES_AUDIO(
        html_color_text(
            "<font size=4><b>Rustling detection uses sound. Make sure you have the correct audio input set.</b></font>",
            COLOR_BLUE
        )
    )
    , FETCH_ATTEMPTS(
        "<b>Number of Fetch Attempts:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        100000
    )
    , SAVE_ITERATIONS0(
        "<b>Save Every this Many Fetches:</b><br>(zero disables saving): ",
        LockMode::UNLOCK_WHILE_RUNNING,
        100
    )
    , ENCOUNTER_BOT_OPTIONS(false, true)
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
    , EXIT_BATTLE_TIMEOUT0(
        "<b>Exit Battle Timeout:</b><br>After running, wait this long to return to overworld.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "10 s"
    )
//    , START_BATTLE_TIMEOUT(
//        "<b>Start Battle Timeout:</b><br>After a battle is detected, wait this long to flee in seconds.",
//        LockMode::LOCK_WHILE_RUNNING,
//        15
//    )
    , RUSTLING_INTERVAL(
        "<b>Rustling Interval:</b><br>How much time between two rustling sounds has to pass to be considered slow rustling in ms.",
        LockMode::UNLOCK_WHILE_RUNNING,
        1350
    )
    , RUSTLING_TIMEOUT0(
        "<b>Rustling Timeout:</b><br>Wait this many ticks to detect rustling.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "3200 ms"
    )
    , SECONDARY_ATTEMPT_MASH_TIME0(
        "<b>Secondary attempt mash time:</b><br>Mash ZL this many ticks for secondary fetch attempts.",
        LockMode::UNLOCK_WHILE_RUNNING,
        "1920 ms"
    )
    , SOUND_THRESHOLD(
        "<b>Maximum Sound Error Coefficient",
        LockMode::UNLOCK_WHILE_RUNNING,
        0.75, 0, 1.0
    )
{
    PA_ADD_OPTION(REQUIRES_AUDIO);
    PA_ADD_OPTION(START_LOCATION);
    PA_ADD_OPTION(FETCH_ATTEMPTS);
    PA_ADD_OPTION(SAVE_ITERATIONS0);

    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(ENCOUNTER_BOT_OPTIONS);
    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(EXIT_BATTLE_TIMEOUT0);
//    PA_ADD_OPTION(START_BATTLE_TIMEOUT);
    PA_ADD_OPTION(RUSTLING_INTERVAL);
    PA_ADD_OPTION(RUSTLING_TIMEOUT0);
    PA_ADD_OPTION(SECONDARY_ATTEMPT_MASH_TIME0);
    PA_ADD_OPTION(SOUND_THRESHOLD);
}


BerryFarmer2::Rustling BerryFarmer2::check_rustling(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    BerryFarmer2_Descriptor::Stats& stats = env.current_stats<BerryFarmer2_Descriptor::Stats>();

    // wait some time in order to not detect rustling from previous fetch attempt
    pbf_wait(context, 80);
    context.wait_for_all_requests();

    BerryTreeRustlingSoundDetector initial_rustling_detector(
        env.console, [&](float error_coefficient) -> bool {
            //  Warning: This callback will be run from a different thread than this function.
            return true;
        },
        (float)SOUND_THRESHOLD
    );

    BerryTreeRustlingSoundDetector secondary_rustling_detector(
        env.console, [&](float error_coefficient) -> bool {
            //  Warning: This callback will be run from a different thread than this function.
            return true;
        },
        (float)SOUND_THRESHOLD
    );

    StandardBattleMenuWatcher battle_menu_detector(false);
    StartBattleWatcher start_battle_detector;

    Rustling result = Rustling::No;
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [&](ProControllerContext& context){
            pbf_wait(context, RUSTLING_TIMEOUT0);
            context.wait_for_all_requests();
        },
        { {initial_rustling_detector}, {battle_menu_detector}, {start_battle_detector} }
    );
    switch (ret){
    case 0:{
        env.console.log("BerryFarmer: Initial Rustling detected.");
        WallClock initial_rustling_time = current_time();
        result = Rustling::Slow;

        int ret1 = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                pbf_wait(context, RUSTLING_TIMEOUT0);
                context.wait_for_all_requests();
            },
            { {secondary_rustling_detector} }
        );

        if (ret1 == 0){
            env.console.log("BerryFarmer: Secondary Rustling detected.");
            WallClock secondary_rustling_time = current_time();
            if (std::chrono::duration_cast<Milliseconds>(secondary_rustling_time - initial_rustling_time).count() <= RUSTLING_INTERVAL){
                result = Rustling::Fast;
            }
        }
        break;
    }
    case 1:
        env.log("Unexpected battle menu.", COLOR_RED);
        stats.add_error();
        env.update_stats();
        pbf_mash_button(context, BUTTON_B, TICKS_PER_SECOND);
        run_away(env.console, context, EXIT_BATTLE_TIMEOUT0);
        result = Rustling::Battle;
        break;
    case 2:{
        env.console.log("BerryFarmer: Battle Start detected.");
//        wait_until(env.console, context, std::chrono::seconds(START_BATTLE_TIMEOUT), { battle_menu_detector });

        //  Detect shiny.
        ShinyDetectionResult encounter_result = detect_shiny_battle(
            env.console, context,
            SHINY_BATTLE_REGULAR,
            std::chrono::seconds(30)
        );

        StandardEncounterHandler handler(
            env, env.console, context,
            LANGUAGE,
            ENCOUNTER_BOT_OPTIONS,
            stats
        );

        bool stop = handler.handle_standard_encounter_end_battle(encounter_result, EXIT_BATTLE_TIMEOUT0);
        if (stop){
            throw ProgramFinishedException();
        }

        result = Rustling::Battle;
        break;
    }
    default:
        result = Rustling::No;
    }
        
    context.wait_for_all_requests();
    return result;
}

uint16_t BerryFarmer2::do_secondary_attempts(SingleSwitchProgramEnvironment& env, ProControllerContext& context, Rustling rustling){
    BerryFarmer2_Descriptor::Stats& stats = env.current_stats<BerryFarmer2_Descriptor::Stats>();

    uint8_t no_rustling = (rustling == Rustling::No) ? 1 : 0;
    Rustling current_rustling = rustling;
    uint16_t attempts = 0;

    while ((current_rustling == Rustling::Slow || current_rustling == Rustling::No) && no_rustling < 3) { 
        /* Slow or No rustling, not in Battle! */
        pbf_mash_button(context, BUTTON_ZL, 240);
        pbf_mash_button(context, BUTTON_B, 10);
        attempts++;
        stats.shakes++;

        current_rustling = check_rustling(env, context);

        if (current_rustling == Rustling::No){
            no_rustling++;
        }
    }
    /* Fast rustling, in Battle or too many times No rustling */
    if (no_rustling >= 3){
        return attempts;
    }
    if (current_rustling == Rustling::Fast){
        // this is the last tree interaction for this time skip
        pbf_mash_button(context, BUTTON_ZL, SECONDARY_ATTEMPT_MASH_TIME0);
        pbf_mash_button(context, BUTTON_B, 10);
        attempts++;
        stats.shakes++;
        current_rustling = check_rustling(env, context);
    }
    if (current_rustling == Rustling::Battle){
        pbf_mash_button(context, BUTTON_B, TICKS_PER_SECOND);
        env.console.log("Clearing dialog boxes...");
//        run_away(env.console, context, EXIT_BATTLE_TIMEOUT);
//        context.wait_for_all_requests();
    }
    return attempts;
}

void BerryFarmer2::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
        ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_FAST0, 160ms);
    }

    BerryFarmer2_Descriptor::Stats& stats = env.current_stats<BerryFarmer2_Descriptor::Stats>();

    uint8_t year = MAX_YEAR;
    uint16_t save_count = 0;
    uint32_t c = 0;
    while (c < FETCH_ATTEMPTS){
        env.update_stats();
        uint16_t iteration_attempts = 1;
        env.log("Fetch Attempts: " + tostr_u_commas(c));
        
        home_roll_date_enter_game_autorollback(env.console, context, year);
        stats.days++;
        // Interact with the tree
        pbf_mash_button(context, BUTTON_ZL, 375);
        pbf_mash_button(context, BUTTON_B, 10);
        stats.shakes++;

        // Rustling after the first fetch attempt
        Rustling current_rustling = check_rustling(env, context);
        
        switch (current_rustling){
        case Rustling::Battle:
            pbf_mash_button(context, BUTTON_B, 1 * TICKS_PER_SECOND);
            run_away(env.console, context, EXIT_BATTLE_TIMEOUT0);
            break;
        case Rustling::Fast:
            // Do nothing -> stop current tree session
            break;
        case Rustling::No:
        case Rustling::Slow:
            iteration_attempts += do_secondary_attempts(env, context, current_rustling);
            break;
        }

        // end tree session
        pbf_mash_button(context, BUTTON_B, iteration_attempts > 1 ? 800 : 600);

        c += iteration_attempts;

        uint16_t save_iterations = SAVE_ITERATIONS0;
        if (save_iterations != 0){
            save_count += iteration_attempts;
            if (save_count >= save_iterations){
                save_count = 0;
                pbf_mash_button(context, BUTTON_B, 2 * TICKS_PER_SECOND);
                pbf_press_button(context, BUTTON_X, 160ms, GameSettings::instance().OVERWORLD_TO_MENU_DELAY0);
                pbf_press_button(context, BUTTON_R, 20, 2 * TICKS_PER_SECOND);
                pbf_press_button(context, BUTTON_ZL, 20, 3 * TICKS_PER_SECOND);
            }
        }

        // Tap HOME and quickly spam B. The B spamming ensures that we don't
        // accidentally update the system if the system update window pops up.
        ssf_press_button(context, BUTTON_HOME, 120ms, 160ms);
        pbf_mash_button(context, BUTTON_B, GameSettings::instance().GAME_TO_HOME_DELAY_FAST0.get() - 120ms);
    }
    VideoSnapshot screen = env.console.video().snapshot();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH, "", screen);
}
}



}
}

