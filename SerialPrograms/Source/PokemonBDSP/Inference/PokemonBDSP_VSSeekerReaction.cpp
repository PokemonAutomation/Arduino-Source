/*  VS Seeker Reaction Bubble
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Tools/VideoOverlaySet.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "CommonFramework/BinaryImage/BinaryImage_FilterRgb32.h"
#include "PokemonBDSP_VSSeekerReaction.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Kernels;
using namespace Kernels::Waterfill;


const QImage& VS_SEEKER_REACTION_BUBBLE(){
    static QImage image(RESOURCE_PATH() + "PokemonBDSP/VSSeekerReactBuble-WhiteFill.png");
    return image;
}

bool is_seeker_bubble(const QImage& image, const WaterFillObject& object){
    size_t width = object.width();
    size_t height = object.height();
    if (width > 2 * height){
        return false;
    }
    if (height > 2 * width){
        return false;
    }

    const QImage& exclamation_mark = VS_SEEKER_REACTION_BUBBLE();
    QImage scaled = image.copy(
        (pxint_t)object.min_x, (pxint_t)object.min_y,
        (pxint_t)width, (pxint_t)height
    );
    scaled = scaled.scaled(exclamation_mark.width(), exclamation_mark.height());
    double rmsd = ImageMatch::pixel_RMSD(exclamation_mark, scaled);
//    cout << "rmsd = " << rmsd << endl;
    return rmsd <= 80;
}

std::vector<ImagePixelBox> find_seeker_bubbles(const QImage& image){
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_min(image, 200, 200, 200);
    std::vector<WaterFillObject> objects = find_objects_inplace(matrix, 400, false);
    std::vector<ImagePixelBox> ret;
    for (const WaterFillObject& object : objects){
        if (is_seeker_bubble(image, object)){
            ret.emplace_back(
                ImagePixelBox(object.min_x, object.min_y, object.max_x, object.max_y)
            );
        }
    }
    return ret;
}



VSSeekerReactionTracker::VSSeekerReactionTracker(VideoOverlay& overlay, const ImageFloatBox& box)
    : m_overlay(overlay)
    , m_box(box)
{}
void VSSeekerReactionTracker::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_box);
}
bool VSSeekerReactionTracker::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point
){
    QImage cropped = extract_box(frame, m_box);
    m_dimensions = cropped.size();
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
