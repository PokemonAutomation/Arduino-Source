/*  Battle Menu Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_BattleMenuDetector_H
#define PokemonAutomation_PokemonSV_BattleMenuDetector_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "PokemonSV_WhiteButtonDetector.h"
#include "PokemonSV_GradientArrowDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class BattleMenuDetector : public StaticScreenDetector{
public:
    BattleMenuDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    WhiteButtonDetector m_status_button;
    GradientArrowDetector m_arrow;
};
class BattleMenuFinder : public VisualInferenceCallback{
public:
    BattleMenuFinder(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    BattleMenuDetector m_detector;
    size_t m_trigger_count = 0;
};



class MoveSelectDetector : public StaticScreenDetector{
public:
    MoveSelectDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    WhiteButtonDetector m_status_button;
    GradientArrowDetector m_arrow;
};
class MoveSelectFinder : public VisualInferenceCallback{
public:
    MoveSelectFinder(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    MoveSelectDetector m_detector;
    size_t m_trigger_count = 0;
};



class TeraCatchDetector : public StaticScreenDetector{
public:
    TeraCatchDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

private:
    GradientArrowDetector m_arrow;
};
class TeraCatchFinder : public VisualInferenceCallback{
public:
    TeraCatchFinder(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

private:
    TeraCatchDetector m_detector;
    size_t m_trigger_count = 0;
};





}
}
}
#endif
