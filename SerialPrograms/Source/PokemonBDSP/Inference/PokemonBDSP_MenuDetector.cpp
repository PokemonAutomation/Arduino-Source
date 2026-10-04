/*  Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include <exception>
#include <QCoreApplication>
#include <QDir>
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
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

std::string menu_detector_test_folder(){
    QDir directory(QCoreApplication::applicationDirPath());
    for (size_t level = 0; level < 6; level++){
        const QString folder = directory.filePath("Packages/Resources/PokemonBDSP/menu_detector");
        if (QDir(folder).exists()){
            return QDir(folder).absolutePath().toStdString();
        }
        if (!directory.cdUp()) break;
    }
    return RESOURCE_PATH() + "PokemonBDSP/menu_detector";
}

bool run_menu_detector_tests(Logger& logger, const std::string& folder){
    const QDir directory(QString::fromStdString(folder));
    const auto files = directory.entryList({"*.png"}, QDir::Files, QDir::Name);
    if (files.isEmpty()){
        logger.log("[Menu tests] No PNG screenshots in " + folder, COLOR_RED);
        return false;
    }
    size_t passed = 0;
    size_t failed = 0;
    auto check = [&](bool actual, bool expected, const std::string& label){
        const bool success = actual == expected;
        (success ? passed : failed)++;
        logger.log(std::string(success ? "[PASS] " : "[FAIL] ") + label +
            ": detected=" + (actual ? "true" : "false") +
            ", expected=" + (expected ? "true" : "false"), success ? COLOR_GREEN : COLOR_RED);
    };
    for (const QString& filename : files){
        const bool expected = !filename.startsWith("non_menu_");
        try{
            const ImageRGB32 image(directory.filePath(filename).toStdString());
            for (size_t width : {size_t(1920), size_t(1280), size_t(960)}){
                const ImageRGB32 scaled = image.scale_to(width, width * 9 / 16);
                MenuDetector detector;
                check(detector.detect(scaled), expected,
                    filename.toStdString() + " at " + std::to_string(width) + "px");
            }
        }catch (const std::exception& error){
            failed++;
            logger.log("[FAIL] " + filename.toStdString() + ": " + error.what(), COLOR_RED);
        }
    }
    // Uniform bright/dark frames must not count as an open menu.
    for (uint32_t color : {uint32_t(0xff000000), uint32_t(0xffffffff)}){
        ImageRGB32 blank(1280, 720);
        for (size_t y = 0; y < blank.height(); y++){
            for (size_t x = 0; x < blank.width(); x++) blank.pixel(x, y) = color;
        }
        MenuDetector detector;
        check(detector.detect(blank), false, color == 0xff000000 ? "Black frame" : "White frame");
    }
    logger.log("[Menu tests] " + std::to_string(passed) + " passed, " +
        std::to_string(failed) + " failed.", failed == 0 ? COLOR_GREEN : COLOR_RED);
    return failed == 0;
}

// Adapter for the existing filename-based command-line test map.
int test_MenuDetector(const ImageViewRGB32& image, bool expected){
    for (size_t width : {size_t(1920), size_t(1280), size_t(960)}){
        const ImageRGB32 scaled = image.scale_to(width, width * 9 / 16);
        MenuDetector detector;
        if (detector.detect(scaled) != expected) return 1;
    }
    return 0;
}

class Test_MenuDetector_Legacy : public UnitTest{
public:
    Test_MenuDetector_Legacy() : UnitTest("PokemonBDSP::MenuDetector screenshot tests"){}
    UnitTestResult run(Logger& logger, CancellableScope&) const override{
        return run_menu_detector_tests(logger, menu_detector_test_folder());
    }
};


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
    database.add<Test_MenuDetector_Legacy>();
}

} // namespace PokemonBDSP
} // namespace NintendoSwitch
} // namespace PokemonAutomation
