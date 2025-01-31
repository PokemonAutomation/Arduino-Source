/*  E Shiny Deoxys
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
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
        "ComputerControl/blob/master/Wiki/Programs/PokemonRSE/ShinyHuntDeoxys.md",
        "Use the Run Away method to shiny hunt Deoxys in Emerald.",
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {SerialPABotBase::OLD_NINTENDO_SWITCH_DEFAULT_REQUIREMENTS}
    )
{}

struct ShinyHuntDeoxys_Descriptor::Stats : public StatsTracker{
    Stats()
        : resets(m_stats["Resets"])
        , shinies(m_stats["Shinies"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shinies");
    }
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& shinies;
};
std::unique_ptr<StatsTracker> ShinyHuntDeoxys_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

ShinyHuntDeoxys::ShinyHuntDeoxys()
    : WALK_UP_DOWN_TIME(
        "<b>Walk up/down time</b><br>Spend this long to run up to the triangle rock.",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "440"
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
    PA_ADD_OPTION(WALK_UP_DOWN_TIME);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void ShinyHuntDeoxys::solve_puzzle(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) {
    env.log("Step 1: Press A from below.");
    pbf_press_button(context, BUTTON_A, 20, 40);

    env.log("Step 2: 5 Left, 1 Down.");
    pbf_press_dpad(context, DPAD_LEFT, 10, 50);
    pbf_press_dpad(context, DPAD_LEFT, 10, 50);
    pbf_press_dpad(context, DPAD_LEFT, 10, 50);
    pbf_press_dpad(context, DPAD_LEFT, 10, 50);
    pbf_press_dpad(context, DPAD_LEFT, 10, 50);

    pbf_press_dpad(context, DPAD_DOWN, 10, 80);

    pbf_press_button(context, BUTTON_A, 20, 40);
    context.wait_for_all_requests();

    env.log("Step 3: 5 Right, 5 Up.");
    pbf_press_dpad(context, DPAD_RIGHT, 10, 50);
    pbf_press_dpad(context, DPAD_RIGHT, 10, 50);
    pbf_press_dpad(context, DPAD_RIGHT, 10, 50);
    pbf_press_dpad(context, DPAD_RIGHT, 10, 50);
    pbf_press_dpad(context, DPAD_RIGHT, 10, 50);

    ssf_press_button(context, BUTTON_B, 0, 90);
    pbf_press_dpad(context, DPAD_UP, 90, 50);

    pbf_press_button(context, BUTTON_A, 20, 40);
    context.wait_for_all_requests();

    env.log("Step 4: 5 Right, 5 Down");
    pbf_press_dpad(context, DPAD_RIGHT, 10, 50);
    pbf_press_dpad(context, DPAD_RIGHT, 10, 50);
    pbf_press_dpad(context, DPAD_RIGHT, 10, 50);
    pbf_press_dpad(context, DPAD_RIGHT, 10, 50);
    pbf_press_dpad(context, DPAD_RIGHT, 10, 50);

    ssf_press_button(context, BUTTON_B, 0, 90);
    pbf_press_dpad(context, DPAD_DOWN, 90, 50);

    pbf_press_button(context, BUTTON_A, 20, 40);
    context.wait_for_all_requests();

    env.log("Step 5: 3 Up, 7 Left");
    pbf_press_dpad(context, DPAD_UP, 10, 50);
    pbf_press_dpad(context, DPAD_UP, 10, 50);
    pbf_press_dpad(context, DPAD_UP, 10, 50);

    ssf_press_button(context, BUTTON_B, 0, 115);
    pbf_press_dpad(context, DPAD_LEFT, 115, 50);

    pbf_press_button(context, BUTTON_A, 20, 40);
    context.wait_for_all_requests();

    env.log("Step 6: 5 Right.");
    ssf_press_button(context, BUTTON_B, 0, 100);
    pbf_press_dpad(context, DPAD_RIGHT, 100, 50);

    pbf_press_button(context, BUTTON_A, 20, 40);
    context.wait_for_all_requests();

    env.log("Step 7: 3 Left, 2 Down.");
    pbf_press_dpad(context, DPAD_LEFT, 10, 50);
    pbf_press_dpad(context, DPAD_LEFT, 10, 50);
    pbf_press_dpad(context, DPAD_LEFT, 10, 50);

    ssf_press_button(context, BUTTON_B, 0, 60);
    pbf_press_dpad(context, DPAD_DOWN, 60, 50);

    pbf_press_button(context, BUTTON_A, 20, 40);
    context.wait_for_all_requests();

    env.log("Step 8: 1 Down, 4 Left.");
    pbf_press_dpad(context, DPAD_DOWN, 10, 50);

    ssf_press_button(context, BUTTON_B, 0, 80);
    pbf_press_dpad(context, DPAD_LEFT, 80, 50);

    pbf_press_button(context, BUTTON_A, 20, 40);
    context.wait_for_all_requests();

    env.log("Step 8: 7 Right.");
    ssf_press_button(context, BUTTON_B, 0, 115);
    pbf_press_dpad(context, DPAD_RIGHT, 115, 50);

    pbf_press_button(context, BUTTON_A, 20, 40);
    context.wait_for_all_requests();

    env.log("Step 9: 4 Left, Down 1.");
    pbf_press_dpad(context, DPAD_LEFT, 10, 50);
    pbf_press_dpad(context, DPAD_LEFT, 10, 50);
    pbf_press_dpad(context, DPAD_LEFT, 10, 50);
    pbf_press_dpad(context, DPAD_LEFT, 10, 50);

    pbf_press_dpad(context, DPAD_DOWN, 10, 50);

    pbf_press_button(context, BUTTON_A, 20, 40);
    context.wait_for_all_requests();

    env.log("Step 10: 4 Up.");
    ssf_press_button(context, BUTTON_B, 0, 80);
    pbf_press_dpad(context, DPAD_UP, 80, 80);
    context.wait_for_all_requests();
}

void ShinyHuntDeoxys::program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context){
    ShinyHuntDeoxys_Descriptor::Stats& stats = env.current_stats<ShinyHuntDeoxys_Descriptor::Stats>();

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

    while (true) {
        env.log("Walking up to Deoxys.");
        //Walk up to the triangle rock from the ship. No bike allowed.
        ssf_press_button(context, BUTTON_B, 0, WALK_UP_DOWN_TIME);
        pbf_press_dpad(context, DPAD_UP, WALK_UP_DOWN_TIME, 20);
        context.wait_for_all_requests();

        solve_puzzle(env, context);

        //Start battle.
        pbf_press_button(context, BUTTON_A, 20, 40);

        bool legendary_shiny = handle_encounter(env.console, context);
        if (legendary_shiny) {
            stats.shinies++;
            env.update_stats();
            send_program_notification(env, NOTIFICATION_SHINY, COLOR_YELLOW, "Shiny found!", {}, "", env.console.video().snapshot(), true);
            break;
        }
        env.log("No shiny found.");
        flee_battle(env.console, context);

        //After pressing the flee button, additional dialog box pops up for Deoxys
        pbf_mash_button(context, BUTTON_B, 250);
        context.wait_for_all_requests();
        
        //Walk down from the triangle rock to the ship.
        ssf_press_button(context, BUTTON_B, 0, WALK_UP_DOWN_TIME);
        pbf_press_dpad(context, DPAD_DOWN, WALK_UP_DOWN_TIME, 20);
        context.wait_for_all_requests();

        stats.resets++;
        env.update_stats();
    }

    //switch - go home when done

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}
