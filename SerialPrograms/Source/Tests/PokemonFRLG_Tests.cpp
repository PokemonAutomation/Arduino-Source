/*  PokemonFRLG Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "CommonFramework/Logging/Logger.h"
#include "PokemonFRLG/Inference/Dialogs/PokemonFRLG_DialogDetector.h"
#include "PokemonFRLG_Tests.h"
#include "TestUtils.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Recording/StreamHistorySession.h"
#include "NintendoSwitch/Controllers/SerialPABotBase/NintendoSwitch_SerialPABotBase_WiredController.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

namespace PokemonAutomation{

using namespace NintendoSwitch;
using namespace NintendoSwitch::PokemonFRLG;

int test_pokemonFRLG_AdvanceWhiteDialogDetector(const ImageViewRGB32& image, bool target){
    auto overlay = DummyVideoOverlay();
    AdvanceWhiteDialogDetector detector(COLOR_RED);
    bool result = detector.detect(image);
    TEST_RESULT_EQUAL(result, target);
    return 0;
}

}
