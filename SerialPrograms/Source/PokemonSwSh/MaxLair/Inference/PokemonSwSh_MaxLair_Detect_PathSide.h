/*  Max Lair Detect Path Side
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_PathSide_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_PathSide_H

#include "CommonFramework/ImageTools/FillGeometry.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


bool is_arrow_pointed_up(const CellMatrix& matrix, const FillGeometry& object);
bool is_arrow_pointed_corner(const CellMatrix& matrix, const FillGeometry& object);

bool is_arrow(const QImage& image, const CellMatrix& matrix, const FillGeometry& object);

int8_t read_side(const QImage& image, int p_min_rgb_sum);
int8_t read_side(const QImage& image);


}
}
}
}
#endif
