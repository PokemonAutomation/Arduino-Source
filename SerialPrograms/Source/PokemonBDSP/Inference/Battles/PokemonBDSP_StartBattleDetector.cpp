/*  Start Battle Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "PokemonBDSP_StartBattleDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{





StartBattleDetector::StartBattleDetector(VideoOverlay& overlay)
    : VisualInferenceCallback("StartBattleDetector")
    , m_screen_box(0.2, 0.2, 0.6, 0.6)
{}
void StartBattleDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_screen_box);
    m_dialog.make_overlays(items);
}
bool StartBattleDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    return detect(frame);
}

bool StartBattleDetector::detect(const ImageViewRGB32& frame){
//    ImageViewRGB32 image = extract_box_reference(frame, m_screen_box);

//    ImageStats stats = image_stats(image);

//    //  Solid screen that's not black.
//    if (stats.average.sum() > 50 && stats.stddev.sum() < 10){
//        return true;
//    }

    return m_dialog.detect(frame);
}


StartBattleMenuOverlapDetector::StartBattleMenuOverlapDetector(VideoOverlay& overlay)
    : VisualInferenceCallback("StartBattleMenuOverlapDetector")
    , m_left(0.02, 0.2, 0.08, 0.5)
    , m_right(0.90, 0.2, 0.08, 0.5)
    , m_battle_detected(false)
{}
void StartBattleMenuOverlapDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_left);
    items.add(COLOR_RED, m_right);
}
bool StartBattleMenuOverlapDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    if (detect(frame)){
        m_battle_detected.store(true, std::memory_order_release);
        return true;
    }
    return false;
}
bool StartBattleMenuOverlapDetector::detect(const ImageViewRGB32& frame){
    ImageViewRGB32 image0 = extract_box_reference(frame, m_left);
    ImageStats stats0 = image_stats(image0);
    //  Solid screen that's not black.
    if (stats0.average.sum() < 50 || stats0.stddev.sum() > 10){
        return false;
    }

    ImageViewRGB32 image1 = extract_box_reference(frame, m_right);
    ImageStats stats1 = image_stats(image1);
    //  Solid screen that's not black.
    if (stats1.average.sum() < 50 && stats1.stddev.sum() > 10){
        return false;
    }

    return true;
}




}
}
}
