/*  Frozen Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "FrozenImageDetector.h"

#include <iostream>
using std::cout;
using std::endl;

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
bool FrozenImageDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    ImageRGB32 image = extract_box_reference(frame, m_box).copy();
    if (m_previous.width() != image.width() || m_previous.height() != image.height()){
        m_timestamp = timestamp;
        m_previous = std::move(image);
        return false;
    }

    double rmsd = ImageMatch::pixel_RMSD(m_previous, image);
//    cout << "rmsd = " << rmsd << endl;
    if (rmsd > m_rmsd_threshold){
        m_timestamp = timestamp;
        m_previous = std::move(image);
        return false;
    }

    return timestamp - m_timestamp > m_timeout;
//    return false;
}


}
