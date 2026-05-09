/*  Dialog Box Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DialogBoxDetector_H
#define PokemonAutomation_PokemonSwSh_DialogBoxDetector_H

#include <atomic>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "PokemonSwSh_DialogTriangleDetector.h"

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


class WhiteDialogBoxDetector : public StaticScreenDetector{
public:
    WhiteDialogBoxDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_right;
    DialogTriangleDetector m_triangle;
};
class WhiteDialogBoxWatcher : public DetectorToFinder<WhiteDialogBoxDetector>{
public:
    WhiteDialogBoxWatcher(Color color = COLOR_RED)
         : DetectorToFinder("WhiteDialogBoxWatcher", std::chrono::milliseconds(250), color)
    {}
};





}
}
}
#endif
