/*  PokemonSV Tests
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "Common/Compiler.h"
#include "PokemonSV_Tests.h"
#include "TestUtils.h"

#include "PokemonSV/Inference/Battles/PokemonSV_BattleMenuDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxDetection.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxEggDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxGenderDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxShinyDetector.h"
#include "PokemonSV/Inference/PokemonSV_MapDetector.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraCardDetector.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraSilhouetteReader.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraTypeReader.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_PicnicDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichRecipeDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichHandDetector.h"
#include "PokemonSV/Inference/PokemonSV_OverworldDetector.h"


#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

namespace PokemonAutomation{

using namespace NintendoSwitch::PokemonSV;

int test_pokemonSV_MapDetector(const ImageViewRGB32& image, const std::vector<std::string>& words){
    // two words: <Map ready to exit> <In fixed view (instead of the rotated view)>
    if (words.size() < 2){
        cerr << "Error: not enough number of words in the filename. Found only " << words.size() << "." << endl;
        return 1;
    }

    bool target_map_existence = false;
    if (parse_bool(words[words.size()-2], target_map_existence) == false){
        cerr << "Error: True/False word " << words[words.size()-2] << " is wrong. Must be \"True\" or \"False\"." << endl;
        return 1;
    }

    bool target_is_fixed_view = false;
    if (parse_bool(words[words.size()-1], target_is_fixed_view) == false){
        cerr << "Error: True/False word " << words[words.size()-1] << " is wrong. Must be \"True\" or \"False\"." << endl;
        return 1;
    }

    MapExitDetector map_exit_detector;
    bool result_map = map_exit_detector.detect(image);

    TEST_RESULT_EQUAL(result_map, target_map_existence);

    if (result_map) {

        MapFixedViewDetector map_fixed_view_detector;
        MapRotatedViewDetector map_rotated_view_detected;
        
        bool result_fixed = map_fixed_view_detector.detect(image);
        bool result_rotated = map_rotated_view_detected.detect(image);

        TEST_RESULT_EQUAL(result_fixed, target_is_fixed_view);
        TEST_RESULT_EQUAL(result_rotated, !target_is_fixed_view);
    }

    return 0;
}

int test_pokemonSV_PicnicDetector(const ImageViewRGB32& image, bool target){
    PicnicDetector detector;

    const bool result = detector.detect(image);
    TEST_RESULT_EQUAL(result, target);

    return 0;
}

int test_pokemonSV_TeraCardFinder(const ImageViewRGB32& image, bool target){
    TeraCardWatcher detector(COLOR_RED);

    bool result = detector.process_frame(image, current_time());
    TEST_RESULT_EQUAL(result, false);

    result = detector.process_frame(image, current_time() + std::chrono::milliseconds(250));
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonSV_TerastallizingDetector(const ImageViewRGB32& image, bool target){
    TerastallizingDetector detector(COLOR_RED);

    bool result = detector.detect(image);
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonSV_TeraSilhouetteReader(const ImageViewRGB32& image, const std::vector<std::string>& keywords){
    TeraSilhouetteReader reader(COLOR_RED);
    
    ImageMatch::ImageMatchResult slugs = reader.read(image);
    if (slugs.results.empty()){
        cerr << "No silhouette detected" << endl;
        return 1;
    }
    std::string best_match = slugs.results.begin()->second;

    if (keywords.empty()){
        cerr << "Must provide a pokemon silhouette slug" << endl;
        return 1;
    }
    TEST_RESULT_EQUAL(best_match, keywords[keywords.size() - 1]);

    return 0;
}

int test_pokemonSV_TeraTypeReader(const ImageViewRGB32& image, const std::vector<std::string>& keywords){
    TeraTypeReader reader(COLOR_RED);

    ImageMatch::ImageMatchResult types = reader.read(image);
    if (types.results.empty()){
        return 1;
    }
    std::string best_match = types.results.begin()->second;

    if (keywords.empty()){
        return 1;
    }
    TEST_RESULT_EQUAL(best_match, keywords[keywords.size() - 1]);

    return 0;
}

int test_pokemonSV_SandwichRecipeDetector(const ImageViewRGB32& image, const std::vector<std::string>& words) {
    auto& logger = global_logger_command_line();

    // seven words: the sandwich recipe IDs on the screen. Order:
    // --------------------------------------
    // recipe_IDs[0]  |   recipe_IDs[1]
    // recipe_IDs[2]  |   recipe_IDs[3]
    // recipe_IDs[4]  |   recipe_IDs[5]
    // --------------------------------------
    // plus the current selected cell ID (range [0, 5]).
    if (words.size() < 7){
        cerr << "Error: not enough number of words in the filename. Found only " << words.size() << "." << endl;
        return 1;
    }

    int target_IDs[6] = {0, 0, 0, 0, 0, 0};
    for(int i = 0; i < 6; i++){
        const auto& word = words[words.size() + i - 7];
        if (parse_int(word, target_IDs[i]) == false || target_IDs[i] > 151){
            cerr << "Error: word " << words[words.size() + i - 7] << " is wrong. Must be an integer < 151. " << endl;
            return 1;
        }
    }

    SandwichRecipeNumberDetector detector(logger);

    size_t detected_IDs[6] = {0, 0, 0, 0, 0, 0};
    detector.detect_recipes(image, detected_IDs);

    for(int i = 0; i < 6; i++){
        if (target_IDs[i] < 0){
            continue;
        }
        TEST_RESULT_COMPONENT_EQUAL(detected_IDs[i], (size_t)target_IDs[i], "recipe at cell " + std::to_string(i));
    }

    int target_selection = 0;
    if (parse_int(words[words.size()-1], target_selection) == false || target_selection < 0 || target_selection >= 6){
        cerr << "Error: word " << words[words.size()-1] << " is wrong. Must be an integer in range [0, 6). " << endl;
    }
    SandwichRecipeSelectionWatcher selection_watcher;
    bool result = selection_watcher.process_frame(image, current_time());

    TEST_RESULT_COMPONENT_EQUAL(result, true, "SandwichRecipeSelectionWatcher::process_frame() result");

    int selected_cell = selection_watcher.selected_recipe_cell();

    TEST_RESULT_COMPONENT_EQUAL(selected_cell, target_selection, "selected cell");

    return 0;
}

int test_pokemonSV_SandwichHandDetector(const ImageViewRGB32& image, const std::vector<std::string>& words){
    // five words: hand_type("Free"/"Grabbing"), <image float box (four words total)>
    if (words.size() < 5){
        cerr << "Error: not enough number of words in the filename. Found only " << words.size() << "." << endl;
        return 1;
    }

    const auto& hand_type_word = words[words.size() - 5];
    SandwichHandLocator::HandType hand_type = SandwichHandLocator::HandType::FREE;
    if (hand_type_word == "Free"){
        hand_type = SandwichHandLocator::HandType::FREE;
    } else if (hand_type_word == "Grabbing"){
        hand_type = SandwichHandLocator::HandType::GRABBING;
    } else {
        cerr << "Error: word " << hand_type_word << " should be \"Free\" or \"Grabbing\"." << endl;
        return 1;
    }

    float box_values[4] = {0.0f};
    for(int i = 0; i < 4; i++){
        if (parse_float(words[words.size() - 4 + i], box_values[i]) == false){
            cerr << "Error: word " << words[words.size() - 4 + i] << " should be a float, range [0.0, 1.0]" << endl;
            return 1;
        }
    }

    ImageFloatBox box(box_values[0], box_values[1], box_values[2], box_values[3]);

    SandwichHandLocator detector(hand_type, box);

    auto result = detector.detect(image);
    bool has_hand = result.first >= 0.0;

    TEST_RESULT_EQUAL(has_hand, true);

    return 0;
}

int test_pokemonSV_BoxPokemonInfoDetector(const ImageViewRGB32& image, const std::vector<std::string>& words){
    // two words: <shiny or not> <gender (1: male, 2: female, 3: genderless)
    if (words.size() < 2){
        cerr << "Error: not enough number of words in the filename. Found only " << words.size() << "." << endl;
        return 1;
    }

    bool target_shiny = false;
    int target_gender = 0;
    if (parse_bool(words[words.size()-2], target_shiny) == false){
        cerr << "Error: word " << words[words.size()-2] << " is wrong. Must be True or False. " << endl;
        return 1;
    }
    if (parse_int(words[words.size()-1], target_gender) == false){
        cerr << "Error: word " << words[words.size()-1] << " is wrong. Must be int (1: male, 2: female, 3: genderless). " << endl;
        return 1;
    }

    SomethingInBoxSlotDetector sth_detector(COLOR_RED);
    bool sth = sth_detector.detect(image);
    TEST_RESULT_EQUAL(sth, true);

    BoxShinyDetector shiny_detector;
    bool shiny_result = shiny_detector.detect(image);

    TEST_RESULT_EQUAL(shiny_result, target_shiny);

    BoxGenderDetector gender_detector;
    int gender_result = (int)gender_detector.detect(image);
    TEST_RESULT_EQUAL(gender_result, target_gender);

    return 0;
}

int test_pokemonSV_SomethingInBoxSlotDetector(const ImageViewRGB32& image, bool target){
    SomethingInBoxSlotDetector sth_detector(COLOR_RED);
    bool sth = sth_detector.detect(image);
    TEST_RESULT_EQUAL(sth, target);

    return 0;
}

int test_pokemonSV_BoxEggDetector(const ImageViewRGB32& image, bool target){
    BoxCurrentEggDetector box_egg_detector(COLOR_RED);
    bool egg = box_egg_detector.detect(image);
    TEST_RESULT_EQUAL(egg, target);

    return 0;
}

int test_pokemonSV_BoxPartyEggDetector(const ImageViewRGB32& image, int target){
    uint8_t num_eggs = 0;
    for(uint8_t i = 0; i < 5; i++){
        BoxEggDetector detector(BoxCursorLocation::PARTY, i+1, 0);
        if (detector.detect(image)){
            num_eggs++;
        }
    }

    TEST_RESULT_EQUAL((int)num_eggs, (int)target);
    return 0;
}

int test_pokemonSV_OverworldDetector(const ImageViewRGB32& image, bool target){
    OverworldDetector detector;
    bool result = detector.detect(image);
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonSV_BoxBottomButtonDetector(const ImageViewRGB32& image, const std::vector<std::string>& words){
    // two words: <button Y exists, true or false> <button B exists, true or false)
    if (words.size() < 2){
        cerr << "Error: not enough number of words in the filename. Found only " << words.size() << "." << endl;
        return 1;
    }

    bool target_y = false, target_b = false;
    if (parse_bool(words[words.size()-2], target_y) == false){
        cerr << "Error: word " << words[words.size()-2] << " is wrong. Must be True or False." << endl;
        return 1;
    }
    if (parse_bool(words[words.size()-1], target_b) == false){
        cerr << "Error: word " << words[words.size()-1] << " is wrong. Must be True or False." << endl;
        return 1;
    }

    BoxBottomButtonYDetector y_detector;
    bool result_y = y_detector.detect(image);
    TEST_RESULT_COMPONENT_EQUAL(result_y, target_y, "button Y");

    // BoxBottomButtonBDetector b_detector;
    // bool result_b = b_detector.detect(image);
    // TEST_RESULT_COMPONENT_EQUAL(result_b, target_b, "button B");

    return 0;
}

}
