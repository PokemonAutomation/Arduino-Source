/*  Dialog Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_DialogDetector_H
#define PokemonAutomation_PokemonSV_DialogDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/Inference/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class DialogDetector : public StaticScreenDetector{
public:
    enum DialogType{
        NO_DIALOG,
        ADVANCE_DIALOG,
        PROMPT_DIALOG,
    };

public:
    DialogDetector(Color color = COLOR_RED);

    virtual void make_overlays(VideoOverlaySet& items) const override;
    virtual bool detect(const ImageViewRGB32& screen) const override;
    DialogType detect_with_type(const ImageViewRGB32& screen) const;

protected:
    Color m_color;
    ImageFloatBox m_box_top;
    ImageFloatBox m_box_bot;
    ImageFloatBox m_arrow;
    ImageFloatBox m_gradient;
};




}
}
}
#endif
