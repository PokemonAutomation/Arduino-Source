/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_DialogDetector_H
#define PokemonAutomation_PokemonLA_DialogDetector_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "PokemonLA/Inference/Objects/PokemonLA_ArcPhoneDetector.h"
#include "PokemonLA/Inference/Objects/PokemonLA_DialogueYellowArrowDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{

// Detect surprise dialogue that is used in cases like the bandits stop you.
class DialogSurpriseDetector : public VisualInferenceCallback{
public:
    DialogSurpriseDetector(Logger& logger, VideoOverlay& overlay, bool stop_on_detected);

    bool detected() const{
        return m_detected.load(std::memory_order_acquire);
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    bool m_stop_on_detected;
    std::atomic<bool> m_detected;
    ImageFloatBox m_title_top;
    ImageFloatBox m_title_bottom;
    ImageFloatBox m_top_white;
    ImageFloatBox m_bottom_white;
    ImageFloatBox m_cursor;
    ArcPhoneDetector m_arc_phone;
};

// Detect normal dialogue that is used in cases like when you talk to gate guard Ross when leaving the village.
// Note: it can only detect dialogue boxes with a title. Title appears when you talk to an NPC with known names.
//   The Galaxy member at each camp that gives you access to the ranch and shop has no dialogue title.
class NormalDialogDetector : public VisualInferenceCallback{
public:
    NormalDialogDetector(Logger& logger, VideoOverlay& overlay, bool stop_on_detected);

    bool detected() const{
        return m_detected.load(std::memory_order_acquire);
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    bool m_stop_on_detected;
    std::atomic<bool> m_detected;
    ImageFloatBox m_title_top;
    ImageFloatBox m_title_bottom;
    ImageFloatBox m_title_left;
    ImageFloatBox m_title_right;
    ImageFloatBox m_top_white;
    ImageFloatBox m_bottom_white;
    ImageFloatBox m_left_white;
    ImageFloatBox m_right_white;
};

// Detect event dialogue that is used in cases like when you interact with the tent in a camp.
class EventDialogDetector : public VisualInferenceCallback{
public:
    EventDialogDetector(Logger& logger, VideoOverlay& overlay, bool stop_on_detected);

    bool detected() const{
        return m_detected.load(std::memory_order_acquire);
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    bool m_stop_on_detected;
    std::atomic<bool> m_detected;
    ImageFloatBox m_left_blue;
    ImageFloatBox m_right_blue;
    DialogueYellowArrowDetector m_yellow_arrow_detector;
};

}
}
}
#endif
