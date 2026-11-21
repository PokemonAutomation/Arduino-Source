/*  Shiny Symbol Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "Tests/ImageUnitTest.h"
#include "VideoGames/PokemonFRLG/PokemonFRLG_Settings.h"
#include "PokemonFRLG_ShinySymbolDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

ShinySymbolDetector::ShinySymbolDetector(Color color)
    : m_box_symbol(0.421231, 0.214308, 0.0406154, 0.0623077)
{}
void ShinySymbolDetector::make_overlays(VideoOverlaySet& items) const{
    const BoxOption& GAME_BOX = GameSettings::instance().GAME_BOX;
    items.add(COLOR_RED, GAME_BOX.inner_to_outer(m_box_symbol));
}
bool ShinySymbolDetector::read(Logger& logger, const ImageViewRGB32& frame){
    ImageViewRGB32 game_screen = extract_box_reference(frame, GameSettings::instance().GAME_BOX);

    const bool replace_color_within_range = false;

    //Filter out background
    ImageRGB32 filtered_region = filter_rgb32_range(
        extract_box_reference(game_screen, m_box_symbol),
        combine_rgb(193, 152, 0), combine_rgb(255, 255, 162), Color(0), replace_color_within_range
    );
    ImageStats stats = image_stats(filtered_region);

    /*
    filtered_region.save("./filtered_only.png");
    cout << stats.average.r << endl;
    cout << stats.average.g << endl;
    cout << stats.average.b << endl;
    */

    /*
    Shiny:
    R: 195.786, G: 182.143, B: 142.286
    */

    if (stats.average.b + 20 < stats.average.r){
        return true;
    }
    return false;
}

class Test_ShinySymbolDetector : public ImageUnitTest{
public:

    Test_ShinySymbolDetector(
        const std::string& image,
        bool expected
    )
        : ImageUnitTest("PokemonFRLG::ShinySymbolDetector - " + image, {image})
        , m_expected(expected)
    {}

    virtual UnitTestResult run_on_images(
        Logger& logger, CancellableScope& scope,
        const std::vector<ImageRGB32>& images
    ) const override{
        ShinySymbolDetector detector(COLOR_RED);
        const ImageRGB32& image = images[0];
        return detector.read(logger, image) == m_expected;
    };

private:
    bool m_expected;
};

void add_tests_ShinySymbolDetector(UnitTestDatabase& database){
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/ShinySymbolDetector/Char_False.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/ShinySymbolDetector/Karp-coloredited_True.png", true);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/ShinySymbolDetector/Karp-snotyak_True.png", true);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/abra_1_deu.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/bulbasaur_1_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/bulbasaur_1_fra.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/bulbasaur_1_jpn.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/bulbasaur_2_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/bulbasaur_3_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/bulbasaur_4_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/deoxys_1_jpn.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/horsea_1_deu.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/horsea_2_deu.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/horsea_3_deu.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/magikarp_1_spa.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/moltres_1_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/nidoran_m_eng.png", true);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/pidgey_1_ita.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/unown_c_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/unown_ex_eng.png", true);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/unown_h_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/unown_j_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/unown_n_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/unown_p_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/unown_qu_eng.png", true);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/venonat_4k_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/arbok_hd60pro_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/arcanine_hd60pro_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/blastoise_hd60pro_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/butterfree_hd60pro_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/diglett_hd60pro_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/dragonite_hd60pro_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/dragonite_usb3_eng.png", true);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/ekans_hd60pro_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/exeggcute_hd60pro_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/fearow_hd60pro_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/gengar_usb3_eng.png", true);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/geodude_usb3_eng.png", true);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/gloom_hd60pro_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/golbat_hd60pro_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/machoke_usb3_eng.png", true);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/magmar_usb3_eng.png", true);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/magnemite_usb3_eng.png", true);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/magneton_usb3_eng.png", true);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/mantine_usb3_eng.png", true);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/mewtwo_usb3_eng.png", true);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/mr-mime_hd60pro_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/nidoking_hd60pro_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/nidoqueen_hd60pro_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/onix_hd60pro_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/parasect_hd60pro_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/pidgeotto_hd60pro_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/psyduck_hd60pro_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/raichu_hd60pro_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/rhydon_usb3_eng.png", true);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/sneasel_usb3_eng.png", true);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/snorlax_hd60pro_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/swinub_usb3_eng.png", true);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page1/weedle_hd60pro_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page2/abra_1_deu.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page2/deoxys_1_jpn.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page2/mewtwo_1_spa.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page2/nidoranf_1_eng.jpg", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page2/raikou_1_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page2/raikou_2_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page2/scyther_1_eng.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/StatsReader/Page2/venonat_1_eng.jpg", false);
}

}
}
}

