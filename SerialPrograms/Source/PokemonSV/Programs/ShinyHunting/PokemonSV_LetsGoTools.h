/*  Let's Go Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_LetsGoTools_H
#define PokemonAutomation_PokemonSV_LetsGoTools_H

#include <deque>
#include "Common/Cpp/Time.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_LetsGoKillDetector.h"

namespace PokemonAutomation{
    class BotBaseContext;
namespace NintendoSwitch{
namespace PokemonSV{



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
    std::deque<WallClock> m_kill_history;
    std::deque<WallClock> m_encounter_history;
};


//  Send your Pokemon out in front in Let's Go. Then run the specified command.
//  Don't return until both the command has finished, and it appears the kill
//  chain has ended.
bool clear_in_front(
    ConsoleHandle& console, BotBaseContext& context,
    LetsGoKillSoundDetector& kill_tracker,
    bool throw_ball_if_bubble,
    std::function<void(BotBaseContext& context)>&& command
);



}
}
}
#endif
