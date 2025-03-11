/*  Receive Pokemon (Blue Background) Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *
 *      Returns true after the background of receiving a pokemon
 *  has been detected and has ended.
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_ReceivePokemonDetector_H
#define PokemonAutomation_PokemonBDSP_ReceivePokemonDetector_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

// Detect the end of receiving a pokemon or an egg.
class ReceivePokemonDetector : public VisualInferenceCallback{
public:
    ReceivePokemonDetector(Color color = COLOR_RED);

    // Whether a receive event happened or is happening.
    bool received() const { return m_received; }

    virtual void make_overlays(VideoOverlaySet& items) const override;

    // Return true only when a receiving event happened and ended.
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    bool m_received = false;
    Color m_color;
    ImageFloatBox m_box0;
    ImageFloatBox m_box1;
};



}
}
}
#endif
