/*  In-Battle Arrow Finder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <iostream>
#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/DebugDumper.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/ImageMatch/ExactImageMatcher.h"
#include "PokemonSwSh_SelectionArrowFinder.h"

//#include <iostream>
//using std::cout;
//using std::endl;

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
    double area_ratio = (double)object.area_ratio();
    if (PreloadSettings::debug().IMAGE_TEMPLATE_MATCHING){
        std::cout << "Object area: " << object.area << ", area ratio: " << area_ratio
                  << " bound [0.4, 0.5]" << std::endl;
        dump_debug_image(global_logger_command_line(), "PokemonSwSh/SelectionArrowFinder", "is_selection_arrow", extract_box_reference(image, object));
    }
    if (area_ratio < 0.4 || area_ratio > 0.5){
        return false;
    }

    ImageRGB32 cropped = extract_box_reference(image, object).copy();
    auto packed_matrix = object.packed_matrix();
    size_t matrix_width = packed_matrix->width();
    size_t matrix_height = packed_matrix->height();
    try{
        filter_by_mask(
            std::move(packed_matrix),
            cropped,
            COLOR_WHITE,
            true
        );
    }catch (InternalProgramError&){
        global_logger_tagged().log(
            "Mismatching matrix and image size.\n"
            "    Image: " + std::to_string(image.width()) + "x" + std::to_string(image.height()) +
            "    Cropped: " + std::to_string(cropped.width()) + "x" + std::to_string(cropped.height()) +
            "    Matrix: " + std::to_string(matrix_width) + "x" + std::to_string(matrix_height) +
            "    Object: " + std::to_string(object.width()) + "x" + std::to_string(object.height()) +
            "    Object X: " + std::to_string(object.min_x) + "-" + std::to_string(object.max_x) +
            "    Object Y: " + std::to_string(object.min_y) + "-" + std::to_string(object.max_y),
            COLOR_RED
        );
        dump_image(global_logger_tagged(), ProgramInfo(), "is_selection_arrow_size_mismatch", image);
        throw;
    }


    double rmsd = SELECTION_ARROW().rmsd(cropped);
    
    if (PreloadSettings::debug().IMAGE_TEMPLATE_MATCHING){
        std::cout << "rmsd: " << rmsd << ", threshold 130" << std::endl;
    }
    return rmsd <= 130;
}
std::vector<ImagePixelBox> find_selection_arrows(const ImageViewRGB32& image, size_t min_area){
    if (PreloadSettings::debug().IMAGE_TEMPLATE_MATCHING){
        std::cout << "Match SwSh selection arrow by waterfill, size range (" << min_area << ", SIZE_MAX) " 
                  << "input image size " << image.width() << " x " << image.height() << std::endl;
    }
    PackedBinaryMatrix matrix = compress_rgb32_to_binary_max(image, 63, 63, 63);
    auto session = make_WaterfillSession(matrix);
    auto finder = session->make_iterator(min_area);
    std::vector<ImagePixelBox> ret;
    WaterfillObject object;
    while (finder->find_next(object, true)){
        if (PreloadSettings::debug().IMAGE_TEMPLATE_MATCHING){
            std::cout << "Found object: " << object.min_x << "-" << object.max_x << ", " << object.min_y << "-" << object.max_y << std::endl;
        }
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
    const double screen_scale = screen.height() / 1080.0;
    // Smallest arrow takes at least 600 pixels on 1920x1080 screen.
    const size_t min_arrow_area = size_t(600.0 * screen_scale * screen_scale);
    std::vector<ImagePixelBox> arrows = find_selection_arrows(
        extract_box_reference(screen, m_box), min_arrow_area);

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

    const ImageFloatBox& arrow = m_arrow_boxes[0];
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



RetrieveEggArrowFinder::RetrieveEggArrowFinder(VideoOverlay& overlay)
    : SelectionArrowFinder(overlay, ImageFloatBox(0.597, 0.640, 0.166, 0.071))
{}


CheckNurseryArrowFinder::CheckNurseryArrowFinder(VideoOverlay& overlay)
    : SelectionArrowFinder(overlay, ImageFloatBox(0.419, 0.570, 0.290, 0.084))
{}


StoragePokemonMenuArrowFinder::StoragePokemonMenuArrowFinder(VideoOverlay& overlay)
    : SelectionArrowFinder(overlay, ImageFloatBox(0.555, 0.413, 0.177, 0.078))
{}


RotomPhoneMenuArrowFinder::RotomPhoneMenuArrowFinder(VideoOverlay& overlay)
    : m_overlay_set(overlay)
{
    for(size_t i_row = 0; i_row < 2; i_row++){
        for(size_t j_col = 0; j_col < 5; j_col++){
            ImageFloatBox box(0.047 + j_col*0.183, 0.175 + 0.333*i_row, 0.059, 0.104);
            m_overlay_set.add(COLOR_YELLOW, box);
        }
    }
}

void RotomPhoneMenuArrowFinder::make_overlays(VideoOverlaySet& items) const{

}
bool RotomPhoneMenuArrowFinder::detect(const ImageViewRGB32& screen){
    return detect_index(screen) >= 0;
}

int RotomPhoneMenuArrowFinder::detect_index(const ImageViewRGB32& screen){
    const double screen_scale = screen.height() / 1080.0;
    const size_t min_arrow_area = size_t(1400 * screen_scale * screen_scale);
    for (size_t i_row = 0; i_row < 2; i_row++){
        for (size_t j_col = 0; j_col < 5; j_col++){
            ImageFloatBox box(0.047 + j_col*0.183, 0.175 + 0.333*i_row, 0.059, 0.104);
            std::vector<ImagePixelBox> arrows = find_selection_arrows(
                extract_box_reference(screen, box),
                min_arrow_area
            );
            if (arrows.size() > 0){
                m_index = (int)(i_row * 5 + j_col);
                return m_index;
            }
        }
    }
    return -1;
}

}
}
}
