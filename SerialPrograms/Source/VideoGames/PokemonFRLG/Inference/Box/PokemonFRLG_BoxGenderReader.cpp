/*  Box Gender Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonTools/Images/ImageFilter.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_BoxGenderReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

BoxGenderReader::BoxGenderReader(Color color)
    : m_color(color)
    , m_box(0.0324, 0.72, 0.041, 0.089)
{}

void BoxGenderReader::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, GameSettings::instance().GAME_BOX.inner_to_outer(m_box));
}

Pokemon::StatsHuntGenderFilter BoxGenderReader::read(const ImageViewRGB32& screen) const{
    const ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);
    const ImageViewRGB32 region = extract_box_reference(game_screen, m_box);
    const ImageRGB32 red = filter_rgb32_range(
        region, combine_rgb(150, 0, 0), combine_rgb(255, 150, 150), Color(0), false
    );
    const ImageRGB32 blue = filter_rgb32_range(
        region, combine_rgb(0, 0, 150), combine_rgb(120, 210, 255), Color(0), false
    );
    const uint64_t red_count = image_stats(red).count;
    const uint64_t blue_count = image_stats(blue).count;
    const size_t threshold = region.width() * region.height() / 100;
    if (red_count > threshold && red_count > blue_count){
        return Pokemon::StatsHuntGenderFilter::Female;
    }
    if (blue_count > threshold && blue_count > red_count){
        return Pokemon::StatsHuntGenderFilter::Male;
    }
    return Pokemon::StatsHuntGenderFilter::Genderless;
}

class Test_BoxGenderReader : public UnitTest{
public:
    Test_BoxGenderReader(const std::string& image, Pokemon::StatsHuntGenderFilter expected)
        : UnitTest("PokemonFRLG::BoxGenderReader - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);
        const auto result = BoxGenderReader().read(image);
        if (result == m_expected){
            return true;
        }
        return "Expected: " + Pokemon::gender_to_string(m_expected) + ", received: " + Pokemon::gender_to_string(result);
    }

private:
    std::string m_image;
    Pokemon::StatsHuntGenderFilter m_expected;
};

void add_tests_BoxGenderReader(UnitTestDatabase& database){
    using Gender = Pokemon::StatsHuntGenderFilter;
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/Arcanine_M_100_White.png", Gender::Male);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/BoxHand_Closed_Gold.png", Gender::Female);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/BoxHand_Closed_White.png", Gender::Male);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/BoxHand_Gold_HD60Pro_NV12.png", Gender::Male);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/BoxHand_Gold_HD60Pro_YUYV.png", Gender::Male);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/BoxHand_White_HD60Pro_NV12.png", Gender::Male);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/BoxHand_White_HD60Pro_YUYV.png", Gender::Male);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/BoxSelection_Gold.png", Gender::Genderless);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/BoxSelection_Gold2.png", Gender::Male);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/BoxSelection_White.png", Gender::Genderless);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/BoxSelection_White2.png", Gender::Male);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/Caterpie_F_4_White.png", Gender::Female);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/CloseSelection_Gold.png", Gender::Genderless);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/CloseSelection_White.png", Gender::Genderless);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/Diglett_F_20_White.png", Gender::Female);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/Doduo-NV12.png", Gender::Male);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/Doduo-YUYV.png", Gender::Male);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/Dragonite_F_100_Whtie.png", Gender::Female);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/Nidokinq_M_23_White.png", Gender::Male);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/nidoran-f-mjpeg.png", Gender::Genderless);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/nidoran-f-NV12.png", Gender::Genderless);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/nidoran-f-YUYV.png", Gender::Genderless);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/nidoran-m-mjpeg.png", Gender::Genderless);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/nidoran-m-NV12.png", Gender::Genderless);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/nidoran-m-YUYV.png", Gender::Genderless);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/PartySelection_Gold.png", Gender::Genderless);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/PartySelection_White.png", Gender::Genderless);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/Rhydon-NV12.png", Gender::Male);
    database.add<Test_BoxGenderReader>("PokemonFRLG/Box/Rhydon-YUYV.png", Gender::Male);
}

}
}
}
