/*  Tutorial Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_TutorialDetector_H
#define PokemonAutomation_PokemonSV_TutorialDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/InferenceCallbacks/VisualInferenceCallback.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{


class TutorialDetector : public StaticScreenDetector{
public:
    TutorialDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;


protected:
    Color m_color;
    ImageFloatBox m_bottom;
    ImageFloatBox m_left;
};
class TutorialWatcher : public DetectorToFinder<TutorialDetector>{
public:
    TutorialWatcher(Color color = COLOR_RED)
         : DetectorToFinder("TutorialWatcher", std::chrono::milliseconds(250), color)
    {}
};






}
}
}
#endif
