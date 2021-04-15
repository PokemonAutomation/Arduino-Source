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
    uint16_t exit_battle_time
)
    : m_stats(stats)
    , m_console(console)
    , m_require_square(require_square)
    , m_exit_battle_time(exit_battle_time)
{}

bool StandardEncounterTracker::run_away(){
    pbf_press_dpad(DPAD_UP, 10, 10);
    pbf_press_button(BUTTON_A, 10, 10);
    pbf_mash_button(BUTTON_B, m_exit_battle_time);
    return true;
}

bool StandardEncounterTracker::process_result(ShinyDetection detection){
    switch (detection){
    case ShinyDetection::NO_BATTLE_MENU:
        return false;
    case ShinyDetection::NOT_SHINY:
        m_stats.add_non_shiny();
        run_away();
        return false;
    case ShinyDetection::STAR_SHINY:
        m_stats.add_star_shiny();
        pbf_wait(m_console, 5 * TICKS_PER_SECOND);
        pbf_press_button(m_console, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
        if (m_require_square){
            run_away();
            return false;
        }
        return true;
    case ShinyDetection::SQUARE_SHINY:
        m_stats.add_square_shiny();
        pbf_wait(m_console, 5 * TICKS_PER_SECOND);
        pbf_press_button(m_console, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
        return true;
    case ShinyDetection::UNKNOWN_SHINY:
        m_stats.add_unknown_shiny();
        pbf_wait(m_console, 5 * TICKS_PER_SECOND);
        pbf_press_button(m_console, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 5 * TICKS_PER_SECOND);
        return true;
    }
    return false;
}


}
}
}

