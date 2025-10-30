/*  Box Detection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
// #include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/ImageMatch/SubObjectTemplateMatcher.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonLZA_BoxDetection.h"

#include <iostream>
#include <sstream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Kernels::Waterfill;

namespace{
    bool debug_switch = false;
}

// match the downward arrow (green border, white interior) in the box system view
class BoxCellSelectionArrowMatcher : public ImageMatch::SubObjectTemplateMatcher{
public:
    BoxCellSelectionArrowMatcher()
        : SubObjectTemplateMatcher("PokemonLZA/SelectionArrowDown.png", 100)
    {
        // relaxed area ratio upper bound
        m_area_ratio_upper = 1.5;
        PackedBinaryMatrix matrix = compress_rgb32_to_binary_range(
            m_matcher.image_template(),
            128, 255,
            128, 255,
            128, 255
        );
        std::vector<WaterfillObject> objects = find_objects_inplace(matrix, 20);
        if (objects.size() != 1){
            throw FileException(
                nullptr, PA_CURRENT_FUNCTION,
                "Failed to find exactly one object in resource.",
                m_path
            );
        }
        
        set_subobject(objects[0]);
    }

    static const BoxCellSelectionArrowMatcher& matcher(){
        static BoxCellSelectionArrowMatcher matcher;
        return matcher;
    }
};


BoxDetector::BoxDetector(Color color, VideoOverlay* overlay)
    : m_color(color)
    , m_plus_button(color, ButtonType::ButtonPlus, {0.581, 0.940, 0.310, 0.046}, overlay)
{
    for(size_t row = 0; row < 6; row++){
         double y = (row == 0 ? 0.122 : 0.333 + (0.797 - 0.331)/ 4.0 * (row-1));
        for(size_t col = 0; col < 6; col++){
            double x = 0.060 + col * (0.386 - 0.059)/5.0;
            m_arrow_boxes.emplace_back(x, y, 0.016, 0.026);
        }
    }
}

void BoxDetector::make_overlays(VideoOverlaySet& items) const{
    m_plus_button.make_overlays(items);
    for(const ImageFloatBox& box : m_arrow_boxes){
        items.add(m_color, box);
    }
}

// detect arrow's white interior first
// then use subobject template matcher BoxCellSelectionArrowMatcher to detect the whole arrow
bool BoxDetector::detect_at_cell(const ImageViewRGB32& image_crop){
    // the arrow's white interior has color between rgb [220, 220, 220] to [255, 255, 255]
    // the arrow's green border has color between rgb [170, 230, 50] to [190, 255, 80]
    std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {uint32_t(Color(220, 220, 220)), 0xffffffff},
        {uint32_t(Color(180, 180, 180)), 0xffffffff},
        {uint32_t(Color(140, 140, 140)), 0xffffffff}
    };
    std::vector<PackedBinaryMatrix> matrices = compress_rgb32_to_binary_range(image_crop, filters);

    const size_t total_crop_area = image_crop.width() * image_crop.height();
    const size_t min_area = static_cast<size_t>(total_crop_area / 60.0);
    const size_t max_area = static_cast<size_t>(total_crop_area / 15.0);

    int saved_object_id = 0;
    if (debug_switch){
        cout << "detect_at_cell() area threshold " << min_area << "-" << max_area << endl;
        cout << "input image size " << image_crop.width() << "x" << image_crop.height() << endl;
    }
    
    bool detected = false;
    auto& matcher = BoxCellSelectionArrowMatcher::matcher();
    for (size_t i_matrix = 0; i_matrix < matrices.size(); i_matrix++){
        PackedBinaryMatrix& matrix = matrices[i_matrix];

        std::unique_ptr<WaterfillSession> session = make_WaterfillSession();
        Kernels::Waterfill::WaterfillObject object;
        
        session->set_source(matrix);
        auto finder = session->make_iterator(min_area);
        const bool keep_object_matrix = false;
        while (finder->find_next(object, keep_object_matrix)){
            if (debug_switch){
                cout << "Find object: area " << object.area << ", save to found_object" << saved_object_id << ".png" << endl;
                ImagePixelBox box(object);
                extract_box_reference(image_crop, box).save("found_object" + std::to_string(saved_object_id++) + ".png");
            }
            if (object.area > max_area){
                continue;
            }

            ImagePixelBox found_arrow_box;
            
            if (debug_switch){
                double rmsd_value = matcher.rmsd(found_arrow_box, image_crop, object);
                cout << "rmsd_value: " << rmsd_value << endl;
                // if (!matcher.check_aspect_ratio(object.width(), object.height())){
                //     cout << "aspect ratio check failed" << endl;
                // }
                // if (!matcher.check_area_ratio(object.area_ratio())){
                //     cout << "area ratio check failed: candidate object " << object.area_ratio() << " template " << matcher.m_subobject_area_ratio << endl;

                // }
            }
            
            if (matcher.matches(found_arrow_box, image_crop, object)){
                if (debug_switch){
                    cout << "detected!!!!!" << endl;
                }
                detected = true;
                break;
            }
        }

        if (detected){
            break;
        }
    }
    return detected;
}

bool BoxDetector::detect(const ImageViewRGB32& screen){
    if (!m_plus_button.detect(screen)){
        return false;
    }
    m_found_row = m_found_col = BoxCursorCoordinates::INVALID;

    bool arrow_found = false;
    for(uint8_t row = 0, cell_idx = 0; row < 6; row++){
        for(uint8_t col = 0; col < 6; col++, cell_idx++){
            ImageViewRGB32 image_crop = extract_box_reference(screen, m_arrow_boxes[cell_idx]);
            // image_crop.save("cell_" + std::to_string(row) + "_" + std::to_string(col) + ".png");
            const uint8_t debug_cell_row = 255, debug_cell_col = 255;
            if (row == debug_cell_row && col == debug_cell_col){
                debug_switch = true;
            }
            bool detected = detect_at_cell(image_crop);
            if (row == debug_cell_row && col == debug_cell_col){
                debug_switch = false;
            }
            if (detected){
                // if (arrow_found){
                //     throw FatalProgramException(ErrorReport::SEND_ERROR_REPORT,
                //         "Multiple box selection arrows detected!", nullptr, screen.copy());
                // }
                arrow_found = true;
                m_found_row = row;
                m_found_col = col;
                break;
            }
        }
        if (arrow_found){
            break;
        }
    }
    // cout << "Box detector arrow found ? " << arrow_found << endl;
    if (!arrow_found){
        // screen.save("mac_box_fletchling_shiny_alpha.png");
    }
    return arrow_found;
}

BoxCursorCoordinates BoxDetector::detected_location() const{
    return {m_found_row, m_found_col};
}

void BoxDetector::move_cursor(
    const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    uint8_t row, uint8_t col
){
    WallClock start = current_time();
    while (true){
        if (current_time() - start > std::chrono::seconds(60)){
            dump_image_and_throw_recoverable_exception(
                info, stream, "BoxMoveCursor",
                "Failed to move cursor to desired location after 1 minute."
            );
        }

        // pbf_wait(context, 500ms);
        context.wait_for_all_requests();
        VideoSnapshot screen = stream.video().snapshot();
        this->detect(screen);
        BoxCursorCoordinates current = this->detected_location();
        if (current.row == BoxCursorCoordinates::INVALID || current.col == BoxCursorCoordinates::INVALID){
            // no cursor found. wait a bit
            pbf_wait(context, 100ms);
            continue;
        }

        if (current.row == row && current.col == col){
//            cout << "done!" << endl;
            return;
        }

        // cout << "Current at row " << int(current.row) << " col " << int(current.col) << endl;

        // try move along x axis first
        if (current.col != col){
            uint8_t left_dist, right_dist;
            if (current.col > col){
                left_dist = current.col - col;
                right_dist = col + 6 - current.col;
            } else{
                right_dist = col - current.col;
                left_dist = current.col + 6 - col;
            }
            if (left_dist < right_dist){
                pbf_press_dpad(context, DPAD_LEFT, 20, 30);
                // cout << "move left" << endl;
            } else {
                pbf_press_dpad(context, DPAD_RIGHT, 20, 30);
                // cout << "move right" << endl;
            }
        } else{ // move along y axis
            uint8_t up_dist, down_dist;
            if (current.row > row){
                up_dist = current.row - row;
                down_dist = row + 6 - current.row;
            } else{
                down_dist = row - current.row;
                up_dist = current.row + 6 - row;
            }
            if (up_dist < down_dist){
                pbf_press_dpad(context, DPAD_UP, 20, 30);
                // cout << "move up" << endl;
            } else {
                pbf_press_dpad(context, DPAD_DOWN, 20, 30);
                // cout << "move down" << endl;
            }
        }
    }
}


SomethingInBoxCellDetector::SomethingInBoxCellDetector(Color color, VideoOverlay* overlay)
: m_right_stick_up_down_detector(color, ButtonType::RightStickUpDown, {0.933, 0.381, 0.027, 0.051}, overlay) {}

void SomethingInBoxCellDetector::make_overlays(VideoOverlaySet& items) const{
    m_right_stick_up_down_detector.make_overlays(items);
}

bool SomethingInBoxCellDetector::detect(const ImageViewRGB32& screen){
    bool detected = m_right_stick_up_down_detector.detect(screen);
    // cout << "SomethingInBoxCellDetector detected? " << detected << endl;
    return detected;
}


}
}
}
