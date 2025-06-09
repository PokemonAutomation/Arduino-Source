/*  Clothing Top Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_ClothingTop_H
#define PokemonAutomation_PokemonSV_ClothingTop_H

#include "CommonTools/VisualDetector.h"
#include "PokemonSV/Inference/Dialogs/PokemonSV_GradientArrowDetector.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{

class ClothingTopDetector : public StaticScreenDetector{
public:
    ClothingTopDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    GradientArrowDetector m_arrow;
};
class ClothingTopWatcher : public DetectorToFinder<ClothingTopDetector>{
public:
    ClothingTopWatcher(Color color)
        : DetectorToFinder("ClothingTopWatcher", std::chrono::milliseconds(250), color)
    {}
};





}
}
}
#endif
