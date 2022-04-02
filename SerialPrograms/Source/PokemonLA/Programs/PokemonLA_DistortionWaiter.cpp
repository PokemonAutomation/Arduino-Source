/*  Distortion Waiter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/InferenceInfra/InferenceRoutines.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/PokemonLA_NotificationReader.h"
#include "PokemonLA_DistortionWaiter.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


DistortionWaiter_Descriptor::DistortionWaiter_Descriptor()
    : RunnableSwitchProgramDescriptor(
        "PokemonLA:DistortionWaiter",
        STRING_POKEMON + " LA", "Distortion Waiter",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/DistortionWaiter.md",
        "Wait for a distortion to appear.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}


DistortionWaiter::DistortionWaiter(const DistortionWaiter_Descriptor& descriptor)
    : SingleSwitchProgramInstance(descriptor)
    , LANGUAGE("<b>Game Language:</b>", Pokemon::PokemonNameReader::instance().languages(), true)
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




class DistortionWaiter::Stats : public StatsTracker{
public:
    Stats()
        : minutes_waited(m_stats["Minutes Waited"])
        , distortions(m_stats["Distortions"])
        , other(m_stats["Other Notifications"])
        , errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Minutes Waited");
        m_display_order.emplace_back("Distortions");
        m_display_order.emplace_back("Other Notifications", true);
        m_display_order.emplace_back("Errors", true);
    }

    std::atomic<uint64_t>& minutes_waited;
    std::atomic<uint64_t>& distortions;
    std::atomic<uint64_t>& other;
    std::atomic<uint64_t>& errors;
};




std::unique_ptr<StatsTracker> DistortionWaiter::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}


void DistortionWaiter::program(SingleSwitchProgramEnvironment& env, const BotBaseContext& context){
    Stats& stats = env.stats<Stats>();


    //  Connect the controller.
    pbf_press_button(env.console, BUTTON_LCLICK, 5, 5);


    NotificationDetector detector(env.console, LANGUAGE);
//    AsyncVisualInferenceSession visual(env, console, console, console);
//    visual += detector;

    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
    while (true){
        env.update_stats();

        int ret = run_until(
            env, context, env.console,
            [&](const BotBaseContext& context){
                for (size_t c = 0; c < 60; c++){
                    pbf_press_button(context, BUTTON_LCLICK, 20, 60 * TICKS_PER_SECOND - 20);
                    context.wait_for_all_requests();
                    auto elapsed = std::chrono::system_clock::now() - start;
                    uint64_t minutes = std::chrono::duration_cast<std::chrono::minutes>(elapsed).count();
                    if (minutes != stats.minutes_waited.load(std::memory_order_relaxed)){
                        stats.minutes_waited.store(minutes, std::memory_order_relaxed);
                        env.update_stats();
                    }
                }
            },
            { &detector }
        );
        if (ret < 0){
            stats.errors++;
            throw OperationFailedException(env.console, "No distortion found after one hour.");
        }

        auto elapsed = std::chrono::system_clock::now() - start;
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

        env.wait_for(std::chrono::seconds(10));
    }


    env.update_stats();

    send_program_notification(
        env.console, NOTIFICATION_DISTORTION,
        COLOR_GREEN,
        env.program_info(),
        "Found Distortion",
        {{"Session Stats", QString::fromStdString(stats.to_str())}},
        env.console.video().snapshot()
    );
    pbf_press_button(env.console, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
}



}
}
}
