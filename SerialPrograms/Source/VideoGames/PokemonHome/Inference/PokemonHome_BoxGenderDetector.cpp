/*  Box Gender Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <iostream>
#include "Common/Cpp/Color.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/StaticGlobals.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "PokemonHome_BoxGenderDetector.h"

using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

namespace{
    ImageFloatBox GENDER_BOX{0.417, 0.097, 0.031, 0.046};
}

void BoxGenderDetector::make_overlays(VideoOverlaySet& items){
    items.add(COLOR_RED, GENDER_BOX);
}

Pokemon::StatsHuntGenderFilter BoxGenderDetector::detect(const ImageViewRGB32& screen){
    const auto region = extract_box_reference(screen, GENDER_BOX);

    // Retain only red pixels from region
    const bool replace_color_within_range = false;
    const ImageRGB32 red_region = filter_rgb32_range(
        region,
        combine_rgb(150, 0, 0), combine_rgb(255, 100, 100), Color(0), replace_color_within_range
    );
    const size_t num_red_pixels = image_stats(red_region).count;

    // Retain only blue pixels from region
    const ImageRGB32 blue_region = filter_rgb32_range(
        region,
        combine_rgb(0, 0, 150), combine_rgb(100, 100, 255), Color(0), replace_color_within_range
    );
    const size_t num_blue_pixels = image_stats(blue_region).count;

    if (STATIC_GLOBALS.COLOR_CHECK){
        cout << "num_red_pixels: " << num_red_pixels << ", num_blue_pixels: " << num_blue_pixels
             << ", region " << region.width() << " x " << region.height() << endl;

        cout << "Save images to ./red_only.png and ./blue_only.png" << endl;
        red_region.save("./red_only.png");
        blue_region.save("./blue_only.png");
    }

    const size_t threshold = region.width() * region.height() / 100;

    if (num_red_pixels > threshold){
        return Pokemon::StatsHuntGenderFilter::Female;
    }else if (num_blue_pixels > threshold){
        return Pokemon::StatsHuntGenderFilter::Male;
    }
    return Pokemon::StatsHuntGenderFilter::Genderless;
}

class Test_BoxGenderDetector : public UnitTest{
public:

    Test_BoxGenderDetector(
        const std::string& image,
        Pokemon::StatsHuntGenderFilter expected
    )
        : UnitTest("PokemonHome::BoxGenderDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);
        Pokemon::StatsHuntGenderFilter result = BoxGenderDetector::detect(image);

        if (result == m_expected)
            return true;

        return "Expected: " + Pokemon::gender_to_string(m_expected) + ", received: " + Pokemon::gender_to_string(result);
    };

private:
    std::string m_image;
    Pokemon::StatsHuntGenderFilter m_expected;
};

void add_tests_BoxGenderDetector(UnitTestDatabase& database){
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/annihilape_Regular.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/bidoof_Regular.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/bulbasaur_Regular.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Go.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/bulbasuar_Shiny_Lza.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/capskid_Regular.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/castform_Regular.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/cyclizar_Regular.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/dudunsparce_Regular.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/dudunsparce_Regular_Sv.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/enamorus_Shiny.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/gimmighoul_Regular.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/glimmet_Regular.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/gogoat_Regular.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/greatTusk_Shiny.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/hatterne_Regular.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/houndstone_Regular.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/ironBunde_Regular.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/ironBundle_Regular_Sv.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/ironJugulis_Regular.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/ironThorns_Regular.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/kilowattrel_Regular.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/kingler_Shiny.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/komala_Regular.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/krabby_Shiny.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/machamp_Regular.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/pancham_Shiny.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/rapidash_Regular.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/rellor_Regular.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/riolu_Regular.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/rowlet_ShinyAlpha.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/scovillain_Regular.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/slitherWing_Shiny.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/squirtle_Shiny.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/tapuLele_Shiny.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/tatsugiri_Regular.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/teddiursa_Regular.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/terapagos_regular.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/vulpix_Regular.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/vulpix_Shiny.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/wartortle_Regular.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/wurmple_Regular.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-NV12.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/GBA-Origin-ElgatoHD60Pro-YUYV.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/Farfetchd-fast-ball.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/Machamp-gigaton-ball.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/Machoke-gigaton-ball.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/Machop-gigaton-ball.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/Magnemite-gigaton-ball.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/Magneton-gigaton-ball.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/Slowpoke-fast-ball.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/absol_Regular_none_1792424917318.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/arrokuda_Regular_none_1792424917770.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/azelf_Regular_none_1792424918264.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/bisharp_Regular_none_1792424918722.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/budew_Regular_none_1792424919162.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/buneary_Regular_none_1792424919615.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/bunnelby_Regular_none_1792424920096.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/buzzwole_Regular_none_1792424920562.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/calyrex_Regular_none_1792424921051.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/centiskorch_Regular_none_1792424921522.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/charjabug_Regular_none_1792424921973.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/charmander_Regular_none_1792424922423.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/cherubi_Regular_none_1792424922878.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923397.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/cottonee_Regular_none_1792424923843.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/cramorant_Regular_none_1792424924277.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/cubone_Regular_none_1792424924758.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/delibird_Regular_none_1792424925212.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/dracovish_Regular_none_1792424925673.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/dreepy_Regular_none_1792424926124.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/dusclops_Regular_none_1792424926621.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927083.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/electrike_Regular_none_1792424927528.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/eternatus_Regular_Shiny_none_1792424927959.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/eternatus_Regular_none_1792424928413.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/ferroseed_Regular_none_1792424928886.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/fletchling_Regular_none_1792424929346.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/gastly_Regular_none_1792424929814.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930272.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/gible_Regular_none_1792424930703.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931148.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/golurk_Regular_none_1792424931632.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/goodra_Regular_none_1792424932092.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/grookey_Regular_none_1792424932537.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/gurdurr_Regular_none_1792424932986.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/gyarados_Regular_none_1792424933448.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424933893.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/hattrem_Regular_none_1792424934353.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/haunter_Regular_none_1792424934828.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/honedge_Regular_none_1792424935285.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/jellicent_Regular_none_1792424935736.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/keldeo_Regular_Shiny_none_1792424936178.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/linoone_Regular_none_1792424936621.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937059.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/litten_Regular_none_1792424937516.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424937960.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/machoke_Regular_none_1792424938413.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/machop_Regular_none_1792424938862.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/magearna_Regular_none_1792424939321.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/magikarp_Regular_none_1792424939790.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940261.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/melmetal_Regular_none_1792424940711.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941158.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/meowth_Regular_none_1792424941611.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/metapod_Regular_none_1792424942051.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/milcery_Regular_none_1792424942492.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/mime-jr_Regular_none_1792424942964.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/mudkip_Regular_none_1792424943412.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/nidoking_Regular_none_1792424943853.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/nidoran-f_Regular_none_1792424944297.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/nidoran-m_Regular_none_1792424944756.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/ninjask_Regular_none_1792424945199.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/noibat_Regular_none_1792424945647.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/orbeetle_Regular_none_1792424946083.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/pangoro_Regular_none_1792424946529.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/pikachu_Regular_none_1792424946978.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/poliwhirl_Regular_none_1792424947416.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/pumpkaboo_Regular_none_1792424947882.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/purrloin_Regular_none_1792424948321.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/ralts_Regular_none_1792424948769.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/rapidash_Regular_none_1792424949229.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/remoraid_Regular_none_1792424949680.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/rhydon_Regular_none_1792424950146.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/rookidee_Regular_none_1792424950592.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/rotom_Regular_none_1792424951021.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/sandaconda_Regular_none_1792424951459.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/sandslash_Regular_none_1792424951934.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/scorbunny_Regular_none_1792424952390.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/seismitoad_Regular_none_1792424952847.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/sharpedo_Regular_none_1792424953289.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/shellos_Regular_none_1792424953738.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/shiinotic_Regular_none_1792424954189.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424954668.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955123.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/sizzlipede_Regular_none_1792424955582.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/skwovet_Regular_none_1792424956033.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/slowpoke_Regular_none_1792424956466.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/snover_Regular_none_1792424956935.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/sobble_Regular_none_1792424957377.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424957817.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/stunfisk_Regular_none_1792424958276.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/tapu-bulu_Regular_none_1792424958716.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/tapu-lele_Regular_none_1792424959172.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424959640.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960105.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960552.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424960992.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/timburr_Regular_none_1792424961449.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/toxapex_Regular_none_1792424961906.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/toxtricity_Regular_none_1792424962375.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/tranquill_Regular_none_1792424962829.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/tympole_Regular_none_1792424963266.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424963705.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/vanillish_Regular_none_1792424964155.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424964627.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/vulpix_Regular_none_1792424965074.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/wishiwashi_Regular_none_1792424965509.png", Pokemon::StatsHuntGenderFilter::Male);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/wobbuffet_Regular_none_1792424965947.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/wynaut_Regular_none_1792424966405.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424966858.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/zarude_Regular_none_1792424967363.png", Pokemon::StatsHuntGenderFilter::Genderless);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424967861.png", Pokemon::StatsHuntGenderFilter::Female);
    database.add<Test_BoxGenderDetector>("PokemonHome/SummaryScreen/zigzagoon_Regular_none_1792424968351.png", Pokemon::StatsHuntGenderFilter::Male);
}


}
}
}
