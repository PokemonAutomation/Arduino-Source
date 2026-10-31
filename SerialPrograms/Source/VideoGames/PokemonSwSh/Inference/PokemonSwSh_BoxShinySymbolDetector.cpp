/*  Box Shiny Symbol Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 *
 */

#include <iostream>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/StaticGlobals.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Tests/ImageUnitTest.h"
#include "PokemonSwSh_BoxShinySymbolDetector.h"

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





class Test_BoxShinySymbolDetector : public ImageUnitTest{
public:
    Test_BoxShinySymbolDetector(
        const std::string& image,
        bool expected
    )
        : ImageUnitTest("PokemonSwSh::BoxShinySymbolDetector - " + image, {image})
        , m_expected(expected)
    {}

    virtual UnitTestResult run_on_images(
        Logger& logger, CancellableScope& scope,
        const std::vector<ImageRGB32>& images
    ) const override{
        BoxShinySymbolDetector detector;
        const ImageRGB32& image = images[0];
        return detector.detect(image) == m_expected;
    };

private:
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
