/*  Pokemon Origin Marks
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_OriginMarks_H
#define PokemonAutomation_Pokemon_OriginMarks_H

#include "Common/Cpp/EnumStringMap.h"

namespace PokemonAutomation{
namespace Pokemon{


enum class OriginMark{
    KALOS,
    ALOLA,
    GAMEBOY,
    GO,
    LGPE,
    GALAR,
    BDSP,
    LA,
    SV,
    LZA,
    NONE,
};
const EnumStringMap<OriginMark>& ORIGIN_MARK_SLUGS();


}
}
#endif
