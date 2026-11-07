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
    // Empty boxes can have a uniform first row. Opt in only when that is expected;
    // the fixed box-screen color anchors are still required.
    BoxDetector(Color color = COLOR_RED, bool allow_empty = false);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    bool m_allow_empty;
    ImageFloatBox m_left;
    ImageFloatBox m_right;
    ImageFloatBox m_bottom;
    ImageFloatBox m_row;
    ImageFloatBox m_title;
};


class BoxWatcher : public BoxDetector, public VisualInferenceCallback{
public:
    BoxWatcher(Color color = COLOR_RED, bool allow_empty = false);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;
};



}
}
}
#endif
