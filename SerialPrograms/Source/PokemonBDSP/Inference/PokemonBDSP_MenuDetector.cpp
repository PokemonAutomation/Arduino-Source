/*  Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <array>
#include <format>
#include <string_view>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "Tests/ImageUnitTest.h"
#include "PokemonBDSP_MenuDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


namespace{

constexpr double STRIPE_TOP = 0.110;
constexpr double STRIPE_TOP_BELOW_BANNER = 0.170;
constexpr double STRIPE_BOTTOM = 0.598;

//  Each column sits in the gap right of an icon, clear of the "NEW" tag on
//  its top right corner. The tag on TOWN MAP extends into a column at 0.160.
constexpr double STRIPE_LEFT = 0.166;

ImageFloatBox menu_stripe(size_t index, bool skip_new_banner){
    double top = skip_new_banner ? STRIPE_TOP_BELOW_BANNER : STRIPE_TOP;
    return ImageFloatBox(STRIPE_LEFT + 0.166 * (double)index, top, 0.015, STRIPE_BOTTOM - top);
}

} // namespace

MenuDetector::MenuDetector(Color color, bool skip_new_banner)
    : m_color(color)
    , m_line0(menu_stripe(0, skip_new_banner))
    , m_line1(menu_stripe(1, skip_new_banner))
    , m_line2(menu_stripe(2, skip_new_banner))
    , m_line3(menu_stripe(3, skip_new_banner))
    , m_line4(menu_stripe(4, skip_new_banner))
    , m_cross(0.20, 0.15, 0.60, 0.37)
    , m_panel_top(0.150, 0.100, 0.700, 0.015)
    , m_panel_left(0.150, 0.125, 0.020, 0.460)
    , m_panel_right(0.835, 0.125, 0.015, 0.460)
    , m_footer(0.050, 0.963, 0.600, 0.025)
    , m_confirm_button(0.736, 0.960, 0.022, 0.038)
    , m_confirm_button_with_swap(0.653, 0.960, 0.022, 0.038)
    , m_save_button(0.835, 0.960, 0.024, 0.038)
    , m_close_button(0.915, 0.960, 0.024, 0.038)
    , m_save_heading(0.920, 0.755, 0.070, 0.040)
    , m_save_background(0.973, 0.820, 0.020, 0.065)
    , m_save_shortcut(0.936, 0.826, 0.029, 0.053)
{}

void MenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_line0);
    items.add(m_color, m_line1);
    items.add(m_color, m_line2);
    items.add(m_color, m_line3);
    items.add(m_color, m_line4);
    items.add(m_color, m_cross);
    items.add(m_color, m_panel_top);
    items.add(m_color, m_panel_left);
    items.add(m_color, m_panel_right);
    items.add(m_color, m_footer);
    items.add(m_color, m_confirm_button);
    items.add(m_color, m_confirm_button_with_swap);
    items.add(m_color, m_save_button);
    items.add(m_color, m_close_button);
    items.add(m_color, m_save_heading);
    items.add(m_color, m_save_background);
    items.add(m_color, m_save_shortcut);
}
// A/R/B prompts and the Save shortcut have dark neutral button backgrounds
// and light lettering. Check their contrast without OCR or image resources.
static bool has_menu_button(const ImageViewRGB32& patch){
    size_t dark = 0;
    size_t light = 0;
    for (size_t y = 0; y < patch.height(); y++){
        for (size_t x = 0; x < patch.width(); x++){
            const uint32_t pixel = patch.pixel(x, y);
            const int r = (pixel >> 16) & 0xff;
            const int g = (pixel >> 8) & 0xff;
            const int b = pixel & 0xff;
            const int low = std::min(r, std::min(g, b));
            const int high = std::max(r, std::max(g, b));
            if (high - low < 35){
                dark += high < 160;
                light += low > 200;
            }
        }
    }
    const double pixels = (double)patch.width() * patch.height();
    return pixels > 0 && dark / pixels >= 0.15 && light / pixels >= 0.08;
}

bool MenuDetector::detect_menu_chrome(const ImageViewRGB32& screen) const{
    // The surrounding panel remains the same with six or eight unlocked icons.
    for (const ImageFloatBox& box : {m_panel_top, m_panel_left, m_panel_right}){
        if (!is_white(image_stats(extract_box_reference(screen, box)))){
            return false;
        }
    }
    const ImageStats footer = image_stats(extract_box_reference(screen, m_footer));
    if (footer.average.b < footer.average.r + 35 ||
        footer.average.g < footer.average.r + 20 || footer.stddev.sum() > 60){
        return false;
    }
    // Unlocking the Swap prompt shifts A Confirm left. Require the same
    // button contrast at either documented footer position, with all other
    // panel, footer and Save checks unchanged.
    if (!has_menu_button(extract_box_reference(screen, m_confirm_button)) &&
        !has_menu_button(extract_box_reference(screen, m_confirm_button_with_swap))){
        return false;
    }
    for (const ImageFloatBox& box : {m_save_button, m_close_button, m_save_shortcut}){
        if (!has_menu_button(extract_box_reference(screen, box))){
            return false;
        }
    }
    // Require both the light Save heading and its cyan background as well.
    const ImageStats heading = image_stats(extract_box_reference(screen, m_save_heading));
    const ImageStats background = image_stats(extract_box_reference(screen, m_save_background));
    return heading.average.r > 180 && heading.average.g > 180 && heading.average.b > 180 &&
        heading.stddev.sum() > 30 &&
        background.average.g > background.average.r + 40 &&
        background.average.b > background.average.r + 40 && background.stddev.sum() < 90;
}

bool MenuDetector::detect(const ImageViewRGB32& screen){
    if (detect_menu_chrome(screen)){
        return true;
    }
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

//  Selects full-height columns or columns starting below the NEW tags.
enum class Columns{
    FromTop,
    BelowNewTags,
};
//  A screen and the existing detector mode used to inspect it.
struct MenuInput{
    std::string_view image;
    Columns columns = Columns::FromTop;
};
//  Whether the screen should be recognized as the X menu.
struct MenuExpectation{
    bool detected = false;
};
//  One screen/mode pair and its independently specified expected result.
struct MenuCase{
    MenuInput input;
    MenuExpectation expectation;
};

//  Verifies both detector modes, including the image-test color distortions.
class Test_MenuDetector : public ImageUnitTest{
public:
    explicit Test_MenuDetector(const MenuCase& test)
        : ImageUnitTest(
            std::format("PokemonBDSP::MenuDetector{} - {}",
                test.input.columns == Columns::BelowNewTags ? "(skip_new_banner)" : "", test.input.image),
            {std::string(test.input.image)}
        )
        , m_columns(test.input.columns)
        , m_expectation(test.expectation)
    {}

    virtual UnitTestResult run_on_images(
        Logger&, CancellableScope&,
        const std::vector<ImageRGB32>& images
    ) const override{
        MenuDetector detector(COLOR_RED, /*skip_new_banner=*/m_columns == Columns::BelowNewTags);
        const bool detected = detector.detect(images[0]);
        if (detected != m_expectation.detected){
            return std::format("Expected menu detected={}, read {}.", m_expectation.detected, detected);
        }
        return true;
    }

private:
    Columns m_columns = Columns::FromTop;
    MenuExpectation m_expectation;
};

} // namespace

void add_tests_MenuDetector(UnitTestDatabase& database){
    static constexpr std::array<MenuCase, 20> CASES{{
        {.input = {.image = "PokemonBDSP/MenuDetector/NewGameBagOptions_True.png", .columns = Columns::FromTop},
         .expectation = {.detected = true}},
        {.input = {.image = "PokemonBDSP/MenuDetector/NewGameBagOptions_True.png", .columns = Columns::BelowNewTags},
         .expectation = {.detected = true}},
        {.input = {.image = "PokemonBDSP/MenuDetector/JubilifeTownMapNew_True.png", .columns = Columns::FromTop},
         .expectation = {.detected = true}},
        {.input = {.image = "PokemonBDSP/MenuDetector/JubilifeTownMapNew_True.png", .columns = Columns::BelowNewTags},
         .expectation = {.detected = true}},
        {.input = {.image = "PokemonBDSP/MenuDetector/FloaromaMenuThreeNew_True.png", .columns = Columns::FromTop},
         .expectation = {.detected = true}},
        {.input = {.image = "PokemonBDSP/MenuDetector/FloaromaMenuThreeNew_True.png", .columns = Columns::BelowNewTags},
         .expectation = {.detected = true}},
        {.input = {.image = "PokemonBDSP/MenuDetector/Home_False.png", .columns = Columns::FromTop},
         .expectation = {.detected = false}},
        {.input = {.image = "PokemonBDSP/MenuDetector/Home_False.png", .columns = Columns::BelowNewTags},
         .expectation = {.detected = false}},
        {.input = {.image = "PokemonBDSP/MenuDetector/OverworldSandgemMart_False.png", .columns = Columns::FromTop},
         .expectation = {.detected = false}},
        {.input = {.image = "PokemonBDSP/MenuDetector/OverworldSandgemMart_False.png", .columns = Columns::BelowNewTags},
         .expectation = {.detected = false}},
        {.input = {.image = "PokemonBDSP/MenuDetector/OverworldOreburghMine_False.png", .columns = Columns::FromTop},
         .expectation = {.detected = false}},
        {.input = {.image = "PokemonBDSP/MenuDetector/OverworldOreburghMine_False.png", .columns = Columns::BelowNewTags},
         .expectation = {.detected = false}},
        {.input = {.image = "PokemonBDSP/MenuDetector/IntroDialog_False.png", .columns = Columns::FromTop},
         .expectation = {.detected = false}},
        {.input = {.image = "PokemonBDSP/MenuDetector/IntroDialog_False.png", .columns = Columns::BelowNewTags},
         .expectation = {.detected = false}},
        {.input = {.image = "PokemonBDSP/MenuDetector/IntroBarry_False.png", .columns = Columns::FromTop},
         .expectation = {.detected = false}},
        {.input = {.image = "PokemonBDSP/MenuDetector/IntroBarry_False.png", .columns = Columns::BelowNewTags},
         .expectation = {.detected = false}},
        {.input = {.image = "PokemonBDSP/MenuDetector/FloaromaOldManDialog_False.png", .columns = Columns::FromTop},
         .expectation = {.detected = false}},
        {.input = {.image = "PokemonBDSP/MenuDetector/FloaromaOldManDialog_False.png", .columns = Columns::BelowNewTags},
         .expectation = {.detected = false}},
        {.input = {.image = "PokemonBDSP/MenuDetector/FloaromaCloudShadows_False.png", .columns = Columns::FromTop},
         .expectation = {.detected = false}},
        {.input = {.image = "PokemonBDSP/MenuDetector/FloaromaCloudShadows_False.png", .columns = Columns::BelowNewTags},
         .expectation = {.detected = false}},
    }};
    for (const MenuCase& test : CASES){
        database.add<Test_MenuDetector>(test);
    }
    for (const char* image : {"Options_4.png", "Pokemon_6.png", "Capsules_8.png", "Bag_6.png",
        "TrainerCard_6_06.png", "TrainerCard_6_08.png", "TrainerCard_6_07.png", "SelectionAnimation.png"}){
        const std::string path = std::string("PokemonBDSP/MenuDetector/") + image;
        database.add<Test_MenuDetector>(MenuCase{.input = {.image = path}, .expectation = {.detected = true}});
    }
    for (const char* image : {
        "Bag_6_01.png",
        "Bag_6_02.png",
        "Bag_6_03.png",
        "Bag_6_04.png",
        "Options_6_01.png",
        "Options_6_02.png",
        "Options_6_03.png",
        "Options_6_04.png",
        "Pokedex_6_01.png",
        "Pokedex_6_02.png",
        "Pokedex_6_03.png",
        "Pokemon_6_01.png",
        "Pokemon_6_02.png",
        "Pokemon_6_03.png",
        "Pokemon_6_04.png",
        "TownMap_6_01.png",
        "TownMap_6_02.png",
        "TownMap_6_03.png",
        "TownMap_6_04.png",
        "TownMap_6_05.png",
        "TrainerCard_6_01.png",
        "TrainerCard_6_02.png",
        "TrainerCard_6_03.png",
        "TrainerCard_6_04.png",
        "TrainerCard_6_05.png"
    }){
        const std::string path = std::string("PokemonBDSP/MenuDetector/") + image;
        for (Columns columns : {Columns::FromTop, Columns::BelowNewTags}){
            database.add<Test_MenuDetector>(MenuCase{
                .input = {.image = path, .columns = columns},
                .expectation = {.detected = true}
            });
        }
    }
}

} // namespace PokemonBDSP
} // namespace NintendoSwitch
} // namespace PokemonAutomation
