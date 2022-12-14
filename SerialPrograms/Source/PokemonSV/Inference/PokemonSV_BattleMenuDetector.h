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
class BattleMenuWatcher : public DetectorToFinder<BattleMenuDetector>{
public:
    BattleMenuWatcher(Color color)
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
class MoveSelectWatcher : public DetectorToFinder<MoveSelectDetector>{
public:
    MoveSelectWatcher(Color color)
         : DetectorToFinder("MoveSelectFinder", std::chrono::milliseconds(250), color)
    {}
};



class TerastallizingDetector : public StaticScreenDetector{
public:
    TerastallizingDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    Color m_color;
    ImageFloatBox m_box;
};



class TeraCatchDetector : public StaticScreenDetector{
public:
    TeraCatchDetector(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    Color m_color;
    ImageFloatBox m_button;
    ImageFloatBox m_box_right;
    GradientArrowDetector m_arrow;
};
class TeraCatchWatcher : public DetectorToFinder<TeraCatchDetector>{
public:
    TeraCatchWatcher(Color color)
         : DetectorToFinder("TeraCatchFinder", std::chrono::milliseconds(250), color)
    {}
};





}
}
}
#endif
