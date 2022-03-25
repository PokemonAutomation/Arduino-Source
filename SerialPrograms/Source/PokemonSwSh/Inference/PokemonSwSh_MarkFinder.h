/*  Mark Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MarkFinder_H
#define PokemonAutomation_PokemonSwSh_MarkFinder_H

#include <vector>
#include "CommonFramework/ImageTools/ImageBoxes.h"

class QImage;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


std::vector<ImagePixelBox> find_exclamation_marks(const ConstImageRef& image);
std::vector<ImagePixelBox> find_question_marks(const ConstImageRef& image);


}
}
}
#endif
