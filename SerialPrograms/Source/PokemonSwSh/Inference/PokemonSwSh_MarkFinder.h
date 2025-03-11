/*  Mark Finder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MarkFinder_H
#define PokemonAutomation_PokemonSwSh_MarkFinder_H

#include <vector>
#include "CommonFramework/ImageTools/ImageBoxes.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


std::vector<ImagePixelBox> find_exclamation_marks(const ImageViewRGB32& image);
std::vector<ImagePixelBox> find_question_marks(const ImageViewRGB32& image);


}
}
}
#endif
