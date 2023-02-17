/*  Let's Go Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_LetsGoTools_H
#define PokemonAutomation_PokemonSV_LetsGoTools_H

#include <deque>
#include <atomic>
#include "Common/Cpp/Time.h"
//#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Options/LanguageOCROption.h"
//#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/InferenceInfra/InferenceSession.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "Pokemon/Pokemon_EncounterStats.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_LetsGoKillDetector.h"
#include "PokemonSV/Inference/Battles/PokemonSV_EncounterWatcher.h"

namespace PokemonAutomation{
    class CancellableScope;
    class BotBaseContext;
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;

class EncounterBotCommonOptions;



class EncounterRateTracker{
public:
    EncounterRateTracker();

    //  Get the # of encounters in the last specified time window.
    //  Returns false if the history is less than the window.
    bool get_encounters_in_window(
        size_t& kills, size_t& encounters,
        std::chrono::seconds time_window
    );

    void report_start(WallClock now = current_time());
    void report_kill();
    void report_encounter();

private:
    WallClock m_start_time;

    //  Note that this keeps the entire history. Nothing is dropped. This is
    //  fine since it's unlikely we'll have enough encounters in one run to
    //  cause memory issues.
    std::deque<WallClock> m_kill_history;
    std::deque<WallClock> m_encounter_history;
};



//
//  Consider a virtual timeline that is starts and stops relative to the wall clock.
//  Now you want to get the last X seconds of the virtual timeline, but on the
//  wall clock instead. Because of the starts and stops, the last X seconds
//  of virtual time will be less than the last X seconds of real time.
//
//  The use case here is the platform reset conditions. Resets are performed if
//  the # of encounters in the last X time drops below Y. But only the time
//  spent hunting for encounters should count. Time spent in battles or resets
//  does not.
//
class DiscontiguousTimeTracker{
public:
    //  Returns zero if there is insufficient virtual time.
    WallDuration last_window_in_realtime(
        WallClock realtime_end,
        WallDuration last_window_in_virtual_time
    );

    void add_block(WallClock start, WallClock end);

private:
    std::deque<std::pair<WallClock, WallClock>> m_blocks;
};





class LetsGoEncounterBotStats : public StatsTracker{
public:
    LetsGoEncounterBotStats()
        : m_kills(m_stats["Kills"])
        , m_encounters(m_stats["Encounters"])
        , m_shinies(m_stats["Shinies"])
    {
        m_display_order.emplace_back("Kills");
        m_display_order.emplace_back("Encounters");
        m_display_order.emplace_back("Shinies");
    }

public:
    std::atomic<uint64_t>& m_kills;
    std::atomic<uint64_t>& m_encounters;
    std::atomic<uint64_t>& m_shinies;
};






class LetsGoEncounterBotTracker{
public:
    LetsGoEncounterBotTracker(
        ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
        LetsGoEncounterBotStats& stats,
        OCR::LanguageOCROption& language
    );

    WallClock last_kill() const{
        return m_kill_sound.last_kill();
    }
    const EncounterFrequencies& encounter_frequencies() const{
        return m_encounter_frequencies;
    }

    //  Get the # of encounters in the last specified time window.
    //  Returns false if the history is less than the window.
    bool get_encounters_in_window(
        size_t& kills, size_t& encounters,
        std::chrono::seconds time_window
    ){
        return m_encounter_rate.get_encounters_in_window(kills, encounters, time_window);
    }
    void reset_rate_tracker_start_time(){
        m_encounter_rate.report_start();
    }

    //  Returns true if you should save the game.
    bool process_battle(EncounterWatcher& watcher, EncounterBotCommonOptions& settings);


private:
    ProgramEnvironment& m_env;
    ConsoleHandle& m_console;
    BotBaseContext& m_context;
    LetsGoEncounterBotStats& m_stats;

    OCR::LanguageOCROption& m_language;

    EncounterRateTracker m_encounter_rate;
    EncounterFrequencies m_encounter_frequencies;

    LetsGoKillSoundDetector m_kill_sound;
    InferenceSession m_session;
};




//  Send your Pokemon out in front in Let's Go. Then run the specified command.
//  Don't return until both the command has finished, and it appears the kill
//  chain has ended.
bool clear_in_front(
    ConsoleHandle& console, BotBaseContext& context,
    LetsGoEncounterBotTracker& tracker,
    bool throw_ball_if_bubble,
    std::function<void(BotBaseContext& context)>&& command
);




}
}
}
#endif
