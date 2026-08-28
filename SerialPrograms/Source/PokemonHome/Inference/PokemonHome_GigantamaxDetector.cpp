/*  Gigantamax Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "PokemonHome_GigantamaxDetector.h"
#include "Tests/TestUtils.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


class GigantamaxMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    GigantamaxMatcher()
        : WaterfillTemplateMatcher(
            "PokemonHome/Home_Gigantamax.png",
            Color(196, 32, 88), Color(255, 220, 235),
            100
        )
    {
        m_aspect_ratio_lower = 0.9;
        m_aspect_ratio_upper = 1.1;
        m_area_ratio_lower = 0.85;
        m_area_ratio_upper = 1.1;
    }

    static const GigantamaxMatcher& instance(){
        static GigantamaxMatcher matcher;
        return matcher;
    }
};


GigantamaxDetector::GigantamaxDetector(
    Color color,
    VideoOverlay* overlay,
    const ImageFloatBox& box
)
    : m_color(color)
    , m_overlay(overlay)
    , m_box(box)
{}
void GigantamaxDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool GigantamaxDetector::detect(const ImageViewRGB32& screen){
    const double screen_rel_size = (screen.height() / 1080.0);
    const double screen_rel_size_2 = screen_rel_size * screen_rel_size;

    const double min_area_1080p = 400;
    const double rmsd_threshold = 80;
    const size_t min_area = size_t(screen_rel_size_2 * min_area_1080p);

    const std::vector<std::pair<uint32_t, uint32_t>> FILTERS = {
        {0xffc42058, 0xffffdceb},
    };

    const bool found = match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_box),
        GigantamaxMatcher::instance(),
        FILTERS,
        {min_area, SIZE_MAX},
        rmsd_threshold,
        [&](Kernels::Waterfill::WaterfillObject& object) -> bool {
            m_last_detected = translate_to_parent(screen, m_box, object);
            return true;
        }
    );

    if (m_overlay){
        if (found){
            m_last_detected_box.emplace(*m_overlay, m_last_detected, COLOR_GREEN);
        }else{
            m_last_detected_box.reset();
        }
    }

    return found;
}

class Test_GigantamaxDetector : public UnitTest{
public:

    Test_GigantamaxDetector(
        const std::string& image,
        bool expected
    )
        : UnitTest("PokemonHome::GigantamaxDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        DummyVideoOverlay video_overlay;
        ImageRGB32 image(m_image);
        GigantamaxDetector detector(COLOR_BLACK, &video_overlay, ImageFloatBox(0.463, 0.09, 0.04, 0.06));
        bool result = detector.detect(image);

        if (result == m_expected)
            return true;

        return "Expected: " + std::to_string(m_expected) + ", received: " + std::to_string(result);
    };

private:
    std::string m_image;
    bool m_expected;
};

void add_tests_GigantamaxDetector(UnitTestDatabase& database){
    database.add<Test_GigantamaxDetector>("PokemonHome/SummaryScreen/annihilape_Regular.png", false);
}


}
}
}
