/*  Egg Fetcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Inference/PokemonSV_DialogDetector.h"
#include "PokemonSV_EggFetcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


EggFetcher_Descriptor::EggFetcher_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:EggFetcher",
        STRING_POKEMON + " BDSP", "Egg Fetcher",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/EggFetcher.md",
        "Automatically fetch eggs from a picnic",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct EggFetcher_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_attempts(m_stats["Fetch Attempts"])
        , m_eggs(m_stats["Eggs Received"])
        , m_errors(m_stats["Errors"])
    {
        m_display_order.emplace_back("Fetch Attempts");
        m_display_order.emplace_back("Eggs Received");
        m_display_order.emplace_back("Errors", true);
    }
    std::atomic<uint64_t>& m_attempts;
    std::atomic<uint64_t>& m_eggs;
    std::atomic<uint64_t>& m_errors;
};
std::unique_ptr<StatsTracker> EggFetcher_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



EggFetcher::EggFetcher()
    : GO_HOME_WHEN_DONE(false)
    , EGGS_TO_FETCH(
        "<b>Fetch this many eggs:</b>",
        LockWhileRunning::LOCKED,
        600
    )
    , FETCH_PERIOD(
        "<b>Fetch Period:</b><br>Wait this long before talking to the basket again.",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        "60 * TICKS_PER_SECOND"
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(EGGS_TO_FETCH);
    PA_ADD_OPTION(FETCH_PERIOD);
    PA_ADD_OPTION(NOTIFICATIONS);
}


size_t EggFetcher::fetch_eggs(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
    EggFetcher_Descriptor::Stats& stats = env.current_stats<EggFetcher_Descriptor::Stats>();

    VideoOverlaySet overlays(env.console.overlay());
    DialogDetector detector;
    detector.make_overlays(overlays);

    {
        pbf_press_button(context, BUTTON_A, 20, 180);
        context.wait_for_all_requests();

        VideoSnapshot screen = env.console.video().snapshot();
        DialogDetector::DialogType detection = detector.detect_with_type(screen);
        if (detection == DialogDetector::NO_DIALOG){
            stats.m_errors++;
            return 0;
        }
        if (detection == DialogDetector::PROMPT_DIALOG){
            stats.m_errors++;
            pbf_press_button(context, BUTTON_B, 20, 230);
            return 0;
        }
    }

    size_t fetched = 0;
    while (true){
        pbf_press_button(context, BUTTON_A, 20, 180);
        context.wait_for_all_requests();

        VideoSnapshot screen = env.console.video().snapshot();
        DialogDetector::DialogType detection = detector.detect_with_type(screen);
        switch (detection){
        case DialogDetector::NO_DIALOG:
            stats.m_attempts++;
            return fetched;
        case DialogDetector::ADVANCE_DIALOG:
            continue;
        case DialogDetector::PROMPT_DIALOG:
            stats.m_eggs++;
            fetched++;
            continue;
        }
    }
}

void EggFetcher::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
//    EggFetcher_Descriptor::Stats& stats = env.current_stats<EggFetcher_Descriptor::Stats>();
//    env.update_stats();

    //  Connect the controller.
    pbf_press_button(context, BUTTON_LCLICK, 10, 0);

    for (size_t fetched = 0; fetched < EGGS_TO_FETCH;){
        env.update_stats();
        send_program_status_notification(env, NOTIFICATION_STATUS_UPDATE);

        fetched += fetch_eggs(env, context);

#if 0
        for (size_t c = 0; c < 4; c++){
            pbf_press_button(context, BUTTON_PLUS, 20, 5 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_R, 20, 5 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_ZR, 20, 5 * TICKS_PER_SECOND);
            pbf_press_button(context, BUTTON_PLUS, 20, 5 * TICKS_PER_SECOND);
        }
#endif

        context.wait_for(std::chrono::milliseconds((uint64_t)FETCH_PERIOD * 1000 / TICKS_PER_SECOND));
        context.wait_for_all_requests();
    }

    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
}






}
}
}
