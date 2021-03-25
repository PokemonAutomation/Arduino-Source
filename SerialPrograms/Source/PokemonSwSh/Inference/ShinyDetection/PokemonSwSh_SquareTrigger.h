/*  Line Trigger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_LineTrigger_H
#define PokemonAutomation_PokemonSwSh_LineTrigger_H

#include <vector>
#include "CommonFramework/Inference/FillGeometry.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


bool is_square_beam(
    const FillMatrix& matrix,
    const FillGeometry& object,
    double min_length
);


bool is_square(
    const FillMatrix& matrix,
    const FillGeometry& object
);


}
}
}
#endif
