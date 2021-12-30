/*  Selection Arrow
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/CommonFilters.h"
#include "CommonFramework/ImageTools/CellMatrix.h"
#include "CommonFramework/ImageTools/FillGeometry.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonBDSP_SelectionArrow.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


const QImage& SELECTION_ARROW(){
    static QImage image(RESOURCE_PATH() + "PokemonBDSP/SelectionArrow.png");
    return image;
}


bool is_selection_arrow(const QImage& image, const FillGeometry& object){
    if (object.box.width() > object.box.height()){
        return false;
    }
    if (object.box.height() > 3 * object.box.width()){
        return false;
    }

    const QImage& exclamation_mark = SELECTION_ARROW();
    QImage scaled = image.copy(object.box.min_x, object.box.min_y, object.box.width(), object.box.height());

//    static int c = 0;
//    scaled.save("test-" + QString::number(c++) + ".png");

    scaled = scaled.scaled(exclamation_mark.width(), exclamation_mark.height());
    double rmsd = ImageMatch::pixel_RMSD(exclamation_mark, scaled);
//    cout << "rmsd = " << rmsd << endl;
    return rmsd <= 80;
}

size_t find_selection_arrows(
    const QImage& image,
    std::vector<ImagePixelBox>& exclamation_marks
){
    CellMatrix matrix(image);
    BlackFilter filter(200);
    matrix.apply_filter(image, filter);

    size_t count = 0;
    std::vector<FillGeometry> objects = find_all_objects(matrix, 1, false, 200);
//    cout << "objects = " << objects.size() << endl;
    for (const FillGeometry& object : objects){
        if (is_selection_arrow(image, object)){
            exclamation_marks.emplace_back(object.box);
            count++;
        }
    }
    return count;
}




SelectionArrowFinder::SelectionArrowFinder(
    VideoOverlay& overlay,
    const ImageFloatBox& box,
    Color color
)
    : m_overlay(overlay)
    , m_color(color)
    , m_box(box)
{}

void SelectionArrowFinder::detect(const QImage& screen){
    std::vector<ImagePixelBox> exclamation_marks;
    find_selection_arrows(extract_box(screen, m_box), exclamation_marks);
//        cout << exclamation_marks.size() << endl;

    m_arrow_boxes.clear();
    for (const ImagePixelBox& mark : exclamation_marks){
        m_arrow_boxes.emplace_back(m_overlay, translate_to_parent(screen, m_box, mark), COLOR_MAGENTA);
    }
}
void SelectionArrowFinder::make_overlays(OverlaySet& items) const{
    items.add(m_color, m_box);
}
bool SelectionArrowFinder::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    detect(frame);
//    cout << m_arrow_boxes.size() << endl;
//    if (!m_arrow_boxes.empty()){
//        extract_box(frame, m_arrow_boxes[0]).save("temp.png");
//        frame.save("test.png");
//    }
    return !m_arrow_boxes.empty();
}









}
}
}
