/*  Party Empty Slot Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_PartyEmptySlotDetector_H
#define PokemonAutomation_PokemonFRLG_PartyEmptySlotDetector_H

#include <chrono>
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "PokemonFRLG_PartySlot.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class PartyEmptySlotDetector : public StaticScreenDetector{
public:
    PartyEmptySlotDetector(
        Color color,
        VideoOverlay* overlay,
        PartySlot slot
    );
    static ImageFloatBox box_for_slot(PartySlot slot);
    virtual void make_overlays(VideoOverlaySet& items) const override;
    // This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    VideoOverlay* m_overlay;
    ImageFloatBox m_box;
};
class PartyEmptySlotWatcher : public DetectorToFinder<PartyEmptySlotDetector>{
public:
    PartyEmptySlotWatcher(
        Color color,
        VideoOverlay* overlay,
        PartySlot slot,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
        : DetectorToFinder("PartyEmptySlotWatcher", hold_duration, color, overlay, slot)
    {}
};

}
}
}
#endif