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
        "Soft reset for a shiny starter.",
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
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        // &NOTIFICATION_ERROR_FATAL,
        })
{
    PA_ADD_OPTION(TARGET);
    PA_ADD_OPTION(NOTIFICATIONS);
}

void StarterReset::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    //assert_16_9_720p_min(env.logger(), env.console);
    StarterReset_Descriptor::Stats& stats = env.current_stats<StarterReset_Descriptor::Stats>();

    /*
    * Stand in front of birch's bag.
    * 
    * text speed fast
    * 
    * have to do the SR method instead of run away
    * 
    * ONLY FOR RS, emerald has rng anyway
    * 
    * This assumes no dry battery. If you have a dry battery, just do RNG.
    */

    /*
    start at birch bag

    starter selection

    wild pooch appears, shiny check (audio?)

    go starter

    now shiny check

    track starter shiny, zig shiny, number of attempts, errors

    if not shiny, soft reset

    soft reset checks for dry battery and returns true or false!
    
    */

    bool shiny_starter = false;
    while (!shiny_starter) {

        float shiny_coefficient = 1.0;
        ShinySoundDetector pooch_detector(env.console, [&](float error_coefficient) -> bool{
            //  Warning: This callback will be run from a different thread than this function.
            shiny_coefficient = error_coefficient;
            return true;
        });

        int ret = run_until(
            env.console, context,
            [&](BotBaseContext& context){
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

                //Wait for battle to start and for Pooch battle cry
                pbf_wait(context, 400);

                context.wait_for_all_requests();

            },
            {{pooch_detector}}
        );
        pooch_detector.throw_if_no_sound();
        if (ret == 0){
            env.log("Shiny Poochyena detected!");
            stats.poochyena++;
        }

        ShinySoundDetector starter_detector(env.console, [&](float error_coefficient) -> bool{
            //  Warning: This callback will be run from a different thread than this function.
            shiny_coefficient = error_coefficient;
            return true;
        });

        int ret2 = run_until(
            env.console, context,
            [&](BotBaseContext& context){
                env.log("Sending out selected starter.");
                //Press A to send out your selected starter
                pbf_press_button(context, BUTTON_A, 40, 400);
            },
            {{starter_detector}}
        );
        starter_detector.throw_if_no_sound();
        if (ret2 == 0){
            env.log("Shiny starter detected!");
            stats.shinystarter++;
            shiny_starter = true;
            break;

        }

        env.log("Soft resetting.");
        soft_reset(env.program_info(), env.console, context);
        stats.resets++;
    }

    //TODO: if system set to nintendo switch, have go home when done option

    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}

}
}
}

