/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EncounterFilterEnums_H
#define PokemonAutomation_PokemonBDSP_EncounterFilterEnums_H

#include <string>
#include <vector>
#include <map>
#include "PokemonSwSh/Options/EncounterFilter/PokemonSwSh_EncounterFilterEnums.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


using EncounterAction = PokemonSwSh::EncounterAction;
using EncounterActionCell = PokemonSwSh::EncounterActionCell;

extern const std::vector<std::string> EncounterAction_NAMES;
extern const std::map<std::string, EncounterAction> EncounterAction_MAP;





enum class ShinyFilter{
    ANYTHING,
    NOT_SHINY,
    SHINY,
    NOTHING,
};
const EnumDropdownDatabase<ShinyFilter>& ShinyFilter_Database();

class ShinyFilterCell : public EnumDropdownCell<ShinyFilter>{
public:
    ShinyFilterCell()
        : EnumDropdownCell<ShinyFilter>(
            ShinyFilter_Database(),
            LockMode::LOCK_WHILE_RUNNING,
            ShinyFilter::ANYTHING
        )
    {}
};

extern const std::vector<std::string> ShinyFilter_NAMES;
extern const std::map<std::string, ShinyFilter> ShinyFilter_MAP;





}
}
}
#endif
