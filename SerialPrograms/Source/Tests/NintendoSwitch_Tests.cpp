/*  NintendoSwitch Tests
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "Common/Compiler.h"
#include "Common/Cpp/Time.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Recording/StreamHistorySession.h"
#include "NintendoSwitch/Controllers/SerialPABotBase/NintendoSwitch_SerialPABotBase_WiredController.h"
#include "NintendoSwitch/Inference/NintendoSwitch_UpdatePopupDetector.h"
#include "NintendoSwitch_Tests.h"
#include "TestUtils.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

namespace PokemonAutomation{

using namespace NintendoSwitch;

// using namespace NintendoSwitch::PokemonLA;
int test_NintendoSwitch_UpdatePopupDetector(const ImageViewRGB32& image, bool target){
    auto& logger = global_logger_command_line();
    DummyBotBase botbase(logger);
    SerialPABotBase::SerialPABotBase_Connection connection(logger, nullptr, false);
    SerialPABotBase_WiredController controller(
        logger, connection,
        ControllerType::NintendoSwitch_WiredController,
        ControllerResetMode::DO_NOT_RESET
    );
    DummyVideoFeed video_feed;
    DummyVideoOverlay video_overlay;
    DummyAudioFeed audio_feed;
    StreamHistorySession history(logger);

    ConsoleHandle console(0, logger, controller, video_feed, video_overlay, audio_feed, history);
    UpdatePopupDetector detector(console);
    bool result = detector.detect(image);
    TEST_RESULT_EQUAL(result, target);
    return 0;
}



}
