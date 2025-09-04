/*  Distortion Waiter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/ProgramStats/StatsTracking.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/PokemonLA_NotificationReader.h"
#include "PokemonLA_DistortionWaiter.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;


DistortionWaiter_Descriptor::DistortionWaiter_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonLA:DistortionWaiter",
        STRING_POKEMON + " LA", "Distortion Waiter",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/DistortionWaiter.md",
        "Wait for a distortion to appear.",
        ProgramControllerClass::StandardController_NoRestrictions,
        FeedbackType::REQUIRED,
        AllowCommandsWhenRunning::DISABLE_COMMANDS
    )
{}
class DistortionWaiter_Descriptor::Stats : public StatsTracker{
public:
    Stats()
        : minutes_waited(m_stats["Minutes Waited"])
        , distortions(m_stats["Distortions"])
        , other(m_stats["Other Notifications"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Minutes Waited");
        m_display_order.emplace_back("Distortions");
        m_display_order.emplace_back("Other Notifications", HIDDEN_IF_ZERO);
        m_display_order.emplace_back("Errors", HIDDEN_IF_ZERO);
    }

    std::atomic<uint64_t>& minutes_waited;
    std::atomic<uint64_t>& distortions;
    std::atomic<uint64_t>& other;
    std::atomic<uint64_t>& errors;
};
std::unique_ptr<StatsTracker> DistortionWaiter_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


DistortionWaiter::DistortionWaiter()
    : LANGUAGE(
        "<b>Game Language:</b>",
        Pokemon::PokemonNameReader::instance().languages(),
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , NOTIFICATION_DISTORTION(
        "Distortion Appeared",
        true, true, ImageAttachmentMode::JPG,
        {"Notifs"}
    )
    , NOTIFICATIONS({
        &NOTIFICATION_DISTORTION,
        &NOTIFICATION_ERROR_RECOVERABLE,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(NOTIFICATIONS);
}



void DistortionWaiter::program(SingleSwitchProgramEnvironment& env, ProControllerContext& context){
    DistortionWaiter_Descriptor::Stats& stats = env.current_stats<DistortionWaiter_Descriptor::Stats>();


    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 5, 5);


    NotificationDetector detector(env.console, LANGUAGE);

    WallClock start = current_time();
    while (true){
        env.update_stats();

        int ret = run_until<ProControllerContext>(
            env.console, context,
            [&](ProControllerContext& context){
                for (size_t c = 0; c < 60; c++){
                    pbf_press_button(context, BUTTON_LCLICK, 20, 60 * TICKS_PER_SECOND - 20);
                    context.wait_for_all_requests();
                    auto elapsed = current_time() - start;
                    uint64_t minutes = std::chrono::duration_cast<std::chrono::minutes>(elapsed).count();
                    if (minutes != stats.minutes_waited.load(std::memory_order_relaxed)){
                        stats.minutes_waited.store(minutes, std::memory_order_relaxed);
                        env.update_stats();
                    }
                }
            },
            {{detector}}
        );
        if (ret < 0){
            stats.errors++;
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "No distortion found after one hour.",
                env.console
            );
        }

        auto elapsed = current_time() - start;
        uint64_t minutes = std::chrono::duration_cast<std::chrono::minutes>(elapsed).count();
        stats.minutes_waited.store(minutes, std::memory_order_relaxed);

        if (detector.result() == Notification::DISTORTION_FORMING){
            stats.distortions++;
            break;
        }
        if (detector.result() == Notification::ERROR){
            stats.errors++;
        }else{
            stats.other++;
        }

        context.wait_for(std::chrono::seconds(10));
    }


    env.update_stats();

    send_program_notification(
        env, NOTIFICATION_DISTORTION,
        COLOR_GREEN,
        "Found Distortion",
        {}, "",
        env.console.video().snapshot()
    );
    pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY0);
}



}
}
}
