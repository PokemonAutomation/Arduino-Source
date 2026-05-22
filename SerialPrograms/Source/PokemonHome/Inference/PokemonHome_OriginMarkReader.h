/*  Origin Mark Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_OriginMarkReader_H
#define PokemonAutomation_PokemonHome_OriginMarkReader_H

#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "Pokemon/Pokemon_OriginMarks.h"
#include <vector>
#include "CommonTools/DetectedBoxes.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


using Pokemon::OriginMark;

class OriginMarkReader{
public:
    OriginMark read_mark(
        const ImageViewRGB32& original_screen,
        const ImageFloatBox& box
    );

private:
    std::vector<DetectedBox> m_last_detected;

};

}
}
}
#endif
