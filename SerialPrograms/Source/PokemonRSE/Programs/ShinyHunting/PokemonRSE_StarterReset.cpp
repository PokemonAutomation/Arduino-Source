/*  RS Starter Reset
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/VideoResolutionCheck.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "PokemonRSE/Inference/Sounds/PokemonRSE_ShinySoundDetector.h"
#include "PokemonRSE/PokemonRSE_Navigation.h"
#include "PokemonRSE_StarterReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

StarterReset_Descriptor::StarterReset_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonRSE:StarterReset",
        "Pokemon RSE", "Starter Reset",
        "ComputerControl/blob/master/Wiki/Programs/PokemonRSE/StarterReset.md",
        "(Audio only) Soft reset for a shiny starter. WIP, audio recognition does not work well.",
        //FeedbackType::VIDEO_AUDIO,
        FeedbackType::NONE,
        AllowCommandsWhenRunning::DISABLE_COMMANDS,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}

struct StarterReset_Descriptor::Stats : public StatsTracker{
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
std::unique_ptr<StatsTracker> StarterReset_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}

StarterReset::StarterReset()
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
    , POOCH_WAIT(
        "<b>Battle start wait:</b><br>Time for battle to start and for Poochyena to appear. Make sure to add extra time in case the Poochyena is shiny.",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "6 * TICKS_PER_SECOND"
    )
    , STARTER_WAIT(
        "<b>Send out starter wait:</b><br>After pressing A to send out your selected starter, wait this long for the animation. Make sure to add extra time in case it is shiny.",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "6 * TICKS_PER_SECOND"
    )
    , NOTIFICATION_SHINY_POOCH(
        "Shiny Poochyena",
        false, false,
        {"Notifs"}
    )
    , NOTIFICATION_SHINY_STARTER(
        "Shiny Starter",
        true, false,
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

void StarterReset::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    //assert_16_9_720p_min(env.logger(), env.console);
    StarterReset_Descriptor::Stats& stats = env.current_stats<StarterReset_Descriptor::Stats>();

    /*
    * Settings: Text Speed fast.
    * Setup: Stand in front of the Professor's bag and save the game.
    * 
    * Required to fight, so have to do the SR method instead of run away
    * Soft reset programs are only for Ruby/Sapphire, as Emerald has the 0 seed issue.
    * 
    * This also assumes no dry battery.
    */

    bool shiny_starter = false;
    while (!shiny_starter) {

        ShinySoundDetector pooch_detector(env.console, [&](float error_coefficient) -> bool{
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
                env.console, ErrorReport::SEND_ERROR_REPORT,
                "StarterReset: Invalid target."
            );
            break;
        }
        pbf_mash_button(context, BUTTON_A, 540);
        env.log("Starter selected. Checking for shiny Poochyena.");


        int ret = run_until(
            env.console, context,
            [&](BotBaseContext& context){
                //Wait for battle to start and for Pooch battle cry
                pbf_wait(context, POOCH_WAIT);

                context.wait_for_all_requests();

            },
            {{pooch_detector}}
        );
        pooch_detector.throw_if_no_sound();
        if (ret == 0){
            env.log("Shiny Poochyena detected!");
            stats.poochyena++;
            send_program_status_notification(env, NOTIFICATION_SHINY_POOCH, "Shiny Poochyena found.");
        }
        else {
            env.log("Poochyena is not shiny.");
        }

        ShinySoundDetector starter_detector(env.console, [&](float error_coefficient) -> bool{
            return true;
        });

        //Press A to send out your selected starter
        env.log("Sending out selected starter.");
        pbf_press_button(context, BUTTON_A, 40, 40);

        int ret2 = run_until(
            env.console, context,
            [&](BotBaseContext& context){
                env.log("Wait for starter to come out.");
                pbf_wait(context, STARTER_WAIT);
                context.wait_for_all_requests();
            },
            {{starter_detector}}
        );
        starter_detector.throw_if_no_sound();
        if (ret2 == 0){
            env.log("Shiny starter detected!");
            stats.shinystarter++;

            send_program_status_notification(env, NOTIFICATION_SHINY_STARTER, "Shiny starter found!");

            shiny_starter = true;
            break;
        }
        else {
            env.log("Starter is not shiny.");
        }

        env.log("Soft resetting.");
        send_program_status_notification(
            env, NOTIFICATION_STATUS_UPDATE,
            "Soft resetting."
        );
        soft_reset(env.program_info(), env.console, context);
        stats.resets++;
    }

    //TODO: if system set to nintendo switch, have go home when done option

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}

