/*  PokemonHome Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "PokemonHome/Inference/PokemonHome_ButtonDetector.h"
#include "PokemonHome_Tests.h"
#include "TestUtils.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

namespace PokemonAutomation{

using namespace NintendoSwitch;
using namespace NintendoSwitch::PokemonHome;

int test_pokemonHome_BoxView(const ImageViewRGB32& image, const std::vector<std::string>& keywords){
    SummaryScreenDetector summary_screen_detector;
    bool result = summary_screen_detector.detect(image);
    TEST_RESULT_EQUAL(result, false);

    BoxViewDetector box_view_detector;
    result = box_view_detector.detect(image);
    TEST_RESULT_EQUAL(result, true);
    return 0;
}

int test_pokemonHome_SummaryScreen(const ImageViewRGB32& image, const std::vector<std::string>& keywords){
    SummaryScreenDetector summary_screen_detector;
    bool result = summary_screen_detector.detect(image);
    TEST_RESULT_EQUAL(result, true);

    BoxViewDetector box_view_detector;
    result = box_view_detector.detect(image);
    TEST_RESULT_EQUAL(result, false);
    return 0;
}

}
