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

}
