/*  Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalAutoPaths.h"
#include <algorithm>
#include <exception>
#include <QCoreApplication>
#include <QDir>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonBDSP_MenuDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


const double STRIPE_TOP = 0.110;
const double STRIPE_TOP_BELOW_BANNER = 0.170;
const double STRIPE_BOTTOM = 0.598;

static ImageFloatBox menu_stripe(size_t index, bool skip_new_banner){
    double top = skip_new_banner ? STRIPE_TOP_BELOW_BANNER : STRIPE_TOP;
    return ImageFloatBox(0.160 + 0.166 * (double)index, top, 0.015, STRIPE_BOTTOM - top);
}

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
    for (const ImageFloatBox& box : {m_confirm_button, m_save_button, m_close_button, m_save_shortcut}){
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
    // Retain the existing layout check for captures supported by the old detector.
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

class Test_MenuDetector : public UnitTest{
public:
    Test_MenuDetector() : UnitTest("PokemonBDSP::MenuDetector screenshot tests"){}
    UnitTestResult run(Logger& logger, CancellableScope&) const override{
        return run_menu_detector_tests(logger, menu_detector_test_folder());
    }
};

void add_tests_MenuDetector(UnitTestDatabase& database){
    database.add<Test_MenuDetector>();
}


}
}
}
