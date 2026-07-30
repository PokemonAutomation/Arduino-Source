/*  Button Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string>
#include "CommonFramework/Globals.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonHome/Inference/PokemonHome_ButtonDetector.h"
#include "PokemonHome_SummaryScreenDetector.h"
#include "Tests/TestUtils.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


SummaryScreenDetector::SummaryScreenDetector(VideoOverlay* overlay) 
    : m_button_B_detector(COLOR_BLACK, ButtonType::ButtonB, { 0.100, 0.956, 0.107, 0.041 }, overlay){}

void SummaryScreenDetector::make_overlays(VideoOverlaySet& items) const{
    m_button_B_detector.make_overlays(items);
}

bool SummaryScreenDetector::detect(const ImageViewRGB32& screen){
    return m_button_B_detector.detect(screen);
}


class Test_SummaryScreenDetector : public UnitTest{
public:

    Test_SummaryScreenDetector(
        const std::string& image,
        bool expected
    )
        : UnitTest("PokemonHome::SummaryScreenDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        DummyVideoOverlay overlay;
        SummaryScreenDetector detector(&overlay);
        ImageRGB32 image(m_image);
        return detector.detect(image) == m_expected;
    };

private:
    std::string m_image;
    bool m_expected;
};

void add_tests_SummaryScreenDetector(UnitTestDatabase& database){
    database.add<Test_SummaryScreenDetector>("PokemonHome/BoxView/BoxView-1.png", false);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/enamorus_Shiny.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/machamp_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/rowlet_ShinyAlpha.png", true);
}

}
}
}
