/*  Check Online Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Globals.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "NintendoSwitch_CheckOnlineDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



CheckOnlineDetector::CheckOnlineDetector(Color color, bool invert)
    : m_color(color)
    , m_invert(invert)
    , m_box_top(0.25, 0.32, 0.50, 0.02)
    , m_box_mid(0.25, 0.57, 0.50, 0.02)
    , m_top(0.10, 0.15, 0.80, 0.03)
    , m_left(0.08, 0.25, 0.10, 0.38)
    , m_bottom_solid(0.10, 0.84, 0.80, 0.04)
    , m_bottom_buttons(0.70, 0.92, 0.28, 0.05)
{}
void CheckOnlineDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box_top);
    items.add(m_color, m_box_mid);
    items.add(m_color, m_top);
    items.add(m_color, m_left);
    items.add(m_color, m_bottom_solid);
    items.add(m_color, m_bottom_buttons);
}
bool CheckOnlineDetector::detect(const ImageViewRGB32& screen){
    ImageStats stats_box_top = image_stats(extract_box_reference(screen, m_box_top));
//    cout << stats_box_top.average << stats_box_top.stddev << endl;
    bool white;
    if (stats_box_top.average.sum() < 300){
        white = false;
    }else if (stats_box_top.average.sum() > 500){
        white = true;
    }else{
        return m_invert;
    }
    if (stats_box_top.stddev.sum() > 10){
        return m_invert;
    }

//    cout << "white: " << white << endl;

    ImageStats stats_box_mid = image_stats(extract_box_reference(screen, m_box_mid));
    if (stats_box_mid.stddev.sum() > 10){
        return m_invert;
    }
    if (euclidean_distance(stats_box_top.average, stats_box_mid.average) > 10){
        return m_invert;
    }

    ImageStats stats_left = image_stats(extract_box_reference(screen, m_left));
//    cout << stats_left.stddev << endl;
    if (stats_left.stddev.sum() < 30){
//        cout << "zxcv" << endl;
        return m_invert;
    }

    ImageStats stats_top = image_stats(extract_box_reference(screen, m_top));
//    cout << stats_top.average << stats_top.stddev << endl;

    ImageStats bottom_solid = image_stats(extract_box_reference(screen, m_bottom_solid));
//    cout << bottom_solid.average << bottom_solid.stddev << endl;

    if (euclidean_distance(stats_top.average, bottom_solid.average) > 10){
//        cout << "qwer" << endl;
        return m_invert;
    }

    if (white){
        if (!is_grey(stats_top, 100, 300) || !is_grey(bottom_solid, 100, 300)){
//            cout << "asdf" << endl;
            return m_invert;
        }
    }else{
        if (!is_grey(stats_top, 0, 100) || !is_grey(bottom_solid, 0, 100)){
//            cout << "zxcv" << endl;
            return m_invert;
        }
    }

    ImageStats stats_bottom_buttons = image_stats(extract_box_reference(screen, m_bottom_buttons));
//    cout << stats_bottom_buttons.average << stats_bottom_buttons.stddev << endl;
    if (stats_bottom_buttons.stddev.sum() < 30){
        return m_invert;
    }

    return !m_invert;
}


class Test_CheckOnlineDetector : public UnitTest{
public:
    Test_CheckOnlineDetector(const std::string& image, bool expected)
        : UnitTest("NintendoSwitch::CheckOnlineDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        CheckOnlineDetector detector{};
        ImageRGB32 image(m_image);
        return detector.detect(image) == m_expected;
    }

private:
    std::string m_image;
    bool m_expected;
};


void add_tests_CheckOnlineDetector(UnitTestDatabase& database){
    database.add<Test_CheckOnlineDetector>("NintendoSwitch/CheckOnlineDetector/French_blackBackground_False.png", false);
    database.add<Test_CheckOnlineDetector>("NintendoSwitch/CheckOnlineDetector/French_blackBackground_True.png", true);
    database.add<Test_CheckOnlineDetector>("NintendoSwitch/CheckOnlineDetector/French_whiteBackground_False.png", false);
    database.add<Test_CheckOnlineDetector>("NintendoSwitch/CheckOnlineDetector/French_whiteBackground_True.png", true);
}



}
}
