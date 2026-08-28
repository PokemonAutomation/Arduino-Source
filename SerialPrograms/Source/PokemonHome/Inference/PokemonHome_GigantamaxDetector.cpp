/*  Gigantamax Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonHome_GigantamaxDetector.h"
#include "Tests/TestUtils.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


class GigantamaxMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    GigantamaxMatcher()
        : WaterfillTemplateMatcher(
            "PokemonHome/Home_Gigantamax.png",
            Color(196, 32, 88), Color(255, 220, 235),
            100
        )
    {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.85;
        m_area_ratio_upper = 1.1;
    }

    static const GigantamaxMatcher& instance(){
        static GigantamaxMatcher matcher;
        return matcher;
    }
};


GigantamaxDetector::GigantamaxDetector(
    Color color,
    VideoOverlay* overlay,
    const ImageFloatBox& box
)
    : m_color(color)
    , m_overlay(overlay)
    , m_box(box)
{}
void GigantamaxDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool GigantamaxDetector::detect(const ImageViewRGB32& screen){
    const double screen_rel_size = (screen.height() / 1080.0);
    const double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    const double min_area_1080p = 400;
    const double rmsd_threshold = 80;
    const size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xffc42058, 0xffffdceb},
    };

    const bool found = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_box),
        GigantamaxMatcher::instance(),
        FILTERS,
        {min_area, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            m_last_detected = translate_to_parent(screen, m_box, object);
            return true;
        }
    );

    if (m_overlay){
        if (found){
            m_last_detected_box.emplace(*m_overlay, m_last_detected, COLOR_GREEN);
        }else{
            m_last_detected_box.reset();
        }
    }

    return found;
}

class Test_GigantamaxDetector : public UnitTest{
public:

    Test_GigantamaxDetector(
        const std::string& image,
        bool expected
    )
        : UnitTest("PokemonHome::GigantamaxDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        DummyVideoOverlay video_overlay;
        ImageRGB32 image(m_image);
        GigantamaxDetector detector(COLOR_BLACK, &video_overlay, ImageFloatBox(0.463, 0.09, 0.04, 0.06));
        bool result = detector.detect(image);

        if (result == m_expected)
            return true;

        return "Expected: " + std::to_string(m_expected) + ", received: " + std::to_string(result);
    };

private:
    std::string m_image;
    bool m_expected;
};

void add_tests_GigantamaxDetector(UnitTestDatabase& database){
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/annihilape_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/bidoof_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/bulbasaur_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Go.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Lza.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/capskid_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/castform_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/cyclizar_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/dudunsparce_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/dudunsparce_Regular_Sv.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/enamorus_Shiny.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/gimmighoul_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/glimmet_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/gogoat_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/greatTusk_Shiny.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/hatterne_Regular.png", true);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/houndstone_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/ironBunde_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/ironBundle_Regular_Sv.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/ironJugulis_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/ironThorns_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/kilowattrel_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/kingler_Shiny.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/komala_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/krabby_Shiny.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/machamp_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/pancham_Shiny.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/rapidash_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/rellor_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/riolu_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/rowlet_ShinyAlpha.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/scovillain_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/slitherWing_Shiny.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/squirtle_Shiny.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/tapuLele_Shiny.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/tatsugiri_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/teddiursa_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/terapagos_regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/vulpix_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/vulpix_Shiny.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/wartortle_Regular.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/wurmple_Regular.png", false);
}


}
}
}
