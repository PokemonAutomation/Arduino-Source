/*  Shiny Dialog Tracker
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonSwSh/Inference/PokemonSwSh_StartBattleDetector.h"
#include "PokemonSwSh_ShinyDialogTracker.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


ShinyDialogTracker::ShinyDialogTracker(VideoFeed& feed, Logger& logger)
    : m_logger(logger)
    , m_dialog(feed)
    , m_end_dialog(std::chrono::system_clock::now())
    , m_dialog_on(false)
    , m_state(EncounterState::BEFORE_ANYTHING)
    , m_wild_animation_duration(0)
    , m_your_animation_duration(0)
{}


void ShinyDialogTracker::push_frame(const QImage& screen, std::chrono::system_clock::time_point timestamp){
    bool dialog_on = m_dialog.detect(screen);
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



}
}
}
