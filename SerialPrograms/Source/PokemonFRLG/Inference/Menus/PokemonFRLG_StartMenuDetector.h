/*  Start Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_StartMenuDetector_H
#define PokemonAutomation_PokemonFRLG_StartMenuDetector_H

#include <chrono>
#include "Common/Cpp/Color.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "PokemonFRLG/Inference/PokemonFRLG_SelectionArrowDetector.h"

namespace PokemonAutomation{
    class CancellableScope;
    class VideoFeed;
namespace NintendoSwitch{
namespace PokemonFRLG{

// Detect Start menu by looking for the selection arrow
class StartMenuDetector : public StaticScreenDetector{
public:
    StartMenuDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    SelectionArrowDetector m_selection_arrow;
};
class StartMenuWatcher : public DetectorToFinder<StartMenuDetector>{
public:
    StartMenuWatcher(Color color = COLOR_RED)
        : DetectorToFinder("StartMenuWatcher", std::chrono::milliseconds(250), color)
    {}
};


}
}
}

#endif
