/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/
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
const EnumDropdownDatabase<EncounterAction>& EncounterAction_Database();


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
const EnumDropdownDatabase<ShinyFilter>& ShinyFilter_Normal_Database();
const EnumDropdownDatabase<ShinyFilter>& ShinyFilter_StopRareStars_Database();
const EnumDropdownDatabase<ShinyFilter>& ShinyFilter_TableRareStars_Database();

class ShinyFilterCell : public EnumDropdownCell<ShinyFilter>{
public:
    ShinyFilterCell(bool rare_stars)
        : EnumDropdownCell<ShinyFilter>(
            rare_stars ? ShinyFilter_TableRareStars_Database() : ShinyFilter_Normal_Database(),
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
