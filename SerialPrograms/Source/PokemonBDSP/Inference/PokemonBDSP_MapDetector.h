/*  Map Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_MapDetector_H
#define PokemonAutomation_PokemonBDSP_MapDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class MapDetector : public StaticScreenDetector{
public:
    MapDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_box0;
    ImageFloatBox m_box1;
    ImageFloatBox m_box2;
};


class MapWatcher : public MapDetector, public VisualInferenceCallback{
public:
    MapWatcher(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;
};




}
}
}
#endif
