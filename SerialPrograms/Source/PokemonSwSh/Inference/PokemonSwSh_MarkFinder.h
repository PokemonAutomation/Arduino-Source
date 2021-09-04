/*  Mark Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MarkFinder_H
#define PokemonAutomation_PokemonSwSh_MarkFinder_H

#include <vector>
#include <QImage>
#include "CommonFramework/ImageTools/ImageBoxes.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


size_t find_marks(
    const QImage& image,
    std::vector<ImagePixelBox>* exclamation_marks,
    std::vector<ImagePixelBox>* question_marks
);



}
}
}
#endif
