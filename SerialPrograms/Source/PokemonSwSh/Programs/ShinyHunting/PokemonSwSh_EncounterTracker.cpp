/*  Encounter Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/SwitchFramework/Switch_PushButtons.h"
#include "PokemonSwSh_EncounterTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


StandardEncounterTracker::StandardEncounterTracker(
    ShinyHuntTracker& stats,
    ConsoleHandle& console,
    bool require_square,
    uint16_t exit_battle_time,
    bool take_video,
    bool run_from_everything
)
    : m_stats(stats)
    , m_console(console)
    , m_require_square(require_square)
    , m_exit_battle_time(exit_battle_time)
    , m_take_video(take_video)
    , m_run_from_everything(run_from_everything)
{}

bool StandardEncounterTracker::run_away(){
    pbf_press_dpad(DPAD_UP, 10, 10);
    pbf_press_button(BUTTON_A, 10, 10);
    pbf_mash_button(BUTTON_B, m_exit_battle_time);
    return true;
}

void StandardEncounterTracker::take_video(){
    if (m_take_video){
        pbf_wait(m_console, 5 * TICKS_PER_SECOND);
        pbf_press_button(m_console, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
    }
}

bool StandardEncounterTracker::process_result(ShinyDetection detection){
    bool stop = false;
    switch (detection){
    case ShinyDetection::NO_BATTLE_MENU:
        return false;

    case ShinyDetection::NOT_SHINY:
        m_stats.add_non_shiny();
        break;

    case ShinyDetection::STAR_SHINY:
        m_stats.add_star_shiny();
        take_video();
        stop = !m_require_square;
        break;

    case ShinyDetection::SQUARE_SHINY:
        m_stats.add_square_shiny();
        take_video();
        stop = true;
        break;

    case ShinyDetection::UNKNOWN_SHINY:
        m_stats.add_unknown_shiny();
        take_video();
        stop = true;
        break;

    }

    if (m_run_from_everything){
        stop = false;
    }

    if (!stop){
        run_away();
    }

    return stop;
}


}
}
}

