/*  In-Battle Arrow Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/BinaryImage_FilterRgb32.h"
#include "CommonFramework/ImageMatch/ExactImageMatcher.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "PokemonSwSh_SelectionArrowFinder.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

using namespace Kernels;
using namespace Kernels::Waterfill;


const ImageMatch::ExactImageMatcher& SELECTION_ARROW(){
    static ImageMatch::ExactImageMatcher matcher(RESOURCE_PATH() + "PokemonSwSh/BattleArrow.png");
    return matcher;
}

bool is_selection_arrow(const ImageViewRGB32& image, const WaterfillObject& object){
    double area = (double)object.area_ratio();
    if (area < 0.4 || area > 0.5){
        return false;
    }

//    size_t width = object.width();
//    size_t height = object.height();
    ImageRGB32 cropped = extract_box_reference(image, object).copy();

    filter_rgb32(
        object.packed_matrix(),
        cropped,
        COLOR_WHITE,
        true
    );

//    cropped.save("cropped.png");

    double rmsd = SELECTION_ARROW().rmsd(cropped);
//    cout << "rmsd = " << rmsd << endl;
    return rmsd <= 110;
}
std::vector<ImagePixelBox> find_selection_arrows(const ImageViewRGB32& image){
    PackedBinaryMatrix2 matrix = compress_rgb32_to_binary_max(image, 63, 63, 63);
    auto session = make_WaterfillSession(matrix);
    auto finder = session->make_iterator(200);
    std::vector<ImagePixelBox> ret;
    WaterfillObject object;
    while (finder->find_next(object, true)){
//        cout << "asdf" << endl;
        if (is_selection_arrow(image, object)){
            ret.emplace_back(object);
        }
    }
    return ret;
}



SelectionArrowFinder::SelectionArrowFinder(VideoOverlay& overlay, const ImageFloatBox& box)
    : VisualInferenceCallback("SelectionArrowFinder")
    , m_overlay(overlay)
    , m_box(box)
{}
void SelectionArrowFinder::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_YELLOW, m_box);
}
bool SelectionArrowFinder::detect(const ImageViewRGB32& screen){
    std::vector<ImagePixelBox> arrows = find_selection_arrows(extract_box_reference(screen, m_box));

    m_arrow_boxes.clear();
    for (const ImagePixelBox& mark : arrows){
        m_arrow_boxes.emplace_back(m_overlay, translate_to_parent(screen, m_box, mark), COLOR_MAGENTA);
    }
    return !m_arrow_boxes.empty();
}
bool SelectionArrowFinder::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
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
int8_t BattleMoveArrowFinder::detect(const ImageViewRGB32& screen){
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
bool BattleMoveArrowFinder::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
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
