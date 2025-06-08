/*  PokemonSV Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include <QDir>
#include <QFileInfo>
#include <QString>

#include "Common/Compiler.h"
#include "PokemonSV_Tests.h"
#include "TestUtils.h"

#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxDetection.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxEggDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxGenderDetector.h"
#include "PokemonSV/Inference/Boxes/PokemonSV_BoxShinyDetector.h"
#include "PokemonSV/Inference/Map/PokemonSV_MapDetector.h"
#include "PokemonSV/Inference/Map/PokemonSV_MapMenuDetector.h"
#include "PokemonSV/Inference/Map/PokemonSV_MapPokeCenterIconDetector.h"
#include "PokemonSV/Inference/Map/PokemonSV_FastTravelDetector.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraCardDetector.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraSilhouetteReader.h"
#include "PokemonSV/Inference/Tera/PokemonSV_TeraTypeReader.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_PicnicDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichRecipeDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichHandDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichIngredientDetector.h"
#include "PokemonSV/Inference/Picnics/PokemonSV_SandwichPlateDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_LetsGoKillDetector.h"
#include "PokemonSV/Inference/Overworld/PokemonSV_OverworldDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV/Inference/PokemonSV_ESPEmotionDetector.h"

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

    WhiteButtonDetector map_exit_detector(COLOR_RED, WhiteButton::ButtonY, {0.800, 0.118, 0.030, 0.060});
    bool result_map = map_exit_detector.detect(image);

    TEST_RESULT_EQUAL(result_map, target_map_existence);

    if (result_map){

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

int test_pokemonSV_SandwichRecipeDetector(const ImageViewRGB32& image, const std::vector<std::string>& words){
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
    bool result = selection_watcher.detect(image);

    TEST_RESULT_COMPONENT_EQUAL(result, true, "SandwichRecipeSelectionWatcher::process_frame() result");

    int selected_cell = selection_watcher.selected_recipe_cell();

    TEST_RESULT_COMPONENT_EQUAL(selected_cell, target_selection, "selected cell");

    return 0;
}

// - the last 4 words should be the coordinates for the FloatBox, that the detector will search
// - the 5th word from last should be the Hand type (Free or Grabbing)
// - optional: if the image should not detect the hand, the 6th word from last should "False"
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
    }else if (hand_type_word == "Grabbing"){
        hand_type = SandwichHandLocator::HandType::GRABBING;
    }else{
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

    bool hand_expected = true;
    if (words.size() >= 6){
        auto hand_expected_word = words[words.size() - 6];
        if(hand_expected_word == "False"){
            hand_expected = false;
        }
    }

    auto result = detector.detect(image);
    bool has_hand = result.first >= 0.0;

    TEST_RESULT_EQUAL(has_hand, hand_expected);

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

int test_pokemonSV_SandwichIngredientsDetector(const ImageViewRGB32& image, const std::vector<std::string>& words){
    // three words: <current ingredient page "Fillings", "Condiments" or "Picks"> <how many fillings determined> <how many condiments determined>
    if (words.size() < 3){
        cerr << "Error: not enough number of words in the filename. Found only " << words.size() << "." << endl;
        return 1;
    }

    std::string target_type = words[words.size() - 3];
    bool is_condiments = target_type == "Condiments";
    bool is_picks = target_type == "Picks";
    if (!is_condiments && !is_picks && target_type != "Fillings"){
        cerr << "Error: word " << words[words.size()-3] << " is wrong. Must be \"Fillings\", \"Condiments\" or \"Picks\"." << endl;
        return 1;
    }
    int num_fillings = 0, num_condiments = 0;
    if (parse_int(words[words.size()-2], num_fillings) == false){
        cerr << "Error: word " << words[words.size()-2] << " is wrong. Must be int of range [0, 6]. " << endl;
        return 1;
    }
    if (parse_int(words[words.size()-1], num_condiments) == false){
        cerr << "Error: word " << words[words.size()-1] << " is wrong. Must be int of range [0, 4]. " << endl;
        return 1;
    }
    
    SandwichCondimentsPageDetector condiments_detector;
    SandwichPicksPageDetector picks_detector;

    bool condi_result = condiments_detector.detect(image);
    TEST_RESULT_COMPONENT_EQUAL(condi_result, is_condiments, "condiments Page");

    bool picks_result = picks_detector.detect(image);
    TEST_RESULT_COMPONENT_EQUAL(picks_result, is_picks, "picks Page");

    for(int i = 0; i < 10; i++){
        auto type = (i < 6 ? SandwichIngredientType::FILLING : SandwichIngredientType::CONDIMENT);
        size_t index = (i < 6 ? i : i - 6);
        DeterminedSandwichIngredientDetector determined_detector(type, index);
        bool target = (i < 6 ? i < num_fillings : i - 6 < num_condiments);
        bool result = determined_detector.detect(image);

        TEST_RESULT_COMPONENT_EQUAL(result, target, "ingredient slot " + std::to_string(i));
    }

    return 0;
}

int test_pokemonSV_SandwichIngredientReader(const std::string& filepath){
    // three words: <current ingredient page "Fillings" or "Condiments"> <language> <current selected ingredient index 0 to 9>
    
    // the target ingredient list is stored in an auxiliary txt file with filename: _<filepath_basename>.txt

    const QString full_path(QString::fromStdString(filepath));
    const QFileInfo fileinfo(full_path);
    const QString filename = fileinfo.fileName();
    const QDir parent_dir = fileinfo.dir();

    const std::string base_name = fileinfo.baseName().toStdString();
    const std::vector<std::string> words = parse_words(base_name);

    if (words.size() < 3){
        cerr << "Error: not enough number of words in the filename. Found only " << words.size() << "." << endl;
        return 1;
    }
    std::string target_type = words[words.size() - 3];
    SandwichIngredientType sandwich_type;
    if (target_type == "Fillings"){
        sandwich_type = SandwichIngredientType::FILLING;
    }else if (target_type == "Condiments"){
        sandwich_type = SandwichIngredientType::CONDIMENT;
    }else{
        return 1;
    }

    Language language = language_code_to_enum(words[words.size() - 2]);
    if (language == Language::None || language == Language::EndOfList){
        cerr << "Error: language word " << words[words.size() - 2] << " is wrong." << endl;
        return 1;
    }

    size_t selected_ingredient = 0;
    if (parse_size_t(words[words.size() - 1], selected_ingredient) == false){
        cerr << "Error: word " << words[words.size() - 1] << " is wrong. Must be int of range [0, 9]. " << endl;
        return 1;
    }

    const QString target_ingredients_path = parent_dir.filePath("_" + fileinfo.baseName() + ".txt");
    std::vector<std::string> target_ingredients;
    if (load_slug_list(target_ingredients_path.toStdString(), target_ingredients) == false){
        return 1;
    }
    if (target_ingredients.size() != 10){
        cerr << "Error: need to have exactly 10 ingredients in " << target_ingredients_path.toStdString() << endl;
    }

    ImageRGB32 image(filepath);

    SandwichIngredientReader reader(sandwich_type);
    for (size_t i = 0; i < 10; ++i){
        if (selected_ingredient == i){
            // The icon matcher only works on the selected item, because we want to remove the yellow / orange background
            ImageMatch::ImageMatchResult results = reader.read_ingredient_page_with_icon_matcher(image, i);

            if (results.results.empty()){
                cerr << "No ingredient detected via icon matcher" << endl;
                return 1;
            }
            std::string best_match_icon_matcher = results.results.begin()->second;
            TEST_RESULT_COMPONENT_EQUAL(best_match_icon_matcher, target_ingredients[i], "image matcher : ingredient slot " + std::to_string(i));
        }
        {
            OCR::StringMatchResult results = reader.read_ingredient_page_with_ocr(image, global_logger_command_line(), language, i);

            if (results.results.empty()){
                cerr << "No ingredient detected via text" << endl;
                return 1;
            }
            std::string best_match_ocr = results.results.begin()->second.token;
            TEST_RESULT_COMPONENT_EQUAL(best_match_ocr, target_ingredients[i], "ocr : ingredient slot " + std::to_string(i));
        }
    }

    return 0;
}

int test_pokemonSV_AdvanceDialogDetector(const ImageViewRGB32& image, bool target){
    AdvanceDialogDetector detector(COLOR_RED);
    bool result = detector.detect(image);
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonSV_SwapMenuDetector(const ImageViewRGB32& image, bool target){
    SwapMenuDetector detector(COLOR_RED);
    bool result = detector.detect(image);
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonSV_DialogBoxDetector(const ImageViewRGB32& image, bool target){
    DialogBoxDetector detector(COLOR_RED);
    bool result = detector.detect(image);
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonSV_FastTravelDetector(const ImageViewRGB32& image, bool target){
    FastTravelDetector detector(COLOR_RED, MINIMAP_AREA);
    bool result = detector.detect(image);
    TEST_RESULT_EQUAL(result, target);

    return 0;
}

int test_pokemonSV_MapPokeCenterIconDetector(const ImageViewRGB32& image, int target){
    MapPokeCenterIconDetector detector(COLOR_RED, MAP_READABLE_AREA);
    const auto result = detector.detect_all(image);
    for(const auto& box : result){
        std::cout << "Box: x=" << box.x << ", y=" << box.y << ", width=" << box.width << ", height=" << box.height << std::endl;
    }
    TEST_RESULT_EQUAL(int(result.size()), target);

    // auto new_image = image.copy();
    // for (const auto& box : result){
    //     auto p_box = floatbox_to_pixelbox(image.width(), image.height(), box);
    //     draw_box(new_image, p_box, uint32_t(COLOR_BLUE));
    // }
    // new_image.save("./test_pokecenter.png");
    return 0;
}

int test_pokemonSV_ESPPressedEmotionDetector(const ImageViewRGB32& image, bool target){
    ESPPressedEmotionDetector detector;
    bool result = detector.detect(image);
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonSV_MapFlyMenuDetector(const ImageViewRGB32& image, bool target){
    MapFlyMenuDetector fly_menu(COLOR_RED);
    MapDestinationMenuDetector dest_menu(COLOR_RED);
    bool result = fly_menu.detect(image);
    TEST_RESULT_EQUAL(result, target);
    result = dest_menu.detect(image);
    TEST_RESULT_EQUAL(result, !target);
    return 0;
}

int test_pokemonSV_SandwichPlateDetector(const ImageViewRGB32& image, const std::vector<std::string>& words){
    // four words: <language> <left plate filling slug> <middle plate filling slug> <right plate filling slug>
    // if any plate is not present, its word is "none"
    // if any plate label is highlighted as yellow, its word is "Yellow".
    if (words.size() < 4){
        cerr << "Error: not enough number of words in the filename. Found only " << words.size() << "." << endl;
        return 1;
    }

    Language language = language_code_to_enum(words[words.size() - 4]);
    if (language == Language::None || language == Language::EndOfList){
        cerr << "Error: language word " << words[words.size() - 4] << " is wrong." << endl;
        return 1;
    }

    auto& logger = global_logger_command_line();

    FixedLimitVector<SandwichPlateDetector> detectors(3);
    detectors.emplace_back(logger, COLOR_RED, language, SandwichPlateDetector::Side::LEFT);
    detectors.emplace_back(logger, COLOR_RED, language, SandwichPlateDetector::Side::MIDDLE);
    detectors.emplace_back(logger, COLOR_RED, language, SandwichPlateDetector::Side::RIGHT);
    
    std::string sides[3] = {"left", "middle", "right"};
    for(int i = 0; i < 3; i++){
        bool is_yellow = detectors[i].is_label_yellow(image);

        std::string target = words[words.size()-3 + i];
        
        if (target == "Yellow"){
            TEST_RESULT_COMPONENT_EQUAL(is_yellow, true, "yellow label detection at side: " + sides[i]);
        }else{
            std::string filling = detectors[i].detect_filling_name(image);   
            if (target == "none"){
                target = "";
            }
            TEST_RESULT_COMPONENT_EQUAL(filling, target, "side: " + sides[i]);
        }
    }
    
    return 0;
}

int test_pokemonSV_RecentlyBattledDetector(const ImageViewRGB32& image, bool target){
    LetsGoKillDetector detector(COLOR_RED, {0.23, 0.23, 0.04, 0.20});

    const bool result = detector.detect(image);
    TEST_RESULT_EQUAL(result, target);

    return 0;
}

}
