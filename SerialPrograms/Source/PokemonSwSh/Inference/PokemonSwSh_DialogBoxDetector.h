/*  Dialog Box Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DialogBoxDetector_H
#define PokemonAutomation_PokemonSwSh_DialogBoxDetector_H

#include <atomic>
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


// Detect the black dialog box. It is used in places like releasing pokemon in box storage.
class BlackDialogBoxDetector : public VisualInferenceCallback{
public:
    BlackDialogBoxDetector(bool stop_on_detected);

    bool detected() const{
        return m_detected.load(std::memory_order_acquire);
    }

    virtual void make_overlays(VideoOverlaySet& items) const override;

    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    bool m_stop_on_detected;

    std::atomic<bool> m_detected;
};





}
}
}
#endif
