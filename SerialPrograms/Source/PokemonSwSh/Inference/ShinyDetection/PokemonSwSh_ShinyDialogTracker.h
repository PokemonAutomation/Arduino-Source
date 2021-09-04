/*  Shiny Dialog Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_ShinyDialogTracker_H
#define PokemonAutomation_PokemonSwSh_ShinyDialogTracker_H

#include <chrono>
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_BattleDialogDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

enum class EncounterState{
    BEFORE_ANYTHING,
    WILD_ANIMATION,
    YOUR_ANIMATION,
    POST_ENTRY,
};


class ShinyDialogTracker{
public:
    ShinyDialogTracker(VideoOverlay& overlay, Logger& logger);

    EncounterState encounter_state() const{ return m_state; }
    std::chrono::milliseconds wild_animation_duration() const{ return m_wild_animation_duration; }
    std::chrono::milliseconds your_animation_duration() const{ return m_your_animation_duration; }

    void push_frame(
        const QImage& screen,
        std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now()
    );

private:
    Logger& m_logger;
    BattleDialogDetector m_dialog;
    std::chrono::system_clock::time_point m_end_dialog;
    bool m_dialog_on;

    EncounterState m_state;
    std::chrono::milliseconds m_wild_animation_duration;
    std::chrono::milliseconds m_your_animation_duration;
};


}
}
}
#endif
