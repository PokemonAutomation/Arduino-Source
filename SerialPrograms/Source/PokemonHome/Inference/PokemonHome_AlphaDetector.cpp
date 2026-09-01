/*  Alpha Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "Tests/TestUtils.h"
#include "PokemonHome_AlphaDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


AlphaDetector::AlphaDetector(Color color, VideoOverlay* overlay)
    : m_color(color)
    , m_overlay(overlay)
    , m_box(0.787, 0.095, 0.024, 0.046)
{}

void AlphaDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool AlphaDetector::detect(const ImageViewRGB32& screen){
    const bool found = image_stddev(extract_box_reference(screen, m_box)).sum() > 40;
    if (m_overlay){
        if (found){
            m_last_detected_box.emplace(*m_overlay, m_box, COLOR_GREEN);
        }else{
            m_last_detected_box.reset();
        }
    }
    return found;
}

class Test_AlphaDetector : public UnitTest{
public:
    Test_AlphaDetector(const std::string& image, bool expected)
        : UnitTest("PokemonHome::AlphaDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);
        AlphaDetector detector;
        return detector.detect(image) == m_expected;
    }

private:
    std::string m_image;
    bool m_expected;
};

void add_tests_AlphaDetector(UnitTestDatabase& database){
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/annihilape_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/bidoof_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/bulbasaur_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Go.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Lza.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/capskid_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/castform_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/cyclizar_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/dudunsparce_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/dudunsparce_Regular_Sv.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/enamorus_Shiny.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/gimmighoul_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/glimmet_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/gogoat_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/greatTusk_Shiny.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/hatterne_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/houndstone_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/ironBunde_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/ironBundle_Regular_Sv.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/ironJugulis_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/ironThorns_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/kilowattrel_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/kingler_Shiny.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/komala_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/krabby_Shiny.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/machamp_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/pancham_Shiny.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/rapidash_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/rellor_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/riolu_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/rowlet_ShinyAlpha.png", true);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/scovillain_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/slitherWing_Shiny.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/squirtle_Shiny.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/tapuLele_Shiny.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/tatsugiri_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/teddiursa_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/terapagos_regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/vulpix_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/vulpix_Shiny.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/wartortle_Regular.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/wurmple_Regular.png", false);
}


}
}
}
