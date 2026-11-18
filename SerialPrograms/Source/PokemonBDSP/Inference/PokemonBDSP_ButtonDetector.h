/*  BDSP Button Icon Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */
#ifndef PokemonAutomation_PokemonBDSP_ButtonDetector_H
#define PokemonAutomation_PokemonBDSP_ButtonDetector_H

#include "CommonFramework/ImageTools/ImageBoxes.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

enum class ButtonType{A, B, X, Y};

// Match the letter inside a gray controller-button icon, without reading its label.
class ButtonDetector{
public:
    ButtonDetector(ButtonType type, const ImageFloatBox& box)
        : m_type(type), m_box(box)
    {}
    bool detect(const ImageViewRGB32& screen, ImageFloatBox* matched_box = nullptr) const;

private:
    ButtonType m_type;
    ImageFloatBox m_box;
};

}
}
}
#endif
