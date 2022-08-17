/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSwSh_EncounterFilterEnums.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



const std::vector<std::string> EncounterAction_NAMES{
    "Stop Program",
    "Run Away",
    "Throw balls.",
    "Throw balls. Save if caught.",
};
const std::map<std::string, EncounterAction> EncounterAction_MAP{
    {EncounterAction_NAMES[0], EncounterAction::StopProgram},
    {EncounterAction_NAMES[1], EncounterAction::RunAway},
    {EncounterAction_NAMES[2], EncounterAction::ThrowBalls},
    {EncounterAction_NAMES[3], EncounterAction::ThrowBallsAndSave},
};


const std::vector<std::string> ShinyFilter_NAMES{
    "Anything",
    "Not Shiny",
    "Any Shiny",
    "Star Shiny",
    "Square Shiny",
    "Nothing",
};
const std::map<std::string, ShinyFilter> ShinyFilter_MAP{
    {ShinyFilter_NAMES[0], ShinyFilter::ANYTHING},
    {ShinyFilter_NAMES[1], ShinyFilter::NOT_SHINY},
    {ShinyFilter_NAMES[2], ShinyFilter::ANY_SHINY},
    {ShinyFilter_NAMES[3], ShinyFilter::STAR_ONLY},
    {ShinyFilter_NAMES[4], ShinyFilter::SQUARE_ONLY},
    {ShinyFilter_NAMES[5], ShinyFilter::NOTHING},
};




ShinyFilterCell::operator ShinyFilter() const{
    if (m_rare_stars){
        return current_index() == 0 ? ShinyFilter::NOT_SHINY : ShinyFilter::SQUARE_ONLY;
    }
    return (ShinyFilter)current_index();
}
void ShinyFilterCell::set(ShinyFilter action){
    if (m_rare_stars){
        if (action == ShinyFilter::NOT_SHINY){
            DropdownCell::set_index(0);
        }
        if (action == ShinyFilter::SQUARE_ONLY){
            DropdownCell::set_index(1);
        }
        return;
    }
    DropdownCell::set_index((size_t)action);
}




}
}
}
