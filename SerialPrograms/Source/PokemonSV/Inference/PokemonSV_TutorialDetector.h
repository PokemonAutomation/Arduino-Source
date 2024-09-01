/*  Tutorial Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_TutorialDetector_H
#define PokemonAutomation_PokemonSV_TutorialDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"

namespace PokemonAutomation{
    class ConsoleHandle;
    class BotBaseContext;
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{


class TutorialDetector : public StaticScreenDetector{
public:
    TutorialDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;


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
