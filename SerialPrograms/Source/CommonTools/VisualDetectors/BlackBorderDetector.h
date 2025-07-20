/*  Black Screen Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_BlackBorderDetector_H
#define PokemonAutomation_CommonTools_BlackBorderDetector_H

#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{


class BlackBorderDetector : public StaticScreenDetector{
public:
    BlackBorderDetector();

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) override;

private:
    ImageFloatBox m_top;
    ImageFloatBox m_bottom;
    ImageFloatBox m_left;
    ImageFloatBox m_right;
//    ImageFloatBox m_body;
};



}
#endif
