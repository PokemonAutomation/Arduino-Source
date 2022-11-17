/*  Gradient Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTypes/BinaryImage.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/ImageMatch/ExactImageMatcher.h"
#include "PokemonSV_GradientArrowDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Kernels;
using namespace Kernels::Waterfill;



const ImageMatch::ExactImageMatcher& GRADIENT_ARROW_HORIZONTAL(){
    static ImageMatch::ExactImageMatcher matcher(RESOURCE_PATH() + "PokemonSV/GradiantArrowHorizontal-Template.png");
    return matcher;
}
const ImageMatch::ExactImageMatcher& GRADIENT_ARROW_VERTICAL(){
    static ImageMatch::ExactImageMatcher matcher(RESOURCE_PATH() + "PokemonSV/GradiantArrowVertical-Template.png");
    return matcher;
}

bool is_gradient_arrow(
    const ImageViewRGB32& image,
    WaterfillObject& object,
    const WaterfillObject& yellow, const WaterfillObject& blue
){
    object = yellow;
    object.merge_assume_no_overlap(blue);

    ImageViewRGB32 cropped = extract_box_reference(image, object);

    const double THRESHOLD = 80;

    double aspect_ratio = object.aspect_ratio();
    if (0.7 < aspect_ratio && aspect_ratio < 1.0){
        double rmsd = GRADIENT_ARROW_HORIZONTAL().rmsd(cropped);
        return rmsd <= THRESHOLD;
    }
    if (1.0  < aspect_ratio && aspect_ratio < 1.43){
        double rmsd = GRADIENT_ARROW_VERTICAL().rmsd(cropped);
        return rmsd <= THRESHOLD;
    }

    return false;
}



GradientArrowDetector::GradientArrowDetector(const ImageFloatBox& box, Color color)
    : m_color(color)
    , m_box(box)
{}
void GradientArrowDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box);
}
bool GradientArrowDetector::detect(const ImageViewRGB32& screen) const{
    std::vector<ImageFloatBox> hits = detect_all(screen);
    return !hits.empty();
}

std::vector<ImageFloatBox> GradientArrowDetector::detect_all(const ImageViewRGB32& screen) const{
    using namespace Kernels::Waterfill;

    ImageViewRGB32 region = extract_box_reference(screen, m_box);
//    region.save("test.png");

    std::vector<WaterfillObject> yellows;
    std::vector<WaterfillObject> blues;
    std::unique_ptr<WaterfillSession> session = make_WaterfillSession();
    {
        PackedBinaryMatrix yellow_matrix = compress_rgb32_to_binary_range(region, 0xffc0c000, 0xffffff7f);
        session->set_source(yellow_matrix);
        auto iter = session->make_iterator(400);
        WaterfillObject object;
//        size_t c = 0;
        while (iter->find_next(object, false)){
//            cout << "yellow = " << object.area << endl;
//            extract_box_reference(region, object).save("yellow-" + std::to_string(c++) + ".png");
            yellows.emplace_back(std::move(object));
        }
    }
    {
        PackedBinaryMatrix blue_matrix = compress_rgb32_to_binary_range(region, 0xff004080, 0xff8fffff);
        session->set_source(blue_matrix);
        auto iter = session->make_iterator(400);
        WaterfillObject object;
//        size_t c = 0;
        while (iter->find_next(object, false)){
//            cout << "blue = " << object.area << endl;
//            extract_box_reference(region, object).save("blue-" + std::to_string(c++) + ".png");
            blues.emplace_back(std::move(object));
        }
    }

    std::vector<ImageFloatBox> hits;

//    size_t c = 0;
    for (WaterfillObject& yellow : yellows){
        for (WaterfillObject& blue : blues){
            WaterfillObject object;
            if (is_gradient_arrow(region, object, yellow, blue)){
                hits.emplace_back(translate_to_parent(screen, m_box, object));
            }
//            double aspect_ratio = object.aspect_ratio();
//            cout << "aspect_ratio = " << aspect_ratio << endl;
//            extract_box_reference(region, object).save("object-" + std::to_string(c++) + ".png");

        }
    }

    return hits;
}



GradientArrowFinder::~GradientArrowFinder() = default;
GradientArrowFinder::GradientArrowFinder(VideoOverlay& overlay, const ImageFloatBox& box, Color color)
    : VisualInferenceCallback("GradientArrowFinder")
    , m_overlay(overlay)
    , m_detector(box, color)
{}

void GradientArrowFinder::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}
bool GradientArrowFinder::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    std::vector<ImageFloatBox> arrows = m_detector.detect_all(frame);
//    cout << "arrors = " << arrows.size() << endl;
    m_arrows.reset(arrows.size());
    for (const ImageFloatBox& arrow : arrows){
        m_arrows.emplace_back(m_overlay, arrow, COLOR_MAGENTA);
    }
    return !arrows.empty();
}




}
}
}
