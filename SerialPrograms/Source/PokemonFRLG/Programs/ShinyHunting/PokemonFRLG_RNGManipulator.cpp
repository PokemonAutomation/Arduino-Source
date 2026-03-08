/*  RNG Manipulator
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Pokemon/Pokemon_Strings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG/Inference/Menus/PokemonFRLG_StartMenuDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_ShinySymbolDetector.h"
#include "PokemonFRLG/PokemonFRLG_Navigation.h"
#include "PokemonFRLG_RNGManipulator.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

RNGManipulator_Descriptor::RNGManipulator_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonFRLG:RNGManipulator",
        Pokemon::STRING_POKEMON + " FRLG", "RNG Manipulator",
        "Programs/PokemonFRLG/RNGManipulator.html",
        "Soft reset with specific timings for hitting a target Seed and Frame for RNG manipulation.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct RNGManipulator_Descriptor::Stats : public StatsTracker{
    Stats()
        : resets(m_stats["Resets"])
        , shinies(m_stats["Shinies"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shinies");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> RNGManipulator_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

RNGManipulator::RNGManipulator()
    : TARGET(
        "<b>Target:</b><br>",
        {
            {Target::starters, "starters", "Bulbasaur / Squirtle / Charmander"},
            {Target::magikarp, "magikarp", "Magikarp"},
            {Target::hitmon, "hitmon", "Hitmonlee / Hitmonchan"},
            {Target::eevee, "eevee", "Eevee"},
            {Target::lapras, "lapras", "Lapras"},
            {Target::fossils, "fossils", "Omanyte / Kabuto / Aerodactyl"},
            {Target::sweetscent, "sweetscent", "Sweet Scent for wild encounters"},
            {Target::wildwalk, "wildwalk", "Hit seed and advance when walking in grass"},
            {Target::fishing, "fishing", "Hit seed and advance when fishing"}
        },
        LockMode::LOCK_WHILE_RUNNING,
        Target::starters
    )    
    , NUM_RESETS(
        "<b>Max Resets:</b><br>",
        LockMode::UNLOCK_WHILE_RUNNING,
        100, 0 // default, min
    )
    , RESET_TYPE(
        "<b>Reset Type:</b><br>\"Hard\" resets return to the home menu and close the game.<br>\"Soft\" resets don't require returning to the home menu, but may be less consistent.",
        {
            {ResetType::hard, "hard", "Hard Reset"},
            {ResetType::soft, "soft", "Soft Reset"}
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        ResetType::hard
    )
    , SEED_DELAY(
        "<b>Title Screen Delay Time (ms):</b><br>The delay between starting the game and advancing past the title screen.",
        LockMode::UNLOCK_WHILE_RUNNING,
        35000, 30000 // default, min
    )
    , LOAD_ADVANCES(
        "<b>Load Screen Advances:</b><br>The number of frames to advance before loading the game.<br>These pass at the \"normal\" rate compared to other consoles.",
        LockMode::UNLOCK_WHILE_RUNNING,
        1000, 200 // default, min
    )
    , DOUBLE_ADVANCES(
        "<b>In-Game Advances:</b><br>The number of frames to advance before finalizing the gift.<br>These pass at double the rate compared to other consoles, where every 2nd frame is skipped.",
        LockMode::UNLOCK_WHILE_RUNNING,
        1000, 900 // default, min
    )
    , TAKE_PICTURES("<b>Take Pictures of Stats:</b><br>Take pictures of the first two pages of the summary screen.<br>Only applies to gifts.", LockMode::UNLOCK_WHILE_RUNNING, true)
    , GO_HOME_WHEN_DONE(true)
    , NOTIFICATION_SHINY(
        "Shiny found",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_SHINY,
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
    })
{
    PA_ADD_OPTION(TARGET);
    PA_ADD_OPTION(NUM_RESETS);
    PA_ADD_OPTION(RESET_TYPE);
    PA_ADD_OPTION(SEED_DELAY);
    PA_ADD_OPTION(LOAD_ADVANCES);
    PA_ADD_OPTION(DOUBLE_ADVANCES);
    PA_ADD_OPTION(TAKE_PICTURES);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

namespace{

// reset_to_starter(SingleSwitchProgramEnvironment& env, ProControllerContext& context, EnumDropdownOption<ResetType>& RESET_TYPE, SimpleIntegerOption<uint64_t>& SEED_DELAY, SimpleIntegerOption<uint64_t>& LOAD_ADVANCES, SimpleIntegerOption<uint64_t>& DOUBLE_ADVANCES, BooleanCheckBoxOption& TAKE_PICTURES){
//     double FRAMERATE = 59.7275; // valid for GBA, but not sure for Switch
//     uint64_t LOAD_DELAY = uint64_t((LOAD_ADVANCES)/ FRAMERATE * 1000);
//     uint64_t DOUBLE_DELAY = uint64_t((DOUBLE_ADVANCES)/ FRAMERATE * 500);
//     env.log("Load screen delay: " + std::to_string(LOAD_DELAY));
//     env.log("In-game delay: " + std::to_string(DOUBLE_DELAY));
//     if (RESET_TYPE ==  ResetType::hard){
//         // close the game
//         pbf_press_button(context, BUTTON_HOME, 200ms, 1300ms);
//         pbf_press_button(context, BUTTON_Y, 200ms, 1300ms);
//         pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
//         // press A to select game
//         pbf_press_button(context, BUTTON_A, 200ms, 2300ms);
//         // press A to select profile and immediately go back to the home screen
//         pbf_press_button(context, BUTTON_A, 100ms, 100ms);
//         pbf_press_button(context, BUTTON_HOME, 200ms, 2800ms);
//         pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(SEED_DELAY - 200));
//     }else{
//         // perform soft reset and 
//         pbf_press_button(context, BUTTON_B | BUTTON_A | BUTTON_X | BUTTON_Y, 200ms, std::chrono::milliseconds(SEED_DELAY - 200));
//     }
//     // Advance to the load game screen and wait
//     pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(LOAD_DELAY - 200));
//     // Load the game (sets the Initial Seed)
//     pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
//     // Skip through the recap
//     pbf_press_button(context, BUTTON_B, 200ms, 2300ms);
//     // Advance through starter dialogue and wait on "really quite energetic!"
//     pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
//     pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
//     pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(DOUBLE_DELAY - 7200));
//     // Finish dialogue (hits the target advance)
//     pbf_press_button(context, BUTTON_A, 200ms, 5800ms);
//     // Decline nickname
//     pbf_press_button(context, BUTTON_B, 200ms, 2300ms);
//     // Advance through rival choiec
//     pbf_press_button(context, BUTTON_B, 200ms, 4800ms);
//     // Navigate to summary
//     pbf_press_button(context, BUTTON_PLUS, 200ms, 800ms);
//     pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
//     pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
//     pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
//     if (TAKE_PICTURES){
//         // Capture both summary screens
//         pbf_wait(context, 1000ms);
//         pbf_press_button(context, BUTTON_CAPTURE, 200ms, 1900ms);
//         pbf_move_left_joystick(context, {+1, 0}, 200ms, 1300ms);
//         pbf_press_button(context, BUTTON_CAPTURE, 200ms, 1800ms);
//     }else{
//         // view both summary screens
//         pbf_wait(context, 2000ms);
//         pbf_move_left_joystick(context, {+1, 0}, 200ms, 1300ms);
//         pbf_wait(context, 2500ms);
//     }

//     context.wait_for_all_requests();
//     env.log("Encounter reached.");
// }

void hard_reset(ProControllerContext& context){
    // close the game
    pbf_press_button(context, BUTTON_HOME, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_Y, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
    // press A to select game
    pbf_press_button(context, BUTTON_A, 200ms, 2300ms);
    // press A to select profile and immediately go back to the home screen
    pbf_press_button(context, BUTTON_A, 100ms, 100ms);
    pbf_press_button(context, BUTTON_HOME, 200ms, 2800ms);
    pbf_press_button(context, BUTTON_A, 200ms, 0ms);
}

void soft_reset(ProControllerContext& context){
    pbf_press_button(context, BUTTON_B | BUTTON_A | BUTTON_X | BUTTON_Y, 200ms, 0ms);
}

void set_seed_after_delay(ProControllerContext& context, SimpleIntegerOption<uint64_t>& SEED_DELAY){
    // wait on title screen for the specified delay
    pbf_wait(context, std::chrono::milliseconds(SEED_DELAY - 200));
    pbf_press_button(context, BUTTON_A, 200ms, 0ms);
}

void load_game_after_delay(ProControllerContext& context, uint64_t& LOAD_DELAY){
    pbf_wait(context, std::chrono::milliseconds(LOAD_DELAY - 200));
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    // skip recap
    pbf_press_button(context, BUTTON_B, 200ms, 2300ms);
    // need to later subtract 4000ms from delay to hit desired number of advances
}

void collect_starter_after_delay(ProControllerContext& context, uint64_t& DOUBLE_DELAY){
    // Advance through starter dialogue and wait on "really quite energetic!"
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(DOUBLE_DELAY - 7200)); // 4000ms + 3000ms + 200ms
    // Finish dialogue (hits the target advance)
    pbf_press_button(context, BUTTON_A, 200ms, 5800ms);
    // Decline nickname
    pbf_press_button(context, BUTTON_B, 200ms, 2300ms);
    // Advance through rival choice
    pbf_press_button(context, BUTTON_B, 200ms, 4800ms);
}

void collect_magikarp_after_delay(ProControllerContext& context, uint64_t& DOUBLE_DELAY){
    // Advance through starter dialogue and wait on YES/NO
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(DOUBLE_DELAY - 7200)); // 4000ms + 3000ms + 200ms
    // Finish dialogue (hits the target advance)
    pbf_press_button(context, BUTTON_A, 200ms, 3800ms);
    // Decline nickname
    pbf_press_button(context, BUTTON_B, 200ms, 1300ms);
}

void go_to_starter_summary(ProControllerContext& context){
    // Navigate to summary (1st party slot)
    pbf_press_button(context, BUTTON_PLUS, 200ms, 800ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
    pbf_press_button(context, BUTTON_A, 200ms, 2300ms);
}

void go_to_summary(ProControllerContext& context){
    // Navigate to summary (last party slot)
    pbf_press_button(context, BUTTON_PLUS, 200ms, 800ms);
    pbf_move_left_joystick(context, {0, -1}, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 1300ms);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
    pbf_press_button(context, BUTTON_A, 200ms, 2300ms);
}

void take_summary_pictures(ProControllerContext& context){
    // Capture both summary screens
    pbf_wait(context, 2000ms);
    pbf_press_button(context, BUTTON_CAPTURE, 200ms, 2300ms);
    pbf_move_left_joystick(context, {+1, 0}, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_CAPTURE, 200ms, 2300ms);
}

bool grass_walk_after_delay(SingleSwitchProgramEnvironment& env, ProControllerContext& context, uint64_t& DOUBLE_DELAY){
    pbf_wait(context, std::chrono::milliseconds(DOUBLE_DELAY - 4000)); // 4000ms from the load menu
    context.wait_for_all_requests();
    // "walk" without moving by tapping the joystick to change directions
    BlackScreenWatcher battle_entered(COLOR_RED);
    run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context) {
            while (true){
                // "walk" without moving by tapping the joystick to change directions
                // this is enough to trigger encounters
                pbf_move_left_joystick(context, {+1, 0}, 33ms, 150ms);
                pbf_move_left_joystick(context, {0, +1}, 33ms, 150ms);
                pbf_move_left_joystick(context, {-1, 0}, 33ms, 150ms);
                pbf_move_left_joystick(context, {0, -1}, 33ms, 150ms);
            }
        },
        { battle_entered }
    );
    bool encounter_shiny = handle_encounter(env.console, context, false);
    return encounter_shiny;
}

} // namespace


void RNGManipulator::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    /*
    * Settings: Text Speed fast
    */

    RNGManipulator_Descriptor::Stats& stats = env.current_stats<RNGManipulator_Descriptor::Stats>();

    bool shiny_found = false;
    uint64_t num_resets = 0;

    double FRAMERATE = 59.7275; // valid for GBA, but not sure for Switch
    uint64_t LOAD_DELAY;
    uint64_t DOUBLE_DELAY;

    VideoSnapshot screen;

    while (!shiny_found){
        LOAD_DELAY = uint64_t((LOAD_ADVANCES)/ FRAMERATE * 1000);
        DOUBLE_DELAY = uint64_t((DOUBLE_ADVANCES)/ FRAMERATE * 500);
        env.log("Load screen delay: " + std::to_string(LOAD_DELAY));
        env.log("In-game delay: " + std::to_string(DOUBLE_DELAY));
        if (RESET_TYPE == ResetType::hard){
            hard_reset(context);
        }else if (RESET_TYPE == ResetType::soft){
            soft_reset(context);
        }else{
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Invalid reset type",
                env.console
            );
        }

        set_seed_after_delay(context, SEED_DELAY);
        load_game_after_delay(context, LOAD_DELAY);

        if (TARGET == Target::starters){
            collect_starter_after_delay(context, DOUBLE_DELAY);
            go_to_starter_summary(context);
            if (TAKE_PICTURES){
                take_summary_pictures(context);
            }
            context.wait_for_all_requests();
            env.log("Starter collected.");

            screen = env.console.video().snapshot();

            ShinySymbolDetector shiny_checker(COLOR_YELLOW);
            shiny_found = shiny_checker.read(env.console.logger(), screen);
        }else if (TARGET == Target::magikarp){
            collect_magikarp_after_delay(context, DOUBLE_DELAY);
            go_to_summary(context);
            if (TAKE_PICTURES){
                take_summary_pictures(context);
            }
            context.wait_for_all_requests();
            env.log("Magikarp collected.");

            screen = env.console.video().snapshot();

            ShinySymbolDetector shiny_checker(COLOR_YELLOW);
            shiny_found = shiny_checker.read(env.console.logger(), screen);
        }else if (TARGET == Target::hitmon){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Hitmonchan/Hitmonlee hunt not implemented",
                env.console
            );
        }else if (TARGET == Target::eevee){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Eevee hunt not implemented",
                env.console
            );
        }else if (TARGET == Target::lapras){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Lapras hunt not implemented",
                env.console
            );
        }else if (TARGET == Target::fossils){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Fossil hunt not implemented",
                env.console
            );
        }else if (TARGET == Target::sweetscent){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Sweet Scent hunt not implemented",
                env.console
            );
        }else if (TARGET == Target::wildwalk){
            shiny_found = grass_walk_after_delay(env, context, DOUBLE_DELAY);
            context.wait_for_all_requests();
        }else if (TARGET == Target::fishing){
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Fishing hunt not implemented",
                env.console
            );
        }else{
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Invalid RNG option",
                env.console
            );
        }

        num_resets++;

        if (shiny_found){
            env.log("Shiny found!");
            stats.shinies++;
            send_program_notification(
                env,
                NOTIFICATION_SHINY,
                COLOR_YELLOW,
                "Shiny found!",
                {}, "",
                screen,
                true
            );
            break;
        }else if (num_resets >= NUM_RESETS){
            send_program_status_notification(
                env, NOTIFICATION_STATUS_UPDATE,
                "Maximum resets reached."
            );
            break;
        }else{
            env.log("Pokemon is not shiny.");
            env.log("Resetting.");
            send_program_status_notification(
                env, NOTIFICATION_STATUS_UPDATE,
                "Resetting."
            );
            stats.resets++;
            env.update_stats();
            context.wait_for_all_requests();
        }
    }

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}

