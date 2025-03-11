/*  Tera AI Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_TeraAIOption_H
#define PokemonAutomation_PokemonSV_TeraAIOption_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "PokemonSV_TeraMoveTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class TeraAIOption : public GroupOption{
public:
    TeraAIOption();

public:
    StaticTextOption description;
    BooleanCheckBoxOption TRY_TO_TERASTILLIZE;
    TeraMoveTable MOVE_TABLE;

};




}
}
}
#endif
