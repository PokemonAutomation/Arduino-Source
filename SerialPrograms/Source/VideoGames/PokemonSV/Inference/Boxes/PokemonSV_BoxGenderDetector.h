/*  Box Gender Detector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_BoxGenderDetector_H
#define PokemonAutomation_PokemonSwSh_BoxGenderDetector_H

#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Pokemon/Inference/Pokemon_BoxGenderDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class BoxGenderDetector : public Pokemon::BoxGenderDetector{

public:
    BoxGenderDetector(Color color = COLOR_RED);

};


}
}
}
#endif
