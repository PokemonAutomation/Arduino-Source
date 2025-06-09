/*  Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_MenuDetector_H
#define PokemonAutomation_PokemonBDSP_MenuDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class MenuDetector : public StaticScreenDetector{
public:
    MenuDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_line0;
    ImageFloatBox m_line1;
    ImageFloatBox m_line2;
    ImageFloatBox m_line3;
    ImageFloatBox m_line4;
    ImageFloatBox m_cross;
};


class MenuWatcher : public MenuDetector, public VisualInferenceCallback{
public:
    MenuWatcher(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;
};




}
}
}
#endif
