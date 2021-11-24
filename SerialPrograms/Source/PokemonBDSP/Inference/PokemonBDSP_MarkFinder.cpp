/*  Mark Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/CommonFilters.h"
#include "CommonFramework/ImageTools/FillGeometry.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "PokemonBDSP_MarkFinder.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


const QImage& EXCLAMATION_MARK(){
    static QImage image(RESOURCE_PATH() + "PokemonBDSP/ExclamationMark-WhiteFill.png");
    return image;
}


bool is_exclamation_mark(const QImage& image, const FillGeometry& object){
    if (object.box.width() > 2 * object.box.height()){
        return false;
    }
    if (object.box.height() > 2 * object.box.width()){
        return false;
    }

    const QImage& exclamation_mark = EXCLAMATION_MARK();
    QImage scaled = image.copy(object.box.min_x, object.box.min_y, object.box.width(), object.box.height());
    scaled = scaled.scaled(exclamation_mark.width(), exclamation_mark.height());
    double rmsd = ImageMatch::pixel_RMSD(exclamation_mark, scaled);
//    cout << "rmsd = " << rmsd << endl;
    return rmsd <= 80;
}


size_t find_exclamation_marks(
    const QImage& image,
    std::vector<ImagePixelBox>& exclamation_marks
){
    CellMatrix matrix(image);
    WhiteFilter filter(200);
    matrix.apply_filter(image, filter);

    size_t count = 0;
    std::vector<FillGeometry> objects = find_all_objects(matrix, 1, true, 400);
//    cout << "objects = " << objects.size() << endl;
    for (const FillGeometry& object : objects){
        if (is_exclamation_mark(image, object)){
            exclamation_marks.emplace_back(object.box);
            count++;
        }
    }
    return count;
}



MarkTracker::MarkTracker(VideoOverlay& overlay, const ImageFloatBox& box)
    : m_overlay(overlay)
    , m_box(box)
{
    add_box(m_box);
}

bool MarkTracker::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point
){
    std::vector<ImagePixelBox> exclamation_marks;
    find_exclamation_marks(extract_box(frame, m_box), exclamation_marks);
//        cout << exclamation_marks.size() << endl;

    m_marks.clear();
    for (const ImagePixelBox& mark : exclamation_marks){
        m_marks.emplace_back(m_overlay, translate_to_parent(frame, m_box, mark), Qt::magenta);
    }
    return false;
}

bool MarkDetector::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    MarkTracker::process_frame(frame, timestamp);
    return !m_marks.empty();
}




}
}
}
