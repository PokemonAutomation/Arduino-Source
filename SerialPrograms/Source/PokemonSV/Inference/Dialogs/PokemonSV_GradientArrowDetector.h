/*  Gradient Arrow Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_GradientArrowDetector_H
#define PokemonAutomation_PokemonSV_GradientArrowDetector_H

#include <vector>
#include "Common/Cpp/Color.h"
//#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"
#include "CommonFramework/Inference/VisualDetector.h"

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
    virtual bool detect(const ImageViewRGB32& screen) const override;

    //  If arrow is found, returns true and "box" contains the box for the arrow.
    //  Otherwise, returns false and "box" is undefined.
    bool detect(ImageFloatBox& box, const ImageViewRGB32& screen) const;

protected:
    Color m_color;
    GradientArrowType m_type;
    ImageFloatBox m_box;
};



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



}
}
}
#endif
