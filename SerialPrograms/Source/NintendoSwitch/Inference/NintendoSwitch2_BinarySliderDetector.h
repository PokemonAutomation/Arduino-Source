/*  Binary Slider Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch2_BinarySliderDetector_H
#define PokemonAutomation_NintendoSwitch2_BinarySliderDetector_H

#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonTools/VisualDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




class BinarySliderDetector{
public:
    BinarySliderDetector(Color color, const ImageFloatBox& box);
    void make_overlays(VideoOverlaySet& items) const;

    std::vector<std::pair<bool, ImagePixelBox>> detect(const ImageViewRGB32& screen) const;

private:
    Color m_color;
    ImageFloatBox m_box;
};




}
}
#endif
