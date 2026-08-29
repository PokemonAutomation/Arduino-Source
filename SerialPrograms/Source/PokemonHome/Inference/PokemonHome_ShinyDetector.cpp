/*  Shiny Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "Tests/TestUtils.h"
#include "PokemonHome_ShinyDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


ShinyDetector::ShinyDetector(Color color, VideoOverlay* overlay)
    : m_color(color)
    , m_overlay(overlay)
    , m_box(0.702, 0.09, 0.04, 0.06)
{}

void ShinyDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}

bool ShinyDetector::detect(const ImageViewRGB32& screen){
    const bool found = image_stddev(extract_box_reference(screen, m_box)).sum() > 30;
    if (m_overlay){
        if (found){
            m_last_detected_box.emplace(*m_overlay, m_box, COLOR_GREEN);
        }else{
            m_last_detected_box.reset();
        }
    }
    return found;
}

class Test_ShinyDetector : public UnitTest{
public:
    Test_ShinyDetector(const std::string& image, bool expected)
        : UnitTest("PokemonHome::ShinyDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        ImageRGB32 image(m_image);
        ShinyDetector detector;
        return detector.detect(image) == m_expected;
    }

private:
    std::string m_image;
    bool m_expected;
};

void add_tests_ShinyDetector(UnitTestDatabase& database){
}


}
}
}
