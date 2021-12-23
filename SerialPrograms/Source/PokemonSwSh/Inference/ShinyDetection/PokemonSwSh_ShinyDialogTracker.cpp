/*  Shiny Dialog Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonSwSh/Inference/Battles/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh_ShinyDialogTracker.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyDialogTracker::ShinyDialogTracker(
    VideoOverlay& overlay, Logger& logger,
    StaticScreenDetector& detector
)
    : m_logger(logger)
    , m_detector(detector)
    , m_overlays(overlay)
    , m_end_dialog(std::chrono::system_clock::now())
    , m_dialog_on(false)
    , m_state(EncounterState::BEFORE_ANYTHING)
    , m_wild_animation_duration(0)
    , m_your_animation_duration(0)
{
    detector.make_overlays(m_overlays);
}


void ShinyDialogTracker::push_frame(const QImage& screen, std::chrono::system_clock::time_point timestamp){
    bool dialog_on = m_detector.detect(screen);
//    cout << dialog_on << endl;
    if (dialog_on == m_dialog_on){
        return;
    }
    m_dialog_on = dialog_on;

    if (!dialog_on){
        m_end_dialog = timestamp;
        m_logger.log("DialogTracker: Dialog on -> off. Starting timer.", "purple");
        return;
    }

    std::chrono::milliseconds gap_duration = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - m_end_dialog);
    m_logger.log(
        "DialogTracker: Dialog off -> on. " +
        QString::number(gap_duration.count() / 1000.) + " seconds",
        "purple"
    );

    switch (m_state){
    case EncounterState::BEFORE_ANYTHING:
        m_state = (EncounterState)((size_t)m_state + 1);
        m_logger.log("DialogTracker: Starting wild animation.", "purple");
        break;
    case EncounterState::WILD_ANIMATION:
        m_state = (EncounterState)((size_t)m_state + 1);
        m_logger.log("DialogTracker: Starting your animation.", "purple");
        m_wild_animation_duration = gap_duration;
        break;
    case EncounterState::YOUR_ANIMATION:
        m_state = (EncounterState)((size_t)m_state + 1);
        m_logger.log("DialogTracker: Starting post-entry.", "purple");
        m_your_animation_duration = gap_duration;
        break;
    case EncounterState::POST_ENTRY:
        m_logger.log("DialogTracker: Starting post-entry.", "purple");
        break;
    }
}
void ShinyDialogTracker::push_end(std::chrono::system_clock::time_point timestamp){
    std::chrono::milliseconds gap_duration = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - m_end_dialog);
    m_logger.log(
        "DialogTracker: End " +
        QString::number(gap_duration.count() / 1000.) + " seconds",
        "purple"
    );
    switch (m_state){
    case EncounterState::BEFORE_ANYTHING:
        break;
    case EncounterState::WILD_ANIMATION:
        m_wild_animation_duration = gap_duration;
        break;
    case EncounterState::YOUR_ANIMATION:
        m_your_animation_duration = gap_duration;
        break;
    case EncounterState::POST_ENTRY:
        break;
    }
}



}
}
}
