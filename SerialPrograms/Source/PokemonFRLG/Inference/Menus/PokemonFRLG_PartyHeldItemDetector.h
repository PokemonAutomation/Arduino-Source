/*  Party Held Item Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_PartyHeldItemDetector_H
#define PokemonAutomation_PokemonFRLG_PartyHeldItemDetector_H

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{


enum class PartyHeldItemSlot{
    SLOT_1,
    SLOT_2,
    SLOT_3,
    SLOT_4,
    SLOT_5,
    SLOT_6
};

class PartyHeldItemDetector : public StaticScreenDetector{
public:
    PartyHeldItemDetector(
        Color color,
        VideoOverlay* overlay,
        const ImageFloatBox& box
    );
    PartyHeldItemDetector(
        Color color,
        VideoOverlay* overlay,
        PartyHeldItemSlot slot
    );

    static ImageFloatBox box_for_slot(PartyHeldItemSlot slot);

    const ImageFloatBox& last_detected() const { return m_last_detected; }

    virtual void make_overlays(VideoOverlaySet& items) const override;

    // This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    friend class PartyHeldItemWatcher;

    const Color m_color;
    VideoOverlay* m_overlay;
    const ImageFloatBox m_box;

    ImageFloatBox m_last_detected;
    std::optional<OverlayBoxScope> m_last_detected_box;
};
class PartyHeldItemWatcher : public DetectorToFinder<PartyHeldItemDetector>{
public:
    PartyHeldItemWatcher(
        Color color,
        VideoOverlay* overlay,
        const ImageFloatBox& box,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
        : DetectorToFinder("PartyHeldItemWatcher", hold_duration, color, overlay, box)
    {}
    PartyHeldItemWatcher(
        Color color,
        VideoOverlay* overlay,
        PartyHeldItemSlot slot,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
        : DetectorToFinder("PartyHeldItemWatcher", hold_duration, color, overlay, box_for_slot(slot))
    {}
};


}
}
}
#endif
