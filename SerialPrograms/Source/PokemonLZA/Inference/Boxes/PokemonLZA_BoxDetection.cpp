/*  Box Detection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageHSV32.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/ImageMatch/SubObjectTemplateMatcher.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/BinaryImage_FilterRgb32.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "Kernels/Waterfill/Kernels_Waterfill.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Session.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Controllers/Procon/NintendoSwitch_ProController.h"
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
        : SubObjectTemplateMatcher("PokemonLZA/SelectionArrowDown.png", 125)
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

        // Analyze template HSV values to determine green hue range
        // analyze_template_hsv();

//        extract_box_reference(m_matcher.image_template(), objects[0]).save("template.png");
    }

    // Optional custom image validation hook called before rmsd checks.
    // Check if the input image contains green pixels with hue close to 51
    virtual bool check_image(const ImageViewRGB32& input_image) const override{
        // Convert RGB to HSV
        ImageHSV32 input_hsv(input_image);
        ImageViewHSV32 hsv_view(input_hsv);

        // Target hue for green arrow border
        const uint32_t target_hue = 50;
        const uint32_t hue_tolerance = 10;
        const uint32_t min_hue = target_hue - hue_tolerance;
        const uint32_t max_hue = target_hue + hue_tolerance;

        // Minimum saturation and value to avoid detecting grayish/dark pixels as green
        const uint32_t min_saturation = 30;  // At least 30/255 saturation
        const uint32_t min_value = 30;        // At least 30/255 brightness

        // Count pixels with green hue
        size_t green_pixel_count = 0;
        const size_t total_pixels = input_image.width() * input_image.height();

        for (size_t y = 0; y < hsv_view.height(); y++){
            for (size_t x = 0; x < hsv_view.width(); x++){
                uint32_t hsv_pixel = hsv_view.pixel(x, y);
                uint32_t hue = (hsv_pixel >> 16) & 0xFF;
                uint32_t sat = (hsv_pixel >> 8) & 0xFF;
                uint32_t val = hsv_pixel & 0xFF;

                // Check if pixel is green with target hue
                if (hue >= min_hue && hue <= max_hue &&
                    sat >= min_saturation &&
                    val >= min_value){
                    green_pixel_count++;
                }
            }
        }

        // Require at least 33% of pixels to be green
        const size_t min_required_green_pixels = total_pixels / 3;
        const bool has_green = green_pixel_count >= min_required_green_pixels;

        if (debug_switch){
            static int counter = 0;
            cout << "check_image() input image: check_image_input_" << counter << ".png" << endl;
            input_image.save("check_image_input_" + std::to_string(counter++) + ".png");
            cout << "check_image() HSV validation:" << endl;
            cout << "  Green pixels found: " << green_pixel_count << " / " << total_pixels
                 << " (" << (100.0 * green_pixel_count / total_pixels) << "%)" << endl;
            cout << "  Required minimum: " << min_required_green_pixels
                 << " (" << (100.0 * min_required_green_pixels / total_pixels) << "%)" << endl;
            cout << "  Hue range: [" << min_hue << ", " << max_hue << "]" << endl;
            cout << "  Result: " << (has_green ? "PASS" : "FAIL") << endl;
        }

        return has_green;
    };

    static const BoxCellSelectionArrowMatcher& matcher(){
        static BoxCellSelectionArrowMatcher matcher;
        return matcher;
    }
};


class BoxCellSelectionArrowGreenPartMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    BoxCellSelectionArrowGreenPartMatcher();

    static const BoxCellSelectionArrowGreenPartMatcher& instance();
};

BoxCellSelectionArrowGreenPartMatcher::BoxCellSelectionArrowGreenPartMatcher()
    : WaterfillTemplateMatcher(
        "PokemonLZA/SelectionArrowDown.png",
        Color(150, 200, 20),
        Color(200, 250, 50),
        100
    )
{}

const BoxCellSelectionArrowGreenPartMatcher& BoxCellSelectionArrowGreenPartMatcher::instance(){
    static BoxCellSelectionArrowGreenPartMatcher matcher;
    return matcher;
}


BoxDetector::BoxDetector(Color color, VideoOverlay* overlay)
    : m_color(color)
    , m_plus_button(color, ButtonType::ButtonPlus, {0.581, 0.940, 0.310, 0.046}, overlay)
{
    for(size_t row = 0; row < 6; row++){
         double y = (row == 0 ? 0.122 : 0.333 + (0.797 - 0.331)/ 4.0 * (row-1));
        for(size_t col = 0; col < 6; col++){
            double x = 0.058 + col * (0.386 - 0.059)/5.0;
            m_arrow_boxes.emplace_back(x, y, 0.018, 0.026);
            m_lifted_arrow_boxes.emplace_back(x+0.011, y-0.010, 0.023, 0.032);
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
bool BoxDetector::detect_at_cell(const Resolution& screen_resolution, const ImageViewRGB32& image_crop){
    if (m_holding_pokemon){
        // If the box cursor is holding a pokemon, it will be a green downard arrow with white interior.
        // This green arrow may appear on top of the bottom part of a pokemon on the upper row of the box.
        // If the pokemon above has white bottom part, like a regular-color Spewpa, the arrrow white interior
        // blends in with the Spewpa white color and make it impossible to find the arrow interior using
        // waterfill. So we do a waterfill for the green part of the arrow against non-green background here.
        const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
            {combine_rgb(170, 230, 50), combine_rgb(200, 255, 100)},
            {combine_rgb(140, 180, 0), combine_rgb(200, 255, 100)},
        };
        const double screen_rel_size = (screen_resolution.height / 1080.0);
        const size_t min_area = static_cast<size_t>(250 * screen_rel_size * screen_rel_size);
        const double rmsd_threshold = 70.0;
        const bool detected = match_template_by_waterfill(
            screen_resolution,
            image_crop,
            BoxCellSelectionArrowGreenPartMatcher::instance(),
            FILTERS,
            {min_area, SIZE_MAX},
            rmsd_threshold,
            [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
                return true;
            }
        );
        if (detected){
            return true;
        }
    }

    // The box curosr either is not holding a pokemon, or it is holding a pokemon but the background of the cursor
    // happens to be green (bottom part of a green pokemon) failing the arrow green part waterfill detection.
    // In both cases, we try to use waterfill to find the white interior of the arrow and then match the full arrow
    // tempalte image around the found white interior: 

    // The arrow's white interior has color between rgb [220, 220, 220] to [255, 255, 255]
    // The arrow's green border has color between rgb [170, 230, 50] to [190, 255, 80]
    std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {0xff808080, 0xffffffff},
        {0xff909090, 0xffffffff},
        {0xffa0a0a0, 0xffffffff},
        {0xffb0b0b0, 0xffffffff},
        {0xffc0c0c0, 0xffffffff},
        {0xffd0d0d0, 0xffffffff},
        {0xffe0e0e0, 0xffffffff},
        {0xfff0f0f0, 0xffffffff},
    };
    std::vector<PackedBinaryMatrix> matrices = compress_rgb32_to_binary_range(image_crop, filters);

    const size_t total_crop_area = image_crop.width() * image_crop.height();
    const size_t min_area = static_cast<size_t>(total_crop_area / 60.0);
    const size_t max_area = static_cast<size_t>(total_crop_area / 10.0);

    int saved_object_id = 0;
    if (debug_switch){
        cout << "detect_at_cell() area threshold " << min_area << " - " << max_area << endl;
        cout << "input image size " << image_crop.width() << "x" << image_crop.height() << endl;
        cout << "Saving image_crop to input_image_crop.png" << endl;
        image_crop.save("input_image_crop.png");
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
            //  Exclude everything that touches the boundaries.
            if (object.min_x == 0 ||
                object.min_y == 0 ||
                object.max_x >= image_crop.width() ||
                object.max_y >= image_crop.height()
            ){
#if 0
                cout << "object.min_x = " << object.min_x << ", object.min_y = " << object.min_y
                     << ", object.max_x = " << object.max_x << ", object.max_y = " << object.max_y
                     << " : " << image_crop.width() << " x " << image_crop.height() << endl;
#endif
                continue;
            }

            if (debug_switch){
                cout << "Find object: area " << object.area << ", save to found_subobject" << saved_object_id << ".png" << endl;
                ImagePixelBox box(object);
                extract_box_reference(image_crop, box).save("found_subobject" + std::to_string(saved_object_id++) + ".png");
            }
            if (object.area > max_area){
                continue;
            }

            ImagePixelBox found_arrow_box;
            
            if (debug_switch){
                double rmsd_value = matcher.rmsd(found_arrow_box, image_crop, object);
                cout << "rmsd_value: " << rmsd_value << endl;
#if 0
                if (!matcher.check_aspect_ratio(object.width(), object.height())){
                     cout << "aspect ratio check failed" << endl;
                }
                if (!matcher.check_area_ratio(object.area_ratio())){
                    cout << "area ratio check failed: candidate object " << object.area_ratio() << " template " << matcher.m_subobject_area_ratio << endl;
                }
#endif
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
    for (uint8_t row = 0, cell_idx = 0; row < 6; row++){
        for (uint8_t col = 0; col < 6; col++, cell_idx++){
#if 0
            if (row != 3 || col != 5){
                continue;
            }
            cout << "row = " << (int)row << ", col = " << (int)col << endl;
#endif
            const auto& box = (m_holding_pokemon ? m_lifted_arrow_boxes[cell_idx] : m_arrow_boxes[cell_idx]);
            ImageViewRGB32 image_crop = extract_box_reference(screen, box);
            // image_crop.save("cell_" + std::to_string(row) + "_" + std::to_string(col) + ".png");
            const uint8_t debug_cell_row = 255, debug_cell_col = 255;
            if (row == debug_cell_row && col == debug_cell_col){
                debug_switch = true;
                cout << "start debugging switch at " << int(row) << ", " << int(col) << endl;
                PreloadSettings::debug().IMAGE_TEMPLATE_MATCHING = true;
            }
            const bool detected = detect_at_cell(screen.size(), image_crop);
            if (row == debug_cell_row && col == debug_cell_col){
                debug_switch = false;
                PreloadSettings::debug().IMAGE_TEMPLATE_MATCHING = false;
            }
            if (detected){
                if (arrow_found && m_debug_mode){
                    cout << "Multiple box selection arrows detected! First detection (" << int(m_found_row) << ", " << int(m_found_col) << ")"
                         << " second detection (" << int(row) << ", " << int(col) << ")" << endl;
                    throw FatalProgramException(ErrorReport::NO_ERROR_REPORT,
                        "Multiple box selection arrows detected!", nullptr, screen.copy());
                }
                arrow_found = true;
                m_found_row = row;
                m_found_col = col;
                if (!m_debug_mode){
                    break;
                }
            }
        }
        if (arrow_found && !m_debug_mode){
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
    uint8_t row, uint8_t col, bool holding_pokemon
){
    if (row >= 6 || col >= 6){
        throw InternalProgramError(&stream.logger(), "BoxDetector::move_cursor",
            "row or col out of range: " + std::to_string(row) + ", " + std::to_string(col));
    }
    this->holding_pokemon(holding_pokemon);

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
                pbf_press_dpad(context, DPAD_LEFT, 160ms, 240ms);
                // cout << "move left" << endl;
            } else {
                pbf_press_dpad(context, DPAD_RIGHT, 160ms, 240ms);
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
                pbf_press_dpad(context, DPAD_UP, 160ms, 240ms);
                // cout << "move up" << endl;
            } else {
                pbf_press_dpad(context, DPAD_DOWN, 160ms, 240ms);
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
