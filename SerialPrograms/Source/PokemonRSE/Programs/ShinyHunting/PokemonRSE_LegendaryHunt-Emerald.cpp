/*  Legendary Hunt - Emerald
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
#include "PokemonRSE_LegendaryHunt-Emerald.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

LegendaryHuntEmerald_Descriptor::LegendaryHuntEmerald_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonRSE:LegendaryHuntEmerald",
        Pokemon::STRING_POKEMON + " RSE", "Legendary Hunt (Emerald)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonRSE/LegendaryHuntEmerald.md",
        "Use the Run Away method to shiny hunt legendaries in Emerald.",
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {SerialPABotBase::OLD_NINTENDO_SWITCH_DEFAULT_REQUIREMENTS}
    )
{}

struct LegendaryHuntEmerald_Descriptor::Stats : public StatsTracker{
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
std::unique_ptr<StatsTracker> LegendaryHuntEmerald_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

LegendaryHuntEmerald::LegendaryHuntEmerald()
    : TARGET(
        "<b>Starter:</b><br>",
        {
            {Target::regis, "regis", "Regirock/Regice/Registeel"},
            {Target::hooh, "hooh", "Ho-Oh"},
            {Target::lugia, "lugia", "Lugia"},
            {Target::latis, "latis", "Latias/Latios (Southern Island)"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Target::hooh
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
    PA_ADD_OPTION(TARGET);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void LegendaryHuntEmerald::reset_lugia(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) {
    BlackScreenOverWatcher exit_area(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    //Turn around, 5 steps down
    ssf_press_button(context, BUTTON_B, 0, 90);
    pbf_press_dpad(context, DPAD_DOWN, 90, 20);

    //Turn right, 3 steps right. Wait for black screen over.
    int ret = run_until<SwitchControllerContext>(
        env.console, context,
        [](SwitchControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0, 90);
            pbf_press_dpad(context, DPAD_RIGHT, 90, 20);
            pbf_wait(context, 300);
        },
        {exit_area}
    );
    context.wait_for_all_requests();
    if (ret != 0){
        env.log("Failed to exit area.", COLOR_RED);
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to exit area.",
            env.console
        );
    }
    else {
        env.log("Left area.");
    }

    BlackScreenOverWatcher enter_area(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
    //turn up, take one step. then turn back down and take a step. wait for black screen over.
    int ret2 = run_until<SwitchControllerContext>(
        env.console, context,
        [](SwitchControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0, 40);
            pbf_press_dpad(context, DPAD_UP, 40, 20);

            ssf_press_button(context, BUTTON_B, 0, 40);
            pbf_press_dpad(context, DPAD_DOWN, 40, 20);
            pbf_wait(context, 300);
        },
        {enter_area}
    );
    context.wait_for_all_requests();
    if (ret2 != 0){
        env.log("Failed to enter area.", COLOR_RED);
        OperationFailedException::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to enter area.",
            env.console
        );
    }
    else {
        env.log("Entered area.");
    }

    //reverse above steps
    ssf_press_button(context, BUTTON_B, 0, 70);
    pbf_press_dpad(context, DPAD_LEFT, 70, 20);

    ssf_press_button(context, BUTTON_B, 0, 90);
    pbf_press_dpad(context, DPAD_UP, 90, 20);

    context.wait_for_all_requests();
}

void LegendaryHuntEmerald::program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context){
    LegendaryHuntEmerald_Descriptor::Stats& stats = env.current_stats<LegendaryHuntEmerald_Descriptor::Stats>();

    /*
    * Text speed fast, battle animations off
    * smoke ball or fast pokemon req.
    * 
    * Don't need to worry about PokeNav or random encounters for any of these targets.
    * 
    * Stand in front of Regis/Ho-Oh/Lugia. Save the game.
    */

    while (true) {
        //Start battle.
        if (TARGET == Target::hooh) {
            //Step forward to start the encounter.
            pbf_press_dpad(context, DPAD_UP, 10, 50);
        }
        else {
            //All other legendaries.
            pbf_press_button(context, BUTTON_A, 20, 40);
        }

        bool legendary_shiny = handle_encounter(env.console, context, true);
        if (legendary_shiny) {
            stats.shinies++;
            env.update_stats();
            send_program_notification(env, NOTIFICATION_SHINY, COLOR_YELLOW, "Shiny found!", {}, "", env.console.video().snapshot(), true);
            break;
        }
        env.log("No shiny found.");
        flee_battle(env.console, context);

        //Close out dialog box
        pbf_mash_button(context, BUTTON_B, 250);
        context.wait_for_all_requests();
        
        //Exit and re-enter the room
        switch (TARGET) {
        case Target::regis:
            //turn around, walk down 4

            //black screen over

            //turn around, up one

            //black screen over

            //reverse the above

            break;
        case Target::hooh:
            //Turn around

            //10 steps down

            //Turn right

            //Take one step

            //Wait for black screen over

            //Turn left and take a step

            //now turn right and take a step

            //wait for black screen over

            //now reverse the above, but only take 9 steps up
            break;
        case Target::lugia:
            reset_lugia(env, context);
            break;
        case Target::latis:
            //TODO
            break;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Invalid target!",
                env.console
            );
            break;
        }

        stats.resets++;
        env.update_stats();
    }

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}
