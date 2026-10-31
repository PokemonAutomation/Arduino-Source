/*  Button Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Tests/ImageUnitTest.h"
#include "Tests/TestUtils.h"
#include "VideoGames/PokemonHome/Inference/PokemonHome_ButtonDetector.h"
#include "PokemonHome_SummaryScreenDetector.h"

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


class Test_SummaryScreenDetector : public ImageUnitTest{
public:

    Test_SummaryScreenDetector(
        const std::string& image,
        bool expected
    )
        : ImageUnitTest("PokemonHome::SummaryScreenDetector - " + image, {image})
        , m_expected(expected)
    {}

    virtual UnitTestResult run_on_images(
        Logger& logger, CancellableScope& scope,
        const std::vector<ImageRGB32>& images
    ) const override{
        DummyVideoOverlay overlay;
        SummaryScreenDetector detector(&overlay);
        const ImageRGB32& image = images[0];
        return detector.detect(image) == m_expected;
    };

private:
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
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-NV12.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-YUYV.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/Farfetchd-fast-ball.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/Machamp-gigaton-ball.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/Machoke-gigaton-ball.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/Machop-gigaton-ball.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/Magnemite-gigaton-ball.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/Magneton-gigaton-ball.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/Slowpoke-fast-ball.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/absol_Regular_none_1792424917318.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/arrokuda_Regular_none_1792424917770.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/azelf_Regular_none_1792424918264.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/bisharp_Regular_none_1792424918722.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/budew_Regular_none_1792424919162.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/buneary_Regular_none_1792424919615.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/bunnelby_Regular_none_1792424920096.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/buzzwole_Regular_none_1792424920562.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/calyrex_Regular_none_1792424921051.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/centiskorch_Regular_none_1792424921522.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/charjabug_Regular_none_1792424921973.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/charmander_Regular_none_1792424922423.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/cherubi_Regular_none_1792424922878.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923397.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923843.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/cramorant_Regular_none_1792424924277.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/cubone_Regular_none_1792424924758.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/delibird_Regular_none_1792424925212.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/dracovish_Regular_none_1792424925673.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/dreepy_Regular_none_1792424926124.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/dusclops_Regular_none_1792424926621.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927083.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927528.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/eternatus_Regular_Shiny_none_1792424927959.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/eternatus_Regular_none_1792424928413.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/ferroseed_Regular_none_1792424928886.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/fletchling_Regular_none_1792424929346.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/gastly_Regular_none_1792424929814.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930272.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930703.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931148.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931632.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/goodra_Regular_none_1792424932092.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/grookey_Regular_none_1792424932537.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/gurdurr_Regular_none_1792424932986.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/gyarados_Regular_none_1792424933448.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424933893.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424934353.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/haunter_Regular_none_1792424934828.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/honedge_Regular_none_1792424935285.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/jellicent_Regular_none_1792424935736.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/keldeo_Regular_Shiny_none_1792424936178.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/linoone_Regular_none_1792424936621.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937059.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937516.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424937960.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424938413.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/machop_Regular_none_1792424938862.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/magearna_Regular_none_1792424939321.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/magikarp_Regular_none_1792424939790.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940261.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940711.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941158.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941611.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/metapod_Regular_none_1792424942051.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/milcery_Regular_none_1792424942492.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/mime-jr_Regular_none_1792424942964.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/mudkip_Regular_none_1792424943412.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/nidoking_Regular_none_1792424943853.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/nidoran-f_Regular_none_1792424944297.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/nidoran-m_Regular_none_1792424944756.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/ninjask_Regular_none_1792424945199.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/noibat_Regular_none_1792424945647.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/orbeetle_Regular_none_1792424946083.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/pangoro_Regular_none_1792424946529.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/pikachu_Regular_none_1792424946978.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/poliwhirl_Regular_none_1792424947416.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/pumpkaboo_Regular_none_1792424947882.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/purrloin_Regular_none_1792424948321.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/ralts_Regular_none_1792424948769.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/rapidash_Regular_none_1792424949229.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/remoraid_Regular_none_1792424949680.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/rhydon_Regular_none_1792424950146.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/rookidee_Regular_none_1792424950592.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/rotom_Regular_none_1792424951021.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/sandaconda_Regular_none_1792424951459.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/sandslash_Regular_none_1792424951934.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/scorbunny_Regular_none_1792424952390.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/seismitoad_Regular_none_1792424952847.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/sharpedo_Regular_none_1792424953289.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/shellos_Regular_none_1792424953738.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/shiinotic_Regular_none_1792424954189.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424954668.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955123.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955582.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/skwovet_Regular_none_1792424956033.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/slowpoke_Regular_none_1792424956466.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/snover_Regular_none_1792424956935.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/sobble_Regular_none_1792424957377.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424957817.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424958276.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/tapu-bulu_Regular_none_1792424958716.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/tapu-lele_Regular_none_1792424959172.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424959640.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960105.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960552.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960992.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424961449.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/toxapex_Regular_none_1792424961906.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/toxtricity_Regular_none_1792424962375.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/tranquill_Regular_none_1792424962829.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/tympole_Regular_none_1792424963266.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424963705.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424964155.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424964627.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424965074.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/wishiwashi_Regular_none_1792424965509.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/wobbuffet_Regular_none_1792424965947.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/wynaut_Regular_none_1792424966405.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424966858.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424967363.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424967861.png", true);
    database.add<Test_SummaryScreenDetector>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424968351.png", true);
}

}
}
}
