/*  PokemonLA Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "Common/Compiler.h"
#include "PokemonSwSh_Tests.h"
#include "TestUtils.h"

#include "PokemonSwSh/Inference/PokemonSwSh_BoxGenderDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_DialogTriangleDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_SelectionArrowFinder.h"
#include "PokemonSwSh/Inference/PokemonSwSh_YCommDetector.h"
#include "PokemonSwSh/MaxLair/Inference/PokemonSwSh_MaxLair_Detect_BattleMenu.h"
#include "PokemonSwSh/Inference/PokemonSwSh_DialogBoxDetector.h"
#include "PokemonSwSh/Inference/PokemonSwSh_BoxShinySymbolDetector.h"

#include <QFileInfo>
#include <QDir>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>
using std::cout;
using std::cerr;
using std::endl;

namespace PokemonAutomation{

namespace NintendoSwitch{
namespace PokemonSwSh{
std::vector<ImagePixelBox> find_selection_arrows(const ImageViewRGB32& image, size_t min_area);
}
}

using namespace NintendoSwitch::PokemonSwSh;

int test_pokemonSwSh_YCommMenuDetector(const ImageViewRGB32& image, bool target){
    YCommMenuDetector detector(true);

    bool result = detector.process_frame(image, current_time());
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonSwSh_MaxLair_BattleMenuDetector(const ImageViewRGB32& image, bool target){
    MaxLairInternal::BattleMenuDetector detector;

    bool result = detector.detect(image);
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonSwSh_DialogTriangleDetector(const ImageViewRGB32& image, bool target){
    auto& logger = global_logger_command_line();
    auto overlay = DummyVideoOverlay();
    const bool stop_on_detected = true;

    DialogTriangleDetector detector(logger, overlay, stop_on_detected);

    bool result = detector.process_frame(image, current_time());
    TEST_RESULT_EQUAL(result, target);
    return 0;    
}

int test_pokemonSwSh_RetrieveEggArrowFinder(const ImageViewRGB32& image, bool target){
    auto overlay = DummyVideoOverlay();
    RetrieveEggArrowFinder detector(overlay);

    bool result = detector.process_frame(image, current_time());
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonSwSh_StoragePokemonMenuArrowFinder(const ImageViewRGB32& image, bool target){
    auto overlay = DummyVideoOverlay();
    StoragePokemonMenuArrowFinder detector(overlay);

    bool result = detector.process_frame(image, current_time());
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonSwSh_CheckNurseryArrowFinder(const ImageViewRGB32& image, bool target){
    auto overlay = DummyVideoOverlay();
    CheckNurseryArrowFinder detector(overlay);

    bool result = detector.process_frame(image, current_time());
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonSwSh_YCommIconDetector(const ImageViewRGB32& image, bool target){
    const bool is_on = true;
    YCommIconDetector detector(is_on);
    bool result = detector.process_frame(image, current_time());
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonSwSh_RotomPhoneMenuArrowFinder(const ImageViewRGB32& image, int target){
    auto overlay = DummyVideoOverlay();
    RotomPhoneMenuArrowFinder finder(overlay);
    int result = finder.detect(image);

    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonSwSh_BlackDialogBoxDetector(const ImageViewRGB32& image, bool target){
    const bool stop_on_detected = true;
    BlackDialogBoxDetector detector(stop_on_detected);
    bool result = detector.process_frame(image, current_time());
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonSwSh_BoxShinySymbolDetector(const ImageViewRGB32& image, bool target){
    const bool result = BoxShinySymbolDetector::detect(image);
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonSwSh_BoxGenderDetector(const ImageViewRGB32& image, int target){
    BoxGenderDetector detector;
    const int result = int(detector.detect(image));
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

int test_pokemonSwSh_SelectionArrowFinder(const ImageViewRGB32& image, int target){
    std::vector<ImagePixelBox> boxes = find_selection_arrows(image, 10);
    for(const auto& box : boxes){
        std::cout << "Found box: " << box.min_x << " " << box.max_x << " " << box.min_y << " " << box.max_y << std::endl;
    }
    TEST_RESULT_EQUAL(boxes.size(), (size_t)target);
    return 0;
}

}
