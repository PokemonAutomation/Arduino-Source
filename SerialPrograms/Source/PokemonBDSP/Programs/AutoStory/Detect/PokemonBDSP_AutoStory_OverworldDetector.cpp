/*  BDSP AutoStory - Overworld Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonBDSP_AutoStory_OverworldDetector.h"

using namespace std::chrono_literals;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


// ---------------------------------------------------------------------------
//  OverworldDetector
// ---------------------------------------------------------------------------

OverworldDetector::OverworldDetector(Color color)
    : m_color(color)
    , m_battle_menu(BattleType::STANDARD, color)
{}

void OverworldDetector::make_overlays(VideoOverlaySet& items) const{
    m_dialog.make_overlays(items);
    m_battle_menu.make_overlays(items);
    m_menu.make_overlays(items);
}

bool OverworldDetector::detect(const ImageViewRGB32& screen){
    // We are in the overworld when none of the overlay screens are present.
    if (m_dialog.detect(screen))     return false;
    if (m_battle_menu.detect(screen)) return false;
    if (m_menu.detect(screen))        return false;
    return true;
}


// ---------------------------------------------------------------------------
//  OverworldWatcher
// ---------------------------------------------------------------------------

OverworldWatcher::OverworldWatcher(Color color)
    : OverworldDetector(color)
    , VisualInferenceCallback("OverworldWatcher")
{}

void OverworldWatcher::make_overlays(VideoOverlaySet& items) const{
    OverworldDetector::make_overlays(items);
}

bool OverworldWatcher::process_frame(const ImageViewRGB32& frame, WallClock /*timestamp*/){
    return detect(frame);
}


// ---------------------------------------------------------------------------
//  PoketchButtonDetector
// ---------------------------------------------------------------------------

PoketchButtonDetector::PoketchButtonDetector(Color color)
    : m_color(color)
    , m_button(0.968, 0.170, 0.022, 0.120)
{}

void PoketchButtonDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_button);
}

bool PoketchButtonDetector::detect(const ImageViewRGB32& screen){
    ImageStats stats = image_stats(extract_box_reference(screen, m_button));
    //  The button is a glossy red with a highlight, so allow a wide stddev.
    return is_solid(stats, {0.74, 0.10, 0.16}, 0.12, 80);
}

PoketchButtonWatcher::PoketchButtonWatcher(Color color)
    : PoketchButtonDetector(color)
    , VisualInferenceCallback("PoketchButtonWatcher")
{}

void PoketchButtonWatcher::make_overlays(VideoOverlaySet& items) const{
    PoketchButtonDetector::make_overlays(items);
}

bool PoketchButtonWatcher::process_frame(const ImageViewRGB32& frame, WallClock){
    return detect(frame);
}


static bool look_for_poketch_button(VideoStream& stream, ProControllerContext& context){
    context.wait_for_all_requests();
    PoketchButtonWatcher watcher;
    return wait_until(stream, context, 2s, {{watcher}}) == 0;
}

bool ensure_poketch_visible(VideoStream& stream, ProControllerContext& context){
    for (size_t attempt = 0; attempt < 3; attempt++){
        if (look_for_poketch_button(stream, context)){
            return true;
        }
        stream.log("[AutoStory] Poketch button not found. Attempt " + std::to_string(attempt + 1) + " of 3.", COLOR_ORANGE);

        if (attempt == 0){
            //  Most likely hidden: one tap shows it.
            pbf_press_button(context, BUTTON_R, 205ms, 0ms);
        }else{
            //  Long press hides it, then a tap shows it at the normal size.
            pbf_press_button(context, BUTTON_R, 1548ms, 0ms);
            pbf_wait(context, 1418ms);
            pbf_press_button(context, BUTTON_R, 143ms, 0ms);
        }
    }

    //  Check the result of the last press sequence.
    return look_for_poketch_button(stream, context);
}


}
}
}
