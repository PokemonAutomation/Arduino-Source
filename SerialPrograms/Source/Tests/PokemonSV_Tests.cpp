/*  PokemonSV Tests
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "Common/Compiler.h"
#include "PokemonSV_Tests.h"
#include "TestUtils.h"

#include "PokemonSV/Inference/PokemonSV_TeraCardDetector.h"
#include "PokemonSV/Inference/PokemonSV_BattleMenuDetector.h"
#include "PokemonSV/Inference/PokemonSV_MapDetector.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

namespace PokemonAutomation{

using namespace NintendoSwitch::PokemonSV;

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

}
