/*  PokemonLZA Tests
*
*  From: https://github.com/PokemonAutomation/
*
*/

#include "Common/Cpp/Time.h"
#include "Common/Cpp/Strings/Unicode.h"
#include "CommonFramework/Logging/Logger.h"
#include "PokemonLZA/Inference/Donuts/PokemonLZA_DonutBerriesDetector.h"
#include "PokemonLZA/Inference/Donuts/PokemonLZA_FlavorPowerDetector.h"
#include "PokemonLZA/Inference/Donuts/PokemonLZA_FlavorPowerScreenDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_AlertEyeDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_MainMenuDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_HyperspaceCalorieDetector.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_BoxDetection.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_BoxInfoDetector.h"
#include "PokemonLZA/Inference/Map/PokemonLZA_MapIconDetector.h"
#include "PokemonLZA/Inference/Map/PokemonLZA_MapDetector.h"
#include "PokemonLZA/Inference/Map/PokemonLZA_DirectionArrowDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_OverworldPartySelectionDetector.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "SerialPrograms/Source/CommonFramework/GlobalSettingsPanel.h"
#include "PokemonLZA_Tests.h"
#include "TestUtils.h"
#include <iostream>
#include <fstream>
#include <map>
#include <filesystem>
using std::cout;
using std::cerr;
using std::endl;


namespace PokemonAutomation{ 


using namespace NintendoSwitch;
using namespace NintendoSwitch::PokemonLZA; 



int test_pokemonLZA_FlatWhiteDialogDetector(const ImageViewRGB32& image, bool target){
    auto overlay = DummyVideoOverlay();
    FlatWhiteDialogDetector detector(COLOR_RED, &overlay);
    bool result = detector.detect(image);
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonLZA_BlueDialogDetector(const ImageViewRGB32& image, bool target){
    auto overlay = DummyVideoOverlay();
    BlueDialogDetector detector(COLOR_RED, &overlay);
    bool result = detector.detect(image);
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonLZA_TransparentBattleDialogDetector(const ImageViewRGB32& image, bool target){
    auto overlay = DummyVideoOverlay();
    TransparentBattleDialogDetector detector(COLOR_RED, &overlay);
    bool result = detector.detect(image);
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonLZA_ButtonDetector(const ImageViewRGB32& image, const std::vector<std::string>& words){
    // two words: <button type> <True/False>
    if (words.size() < 2){
        cerr << "Error: not enough number of words in the filename. Found only " << words.size() << "." << endl;
        return 1;
    }

    // Parse button type from second to last word
    const std::string& button_type_str = words[words.size() - 2];
    ButtonType button_type;

    if (button_type_str == "ButtonA"){
        button_type = ButtonType::ButtonA;
    }else if (button_type_str == "ButtonB"){
        button_type = ButtonType::ButtonB;
    }else if (button_type_str == "ButtonX"){
        button_type = ButtonType::ButtonX;
    }else if (button_type_str == "ButtonY"){
        button_type = ButtonType::ButtonY;
    }else if (button_type_str == "ButtonL"){
        button_type = ButtonType::ButtonL;
    }else if (button_type_str == "ButtonR"){
        button_type = ButtonType::ButtonR;
    }else if (button_type_str == "ButtonPlus"){
        button_type = ButtonType::ButtonPlus;
    }else if (button_type_str == "ButtonMinus"){
        button_type = ButtonType::ButtonMinus;
    }else if (button_type_str == "ButtonRight"){
        button_type = ButtonType::ButtonRight;
    }else{
        cerr << "Error: unknown button type '" << button_type_str << "' in filename." << endl;
        return 1;
    }

    // Parse True/False from last word
    bool target = false;
    if (parse_bool(words[words.size() - 1], target) == false){
        cerr << "Error: last word in filename should be True or False." << endl;
        return 1;
    }

    // Run detector with the full image as the search box
    auto overlay = DummyVideoOverlay();
    ButtonDetector detector(COLOR_RED, button_type, ImageFloatBox(0.0, 0.0, 1.0, 1.0), &overlay);
    bool result = detector.detect(image);

    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonLZA_MainMenuDetector(const ImageViewRGB32& image, bool target){
    auto overlay = DummyVideoOverlay();
    MainMenuDetector detector(COLOR_RED, &overlay);
    bool result = detector.detect(image);
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonLZA_AlertEyeDetector(const ImageViewRGB32& image, bool target){
    auto overlay = DummyVideoOverlay();
    AlertEyeDetector detector(COLOR_RED, &overlay);
    bool result = detector.detect(image);
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonLZA_SelectionArrowDetector(const ImageViewRGB32& image, const std::vector<std::string>& words){
    // two words: <situation> <True/False>
    if (words.size() < 2){
        cerr << "Error: not enough number of words in the filename. Found only " << words.size() << "." << endl;
        return 1;
    }

    // Parse situation from second to last word
    const std::string& situation_str = words[words.size() - 2];
    SelectionArrowType arrow_type;
    ImageFloatBox search_box;

    if (situation_str == "Fossil"){
        arrow_type = SelectionArrowType::RIGHT;
        search_box = ImageFloatBox(0.6300, 0.4440, 0.2260, 0.3190);
    }else{
        cerr << "Error: unknown situation '" << situation_str << "' in filename." << endl;
        return 1;
    }

    // Parse True/False from last word
    bool target = false;
    if (parse_bool(words[words.size() - 1], target) == false){
        cerr << "Error: last word in filename should be True or False." << endl;
        return 1;
    }

    // Run detector with the specified search box and arrow type
    auto overlay = DummyVideoOverlay();
    SelectionArrowDetector detector(COLOR_RED, &overlay, arrow_type, search_box);
    bool result = detector.detect(image);

    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonLZA_BoxCellInfoDetector(const ImageViewRGB32& image, const std::vector<std::string>& words){
    // Expected filename format: <...>_<row>_<col>_<status>_<dex_info>.png
    // Where status is one of: Empty, Shiny, Alpha, ShinyAlpha and can be followed by "Held"
    // Where dex_info is one of: "None", "L<number>" (Lumiose dex), or "H<number>" (Hyperspace dex)
    // Examples:
    //   test_2_3_Empty_None.png -> row 2, col 3, empty cell, no dex number
    //   test_2_3_Regular_L25.png -> row 2, col 3, non-shiny, non-alpha pokemon, Lumiose dex #25
    //   test_2_3_Shiny_H100.png -> row 2, col 3, shiny (non-alpha), Hyperspace dex #100
    //   test_2_3_Alpha_L50.png -> row 2, col 3, alpha (non-shiny), Lumiose dex #50
    //   test_2_3_ShinyAlphaHeld_H75.png -> row 2, col 3, shiny alpha and holding a pokemon, Hyperspace dex #75

    if (words.size() < 4){
        cerr << "Error: filename must have at least 4 words (row, col, status, dex_info)." << endl;
        return 1;
    }

    // Parse row from fourth-to-last word
    int expected_row;
    if (parse_int(words[words.size() - 4], expected_row) == false){
        cerr << "Error: fourth-to-last word in filename should be row number (0-5)." << endl;
        return 1;
    }
    if (expected_row < 0 || expected_row > 5){
        cerr << "Error: row must be between 0 and 5, got " << expected_row << "." << endl;
        return 1;
    }

    // Parse col from third-to-last word
    int expected_col;
    if (parse_int(words[words.size() - 3], expected_col) == false){
        cerr << "Error: third-to-last word in filename should be col number (0-5)." << endl;
        return 1;
    }
    if (expected_col < 0 || expected_col > 5){
        cerr << "Error: col must be between 0 and 5, got " << expected_col << "." << endl;
        return 1;
    }

    // Parse status from second-to-last word
    std::string status_word = words[words.size() - 2];
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
    } else {
        cerr << "Error: second-to-last word must be 'Empty', 'Shiny', 'Alpha', or 'ShinyAlpha', got '" << status_word << "'." << endl;
        return 1;
    }

    // Parse dex info from last word
    std::string dex_info_word = words[words.size() - 1];
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
            cerr << "Error: invalid dex number in '" << dex_info_word << "'. Expected format: L<number> or H<number>." << endl;
            return 1;
        }
        expected_dex_number = static_cast<uint16_t>(dex_num_int);
    } else {
        cerr << "Error: last word must be 'None', 'L<number>', or 'H<number>', got '" << dex_info_word << "'." << endl;
        return 1;
    }

    // Run detectors
    auto overlay = DummyVideoOverlay();

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
        cerr << "Error: BoxDetector did not detect box system view." << endl;
        return 1;
    }

    BoxCursorCoordinates coords = box_detector.detected_location();
    if (coords.row == BoxCursorCoordinates::INVALID || coords.col == BoxCursorCoordinates::INVALID){
        cerr << "Error: detect_location() returned INVALID coordinates." << endl;
        return 1;
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
            cerr << "Error: BoxDexNumberDetector failed to detect dex number." << endl;
            return 1;
        }

        DexType detected_dex_type = dex_detector.dex_type();
        uint16_t detected_dex_number = dex_detector.dex_number();

        std::string expected_dex_type_str = (expected_dex_type == DexType::LUMIOSE) ? "Lumiose" : "Hyperspace";
        std::string detected_dex_type_str = (detected_dex_type == DexType::LUMIOSE) ? "Lumiose" : "Hyperspace";

        if (detected_dex_type != expected_dex_type){
            cerr << "Error: dex type mismatch. Expected " << expected_dex_type_str
                 << " but detected " << detected_dex_type_str << "." << endl;
            return 1;
        }

        TEST_RESULT_COMPONENT_EQUAL((int)detected_dex_number, (int)expected_dex_number, "dex_number");
    }

    return 0;
}

int test_pokemonLZA_MapIconDetector(const std::string& filepath){
    // The target detection results are stored in an auxiliary txt file with filename: _<filepath_basename>.txt
    // Each line in the txt file has format: "<MapIconType> <count>"
    // For example: "PokemonCenter 2" means there should be 2 Pokemon Centers detected

    std::filesystem::path file_path(utf8_to_utf8(filepath));
    std::filesystem::path parent_dir = file_path.parent_path();
    std::string base_name = utf8_to_str(file_path.stem().generic_u8string());

    std::filesystem::path target_detections_path = parent_dir / utf8_to_utf8("_" + base_name + ".txt");

    // Load expected detections from txt file
    std::map<MapIconType, int> expected_counts;
    std::ifstream file(target_detections_path);
    if (!file.is_open()){
        cerr << "Error: cannot open target detection file " << target_detections_path << endl;
        return 1;
    }

    std::string line;
    while (std::getline(file, line)){
        // Skip empty lines
        if (line.empty()){
            continue;
        }

        // Parse line: "<MapIconType> <count>"
        size_t space_pos = line.find(' ');
        if (space_pos == std::string::npos){
            cerr << "Error: invalid line format in " << target_detections_path << ": " << line << endl;
            return 1;
        }

        std::string type_str = line.substr(0, space_pos);
        std::string count_str = line.substr(space_pos + 1);

        int count = 0;
        if (parse_int(count_str, count) == false){
            cerr << "Error: invalid count in " << target_detections_path << ": " << count_str << endl;
            return 1;
        }

        try{
            MapIconType type = string_to_map_icon_type(type_str);
            expected_counts[type] = count;
        }catch (const std::exception&){
            cerr << "Error: unknown MapIconType in " << target_detections_path << ": " << type_str << endl;
            return 1;
        }
    }
    file.close();

    // Load image and run detector for each expected icon type
    ImageRGB32 image(filepath);

// #define SAVE_DEBUG_IMAGE
#ifdef SAVE_DEBUG_IMAGE
    auto debug_image = image.copy();
    
    // Color palette for different icon types
    const uint32_t colors[] = {
        uint32_t(COLOR_RED),
        uint32_t(COLOR_GREEN),
        uint32_t(COLOR_BLUE),
        uint32_t(COLOR_YELLOW),
        uint32_t(COLOR_CYAN),
        uint32_t(COLOR_MAGENTA),
        0xFFFF8000, // Orange
        0xFF00FF80, // Spring green
        0xFF8000FF, // Purple
        0xFFFF0080, // Pink
    };
    size_t color_index = 0;
#endif

    std::map<MapIconType, std::vector<PokemonAutomation::DetectedBox>> detected_boxes;
    for (const auto& pair : expected_counts){
        MapIconType icon_type = pair.first;

        // Run detector with the full image as search box
        MapIconDetector detector(COLOR_RED, icon_type, ImageFloatBox(0.0, 0.0, 1.0, 1.0));
        detector.detect(image);

        detected_boxes[icon_type] = detector.last_detected();
        const auto& detections = detected_boxes[icon_type];

        int detected_count = (int)detections.size();
        std::string icon_type_name = map_icon_type_to_string(icon_type);

#ifdef SAVE_DEBUG_IMAGE
        uint32_t box_color = colors[color_index++ % (sizeof(colors) / sizeof(colors[0]))];
#endif
        cout << icon_type_name << ": " << detected_count << endl;
        // Draw detected boxes on debug image
        for (const auto& detection : detections){
            const auto& box = detection.box;
            cout << "  Box: x=" << box.x << ", y=" << box.y
                    << ", width=" << box.width << ", height=" << box.height << endl;
#ifdef SAVE_DEBUG_IMAGE
            draw_box(debug_image, detection.box, box_color);
#endif
        }
    }

#ifdef SAVE_DEBUG_IMAGE
    // Save debug image
    std::filesystem::path debug_path = parent_dir / ("_" + base_name + "_debug.png");
    debug_image.save(debug_path.string());
    cout << "Debug image saved to: " << debug_path << endl;
#endif

    // test detection
    for (const auto& pair : expected_counts){
        MapIconType icon_type = pair.first;
        const auto& detections = detected_boxes[icon_type];
        int detected_count = (int)detections.size();
        std::string icon_type_name = map_icon_type_to_string(icon_type);
        TEST_RESULT_COMPONENT_EQUAL(detected_count, pair.second, icon_type_name);
    }

    return 0;
}

int test_pokemonLZA_OverworldPartySelectionDetector(const ImageViewRGB32& image, const std::vector<std::string>& words){
    // Expected filename format: <...>_<dpad_up_idx>_<dpad_down_idx>.png
    // Where indices are 0-5 for party positions, or 6 for INVALID_PARTY_IDX
    // Examples:
    //   test_0_1.png -> dpad_up_idx = 0, dpad_down_idx = 1
    //   test_6_2.png -> dpad_up_idx = 6 (INVALID), dpad_down_idx = 2
    //   test_1_6.png -> dpad_up_idx = 1, dpad_down_idx = 6 (INVALID)

    if (words.size() < 2){
        cerr << "Error: filename must have at least 2 words (dpad_up_idx, dpad_down_idx)." << endl;
        return 1;
    }

    // Parse dpad_up_idx from second-to-last word
    int expected_up_idx;
    if (parse_int(words[words.size() - 2], expected_up_idx) == false){
        cerr << "Error: second-to-last word in filename should be dpad_up_idx (0-6)." << endl;
        return 1;
    }
    if (expected_up_idx < 0 || expected_up_idx > 6){
        cerr << "Error: dpad_up_idx must be between 0 and 6, got " << expected_up_idx << "." << endl;
        return 1;
    }

    // Parse dpad_down_idx from last word
    int expected_down_idx;
    if (parse_int(words[words.size() - 1], expected_down_idx) == false){
        cerr << "Error: last word in filename should be dpad_down_idx (0-6)." << endl;
        return 1;
    }
    if (expected_down_idx < 0 || expected_down_idx > 6){
        cerr << "Error: dpad_down_idx must be between 0 and 6, got " << expected_down_idx << "." << endl;
        return 1;
    }

    // Run detector
    auto overlay = DummyVideoOverlay();
    OverworldPartySelectionDetector detector(COLOR_RED, &overlay);
    detector.set_debug_mode(true);
    bool detected = detector.detect(image);

    // Check if detection matches expectations
    uint8_t detected_up_idx = detector.dpad_up_idx();
    uint8_t detected_down_idx = detector.dpad_down_idx();

    // If both expected indices are INVALID_PARTY_IDX (6), detection should fail
    if (expected_up_idx == 6 && expected_down_idx == 6){
        if (detected){
            cerr << "Error: detector should not detect anything when both indices are INVALID." << endl;
            return 1;
        }
        TEST_RESULT_EQUAL(detected, false);
        return 0;
    }

    // Otherwise, detection should succeed
    if (!detected){
        cerr << "Error: detector failed to detect party selection screen." << endl;
        return 1;
    }

    TEST_RESULT_COMPONENT_EQUAL((int)detected_up_idx, expected_up_idx, "dpad_up_idx");
    TEST_RESULT_COMPONENT_EQUAL((int)detected_down_idx, expected_down_idx, "dpad_down_idx");

    return 0;
}

int test_pokemonLZA_DirectionArrowDetector(const ImageViewRGB32& image, int target_angle){
    // Expected filename format: <...>_<angle>.png
    // Where angle is an integer in range [0, 360)
    // Examples:
    //   arrow_0.png -> pointing right (0 degrees)
    //   arrow_90.png -> pointing down (90 degrees)
    //   arrow_180.png -> pointing left (180 degrees)
    //   arrow_270.png -> pointing up (270 degrees)

    if (target_angle < 0 || target_angle >= 360){
        cerr << "Error: target angle must be in range [0, 360), got " << target_angle << "." << endl;
        return 1;
    }

    // Run detector
    DirectionArrowDetector detector(COLOR_RED);
    bool detected = detector.detect(image);

    if (!detected){
        cerr << "Error: detector failed to detect arrow in image." << endl;
        return 1;
    }

    double detected_angle = detector.detected_angle_deg();

    cout << "Target angle: " << target_angle << ", Detected angle: " << detected_angle << endl;

    // Allow some tolerance in angle detection (±10 degrees)
    const double tolerance = 10.0;

    // Calculate the angular difference, accounting for wrap-around
    double diff = std::abs(detected_angle - target_angle);
    if (diff > 180.0){
        diff = 360.0 - diff;
    }

    if (diff > tolerance){
        cerr << "Error: detected angle " << detected_angle
             << " differs from target angle " << target_angle
             << " by " << diff << " degrees (tolerance: " << tolerance << " degrees)." << endl;
        return 1;
    }

    cout << "Angle detection successful within tolerance (diff: " << diff << " degrees)." << endl;
    return 0;
}

int test_pokemonLZA_MapDetector(const ImageViewRGB32& image, bool target){
    auto overlay = DummyVideoOverlay();
    MapDetector detector(COLOR_RED, &overlay);
    bool result = detector.detect(image);
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonLZA_HyperspaceCalorieDetector(const ImageViewRGB32& image, int expected_calorie){
    // Expected filename format: <...>_<calorie_number>.png
    // Where calorie_number is an integer in range [1, 9999]
    // Examples:
    //   calorie_100.png -> 100 calories
    //   calorie_1500.png -> 1500 calories
    //   calorie_9999.png -> 9999 calories

    if (expected_calorie <= 0 || expected_calorie > 9999){
        cerr << "Error: expected calorie must be in range [1, 9999], got " << expected_calorie << "." << endl;
        return 1;
    }

    // Run detector
    HyperspaceCalorieDetector detector(global_logger_command_line());
    bool detected = detector.detect(image);

    if (!detected){
        cerr << "Error: detector failed to detect calorie number in image." << endl;
        return 1;
    }

    uint16_t detected_calorie = detector.calorie_number();

    cout << "Expected calorie: " << expected_calorie << ", Detected calorie: " << detected_calorie << endl;

    TEST_RESULT_EQUAL((int)detected_calorie, expected_calorie);

    return 0;
}

int test_pokemonLZA_FlavorPowerScreenDetector(const std::string& filepath){
    // Expected filename format: <...>_<language>.png
    // Where language is a language code (eng, jpn, chiSim, chiTra, etc.)
    // The ground truth power slugs are in _<basename>.txt, one slug per line (3 lines total)
    // Each line can be "empty" (no power) or a power slug like "sparkling-power-ground-3"
    //
    // To generate ground truth files, set GENERATE_TEST_GOLDEN_FILES to true in settings

    std::filesystem::path file_path(filepath);
    std::string filename = file_path.filename().string();
    std::filesystem::path parent_dir = file_path.parent_path();
    std::string base_name = file_path.stem().string();

    const std::vector<std::string> words = parse_words(base_name);

    if (words.size() < 1){
        cerr << "Error: not enough number of words in the filename. Found only " << words.size() << "." << endl;
        return 1;
    }

    std::string code = words[words.size() - 1];
    if (code == "chiSim"){
        code = "chi_sim";
    } else if (code == "chiTra"){
        code = "chi_tra";
    }
    Language language = language_code_to_enum(code);
    if (language == Language::None || language == Language::EndOfList){
        cerr << "Error: language word " << words[words.size() - 1] << " is wrong." << endl;
        return 1;
    }

    ImageRGB32 image(filepath);

    // First check if this is a flavor power screen
    FlavorPowerScreenDetector screen_detector;
    bool is_flavor_screen = screen_detector.detect(image);

    if (!is_flavor_screen){
        cerr << "Error: FlavorPowerScreenDetector did not detect flavor power screen." << endl;
        return 1;
    }

    if (PreloadSettings::debug().GENERATE_TEST_GOLDEN_FILES){
        // Golden file generation mode: detect all powers and write to file
        cout << "Generating golden file for: " << filepath << endl;

        std::filesystem::path target_powers_path = parent_dir / ("_" + base_name + ".txt");
        std::ofstream output_file(target_powers_path);
        if (!output_file.is_open()){
            cerr << "Error: cannot open output file " << target_powers_path << " for writing" << endl;
            return 1;
        }

        for (int i = 0; i < 3; i++){
            FlavorPowerDetector power_detector(global_logger_command_line(), COLOR_RED, language, i);
            std::string power_slug = power_detector.detect_power(image);

            if (power_slug.empty()){
                output_file << "empty" << endl;
                cout << "  Slot " << i << ": empty" << endl;
            } else {
                output_file << power_slug << endl;
                cout << "  Slot " << i << ": " << power_slug << endl;
            }
        }

        output_file.close();
        cout << "Golden file saved to: " << target_powers_path << endl;
        return 0;
    }

    // Normal testing mode: load golden file and verify
    std::filesystem::path target_powers_path = parent_dir / ("_" + base_name + ".txt");
    std::vector<std::string> expected_powers;
    if (load_slug_list(target_powers_path.string(), expected_powers) == false){
        cout << "Loading slug list " << target_powers_path << " failed." << endl;
        return 1;
    }
    if (expected_powers.size() != 3){
        cerr << "Error: need to have exactly 3 power slots in " << target_powers_path << endl;
        return 1;
    }

    for (int i = 0; i < 3; i++){
        FlavorPowerIconDetector power_icon_detector(global_logger_command_line(), i);
        FlavorPowerDetector power_detector(global_logger_command_line(), COLOR_RED, language, i);

        std::string detected_power_slug = power_detector.detect_power(image);
        int detected_power_level = power_icon_detector.detect(image);

        std::string expected_power = expected_powers[i];

        // Check against expected power
        if (expected_power == "empty"){
            if (!detected_power_slug.empty()){
                cerr << "Error: Slot " << i << " - expected empty but OCR detected: " << detected_power_slug << endl;
                return 1;
            }
            if (detected_power_level > 0){
                cerr << "Error: Slot " << i << " - expected empty but icon detector detected level: " << detected_power_level << endl;
                return 1;
            }
        } else {
            TEST_RESULT_COMPONENT_EQUAL(detected_power_slug, expected_power, "power slug for slot " + std::to_string(i));

            // Verify that power level from icon matches the slug
            // power slug is sth like "sparkling-power-ground-3" which ends with the power level character
            if (detected_power_slug.empty()){
                cerr << "Error: Slot " << i << " - OCR detected empty but expected: " << expected_power << endl;
                return 1;
            }
            int expected_level = detected_power_slug.back() - '1' + 1;
            if (detected_power_level != expected_level){
                cerr << "Error: Slot " << i << " - OCR reads power " << detected_power_slug
                     << " but power icon detector gets power level " << detected_power_level << endl;
                return 1;
            }
        }
    }

    return 0;
}

int test_pokemonLZA_DonutBerriesReader(const std::string& filepath){
    // Expected filename format: <...>_<language>_<selected_berry_index>.png
    // Where language is a language code (eng, jpn, etc.)
    // Where selected_berry_index is 0-7 (since there are 8 berry lines on a page)
    // The ground truth berry slugs are in _<basename>.txt, one slug per line (8 lines total)
    //
    // To generate ground truth files, uncomment the following line and rebuild:
    // #define GENERATE_DONUT_BERRIES_GROUND_TRUTH

    std::filesystem::path file_path(filepath);
    std::string filename = file_path.filename().string();
    std::filesystem::path parent_dir = file_path.parent_path();
    std::string base_name = file_path.stem().string();

    const std::vector<std::string> words = parse_words(base_name);

    if (words.size() < 2){
        cerr << "Error: not enough number of words in the filename. Found only " << words.size() << "." << endl;
        return 1;
    }

    std::string code = words[words.size() - 2];
    if (code == "chiSim"){
        code = "chi_sim";
    } else if (code == "chiTra"){
        code = "chi_tra";
    }
    Language language = language_code_to_enum(code);
    if (language == Language::None || language == Language::EndOfList){
        cerr << "Error: language word " << words[words.size() - 2] << " is wrong." << endl;
        return 1;
    }

    size_t selected_berry = 0;
    if (parse_size_t(words[words.size() - 1], selected_berry) == false){
        cerr << "Error: word " << words[words.size() - 1] << " is wrong. Must be int of range [0, 7]. " << endl;
        return 1;
    }
    if (selected_berry >= DonutBerriesReader::BERRY_PAGE_LINES){
        cerr << "Error: selected_berry must be in range [0, " << DonutBerriesReader::BERRY_PAGE_LINES - 1 << "], got " << selected_berry << "." << endl;
        return 1;
    }

    ImageRGB32 image(filepath);

    if (PreloadSettings::debug().GENERATE_TEST_GOLDEN_FILES){
        // Golden file generation mode: read berry names and write to file
        cout << "Generating golden file for: " << filepath << endl;

        std::filesystem::path target_berries_path = parent_dir / ("_" + base_name + ".txt");
        std::ofstream output_file(target_berries_path);
        if (!output_file.is_open()){
            cerr << "Error: cannot open output file " << target_berries_path << " for writing" << endl;
            return 1;
        }

        DonutBerriesReader reader;
        for (size_t i = 0; i < DonutBerriesReader::BERRY_PAGE_LINES; ++i){
            OCR::StringMatchResult results = reader.read_berry_page_with_ocr(image, global_logger_command_line(), language, i);

            if (results.results.empty()){
                cerr << "Warning: No berry detected via OCR at slot " << i << endl;
                output_file << "unknown-berry" << endl;
            } else {
                std::string best_match_ocr = results.results.begin()->second.token;
                output_file << best_match_ocr << endl;
                cout << "  Slot " << i << ": " << best_match_ocr << endl;
            }
        }

        output_file.close();
        cout << "Golden file saved to: " << target_berries_path << endl;
        return 0;
    }
    
    // Normal testing mode: load golden file and verify
    std::filesystem::path target_berries_path = parent_dir / ("_" + base_name + ".txt");
    std::vector<std::string> target_berries;
    if (load_slug_list(target_berries_path.string(), target_berries) == false){
        return 1;
    }
    if (target_berries.size() != DonutBerriesReader::BERRY_PAGE_LINES){
        cerr << "Error: need to have exactly " << DonutBerriesReader::BERRY_PAGE_LINES << " berries in " << target_berries_path << endl;
        return 1;
    }

    DonutBerriesReader reader;
    for (size_t i = 0; i < DonutBerriesReader::BERRY_PAGE_LINES; ++i){
        // Test DonutBerriesSelectionDetector - should only detect at selected_berry index
        DonutBerriesSelectionDetector selection_detector(i);
        bool is_selected = selection_detector.detect(image);
        bool expected_selected = (i == selected_berry);
        TEST_RESULT_COMPONENT_EQUAL(is_selected, expected_selected, "selection detector : berry slot " + std::to_string(i));

        // Test OCR for all berries
        OCR::StringMatchResult results = reader.read_berry_page_with_ocr(image, global_logger_command_line(), language, i);

        if (results.results.empty()){
            cerr << "No berry detected via OCR" << endl;
            return 1;
        }
        std::string best_match_ocr = results.results.begin()->second.token;
        TEST_RESULT_COMPONENT_EQUAL(best_match_ocr, target_berries[i], "ocr : berry slot " + std::to_string(i));
    }

    return 0;
}


}
