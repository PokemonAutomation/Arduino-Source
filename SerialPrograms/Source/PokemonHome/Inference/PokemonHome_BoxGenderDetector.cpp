/*  Box Gender Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

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

#include <iostream>
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
}


}
}
}
