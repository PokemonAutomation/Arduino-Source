/*  Square Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_SquareDetector_H
#define PokemonAutomation_PokemonSwSh_SquareDetector_H

#include "CommonFramework/ImageTools/FillGeometry.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



bool is_square2(
    const QImage& image,
    const CellMatrix& matrix,
    const FillGeometry& object,
    double max_deviation = 0.04
);



}
}
}
#endif
