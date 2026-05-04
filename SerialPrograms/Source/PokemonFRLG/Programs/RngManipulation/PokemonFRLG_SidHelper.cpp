/*  SID Helper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <vector>
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Pokemon_AdvRng.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_TrainerCardDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_TrainerIdReader.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG/Programs/PokemonFRLG_StartMenuNavigation.h"
#include "PokemonFRLG_SidHelper.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

SidHelper_Descriptor::SidHelper_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:SidHelper",
        Pokemon::STRING_POKEMON + " FRLG", "SID Helper",
        "Programs/PokemonFRLG/SidHelper.html",
        "Hit a specific RNG advance when starting a new game and determine the corresponding SID.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct SidHelper_Descriptor::Stats : public StatsTracker {
public:
    Stats()
        : errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& errors;
};

std::unique_ptr<StatsTracker> SidHelper_Descriptor::make_stats() const{
//    return std::unique_ptr<StatsTracker>(new Stats());
    return nullptr;
}

SidHelper::SidHelper()
    : LANGUAGE(
        "<b>Game Language:</b><br>"
        "Language affects the number of advances (based on the number of text characters) that pass after the last button press.",
        {
            Language::English,
            Language::Japanese,
            Language::Spanish,
            Language::French,
            Language::German,
            Language::Italian,
        },
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , TARGET_ADVANCES(
        "<b>Target Advances:</b><br>"
        "The target advances for finalizing the SID. This is arbitrary unless you're attempting to hit a specific TID/SID combination.<br>"
        "This value should always be odd.",
        LockMode::LOCK_WHILE_RUNNING, 
        2301, 2275 // default, min
    )
    , NUM_CANDIDATES(
        "<b># Candidate SIDs:</b><br>"
        "The number of SIDs to list near the target.",
        LockMode::LOCK_WHILE_RUNNING, 
        5, 1, 11 // default, min, max
    )
    , GO_HOME_WHEN_DONE(false)
    , NOTIFICATION_SIDS("SID Candidates", true, true)
    , NOTIFICATIONS({
        &NOTIFICATION_SIDS,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(SIDS_DISPLAY);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(TARGET_ADVANCES);
    PA_ADD_OPTION(NUM_CANDIDATES);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

namespace{

void set_sid_from_name_screen(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    const uint64_t& SID_DELAY
){
    // this is performed blind to try to maximize consistency of timing
    // ensure the OK button is selected
    pbf_press_button(context, BUTTON_PLUS, 200ms, 300ms);
    // confirm name (SID delay starts now)
    pbf_press_button(context, BUTTON_A, 200ms, 1800ms);
    // advance to "...about to unfold!", picking the default rival name
    pbf_press_button(context, BUTTON_A, 200ms, 2300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1050ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1550ms);
    pbf_move_left_joystick(context, {0,-1}, 200ms, 300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1050ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1550ms);
    pbf_press_button(context, BUTTON_A, 200ms, 2300ms);

    // 2000 + 2500 + 1250 + 1750 + 500 + 1250 + 1750 + 2500 + 200 ms
    Milliseconds delay(SID_DELAY > 13700
        ? SID_DELAY - 13700
        : 0
    );
    pbf_press_button(context, BUTTON_A, 200ms, delay);

    // finish dialogue and lock in SID
    pbf_press_button(context, BUTTON_A, 200ms, 0ms);
    context.wait_for_all_requests();
}

void finish_intro_animations(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    BlackScreenWatcher black_screen;
    context.wait_for_all_requests();
    int ret = wait_until(
        env.console, context, 10s,
        { black_screen }
    );

    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT, "SidHelper(): black screen not detected within 10 seconds of setting SID", env.console
        );
    }
    
    BlackScreenOverWatcher black_screen_over;
    context.wait_for_all_requests();
    int ret2 = wait_until(
        env.console, context, 10s,
        { black_screen_over }
    );

    if (ret2 < 0){
        env.log("finish_intro_animations(): end of black screen not detected within 10 seconds. Continuing anyway...");
        // if there's a problem, an error will be thrown when trying to navigate to the trainer card
    }

    pbf_wait(context, 2000ms);
    context.wait_for_all_requests();
}

void navigate_to_trainer_card(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    int errors = 0;
    while (true){
        if (errors >= 5){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT, "SidHelper(): failed 5 times to navigate to the Trainer Card", env.console
            ); 
        }

        open_start_menu(env.console, context);

        // the positions are different before recieving a Pokemon and Pokedex
        bool cursor = move_cursor_to_position(
            env.console, context, SelectionArrowPositionStartMenu(1)
        );
        if (!cursor){
            errors++;
            pbf_mash_button(context, BUTTON_B, 2000ms);
            continue;
        }

        TrainerCardWatcher trainer_card(COLOR_RED);
        context.wait_for_all_requests();
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                pbf_press_button(context, BUTTON_A, 200ms, 2300ms);
                pbf_press_button(context, BUTTON_A, 200ms, 2300ms);
            },
            { trainer_card }
        );

        if (ret < 0){
            env.log("SidHelper(): failed to detect Trainer Card");
            errors++;
            pbf_mash_button(context, BUTTON_B, 2000ms);
            continue;
        }

        break;
    }
}

uint16_t read_tid(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    TrainerIdReader reader;
    VideoOverlaySet overlays(env.console.overlay());
    reader.make_overlays(overlays);

    VideoSnapshot screen = env.console.video().snapshot();
    env.log("Trainer Card detected.");
    env.log("Reading TID...");
    uint16_t tid = reader.read_tid(env.logger(), screen);
    env.log("TID: " + std::to_string(tid));

    context.wait_for_all_requests();

    return tid;
}

std::vector<std::pair<std::string, std::string>> get_sid_messages(
    SingleSwitchProgramEnvironment& env,
    ProControllerContext& context,
    uint16_t tid,
    SimpleIntegerOption<uint32_t>& TARGET_ADVANCES,
    SimpleIntegerOption<uint8_t>& NUM_CANDIDATES
){
    std::vector<std::pair<std::string, std::string>> messages;

    int start = -1 * NUM_CANDIDATES / 2;
    int end = start + NUM_CANDIDATES;

    Pokemon::AdvRngSearcher searcher(tid, TARGET_ADVANCES + 2*start);

    for (int i = start; i < end; i++){
        std::pair<std::string, std::string> m;
        uint16_t sid = searcher.state.s0 >> 16;

        m.first = std::to_string(searcher.state.advance) + " Advances";
        m.second = std::to_string(sid);
        messages.push_back(m);

        env.log(m.first + ": " + m.second);

        searcher.advance_state();
        searcher.advance_state(); // 2 by 2
    }

    return messages;
}

} // namespace

void SidHelper::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    if (TARGET_ADVANCES % 2 == 0){
        OperationFailedException::fire(
            ErrorReport::NO_ERROR_REPORT, "SidHelper(): the Target Advances setting needs to be odd", env.console
        ); 
    }

    home_black_border_check(env.console, context);

    // SidHelper_Descriptor::Stats& stats = env.current_stats<SidHelper_Descriptor::Stats>();

    double FRAMERATE = 59.999977; // FPS
    double FRAME_DURATION = 1000 / FRAMERATE; // ms

    uint64_t FINAL_TEXT_FRAMES;
    Language lang = LANGUAGE;
    switch (lang){
    case Language::English:
        FINAL_TEXT_FRAMES = 249;
        break;
    case Language::Japanese:
        FINAL_TEXT_FRAMES = 194;
        break;
    case Language::Italian:
        FINAL_TEXT_FRAMES = 236;
        break;
    case Language::French:
        FINAL_TEXT_FRAMES = 205;
        break;
    case Language::German:
        FINAL_TEXT_FRAMES = 208;
        break;
    case Language::Spanish:
        FINAL_TEXT_FRAMES = 202;
        break;
    default:
        FINAL_TEXT_FRAMES = 249;
    }

    const double& FIXED_ADVANCES_OFFSET = 7; // determined empirically. Probably not console/setup dependent

    const uint64_t SID_DELAY = uint64_t((TARGET_ADVANCES - 2*FINAL_TEXT_FRAMES + FIXED_ADVANCES_OFFSET) * FRAME_DURATION / 2); // advances pass 2 by 2
    env.log("Delay: " + std::to_string(SID_DELAY) + "ms");

    set_sid_from_name_screen(env, context, SID_DELAY);
    finish_intro_animations(env, context);
    navigate_to_trainer_card(env, context);

    uint16_t tid = read_tid(env, context);

    std::vector<std::pair<std::string, std::string>> sid_messages = get_sid_messages(
        env, context, tid, TARGET_ADVANCES, NUM_CANDIDATES
    );

    SIDS_DISPLAY.set(tid, sid_messages);

    send_program_notification(env, NOTIFICATION_SIDS, COLOR_BLUE, "Possible SIDs:", sid_messages, "");

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}


}
}
}
