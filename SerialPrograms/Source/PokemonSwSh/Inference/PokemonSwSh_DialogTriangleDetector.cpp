/*  Dialog Triangle Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stdint.h>
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Logging/AbstractLogger.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/WaterfillUtilities.h"
#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "PokemonSwSh_DialogTriangleDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{




class DialogTriangleMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    DialogTriangleMatcher();
    static const DialogTriangleMatcher& instance();
};


DialogTriangleMatcher::DialogTriangleMatcher()
    : WaterfillTemplateMatcher(
        "PokemonSwSh/DialogBlackTriangle.png",
        Color(0,0,0), Color(30, 30, 30), 50
    )
{
    m_aspect_ratio_lower = 0.9;
    m_aspect_ratio_upper = 1.1;
    m_area_ratio_lower = 0.9;
    m_area_ratio_upper = 1.1;
}

const DialogTriangleMatcher& DialogTriangleMatcher::instance(){
    static DialogTriangleMatcher matcher;
    return matcher;
}




DialogTriangleDetector::DialogTriangleDetector(
    Color color,
    ImageFloatBox box
)
    : m_color(color)
    , m_box(box)
{}

void DialogTriangleDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_box);
}
bool DialogTriangleDetector::detect(const ImageViewRGB32& screen){
    const std::vector<std::pair<uint32_t, uint32_t>> filters = {
        {combine_rgb(0, 0, 0), combine_rgb(50, 50, 50)}
    };

    const double screen_rel_size = (screen.height() / 1080.0);
    const size_t min_size = size_t(screen_rel_size * screen_rel_size * 500.0);

    return match_template_by_waterfill(
        screen.size(),
        extract_box_reference(screen, m_box),
        DialogTriangleMatcher::instance(),
        filters,
        {min_size, SIZE_MAX},
        80,
        [](Kernels::Waterfill::WaterfillObject& object) -> bool { return true; }
    );
}








class Test_DialogTriangleDetector : public UnitTest{
public:
    Test_DialogTriangleDetector(
        const std::string& image,
        bool expected
    )
        : UnitTest("PokemonSwSh::DialogTriangleDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        DialogTriangleDetector detector(COLOR_RED);
        ImageRGB32 image(m_image);
        return detector.detect(image) == m_expected;
    };

private:
    std::string m_image;
    bool m_expected;
};


void add_tests_DialogTriangleDetector(UnitTestDatabase& database){

}







}
}
}

