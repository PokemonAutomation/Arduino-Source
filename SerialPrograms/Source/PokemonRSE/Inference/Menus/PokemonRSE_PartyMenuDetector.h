/*  Party Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonRSE_PartyMenuDetector_H
#define PokemonAutomation_PokemonRSE_PartyMenuDetector_H

#include <chrono>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "PokemonRSE/PokemonRSE_Settings.h"
//#include "PokemonFRLG_PartySlot.h"

namespace PokemonAutomation{
    class CancellableScope;
    class VideoFeed;
namespace NintendoSwitch{
namespace PokemonRSE{


enum class PartySlot{
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX
    //CXL
};


// Detect the Party Menu (Start Menu -> Pokemon).
class PartyMenuDetector : public StaticScreenDetector{
public:
    PartyMenuDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_dialog_box;
    ImageFloatBox m_background_box_e;
    ImageFloatBox m_top_bar_rs; //red, dark red, black bar on top
};
class PartyMenuWatcher : public DetectorToFinder<PartyMenuDetector>{
public:
    PartyMenuWatcher(Color color)
        : DetectorToFinder("PartyMenuWatcher", std::chrono::milliseconds(250), color)
    {}
};


//Detect the highlighted slot.
//Looks for the red outline
class PartySlotDetector : public StaticScreenDetector{
public:
    PartySlotDetector(
        Color color,
        const ImageFloatBox& box
    );

    PartySlotDetector(
        Color color,
        PartySlot position
    );

    static ImageFloatBox party_slot_boxes(PartySlot position);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    const Color m_color;
    const ImageFloatBox m_party_box;
};
class PartySlotWatcher : public DetectorToFinder<PartySlotDetector>{
public:
    PartySlotWatcher(
        Color color,
        PartySlot position,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
        : DetectorToFinder("PartySlotWatcher", hold_duration, color, party_slot_boxes(position))
    {
    }
};


}
}
}

#endif
