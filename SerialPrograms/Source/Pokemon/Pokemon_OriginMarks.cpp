/*  Pokemon Origin Marks
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Pokemon_OriginMarks.h"

namespace PokemonAutomation{
namespace Pokemon{

const EnumStringMap<OriginMark>& ORIGIN_MARK_SLUGS(){
    static EnumStringMap<OriginMark> database{
        {OriginMark::KALOS, "kalos"},
        {OriginMark::ALOLA, "alola"},
        {OriginMark::GAMEBOY, "gameboy"},
        {OriginMark::GO, "go"},
        {OriginMark::LGPE, "lgpe"},
        {OriginMark::GALAR, "galar"},
        {OriginMark::BDSP, "bdsp"},
        {OriginMark::LA, "la"},
        {OriginMark::SV, "sv"},
        {OriginMark::LZA, "lza"},
        {OriginMark::NONE, "none"}
    };
    return database;
}


}
}
