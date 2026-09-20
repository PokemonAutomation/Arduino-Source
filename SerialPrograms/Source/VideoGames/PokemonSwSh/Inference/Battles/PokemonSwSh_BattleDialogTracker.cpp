/*  Encounter Dialog Tracker
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "PokemonSwSh_BattleDialogTracker.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


EncounterDialogTracker::EncounterDialogTracker(
    Logger& logger,
    StaticScreenDetector& dialog_detector
)
    : VisualInferenceCallback("EncounterDialogTracker")
    , m_logger(logger)
    , m_dialog_detector(dialog_detector)
    , m_end_dialog(current_time())
    , m_dialog_on(false)
    , m_state(EncounterState::BEFORE_ANYTHING)
    , m_wild_animation_duration(0)
    , m_your_animation_duration(0)
{}

void EncounterDialogTracker::make_overlays(VideoOverlaySet& items) const{
    m_dialog_detector.make_overlays(items);
}
bool EncounterDialogTracker::process_frame(const ImageViewRGB32& screen, WallClock timestamp){
    bool dialog_on = m_dialog_detector.detect(screen);
//    cout << dialog_on << endl;
    if (dialog_on == m_dialog_on.load(std::memory_order_relaxed)){
        return false;
    }
    m_dialog_on.store(dialog_on, std::memory_order_release);

    if (!dialog_on){
        m_end_dialog = timestamp;
        m_logger.log("DialogTracker: Dialog on -> off. Starting timer.", COLOR_PURPLE);
        return false;
    }

    std::chrono::milliseconds gap_duration = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - m_end_dialog);
    m_logger.log(
        "DialogTracker: Dialog off -> on. " +
        tostr_default(gap_duration.count() / 1000.) + " seconds",
        COLOR_PURPLE
    );

//    static int c = 0;
//    screen.save("test-" + std::to_string(c++) + ".png");

    EncounterState state = m_state.load(std::memory_order_relaxed);
    switch (state){
    case EncounterState::BEFORE_ANYTHING:
        m_state.store((EncounterState)((size_t)state + 1), std::memory_order_release);
        m_logger.log("DialogTracker: Starting wild animation.", COLOR_PURPLE);
        break;
    case EncounterState::WILD_ANIMATION:
        m_wild_animation_duration = gap_duration;
        m_state.store((EncounterState)((size_t)state + 1), std::memory_order_release);
        m_logger.log("DialogTracker: Starting your animation.", COLOR_PURPLE);
        break;
    case EncounterState::YOUR_ANIMATION:
        m_your_animation_duration = gap_duration;
        m_state.store((EncounterState)((size_t)state + 1), std::memory_order_release);
        m_logger.log("DialogTracker: Starting post-entry.", COLOR_PURPLE);
        break;
    case EncounterState::POST_ENTRY:
        m_logger.log("DialogTracker: Starting post-entry.", COLOR_PURPLE);
        break;
    }
    return false;
}
void EncounterDialogTracker::push_end(WallClock timestamp){
    std::chrono::milliseconds gap_duration = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp - m_end_dialog);
    m_logger.log(
        "DialogTracker: End " +
        tostr_default(gap_duration.count() / 1000.) + " seconds",
        COLOR_PURPLE
    );
    EncounterState state = m_state.load(std::memory_order_relaxed);
    switch (state){
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
