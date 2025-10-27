/*  Box Shiny Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_BoxShinyDetector_H
#define PokemonAutomation_PokemonLZA_BoxShinyDetector_H

#include <optional>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

// Detect shiny sparkles when viewing a Pokemon in the box system
class BoxShinyDetector : public StaticScreenDetector{
public:
    BoxShinyDetector(Color color = COLOR_RED, VideoOverlay* overlay = nullptr);

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    friend class BoxShinyWatcher;

    Color m_color;
    ImageFloatBox m_box;
    VideoOverlay* m_overlay;

    ImageFloatBox m_last_detected;
    std::optional<OverlayBoxScope> m_last_detected_box;
};

class BoxShinyWatcher : public DetectorToFinder<BoxShinyDetector>{
public:
    BoxShinyWatcher(
        Color color = COLOR_RED,
        VideoOverlay* overlay = nullptr,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("BoxShinyWatcher", hold_duration, color, overlay)
    {}
};




}
}
}
#endif
