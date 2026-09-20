/*  Gradient Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_GradientArrowDetector_H
#define PokemonAutomation_PokemonSV_GradientArrowDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

enum class GradientArrowType{
    RIGHT,
    DOWN,
};


class GradientArrowDetector : public StaticScreenDetector{
public:
    GradientArrowDetector(
        Color color,
        GradientArrowType type,
        const ImageFloatBox& box
    );

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

    //  If arrow is found, returns true and "box" contains the box for the arrow.
    //  Otherwise, returns false and "box" is undefined.
    bool detect(ImageFloatBox& box, const ImageViewRGB32& screen) const;

protected:
    Color m_color;
    GradientArrowType m_type;
    ImageFloatBox m_box;
};


class GradientArrowWatcher : public DetectorToFinder<GradientArrowDetector>{
public:
    GradientArrowWatcher(
        Color color,
        GradientArrowType type,
        const ImageFloatBox& box,
        std::chrono::milliseconds hold_duration = std::chrono::milliseconds(250)
    )
        : DetectorToFinder("GradientArrowWatcher", hold_duration, color, type, box)
    {}
};


#if 0
class GradientArrowWatcher : public VisualInferenceCallback{
public:
    ~GradientArrowWatcher();
    GradientArrowWatcher(
        Color color,
        GradientArrowType type,
        const ImageFloatBox& box
    );

//    VideoSnapshot last_detected() const{ return m_last_detected; }

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool process_frame(const VideoSnapshot& frame) override;
//    virtual bool process_frame(const ImageViewRGB32& frame, WallClock timestamp) override;

protected:
    GradientArrowDetector m_detector;

//    VideoSnapshot m_last_detected;
};
#endif


}
}
}
#endif
