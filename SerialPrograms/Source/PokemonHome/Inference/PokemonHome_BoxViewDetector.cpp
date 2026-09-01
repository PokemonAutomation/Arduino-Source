/*  Button Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string>
#include "Common/Cpp/Color.h"
#include "CommonFramework/GlobalAutoPaths.h"
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
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/annihilape_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/bidoof_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/bulbasaur_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Go.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Lza.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/capskid_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/castform_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/cyclizar_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/dudunsparce_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/dudunsparce_Regular_Sv.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/enamorus_Shiny.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/gimmighoul_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/glimmet_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/gogoat_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/greatTusk_Shiny.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/hatterne_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/houndstone_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/ironBunde_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/ironBundle_Regular_Sv.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/ironJugulis_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/ironThorns_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/kilowattrel_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/kingler_Shiny.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/komala_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/krabby_Shiny.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/machamp_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/pancham_Shiny.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/rapidash_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/rellor_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/riolu_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/rowlet_ShinyAlpha.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/scovillain_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/slitherWing_Shiny.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/squirtle_Shiny.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/tapuLele_Shiny.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/tatsugiri_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/teddiursa_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/terapagos_regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/vulpix_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/vulpix_Shiny.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/wartortle_Regular.png", false);
    database.add<Test_BoxViewDetector>("PokemonHome/SummaryScreen/wurmple_Regular.png", false);
}

}
}
}
