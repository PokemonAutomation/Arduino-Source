/*  RNG Manipulator
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonTools/Random.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Pokemon/Pokemon_Strings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonFRLG/Inference/PokemonFRLG_SelectionArrowDetector.h"
#include "PokemonFRLG/Programs/PokemonFRLG_StartMenuNavigation.h"
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
            // {Target::hitmon, "hitmon", "Hitmonlee / Hitmonchan"},
            {Target::eevee, "eevee", "Eevee"},
            // {Target::lapras, "lapras", "Lapras"},
            // {Target::fossils, "fossils", "Omanyte / Kabuto / Aerodactyl"},
            {Target::snorlax, "snorlax", "Snorlax"},
            {Target::sweetscent, "sweetscent", "Sweet Scent for wild encounters"},
            {Target::fishing, "fishing", "Fishing"},
            // {Target::static, "static", "Static overworld encounters (including Legendaries)"},
            // {Target::roaming, "roaming", "Roaming Legendaries"}
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
        "<b>Seed Delay Time (ms):</b><br>The delay between starting the game and advancing past the title screen. Set this to match your target seed.",
        LockMode::UNLOCK_WHILE_RUNNING,
        32000, 30000 // default, min
    )
    , SEED_CALIBRATION(
         "<b>Seed Calibration (ms):</b><br>Modifies the seed delay time.",
        LockMode::UNLOCK_WHILE_RUNNING,
        0  // default
    )
    , LOAD_ADVANCES(
        "<b>Load Screen Advances:</b><br>The number of frames to advance before loading the game.<br>These pass at the \"normal\" rate compared to other consoles.",
        LockMode::UNLOCK_WHILE_RUNNING,
        1000, 200 // default, min
    )
    , INGAME_ADVANCES(
        "<b>In-Game Advances:</b><br>The number of frames to advance before triggering the gift/encounter.<br>These pass at double the rate compared to other consoles, where every 2nd frame is skipped.",
        LockMode::UNLOCK_WHILE_RUNNING,
        1000, 700 // default, min
    )
    , ADVANCES_CALIBRATION(
        "<b>Advances Calibration:</b><br>Modifies the frame advances passed after loading the game.<br>",
        LockMode::UNLOCK_WHILE_RUNNING,
        0 // default
    )
    , USE_TEACHY_TV(
        "<b>Use Teachy TV:</b><br>Opens the Teachy TV to quickly advance in-game frames at 313x speed.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false // default
    )
    , TAKE_PICTURES(
        "<b>Take Pictures of Stats:</b><br>Take pictures of the first two pages of the summary screen.<br>Only applies to gifts. Useful for calibrating your seed and advances.", 
        LockMode::UNLOCK_WHILE_RUNNING, 
        false // default
    )
    , TAKE_VIDEO(
        "<b>Take Video:</b><br>Record a video when the shiny is found.", 
        LockMode::UNLOCK_WHILE_RUNNING, 
        true // default
    )
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
    PA_ADD_OPTION(SEED_CALIBRATION);
    PA_ADD_OPTION(LOAD_ADVANCES);
    PA_ADD_OPTION(INGAME_ADVANCES);
    PA_ADD_OPTION(ADVANCES_CALIBRATION);
    PA_ADD_OPTION(USE_TEACHY_TV);
    PA_ADD_OPTION(TAKE_PICTURES);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

namespace{

void hard_reset(ProControllerContext& context){
    // close the game
    pbf_press_button(context, BUTTON_HOME, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_Y, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 2800ms);
    // press A to select game
    pbf_press_button(context, BUTTON_A, 200ms, 2300ms);
    // press A to select profile and launch the game
    pbf_press_button(context, BUTTON_A, 200ms, 0ms);
}

void soft_reset(ProControllerContext& context){
    pbf_press_button(context, BUTTON_B | BUTTON_A | BUTTON_X | BUTTON_Y, 200ms, 0ms);
}

uint64_t wait_for_copyright_text(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // wait for copyright text to appear
    BlackScreenWatcher black_screen(COLOR_RED, ImageFloatBox {0.25, 0.25, 0.55, 0.55});
    context.wait_for_all_requests();
    int black_ret = wait_until(
        env.console, context, 10000ms,
        {black_screen},
        1ms
    );
    if (black_ret != 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Black screen not detected within 10 seconds of starting game.",
            env.console
        );
    }
    BlackScreenOverWatcher copyright_detected(COLOR_RED, ImageFloatBox {0.25, 0.25, 0.55, 0.55});
    context.wait_for_all_requests();
    WallClock start_time = current_time();
    int ret = wait_until(
        env.console, context, 10000ms,
        {copyright_detected },
        1ms
    );
    if (ret != 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Black screen detected for more than 10 seconds after starting game.",
            env.console
        );
    }
    auto elapsed = current_time() - start_time;
    return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
}

void set_seed_after_delay(ProControllerContext& context, SimpleIntegerOption<uint64_t>& SEED_DELAY,  SimpleIntegerOption<int64_t>& SEED_CALIBRATION, int64_t& FIXED_SEED_OFFSET){
    // wait on title screen for the specified delay
    pbf_wait(context, std::chrono::milliseconds(SEED_DELAY + SEED_CALIBRATION + FIXED_SEED_OFFSET));
    // hold A for a few seconds through the transition to the load screen
    pbf_press_button(context, BUTTON_A, 3000ms, 0ms);
}

void load_game_after_delay(ProControllerContext& context, uint64_t& LOAD_DELAY){
    pbf_wait(context, std::chrono::milliseconds(LOAD_DELAY - 3000));
    pbf_press_button(context, BUTTON_A, 33ms, 1467ms);
    // skip recap
    pbf_press_button(context, BUTTON_B, 33ms, 2467ms);
    // need to later subtract 4000ms from delay to hit desired number of advances
}

void wait_with_teachy_tv(ProControllerContext& context, uint64_t& TEACHY_DELAY){
    // open start menu -> bag -> key items -> Teachy TV -> use
    pbf_press_button(context, BUTTON_PLUS, 200ms, 300ms);
    pbf_move_left_joystick(context, {0, -1}, 200ms, 300ms);
    pbf_move_left_joystick(context, {0, -1}, 200ms, 300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 2300ms);
    pbf_move_left_joystick(context, {+1, 0}, 200ms, 2300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 300ms);
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(TEACHY_DELAY));
    // close teachy tv -> close bag -> reset start menu cursor position - > close start menu
    pbf_press_button(context, BUTTON_B, 200ms, 2300ms);
    pbf_press_button(context, BUTTON_B, 200ms, 2300ms);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    pbf_press_button(context, BUTTON_B, 200ms, 300ms);
    // total non-teachy delay duration: 13700ms
}

int watch_for_shiny_encounter(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    BlackScreenWatcher battle_entered(COLOR_RED);
    context.wait_for_all_requests();
    env.log("Wild encounter started.");
    int ret = wait_until(
        env.console, context, 10000ms,
        {battle_entered}
    );
    if (ret != 0){
        // OperationFailedException::fire(
        //     ErrorReport::SEND_ERROR_REPORT,
        //     "Failed to initiate encounter.",
        //     env.console
        // );
        return -1;
    }
    bool encounter_shiny = handle_encounter(env.console, context, false);
    return encounter_shiny ? 1 : 0;
}

void collect_starter_after_delay(SingleSwitchProgramEnvironment& env, ProControllerContext& context, uint64_t& DOUBLE_DELAY){
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
    context.wait_for_all_requests();
    env.log("Starter collected.");
}

void collect_magikarp_after_delay(SingleSwitchProgramEnvironment& env, ProControllerContext& context, uint64_t& DOUBLE_DELAY){
    // Advance through starter dialogue and wait on YES/NO
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(DOUBLE_DELAY - 7200)); // 4000ms + 3000ms + 200ms
    // Finish dialogue (hits the target advance)
    pbf_press_button(context, BUTTON_A, 200ms, 3800ms);
    // Decline nickname
    pbf_press_button(context, BUTTON_B, 200ms, 1300ms);
    context.wait_for_all_requests();
    env.log("Magikarp collected.");
}

void collect_eevee_after_delay(SingleSwitchProgramEnvironment& env, ProControllerContext& context, uint64_t& DOUBLE_DELAY){
    // No dialogue to advance through -- just wait
    pbf_wait(context, std::chrono::milliseconds(DOUBLE_DELAY - 4000));
    // Interact with the pokeball
    pbf_press_button(context, BUTTON_A, 200ms, 3800ms);
    // Decline nickname
    pbf_press_button(context, BUTTON_B, 200ms, 1300ms);
    context.wait_for_all_requests();
    env.log("Eevee collected.");
}

void encounter_snorlax_after_delay(SingleSwitchProgramEnvironment& env, ProControllerContext& context, uint64_t& DOUBLE_DELAY){
    // Interact with Snorlax, YES to PokeFlute, wait on "woke up!"
    pbf_press_button(context, BUTTON_A, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 9800ms); // PokeFlute tune
    pbf_press_button(context, BUTTON_A, 200ms, std::chrono::milliseconds(DOUBLE_DELAY - 15700)); // 4000ms + 1500ms + 10000ms + 200ms
    pbf_press_button(context, BUTTON_A, 200ms, 200ms); 
    context.wait_for_all_requests();
    env.log("Snorlax encounter started.");
}

void open_party_menu_from_overworld(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    pbf_press_button(context, BUTTON_PLUS, 200ms, 800ms);
    int ret = move_cursor_to_position(env.console, context, SelectionArrowPositionStartMenu::POKEMON);
    if (ret < 0){
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to navigate to POKEMON on the Start menu.",
            env.console
        ); 
    }
    pbf_press_button(context, BUTTON_A, 200ms, 1800ms);
    context.wait_for_all_requests();
}

void go_to_starter_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // Navigate to summary (1st party slot)
    open_party_menu_from_overworld(env, context);
    pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
    pbf_press_button(context, BUTTON_A, 200ms, 2300ms);
}

void go_to_sixth_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // navigate to summary (last party slot)
    open_party_menu_from_overworld(env, context);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    // open summary
    pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
    pbf_press_button(context, BUTTON_A, 200ms, 2300ms);
}

void use_sweet_scent(SingleSwitchProgramEnvironment& env, ProControllerContext& context, uint64_t& DOUBLE_DELAY){
    // navigate to last party slot
    pbf_press_button(context, BUTTON_PLUS, 200ms, 300ms);
    pbf_move_left_joystick(context, {0, -1}, 200ms, 800ms);
    pbf_press_button(context, BUTTON_A, 200ms, 1000ms);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    pbf_move_left_joystick(context, {0, +1}, 200ms, 300ms);
    pbf_press_button(context, BUTTON_A, 200ms, 300ms);
    // hover over Sweet Scent (2nd option, but maybe HMs could change this)
    pbf_move_left_joystick(context, {0, -1}, 200ms, std::chrono::milliseconds(DOUBLE_DELAY - 8400));
    pbf_press_button(context, BUTTON_A, 200ms, 800ms);
    context.wait_for_all_requests();
    env.log("Sweet Scent used.");
}

void use_registered_fishing_rod(SingleSwitchProgramEnvironment& env, ProControllerContext& context, uint64_t& DOUBLE_DELAY){
    uint32_t rng_wait = 50 * random_u32(0, 20); // helps avoid always hitting "Not even a nibble" (?)
    pbf_wait(context, std::chrono::milliseconds(rng_wait));
    pbf_press_button(context, BUTTON_MINUS, 200ms, std::chrono::milliseconds(DOUBLE_DELAY - rng_wait - 4200));
    pbf_press_button(context, BUTTON_A, 200ms, 800ms);
    context.wait_for_all_requests();
}

void take_summary_pictures(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    // Capture both summary screens
    pbf_wait(context, 2000ms);
    pbf_press_button(context, BUTTON_CAPTURE, 200ms, 2300ms);
    pbf_move_left_joystick(context, {+1, 0}, 200ms, 1300ms);
    pbf_press_button(context, BUTTON_CAPTURE, 200ms, 2300ms);
    context.wait_for_all_requests();
    env.log("Pictures take of Summary pages 1 and 2.");
}


} // namespace


void RNGManipulator::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    /*
    * Settings: Text Speed fast
    */

    RNGManipulator_Descriptor::Stats& stats = env.current_stats<RNGManipulator_Descriptor::Stats>();

    bool shiny_found = false;

    double FRAMERATE = 60.0;        // FPS. tested on Switch 1

    int64_t FIXED_SEED_OFFSET = -2020;    // milliseconds. tested on Switch 1 against ten-lines seeds
    int64_t FIXED_ADVANCES_OFFSET = 162;  // frames. test on Switch 1

    uint64_t LOAD_DELAY;
    uint64_t DOUBLE_DELAY;
    uint64_t TEACHY_DELAY;

    VideoSnapshot screen;

    while (!shiny_found){
        uint64_t MODIFIED_INGAME_ADVANCES = INGAME_ADVANCES + FIXED_ADVANCES_OFFSET + ADVANCES_CALIBRATION;
        uint64_t TEACHY_ADVANCES = 0;

        bool should_use_teachy_tv = USE_TEACHY_TV && MODIFIED_INGAME_ADVANCES > 5000;
        if (should_use_teachy_tv) {
            TEACHY_ADVANCES = uint64_t((int)std::floor((MODIFIED_INGAME_ADVANCES - 5000) / 313) * 313);
        }

        LOAD_DELAY = uint64_t((LOAD_ADVANCES)/ FRAMERATE * 1000);
        TEACHY_DELAY = uint64_t(TEACHY_ADVANCES / FRAMERATE * 1000 / 313);
        DOUBLE_DELAY = uint64_t((MODIFIED_INGAME_ADVANCES - TEACHY_ADVANCES) / FRAMERATE * 500) - (should_use_teachy_tv ? 13700 : 0);
        env.log("Load screen delay: " + std::to_string(LOAD_DELAY) + "ms");
        env.log("In-game delay: " + std::to_string(DOUBLE_DELAY) + "ms");
        env.log("Teachy TV delay: " + std::to_string(TEACHY_DELAY) + "ms");
        env.log("Total time: " + std::to_string(SEED_DELAY + SEED_CALIBRATION + FIXED_SEED_OFFSET + LOAD_DELAY + DOUBLE_DELAY + TEACHY_DELAY) + "ms");


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

        uint64_t STARTUP_DELAY = wait_for_copyright_text(env, context);
        env.log("Startup delay: " + std::to_string(STARTUP_DELAY) + "ms");

        set_seed_after_delay(context, SEED_DELAY, SEED_CALIBRATION, FIXED_SEED_OFFSET);
        load_game_after_delay(context, LOAD_DELAY);
        if (should_use_teachy_tv){
            wait_with_teachy_tv(context, TEACHY_DELAY);
        }

        if (TARGET == Target::starters){
            collect_starter_after_delay(env, context, DOUBLE_DELAY);
            go_to_starter_summary(env, context);
            if (TAKE_PICTURES){
                take_summary_pictures(env, context);
            }
            context.wait_for_all_requests();
            screen = env.console.video().snapshot();
            ShinySymbolDetector shiny_checker(COLOR_YELLOW);
            shiny_found = shiny_checker.read(env.console.logger(), screen);
        }else if (TARGET == Target::magikarp){
            collect_magikarp_after_delay(env, context, DOUBLE_DELAY);
            go_to_sixth_summary(env, context);
            if (TAKE_PICTURES){
                take_summary_pictures(env, context);
            }
            context.wait_for_all_requests();
            screen = env.console.video().snapshot();
            ShinySymbolDetector shiny_checker(COLOR_YELLOW);
            shiny_found = shiny_checker.read(env.console.logger(), screen);
        }else if (TARGET == Target::eevee) {
            collect_eevee_after_delay(env, context, DOUBLE_DELAY);
            go_to_sixth_summary(env, context);
            if (TAKE_PICTURES){
                take_summary_pictures(env, context);
            }
            context.wait_for_all_requests();
            screen = env.console.video().snapshot();
            ShinySymbolDetector shiny_checker(COLOR_YELLOW);
            shiny_found = shiny_checker.read(env.console.logger(), screen);
        }else if (TARGET == Target::snorlax){
            encounter_snorlax_after_delay(env, context, DOUBLE_DELAY);
            shiny_found = watch_for_shiny_encounter(env, context) == 1;
            context.wait_for_all_requests();
        }else if (TARGET == Target::sweetscent){
            use_sweet_scent(env, context, DOUBLE_DELAY);
            shiny_found = watch_for_shiny_encounter(env, context) == 1;
            context.wait_for_all_requests();
        }else if (TARGET == Target::fishing){
            use_registered_fishing_rod(env, context, DOUBLE_DELAY);
            int ret = watch_for_shiny_encounter(env, context);
            if (ret < 0 ){
                continue;
                // keep going if "Not even a nibble..."
            }
            shiny_found = ret == 1;
            context.wait_for_all_requests();
        }else{
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Option not yet implemented.",
                env.console
            );
        }

        stats.resets++;
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
            if (TAKE_VIDEO){
                pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 0ms);
            }
            break;
        }else if (stats.resets >= NUM_RESETS){
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

