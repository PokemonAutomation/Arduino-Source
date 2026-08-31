/*  Button Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string>
#include "CommonFramework/GlobalAutoPaths.h"
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
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/annihilape_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/bidoof_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/bulbasaur_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Go.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Lza.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/capskid_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/castform_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/cyclizar_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/dudunsparce_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/dudunsparce_Regular_Sv.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/enamorus_Shiny.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/gimmighoul_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/glimmet_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/gogoat_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/greatTusk_Shiny.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/hatterne_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/houndstone_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/ironBunde_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/ironBundle_Regular_Sv.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/ironJugulis_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/ironThorns_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/kilowattrel_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/kingler_Shiny.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/komala_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/krabby_Shiny.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/machamp_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/pancham_Shiny.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/rapidash_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/rellor_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/riolu_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/rowlet_ShinyAlpha.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/scovillain_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/slitherWing_Shiny.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/squirtle_Shiny.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/tapuLele_Shiny.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/tatsugiri_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/teddiursa_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/terapagos_regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/vulpix_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/vulpix_Shiny.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/wartortle_Regular.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/wurmple_Regular.png", true);
}

}
}
}
