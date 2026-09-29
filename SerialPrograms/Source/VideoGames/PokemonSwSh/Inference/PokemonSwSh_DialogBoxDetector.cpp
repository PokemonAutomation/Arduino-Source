/*  Dialog Triangle Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <array>

#include "Common/Cpp/Color.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "PokemonSwSh_DialogBoxDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


namespace{
    // This box covers all possible locations of the black triangle arrow
    std::array<ImageFloatBox, 4> BLACK_BOXES{{
        {0.254, 0.806, 0.012, 0.052},
        {0.730, 0.806, 0.016, 0.068},
        {0.741, 0.880, 0.005, 0.055},
        {0.710, 0.880, 0.007, 0.055},
    }};
}


BlackDialogBoxDetector::BlackDialogBoxDetector(
    bool stop_on_detected
)
    : VisualInferenceCallback("BlackDialogBoxDetector")
    , m_stop_on_detected(stop_on_detected)
{}


void BlackDialogBoxDetector::make_overlays(VideoOverlaySet& items) const{
    for (const auto& box : BLACK_BOXES){
        items.add(COLOR_RED, box);
    }
}


bool BlackDialogBoxDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    bool detected = true;
    for (const auto& box : BLACK_BOXES){
        if (is_black(extract_box_reference(frame, box), 200, 35) == false){
            detected = false;
            break;
        }
    }

    m_detected.store(detected, std::memory_order_release);

    return detected && m_stop_on_detected;
}

BlackDialogBoxDetector2::BlackDialogBoxDetector2(Color color)
    : m_color(color)
    , m_bottom(0.184009, 0.955, 0.552, 0.013)
    , m_left(0.175246, 0.807018, 0.019715, 0.064327)
    , m_right(0.808324, 0.810916, 0.014239, 0.146199)
{}
void BlackDialogBoxDetector2::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_bottom);
    items.add(m_color, m_left);
    items.add(m_color, m_right);
}

bool BlackDialogBoxDetector2::detect(const ImageViewRGB32& screen){
    return is_grey(extract_box_reference(screen, m_bottom), 50, 300)
            && is_black(extract_box_reference(screen, m_left), 200, 50)
            && is_black(extract_box_reference(screen, m_right), 200, 50);
}




WhiteDialogBoxDetector::WhiteDialogBoxDetector(Color color)
    : m_color(color)
    , m_right(0.782, 0.850, 0.030, 0.050)
    , m_triangle(color)
{}

void WhiteDialogBoxDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_right);
    m_triangle.make_overlays(items);
}
bool WhiteDialogBoxDetector::detect(const ImageViewRGB32& screen){
    ImageStats right = image_stats(extract_box_reference(screen, m_right));
    if (!is_grey(right, 400, 1000)){
        return false;
    }

    if (!m_triangle.detect(screen)){
        return false;
    }

    return true;
}







class Test_BlackDialogBoxDetector : public UnitTest{
public:
    Test_BlackDialogBoxDetector(
        const std::string& image,
        bool expected
    )
        : UnitTest("PokemonSwSh::BlackDialogBoxDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        BlackDialogBoxDetector detector(true);
        ImageRGB32 image(m_image);
        return detector.process_frame(image, current_time()) == m_expected;
    };

private:
    std::string m_image;
    bool m_expected;
};

class Test_BlackDialogBoxDetector2 : public UnitTest{
public:
    Test_BlackDialogBoxDetector2(
        const std::string& image,
        bool expected
    )
        : UnitTest("PokemonSwSh::BlackDialogBoxDetector2 - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        BlackDialogBoxDetector2 detector;
        ImageRGB32 image(m_image);
        return detector.detect(image) == m_expected;
    };

private:
    std::string m_image;
    bool m_expected;
};


void add_tests_DialogBoxDetector(UnitTestDatabase& database){
    database.add<Test_BlackDialogBoxDetector2>("PokemonSwSh/BlackDialogBoxDetector2/black-dialog-1.png", true);

}



}
}
}

