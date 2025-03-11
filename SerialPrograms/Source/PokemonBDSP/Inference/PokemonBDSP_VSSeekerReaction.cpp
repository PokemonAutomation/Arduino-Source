/*  VS Seeker Reaction Bubble
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageDiff.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "PokemonBDSP_VSSeekerReaction.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Kernels;
using namespace Kernels::Waterfill;


const ImageViewRGB32& VS_SEEKER_REACTION_BUBBLE(){
    static ImageRGB32 image(RESOURCE_PATH() + "PokemonBDSP/VSSeekerReactBuble-WhiteFill.png");
    return image;
}

bool is_seeker_bubble(const ImageViewRGB32& image, const WaterfillObject& object){
    size_t width = object.width();
    size_t height = object.height();
    if (width > 2 * height){
        return false;
    }
    if (height > 2 * width){
        return false;
    }

    const ImageViewRGB32& exclamation_mark = VS_SEEKER_REACTION_BUBBLE();
    ImageRGB32 scaled = extract_box_reference(image, object).scale_to(exclamation_mark.width(), exclamation_mark.height());
    double rmsd = ImageMatch::pixel_RMSD(exclamation_mark, scaled);
//    cout << "rmsd = " << rmsd << endl;
    return rmsd <= 80;
}

std::vector<ImagePixelBox> find_seeker_bubbles(const ImageViewRGB32& image){
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_min(image, 200, 200, 200);
    std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 400);
    std::vector<ImagePixelBox> ret;
    for (const WaterfillObject& object : objects){
        if (is_seeker_bubble(image, object)){
            ret.emplace_back(
                ImagePixelBox(object.min_x, object.min_y, object.max_x, object.max_y)
            );
        }
    }
    return ret;
}



VSSeekerReactionTracker::VSSeekerReactionTracker(VideoOverlay& overlay, const ImageFloatBox& box)
    : VisualInferenceCallback("VSSeekerReactionTracker")
    , m_overlay(overlay)
    , m_box(box)
{}
void VSSeekerReactionTracker::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box);
}
bool VSSeekerReactionTracker::process_frame(const ImageViewRGB32& frame, WallClock){
    ImageViewRGB32 cropped = extract_box_reference(frame, m_box);
    m_dimensions = QSize((int)cropped.width(), (int)cropped.height());
    m_bubbles = find_seeker_bubbles(cropped);
//        cout << exclamation_marks.size() << endl;

    m_boxes.clear();
    for (const ImagePixelBox& mark : m_bubbles){
        m_boxes.emplace_back(m_overlay, translate_to_parent(frame, m_box, mark), COLOR_MAGENTA);
    }
    return false;
}






}
}
}
