/*  E Shiny Deoxys
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "Pokemon/Pokemon_Strings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonRSE/Inference/Dialogs/PokemonRSE_DialogDetector.h"
#include "PokemonRSE/PokemonRSE_Navigation.h"
#include "PokemonRSE_ShinyHunt-Deoxys.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

ShinyHuntDeoxys_Descriptor::ShinyHuntDeoxys_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonRSE:ShinyHuntDeoxys",
        Pokemon::STRING_POKEMON + " RSE", "Shiny Hunt - Deoxys",
        "Programs/PokemonRSE/ShinyHuntDeoxys.html",
        "Use the Run Away method to shiny hunt Deoxys in Emerald.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct ShinyHuntDeoxys_Descriptor::Stats : public StatsTracker{
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
std::unique_ptr<StatsTracker> ShinyHuntDeoxys_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

ShinyHuntDeoxys::ShinyHuntDeoxys()
    : STARTPOS(
        "<b>Starting Position:</b><br>",
        {
            {StartPos::boat, "boat", "Boat/Walk up"},
            {StartPos::rock_unsolved, "rock_unsolved", "Triangle rock, puzzle is unsolved"},
            {StartPos::rock_solved, "rock_solved", "Triangle rock, puzzle is already solved"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        StartPos::rock_unsolved
    )
    , TAKE_VIDEO("<b>Take Video:</b><br>Record a video when the shiny starter is found.", LockMode::UNLOCK_WHILE_RUNNING, true)
    , GO_HOME_WHEN_DONE(true)
    , WALK_UP_DOWN_TIME0(
        "<b>Walk up/down time</b><br>Spend this long to run up to the triangle rock.",
        LockMode::LOCK_WHILE_RUNNING,
        "3520 ms"
    )
    , NOTIFICATION_SHINY(
        "Shiny Found",
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
    PA_ADD_STATIC(SHINY_REQUIRES_AUDIO);
    PA_ADD_OPTION(STARTPOS);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(WALK_UP_DOWN_TIME0);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void ShinyHuntDeoxys::solve_puzzle(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    env.log("Step 1: Press A from below.");
    pbf_press_button(context, BUTTON_A, 160ms, 320ms);

    env.log("Step 2: 5 Left, 1 Down.");
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 400ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 400ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 400ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 400ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 400ms);

    pbf_press_dpad(context, DPAD_DOWN, 80ms, 640ms);

    pbf_press_button(context, BUTTON_A, 160ms, 320ms);
    context.wait_for_all_requests();

    env.log("Step 3: 5 Right, 5 Up.");
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 400ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 400ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 400ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 400ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 400ms);

    ssf_press_button(context, BUTTON_B, 0ms, 720ms);
    pbf_press_dpad(context, DPAD_UP, 720ms, 400ms);

    pbf_press_button(context, BUTTON_A, 160ms, 320ms);
    context.wait_for_all_requests();

    env.log("Step 4: 5 Right, 5 Down");
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 400ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 400ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 400ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 400ms);
    pbf_press_dpad(context, DPAD_RIGHT, 80ms, 400ms);

    ssf_press_button(context, BUTTON_B, 0ms, 720ms);
    pbf_press_dpad(context, DPAD_DOWN, 720ms, 400ms);

    pbf_press_button(context, BUTTON_A, 160ms, 320ms);
    context.wait_for_all_requests();

    env.log("Step 5: 3 Up, 7 Left");
    pbf_press_dpad(context, DPAD_UP, 80ms, 400ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 400ms);
    pbf_press_dpad(context, DPAD_UP, 80ms, 400ms);

    ssf_press_button(context, BUTTON_B, 0ms, 920ms);
    pbf_press_dpad(context, DPAD_LEFT, 920ms, 400ms);

    pbf_press_button(context, BUTTON_A, 160ms, 320ms);
    context.wait_for_all_requests();

    env.log("Step 6: 5 Right.");
    ssf_press_button(context, BUTTON_B, 0ms, 800ms);
    pbf_press_dpad(context, DPAD_RIGHT, 800ms, 400ms);

    pbf_press_button(context, BUTTON_A, 160ms, 320ms);
    context.wait_for_all_requests();

    env.log("Step 7: 3 Left, 2 Down.");
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 400ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 400ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 400ms);

    ssf_press_button(context, BUTTON_B, 0ms, 480ms);
    pbf_press_dpad(context, DPAD_DOWN, 480ms, 400ms);

    pbf_press_button(context, BUTTON_A, 160ms, 320ms);
    context.wait_for_all_requests();

    env.log("Step 8: 1 Down, 4 Left.");
    pbf_press_dpad(context, DPAD_DOWN, 80ms, 400ms);

    ssf_press_button(context, BUTTON_B, 0ms, 640ms);
    pbf_press_dpad(context, DPAD_LEFT, 640ms, 400ms);

    pbf_press_button(context, BUTTON_A, 160ms, 320ms);
    context.wait_for_all_requests();

    env.log("Step 8: 7 Right.");
    ssf_press_button(context, BUTTON_B, 0ms, 920ms);
    pbf_press_dpad(context, DPAD_RIGHT, 920ms, 400ms);

    pbf_press_button(context, BUTTON_A, 160ms, 320ms);
    context.wait_for_all_requests();

    env.log("Step 9: 4 Left, Down 1.");
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 400ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 400ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 400ms);
    pbf_press_dpad(context, DPAD_LEFT, 80ms, 400ms);

    pbf_press_dpad(context, DPAD_DOWN, 80ms, 400ms);

    pbf_press_button(context, BUTTON_A, 160ms, 320ms);
    context.wait_for_all_requests();

    env.log("Step 10: 4 Up.");
    ssf_press_button(context, BUTTON_B, 0ms, 640ms);
    pbf_press_dpad(context, DPAD_UP, 640ms, 640ms);
    context.wait_for_all_requests();
}

void ShinyHuntDeoxys::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    ShinyHuntDeoxys_Descriptor::Stats& stats = env.current_stats<ShinyHuntDeoxys_Descriptor::Stats>();

    home_black_border_check(env.console, context);

    /*
    * Settings: Text Speed fast. Turn off animations.
    * Full screen, no filter.
    * If on a retro handheld, make sure the screen matches that of NSO+.
    * 
    * Setup: Lead is faster or has a Smoke Ball.
    * No abilities or items that activate on entry.
    * Lead cannot be shiny.
    * Stand enter Birth Island and stay at the door. Save the game.
    * 
    * Emerald only. This uses the Run Away method due to the game's RNG issues.
    * If powering off your game/resetting, try to start with different timing to avoid repeated frames.
    * 
    * Do not have to handle random PokeNav calls for deoxys/mew.
    * Would have to handle both PokeNav and random encounters for ray/others? Why is PokeNav a thing???
    */

    bool first_run = true;

    while (true){
        if (first_run){
            switch (STARTPOS){
            case StartPos::rock_solved:
                env.log("StartPos: Already in position.");
                break;
            case StartPos::boat:
                env.log("StartPos: Walking up to Deoxys.");
                //Walk up to the triangle rock from the ship. No bike allowed.
                ssf_press_button(context, BUTTON_B, 0ms, WALK_UP_DOWN_TIME0);
                pbf_press_dpad(context, DPAD_UP, WALK_UP_DOWN_TIME0, 160ms);
                context.wait_for_all_requests();
            case StartPos::rock_unsolved:
                env.log("StartPos: Solving puzzle.");
                solve_puzzle(env, context);
                break;
            default:
                stats.errors++;
                env.update_stats();
                OperationFailedException::fire(
                    ErrorReport::SEND_ERROR_REPORT,
                    "Invalid starting position selected.",
                    env.console
                );
                break;
            }
            first_run = false;
        }

        //Start battle
        BlackScreenWatcher legendary_battle_start(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
        int ret3 = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                for (int i = 0; i < 5; i++){
                    pbf_mash_button(context, BUTTON_A, 3000ms);
                    pbf_wait(context, 10000ms);
                    context.wait_for_all_requests();
                }
            },
            {legendary_battle_start}
        );
        context.wait_for_all_requests();
        if (ret3 != 0){
            env.log("Failed to start battle after 5 attempts.", COLOR_RED);
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to start battle after 5 attempts.",
                env.console
            );
        }else{
            env.log("Legendary battle started.");
        }
        context.wait_for_all_requests();

        bool legendary_shiny = handle_encounter(env.console, context, true);
        if (legendary_shiny){
            stats.shinies++;
            env.update_stats();

            if (TAKE_VIDEO){
                pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 0ms);
            }

            send_program_notification(env,
                NOTIFICATION_SHINY,
                COLOR_YELLOW,
                "Shiny found!",
                {}, "",
                env.console.video().snapshot(),
                true
            );
            break;
        }
        env.log("No shiny found.");
        flee_battle(env.console, context);

        //After pressing the flee button, additional dialog box pops up for Deoxys
        pbf_mash_button(context, BUTTON_B, 2000ms);
        context.wait_for_all_requests();
        
        //Walk down from the triangle rock to the ship.
        env.log("Walking down to ship.");
        ssf_press_button(context, BUTTON_B, 0ms, WALK_UP_DOWN_TIME0);
        pbf_press_dpad(context, DPAD_DOWN, WALK_UP_DOWN_TIME0, 160ms);
        context.wait_for_all_requests();

        env.log("Walking up to Deoxys rock.");
        //Walk up to the triangle rock from the ship. Bike is not allowed on Birth Island.
        ssf_press_button(context, BUTTON_B, 0ms, WALK_UP_DOWN_TIME0);
        pbf_press_dpad(context, DPAD_UP, WALK_UP_DOWN_TIME0, 160ms);
        context.wait_for_all_requests();

        env.log("Solving puzzle.");
        solve_puzzle(env, context);

        stats.resets++;
        env.update_stats();
    }

    if (GO_HOME_WHEN_DONE){
        pbf_press_button(context, BUTTON_HOME, 200ms, 1000ms);
    }
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}
