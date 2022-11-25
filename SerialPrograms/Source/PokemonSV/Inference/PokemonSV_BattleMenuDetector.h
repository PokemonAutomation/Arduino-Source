/*  Battle Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_BattleMenuDetector_H
#define PokemonAutomation_PokemonSV_BattleMenuDetector_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV_GradientArrowDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class BattleMenuDetector : public StaticScreenDetector{
public:
    BattleMenuDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    WhiteButtonDetector m_status_button;
    GradientArrowDetector m_arrow;
};
class BattleMenuFinder : public DetectorToFinder<BattleMenuDetector>{
public:
    BattleMenuFinder(Color color)
         : DetectorToFinder("BattleMenuFinder", std::chrono::milliseconds(250), color)
    {}
};



class MoveSelectDetector : public StaticScreenDetector{
public:
    MoveSelectDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    WhiteButtonDetector m_status_button;
    GradientArrowDetector m_arrow;
};
class MoveSelectFinder : public DetectorToFinder<MoveSelectDetector>{
public:
    MoveSelectFinder(Color color)
         : DetectorToFinder("MoveSelectFinder", std::chrono::milliseconds(250), color)
    {}
};



class TeraCatchDetector : public StaticScreenDetector{
public:
    TeraCatchDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    Color m_color;
    ImageFloatBox m_box;
    GradientArrowDetector m_arrow;
};
class TeraCatchFinder : public DetectorToFinder<TeraCatchDetector>{
public:
    TeraCatchFinder(Color color)
         : DetectorToFinder("TeraCatchFinder", std::chrono::milliseconds(250), color)
    {}
};





}
}
}
#endif
