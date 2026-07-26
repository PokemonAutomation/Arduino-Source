/*  Battle Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonTools/Images/SolidColorTest.h"
#include "Tests/TestUtils.h"
#include "PokemonLA_BattleMenuDetector.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


BattleMenuDetector::BattleMenuDetector(Logger& logger, VideoOverlay& overlay, bool stop_on_detected)
    : VisualInferenceCallback("BattleMenuDetector")
    , m_stop_on_detected(stop_on_detected)
    , m_detected(false)
    , m_pokemon_stroke_bg_left  (0.056, 0.948, 0.013, 0.020)
    , m_pokemon_stroke_bg_right (0.174, 0.948, 0.032, 0.046)
    , m_button_A_detector(logger, overlay, ButtonType::ButtonA, ImageFloatBox{0.764, 0.752, 0.034, 0.054}, std::chrono::milliseconds(0), false)
    // , m_button_B_detector(logger, overlay, ButtonType::ButtonB, ImageFloatBox{0.720, 0.828, 0.027, 0.048}, std::chrono::milliseconds(0), false)
{}

void BattleMenuDetector::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_pokemon_stroke_bg_left);
    items.add(COLOR_RED, m_pokemon_stroke_bg_right);
}


bool BattleMenuDetector::process_frame(const ImageViewRGB32& frame, WallClock timestamp){
    const ImageStats stroke_left = image_stats(extract_box_reference(frame, m_pokemon_stroke_bg_left));
//    cout << stroke_left.average << stroke_left.stddev << endl;
    if (is_solid(stroke_left,{0.179,0.386,0.435}, 0.2, 15) == false){
        m_detected.store(false, std::memory_order_release);
        return false;
    }
    

    const ImageStats stroke_right = image_stats(extract_box_reference(frame, m_pokemon_stroke_bg_right));
    if (is_solid(stroke_right, {0.228,0.358,0.414}, 0.2, 15) == false){
        m_detected.store(false, std::memory_order_release);
        return false;
    }

    m_button_A_detector.process_frame(frame, timestamp);

    // m_button_B_detector.process_frame(frame, timestamp);
    // hits += m_button_B_detector.detected();

    bool detected = m_button_A_detector.detected();
    m_detected.store(detected, std::memory_order_release);

    return detected && m_stop_on_detected;
}






class Test_BattleMenuDetector : public UnitTest{
public:
    Test_BattleMenuDetector(
        const std::string& image,
        bool expected
    )
        : UnitTest("PokemonLA::BattleMenuDetector - " + image)
        , m_image(UNIT_TEST_RESOURCE_PATH() + image)
        , m_expected(expected)
    {}

    virtual UnitTestResult run(Logger& logger, CancellableScope& scope) const override{
        DummyVideoOverlay overlay;
        BattleMenuDetector detector(logger, overlay, true);
        ImageRGB32 image(m_image);
        return detector.process_frame(image, current_time()) == m_expected;
    };

private:
    std::string m_image;
    bool m_expected;
};


void add_tests_BattleMenuDetector(UnitTestDatabase& database){
    database.add<Test_BattleMenuDetector>("PokemonLA/BattleMenuDetector/CoastlandsMidday_True.png", true);
    database.add<Test_BattleMenuDetector>("PokemonLA/BattleMenuDetector/CoastlandsSeaMidday_True.jpg", true);
}













}
}
}
