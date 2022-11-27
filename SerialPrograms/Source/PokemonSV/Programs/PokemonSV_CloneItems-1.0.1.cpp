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
#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSV/Inference/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_MainMenuDetector.h"
#include "PokemonSV/Programs/PokemonSV_Navigation.h"
#include "PokemonSV/Programs/PokemonSV_BasicCatcher.h"
#include "PokemonSV/Programs/TeraRaids/PokemonSV_TeraBattler.h"
#include "PokemonSV_CloneItems-1.0.1.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;


CloneItems101_Descriptor::CloneItems101_Descriptor()
    : SingleSwitchProgramDescriptor(
        "PokemonSV:CloneItems1.0.1",
        STRING_POKEMON + " SV", "Clone Items (1.0.1)",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/CloneItems101.md",
        "Clone items using the ride legendary glitch.",
        FeedbackType::REQUIRED, false,
        PABotBaseLevel::PABOTBASE_12KB
    )
{}
struct CloneItems101_Descriptor::Stats : public StatsTracker{
    Stats()
        : m_skips(m_stats["Date Skips"])
//        , m_raids(m_stats["Raids"])
        , m_wins(m_stats["Wins"])
        , m_losses(m_stats["Losses"])
        , m_skipped(m_stats["Skipped"])
        , m_errors(m_stats["Errors"])
        , m_caught(m_stats["Caught"])
        , m_shinies(m_stats["Shinies"])
        , m_cloned(m_stats["Cloned"])
        , m_failed(m_stats["Failed"])
    {
        m_display_order.emplace_back("Date Skips");
//        m_display_order.emplace_back("Raids");
        m_display_order.emplace_back("Wins");
        m_display_order.emplace_back("Losses");
        m_display_order.emplace_back("Skipped");
        m_display_order.emplace_back("Errors", true);
        m_display_order.emplace_back("Caught", true);
        m_display_order.emplace_back("Shinies", true);
        m_display_order.emplace_back("Cloned");
        m_display_order.emplace_back("Failed", true);
    }
    std::atomic<uint64_t>& m_skips;
//    std::atomic<uint64_t>& m_raids;
    std::atomic<uint64_t>& m_wins;
    std::atomic<uint64_t>& m_losses;
    std::atomic<uint64_t>& m_skipped;
    std::atomic<uint64_t>& m_errors;
    std::atomic<uint64_t>& m_caught;
    std::atomic<uint64_t>& m_shinies;
    std::atomic<uint64_t>& m_cloned;
    std::atomic<uint64_t>& m_failed;
};
std::unique_ptr<StatsTracker> CloneItems101_Descriptor::make_stats() const{
    return std::unique_ptr<StatsTracker>(new Stats());
}



CloneItems101::CloneItems101()
    : GO_HOME_WHEN_DONE(false)
    , LANGUAGE(
        "<b>Game Language:</b>",
        PokemonNameReader::instance().languages(),
        LockWhileRunning::UNLOCKED
    )
    , ITEMS_TO_CLONE(
        "<b>Items to Clone:</b><br>Stop program after cloning this many items.",
        LockWhileRunning::UNLOCKED,
        999, 1, 999
    )
    , MAX_STARS(
        "<b>Max Stars:</b><br>Skip raids with more than this many stars to save time since you're likely to lose.",
        LockWhileRunning::UNLOCKED,
        4, 1, 7
    )
    , BALL_SELECT(
        "<b>Ball Select:</b>",
        LockWhileRunning::UNLOCKED,
        "poke-ball"
    )
    , FIX_TIME_ON_CATCH(
        "<b>Fix Clock on Catch:</b><br>Fix the time when catching so the caught date will be correct.",
        LockWhileRunning::UNLOCKED, true
    )
    , A_TO_B_DELAY(
        "<b>A-to-B Delay:</b><br>The delay between the critical A-to-B press that activates the glitch.",
        LockWhileRunning::UNLOCKED,
        TICKS_PER_SECOND,
        "50"
    )
    , NOTIFICATION_STATUS_UPDATE("Status Update", true, false, std::chrono::seconds(3600))
    , NOTIFICATIONS({
        &NOTIFICATION_STATUS_UPDATE,
        &NOTIFICATION_PROGRAM_FINISH,
        &NOTIFICATION_ERROR_FATAL,
    })
{
    PA_ADD_OPTION(GO_HOME_WHEN_DONE);
    PA_ADD_OPTION(LANGUAGE);
    PA_ADD_OPTION(ITEMS_TO_CLONE);
    PA_ADD_OPTION(MAX_STARS);
    PA_ADD_OPTION(BALL_SELECT);
    PA_ADD_OPTION(FIX_TIME_ON_CATCH);
    PA_ADD_OPTION(A_TO_B_DELAY);
    PA_ADD_OPTION(NOTIFICATIONS);
}


void CloneItems101::program(SingleSwitchProgramEnvironment& env, BotBaseContext& context){
//    CloneItems101_Descriptor::Stats& stats = env.current_stats<CloneItems101_Descriptor::Stats>();


    MainMenuDetector menu_detector;
    menu_detector.move_cursor(env.console, context, MenuSide::LEFT, 5);





    env.update_stats();
    send_program_finished_notification(env, NOTIFICATION_PROGRAM_FINISH);
    GO_HOME_WHEN_DONE.run_end_of_program(context);
}






}
}
}
