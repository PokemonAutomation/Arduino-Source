/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EncounterFilterEnums_H
#define PokemonAutomation_PokemonSwSh_EncounterFilterEnums_H

#include <string>
#include <vector>
#include <map>
#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



enum class EncounterAction{
    StopProgram,
    RunAway,
    ThrowBalls,
    ThrowBallsAndSave,
};
const EnumDatabase<EncounterAction>& EncounterAction_Database();


class EncounterActionCell : public EnumDropdownCell<EncounterAction>{
public:
    EncounterActionCell()
        : EnumDropdownCell<EncounterAction>(
            EncounterAction_Database(),
            LockMode::LOCK_WHILE_RUNNING,
            EncounterAction::StopProgram
        )
    {}
};



enum class ShinyFilter{
    ANYTHING,
    NOT_SHINY,
    ANY_SHINY,
    STAR_ONLY,
    SQUARE_ONLY,
    NOTHING,
};
const EnumDatabase<ShinyFilter>& ShinyFilter_Normal_Database();
const EnumDatabase<ShinyFilter>& ShinyFilter_RareStars_Database();

class ShinyFilterCell : public EnumDropdownCell<ShinyFilter>{
public:
    ShinyFilterCell(bool rare_stars)
        : EnumDropdownCell<ShinyFilter>(
            rare_stars ? ShinyFilter_RareStars_Database() : ShinyFilter_Normal_Database(),
            LockMode::LOCK_WHILE_RUNNING,
            ShinyFilter::NOT_SHINY
        )
    {}
};


extern const std::vector<std::string> ShinyFilter_NAMES;
extern const std::map<std::string, ShinyFilter> ShinyFilter_MAP;





}
}
}
#endif
