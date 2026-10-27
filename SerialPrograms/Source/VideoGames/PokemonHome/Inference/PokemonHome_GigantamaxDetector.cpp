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
        {0xffc42058, 0xffffe8ff},
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
        GigantamaxDetector detector(COLOR_BLACK, &video_overlay);
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
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-NV12.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-YUYV.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/Farfetchd-fast-ball.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/Machamp-gigaton-ball.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/Machoke-gigaton-ball.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/Machop-gigaton-ball.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/Magnemite-gigaton-ball.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/Magneton-gigaton-ball.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/Slowpoke-fast-ball.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/absol_Regular_none_1792424917318.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/arrokuda_Regular_none_1792424917770.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/azelf_Regular_none_1792424918264.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/bisharp_Regular_none_1792424918722.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/budew_Regular_none_1792424919162.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/buneary_Regular_none_1792424919615.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/bunnelby_Regular_none_1792424920096.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/buzzwole_Regular_none_1792424920562.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/calyrex_Regular_none_1792424921051.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/centiskorch_Regular_none_1792424921522.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/charjabug_Regular_none_1792424921973.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/charmander_Regular_none_1792424922423.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/cherubi_Regular_none_1792424922878.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923397.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923843.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/cramorant_Regular_none_1792424924277.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/cubone_Regular_none_1792424924758.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/delibird_Regular_none_1792424925212.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/dracovish_Regular_none_1792424925673.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/dreepy_Regular_none_1792424926124.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/dusclops_Regular_none_1792424926621.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927083.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927528.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/eternatus_Regular_Shiny_none_1792424927959.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/eternatus_Regular_none_1792424928413.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/ferroseed_Regular_none_1792424928886.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/fletchling_Regular_none_1792424929346.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/gastly_Regular_none_1792424929814.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930272.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930703.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931148.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931632.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/goodra_Regular_none_1792424932092.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/grookey_Regular_none_1792424932537.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/gurdurr_Regular_none_1792424932986.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/gyarados_Regular_none_1792424933448.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424933893.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424934353.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/haunter_Regular_none_1792424934828.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/honedge_Regular_none_1792424935285.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/jellicent_Regular_none_1792424935736.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/keldeo_Regular_Shiny_none_1792424936178.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/linoone_Regular_none_1792424936621.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937059.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937516.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424937960.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424938413.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/machop_Regular_none_1792424938862.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/magearna_Regular_none_1792424939321.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/magikarp_Regular_none_1792424939790.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940261.png", true);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940711.png", true);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941158.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941611.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/metapod_Regular_none_1792424942051.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/milcery_Regular_none_1792424942492.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/mime-jr_Regular_none_1792424942964.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/mudkip_Regular_none_1792424943412.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/nidoking_Regular_none_1792424943853.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/nidoran-f_Regular_none_1792424944297.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/nidoran-m_Regular_none_1792424944756.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/ninjask_Regular_none_1792424945199.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/noibat_Regular_none_1792424945647.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/orbeetle_Regular_none_1792424946083.png", true);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/pangoro_Regular_none_1792424946529.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/pikachu_Regular_none_1792424946978.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/poliwhirl_Regular_none_1792424947416.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/pumpkaboo_Regular_none_1792424947882.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/purrloin_Regular_none_1792424948321.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/ralts_Regular_none_1792424948769.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/rapidash_Regular_none_1792424949229.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/remoraid_Regular_none_1792424949680.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/rhydon_Regular_none_1792424950146.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/rookidee_Regular_none_1792424950592.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/rotom_Regular_none_1792424951021.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/sandaconda_Regular_none_1792424951459.png", true);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/sandslash_Regular_none_1792424951934.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/scorbunny_Regular_none_1792424952390.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/seismitoad_Regular_none_1792424952847.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/sharpedo_Regular_none_1792424953289.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/shellos_Regular_none_1792424953738.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/shiinotic_Regular_none_1792424954189.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424954668.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955123.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955582.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/skwovet_Regular_none_1792424956033.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/slowpoke_Regular_none_1792424956466.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/snover_Regular_none_1792424956935.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/sobble_Regular_none_1792424957377.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424957817.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424958276.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/tapu-bulu_Regular_none_1792424958716.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/tapu-lele_Regular_none_1792424959172.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424959640.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960105.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960552.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960992.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424961449.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/toxapex_Regular_none_1792424961906.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/toxtricity_Regular_none_1792424962375.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/tranquill_Regular_none_1792424962829.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/tympole_Regular_none_1792424963266.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424963705.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424964155.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424964627.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424965074.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/wishiwashi_Regular_none_1792424965509.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/wobbuffet_Regular_none_1792424965947.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/wynaut_Regular_none_1792424966405.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424966858.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424967363.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424967861.png", false);
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424968351.png", false);
}


}
}
}
