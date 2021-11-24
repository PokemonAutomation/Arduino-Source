/*  VS Seeker Reaction Bubble
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/CommonFilters.h"
#include "CommonFramework/ImageTools/FillGeometry.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "PokemonBDSP_VSSeekerReaction.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


const QImage& VS_SEEKER_REACTION_BUBBLE(){
    static QImage image(RESOURCE_PATH() + "PokemonBDSP/VSSeekerReactBuble-WhiteFill.png");
    return image;
}

bool is_seeker_bubble(const QImage& image, const FillGeometry& object){
    if (object.box.width() > 2 * object.box.height()){
        return false;
    }
    if (object.box.height() > 2 * object.box.width()){
        return false;
    }

    const QImage& exclamation_mark = VS_SEEKER_REACTION_BUBBLE();
    QImage scaled = image.copy(object.box.min_x, object.box.min_y, object.box.width(), object.box.height());
    scaled = scaled.scaled(exclamation_mark.width(), exclamation_mark.height());
    double rmsd = ImageMatch::pixel_RMSD(exclamation_mark, scaled);
//    cout << "rmsd = " << rmsd << endl;
    return rmsd <= 80;
}

size_t find_seeker_bubbles(
    const QImage& image,
    std::vector<ImagePixelBox>& bubbles
){
    CellMatrix matrix(image);
    WhiteFilter filter(200);
    matrix.apply_filter(image, filter);

    size_t count = 0;
//    size_t id = 0;
    std::vector<FillGeometry> objects = find_all_objects(matrix, 1, true, 400);
//    cout << "objects = " << objects.size() << endl;
    for (const FillGeometry& object : objects){
//        image.copy(
//            object.box.min_x, object.box.min_y, object.box.width(), object.box.height()
//        ).save("test-" + QString::number(id++) + ".png");

        if (is_seeker_bubble(image, object)){
            bubbles.emplace_back(object.box);
            count++;
        }
    }
    return count;
}



VSSeekerReactionTracker::VSSeekerReactionTracker(VideoOverlay& overlay, const ImageFloatBox& box)
    : m_overlay(overlay)
    , m_box(box)
{
    add_box(m_box);
}

bool VSSeekerReactionTracker::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point
){
    m_bubbles.clear();
    find_seeker_bubbles(extract_box(frame, m_box), m_bubbles);
//        cout << exclamation_marks.size() << endl;

    m_boxes.clear();
    for (const ImagePixelBox& mark : m_bubbles){
        m_boxes.emplace_back(m_overlay, translate_to_parent(frame, m_box, mark), Qt::magenta);
    }
    return false;
}






}
}
}
