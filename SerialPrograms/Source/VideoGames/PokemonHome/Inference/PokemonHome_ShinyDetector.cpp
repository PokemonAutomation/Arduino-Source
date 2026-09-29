/*  Shiny Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "Tests/TestUtils.h"
#include "PokemonHome_ShinyDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


ShinyDetector::ShinyDetector(Color color, VideoOverlay* overlay)
    : m_color(color)
    , m_overlay(overlay)
    , m_box(0.702, 0.09, 0.04, 0.06)
{}

void ShinyDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool ShinyDetector::detect(const ImageViewRGB32& screen){
    const bool found = image_stddev(extract_box_reference(screen, m_box)).sum() > 30;
    if (m_overlay){
        if (found){
            m_last_detected_box.emplace(*m_overlay, m_box, COLOR_GREEN);
        }else{
            m_last_detected_box.reset();
        }
    }
    return found;
}

class Test_ShinyDetector : public UnitTest{
public:
    Test_ShinyDetector(const std::string& image, bool expected)
        : UnitTest("PokemonHome::ShinyDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);
        ShinyDetector detector;
        return detector.detect(image) == m_expected;
    }

private:
    std::string m_image;
    bool m_expected;
};

void add_tests_ShinyDetector(UnitTestDatabase& database){
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/annihilape_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/bidoof_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/bulbasaur_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Go.png", true);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Lza.png", true);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/capskid_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/castform_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/cyclizar_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/dudunsparce_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/dudunsparce_Regular_Sv.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/enamorus_Shiny.png", true);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/gimmighoul_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/glimmet_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/gogoat_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/greatTusk_Shiny.png", true);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/hatterne_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/houndstone_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/ironBunde_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/ironBundle_Regular_Sv.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/ironJugulis_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/ironThorns_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/kilowattrel_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/kingler_Shiny.png", true);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/komala_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/krabby_Shiny.png", true);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/machamp_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/pancham_Shiny.png", true);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/rapidash_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/rellor_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/riolu_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/rowlet_ShinyAlpha.png", true);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/scovillain_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/slitherWing_Shiny.png", true);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/squirtle_Shiny.png", true);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/tapuLele_Shiny.png", true);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/tatsugiri_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/teddiursa_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/terapagos_regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/vulpix_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/vulpix_Shiny.png", true);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/wartortle_Regular.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/wurmple_Regular.png", false);
}


}
}
}
