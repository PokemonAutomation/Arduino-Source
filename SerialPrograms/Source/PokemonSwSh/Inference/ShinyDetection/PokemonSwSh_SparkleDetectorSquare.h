/*  Square Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_SquareDetector2_H
#define PokemonAutomation_PokemonSwSh_SquareDetector2_H

#include <cstddef>

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{

class WaterFillObject;

}
}
}

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



bool is_square_sparkle(const Kernels::Waterfill::WaterFillObject& object, double max_deviation = 0.04);
bool is_line_sparkle(const Kernels::Waterfill::WaterFillObject& object, size_t min_pixel_width = 100);




}
}
}
#endif
