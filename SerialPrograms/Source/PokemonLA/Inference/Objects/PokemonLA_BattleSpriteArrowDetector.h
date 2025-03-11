/*  Battle Sprite Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Detect the white arrow pointing towards left on the selected sprite, in the upper right corner
 *  of the screen, during a multi-pokemon battle.
 */

#ifndef PokemonAutomation_PokemonLA_BattleSpriteArrowDetector_H
#define PokemonAutomation_PokemonLA_BattleSpriteArrowDetector_H

#include "CommonTools/ImageMatch/WaterfillTemplateMatcher.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/DetectionDebouncer.h"
#include "PokemonLA/Inference/Objects/PokemonLA_WhiteObjectDetector.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonLA{


class BattleSpriteArrowMatcher : public ImageMatch::WaterfillTemplateMatcher{
public:
    BattleSpriteArrowMatcher();
    static const BattleSpriteArrowMatcher& instance();
};



class BattleSpriteArrowTracker : public WhiteObjectDetector{
public:
    BattleSpriteArrowTracker();

    virtual void process_object(const ImageViewRGB32& image, const WaterfillObject& object) override;
    virtual void finish(const ImageViewRGB32& image) override;
};



class BattleSpriteArrowDetector : public VisualInferenceCallback{
public:
    // Sprite index: the index of the sprite the current selected arrow is on top of.
    //   The index is ordered from right to left on the screen.
    BattleSpriteArrowDetector(
        Logger& logger, VideoOverlay& overlay,
        size_t sprite_index,
        std::chrono::milliseconds min_streak,
        bool stop_on_detected
    );

    bool detected() const{
        return m_debouncer.get();
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    Logger& m_logger;
    ImageFloatBox m_box;
    bool m_stop_on_detected;

    BattleSpriteArrowTracker m_tracker;
    WhiteObjectWatcher m_watcher;

    DetectionDebouncer<bool> m_debouncer;
};



}
}
}
#endif
