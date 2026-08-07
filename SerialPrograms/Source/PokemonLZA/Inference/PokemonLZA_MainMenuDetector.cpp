/*  Main Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonLZA_MainMenuDetector.h"
#include "Tests/TestUtils.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


MainMenuDetector::MainMenuDetector(Color color, VideoOverlay* overlay)
    : m_right_arrow_button(color, ButtonType::ButtonRight, {0.382, 0.081, 0.029, 0.056}, overlay)
    , m_b_button(color, ButtonType::ButtonB, {0.87, 0.940, 0.077, 0.044}, overlay)
{}

void MainMenuDetector::make_overlays(VideoOverlaySet& items) const{
    m_right_arrow_button.make_overlays(items);
    m_b_button.make_overlays(items);
}

bool MainMenuDetector::detect(const ImageViewRGB32& screen){
    return m_right_arrow_button.detect(screen) && m_b_button.detect(screen);
}

class Test_MainMenuDetector : public UnitTest{
public:
    Test_MainMenuDetector(
        const std::string& image,
        bool expected
    )
        : UnitTest("PokemonPLZA::MainMenuDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        DummyVideoOverlay overlay;
        MainMenuDetector detector(COLOR_RED, &overlay);
        ImageRGB32 image(m_image);
        return detector.detect(image) == m_expected;
    };

private:
    std::string m_image;
    bool m_expected;
};


void add_tests_MainMenuDetector(UnitTestDatabase& database){
    database.add<Test_MainMenuDetector>("PokemonLZA/MainMenuDetector/french_main_menu_True.png", true);
    database.add<Test_MainMenuDetector>("PokemonLZA/MainMenuDetector/mac_main_menu_True.png", true);
}


}
}
}
