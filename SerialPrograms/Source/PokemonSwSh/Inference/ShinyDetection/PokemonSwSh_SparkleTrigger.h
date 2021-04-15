/*  Shiny Sparkle Trigger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_SparkleTrigger_H
#define PokemonAutomation_PokemonSwSh_SparkleTrigger_H

#include "CommonFramework/Inference/FillMatrix.h"
#include "CommonFramework/Inference/FillGeometry.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class SparkleDetector{
    static const double ANGLE_TOLERANCE_DEGREES;

public:
    SparkleDetector(
        const FillMatrix& matrix,
        const FillGeometry& box,
        double percentile = 0.85
    );

    bool is_ball();
    bool is_star();


private:
    FillGeometry m_box;
    FillMatrix m_matrix;
    double m_percentile;

    std::multimap<int64_t, FillGeometry> m_regions;
    uint64_t m_radius_sqr;
};








}
}
}
#endif
