/*  Dialog Triangle Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DialogTriangleDetector_H
#define PokemonAutomation_PokemonSwSh_DialogTriangleDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


// Detect the black triangle arrow in the lower right portion of the white dialog box.
// The dialog box shows up when talking to npcs.
class DialogTriangleDetector : public StaticScreenDetector{
public:
    DialogTriangleDetector(
        Color color,
        ImageFloatBox box = {0.771, 0.901, 0.031, 0.069}
    );

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    Color m_color;
    ImageFloatBox m_box;
};
class DialogTriangleWatcher : public DetectorToFinder<DialogTriangleDetector>{
public:
    DialogTriangleWatcher(Color color = COLOR_RED)
         : DetectorToFinder("DialogTriangleWatcher", std::chrono::milliseconds(250), color)
    {}
};




}
}
}
#endif
