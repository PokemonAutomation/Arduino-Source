/*  Overworld Party Selection Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_OverworldPartySelectionDetector_H
#define PokemonAutomation_PokemonLZA_OverworldPartySelectionDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "PokemonLZA/Inference/PokemonLZA_ButtonDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

// Detect the dpad up and down button shown on the party list when in the overworld.
// This can determine whether we are at the overworld, which pokemon we will send
// into next battle first, and whether the current selected pokemon is out of pokeball.
// we have three modes when party is visible in the overworld:
// - Visible dpad up. No dpad down: No pokemon sent out. All party pokemon in balls.
// - No dpad up. Visible dpad down: Current selected pokemon is sent out.
// - Visible dpad up and dpad down: Pokemon pointed by dpad down is sent out while current selected pokemon,
//   (selected by dpad up) can be sent out next.
class OverworldPartySelectionDetector : public StaticScreenDetector{
public:
    OverworldPartySelectionDetector(Color color = COLOR_RED, VideoOverlay* overlay = nullptr);

    const uint8_t INVALID_PARTY_IDX = 6;

    virtual void make_overlays(VideoOverlaySet& items) const override;

    //  This is not const so that detectors can save/cache state.
    virtual bool detect(const ImageViewRGB32& screen) override;

    virtual void reset_state() override {
        m_detected_up_idx = INVALID_PARTY_IDX;
        m_detected_down_idx = INVALID_PARTY_IDX;
    }

    // Return detected the pokemon idx in the party where the current selected pokemon is.
    // Return INVALID_PARTY_IDX if no overworld party selection screen detected.
    uint8_t selected_party_idx() const;
    // Return detected dpad up button idx in the party.
    // Return INVALID_PARTY_IDX if no overworld party selection screen detected.
    uint8_t dpad_up_idx() const { return m_detected_up_idx; }
    // Return detected dpad down button idx in the party.
    // Return INVALID_PARTY_IDX if no overworld party selection screen detected.
    uint8_t dpad_down_idx() const { return m_detected_down_idx; }


    // Under debug mode, will throw FatalProgramException when more than one box cell
    // detects a cursor. This is used for debugging.
    void set_debug_mode(bool debug_mode) { m_debug_mode = debug_mode; }

private:
    // 6 regions, one for each party pokemon, to detect dpad buttons
    std::array<ButtonDetector, 6> m_dpad_ups;
    std::array<ButtonDetector, 6> m_dpad_downs;

    uint8_t m_detected_up_idx = INVALID_PARTY_IDX;
    uint8_t m_detected_down_idx = INVALID_PARTY_IDX;

    bool m_debug_mode = false;
};

class OverworldPartySelectionWatcher : public DetectorToFinder<OverworldPartySelectionDetector>{
public:
    OverworldPartySelectionWatcher(
        Color color = COLOR_RED,
        VideoOverlay* overlay = nullptr,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(150)
    )
         : DetectorToFinder("OverworldPartySelectionWatcher", hold_duration, color, overlay)
    {}
};




}
}
}
#endif
