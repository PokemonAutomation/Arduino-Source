/*  Box Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_BoxDetector_H
#define PokemonAutomation_PokemonBDSP_BoxDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class BoxDetector : public StaticScreenDetector{
public:
    BoxDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_left;
    ImageFloatBox m_right;
    ImageFloatBox m_bottom;
    ImageFloatBox m_row;
};


class BoxWatcher : public BoxDetector, public VisualInferenceCallback{
public:
    BoxWatcher(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;
};



}
}
}
#endif
