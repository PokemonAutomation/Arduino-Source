/*  Button Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string>
#include "Common/Cpp/Color.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonHome/Inference/PokemonHome_ButtonDetector.h"
#include "PokemonHome_BoxViewDetector.h"
#include "Tests/TestUtils.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

BoxViewDetector::BoxViewDetector(VideoOverlay* overlay) : m_button_plus_detector(COLOR_BLACK, ButtonType::ButtonPlus, { 0.100, 0.956, 0.107, 0.041 }, overlay){}

void BoxViewDetector::make_overlays(VideoOverlaySet& items) const{
    m_button_plus_detector.make_overlays(items);
}

bool BoxViewDetector::detect(const ImageViewRGB32& screen){
    return m_button_plus_detector.detect(screen);
}



class Test_BoxViewDetector : public UnitTest{
public:

    Test_BoxViewDetector(
        const std::string& image,
        bool expected
    )
        : UnitTest("PokemonHome::BoxViewDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        DummyVideoOverlay overlay;
        BoxViewDetector detector(&overlay);
        ImageRGB32 image(m_image);
        return detector.detect(image) == m_expected;
    };

private:
    std::string m_image;
    bool m_expected;
};

void add_tests_BoxViewDetector(UnitTestDatabase& database){
    database.add<Test_BoxViewDetector>("PokemonHome/BoxView/BoxView-1.png", true);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/enamorus_Shiny.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/machamp_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/rowlet_ShinyAlpha.png", false);
}

}
}
}
