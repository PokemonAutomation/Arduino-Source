/*  Shiny Sparkle Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_SparkleDetector_H
#define PokemonAutomation_PokemonSwSh_SparkleDetector_H

#include <vector>
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"

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



struct RadialSparkleDetector{
    using WaterFillObject = Kernels::Waterfill::WaterFillObject;

public:
    ~RadialSparkleDetector();
    RadialSparkleDetector(const WaterFillObject& obj);

    bool is_ball() const;
    bool is_star() const;

private:
    const WaterFillObject& m_object;
    Kernels::PackedBinaryMatrix m_matrix;

    uint64_t m_radius_sqr;
    std::multimap<int64_t, WaterFillObject> m_regions;
};





}
}
}
#endif
