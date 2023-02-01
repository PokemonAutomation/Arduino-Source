/*  Area Zero Sky Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "PokemonSV_AreaZeroSkyDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



bool AreaZeroSkyDetector::detect(Kernels::Waterfill::WaterfillObject& object, const ImageViewRGB32& screen) const{
    using namespace Kernels::Waterfill;

    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(screen, 0xffc0c0c0, 0xffffffff);

    size_t min_width = screen.width() / 4;
    size_t min_height = screen.height() / 4;

    std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
    auto iter = session->make_iterator(10000);
    while (iter->find_next(object, false)){
//        if (object.min_y != 0){
//            continue;
//        }
        if (object.width() < min_width || object.height() < min_height){
            continue;
        }
        return true;
    }
    return false;
}
bool AreaZeroSkyDetector::detect(const ImageViewRGB32& screen) const{
    using namespace Kernels::Waterfill;

    WaterfillObject object;
    return detect(object, screen);
}




AreaZeroSkyTracker::AreaZeroSkyTracker(VideoOverlay& overlay)
    : VisualInferenceCallback("AreaZeroSkyTracker")
    , m_overlay(overlay)
{}

bool AreaZeroSkyTracker::sky_location(double& x, double& y) const{
    SpinLock m_lock;
    if (!m_box){
        return false;
    }
    x = m_center_x;
    y = m_center_y;
    return true;
}

void AreaZeroSkyTracker::make_overlays(VideoOverlaySet& items) const{}
bool AreaZeroSkyTracker::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    using namespace Kernels::Waterfill;

    WaterfillObject object;
    bool detected = this->detect(object, frame);
    SpinLockGuard lg(m_lock);
    if (detected){
        m_box.reset(new OverlayBoxScope(
            m_overlay,
            COLOR_GREEN,
            translate_to_parent(frame, {0, 0, 1, 1}, object),
            "Area Zero Sky"
        ));
        m_center_x = object.center_of_gravity_x() / frame.width();
        m_center_y = object.center_of_gravity_y() / frame.height();
    }else{
        m_box.reset();
    }
    return false;
}




}
}
}
