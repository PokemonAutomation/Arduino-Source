/*  PokemonSV Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSV/Inference/Battles/PokemonSV_NormalBattleMenus.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_DialogDetector.h"
#include "PokemonSV_Tests_Old.h"
#include "TestUtils.h"

namespace PokemonAutomation{

using namespace NintendoSwitch::PokemonSV;

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

}
