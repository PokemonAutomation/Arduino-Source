/*  In-Battle Arrow Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/CellMatrix.h"
#include "CommonFramework/ImageTools/FillGeometry.h"
#include "CommonFramework/ImageMatch/ImageDiff.h"
#include "CommonFramework/Inference/ImageTools.h"
#include "PokemonSwSh_SelectionArrowFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


#if 1
struct BlackFilter{
    size_t count = 0;

    void operator()(CellMatrix::ObjectID& cell, const QImage& image, int x, int y){
        QRgb pixel = image.pixel(x, y);
        int set = qRed(pixel) < 64 && qGreen(pixel) < 64 && qBlue(pixel) < 64;
//        int set = (pixel & 0x00c0c080) == 0x00c0c080 ? 1 : 0;
        cell = set;
        count += set;
    }
};
#else
struct BlackFilter{
    size_t count = 0;

    void operator()(CellMatrix::ObjectID& cell, QImage& image, int x, int y){
        QRgb pixel = image.pixel(x, y);
        bool black = qRed(pixel) < 48 && qGreen(pixel) < 48 && qBlue(pixel) < 48;
//        int set = (pixel & 0x00c0c080) == 0x00c0c080 ? 1 : 0;
        if (!black){
            image.setPixel(x, y, QColor(Qt::blue).rgb());
        }
        CellMatrix::ObjectID set = black ? 1 : 0;
        cell = set;
        count += set;
    }
};
#endif





const QImage& SelectionArrowDetector_reference_image(){
    static QImage image(RESOURCE_PATH() + "PokemonSwSh/BattleArrow.png");
    return image;
}
bool SelectionArrowDetector_is_arrow(
    const QImage& image, const CellMatrix& matrix,
    const FillGeometry& object
){
    double area = (double)object.area / object.box.area();
    if (area < 0.4 || area > 0.5){
        return false;
    }

    QImage oimage = image.copy(
        object.box.min_x, object.box.min_y,
        object.box.width(), object.box.height()
    );
//    oimage.convertToFormat(QImage::Format::Format_ARGB32);
//    cout << "asdf" << endl;
//    oimage.save("test-" + QString::number(object.id) + ".png");

    int width = oimage.width();
    int height = oimage.height();
    for (int r = 0; r < height; r++){
        for (int c = 0; c < width; c++){
            if (matrix[r + object.box.min_y][c + object.box.min_x] != object.id){
//                cout << "asdf" << endl;
//                QRgb pixel = oimage.pixel(c, r);
//                pixel &= 0x00ffffff;
                oimage.setPixel(c, r, 0xffffffff);
            }
        }
    }
//    oimage.save("test0.png");

    const QImage& reference = SelectionArrowDetector_reference_image();
    oimage = oimage.scaled(reference.size());
//    oimage.save("test1.png");

    double diff = ImageMatch::pixel_RMSD(reference, oimage, 0xffffffff);
//    cout << "diff = " << diff << endl;

//    image_diff_greyscale(oimage, reference).save("diff.png");

//    return true;
    return diff < 150;
}



SelectionArrowFinder::SelectionArrowFinder(VideoOverlay& overlay, const ImageFloatBox& box)
    : m_overlay(overlay)
    , m_box(box)
{}
void SelectionArrowFinder::make_overlays(OverlaySet& items) const{
    items.add(Qt::yellow, m_box);
}
bool SelectionArrowFinder::detect(const QImage& screen){
    QImage image = extract_box(screen, m_box);

    CellMatrix matrix(image);

    BlackFilter filter;
    matrix.apply_filter(image, filter);

    std::vector<FillGeometry> objects = find_all_objects(matrix, 1, false, 200);

//    double arrow_y_center = -1;
    m_arrow_boxes.clear();
//    cout << objects.size() << endl;

    for (const FillGeometry& object : objects){
        if (!SelectionArrowDetector_is_arrow(image, matrix, object)){
            continue;
        }
        ImageFloatBox box = translate_to_parent(screen, m_box, object.box);
        m_arrow_boxes.emplace_back(m_overlay, box, Qt::green);
//        arrow_y_center = box.y + box.height * 0.5;
    }
    return !m_arrow_boxes.empty();
}
bool SelectionArrowFinder::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    detect(frame);
//    cout << m_arrow_boxes.size() << endl;
    return !m_arrow_boxes.empty();
}



BattleMoveArrowFinder::BattleMoveArrowFinder(VideoOverlay& overlay)
    : SelectionArrowFinder(overlay, ImageFloatBox(0.640, 0.600, 0.055, 0.380))
    , m_arrow_slot(-1)
{}

int8_t BattleMoveArrowFinder::get_slot(){
    return m_arrow_slot.load(std::memory_order_acquire);
}
int8_t BattleMoveArrowFinder::detect(const QImage& screen){
    SelectionArrowFinder::detect(screen);

    if (m_arrow_boxes.empty()){
        return -1;
    }

    const InferenceBoxScope& arrow = m_arrow_boxes[0];
    double arrow_y_center = arrow.y + arrow.height * 0.5;

    int8_t slot = arrow_slot(arrow_y_center);
    m_arrow_slot.store(slot, std::memory_order_release);
    return slot;
}
bool BattleMoveArrowFinder::process_frame(
    const QImage& frame,
    std::chrono::system_clock::time_point timestamp
){
    detect(frame);
    return false;
}

int8_t BattleMoveArrowFinder::arrow_slot(double y_center){
    if (y_center < 0){
        return -1;
    }
    y_center -= 0.647222;
    y_center /= 0.0962963;
    return (int8_t)(y_center + 0.5);
}


}
}
}
