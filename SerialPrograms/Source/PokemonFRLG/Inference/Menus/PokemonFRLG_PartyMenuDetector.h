/*  Party Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_PartyMenuDetector_H
#define PokemonAutomation_PokemonFRLG_PartyMenuDetector_H

#include <chrono>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "PokemonFRLG/PokemonFRLG_Settings.h"

namespace PokemonAutomation{
    class CancellableScope;
    class VideoFeed;
namespace NintendoSwitch{
namespace PokemonFRLG{

enum class PartySlot{
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX
    //CXL
};

// The Party menu has a white box on the bottom
// The background around the edges is dark teal/navy
class PartyMenuDetector : public StaticScreenDetector{
public:
    PartyMenuDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_dialog_top_box;
    ImageFloatBox m_page_background_box;
};
class PartyMenuWatcher : public DetectorToFinder<PartyMenuDetector>{
public:
    PartyMenuWatcher(Color color)
        : DetectorToFinder("PartyMenuWatcher", std::chrono::milliseconds(250), color)
    {}
};

// The Party menu has a white box in the bottom right corner when a Pokemon is selected
// The background around the edges is dark teal/navy
class PartySelectionDetector : public StaticScreenDetector{
public:
    PartySelectionDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_dialog_right_box;
    ImageFloatBox m_page_background_box;
};
class PartySelectionWatcher : public DetectorToFinder<PartySelectionDetector>{
public:
    PartySelectionWatcher(Color color)
        : DetectorToFinder("PartySelectionWatcher", std::chrono::milliseconds(250), color)
    {}
};


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
