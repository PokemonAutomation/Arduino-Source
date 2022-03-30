/*  Shiny Sparkle Detector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_SparkleDetector_H
#define PokemonAutomation_PokemonSwSh_SparkleDetector_H

#include <vector>
#include <map>
#include "CommonFramework/ImageTypes/BinaryImage.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"

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



struct RadialSparkleDetector{
    using WaterfillObject = Kernels::Waterfill::WaterfillObject;

public:
    ~RadialSparkleDetector();
    RadialSparkleDetector(const WaterfillObject& obj);

    bool is_ball() const;
    bool is_star() const;

private:
    const WaterfillObject& m_object;
    PackedBinaryMatrix2 m_matrix;

    uint64_t m_radius_sqr;
    std::multimap<int64_t, WaterfillObject> m_regions;
};





}
}
}
#endif
