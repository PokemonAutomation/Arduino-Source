/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraCardDetector_H
#define PokemonAutomation_PokemonSV_TeraCardDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Inference/VisualDetector.h"
#include "CommonFramework/InferenceInfra/VisualInferenceCallback.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class TeraCardReader : public StaticScreenDetector{
public:
    TeraCardReader(Color color);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;

    size_t stars(const ImageViewRGB32& screen) const;

protected:
    Color m_color;
    ImageFloatBox m_top;
    ImageFloatBox m_bottom_left;
    ImageFloatBox m_bottom_right;
    ImageFloatBox m_label;
    ImageFloatBox m_cursor;
    ImageFloatBox m_stars;
};
class TeraCardFinder : public DetectorToFinder<TeraCardReader>{
public:
    TeraCardFinder(Color color, std::chrono::milliseconds duration = std::chrono::milliseconds(250))
         : DetectorToFinder("TeraCardFinder", duration, color)
    {}
};



}
}
}
#endif
