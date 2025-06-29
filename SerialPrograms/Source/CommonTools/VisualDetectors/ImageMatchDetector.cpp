/*  Image Match Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "ImageMatchDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



ImageMatchDetector::ImageMatchDetector(
    std::shared_ptr<const ImageRGB32> reference_image, const ImageFloatBox& box,
    double max_rmsd, bool scale_brightness,
    Color color
)
    : m_reference_image(std::move(reference_image))
    , m_reference_image_cropped(extract_box_reference(*m_reference_image, box))
    , m_average_brightness(image_stats(m_reference_image_cropped).average)
    , m_max_rmsd(max_rmsd)
    , m_scale_brightness(scale_brightness)
    , m_color(color)
    , m_box(box)
{}

double ImageMatchDetector::rmsd(const ImageViewRGB32& frame) const{
    if (!frame){
        return 1000;
    }
    ImageViewRGB32 image = extract_box_reference(frame, m_box);
    ImageRGB32 scaled = image.scale_to(m_reference_image_cropped.width(), m_reference_image_cropped.height());

#if 0
    if (image.width() != (size_t)scaled.width() || image.height() != (size_t)scaled.height()){
        cout << image.width() << " x " << image.height() << " - " << scaled.width() << " x " << scaled.height() << endl;
        dump_image(global_logger_tagged(), ProgramInfo(), "ImageMatchDetector-rmsd", frame);
    }
#endif

    if (m_scale_brightness){
        FloatPixel image_brightness = ImageMatch::pixel_average(scaled, m_reference_image_cropped);
        FloatPixel scale = m_average_brightness / image_brightness;
        if (std::isnan(scale.r)) scale.r = 1.0;
        if (std::isnan(scale.g)) scale.g = 1.0;
        if (std::isnan(scale.b)) scale.b = 1.0;
        scale.bound(0.8, 1.2);
        ImageMatch::scale_brightness(scaled, scale);
    }

//    cout << "asdf" << endl;
    double ret = ImageMatch::pixel_RMSD(m_reference_image_cropped, scaled);
//    cout << "rmsd = " << ret << endl;
    return ret;
}

void ImageMatchDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool ImageMatchDetector::detect(const ImageViewRGB32& screen){
    return rmsd(screen) <= m_max_rmsd;
}



ImageMatchWatcher::ImageMatchWatcher(
    std::shared_ptr<const ImageRGB32> reference_image, const ImageFloatBox& box,
    double max_rmsd, bool scale_brightness,
    std::chrono::milliseconds hold_duration,
    Color color
)
    : ImageMatchDetector(std::move(reference_image), box, max_rmsd, scale_brightness, color)
    , VisualInferenceCallback("ImageMatchWatcher")
    , m_hold_duration(hold_duration)
    , m_last_match(false)
    , m_start_of_match(WallClock::min())
{}

void ImageMatchWatcher::make_overlays(VideoOverlaySet& items) const{
    ImageMatchDetector::make_overlays(items);
}
bool ImageMatchWatcher::process_frame(const ImageViewRGB32& frame, WallClock){
    if (!detect(frame)){
        m_last_match = false;
        return false;
    }
    auto now = current_time();
    if (!m_last_match){
        m_last_match = true;
        m_start_of_match = now;
        return false;
    }
    return now - m_start_of_match >= m_hold_duration;
}






}
