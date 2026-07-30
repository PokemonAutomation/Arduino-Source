/*  Summary Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_SummaryScreenDetector_H
#define PokemonAutomation_PokemonHome_SummaryScreenDetector_H

#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "PokemonHome/Inference/PokemonHome_ButtonDetector.h"

namespace PokemonAutomation{
class Logger;
namespace NintendoSwitch{
namespace PokemonHome{

class SummaryScreenDetector : public StaticScreenDetector{
public:
    SummaryScreenDetector(VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    virtual void reset_state() override { m_button_B_detector.reset_state(); }

private:
    ButtonDetector m_button_B_detector;

};
class SummaryScreenWatcher : public DetectorToFinder<SummaryScreenDetector>{
public:
    SummaryScreenWatcher(VideoOverlay* overlay = nullptr)
        : DetectorToFinder("SummaryScreenWatcher", Milliseconds(100), overlay)
    {}
};

void add_tests_SummaryScreenDetector(UnitTestDatabase& database);

}
}
}
#endif