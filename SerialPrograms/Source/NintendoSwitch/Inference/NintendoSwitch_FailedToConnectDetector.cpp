/*  Failed To Connect Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Globals.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "NintendoSwitch_FailedToConnectDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{



FailedToConnectDetector::FailedToConnectDetector(Color color)
    : m_color(color)
    , m_close_button(0.215, 0.679, 0.071, 0.031)
    , m_top(0.10, 0.15, 0.80, 0.03)
    , m_bottom(0.10, 0.84, 0.80, 0.04)
{}
void FailedToConnectDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_close_button);
    items.add(m_color, m_top);
    items.add(m_color, m_bottom);
}
bool FailedToConnectDetector::detect(const ImageViewRGB32& screen){
    ImageStats stats_close_button = image_stats(extract_box_reference(screen, m_close_button));
    if (!is_black(stats_close_button)){
        return false;
    }

    ImageStats stats_top = image_stats(extract_box_reference(screen, m_top));
    ImageStats bottom_solid = image_stats(extract_box_reference(screen, m_bottom));

    if (euclidean_distance(stats_top.average, bottom_solid.average) > 10){
        return false;
    }

    bool top_dark_grey = is_grey(stats_top, 0, 100);
    bool bottom_dark_grey = is_grey(bottom_solid, 0, 100);
    if (top_dark_grey != bottom_dark_grey){
        return false;
    }

    bool top_light_grey = is_grey(stats_top, 100, 300);
    bool bottom_light_grey = is_grey(bottom_solid, 100, 300);
    if (top_light_grey != bottom_light_grey){
        return false;
    }

    return true;
}


class Test_FailedToConnectDetector : public UnitTest{
public:
    Test_FailedToConnectDetector(const std::string& image, bool expected)
        : UnitTest("NintendoSwitch::FailedToConnectDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        FailedToConnectDetector detector{};
        ImageRGB32 image(m_image);
        return detector.detect(image) == m_expected;
    }

private:
    std::string m_image;
    bool m_expected;
};


void add_tests_FailedToConnectDetector(UnitTestDatabase& database){
    database.add<Test_FailedToConnectDetector>("NintendoSwitch/FailedToConnectDetector/French_blackBackground_False.png", false);
    database.add<Test_FailedToConnectDetector>("NintendoSwitch/FailedToConnectDetector/French_blackBackground_True.png", true);
    database.add<Test_FailedToConnectDetector>("NintendoSwitch/FailedToConnectDetector/French_whiteBackground_False.png", false);
    database.add<Test_FailedToConnectDetector>("NintendoSwitch/FailedToConnectDetector/French_whiteBackground_True.png", true);
}



}
}
