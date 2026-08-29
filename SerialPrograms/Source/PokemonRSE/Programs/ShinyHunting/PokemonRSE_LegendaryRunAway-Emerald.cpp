/*  Legendary Run Away - Emerald
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Exceptions/OperationFailedExceptionWithScreenshot.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "Pokemon/Pokemon_Strings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonRSE/Inference/Dialogs/PokemonRSE_BattleDialogs.h"
#include "PokemonRSE/Inference/Dialogs/PokemonRSE_DialogDetector.h"
#include "PokemonRSE/PokemonRSE_Navigation.h"
#include "PokemonRSE_LegendaryRunAway-Emerald.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

LegendaryRunAwayEmerald_Descriptor::LegendaryRunAwayEmerald_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonRSE:LegendaryRunAwayEmerald",
        Pokemon::STRING_POKEMON + " RSE", "Legendary Run Away (Emerald)",
        "Programs/PokemonRSE/LegendaryRunAwayEmerald.html",
        "Use the Run Away method to shiny hunt legendaries in Emerald.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct LegendaryRunAwayEmerald_Descriptor::Stats : public StatsTracker{
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
std::unique_ptr<StatsTracker> LegendaryRunAwayEmerald_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

LegendaryRunAwayEmerald::LegendaryRunAwayEmerald()
    : TARGET(
        "<b>Target:</b><br>",
        {
            {Target::regis, "regis", "Regirock/Regice/Registeel"},
            {Target::groudon, "groudon", "Groudon"},
            {Target::kyogre, "kyogre", "Kyogre"},
            {Target::hooh, "hooh", "Ho-Oh"},
            {Target::lugia, "lugia", "Lugia"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Target::regis
    )
    , TAKE_VIDEO("<b>Take Video:</b><br>Record a video when the shiny is found.", LockMode::UNLOCK_WHILE_RUNNING, true)
    , GO_HOME_WHEN_DONE(true)
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
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , GROUDON_LEFT_FIRST(
        "<b>Groudon first left/right time:</b><br>Time it takes to turn and run 10 steps left/right after/before the encounter.",
        LockMode::LOCK_WHILE_RUNNING,
        "1440 ms"
    )
    , GROUDON_RIGHT_FIRST(
        "<b>Groudon second right/left time:</b><br>Time it takes to turn and run 2 steps right/left after/before the encounter.",
        LockMode::LOCK_WHILE_RUNNING,
        "280 ms"
    )
    , GROUDON_LEFT_SECOND(
        "<b>Groudon third left/right time:</b><br>Time it takes to turn and run 4 steps left/right after/before the encounter.",
        LockMode::LOCK_WHILE_RUNNING,
        "600 ms"
    )
    , KYOGRE_RIGHT_FIRST(
        "<b>Kyogre first left/right time:</b><br>Time it takes to turn and run 9 steps right/left after/before the encounter.",
        LockMode::LOCK_WHILE_RUNNING,
        "1180 ms"
    )
    , KYOGRE_UP_FIRST(
        "<b>Kyogre up/down time:</b><br>Time it takes to turn and run 10 steps up/down after/before the encounter.",
        LockMode::LOCK_WHILE_RUNNING,
        "1400 ms"
    )
    , KYOGRE_RIGHT_SECOND(
        "<b>Kyogre second left/right time:</b><br>Time it takes to turn and run 6 steps right/left after/before the encounter.",
        LockMode::LOCK_WHILE_RUNNING,
        "830 ms"
    )
    , HOOH_UP_DOWN(
        "<b>Ho-Oh up/down time:</b><br>Time it takes to run up to Ho-Oh or down away to reset.",
        LockMode::LOCK_WHILE_RUNNING,
        "1440 ms"
    )
    , HOOH_LEFT_RIGHT(
        "<b>Ho-Oh left/right time:</b><br>Time it takes when facing the same direction to take one step left or right.",
        LockMode::LOCK_WHILE_RUNNING,
        "240 ms"
    )
    , LUGIA_UP_DOWN0(
        "<b>Lugia up/down time:</b><br>Time it takes to run up to Lugia or down away to reset.",
        LockMode::LOCK_WHILE_RUNNING,
        "720 ms"
    )
    , LUGIA_LEFT_RIGHT0(
        "<b>Lugia left time:</b><br>Time it takes walk three steps left after entering Lugia's room.",
        LockMode::LOCK_WHILE_RUNNING,
        "520 ms"
    )
{
    PA_ADD_STATIC(SHINY_REQUIRES_AUDIO);
    PA_ADD_OPTION(TARGET);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(GROUDON_LEFT_FIRST);
    PA_ADD_OPTION(GROUDON_RIGHT_FIRST);
    PA_ADD_OPTION(GROUDON_LEFT_SECOND);
    PA_ADD_OPTION(KYOGRE_RIGHT_FIRST);
    PA_ADD_OPTION(KYOGRE_UP_FIRST);
    PA_ADD_OPTION(KYOGRE_RIGHT_SECOND);
    PA_ADD_OPTION(HOOH_UP_DOWN);
    PA_ADD_OPTION(HOOH_LEFT_RIGHT);
    PA_ADD_OPTION(LUGIA_UP_DOWN0);
    PA_ADD_OPTION(LUGIA_LEFT_RIGHT0);
}

void LegendaryRunAwayEmerald::reset_regi(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    LegendaryRunAwayEmerald_Descriptor::Stats& stats = env.current_stats<LegendaryRunAwayEmerald_Descriptor::Stats>();

    //turn around, walk down 4/until black screen over
    BlackScreenOverWatcher exit_area(COLOR_RED);
    BlackScreenOverWatcher enter_area(COLOR_RED);
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 960ms);
            pbf_press_dpad(context, DPAD_DOWN, 960ms, 160ms);
            pbf_wait(context, 2400ms);
        },
        {exit_area}
    );
    context.wait_for_all_requests();
    if (ret != 0){
        env.log("Failed to exit area.", COLOR_RED);
        stats.errors++;
        env.update_stats();
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to exit area.",
            env.console
        );
    }else{
        env.log("Left area.");
    }
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();

    //turn around, up one/black screen over
    int ret2 = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            pbf_press_dpad(context, DPAD_UP, 960ms, 160ms);
            pbf_wait(context, 2400ms);
        },
        {enter_area}
    );
    context.wait_for_all_requests();
    if (ret2 != 0){
        env.log("Failed to enter area.", COLOR_RED);
        stats.errors++;
        env.update_stats();
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to enter area.",
            env.console
        );
    }else{
        env.log("Entered area.");
    }
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();

    //walk back up to the regi
    ssf_press_button(context, BUTTON_B, 0ms, 480ms);
    pbf_press_dpad(context, DPAD_UP, 480ms, 160ms);

    context.wait_for_all_requests();
}

void LegendaryRunAwayEmerald::reset_groudon(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    LegendaryRunAwayEmerald_Descriptor::Stats& stats = env.current_stats<LegendaryRunAwayEmerald_Descriptor::Stats>();

    //Turn left. Take 10 steps.
    ssf_press_button(context, BUTTON_B, 0ms, GROUDON_LEFT_FIRST);
    pbf_press_dpad(context, DPAD_LEFT, GROUDON_LEFT_FIRST, 0ms);
    context.wait_for_all_requests();

    //Turn up. Take 14 steps. (Bump into wall.)
    ssf_press_button(context, BUTTON_B, 0ms, 1920ms);
    pbf_press_dpad(context, DPAD_UP, 1920ms, 0ms);
    context.wait_for_all_requests();

    //Turn right. Take 2 steps.
    ssf_press_button(context, BUTTON_B, 0ms, GROUDON_RIGHT_FIRST);
    pbf_press_dpad(context, DPAD_RIGHT, GROUDON_RIGHT_FIRST, 0ms);
    context.wait_for_all_requests();

    //Turn up. Take 8 steps (Bump into wall.)
    ssf_press_button(context, BUTTON_B, 0ms, 1120ms);
    pbf_press_dpad(context, DPAD_UP, 1120ms, 0ms);
    context.wait_for_all_requests();

    //Turn left. Take 4 steps.
    ssf_press_button(context, BUTTON_B, 0ms, GROUDON_LEFT_SECOND);
    pbf_press_dpad(context, DPAD_LEFT, GROUDON_LEFT_SECOND, 0ms);
    context.wait_for_all_requests();

    //Turn down. Exit. Black screen over.
    BlackScreenOverWatcher exit_area(COLOR_RED);
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 720ms);
            pbf_press_dpad(context, DPAD_DOWN, 720ms, 0ms);
            pbf_wait(context, 2400ms);
        },
        {exit_area}
    );
    context.wait_for_all_requests();
    if (ret != 0){
        env.log("Failed to exit area.", COLOR_RED);
        stats.errors++;
        env.update_stats();
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to exit area.",
            env.console
        );
    }else{
        env.log("Left area.");
    }
    pbf_wait(context, 1000ms);
    context.wait_for_all_requests();

    //Reverse above steps.
    BlackScreenOverWatcher enter_area(COLOR_RED);
    int ret2 = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 720ms);
            pbf_press_dpad(context, DPAD_UP, 720ms, 0ms);
            pbf_wait(context, 2400ms);
        },
        {enter_area}
    );
    context.wait_for_all_requests();
    if (ret2 != 0){
        stats.errors++;
        env.update_stats();
        env.log("Failed to enter area.", COLOR_RED);
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to enter area.",
            env.console
        );
    }else{
        env.log("Entered area.");
    }
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();

    ssf_press_button(context, BUTTON_B, 0ms, GROUDON_LEFT_SECOND);
    pbf_press_dpad(context, DPAD_RIGHT, GROUDON_LEFT_SECOND, 160ms);
    context.wait_for_all_requests();

    ssf_press_button(context, BUTTON_B, 0ms, 1120ms);
    pbf_press_dpad(context, DPAD_DOWN, 1120ms, 160ms);
    context.wait_for_all_requests();

    ssf_press_button(context, BUTTON_B, 0ms, GROUDON_RIGHT_FIRST);
    pbf_press_dpad(context, DPAD_LEFT, GROUDON_RIGHT_FIRST, 160ms);
    context.wait_for_all_requests();

    ssf_press_button(context, BUTTON_B, 0ms, 1920ms);
    pbf_press_dpad(context, DPAD_DOWN, 1920ms, 160ms);
    context.wait_for_all_requests();

    ssf_press_button(context, BUTTON_B, 0ms, GROUDON_LEFT_FIRST);
    pbf_press_dpad(context, DPAD_RIGHT, GROUDON_LEFT_FIRST, 160ms);
    context.wait_for_all_requests();
}

void LegendaryRunAwayEmerald::reset_kyogre(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    LegendaryRunAwayEmerald_Descriptor::Stats& stats = env.current_stats<LegendaryRunAwayEmerald_Descriptor::Stats>();

    //Turn down. Take 1 step and hit the wall. No config needed.
    ssf_press_button(context, BUTTON_B, 0ms, 200ms);
    pbf_press_dpad(context, DPAD_DOWN, 200ms, 0ms);
    context.wait_for_all_requests();

    //Turn right. Take 9 steps.
    ssf_press_button(context, BUTTON_B, 0ms, KYOGRE_RIGHT_FIRST);
    pbf_press_dpad(context, DPAD_RIGHT, KYOGRE_RIGHT_FIRST, 0ms);
    context.wait_for_all_requests();

    //Turn up. 13 steps. Wall. No config needed.
    ssf_press_button(context, BUTTON_B, 0ms, 1800ms);
    pbf_press_dpad(context, DPAD_UP, 1800ms, 0ms);
    context.wait_for_all_requests();

    //Turn left. 4 steps. Wall. No config needed.
    ssf_press_button(context, BUTTON_B, 0ms, 680ms);
    pbf_press_dpad(context, DPAD_LEFT, 680ms, 0ms);
    context.wait_for_all_requests();

    //Turn up. 10 steps.
    ssf_press_button(context, BUTTON_B, 0ms, KYOGRE_UP_FIRST);
    pbf_press_dpad(context, DPAD_UP, KYOGRE_UP_FIRST, 0ms);
    context.wait_for_all_requests();

    //Turn right. 6 steps.
    ssf_press_button(context, BUTTON_B, 0ms, KYOGRE_RIGHT_SECOND);
    pbf_press_dpad(context, DPAD_RIGHT, KYOGRE_RIGHT_SECOND, 0ms);
    context.wait_for_all_requests();

    //Turn down. Exit. Black screen over.
    BlackScreenOverWatcher exit_area(COLOR_RED);
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 720ms);
            pbf_press_dpad(context, DPAD_DOWN, 720ms, 160ms);
            pbf_wait(context, 2400ms);
        },
        {exit_area}
    );
    context.wait_for_all_requests();
    if (ret != 0){
        env.log("Failed to exit area.", COLOR_RED);
        stats.errors++;
        env.update_stats();
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to exit area.",
            env.console
        );
    }else{
        env.log("Left area.");
    }
    pbf_wait(context, 1000ms);
    context.wait_for_all_requests();

    BlackScreenOverWatcher enter_area(COLOR_RED);
    int ret2 = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 720ms);
            pbf_press_dpad(context, DPAD_UP, 720ms, 0ms);
            pbf_wait(context, 2400ms);
        },
        {enter_area}
    );
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();
    if (ret2 != 0){
        env.log("Failed to enter area.", COLOR_RED);
        stats.errors++;
        env.update_stats();
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to enter area.",
            env.console
        );
    }else{
        env.log("Entered area.");
    }
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();

    ssf_press_button(context, BUTTON_B, 0ms, KYOGRE_RIGHT_SECOND);
    pbf_press_dpad(context, DPAD_LEFT, KYOGRE_RIGHT_SECOND, 0ms);
    context.wait_for_all_requests();

    ssf_press_button(context, BUTTON_B, 0ms, KYOGRE_UP_FIRST);
    pbf_press_dpad(context, DPAD_DOWN, KYOGRE_UP_FIRST, 0ms);
    context.wait_for_all_requests();

    ssf_press_button(context, BUTTON_B, 0ms, 680ms);
    pbf_press_dpad(context, DPAD_RIGHT, 680ms, 0ms);
    context.wait_for_all_requests();

    ssf_press_button(context, BUTTON_B, 0ms, 1800ms);
    pbf_press_dpad(context, DPAD_DOWN, 1800ms, 0ms);
    context.wait_for_all_requests();

    ssf_press_button(context, BUTTON_B, 0ms, KYOGRE_RIGHT_FIRST);
    pbf_press_dpad(context, DPAD_LEFT, KYOGRE_RIGHT_FIRST, 0ms);
    context.wait_for_all_requests();

    ssf_press_button(context, BUTTON_B, 0ms, 200ms);
    pbf_press_dpad(context, DPAD_UP, 200ms, 0ms);
    context.wait_for_all_requests();
}

void LegendaryRunAwayEmerald::reset_hooh(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    LegendaryRunAwayEmerald_Descriptor::Stats& stats = env.current_stats<LegendaryRunAwayEmerald_Descriptor::Stats>();

    BlackScreenOverWatcher exit_area(COLOR_RED);
    //Turn around, 10 steps down
    ssf_press_button(context, BUTTON_B, 0ms, HOOH_UP_DOWN);
    pbf_press_dpad(context, DPAD_DOWN, HOOH_UP_DOWN, 0ms);
    context.wait_for_all_requests();

    //Turn right, take 1 step. Wait for black screen over.
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 500ms);
            pbf_press_dpad(context, DPAD_RIGHT, 500ms, 0ms);
            pbf_wait(context, 2400ms);
        },
        {exit_area}
    );
    context.wait_for_all_requests();
    if (ret != 0){
        env.log("Failed to exit area.", COLOR_RED);
        stats.errors++;
        env.update_stats();
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to exit area.",
            env.console
        );
    }else{
        env.log("Left area.");
    }
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();

    BlackScreenOverWatcher enter_area(COLOR_RED);
    //turn left, take one step. now turn back right and take a step. wait for black screen over.
    int ret2 = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 500ms);
            pbf_press_dpad(context, DPAD_LEFT, 500ms, 0ms);

            ssf_press_button(context, BUTTON_B, 0ms, 500ms);
            pbf_press_dpad(context, DPAD_RIGHT, 500ms, 0ms);
            pbf_wait(context, 2400ms);
        },
        {enter_area}
    );
    context.wait_for_all_requests();
    if (ret2 != 0){
        env.log("Failed to enter area.", COLOR_RED);
        stats.errors++;
        env.update_stats();
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to enter area.",
            env.console
        );
    }else{
        env.log("Entered area.");
    }
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();

    //reverse above steps, but only take 9 steps up
    //doesn't really matter since we want to trigger the encounter anyway
    pbf_press_dpad(context, DPAD_LEFT, HOOH_LEFT_RIGHT, 160ms);

    ssf_press_button(context, BUTTON_B, 0ms, HOOH_UP_DOWN);
    pbf_press_dpad(context, DPAD_UP, HOOH_UP_DOWN, 160ms);

    context.wait_for_all_requests();
}

void LegendaryRunAwayEmerald::reset_lugia(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    LegendaryRunAwayEmerald_Descriptor::Stats& stats = env.current_stats<LegendaryRunAwayEmerald_Descriptor::Stats>();

    BlackScreenOverWatcher exit_area(COLOR_RED);
    //Turn around, 5 steps down
    ssf_press_button(context, BUTTON_B, 0ms, LUGIA_UP_DOWN0);
    pbf_press_dpad(context, DPAD_DOWN, LUGIA_UP_DOWN0, 0ms);
    context.wait_for_all_requests();

    //Turn right, 3 steps right. Wait for black screen over.
    int ret = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 720ms);
            pbf_press_dpad(context, DPAD_RIGHT, 720ms, 0ms);
            pbf_wait(context, 2400ms);
        },
        {exit_area}
    );
    context.wait_for_all_requests();
    if (ret != 0){
        env.log("Failed to exit area.", COLOR_RED);
        stats.errors++;
        env.update_stats();
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to exit area.",
            env.console
        );
    }else{
        env.log("Left area.");
    }
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();

    BlackScreenOverWatcher enter_area(COLOR_RED);
    //turn up, take one step. then turn back down and take a step. wait for black screen over.
    int ret2 = run_until<ProControllerContext>(
        env.console, context,
        [](ProControllerContext& context){
            ssf_press_button(context, BUTTON_B, 0ms, 500ms);
            pbf_press_dpad(context, DPAD_UP, 500ms, 0ms);

            ssf_press_button(context, BUTTON_B, 0ms, 500ms);
            pbf_press_dpad(context, DPAD_DOWN, 500ms, 0ms);
            pbf_wait(context, 2400ms);
        },
        {enter_area}
    );
    context.wait_for_all_requests();
    if (ret2 != 0){
        env.log("Failed to enter area.", COLOR_RED);
        stats.errors++;
        env.update_stats();
        OperationFailedExceptionWithScreenshot::fire(
            ErrorReport::SEND_ERROR_REPORT,
            "Failed to enter area.",
            env.console
        );
    }else{
        env.log("Entered area.");
    }
    pbf_wait(context, 500ms);
    context.wait_for_all_requests();

    //reverse above steps
    ssf_press_button(context, BUTTON_B, 0ms, LUGIA_LEFT_RIGHT0);
    pbf_press_dpad(context, DPAD_LEFT, LUGIA_LEFT_RIGHT0, 0ms);

    ssf_press_button(context, BUTTON_B, 0ms, LUGIA_UP_DOWN0);
    pbf_press_dpad(context, DPAD_UP, LUGIA_UP_DOWN0, 0ms);

    context.wait_for_all_requests();
}

void LegendaryRunAwayEmerald::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    LegendaryRunAwayEmerald_Descriptor::Stats& stats = env.current_stats<LegendaryRunAwayEmerald_Descriptor::Stats>();

    home_black_border_check(env.console, context);

    /*
    * Text speed fast, battle animations off
    * smoke ball or fast pokemon req. no entry effects.
    * 
    * Don't need to worry about PokeNav or random encounters for any of these targets.
    * 
    * Stand in front of Regis/Ho-Oh/Lugia. Save the game.
    */

    while (true){
        BlackScreenWatcher legendary_battle_start(COLOR_RED);
        int ret3 = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                for (int i = 0; i < 5; i++){
                    switch (TARGET){
                    case Target::hooh:
                    case Target::kyogre:
                    case Target::groudon:
                        //Step forward to start the encounter.
                        pbf_press_dpad(context, DPAD_UP, 160ms, 400ms);
                        break;
                    //case Target::groudon: //Step up is easier.
                    //    pbf_press_dpad(context, DPAD_RIGHT, 160ms, 400ms);
                    //    break;
                    //case Target::kyogre:
                    //    pbf_press_dpad(context, DPAD_LEFT, 160ms, 400ms);
                    //    break;
                    case Target::lugia:
                    case Target::regis:
                        pbf_mash_button(context, BUTTON_A, 5000ms);
                        context.wait_for_all_requests();
                    default:;
                    }
                    pbf_wait(context, 2000ms);
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
            OperationFailedExceptionWithScreenshot::fire(
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
        
        //Close out dialog box
        pbf_mash_button(context, BUTTON_B, 2000ms);
        context.wait_for_all_requests();
        
        //Exit and re-enter the room
        switch (TARGET){
        case Target::regis:
            reset_regi(env, context);
            break;
        case Target::groudon:
            reset_groudon(env, context);
            break;
        case Target::kyogre:
            reset_kyogre(env, context);
            break;
        case Target::hooh:
            reset_hooh(env, context);
            break;
        case Target::lugia:
            reset_lugia(env, context);
            break;
        default:
            OperationFailedExceptionWithScreenshot::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Invalid target!",
                env.console
            );
            break;
        }

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
