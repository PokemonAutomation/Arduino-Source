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
#include "CommonTools/VisualDetectors/BlackScreenDetector.h"
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
        "Programs/PokemonRSE/AudioStarterReset.html",
        "Soft reset for a shiny starter. Ruby and Sapphire only.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::VIDEO_AUDIO,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}

struct AudioStarterReset_Descriptor::Stats : public StatsTracker{
    Stats()
        : resets(m_stats["Resets"])
        , poochyena(m_stats["Shiny Poochyena"])
        , shinystarter(m_stats["Shiny Starter"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Resets");
        m_display_order.emplace_back("Shiny Poochyena");
        m_display_order.emplace_back("Shiny Starter");
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }
    std::atomic<uint64_t>& resets;
    std::atomic<uint64_t>& poochyena;
    std::atomic<uint64_t>& shinystarter;
    std::atomic<uint64_t>& errors;
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
    , TAKE_VIDEO("<b>Take Video:</b><br>Record a video when the shiny starter is found.", LockMode::UNLOCK_WHILE_RUNNING, true)
    , GO_HOME_WHEN_DONE(true)
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
    PA_ADD_STATIC(SHINY_REQUIRES_AUDIO);
    PA_ADD_OPTION(TARGET);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void AudioStarterReset::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    AudioStarterReset_Descriptor::Stats& stats = env.current_stats<AudioStarterReset_Descriptor::Stats>();

    home_black_border_check(env.console, context);

    /*
    * Settings: Text Speed fast.
    * Setup: Stand in front of the Professor's bag and save the game.
    * 
    * Required to fight, so have to do the SR method instead of run away
    * Soft reset programs are only for Ruby/Sapphire, as Emerald always has the same starting seed.
    * 
    * This also assumes no dry battery.
    */

    bool shiny_starter = false;
    while (!shiny_starter){
        float shiny_coefficient = 1.0;
        ShinySoundDetector pooch_detector(env.console, [&](float error_coefficient) -> bool{
            shiny_coefficient = error_coefficient;
            return true;
        });

        //Pressing A opens the bag so the screen goes black
        env.log("Opening bag and selecting starter.");
        BlackScreenOverWatcher bag_opened(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
        int ret = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                for (int i = 0; i < 10; i++){
                    pbf_press_button(context, BUTTON_A, 320ms, 640ms);
                    pbf_wait(context, 2000ms);
                    context.wait_for_all_requests();
                }
            },
            {bag_opened}
        );
        context.wait_for_all_requests();
        if (ret != 0){
            env.log("Failed to open bag after 10 attempts.", COLOR_RED);
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to open bag after 10 attempts.",
                env.console
            );
        }else{
            env.log("Opened professor's bag.");
        }

        switch (TARGET){
        case Target::treecko:
            env.log("Treecko selected. Moving left.");
            pbf_press_dpad(context, DPAD_LEFT, 320ms, 800ms);
            break;
        case Target::torchic:
            //Default cursor position, do nothing.
            env.log("Torchic selected. Do not move cursor.");
            break;
        case Target::mudkip:
            env.log("Mudkip selected. Moving right.");
            pbf_press_dpad(context, DPAD_RIGHT, 320ms, 800ms);
            break;
        default:
            env.log("Invalid target selected.");
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "AudioStarterReset: Invalid target.",
                env.console
            );
            break;
        }

        //Mash A to select starter. Stop once black screen is detected to start listening for shiny pooch.
        BlackScreenWatcher starter_battle_start(COLOR_RED, {0.282, 0.064, 0.448, 0.871});
        int ret3 = run_until<ProControllerContext>(
            env.console, context,
            [](ProControllerContext& context){
                for (int i = 0; i < 10; i++){
                    pbf_mash_button(context, BUTTON_A, 5000ms);
                    pbf_wait(context, 2000ms);
                    context.wait_for_all_requests();
                }
            },
            {starter_battle_start}
        );
        context.wait_for_all_requests();
        if (ret3 != 0){
            env.log("Failed to start battle after 10 attempts.", COLOR_RED);
            stats.errors++;
            env.update_stats();
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to start battle after 10 attempts.",
                env.console
            );
        }else{
            env.log("Pooch battle started.");
        }
        context.wait_for_all_requests();

        env.log("Checking for shiny Poochyena.");
        AdvanceBattleDialogWatcher pooch_appeared(COLOR_YELLOW);

        int res = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                int ret = wait_until(
                    env.console, context,
                    std::chrono::seconds(20),
                    {{pooch_appeared}}
                );
                if (ret == 0){
                    env.log("Advance arrow detected.");
                } else {
                    env.log("Battle Advance arrow was not detected.");
                    stats.errors++;
                    env.update_stats();
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Battle Advance arrow was not detected.",
                        env.console
                    );
                }
                pbf_wait(context, 1000ms);
                context.wait_for_all_requests();
            },
            {{pooch_detector}}
        );
        pooch_detector.throw_if_no_sound(std::chrono::milliseconds(1000));
        if (res == 0){
            env.log("Shiny Poochyena detected!");
            stats.poochyena++;
            env.update_stats();
            send_program_notification(env,
                NOTIFICATION_SHINY_POOCH,
                COLOR_YELLOW,
                "Shiny Poochyena found",
                {}, "",
                env.console.video().snapshot(),
                true
            );
        }else{
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
            [&](ProControllerContext& context){
                env.log("Sending out selected starter.");
                pbf_press_button(context, BUTTON_A, 320ms, 320ms);

                int ret = wait_until(
                    env.console, context,
                    std::chrono::seconds(20),
                    {{battle_menu}}
                );
                if (ret == 0){
                    env.log("Battle menu detecteed!");
                } else {
                    env.log("Battle menu was not detected.");
                    stats.errors++;
                    env.update_stats();
                    OperationFailedException::fire(
                        ErrorReport::SEND_ERROR_REPORT,
                        "Battle menu was not detected.",
                        env.console
                    );
                }
                pbf_wait(context, 1000ms);
                context.wait_for_all_requests();
            },
            {{starter_detector}}
        );
        starter_detector.throw_if_no_sound(std::chrono::milliseconds(1000));
        context.wait_for_all_requests();
        if (res2 == 0){
            env.log("Shiny starter detected!");
            stats.shinystarter++;
            env.update_stats();
            send_program_notification(
                env,
                NOTIFICATION_SHINY_STARTER,
                COLOR_YELLOW,
                "Shiny starter found!",
                {}, "",
                env.console.video().snapshot(),
                true
            );
            shiny_starter = true;
            if (TAKE_VIDEO){
                pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 0ms);
            }
        }else{
            env.log("Starter is not shiny.");
            env.log("Soft resetting.");
            send_program_status_notification(
                env, NOTIFICATION_STATUS_UPDATE,
                "Soft resetting."
            );
            stats.errors += soft_reset(env.console, context);
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

