/*  Transparent Dialogue Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/Globals.h"
#include "PokemonLA_TransparentDialogueDetector.h"
#include "Tests/TestUtils.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


TransparentDialogueDetector::TransparentDialogueDetector(
    Logger& logger, VideoOverlay& overlay,
    bool stop_on_detected
)
    : VisualInferenceCallback("TransparentDialogueDetector")
    , m_arrow_detector(logger, overlay, stop_on_detected)
    , m_ellipse_detector(logger, overlay, std::chrono::milliseconds(0), stop_on_detected)
{}


void TransparentDialogueDetector::make_overlays(VideoOverlaySet& items) const{
    m_arrow_detector.make_overlays(items);
    m_ellipse_detector.make_overlays(items);
}


bool TransparentDialogueDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    bool stop = m_arrow_detector.process_frame(frame, timestamp);
    bool detected = m_arrow_detector.detected();
    if (detected){
        stop = stop && m_ellipse_detector.process_frame(frame, timestamp);
        detected = m_ellipse_detector.detected();
    }

    m_detected.store(detected, std::memory_order_release);

    return stop;
}






class Test_TransparentDialogueDetector : public UnitTest{
public:
    Test_TransparentDialogueDetector(
        const std::string& image,
        bool expected
    )
        : UnitTest("PokemonLA::TransparentDialogueDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        DummyVideoOverlay overlay;
        TransparentDialogueDetector detector(logger, overlay, true);
        ImageRGB32 image(m_image);
        return detector.process_frame(image, current_time()) == m_expected;
    };

private:
    std::string m_image;
    bool m_expected;
};


void add_tests_TransparentDialogueDetector(UnitTestDatabase& database){
    database.add<Test_TransparentDialogueDetector>("PokemonLA/TransparentDialogueDetector/macOS_bright/Avalugg_1_False.png", false);
    database.add<Test_TransparentDialogueDetector>("PokemonLA/TransparentDialogueDetector/macOS_bright/Avalugg_2_False.png", false);
    database.add<Test_TransparentDialogueDetector>("PokemonLA/TransparentDialogueDetector/macOS_bright/BrenDay_True.png", true);
    database.add<Test_TransparentDialogueDetector>("PokemonLA/TransparentDialogueDetector/macOS_bright/BrenEvening_1_True.png", true);
    database.add<Test_TransparentDialogueDetector>("PokemonLA/TransparentDialogueDetector/macOS_bright/BrenEvening_2_True.png", true);
    database.add<Test_TransparentDialogueDetector>("PokemonLA/TransparentDialogueDetector/macOS_bright/BrenMorning_True.png", true);
    database.add<Test_TransparentDialogueDetector>("PokemonLA/TransparentDialogueDetector/macOS_bright/BrenNight_True.png", true);
    database.add<Test_TransparentDialogueDetector>("PokemonLA/TransparentDialogueDetector/macOS_bright/FieldlandsCoinDay_True.png", true);
    database.add<Test_TransparentDialogueDetector>("PokemonLA/TransparentDialogueDetector/macOS_bright/MirelandsCoinEvening_True.png", true);
    database.add<Test_TransparentDialogueDetector>("PokemonLA/TransparentDialogueDetector/macOS_bright/WendyNight_True.png", true);

}


}
}
}
