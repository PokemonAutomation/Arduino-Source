/*  Shiny Symbol Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Color.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"
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

class Test_ShinySymbolDetector : public UnitTest{
public:

    Test_ShinySymbolDetector(
        const std::string& image,
        bool expected
    )
        : UnitTest("PokemonHome::ShinySymbolDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ShinySymbolDetector detector(COLOR_RED);
        ImageRGB32 image(m_image);
        return detector.read(logger, image) == m_expected;
    };

private:
    std::string m_image;
    bool m_expected;
};

void add_tests_ShinySymbolDetector(UnitTestDatabase& database){
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/ShinySymbolDetector/Char_False.png", false);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/ShinySymbolDetector/Karp-coloredited_True.png", true);
    database.add<Test_ShinySymbolDetector>("PokemonFRLG/ShinySymbolDetector/Karp-snotyak_True.png", true);
}

}
}
}

