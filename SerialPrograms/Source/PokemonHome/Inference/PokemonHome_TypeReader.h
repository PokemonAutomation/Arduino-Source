/*  Type Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_TypeReader_H
#define PokemonAutomation_PokemonHome_TypeReader_H

#include <utility>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "Pokemon/Pokemon_Types.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

using namespace Pokemon;


class TypeReader{
public:
    TypeReader(const ImageFloatBox& box);

    //  Returns up to 2 detected types, best match first.
    //  The second type is PokemonType::NONE for single-type Pokemon.
    std::pair<PokemonType, PokemonType> read_types(const ImageViewRGB32& screen) const;

private:
    OverlayBoxScope m_box;
};



}
}
}
#endif
