/*  Picnic Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Tests/TestImageDistortion.h"
#include "PokemonSV_PicnicDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


PicnicDetector::PicnicDetector(Color color)
    : m_color(color)
    , m_button_b(color, WhiteButton::ButtonB, {0.024, 0.702, 0.024, 0.045})
    , m_button_minus(color, WhiteButton::ButtonMinus, {0.024, 0.866, 0.024, 0.045})
    , m_button_y(color, WhiteButton::ButtonY, {0.024, 0.917, 0.024, 0.045})
{}

void PicnicDetector::make_overlays(VideoOverlaySet& items) const{
    m_button_b.make_overlays(items);
    m_button_minus.make_overlays(items);
    m_button_y.make_overlays(items);
}

bool PicnicDetector::detect(const ImageViewRGB32& frame){
    return m_button_b.detect(frame)
        && m_button_minus.detect(frame)
        && m_button_y.detect(frame);
}






class Test_PicnicDetector : public UnitTest{
public:
    Test_PicnicDetector(
        const std::string& image,
        bool expected
    )
        : UnitTest("PokemonSV::PicnicDetector - " + image)
        , m_image(image)
        , m_expected(expected)
    {}

    virtual std::vector<std::string> image_paths() const override{
        return {m_image};
    }

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        PicnicDetector detector;
        ImageRGB32 image(UNIT_TEST_RESOURCE_PATH() + m_image);
        apply_test_image_distortion(image, m_image, distortion_label());
        return detector.detect(image) == m_expected;
    };

private:
    std::string m_image;
    bool m_expected;
};




void add_tests_PicnicDetector(UnitTestDatabase& database){
    database.add<Test_PicnicDetector>("PokemonSV/PicnicDetector/macOS/Picinic_1_True.png", true);
    database.add<Test_PicnicDetector>("PokemonSV/PicnicDetector/macOS/Picnic_0_True.png", true);
}





}
}
}
