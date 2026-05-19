/*  BDSP AutoStory - Overworld Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonBDSP_AutoStory_OverworldDetector.h"

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


}
}
}
