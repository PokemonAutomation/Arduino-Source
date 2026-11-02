/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Tests/ImageUnitTest.h"
#include "PokemonBDSP_DialogDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


ShortDialogDetector::ShortDialogDetector(Color color)
    : m_color(color)
//    , m_bottom(0.50, 0.91, 0.29, 0.05)
    , m_left_white(0.21, 0.835, 0.008, 0.12)
    , m_left(0.18, 0.835, 0.02, 0.12)
    , m_right_white(0.785, 0.835, 0.008, 0.12)
    , m_right_white_low(0.785, 0.915, 0.008, 0.04)
    , m_right(0.822, 0.835, 0.02, 0.12)
{}
void ShortDialogDetector::make_overlays(VideoOverlaySet& items) const{
//    items.add(m_color, m_bottom);
    items.add(m_color, m_left_white);
    items.add(m_color, m_left);
    items.add(m_color, m_right_white);
    items.add(m_color, m_right);
}
bool ShortDialogDetector::detect(const ImageViewRGB32& screen){
    ImageStats left_white = image_stats(extract_box_reference(screen, m_left_white));
//    cout << left_white.average << left_white.stddev << endl;
    if (!is_white(left_white)){
        return false;
    }
    ImageStats right_white = image_stats(extract_box_reference(screen, m_right_white));
//    cout << right_white.average << right_white.stddev << endl;
    //  A long first line runs into m_right_white (Rowan's "Now, go on, and
    //  leap into the world of Pokemon!"): then the box's right end below it.
    if (!is_white(right_white) && !is_white(image_stats(extract_box_reference(screen, m_right_white_low)))){
        return false;
    }
//    ImageStats bottom = image_stats(extract_box_reference(screen, m_bottom));
//    if (!is_white(bottom)){
//        return false;
//    }
    ImageStats left = image_stats(extract_box_reference(screen, m_left));
//    cout << left.stddev << endl;
    if (left.stddev.sum() < 30){
        return false;
    }
    ImageStats right = image_stats(extract_box_reference(screen, m_right));
//    cout << right.stddev << endl;
    if (right.stddev.sum() < 30){
        return false;
    }
    return true;
}





BattleDialogDetector::BattleDialogDetector(Color color)
    : m_color(color)
    , m_bottom(0.50, 0.91, 0.40, 0.05)
    , m_left_white(0.07, 0.835, 0.008, 0.12)
    , m_left(0.04, 0.835, 0.02, 0.12)
    , m_right(0.965, 0.835, 0.02, 0.12)
{}
void BattleDialogDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_bottom);
    items.add(m_color, m_left_white);
    items.add(m_color, m_left);
    items.add(m_color, m_right);
}
bool BattleDialogDetector::detect(const ImageViewRGB32& screen){
    ImageStats left_white = image_stats(extract_box_reference(screen, m_left_white));
    if (!is_white(left_white)){
        return false;
    }
    ImageStats bottom = image_stats(extract_box_reference(screen, m_bottom));
    if (!is_white(bottom)){
        return false;
    }
    ImageStats left = image_stats(extract_box_reference(screen, m_left));
//    cout << left.stddev << endl;
    if (left.stddev.sum() < 50){
        return false;
    }
    ImageStats right = image_stats(extract_box_reference(screen, m_right));
//    cout << right.stddev << endl;
    if (right.stddev.sum() < 50){
        return false;
    }
    return true;
}




ShortDialogPromptDetector::ShortDialogPromptDetector(
    VideoOverlay& overlay,
    const ImageFloatBox& box,
    Color color
)
    : VisualInferenceCallback("ShortDialogPromptDetector")
    , m_dialog(color)
    , m_arrow(overlay, box, color)
{}
void ShortDialogPromptDetector::make_overlays(VideoOverlaySet& items) const{
    m_dialog.make_overlays(items);
    m_arrow.make_overlays(items);
}
bool ShortDialogPromptDetector::process_frame(const ImageViewRGB32& screen, WallClock timestamp){
    return m_dialog.detect(screen) && m_arrow.process_frame(screen, timestamp);
}









class Test_ShortDialogDetector : public ImageUnitTest{
public:
    Test_ShortDialogDetector(const std::string& image, bool expected)
        : ImageUnitTest("PokemonBDSP::ShortDialogDetector - " + image, {image})
        , m_expected(expected)
    {}

    virtual UnitTestResult run_on_images(
        Logger&, CancellableScope&,
        const std::vector<ImageRGB32>& images
    ) const override{
        return ShortDialogDetector().detect(images[0]) == m_expected;
    }

private:
    bool m_expected;
};

void add_tests_DialogDetector(UnitTestDatabase& database){
    database.add<Test_ShortDialogDetector>("PokemonBDSP/DialogDetector/IntroRowanNameTab_True.png", true);
    database.add<Test_ShortDialogDetector>("PokemonBDSP/DialogDetector/IntroRowanNoArrow_True.png", true);
    //  A short line: the box's right end is white where the old check looks.
    database.add<Test_ShortDialogDetector>("PokemonBDSP/DialogDetector/IntroRowanShortLine_True.png", true);
    //  A "???" name tab, in Jubilife's yellow plaza after the tag battle.
    database.add<Test_ShortDialogDetector>("PokemonBDSP/DialogDetector/JubilifeUnknownNameTab_True.png", true);
    //  One line, no name tab, over Floaroma's flowers.
    database.add<Test_ShortDialogDetector>("PokemonBDSP/DialogDetector/FloaromaOldManOneLine_True.png", true);
    database.add<Test_ShortDialogDetector>("PokemonBDSP/DialogDetector/FloaromaCloudShadows_False.png", false);
}


}
}
}
