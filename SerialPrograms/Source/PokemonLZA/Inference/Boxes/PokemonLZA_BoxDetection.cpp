/*  Box Detection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/StaticGlobals.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/ImageTools/ImageStats.h"
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
#include "PokemonLZA/Inference/Boxes/PokemonLZA_BoxInfoDetector.h"
#include "PokemonLZA_BoxDetection.h"
#include "Tests/TestUtils.h"

#include <iostream>
 //#include <sstream>
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
    for (size_t row = 0; row < 6; row++){
        double y = (row == 0 ? 0.122 : 0.333 + (0.797 - 0.331)/ 4.0 * (row-1));
        for (size_t col = 0; col < 6; col++){
            double x = 0.058 + col * (0.386 - 0.059)/5.0;
            m_arrow_boxes.emplace_back(x, y, 0.018, 0.026);
            // m_lifted_arrow_boxes.emplace_back(x+0.011, y-0.010, 0.023, 0.032);
            m_gaps_for_lifted.emplace_back(x+0.011, y+0.010, 0.023, 0.004);
        }
    }
}

void BoxDetector::make_overlays(VideoOverlaySet& items) const{
    m_plus_button.make_overlays(items);
    for (const ImageFloatBox& box : m_arrow_boxes){
        items.add(m_color, box);
    }
}


// detect arrow's white interior first
// then use subobject template matcher BoxCellSelectionArrowMatcher to detect the whole arrow
bool BoxDetector::detect_at_cell(uint8_t cell_idx, const ImageViewRGB32& screen){
#if 0
    const auto& box = (m_holding_pokemon ? m_lifted_arrow_boxes[cell_idx] : m_arrow_boxes[cell_idx]);
    ImageViewRGB32 image_crop = extract_box_reference(screen, box);

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
        const double screen_rel_size = (screen.height() / 1080.0);
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
#else
    if (m_holding_pokemon){
        // If the box cursor is holding a pokemon, it will be a green downward arrow with white interior.
        // This green arrow may appear on top of the bottom part of a pokemon on the upper row of the box.
        // If the pokemon above has white bottom part, like a regular-color Spewpa, the arrrow white interior
        // blends in with the Spewpa white color and make it impossible to find the arrow interior using
        // waterfill. If the held pokemon is green (e.g. shiny Scizor) it will blend with the green part of
        // the arrow, making waterfill impossible. So we have to use this alternative detection:
        // Use the stddev of a cross section of the green-white arrow.
        // If there is an arrow, the bright green and white color makes the stddev quite high (>= 100)
        // while without the arrow, it is the background, which are the blurred overworld view which
        // typically has stddev lower than 50.
        ImageViewRGB32 image_crop = extract_box_reference(screen, m_gaps_for_lifted[cell_idx]);
        const auto stats = image_stats(image_crop);
        return stats.stddev.sum() > 80;
    }

    ImageViewRGB32 image_crop = extract_box_reference(screen, m_arrow_boxes[cell_idx]);
#endif

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
            // image_crop.save("cell_" + std::to_string(row) + "_" + std::to_string(col) + ".png");
            const uint8_t debug_cell_row = STATIC_GLOBALS.BOX_SYSTEM_CELL_ROW;
            const uint8_t debug_cell_col = STATIC_GLOBALS.BOX_SYSTEM_CELL_COL;
            if (row == debug_cell_row && col == debug_cell_col){
                debug_switch = true;
                cout << "start debugging switch at " << int(row) << ", " << int(col) << endl;
                STATIC_GLOBALS.IMAGE_TEMPLATE_MATCHING = true;
            }
            const bool detected = detect_at_cell(cell_idx, screen);
            if (row == debug_cell_row && col == debug_cell_col){
                debug_switch = false;
                STATIC_GLOBALS.IMAGE_TEMPLATE_MATCHING = false;
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
            }else{
                right_dist = col - current.col;
                left_dist = current.col + 6 - col;
            }
            if (left_dist < right_dist){
                pbf_press_dpad(context, DPAD_LEFT, 160ms, 240ms);
                // cout << "move left" << endl;
            }else{
                pbf_press_dpad(context, DPAD_RIGHT, 160ms, 240ms);
                // cout << "move right" << endl;
            }
        }else{ // move along y axis
            uint8_t up_dist, down_dist;
            if (current.row > row){
                up_dist = current.row - row;
                down_dist = row + 6 - current.row;
            }else{
                down_dist = row - current.row;
                up_dist = current.row + 6 - row;
            }
            if (up_dist < down_dist){
                pbf_press_dpad(context, DPAD_UP, 160ms, 240ms);
                // cout << "move up" << endl;
            }else{
                pbf_press_dpad(context, DPAD_DOWN, 160ms, 240ms);
                // cout << "move down" << endl;
            }
        }
    }
}


SomethingInBoxCellDetector::SomethingInBoxCellDetector(Color color, VideoOverlay* overlay)
: m_right_stick_up_down_detector(color, ButtonType::RightStickUpDown, {0.933, 0.381, 0.027, 0.051}, overlay)
, m_left_white_space_box{0.453, 0.100, 0.009, 0.059}, m_right_white_space_box{0.947, 0.107, 0.010, 0.052} {}

void SomethingInBoxCellDetector::make_overlays(VideoOverlaySet& items) const{
    m_right_stick_up_down_detector.make_overlays(items);
    items.add(COLOR_BLACK, m_left_white_space_box);
    items.add(COLOR_BLACK, m_right_white_space_box);
}

bool SomethingInBoxCellDetector::detect(const ImageViewRGB32& screen){
    const double min_white_color_sum = 500.0;
    const double max_white_color_stddev_sum = 15.0;
    bool detected = is_white(extract_box_reference(screen, m_left_white_space_box), min_white_color_sum, max_white_color_stddev_sum);
    if (!detected){
        return false;
    }
    detected = is_white(extract_box_reference(screen, m_right_white_space_box), min_white_color_sum, max_white_color_stddev_sum);
    if (!detected){
        return false;
    }
    return m_right_stick_up_down_detector.detect(screen);
}






class Test_BoxCellInfoDetector : public UnitTest{
public:
    Test_BoxCellInfoDetector(
        const std::string& image,
        std::vector<std::string> words
    )
        : UnitTest("PokemonPLZA::BoxCellInfoDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_words(std::move(words))
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        // Expected filename format: <...>_<row>_<col>_<status>_<dex_info>.png
    // Where status is one of: Empty, Shiny, Alpha, ShinyAlpha and can be followed by "Held"
    // Where dex_info is one of: "None", "L<number>" (Lumiose dex), or "H<number>" (Hyperspace dex)
    // Examples:
    //   test_2_3_Empty_None.png -> row 2, col 3, empty cell, no dex number
    //   test_2_3_Regular_L25.png -> row 2, col 3, non-shiny, non-alpha pokemon, Lumiose dex #25
    //   test_2_3_Shiny_H100.png -> row 2, col 3, shiny (non-alpha), Hyperspace dex #100
    //   test_2_3_Alpha_L50.png -> row 2, col 3, alpha (non-shiny), Lumiose dex #50
    //   test_2_3_ShinyAlphaHeld_H75.png -> row 2, col 3, shiny alpha and holding a pokemon, Hyperspace dex #75

        if (m_words.size() < 4){
            std::stringstream ss;
            ss << "Error: filename must have at least 4 words (row, col, status, dex_info)." << endl;
            return ss.str();
        }

        // Parse row from fourth-to-last word
        int expected_row;
        if (parse_int(m_words[m_words.size() - 4], expected_row) == false){
            std::stringstream ss;
            ss << "Error: fourth-to-last word in filename should be row number (0-5)." << endl;
            return ss.str();
        }
        if (expected_row < 0 || expected_row > 5){
            std::stringstream ss;
            ss << "Error: row must be between 0 and 5, got " << expected_row << "." << endl;
            return ss.str();
        }

        // Parse col from third-to-last word
        int expected_col;
        if (parse_int(m_words[m_words.size() - 3], expected_col) == false){
            std::stringstream ss;
            ss << "Error: third-to-last word in filename should be col number (0-5)." << endl;
            return ss.str();
        }
        if (expected_col < 0 || expected_col > 5){
            std::stringstream ss;
            ss << "Error: col must be between 0 and 5, got " << expected_col << "." << endl;
            return ss.str();
        }

        // Parse status from second-to-last word
        std::string status_word = m_words[m_words.size() - 2];
        bool holding_pokemon = false;
        if (status_word.ends_with("Held")){
            holding_pokemon = true;
            status_word = status_word.substr(0, status_word.size() - 4);
        }
        bool expected_something_in_cell;
        bool expected_shiny;
        bool expected_alpha;

        if (status_word == "Empty"){
            expected_something_in_cell = false;
            expected_shiny = false;
            expected_alpha = false;
        } else if (status_word == "Regular"){
            expected_something_in_cell = true;
            expected_shiny = false;
            expected_alpha = false;
        } else if (status_word == "Shiny"){
            expected_something_in_cell = true;
            expected_shiny = true;
            expected_alpha = false;
        } else if (status_word == "Alpha"){
            expected_something_in_cell = true;
            expected_shiny = false;
            expected_alpha = true;
        } else if (status_word == "ShinyAlpha"){
            expected_something_in_cell = true;
            expected_shiny = true;
            expected_alpha = true;
        } else{
            std::stringstream ss;
            ss << "Error: second-to-last word must be 'Empty', 'Shiny', 'Alpha', or 'ShinyAlpha', got '" << status_word << "'." << endl;
            return ss.str();
        }

        // Parse dex info from last word
        std::string dex_info_word = m_words[m_words.size() - 1];
        bool expect_dex_detection = false;
        DexType expected_dex_type = DexType::LUMIOSE;
        uint16_t expected_dex_number = 0;

        if (dex_info_word == "None"){
            expect_dex_detection = false;
        } else if (dex_info_word.size() >= 2 && (dex_info_word[0] == 'L' || dex_info_word[0] == 'H')){
            expect_dex_detection = true;
            expected_dex_type = (dex_info_word[0] == 'L') ? DexType::LUMIOSE : DexType::HYPERSPACE;

            std::string number_str = dex_info_word.substr(1);
            int dex_num_int;
            if (parse_int(number_str, dex_num_int) == false || dex_num_int <= 0){
                std::stringstream ss;
                ss << "Error: invalid dex number in '" << dex_info_word << "'. Expected format: L<number> or H<number>." << endl;
                return ss.str();
            }
            expected_dex_number = static_cast<uint16_t>(dex_num_int);
        } else{
            std::stringstream ss;
            ss << "Error: last word must be 'None', 'L<number>', or 'H<number>', got '" << dex_info_word << "'." << endl;
            return ss.str();
        }

        // Run detectors
        auto overlay = DummyVideoOverlay();
        ImageRGB32 image(m_image);

        // Test BoxDetector for row and col
        BoxDetector box_detector(COLOR_RED, &overlay);
        box_detector.set_debug_mode(true);
        box_detector.holding_pokemon(holding_pokemon);

        // #define PROFILE_BOX_DETECTION
#ifdef PROFILE_BOX_DETECTION
    // Profile the template matching performance
        const int num_iterations = 100;
        auto time_start = current_time();
        bool in_box_system = false;
        for (int i = 0; i < num_iterations; i++){
            in_box_system = box_detector.detect(image);
        }
        auto time_end = current_time();

        const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_start).count();
        const double ms_total = ns / 1000000.0;
        const double ms_per_iteration = ms_total / num_iterations;

        cout << "BoxDetector::detect() performance:" << endl;
        cout << "  Total time for " << num_iterations << " iterations: " << ms_total << " ms" << endl;
        cout << "  Average time per iteration: " << ms_per_iteration << " ms" << endl;
        cout << "  Throughput: " << (1000.0 / ms_per_iteration) << " detections/second" << endl;
#else
        bool in_box_system = box_detector.detect(image);
#endif

        if (!in_box_system){
            std::stringstream ss;
            ss << "Error: BoxDetector did not detect box system view." << endl;
            return ss.str();
        }

        BoxCursorCoordinates coords = box_detector.detected_location();
        if (coords.row == BoxCursorCoordinates::INVALID || coords.col == BoxCursorCoordinates::INVALID){
            std::stringstream ss;
            ss << "Error: detect_location() returned INVALID coordinates." << endl;
            return ss.str();
        }

        TEST_RESULT_COMPONENT_EQUAL((int)coords.row, expected_row, "row");
        TEST_RESULT_COMPONENT_EQUAL((int)coords.col, expected_col, "col");

        // Test SomethingInBoxCellDetector
        SomethingInBoxCellDetector something_detector(COLOR_RED, &overlay);
        bool detected_something = something_detector.detect(image);
        TEST_RESULT_COMPONENT_EQUAL(detected_something, expected_something_in_cell, "something_in_cell");

        // Test BoxShinyDetector
        BoxShinyDetector shiny_detector(COLOR_RED, &overlay);
        bool detected_shiny = shiny_detector.detect(image);
        TEST_RESULT_COMPONENT_EQUAL(detected_shiny, expected_shiny, "shiny");

        // Test BoxAlphaDetector
        BoxAlphaDetector alpha_detector(COLOR_RED, &overlay);
        bool detected_alpha = alpha_detector.detect(image);
        TEST_RESULT_COMPONENT_EQUAL(detected_alpha, expected_alpha, "alpha");

        // Test BoxDexNumberDetector
        if (expect_dex_detection){
            BoxDexNumberDetector dex_detector(global_logger_command_line());
            bool detected_dex = dex_detector.detect(image);

            if (!detected_dex){
                std::stringstream ss;
                ss << "Error: BoxDexNumberDetector failed to detect dex number." << endl;
                return ss.str();
            }

            DexType detected_dex_type = dex_detector.dex_type();
            uint16_t detected_dex_number = dex_detector.dex_number();

            std::string expected_dex_type_str = (expected_dex_type == DexType::LUMIOSE) ? "Lumiose" : "Hyperspace";
            std::string detected_dex_type_str = (detected_dex_type == DexType::LUMIOSE) ? "Lumiose" : "Hyperspace";

            if (detected_dex_type != expected_dex_type){
                std::stringstream ss;
                ss << "Error: dex type mismatch. Expected " << expected_dex_type_str
                    << " but detected " << detected_dex_type_str << "." << endl;
                return ss.str();
            }

            TEST_RESULT_COMPONENT_EQUAL((int)detected_dex_number, (int)expected_dex_number, "dex_number");
        }

        return true;
    };

private:
    std::string m_image;
    std::vector<std::string> m_words;

};


void add_tests_BoxCellInfoDetector(UnitTestDatabase& database){
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/french_box_gyarados_2_1_Regular_L033.jpg", std::vector<std::string>{"2", "1", "Regular", "L033"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/french_box_haunter_1_4_Shiny_L066.jpg", std::vector<std::string>{"1", "4", "Shiny", "L066"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/french_box_lopunny_1_1_Alpha_L110.jpg", std::vector<std::string>{"1", "1", "Alpha", "L110"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/french_box_ralts_1_0_ShinyAlpha_L087.jpg", std::vector<std::string>{"1", "0", "ShinyAlpha", "L087"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/mac_box_0_0_Empty_None.jpg", std::vector<std::string>{"0", "0", "Empty", "None"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/mac_box_0_4_Empty_None.jpg", std::vector<std::string>{"0", "4", "Empty", "None"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/mac_box_1_0_Empty_None.jpg", std::vector<std::string>{"1", "0", "Empty", "None"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/mac_box_1_0_Regular_H014.jpg", std::vector<std::string>{"1", "0", "Regular", "H014"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/mac_box_2_5_Empty_None.jpg", std::vector<std::string>{"2", "5", "Empty", "None"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/mac_box_4_4_Empty_None.jpg", std::vector<std::string>{"4", "4", "Empty", "None"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/mac_box_clauncher_3_2_Shiny_L163.jpg", std::vector<std::string>{"3", "2", "Shiny", "L163"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/mac_box_dragonite_0_5_Regular_L147.jpg", std::vector<std::string>{"0", "5", "Regular", "L147"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/mac_box_flabebe_1_0_Alpha_L038.jpg", std::vector<std::string>{"1", "0", "Alpha", "L038"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/mac_box_fletchling_1_5_ShinyAlpha_L010.jpg", std::vector<std::string>{"1", "5", "ShinyAlpha", "L010"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/mac_box_florges_5_5_Regular_L040.jpg", std::vector<std::string>{"5", "5", "Regular", "L040"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/mac_box_furfrou_4_2_Regular_L158.jpg", std::vector<std::string>{"4", "2", "Regular", "L158"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/mac_box_gardevoir_0_0_Regular_L089.jpg", std::vector<std::string>{"0", "0", "Regular", "L089"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/mac_box_greninja_1_5_ShinyAlpha_L211.jpg", std::vector<std::string>{"1", "5", "ShinyAlpha", "L211"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/mac_box_hippopotas_3_3_Alpha_L118.jpg", std::vector<std::string>{"3", "3", "Alpha", "L118"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/mac_box_igglybuff_2_3_Regular_H076.jpg", std::vector<std::string>{"2", "3", "Regular", "H076"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/mac_box_magikarp_2_0_Regular_L032.jpg", std::vector<std::string>{"2", "0", "Regular", "L032"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/mac_box_mareep_3_4_Shiny_L024.jpg", std::vector<std::string>{"3", "4", "Shiny", "L024"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/mac_box_pyroar_0_5_ShinyAlpha_L046.jpg", std::vector<std::string>{"0", "5", "ShinyAlpha", "L046"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/mac_box_pyroar_1_0_ShinyAlpha_L046.jpg", std::vector<std::string>{"1", "0", "ShinyAlpha", "L046"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/mac_box_tyrunt_1_0_Regular_L193.jpg", std::vector<std::string>{"1", "0", "Regular", "L193"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/mac_box_victreebel_1_3_Alpha_L076.jpg", std::vector<std::string>{"1", "3", "Alpha", "L076"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/Bug/box_dragalge_0_5_Alpha_L162.jpg", std::vector<std::string>{"0", "5", "Alpha", "L162"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/Bug/box_greninja_0_4_ShinyAlpha_L211.jpg", std::vector<std::string>{"0", "4", "ShinyAlpha", "L211"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/Bug/box_scolipede_0_3_Shiny_L070.jpg", std::vector<std::string>{"0", "3", "Shiny", "L070"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/dhruv/box_2_2_ShinyHeld_H067.jpg", std::vector<std::string>{"2", "2", "ShinyHeld", "H067"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/dhruv/box_tinkatonAbove_2_2_ShinyHeld_H067.jpg", std::vector<std::string>{"2", "2", "ShinyHeld", "H067"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/dolphincurry/box_tyrunt_no1_1_0_Regular_L193.jpg", std::vector<std::string>{"1", "0", "Regular", "L193"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/dolphincurry/box_tyrunt_no2_1_0_Regular_L193.jpg", std::vector<std::string>{"1", "0", "Regular", "L193"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/dolphincurry/box_tyrunt_no3_1_0_Regular_L193.jpg", std::vector<std::string>{"1", "0", "Regular", "L193"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/held/mac_box_0_0_AlphaHeld_L227.jpg", std::vector<std::string>{"0", "0", "AlphaHeld", "L227"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/held/mac_box_1_5_RegularHeld_H050.jpg", std::vector<std::string>{"1", "5", "RegularHeld", "H050"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/held/mac_box_2_1_ShinyHeld_L111.jpg", std::vector<std::string>{"2", "1", "ShinyHeld", "L111"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/held/mac_box_2_4_EmptyHeld_None.jpg", std::vector<std::string>{"2", "4", "EmptyHeld", "None"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/held/mac_box_5_5_EmptyHeld_None.jpg", std::vector<std::string>{"5", "5", "EmptyHeld", "None"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/k3lpoke/bright_box_gourgeist_4_2_ShinyAlpha_L205.jpg", std::vector<std::string>{"4", "2", "ShinyAlpha", "L205"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/k3lpoke/bright_box_pangoro_1_2_Alpha_L048.jpg", std::vector<std::string>{"1", "2", "Alpha", "L048"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/k3lpoke/bright_box_pumpkaboo_4_3_Shiny_L204.jpg", std::vector<std::string>{"4", "3", "Shiny", "L204"});
    database.add<Test_BoxCellInfoDetector>("PokemonLZA/BoxCellInfoDetector/Quantum/box_2_3_ShinyHeld_L172.jpg", std::vector<std::string>{"2", "3", "ShinyHeld", "L172"});
}



}
}
}
