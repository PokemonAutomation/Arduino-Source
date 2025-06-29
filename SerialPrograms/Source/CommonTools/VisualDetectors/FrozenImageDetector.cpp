/*  Frozen Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "FrozenImageDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


FrozenImageDetector::FrozenImageDetector(std::chrono::milliseconds timeout, double rmsd_threshold)
    : VisualInferenceCallback("FrozenImageDetector")
    , m_color(COLOR_CYAN)
    , m_box(0.0, 0.0, 1.0, 1.0)
    , m_timeout(timeout)
    , m_rmsd_threshold(rmsd_threshold)
{}
FrozenImageDetector::FrozenImageDetector(
    Color color, const ImageFloatBox& box,
    std::chrono::milliseconds timeout, double rmsd_threshold
)
    : VisualInferenceCallback("FrozenImageDetector")
    , m_color(color)
    , m_box(box)
    , m_timeout(timeout)
    , m_rmsd_threshold(rmsd_threshold)
{}
void FrozenImageDetector::make_overlays(VideoOverlaySet& set) const{
    set.add(m_color, m_box);
}
bool FrozenImageDetector::process_frame(const VideoSnapshot& frame){
    if (m_previous->width() != frame->width() || m_previous->height() != frame->height()){
        m_previous = frame;
        return false;
    }

    double rmsd = ImageMatch::pixel_RMSD(
        extract_box_reference(m_previous, m_box),
        extract_box_reference(frame, m_box)
    );
//    cout << "rmsd = " << rmsd << endl;
    if (rmsd > m_rmsd_threshold){
        m_previous = frame;
        return false;
    }

    return frame.timestamp - m_previous.timestamp > m_timeout;
//    return false;
}
bool FrozenImageDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    return process_frame(VideoSnapshot(frame.copy(), timestamp));
}


}
