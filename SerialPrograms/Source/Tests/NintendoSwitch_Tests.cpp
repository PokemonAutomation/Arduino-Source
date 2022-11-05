/*  NintendoSwitch Tests
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "Common/Compiler.h"
#include "Common/Cpp/Time.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "NintendoSwitch/Inference/NintendoSwitch_DetectHome.h"
#include "NintendoSwitch_Tests.h"
#include "TestUtils.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

namespace PokemonAutomation{

using namespace NintendoSwitch;

// using namespace NintendoSwitch::PokemonLA;
int test_NintendoSwitch_UpdateMenuDetector(const ImageViewRGB32& image, bool target){
    UpdateMenuDetector detector;
    bool result = detector.detect(image);
    TEST_RESULT_EQUAL(result, target);
    return 0;
}



}
