/*  Main Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_MainMenuDetector_H
#define PokemonAutomation_PokemonLZA_MainMenuDetector_H

//#include <optional>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

// Detect game main menu, which has menu entry "Boxes", "Satchel", "Pokedex", "Mable's Research",
// "Z-A Royale", "Link Play" on the left and the current party pokemon status on the right.
class MainMenuDetector : public StaticScreenDetector{
public:
    MainMenuDetector(Color color = COLOR_RED, VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ButtonDetector m_right_arrow_button;
    ButtonDetector m_b_button;
};

class MainMenuWatcher : public DetectorToFinder<MainMenuDetector>{
public:
    MainMenuWatcher(
        Color color = COLOR_RED,
        VideoOverlay* overlay = nullptr,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("MainMenuWatcher", hold_duration, color, overlay)
    {}
};




}
}
}
#endif
