/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/ImageTypes/BinaryImage.h"
#include "CommonFramework/ImageTools/SolidColorTest.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonSV/Inference/PokemonSV_GradientArrowDetector.h"
#include "PokemonSV_TeraCardDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



TeraCardReader::TeraCardReader(Color color)
    : m_color(color)
    , m_top(0.15, 0.13, 0.40, 0.03)
    , m_bottom_left(0.15, 0.80, 0.10, 0.06)
    , m_bottom_right(0.73, 0.85, 0.12, 0.02)
    , m_label(0.75, 0.67, 0.10, 0.05)
    , m_cursor(0.135, 0.25, 0.05, 0.25)
    , m_stars(0.500, 0.555, 0.310, 0.070)
{}
void TeraCardReader::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_top);
    items.add(COLOR_RED, m_bottom_left);
    items.add(COLOR_RED, m_bottom_right);
    items.add(COLOR_RED, m_label);
    items.add(COLOR_RED, m_cursor);
    items.add(COLOR_RED, m_stars);
}
bool TeraCardReader::detect(const ImageViewRGB32& screen) const{
    ImageStats top = image_stats(extract_box_reference(screen, m_top));
//    cout << top.average << top.stddev << endl;
    if (!is_solid(top, {0.354167, 0.345833, 0.3})){
        return false;
    }

    ImageStats bottom_left = image_stats(extract_box_reference(screen, m_bottom_left));
//    cout << bottom_left.average << bottom_left.stddev << endl;
    if (!is_solid(bottom_left, {0.354167, 0.345833, 0.3})){
        return false;
    }
    ImageStats bottom_right = image_stats(extract_box_reference(screen, m_bottom_right));
//    cout << bottom_right.average << bottom_right.stddev << endl;
    if (!is_solid(bottom_right, {0.354167, 0.345833, 0.3})){
        return false;
    }

    if (euclidean_distance(top.average, bottom_left.average) > 20){
        return false;
    }
    if (euclidean_distance(top.average, bottom_right.average) > 20){
        return false;
    }
    if (euclidean_distance(bottom_left.average, bottom_right.average) > 20){
        return false;
    }

    ImageStats label = image_stats(extract_box_reference(screen, m_label));
//    cout << label.average << label.stddev << endl;
    if (!is_solid(label, {0.370075, 0.369063, 0.260862})){
        return false;
    }

    GradientArrowDetector arrow_detector(m_cursor);
    if (!arrow_detector.detect(screen)){
        return false;
    }

    return true;
}
size_t TeraCardReader::stars(const ImageViewRGB32& screen) const{
    using namespace Kernels::Waterfill;

    ImageViewRGB32 cropped = extract_box_reference(screen, m_stars);
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(cropped, 0xff808000, 0xffffff80);

    std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
    auto iter = session->make_iterator(100);
    WaterfillObject object;
    size_t count = 0;
    while (iter->find_next(object, false)){
//        extract_box_reference(screen, object).save("test-" + std::to_string(count) + ".png");
        count++;
    }
//    cout << "count = " << count << endl;

    if (1 <= count && count <= 5){
        return count;
    }

    ImageStats background = image_stats(extract_box_reference(screen, ImageFloatBox{0.55, 0.62, 0.20, 0.03}));
    Color background_average = background.average.round();

    matrix = compress_rgb32_to_binary_euclidean(cropped, (uint32_t)background_average, 100);

    matrix.invert();
//    cout << matrix.dump() << endl;
    session = make_WaterfillSession(matrix);
    iter = session->make_iterator(100);
    count = 0;
    while (iter->find_next(object, false)){
//        extract_box_reference(screen, object).save("test-" + std::to_string(count) + ".png");
        count++;
    }
//    cout <<
    if (6 <= count && count <= 7){
        return count;
    }

    return 0;
}


TeraCardFinder::TeraCardFinder()
    : VisualInferenceCallback("TeraCardFinder")
{}

void TeraCardFinder::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}
bool TeraCardFinder::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    //  Need 5 consecutive successful detections.
    if (!m_detector.detect(frame)){
        m_trigger_count = 0;
        return false;
    }
    m_trigger_count++;
    bool detected = m_trigger_count >= 5;
    if (detected){
//        cout << "Detected Battle Menu" << endl;
    }
    return detected;
}




}
}
}
