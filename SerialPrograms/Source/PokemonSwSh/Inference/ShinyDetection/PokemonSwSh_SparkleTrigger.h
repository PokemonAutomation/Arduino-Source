/*  Shiny Sparkle Trigger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_SparkleTrigger_H
#define PokemonAutomation_PokemonSwSh_SparkleTrigger_H

#include <map>
#include "CommonFramework/ImageTools/CellMatrix.h"
#include "CommonFramework/ImageTools/FillGeometry.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class SparkleDetectorOld{
    static const double ANGLE_TOLERANCE_DEGREES;

public:
    SparkleDetectorOld(
        const CellMatrix& matrix,
        const FillGeometry& box
    );

    bool is_ball();
    bool is_star();


private:
    FillGeometry m_box;
    CellMatrix m_matrix;

    std::multimap<size_t, FillGeometry, std::greater<size_t>> m_regions;
    uint64_t m_radius_sqr;
};








}
}
}
#endif
