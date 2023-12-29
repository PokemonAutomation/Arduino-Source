/*  Singles AI Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_SinglesAIOption_H
#define PokemonAutomation_PokemonSV_SinglesAIOption_H

#include "Common/Cpp/Containers/FixedLimitVector.h"
#include "Common/Cpp/Options/StaticTextOption.h"
//#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "PokemonSV_SinglesMoveTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class SinglesAIOption : public GroupOption{
public:
    SinglesAIOption(bool trainer_battle);

public:
    StaticTextOption description;
    FixedLimitVector<SinglesMoveTable> MOVE_TABLES;

};




}
}
}
#endif
