/*  Square Trigger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_SquareTrigger_H
#define PokemonAutomation_PokemonSwSh_SquareTrigger_H

#include <vector>
#include "CommonFramework/ImageTools/FillGeometry.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


bool is_square_beam(
    const CellMatrix& matrix,
    const FillGeometry& object,
    double min_length
);


//bool is_square(
//    const FillMatrix& matrix,
//    const FillGeometry& object
//);


}
}
}
#endif
