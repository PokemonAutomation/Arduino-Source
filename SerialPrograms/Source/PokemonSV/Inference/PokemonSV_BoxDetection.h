/*  Box Detection
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_BoxDetection_H
#define PokemonAutomation_PokemonSV_BoxDetection_H

#include "PokemonSV_DialogDetector.h"
#include "PokemonSV_GradientArrowDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


//  Detect whether the cursor if over a Pokemon in the box.
class SomethingInBoxSlotDetector : public StaticScreenDetector{
public:
    SomethingInBoxSlotDetector(bool true_if_exists, Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

protected:
    bool m_true_if_exists;
    Color m_color;
    ImageFloatBox m_exists;
};
class SomethingInBoxSlotFinder : public DetectorToFinder_ConsecutiveDebounce<SomethingInBoxSlotDetector>{
public:
    SomethingInBoxSlotFinder(bool stop_on_exists, Color color = COLOR_RED)
         : DetectorToFinder_ConsecutiveDebounce("SomethingInBoxSlot", 5, stop_on_exists, color)
    {}
};



//  Detect whether you have a Pokemon selected in the box system.
class BoxSelectDetector : public StaticScreenDetector{
public:
    BoxSelectDetector(Color color = COLOR_RED);

    bool exists(const ImageViewRGB32& screen) const;

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

protected:
    Color m_color;
    SomethingInBoxSlotDetector m_exists;
    AdvanceDialogDetector m_dialog;
    GradientArrowDetector m_gradient;
};
class BoxSelectFinder : public DetectorToFinder_ConsecutiveDebounce<BoxSelectDetector>{
public:
    BoxSelectFinder(Color color = COLOR_RED)
         : DetectorToFinder_ConsecutiveDebounce("BoxSelectFinder", 5, color)
    {}
};








}
}
}
#endif
