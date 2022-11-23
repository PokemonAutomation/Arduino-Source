/*  PokemonSV Tests
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "Common/Compiler.h"
#include "PokemonSV_Tests.h"
#include "TestUtils.h"

#include "PokemonSV/Inference/PokemonSV_TeraCardDetector.h"

namespace PokemonAutomation{

using namespace NintendoSwitch::PokemonSV;

int test_pokemonSV_TeraCardFinder(const ImageViewRGB32& image, bool target){
    TeraCardFinder detector;

    for (int i = 0; i < 4; ++i){
        bool result = detector.process_frame(image, current_time());
        TEST_RESULT_EQUAL(result, false);
    }
    bool result = detector.process_frame(image, current_time());
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

}
