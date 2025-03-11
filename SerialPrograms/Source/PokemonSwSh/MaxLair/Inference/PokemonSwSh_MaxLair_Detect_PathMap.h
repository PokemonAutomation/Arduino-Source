/*  Max Lair Detect Path Map
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Detect_PathMap_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Detect_PathMap_H

#include <deque>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "Pokemon/Pokemon_Types.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_State.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{
using namespace Pokemon;


bool read_type_array(
    VideoStream& stream,
    const ImageViewRGB32& screen,
    const ImageFloatBox& box,
    std::deque<OverlayBoxScope>& hits,
    size_t count,
    PokemonType* type, ImagePixelBox* boxes
);


bool read_path(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
    PathMap& path,
    const ImageFloatBox& box
);


}
}
}
}
#endif
