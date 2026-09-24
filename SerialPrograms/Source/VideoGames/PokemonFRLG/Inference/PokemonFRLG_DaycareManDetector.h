/*  Daycare Man Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_DaycareManDetector_H
#define PokemonAutomation_PokemonFRLG_DaycareManDetector_H

#include "Common/Cpp/Logging/AbstractLogger.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

// Detects the grass in front of where the Day Care Man usually stands.
// If an egg is ready, he will occupy the space. Otherwise, it will be solid green (empty grass).
// Also detects other parts of the scene to avoid triggering on a black screen, etc
class DaycareManDetector : public StaticScreenDetector{
public:
    DaycareManDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const;
    bool detect(const ImageViewRGB32& screen);

private:
    ImageFloatBox m_box_man;
    ImageFloatBox m_box_grass;
    ImageFloatBox m_box_puddle;
    ImageFloatBox m_box_sand;
    ImageFloatBox m_box_roof;
};
class DaycareManWatcher : public DetectorToFinder<DaycareManDetector>{
public:
    DaycareManWatcher(Color color)
        : DetectorToFinder("DaycareManWatcher", std::chrono::milliseconds(250), color)
    {}
};


}
}
}
#endif
