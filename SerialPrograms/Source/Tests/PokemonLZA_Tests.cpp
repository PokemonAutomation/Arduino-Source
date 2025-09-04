/*  PokemonLZA Tests
*
*  From: https://github.com/PokemonAutomation/
*
*/

#include "Common/Compiler.h"
#include "PokemonLZA_Tests.h"
#include "TestUtils.h"
#include "CommonFramework/Language.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Recording/StreamHistorySession.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "PokemonLZA/Inference/PokemonLZA_DialogDetector.h" 
#include <QFileInfo>
#include <QDir>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
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


} 
