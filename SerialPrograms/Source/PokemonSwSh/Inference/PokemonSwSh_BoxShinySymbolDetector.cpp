/*  Box Shiny Symbol Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/StaticGlobals.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "PokemonSwSh_BoxShinySymbolDetector.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


void BoxShinySymbolDetector::make_overlays(VideoOverlaySet& items){
    items.add(COLOR_RED, m_box);
}

bool BoxShinySymbolDetector::detect(const ImageViewRGB32& screen){
    const ImageStats symbol = image_stats(extract_box_reference(screen, m_box));
    if (STATIC_GLOBALS.COLOR_CHECK){
        cout << "Symbol region stddev " << symbol.stddev.to_string() << " (sum " << symbol.stddev.sum() << "), threshold: 50" << endl;
    }
    return symbol.stddev.sum() > 50;
}





class Test_BoxShinySymbolDetector : public UnitTest{
public:
    Test_BoxShinySymbolDetector(
        const std::string& image,
        bool expected
    )
        : UnitTest("PokemonSwSh::BoxShinySymbolDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        BoxShinySymbolDetector detector;
        ImageRGB32 image(m_image);
        return detector.detect(image) == m_expected;
    };

private:
    std::string m_image;
    bool m_expected;
};




void add_tests_BoxShinySymbolDetector(UnitTestDatabase& database){
    database.add<Test_BoxShinySymbolDetector>("PokemonSwSh/BoxShinySymbolDetector/macOS_bright/Drifblim_Jpn_False.jpg", false);
    database.add<Test_BoxShinySymbolDetector>("PokemonSwSh/BoxShinySymbolDetector/macOS_bright/Nuzleaf_True.jpg", true);
    database.add<Test_BoxShinySymbolDetector>("PokemonSwSh/BoxShinySymbolDetector/Switch/Centiskorch_False.jpg", false);
    database.add<Test_BoxShinySymbolDetector>("PokemonSwSh/BoxShinySymbolDetector/Switch/Claydol_False.jpg", false);
    database.add<Test_BoxShinySymbolDetector>("PokemonSwSh/BoxShinySymbolDetector/Switch/Nuzleaf_True.jpg", true);
    database.add<Test_BoxShinySymbolDetector>("PokemonSwSh/BoxShinySymbolDetector/WinMyPin/Gardevoir_False.png", false);
    database.add<Test_BoxShinySymbolDetector>("PokemonSwSh/BoxShinySymbolDetector/WinMyPin/Lilligant_True.png", true);
    database.add<Test_BoxShinySymbolDetector>("PokemonSwSh/BoxShinySymbolDetector/WinNoNameBrand/Gardevoir_True.png", true);
    database.add<Test_BoxShinySymbolDetector>("PokemonSwSh/BoxShinySymbolDetector/WinNoNameBrand/Urshifu_False.png", false);
    database.add<Test_BoxShinySymbolDetector>("PokemonSwSh/BoxShinySymbolDetector/WinNoNameBrand/Zigzagoon_False.png", false);
}










}
}
}
