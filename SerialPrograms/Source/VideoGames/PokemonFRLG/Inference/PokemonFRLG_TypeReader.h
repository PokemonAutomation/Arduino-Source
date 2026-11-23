/*  Type Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_TypeReader_H
#define PokemonAutomation_PokemonFRLG_TypeReader_H

#include <utility>
#include "CommonFramework/Language.h"
#include "Pokemon/Pokemon_Types.h"

namespace PokemonAutomation{
    class ImageViewRGB32;
    class UnitTestDatabase;
namespace NintendoSwitch{
namespace PokemonFRLG{


class TypeReader{
public:
    //  Read the two type slots from summary page 1 in display order.
    //  An empty or unreadable slot is NONE. If the first slot cannot be read,
    //  or the frame is not summary page 1, both types are NONE.
    std::pair<Pokemon::PokemonType, Pokemon::PokemonType> read(Language language, const ImageViewRGB32& screen) const;
};

void add_tests_TypeReader(UnitTestDatabase& database);

}
}
}
#endif
