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
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-NV12.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-YUYV.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/Farfetchd-fast-ball.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/Machamp-gigaton-ball.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/Machoke-gigaton-ball.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/Machop-gigaton-ball.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/Magnemite-gigaton-ball.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/Magneton-gigaton-ball.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/Slowpoke-fast-ball.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/absol_Regular_none_1792424917318.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/arrokuda_Regular_none_1792424917770.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/azelf_Regular_none_1792424918264.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/bisharp_Regular_none_1792424918722.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/budew_Regular_none_1792424919162.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/buneary_Regular_none_1792424919615.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/bunnelby_Regular_none_1792424920096.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/buzzwole_Regular_none_1792424920562.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/calyrex_Regular_none_1792424921051.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/centiskorch_Regular_none_1792424921522.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/charjabug_Regular_none_1792424921973.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/charmander_Regular_none_1792424922423.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/cherubi_Regular_none_1792424922878.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923397.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923843.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/cramorant_Regular_none_1792424924277.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/cubone_Regular_none_1792424924758.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/delibird_Regular_none_1792424925212.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/dracovish_Regular_none_1792424925673.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/dreepy_Regular_none_1792424926124.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/dusclops_Regular_none_1792424926621.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927083.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927528.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/eternatus_Regular_Shiny_none_1792424927959.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/eternatus_Regular_none_1792424928413.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/ferroseed_Regular_none_1792424928886.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/fletchling_Regular_none_1792424929346.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/gastly_Regular_none_1792424929814.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930272.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930703.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931148.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931632.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/goodra_Regular_none_1792424932092.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/grookey_Regular_none_1792424932537.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/gurdurr_Regular_none_1792424932986.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/gyarados_Regular_none_1792424933448.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424933893.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424934353.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/haunter_Regular_none_1792424934828.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/honedge_Regular_none_1792424935285.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/jellicent_Regular_none_1792424935736.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/keldeo_Regular_Shiny_none_1792424936178.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/linoone_Regular_none_1792424936621.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937059.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937516.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424937960.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424938413.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/machop_Regular_none_1792424938862.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/magearna_Regular_none_1792424939321.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/magikarp_Regular_none_1792424939790.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940261.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940711.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941158.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941611.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/metapod_Regular_none_1792424942051.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/milcery_Regular_none_1792424942492.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/mime-jr_Regular_none_1792424942964.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/mudkip_Regular_none_1792424943412.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/nidoking_Regular_none_1792424943853.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/nidoran-f_Regular_none_1792424944297.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/nidoran-m_Regular_none_1792424944756.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/ninjask_Regular_none_1792424945199.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/noibat_Regular_none_1792424945647.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/orbeetle_Regular_none_1792424946083.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/pangoro_Regular_none_1792424946529.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/pikachu_Regular_none_1792424946978.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/poliwhirl_Regular_none_1792424947416.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/pumpkaboo_Regular_none_1792424947882.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/purrloin_Regular_none_1792424948321.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/ralts_Regular_none_1792424948769.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/rapidash_Regular_none_1792424949229.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/remoraid_Regular_none_1792424949680.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/rhydon_Regular_none_1792424950146.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/rookidee_Regular_none_1792424950592.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/rotom_Regular_none_1792424951021.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/sandaconda_Regular_none_1792424951459.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/sandslash_Regular_none_1792424951934.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/scorbunny_Regular_none_1792424952390.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/seismitoad_Regular_none_1792424952847.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/sharpedo_Regular_none_1792424953289.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/shellos_Regular_none_1792424953738.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/shiinotic_Regular_none_1792424954189.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424954668.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955123.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955582.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/skwovet_Regular_none_1792424956033.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/slowpoke_Regular_none_1792424956466.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/snover_Regular_none_1792424956935.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/sobble_Regular_none_1792424957377.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424957817.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424958276.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/tapu-bulu_Regular_none_1792424958716.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/tapu-lele_Regular_none_1792424959172.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424959640.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960105.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960552.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960992.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424961449.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/toxapex_Regular_none_1792424961906.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/toxtricity_Regular_none_1792424962375.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/tranquill_Regular_none_1792424962829.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/tympole_Regular_none_1792424963266.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424963705.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424964155.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424964627.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424965074.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/wishiwashi_Regular_none_1792424965509.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/wobbuffet_Regular_none_1792424965947.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/wynaut_Regular_none_1792424966405.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424966858.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424967363.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424967861.png", false);
    database.add<Test_AlphaDetector>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424968351.png", false);
}


}
}
}
