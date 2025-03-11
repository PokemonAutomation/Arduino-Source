/*  Selection Arrow
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/ImageMatch/ExactImageMatcher.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "PokemonBDSP_SelectionArrow.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Kernels;
using namespace Kernels::Waterfill;


const ImageMatch::ExactImageMatcher& SELECTION_ARROW(){
    static ImageMatch::ExactImageMatcher matcher(RESOURCE_PATH() + "PokemonBDSP/SelectionArrow.png");
    return matcher;
}


bool is_selection_arrow(const ImageViewRGB32& image, const WaterfillObject& object){
    size_t width = object.width();
    size_t height = object.height();
    if (width > height){
        return false;
    }
    if (height > 3 * width){
        return false;
    }

    ImageViewRGB32 cropped = extract_box_reference(image, object);

//    static int c = 0;
//    scaled.save("test-" + std::to_string(c++) + ".png");

    double rmsd = SELECTION_ARROW().rmsd(cropped);

//    scaled = scaled.scaled(exclamation_mark.width(), exclamation_mark.height());
//    double rmsd = ImageMatch::pixel_RMSD(exclamation_mark, scaled);
//    cout << "rmsd = " << rmsd << endl;
    return rmsd <= 90;
}


std::vector<ImagePixelBox> find_selection_arrows(const ImageViewRGB32& image){
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_max(image, 200, 200, 200);
    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 200);
    std::vector<ImagePixelBox> ret;
    for (const WaterfillObject& object : objects){
        if (is_selection_arrow(image, object)){
            ret.emplace_back(
                ImagePixelBox(object.min_x, object.min_y, object.max_x, object.max_y)
            );
        }
    }
    return ret;
}




SelectionArrowFinder::SelectionArrowFinder(
    VideoOverlay& overlay,
    const ImageFloatBox& box,
    Color color
)
    : VisualInferenceCallback("SelectionArrowFinder")
    , m_overlay(overlay)
    , m_color(color)
    , m_box(box)
{}

bool SelectionArrowFinder::detect(const ImageViewRGB32& screen){
    std::vector<ImagePixelBox> arrows = find_selection_arrows(extract_box_reference(screen, m_box));

    m_arrow_boxes.clear();
    for (const ImagePixelBox& mark : arrows){
        m_arrow_boxes.emplace_back(m_overlay, translate_to_parent(screen, m_box, mark), COLOR_MAGENTA);
    }

    return !m_arrow_boxes.empty();
}
void SelectionArrowFinder::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool SelectionArrowFinder::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    detect(frame);
//    cout << m_arrow_boxes.size() << endl;
//    if (!m_arrow_boxes.empty()){
//        extract_box(frame, m_arrow_boxes[0]).save("temp.png");
//        frame.save("test.png");
//    }
//    return !m_arrow_boxes.empty();

    //  Need 5 consecutive successful detections.
    if (m_arrow_boxes.empty()){
        m_trigger_count = 0;
        return false;
    }
    m_trigger_count++;
    return m_trigger_count >= 5;
}









}
}
}
