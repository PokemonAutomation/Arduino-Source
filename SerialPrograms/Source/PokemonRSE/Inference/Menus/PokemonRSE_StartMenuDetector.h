/*  Start Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonRSE_StartMenuDetector_H
#define PokemonAutomation_PokemonRSE_StartMenuDetector_H

#include <chrono>
#include "Common/Cpp/Color.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
    class CancellableScope;
    class VideoFeed;
namespace NintendoSwitch{
namespace PokemonRSE{

// Detect the full start menu by looking for empty white sections on the top and bottom
// Works for all RSE, all languages. Will not work for early game (before pokedex or party).
// No arrow detection, RS japan and all emerald languages have an arrow,
// but RS for all non-japan languages use a red box for selection.
class StartMenuDetector : public StaticScreenDetector{
public:
    StartMenuDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_menu_top_box;
    ImageFloatBox m_menu_bottom_box;
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
