/*  Mark Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Qt/ImageOpener.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
//#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "CommonFramework/ImageMatch/ExactImageMatcher.h"
#include "PokemonBDSP_MarkFinder.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Kernels;
using namespace Kernels::Waterfill;


const ImageMatch::ExactImageMatcher& EXCLAMATION_MARK(){
    static ImageMatch::ExactImageMatcher matcher(open_image(RESOURCE_PATH() + "PokemonBDSP/ExclamationMark-WhiteFill.png"));
    return matcher;
}


bool is_exclamation_mark(const ImageViewRGB32& image, const WaterfillObject& object){
    size_t width = object.width();
    size_t height = object.height();
    if (width > 2 * height){
        return false;
    }
    if (height > 2 * width){
        return false;
    }

//    const QImage& exclamation_mark = EXCLAMATION_MARK();
    ImageViewRGB32 obj = extract_box_reference(image, object);
    double rmsd = EXCLAMATION_MARK().rmsd(obj);
//    double rmsd = ImageMatch::pixel_RMSD(exclamation_mark, scaled);
//    cout << "rmsd = " << rmsd << endl;
    return rmsd <= 80;
}


std::vector<ImagePixelBox> find_exclamation_marks(const ImageViewRGB32& image){
    PackedBinaryMatrix2 matrix = compress_rgb32_to_binary_min(image, 200, 200, 200);
    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 400);
    std::vector<ImagePixelBox> ret;
    for (const WaterfillObject& object : objects){
        if (is_exclamation_mark(image, object)){
            ret.emplace_back(
                ImagePixelBox(object.min_x, object.min_y, object.max_x, object.max_y)
            );
        }
    }
    return ret;
}



MarkTracker::MarkTracker(VideoOverlay& overlay, const ImageFloatBox& box)
    : VisualInferenceCallback("MarkTracker")
    , m_overlay(overlay)
    , m_box(box)
{}

void MarkTracker::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box);
}
bool MarkTracker::process_frame(const QImage& frame, WallClock){
    std::vector<ImagePixelBox> exclamation_marks = find_exclamation_marks(extract_box_reference(frame, m_box));
//        cout << exclamation_marks.size() << endl;

    m_marks.clear();
    for (const ImagePixelBox& mark : exclamation_marks){
        m_marks.emplace_back(m_overlay, translate_to_parent(frame, m_box, mark), COLOR_MAGENTA);
    }
    return false;
}

bool MarkDetector::process_frame(const QImage& frame, WallClock timestamp){
    MarkTracker::process_frame(frame, timestamp);
    return !m_marks.empty();
}




}
}
}
