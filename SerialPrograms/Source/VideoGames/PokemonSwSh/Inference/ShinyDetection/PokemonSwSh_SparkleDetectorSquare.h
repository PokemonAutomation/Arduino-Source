/*  Square Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_SquareDetector_H
#define PokemonAutomation_PokemonSwSh_SquareDetector_H

#include <cstddef>

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{

class WaterfillObject;

}
}
}

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



bool is_square_sparkle(const Kernels::Waterfill::WaterfillObject& object, double max_deviation = 0.04);
bool is_line_sparkle(const Kernels::Waterfill::WaterfillObject& object, size_t min_pixel_width = 100);




}
}
}
#endif
