/*  Dialog Arrow Detector
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
#include "PokemonSV_DialogArrowDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Kernels;
using namespace Kernels::Waterfill;



const ImageMatch::ExactImageMatcher& DIALOG_ARROW(){
    static ImageMatch::ExactImageMatcher matcher(RESOURCE_PATH() + "PokemonSV/DialogArrow.png");
    return matcher;
}

bool is_dialog_arrow(const ImageViewRGB32& image, const WaterfillObject& object){
    double aspect_ratio = object.aspect_ratio();
//    cout << "aspect_ratio = " << aspect_ratio << endl;
    if (aspect_ratio < 1.0 || aspect_ratio > 1.3){
        return false;
    }
//    double area = (double)object.area_ratio();
//    if (area < 0.4 || area > 0.5){
//        return false;
//    }

    ImageViewRGB32 cropped = extract_box_reference(image, object);

    double rmsd = DIALOG_ARROW().rmsd(cropped);
//    cout << "rmsd = " << rmsd << endl;
    return rmsd <= 80;
}



DialogArrowDetector::DialogArrowDetector(const ImageFloatBox& box, Color color)
    : m_color(color)
    , m_box(box)
{}
void DialogArrowDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box);
}
bool DialogArrowDetector::detect(const ImageViewRGB32& screen) const{
    std::vector<ImageFloatBox> hits = detect_all(screen);
    return !hits.empty();
}

std::vector<ImageFloatBox> DialogArrowDetector::detect_all(const ImageViewRGB32& screen) const{
    using namespace Kernels::Waterfill;

    ImageViewRGB32 region = extract_box_reference(screen, m_box);
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(region, 0xff000000, 0xff7f7fbf);

    std::vector<ImageFloatBox> hits;

    std::unique_ptr<WaterfillSession> session = make_WaterfillSession(matrix);
    auto iter = session->make_iterator(20);
    WaterfillObject object;
    while (iter->find_next(object, false)){
        if (is_dialog_arrow(region, object)){
            hits.emplace_back(translate_to_parent(screen, m_box, object));
        }
    }

    return hits;
}



DialogArrowFinder::~DialogArrowFinder() = default;
DialogArrowFinder::DialogArrowFinder(VideoOverlay& overlay, const ImageFloatBox& box, Color color)
    : VisualInferenceCallback("GradientArrowFinder")
    , m_overlay(overlay)
    , m_detector(box, color)
{}

void DialogArrowFinder::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}
bool DialogArrowFinder::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
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
