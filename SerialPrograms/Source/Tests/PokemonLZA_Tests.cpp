/*  PokemonLZA Tests
*
*  From: https://github.com/PokemonAutomation/
*
*/

#include "PokemonLZA_Tests.h"
#include "TestUtils.h"
#include "CommonFramework/Logging/Logger.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_SelectionArrowDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_AlertEyeDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_MainMenuDetector.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_BoxDetection.h"
#include "PokemonLZA/Inference/Boxes/PokemonLZA_BoxInfoDetector.h"
#include <iostream>
#include <map>
using std::cout;
using std::cerr;
using std::endl;


namespace PokemonAutomation{ 


using namespace NintendoSwitch;
using namespace NintendoSwitch::PokemonLZA; 



int test_pokemonZLA_NormalDialogBoxDetector(const ImageViewRGB32& image, bool target){
    auto& logger = global_logger_command_line();
    auto overlay = DummyVideoOverlay();
    const bool stop_on_detected = true;
    NormalDialogDetector detector(logger, overlay, stop_on_detected);
    bool result = detector.process_frame(image, current_time());
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

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
    // Expected filename format: <...>_<row>_<col>_<status>.png
    // Where status is one of: Empty, Shiny, Alpha, ShinyAlpha
    // Examples:
    //   test_2_3_Empty.png -> row 2, col 3, empty cell
    //   test_2_3_Regular.png -> row 2, col 3, non-shiny, non-alpha pokemon
    //   test_2_3_Shiny.png -> row 2, col 3, shiny (non-alpha)
    //   test_2_3_Alpha.png -> row 2, col 3, alpha (non-shiny)
    //   test_2_3_ShinyAlpha.png -> row 2, col 3, shiny alpha

    if (words.size() < 3){
        cerr << "Error: filename must have at least 3 words (row, col, status)." << endl;
        return 1;
    }

    // Parse row from third-to-last word
    int expected_row;
    if (parse_int(words[words.size() - 3], expected_row) == false){
        cerr << "Error: third-to-last word in filename should be row number (0-5)." << endl;
        return 1;
    }
    if (expected_row < 0 || expected_row > 5){
        cerr << "Error: row must be between 0 and 5, got " << expected_row << "." << endl;
        return 1;
    }

    // Parse col from second-to-last word
    int expected_col;
    if (parse_int(words[words.size() - 2], expected_col) == false){
        cerr << "Error: second-to-last word in filename should be col number (0-5)." << endl;
        return 1;
    }
    if (expected_col < 0 || expected_col > 5){
        cerr << "Error: col must be between 0 and 5, got " << expected_col << "." << endl;
        return 1;
    }

    // Parse status from last word
    const std::string& status_word = words[words.size() - 1];
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
        cerr << "Error: last word must be 'Empty', 'Shiny', 'Alpha', or 'ShinyAlpha', got '" << status_word << "'." << endl;
        return 1;
    }

    // Run detectors
    auto overlay = DummyVideoOverlay();

    // Test BoxDetector for row and col
    BoxDetector box_detector(COLOR_RED, &overlay);
    box_detector.set_debug_mode(true);
    bool in_box_system = box_detector.detect(image);
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

    return 0;
}


}
