/*  Map Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonSV_MapMenuDetector.h"
#include "Tests/TestUtils.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

MapFlyMenuDetector::~MapFlyMenuDetector() = default;

MapFlyMenuDetector::MapFlyMenuDetector(Color color)
    : m_color(color)
    , m_middle_box(0.523, 0.680, 0.080, 0.010)
    , m_bottom_box(0.523, 0.744, 0.080, 0.020)
{}

void MapFlyMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_middle_box);
    items.add(m_color, m_bottom_box);
}

bool MapFlyMenuDetector::detect(const ImageViewRGB32& screen){
    const ImageStats stats0 = image_stats(extract_box_reference(screen, m_middle_box));
    // The white menu background is actually a bit translucent. So we have to relax the `is_white()` condition.
    const double min_rgb_sum = 500.0;
    const double min_stddev_sum = 25.0;
    if (!is_white(stats0, min_rgb_sum, min_stddev_sum)){
        return false;
    }
    const ImageStats stats1 = image_stats(extract_box_reference(screen, m_bottom_box));
    return is_white(stats1, min_rgb_sum, min_stddev_sum);
}


MapFlyMenuWatcher::~MapFlyMenuWatcher() = default;

MapFlyMenuWatcher::MapFlyMenuWatcher(Color color)
    : VisualInferenceCallback("MapFlyMenuWatcher")
    , m_detector(color)
{}

void MapFlyMenuWatcher::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}

bool MapFlyMenuWatcher::process_frame(const ImageViewRGB32& screen, WallClock timestamp){
    return m_detector.detect(screen);
}


MapDestinationMenuDetector::MapDestinationMenuDetector(Color color)
    : m_color(color)
    , m_bottom_box(0.523, 0.670, 0.080, 0.020)
    , m_fly_menu_box(0.523, 0.744, 0.080, 0.020)
{}

void MapDestinationMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_bottom_box);
    items.add(m_color, m_fly_menu_box);
}

bool MapDestinationMenuDetector::detect(const ImageViewRGB32& screen){
    const ImageStats stats0 = image_stats(extract_box_reference(screen, m_bottom_box));
    // The white menu background is actually a bit translucent. So we have to relax the `is_white()` condition.
    const double min_rgb_sum = 500.0;
    const double min_stddev_sum = 25.0;
    if (!is_white(stats0, min_rgb_sum, min_stddev_sum)){
        return false;
    }
    const ImageStats stats1 = image_stats(extract_box_reference(screen, m_fly_menu_box));
    return !is_white(stats1, min_rgb_sum, min_stddev_sum);
}


MapDestinationMenuWatcher::~MapDestinationMenuWatcher() = default;

MapDestinationMenuWatcher::MapDestinationMenuWatcher(Color color)
    : VisualInferenceCallback("MapDestinationMenuWatcher")
    , m_detector(color)
{}

void MapDestinationMenuWatcher::make_overlays(VideoOverlaySet& items) const{
    m_detector.make_overlays(items);
}

bool MapDestinationMenuWatcher::process_frame(const ImageViewRGB32& screen, WallClock timestamp){
    return m_detector.detect(screen);
}


class Test_MapFlyMenuDetector : public UnitTest{
public:
    Test_MapFlyMenuDetector(const std::string& image, bool expected)
        : UnitTest("PokemonSV::MapFlyMenuDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        MapFlyMenuDetector fly_menu(COLOR_RED);
        MapDestinationMenuDetector dest_menu(COLOR_RED);
        ImageRGB32 image(m_image);
        TEST_RESULT_EQUAL(fly_menu.detect(image), m_expected);
        TEST_RESULT_EQUAL(dest_menu.detect(image), !m_expected);
        return true;
    }

private:
    std::string m_image;
    bool m_expected;
};


void add_tests_MapMenuDetector(UnitTestDatabase& database){
    database.add<Test_MapFlyMenuDetector>("PokemonSV/MapFlyMenuDetector/ABitOutsideSouthGate_True.png", true);
    database.add<Test_MapFlyMenuDetector>("PokemonSV/MapFlyMenuDetector/AtSouthGate_True.png", true);
    database.add<Test_MapFlyMenuDetector>("PokemonSV/MapFlyMenuDetector/FraKitakamiForest_False.png", false);
    database.add<Test_MapFlyMenuDetector>("PokemonSV/MapFlyMenuDetector/FraKitakamiTown_True.png", true);
    database.add<Test_MapFlyMenuDetector>("PokemonSV/MapFlyMenuDetector/SouthGate_False.png", false);
    database.add<Test_MapFlyMenuDetector>("PokemonSV/MapFlyMenuDetector/SouthGate_True.png", true);
    database.add<Test_MapFlyMenuDetector>("PokemonSV/MapFlyMenuDetector/WestOfSouthGate_False.png", false);
}






}
}
}
