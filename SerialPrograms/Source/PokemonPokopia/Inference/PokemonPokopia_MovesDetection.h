/*  Moves Detection
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonPokopia_MovesDetection_H
#define PokemonAutomation_PokemonPokopia_MovesDetection_H

#include <optional>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "PokemonPokopia/Inference/PokemonPokopia_ButtonDetector.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonPokopia{

const ImageFloatBox MOVES_LEFT_ARROW_BOX{0.806500, 0.935000, 0.025000, 0.045000};
const ImageFloatBox MOVES_RIGHT_ARROW_BOX{0.960600, 0.935000, 0.025000, 0.045000};


class OverworldDetector : public StaticScreenDetector{
public:
    OverworldDetector(
        Color color,
        VideoOverlay* overlay = nullptr
    );
    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    virtual void reset_state() override { m_last_detected_box.reset(); }

private:
    Color m_color;
    VideoOverlay* m_overlay;

    ImageFloatBox m_last_detected;
    std::optional<OverlayBoxScope> m_last_detected_box;
};
class OverworldWatcher : public DetectorToFinder<OverworldDetector>{
public:
    OverworldWatcher(
        Color color,
        VideoOverlay* overlay = nullptr,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("OverworldWatcher", hold_duration, color, overlay)
    {}
};
class OverworldGoneWatcher : public DetectorToFinder<OverworldDetector>{
public:
    OverworldGoneWatcher(
        Color color,
        VideoOverlay* overlay = nullptr,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
         : DetectorToFinder("OverworldGoneWatcher", FinderType::GONE, hold_duration, color, overlay)
    {}
};



}
}
}
#endif
