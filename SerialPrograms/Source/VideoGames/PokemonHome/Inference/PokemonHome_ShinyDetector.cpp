/*  Shiny Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
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
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-NV12.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-YUYV.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/Farfetchd-fast-ball.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/Machamp-gigaton-ball.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/Machoke-gigaton-ball.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/Machop-gigaton-ball.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/Magnemite-gigaton-ball.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/Magneton-gigaton-ball.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/Slowpoke-fast-ball.png", true);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/absol_Regular_none_1792424917318.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/arrokuda_Regular_none_1792424917770.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/azelf_Regular_none_1792424918264.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/bisharp_Regular_none_1792424918722.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/budew_Regular_none_1792424919162.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/buneary_Regular_none_1792424919615.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/bunnelby_Regular_none_1792424920096.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/buzzwole_Regular_none_1792424920562.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/calyrex_Regular_none_1792424921051.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/centiskorch_Regular_none_1792424921522.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/charjabug_Regular_none_1792424921973.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/charmander_Regular_none_1792424922423.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/cherubi_Regular_none_1792424922878.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923397.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923843.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/cramorant_Regular_none_1792424924277.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/cubone_Regular_none_1792424924758.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/delibird_Regular_none_1792424925212.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/dracovish_Regular_none_1792424925673.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/dreepy_Regular_none_1792424926124.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/dusclops_Regular_none_1792424926621.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927083.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927528.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/eternatus_Regular_Shiny_none_1792424927959.png", true);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/eternatus_Regular_none_1792424928413.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/ferroseed_Regular_none_1792424928886.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/fletchling_Regular_none_1792424929346.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/gastly_Regular_none_1792424929814.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930272.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930703.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931148.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931632.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/goodra_Regular_none_1792424932092.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/grookey_Regular_none_1792424932537.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/gurdurr_Regular_none_1792424932986.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/gyarados_Regular_none_1792424933448.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424933893.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424934353.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/haunter_Regular_none_1792424934828.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/honedge_Regular_none_1792424935285.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/jellicent_Regular_none_1792424935736.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/keldeo_Regular_Shiny_none_1792424936178.png", true);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/linoone_Regular_none_1792424936621.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937059.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937516.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424937960.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424938413.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/machop_Regular_none_1792424938862.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/magearna_Regular_none_1792424939321.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/magikarp_Regular_none_1792424939790.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940261.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940711.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941158.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941611.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/metapod_Regular_none_1792424942051.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/milcery_Regular_none_1792424942492.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/mime-jr_Regular_none_1792424942964.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/mudkip_Regular_none_1792424943412.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/nidoking_Regular_none_1792424943853.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/nidoran-f_Regular_none_1792424944297.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/nidoran-m_Regular_none_1792424944756.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/ninjask_Regular_none_1792424945199.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/noibat_Regular_none_1792424945647.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/orbeetle_Regular_none_1792424946083.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/pangoro_Regular_none_1792424946529.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/pikachu_Regular_none_1792424946978.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/poliwhirl_Regular_none_1792424947416.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/pumpkaboo_Regular_none_1792424947882.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/purrloin_Regular_none_1792424948321.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/ralts_Regular_none_1792424948769.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/rapidash_Regular_none_1792424949229.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/remoraid_Regular_none_1792424949680.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/rhydon_Regular_none_1792424950146.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/rookidee_Regular_none_1792424950592.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/rotom_Regular_none_1792424951021.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/sandaconda_Regular_none_1792424951459.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/sandslash_Regular_none_1792424951934.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/scorbunny_Regular_none_1792424952390.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/seismitoad_Regular_none_1792424952847.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/sharpedo_Regular_none_1792424953289.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/shellos_Regular_none_1792424953738.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/shiinotic_Regular_none_1792424954189.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424954668.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955123.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955582.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/skwovet_Regular_none_1792424956033.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/slowpoke_Regular_none_1792424956466.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/snover_Regular_none_1792424956935.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/sobble_Regular_none_1792424957377.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424957817.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424958276.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/tapu-bulu_Regular_none_1792424958716.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/tapu-lele_Regular_none_1792424959172.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424959640.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960105.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960552.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960992.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424961449.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/toxapex_Regular_none_1792424961906.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/toxtricity_Regular_none_1792424962375.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/tranquill_Regular_none_1792424962829.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/tympole_Regular_none_1792424963266.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424963705.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424964155.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424964627.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424965074.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/wishiwashi_Regular_none_1792424965509.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/wobbuffet_Regular_none_1792424965947.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/wynaut_Regular_none_1792424966405.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424966858.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424967363.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424967861.png", false);
    database.add<Test_ShinyDetector>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424968351.png", false);
}


}
}
}
