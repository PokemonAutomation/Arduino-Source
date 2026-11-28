/*  Tutorial Window Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_TutorialWindowDetector_H
#define PokemonAutomation_PokemonLZA_TutorialWindowDetector_H

#include <chrono>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

// Match the question-book badge and white frame shared by tutorial windows
// Includes single-page windows and windows with navigation arrows
class TutorialWindowDetector : public StaticScreenDetector{
public:
    TutorialWindowDetector(Color color = COLOR_CYAN);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_book;
    ImageFloatBox m_top_border;
    ImageFloatBox m_left_border;
};

class TutorialWindowWatcher : public DetectorToFinder<TutorialWindowDetector>{
public:
    TutorialWindowWatcher(
        Color color = COLOR_CYAN,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(100)
    )
        : DetectorToFinder("TutorialWindowWatcher", hold_duration, color)
    {}
};

}
}
}
#endif
