/*  Overworld Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "PokemonLA_OverworldDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



OverworldDetector::OverworldDetector(LoggerQt& logger, VideoOverlay& overlay)
    : VisualInferenceCallback("OverworldDetector")
    , m_arc_phone(logger, overlay, std::chrono::milliseconds(100), true)
{}

void OverworldDetector::make_overlays(VideoOverlaySet& items) const{
    m_arc_phone.make_overlays(items);
    m_mount.make_overlays(items);
}
bool OverworldDetector::process_frame(const QImage& frame, WallClock timestamp){
    m_arc_phone.process_frame(frame, timestamp);
    if (!m_arc_phone.detected()){
        return false;
    }
    return m_mount.detect(frame) != MountState::NOTHING;
}




bool is_pokemon_selection(VideoOverlay& overlay, const ConstImageRef& frame){
    InferenceBoxScope box(overlay, 0.843, 0.96, 0.075, 0.005);
    ImageStats stats = image_stats(extract_box_reference(frame, box));
//    cout << stats.average << stats.stddev << endl;
//    extract_box_reference(frame, box).save("test.png");
    if (is_solid(stats, {0.0652401, 0.606812, 0.327948}, 0.15, 50)){
        return true;
    }
    return false;
}




}
}
}
