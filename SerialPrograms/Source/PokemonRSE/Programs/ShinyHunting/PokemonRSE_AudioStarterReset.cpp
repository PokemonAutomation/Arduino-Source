/*  RS Starter Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/StartupChecks/StartProgramChecks.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonRSE/Inference/Dialogs/PokemonRSE_DialogDetector.h"
#include "PokemonRSE/Inference/Sounds/PokemonRSE_ShinySoundDetector.h"
#include "PokemonRSE/PokemonRSE_Navigation.h"
#include "PokemonRSE_AudioStarterReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

AudioStarterReset_Descriptor::AudioStarterReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonRSE:AudioStarterReset",
        Pokemon::STRING_POKEMON + " RSE", "Starter Reset",
        "ComputerControl/blob/master/Wiki/Programs/PokemonRSE/AudioStarterReset.md",
        "Soft reset for a shiny starter. Ruby and Sapphire only.",
        ProgramControllerClass::StandardController_RequiresPrecision,
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        {}
    )
{}

struct AudioStarterReset_Descriptor::Stats : public StatsTracker{
    Stats()
        : resets(m_stats["Resets"])
        , poochyena(m_stats["Shiny Poochyena"])
        , shinystarter(m_stats["Shiny Starter"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shiny Poochyena");
        m_display_order.emplace_back("Shiny Starter");
    }
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& poochyena;
    std::atomic<uint64_t>& shinystarter;
};
std::unique_ptr<StatsTracker> AudioStarterReset_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

AudioStarterReset::AudioStarterReset()
    : TARGET(
        "<b>Starter:</b><br>",
        {
            {Target::treecko, "treecko", "Treecko"},
            {Target::torchic, "torchic", "Torchic"},
            {Target::mudkip, "mudkip", "Mudkip"},
        },
        LockMode::LOCK_WHILE_RUNNING,
        Target::treecko
    )
    , NOTIFICATION_SHINY_POOCH(
        "Shiny Poochyena",
        false, false, ImageAttachmentMode::JPG,
        {"Notifs"}
    )
    , NOTIFICATION_SHINY_STARTER(
        "Shiny Starter",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_SHINY_POOCH,
        &NOTIFICATION_SHINY_STARTER,
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        })
{
    PA_ADD_OPTION(TARGET);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void AudioStarterReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    StartProgramChecks::check_performance_class_wired_or_wireless(context);

    AudioStarterReset_Descriptor::Stats& stats = env.current_stats<AudioStarterReset_Descriptor::Stats>();

    /*
    * Settings: Text Speed fast.
    * Full screen, no filter? The device I'm using to test has similar looking output, but I don't have switch online+.
    * If on a retro handheld, make sure the screen matches that of NSO+.
    * 
    * Setup: Stand in front of the Professor's bag and save the game.
    * 
    * Required to fight, so have to do the SR method instead of run away
    * Soft reset programs are only for Ruby/Sapphire, as Emerald has the 0 seed issue.
    * 
    * This also assumes no dry battery.
    */

    bool shiny_starter = false;
    while (!shiny_starter) {
        float shiny_coefficient = 1.0;
        ShinySoundDetector pooch_detector(env.console, [&](float error_coefficient) -> bool{
            shiny_coefficient = error_coefficient;
            return true;
        });

        env.log("Opening bag and selecting starter.");
        pbf_press_button(context, BUTTON_A, 40, 180);

        switch (TARGET) {
        case Target::treecko:
            pbf_press_dpad(context, DPAD_LEFT, 40, 100);
            break;
        case Target::torchic:
            //Default cursor position, do nothing.
            break;
        case Target::mudkip:
            pbf_press_dpad(context, DPAD_RIGHT, 40, 100);
            break;
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "AudioStarterReset: Invalid target.",
                env.console
            );
            break;
        }
        pbf_mash_button(context, BUTTON_A, 540);
        context.wait_for_all_requests();

        env.log("Starter selected. Checking for shiny Poochyena.");
        AdvanceBattleDialogWatcher pooch_appeared(COLOR_YELLOW);

        int res = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context) {
                int ret = wait_until(
                    env.console, context,
                    std::chrono::seconds(20),
                    {{pooch_appeared}}
                );
                if (ret == 0) {
                    env.log("Advance arrow detected.");
                }
                pbf_wait(context, 125);
                context.wait_for_all_requests();
            },
            {{pooch_detector}}
        );
        pooch_detector.throw_if_no_sound();
        if (res == 0){
            env.log("Shiny Poochyena detected!");
            stats.poochyena++;
            env.update_stats();
            send_program_notification(env, NOTIFICATION_SHINY_POOCH, COLOR_YELLOW, "Shiny Poochyena found", {}, "", env.console.video().snapshot(), true);
        }
        else {
            env.log("Poochyena is not shiny.");
        }
        context.wait_for_all_requests();

        float shiny_coefficient2 = 1.0;
        ShinySoundDetector starter_detector(env.console, [&](float error_coefficient) -> bool{
            shiny_coefficient2 = error_coefficient;
            return true;
        });

        BattleMenuWatcher battle_menu(COLOR_RED);
        int res2 = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context) {
                env.log("Sending out selected starter.");
                pbf_press_button(context, BUTTON_A, 40, 40);

                int ret = wait_until(
                    env.console, context,
                    std::chrono::seconds(20),
                    {{battle_menu}}
                );
                if (ret == 0) {
                    env.log("Battle menu detecteed!");
                }
                pbf_wait(context, 125);
                context.wait_for_all_requests();
            },
            {{starter_detector}}
        );
        starter_detector.throw_if_no_sound();
        context.wait_for_all_requests();
        if (res2 == 0){
            env.log("Shiny starter detected!");
            stats.shinystarter++;
            env.update_stats();
            send_program_notification(env, NOTIFICATION_SHINY_STARTER, COLOR_YELLOW, "Shiny starter found!", {}, "", env.console.video().snapshot(), true);
            shiny_starter = true;
        }
        else {
            env.log("Starter is not shiny.");
            env.log("Soft resetting.");
            send_program_status_notification(
                env, NOTIFICATION_STATUS_UPDATE,
                "Soft resetting."
            );
            stats.resets++;
            env.update_stats();
            soft_reset(env.program_info(), env.console, context);
        }
    }

    //if system set to nintendo switch, have go home when done option?

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}

