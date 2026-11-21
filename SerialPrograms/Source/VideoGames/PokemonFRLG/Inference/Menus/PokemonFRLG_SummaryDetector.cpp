/*  Summary Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "Tests/ImageUnitTest.h"
#include "Tests/TestUtils.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_SummaryDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

SummaryDetector::SummaryDetector(Color color)
    : m_page_1_box(0.441235, 0.033869, 0.016889, 0.030754) //white
    , m_page_2_box(0.509681, 0.035206, 0.014223, 0.026742) //blue
    , m_page_3_box(0.577239, 0.039218, 0.012445, 0.021394) //blue
{}
void SummaryDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_1_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_2_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_3_box));
}
bool SummaryDetector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 page_1_image = extract_box_reference(game_screen, m_page_1_box);
    ImageViewRGB32 page_2_image = extract_box_reference(game_screen, m_page_2_box);
    ImageViewRGB32 page_3_image = extract_box_reference(game_screen, m_page_3_box);
    if (is_white(page_1_image)
        && is_solid(page_2_image, { 0, 0.3333, 0.6666 }, 0.25, 20)
        && is_solid(page_3_image, { 0, 0.3333, 0.6666 }, 0.25, 20)
    ){
        return true;
    }
    return false;
}

SummaryPage2Detector::SummaryPage2Detector(Color color)
    : m_page_1_box(0.441235, 0.033869, 0.016889, 0.030754) //tan
    , m_page_2_box(0.509681, 0.035206, 0.014223, 0.026742) //white
    , m_page_3_box(0.577239, 0.039218, 0.012445, 0.021394) //blue
{}
void SummaryPage2Detector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_1_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_2_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_3_box));
}
bool SummaryPage2Detector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 page_1_image = extract_box_reference(game_screen, m_page_1_box);
    ImageViewRGB32 page_2_image = extract_box_reference(game_screen, m_page_2_box);
    ImageViewRGB32 page_3_image = extract_box_reference(game_screen, m_page_3_box);
    if (is_solid(page_1_image, { 0.5117, 0.3763, 0.2294 }, 0.25, 20)
        && is_white(page_2_image)
        && is_solid(page_3_image, { 0, 0.3333, 0.6666 }, 0.25, 20)
    ){
        return true;
    }
    return false;
}

SummaryPage3Detector::SummaryPage3Detector(Color color)
    : m_page_1_box(0.441235, 0.033869, 0.016889, 0.030754) //tan
    , m_page_2_box(0.509681, 0.035206, 0.014223, 0.026742) //tan
    , m_page_3_box(0.577239, 0.039218, 0.012445, 0.021394) //white
{}
void SummaryPage3Detector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_1_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_2_box));
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_page_3_box));
}
bool SummaryPage3Detector::detect(const ImageViewRGB32& screen){
    ImageViewRGB32 game_screen = extract_box_reference(screen, GameSettings::instance().GAME_BOX);

    ImageViewRGB32 page_1_image = extract_box_reference(game_screen, m_page_1_box);
    ImageViewRGB32 page_2_image = extract_box_reference(game_screen, m_page_2_box);
    ImageViewRGB32 page_3_image = extract_box_reference(game_screen, m_page_3_box);
    if (is_solid(page_1_image, { 0.5117, 0.3763, 0.2294 }, 0.25, 20)
        && is_solid(page_2_image, { 0.5117, 0.3763, 0.2294 }, 0.25, 20)
        && is_white(page_3_image)
    ){
        return true;
    }
    return false;
}


class Test_SummaryDetector : public ImageUnitTest{
public:
    Test_SummaryDetector(const std::string& image, int page)
        : ImageUnitTest("PokemonFRLG::SummaryDetector - " + image, {image})
        , m_expected_page(page)
    {}

private:
    UnitTestResult run_on_images(
        Logger&, CancellableScope&,
        const std::vector<ImageRGB32>& images
    ) const override{
        const ImageRGB32& image = images[0];
        const bool page1 = SummaryDetector(COLOR_RED).detect(image);
        const bool page2 = SummaryPage2Detector(COLOR_RED).detect(image);
        const bool page3 = SummaryPage3Detector(COLOR_RED).detect(image);
        TEST_RESULT_COMPONENT_EQUAL_STR(page1, m_expected_page == 1, "SummaryDetector");
        TEST_RESULT_COMPONENT_EQUAL_STR(page2, m_expected_page == 2, "SummaryPage2Detector");
        TEST_RESULT_COMPONENT_EQUAL_STR(page3, m_expected_page == 3, "SummaryPage3Detector");
        return true;
    }

    int m_expected_page;
};

void add_tests_SummaryDetector(UnitTestDatabase& database){
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/abra_1_deu.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/bulbasaur_1_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/bulbasaur_1_fra.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/bulbasaur_1_jpn.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/bulbasaur_2_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/bulbasaur_3_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/bulbasaur_4_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/deoxys_1_jpn.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/horsea_1_deu.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/horsea_2_deu.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/horsea_3_deu.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/magikarp_1_spa.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/moltres_1_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/nidoran_m_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/pidgey_1_ita.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/unown_c_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/unown_ex_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/unown_h_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/unown_j_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/unown_n_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/unown_p_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/unown_qu_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/venonat_4k_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/arbok_hd60pro_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/arcanine_hd60pro_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/blastoise_hd60pro_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/butterfree_hd60pro_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/diglett_hd60pro_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/dragonite_hd60pro_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/dragonite_usb3_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/ekans_hd60pro_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/exeggcute_hd60pro_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/fearow_hd60pro_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/gengar_usb3_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/geodude_usb3_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/gloom_hd60pro_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/golbat_hd60pro_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/machoke_usb3_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/magmar_usb3_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/magnemite_usb3_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/magneton_usb3_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/mantine_usb3_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/mewtwo_usb3_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/mr-mime_hd60pro_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/nidoking_hd60pro_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/nidoqueen_hd60pro_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/onix_hd60pro_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/parasect_hd60pro_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/pidgeotto_hd60pro_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/psyduck_hd60pro_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/raichu_hd60pro_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/rhydon_usb3_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/sneasel_usb3_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/snorlax_hd60pro_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/swinub_usb3_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page1/weedle_hd60pro_eng.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page2/abra_1_deu.png", 2);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page2/deoxys_1_jpn.png", 2);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page2/mewtwo_1_spa.png", 2);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page2/nidoranf_1_eng.jpg", 2);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page2/raikou_1_eng.png", 2);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page2/raikou_2_eng.png", 2);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page2/scyther_1_eng.png", 2);
    database.add<Test_SummaryDetector>("PokemonFRLG/StatsReader/Page2/venonat_1_eng.jpg", 2);
    database.add<Test_SummaryDetector>("PokemonFRLG/ShinySymbolDetector/Char_False.png", 1);
    database.add<Test_SummaryDetector>("PokemonFRLG/ShinySymbolDetector/Karp-coloredited_True.png", 2);
    database.add<Test_SummaryDetector>("PokemonFRLG/ShinySymbolDetector/Karp-snotyak_True.png", 2);
}


}
}
}
