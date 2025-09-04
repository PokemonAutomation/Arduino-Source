/*  Cram-o-matic RNG Manipulation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Credit goes to Anubis for discovering how the Cram-o-matic works
 *  and for the original code to calculate how many advances are needed
 *  to get the wanted balls.
 *
 */

#include <algorithm>
#include <set>
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/DebugDumper.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Inference/Pokemon_PokeballNameReader.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSwSh/PokemonSwSh_Settings.h"
#include "PokemonSwSh/Commands/PokemonSwSh_Commands_DateSpam.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "PokemonSwSh/Programs/PokemonSwSh_GameEntry.h"
#include "PokemonSwSh/Programs/RNG/PokemonSwSh_BasicRNG.h"
#include "PokemonSwSh/Programs/RNG/PokemonSwSh_CramomaticRNG.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
using namespace Pokemon;

CramomaticRNG_Descriptor::CramomaticRNG_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSwSh:CramomaticRNG",
        STRING_POKEMON + " SwSh", "Cram-o-matic RNG",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/CramomaticRNG.md",
        "Perform RNG manipulation to get rare balls from the Cram-o-matic.",
        ProgramControllerClass::StandardController_PerformanceClassSensitive,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

class CramomaticRNG_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : iterations(m_stats["Iterations"])
        , reads(m_stats["Seed Reads"])
        , errors(m_stats["Errors"])
        , total_balls(m_stats["Balls"])
        , apri_balls(m_stats["Apriballs"])
        , sport_safari_balls(m_stats["Safari/Sport Balls"])
        , bonus(m_stats["Bonus"])
    {
        m_display_order.emplace_back("Iterations");
        m_display_order.emplace_back("Seed Reads");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Balls");
        m_display_order.emplace_back("Apriballs");
        m_display_order.emplace_back("Safari/Sport Balls");
        m_display_order.emplace_back("Bonus", HIDDEN_IF_ZERO);
    }

public:
    std::atomic<uint64_t>& iterations;
    std::atomic<uint64_t>& reads;
    std::atomic<uint64_t>& errors;
    std::atomic<uint64_t>& total_balls;
    std::atomic<uint64_t>& apri_balls;
    std::atomic<uint64_t>& sport_safari_balls;
    std::atomic<uint64_t>& bonus;
};
std::unique_ptr<StatsTracker> CramomaticRNG_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

CramomaticRNG::CramomaticRNG()
    : LANGUAGE(
        "<b>Game Language:</b><br>Required to read the ball received.",
        PokemonNameReader::instance().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , NUM_APRICORN_ONE(
        "<b>Primary Apricorns:</b><br>Number of Apricorns in the selected slot.",
        LockMode::LOCK_WHILE_RUNNING,
        0
    )
    , NUM_APRICORN_TWO(
        "<b>Secondary Apricorns:</b><br>Number of Apricorns in the slot below the selected one.",
        LockMode::LOCK_WHILE_RUNNING,
        0
    )
    , NUM_NPCS(
        "<b>NPCs:</b><br>Number of NPCs in the dojo, including " + STRING_POKEMON + ".",
        LockMode::LOCK_WHILE_RUNNING,
        21
    )
    , BALL_TABLE(
        "<b>Wanted Balls:</b><br>Exact kind depends on the currently selected apricorn."
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , MAX_PRIORITY_ADVANCES(
        "<b>Priority Advances:</b><br>How many advances to check when checking for higher priority selections.",
        LockMode::LOCK_WHILE_RUNNING,
        300
    )
    , MAX_UNKNOWN_ADVANCES(
        "<b>Max Unknown advances:</b><br>How many advances to check when updating the rng state.",
        LockMode::LOCK_WHILE_RUNNING,
        300
    )
    , ADVANCE_PRESS_DURATION(
        "<b>Advance Press Duration:</b><br>"
        "Hold the button down for this long to advance once.<br>"
        "<font color=\"red\">For tick-imprecise controllers, this number will be increased automatically.</font>",
        LockMode::LOCK_WHILE_RUNNING,
        "80 ms"
    )
    , ADVANCE_RELEASE_DURATION(
        "<b>Advance Release Duration:</b><br>"
        "After releasing the button, wait this long before pressing it again.<br>"
        "<font color=\"red\">For tick-imprecise controllers, this number will be increased automatically.</font>",
        LockMode::LOCK_WHILE_RUNNING,
        "80 ms"
    )
    , SAVE_SCREENSHOTS(
        "<b>Save Debug Screenshots:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , LOG_VALUES(
        "<b>Log Animation Values:</br>",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
{
    PA_ADD_OPTION(START_LOCATION);

    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(NUM_APRICORN_ONE);
    PA_ADD_OPTION(NUM_APRICORN_TWO);
    PA_ADD_OPTION(NUM_NPCS);
    PA_ADD_OPTION(BALL_TABLE);

    PA_ADD_OPTION(TOUCH_DATE_INTERVAL);

    PA_ADD_OPTION(NOTIFICATIONS);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(MAX_PRIORITY_ADVANCES);
    PA_ADD_OPTION(MAX_UNKNOWN_ADVANCES);
    PA_ADD_OPTION(ADVANCE_PRESS_DURATION);
    PA_ADD_OPTION(ADVANCE_RELEASE_DURATION);
    PA_ADD_OPTION(SAVE_SCREENSHOTS);
    PA_ADD_OPTION(LOG_VALUES);
}

void CramomaticRNG::navigate_to_party(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    pbf_press_button(context, BUTTON_X, 10, 125);
    pbf_press_button(context, BUTTON_A, 20, 10);
    pbf_wait(context, 2000ms);
}

CramomaticTarget CramomaticRNG::calculate_target(SingleSwitchProgramEnvironment& env, Xoroshiro128PlusState state, std::vector<CramomaticSelection> selected_balls){
    Xoroshiro128Plus rng(state);
    size_t advances = 0;
    uint16_t priority_advances = 0;
    std::vector<CramomaticTarget> possible_targets;

    std::sort(selected_balls.begin(), selected_balls.end(), [](CramomaticSelection sel1, CramomaticSelection sel2) { return sel1.priority > sel2.priority; });
    // priority_advances only starts counting up after the first good result is found
    while (priority_advances <= MAX_PRIORITY_ADVANCES){
        // calculate the result for the current temp_rng state
        Xoroshiro128PlusState temp_state = predict_state_after_menu_close(rng.get_state(), NUM_NPCS);
        Xoroshiro128Plus temp_rng(temp_state);


        /*uint64_t item_roll =*/ temp_rng.nextInt(4);
        uint64_t ball_roll = temp_rng.nextInt(100);
        bool is_safari_sport = temp_rng.nextInt(1000) == 0;
        bool is_bonus = false;

        if (is_safari_sport || ball_roll == 99){
            is_bonus = temp_rng.nextInt(1000) == 0;
        }else{
            is_bonus = temp_rng.nextInt(100) == 0;
        }

        CramomaticBallType type;
        if (is_safari_sport){
            type = CramomaticBallType::Safari;
        }else if (ball_roll < 25){
            type = CramomaticBallType::Poke;
        }else if (ball_roll < 50){
            type = CramomaticBallType::Great;
        }else if (ball_roll < 75){
            type = CramomaticBallType::Shop1;
        }else if (ball_roll < 99){
            type = CramomaticBallType::Shop2;
        }else{
            type = CramomaticBallType::Apricorn;
        }
        

        // check whether the result is a good result
        for (size_t i = 0; i < selected_balls.size(); i++){
            CramomaticSelection selection = selected_balls[i];
            if (!selection.is_bonus || is_bonus){
                if (is_safari_sport){
                    if (selection.ball_type == CramomaticBallType::Safari || selection.ball_type == CramomaticBallType::Sport){
                        type = selection.ball_type;
                    }
                }
                
                if (selection.ball_type == type){
                    CramomaticTarget target;
                    target.ball_type = type;
                    target.is_bonus = is_bonus;
                    target.needed_advances = advances;
                    possible_targets.emplace_back(target);

                    priority_advances = 0;

                    uint16_t priority = selection.priority;
                    selected_balls.erase(
                        std::remove_if(selected_balls.begin(), selected_balls.end()
                            , [priority](CramomaticSelection sel) { return sel.priority <= priority; })
                        , selected_balls.end());
                    break;
                }

            }
        }
        if (possible_targets.size() > 0){
            if (selected_balls.empty()){
                //priority_advances = MAX_PRIORITY_ADVANCES + 1;
                break;
            }
            priority_advances++;
        }

        rng.next();
        advances++;
    }

    // Choose the first result which doesn't overshadow a higher priority choice.
    // Ignores rng advances done by NPCs when the menu closes
    while (possible_targets.size() > 1){
        auto last_target = possible_targets.end() - 1;
        auto second_to_last_target = possible_targets.end() - 2;

        if ((*last_target).needed_advances - (*second_to_last_target).needed_advances > MAX_PRIORITY_ADVANCES){
            possible_targets.erase(last_target);
        }else{
            possible_targets.erase(second_to_last_target);
        }
    }

    return possible_targets[0];
}

void CramomaticRNG::leave_to_overworld_and_interact(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    pbf_press_button(context, BUTTON_B, 2000ms, 40ms);
    pbf_press_button(context, BUTTON_B, 10, 70);

    pbf_mash_button(context, BUTTON_A, 320);
    pbf_wait(context, 125);
}

void CramomaticRNG::choose_apricorn(SingleSwitchProgramEnvironment& env, ProControllerContext& context, bool sport){
    // check whether the bag is open
    context.wait_for_all_requests();
    VideoOverlaySet boxes(env.console);
    SelectionArrowFinder bag_arrow_detector(env.console, ImageFloatBox(0.465, 0.195, 0.054, 0.57));
    bag_arrow_detector.make_overlays(boxes);

    int ret = wait_until(env.console, context, Milliseconds(5000), { bag_arrow_detector });
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Could not detect bag.",
            env.console
        );
    }

    // select the apricorn(s)
    pbf_wait(context, 1000ms);
    pbf_press_button(context, BUTTON_A, 10, 30);
    if (sport){
        pbf_press_dpad(context, DPAD_DOWN, 20, 10);
    }
    pbf_press_button(context, BUTTON_A, 10, 30);
    pbf_press_button(context, BUTTON_A, 10, 30);
    if (sport){
        pbf_press_dpad(context, DPAD_UP, 20, 10);
    }
    pbf_press_button(context, BUTTON_A, 10, 30);

    pbf_mash_button(context, BUTTON_A, 5000ms);
}

std::pair<bool, std::string> CramomaticRNG::receive_ball(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // receive ball and refuse to use the cram-o-matic again
    VideoOverlaySet boxes(env.console);

    SelectionArrowFinder arrow_detector(env.console, ImageFloatBox(0.350, 0.450, 0.500, 0.400));
    arrow_detector.make_overlays(boxes);

    OverlayBoxScope dialog_text(env.console, {0.21, 0.80, 0.53, 0.17});
    OverlayBoxScope dialog_box(env.console, {0.70, 0.90, 0.03, 0.05});
    const double LOG10P_THRESHOLD = -1.5;
    std::string best_ball;

    size_t presses = 0;
    bool arrow_detected = false;

    while (presses < 30 && !arrow_detected){
        presses++;
        pbf_press_button(context, BUTTON_B, 10, 165);
        context.wait_for_all_requests();

        VideoSnapshot screen = env.console.video().snapshot();
        if (SAVE_SCREENSHOTS){
            dump_debug_image(env.logger(), "cramomatic-rng", "receive", screen);
        }

        //  If we detect a dialog box, attempt to read the ball that was received.
        ImageStats stats = image_stats(extract_box_reference(screen, dialog_box));
//        cout << stats.average << stats.stddev << endl;
        if (is_grey(stats, 0, 200, 5)){
            OCR::StringMatchResult result = PokeballNameReader::instance().read_substring(
                env.console, LANGUAGE,
                extract_box_reference(screen, dialog_text),
                {{0xff007f7f, 0xffc0ffff}}
            );
            result.clear_beyond_log10p(LOG10P_THRESHOLD);
            if (best_ball.empty() && !result.results.empty()){
                auto iter = result.results.begin();
                if (iter->first < LOG10P_THRESHOLD){
                    best_ball = iter->second.token;
                }
            }
        }

        if (arrow_detector.detect(screen)){
            arrow_detected = true;
        }
    }
    pbf_press_button(context, BUTTON_B, 80ms, 1000ms);
    return {arrow_detected, best_ball};
}

void CramomaticRNG::recover_from_wrong_state(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // Mash the B button to exit potential menus or dialog boxes
    pbf_mash_button(context, BUTTON_B, 30s);

    // take a step in case Hyde repositioned the player
    pbf_move_left_joystick(context, 128, 0, 1000ms, 80ms);

    context.wait_for_all_requests();
}


void CramomaticRNG::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    CramomaticRNG_Descriptor::Stats& stats = env.current_stats<CramomaticRNG_Descriptor::Stats>();
    env.update_stats();

    std::vector<CramomaticSelection> selections = BALL_TABLE.selected_balls();
    if (selections.empty()){
        throw UserSetupError(env.console, "At least one type of ball needs to be selected!");
    }


    if (START_LOCATION.start_in_grip_menu()){
        grip_menu_connect_go_home(context);
        resume_game_back_out(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST, 200);
    }else{
        pbf_press_button(context, BUTTON_B, 5, 5);
    }

    static const std::set<std::string> APRIBALLS{
        "fast-ball",
        "friend-ball",
        "lure-ball",
        "level-ball",
        "heavy-ball",
        "love-ball",
        "moon-ball",
    };
    static const std::set<std::string> RARE_BALLS{
        "sport-ball",
        "safari-ball",
    };

    Xoroshiro128Plus rng(0, 0);
    bool is_state_valid = false;
    uint32_t num_apricorn_one = NUM_APRICORN_ONE;
    uint32_t num_apricorn_two = NUM_APRICORN_TWO;

    // if there is no Sport Ball in the selected balls we ignore num_apricorn_two
    bool sport_wanted = false;
    for (CramomaticSelection selection : selections){
        if (selection.ball_type == CramomaticBallType::Sport){
            sport_wanted = true;
            break;
        }
    }

    size_t iteration = 0;
    uint16_t state_errors = 0;
    uint16_t apricorn_selection_errors = 0;
    while (num_apricorn_one > 4 && (!sport_wanted || num_apricorn_two > 2)){
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        //  Touch the date.
        if (TOUCH_DATE_INTERVAL.ok_to_touch_now()){
            env.log("Touching date to prevent rollover.");
            ssf_press_button(context, BUTTON_HOME, GameSettings::instance().GAME_TO_HOME_DELAY_SAFE0, 160ms);
            touch_date_from_home(env.console, context, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);
            resume_game_no_interact(env.console, context, ConsoleSettings::instance().TOLERATE_SYSTEM_UPDATE_MENU_FAST);
        }

        env.console.log("Cram-o-matic RNG iteration: " + std::to_string(iteration));
        navigate_to_party(env, context);
        context.wait_for_all_requests();

        if (!is_state_valid){
            rng = Xoroshiro128Plus(find_rng_state(env.console, context, SAVE_SCREENSHOTS, LOG_VALUES));
//            rng = Xoroshiro128Plus(100, 10000);
            is_state_valid = true;
            stats.reads++;
        }else{
            rng = Xoroshiro128Plus(refind_rng_state(env.console, context, rng.get_state(), 0, MAX_UNKNOWN_ADVANCES, SAVE_SCREENSHOTS, LOG_VALUES));
            stats.reads++;
        }
        env.update_stats();

        Xoroshiro128PlusState rng_state = rng.get_state();
        if (rng_state.s0 == 0 && rng_state.s1 == 0){
            stats.errors++;
            env.update_stats();

            state_errors++;
            if (state_errors >= 3){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Detected invalid RNG state three times in a row.",
                    env.console
                );
            }
            VideoSnapshot screen = env.console.video().snapshot();
            send_program_recoverable_error_notification(env, NOTIFICATION_ERROR_RECOVERABLE, "Detected invalid RNG state.", screen);
            recover_from_wrong_state(env, context);
            is_state_valid = false;
            continue;
        }

        CramomaticTarget target = calculate_target(env, rng.get_state(), BALL_TABLE.selected_balls());
        bool sport = target.ball_type == CramomaticBallType::Sport;
        env.console.log("Needed advances: " + std::to_string(target.needed_advances));
        num_apricorn_one -= sport ? 2 : 4;
        num_apricorn_two -= sport ? 2 : 0;

        do_rng_advances(
            env.console, context,
            rng,
            target.needed_advances,
            ADVANCE_PRESS_DURATION,
            ADVANCE_RELEASE_DURATION
        );
        leave_to_overworld_and_interact(env, context);

        try{
            choose_apricorn(env, context, sport);
        }catch (OperationFailedException& e){
            stats.errors++;
            env.update_stats();

            apricorn_selection_errors++;
            if (apricorn_selection_errors >= 3){
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Could not detect the bag three times on a row.",
                env.console
                );
            }
            send_program_recoverable_error_notification(
                env,
                NOTIFICATION_ERROR_RECOVERABLE,
                e.message(),
                e.screenshot_view()
            );
            is_state_valid = false;
            recover_from_wrong_state(env, context);
            continue;
        }

        std::pair<bool, std::string> result = receive_ball(env, context);

        //  Update ball stats.
        if (!result.second.empty()){
            stats.total_balls++;
        }
        auto iter = APRIBALLS.find(result.second);
        if (iter != APRIBALLS.end()){
            stats.apri_balls++;
        }
        iter = RARE_BALLS.find(result.second);
        if (iter != RARE_BALLS.end()){
            stats.sport_safari_balls++;
        }

        //  Out of apricorns.
        if (!result.first || num_apricorn_one <= 4 || (sport_wanted && num_apricorn_two <= 2)){
            throw_and_log<ProgramFinishedException>(env.console, "Out of apricorns.", env.console);
        }


#if 0
//        cout << "Ball Slug = " << ball << endl;
        if (result.first || num_apricorn_one <= 4 || (sport_wanted && num_apricorn_two <= 2)){
        }else{
            is_state_valid = false;
            stats.errors++;
        }
#endif

#if 0
        if (!ball.empty() || num_apricorn_one <= 4 || (sport_wanted && num_apricorn_two <= 2)){
            int amount = target.is_bonus ? 5 : 1;
            if (target.is_bonus){
                stats.bonus++;
            }
            if (target.ball_type == CramomaticBallType::Apricorn){
                stats.apri_balls += amount;
            }else if (target.ball_type == CramomaticBallType::Sport || target.ball_type == CramomaticBallType::Safari){
                stats.sport_safari_balls += amount;
            }
            stats.total_balls += amount;
        }else{
            is_state_valid = false;
            stats.errors++;
        }
#endif
        env.update_stats();

        state_errors = 0;
        apricorn_selection_errors = 0;

        iteration++;
        stats.iterations++;
        env.update_stats();
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}


}
}
}
