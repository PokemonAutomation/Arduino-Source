/*  Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Tests/ImageUnitTest.h"
#include "PokemonBDSP_MenuDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


const double STRIPE_TOP = 0.110;
const double STRIPE_TOP_BELOW_BANNER = 0.170;
const double STRIPE_BOTTOM = 0.598;

//  Each column sits in the gap right of an icon, clear of the "NEW" tag on
//  its top right corner: from 0.160 the column took the edge of the tag on
//  TOWN MAP and the menu went unseen (r65, Jubilife).
const double STRIPE_LEFT = 0.166;

static ImageFloatBox menu_stripe(size_t index, bool skip_new_banner){
    double top = skip_new_banner ? STRIPE_TOP_BELOW_BANNER : STRIPE_TOP;
    return ImageFloatBox(STRIPE_LEFT + 0.166 * (double)index, top, 0.015, STRIPE_BOTTOM - top);
}

MenuDetector::MenuDetector(Color color, bool skip_new_banner)
    : m_color(color)
    , m_line0(menu_stripe(0, skip_new_banner))
    , m_line1(menu_stripe(1, skip_new_banner))
    , m_line2(menu_stripe(2, skip_new_banner))
    , m_line3(menu_stripe(3, skip_new_banner))
    , m_line4(menu_stripe(4, skip_new_banner))
    , m_cross(0.20, 0.15, 0.60, 0.37)
{}

void MenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_line0);
    items.add(m_color, m_line1);
    items.add(m_color, m_line2);
    items.add(m_color, m_line3);
    items.add(m_color, m_line4);
    items.add(m_color, m_cross);
}
bool MenuDetector::detect(const ImageViewRGB32& screen){
    ImageStats stats0 = image_stats(extract_box_reference(screen, m_line0));
    if (!is_white(stats0)){
        return false;
    }
    ImageStats stats1 = image_stats(extract_box_reference(screen, m_line1));
    if (!is_white(stats1)){
        return false;
    }
    ImageStats stats2 = image_stats(extract_box_reference(screen, m_line2));
    if (!is_white(stats2)){
        return false;
    }
    ImageStats stats3 = image_stats(extract_box_reference(screen, m_line3));
    if (!is_white(stats3)){
        return false;
    }
    ImageStats stats4 = image_stats(extract_box_reference(screen, m_line4));
    if (!is_white(stats4)){
        return false;
    }
    ImageStats cross = image_stats(extract_box_reference(screen, m_cross));
//    cout << cross.average << cross.stddev << endl;
    if (cross.stddev.sum() < 100){
        return false;
    }
    return true;
}


MenuWatcher::MenuWatcher(Color color, bool skip_new_banner)
    : MenuDetector(color, skip_new_banner)
    , VisualInferenceCallback("MenuWatcher")
{}
void MenuWatcher::make_overlays(VideoOverlaySet& items) const{
    MenuDetector::make_overlays(items);
}
bool MenuWatcher::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    return detect(frame);
}



namespace{

//  Which columns the detector checks: the default ones from the top, or the
//  ones below the "NEW" tags (a new game's menu, skip_new_banner = true).
enum class Columns{
    FromTop,
    BelowNewTags,
};

class Test_MenuDetector : public ImageUnitTest{
public:
    Test_MenuDetector(const std::string& image, Columns columns, bool expected)
        : ImageUnitTest(
            "PokemonBDSP::MenuDetector" + std::string(columns == Columns::BelowNewTags ? "(skip_new_banner)" : "") + " - " + image,
            {image}
        )
        , m_columns(columns)
        , m_expected(expected)
    {}

    virtual UnitTestResult run_on_images(
        Logger&, CancellableScope&,
        const std::vector<ImageRGB32>& images
    ) const override{
        MenuDetector detector(COLOR_RED, m_columns == Columns::BelowNewTags);
        return detector.detect(images[0]) == m_expected;
    }

private:
    Columns m_columns;
    bool m_expected;
};

}

void add_tests_MenuDetector(UnitTestDatabase& database){
    //  The new game's menu: a "NEW" tag above OPTIONS. Jubilife after the
    //  Town Map: one on TOWN MAP, and the quest's text below.
    for (const char* image : {
        "PokemonBDSP/MenuDetector/NewGameBagOptions_True.png",
        "PokemonBDSP/MenuDetector/JubilifeTownMapNew_True.png",
        //  Three NEW tags (Trainer Card, Town Map, Ball Capsules), cursor on Bag.
        "PokemonBDSP/MenuDetector/FloaromaMenuThreeNew_True.png",
    }){
        database.add<Test_MenuDetector>(image, Columns::FromTop, true);
        database.add<Test_MenuDetector>(image, Columns::BelowNewTags, true);
    }
    for (const char* image : {
        "PokemonBDSP/MenuDetector/Home_False.png",
        "PokemonBDSP/MenuDetector/OverworldSandgemMart_False.png",
        "PokemonBDSP/MenuDetector/OverworldOreburghMine_False.png",
        "PokemonBDSP/MenuDetector/IntroDialog_False.png",
        "PokemonBDSP/MenuDetector/IntroBarry_False.png",
        "PokemonBDSP/MenuDetector/FloaromaOldManDialog_False.png",
        //  Cloud shadows over Floaroma's flower field.
        "PokemonBDSP/MenuDetector/FloaromaCloudShadows_False.png",
    }){
        database.add<Test_MenuDetector>(image, Columns::FromTop, false);
        database.add<Test_MenuDetector>(image, Columns::BelowNewTags, false);
    }
}


}
}
}
