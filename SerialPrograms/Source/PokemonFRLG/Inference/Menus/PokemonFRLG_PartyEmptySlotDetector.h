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
        PartySlot slot
    );
    static ImageFloatBox box_for_slot(PartySlot slot);
    virtual void make_overlays(VideoOverlaySet& items) const override;
    // This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_box;
};

}
}
}
#endif