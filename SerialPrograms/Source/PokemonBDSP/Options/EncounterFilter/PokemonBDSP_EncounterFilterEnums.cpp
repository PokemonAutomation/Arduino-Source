/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonBDSP_EncounterFilterEnums.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



const EnumDropdownDatabase<ShinyFilter>& ShinyFilter_Database(){
    static const EnumDropdownDatabase<ShinyFilter> database({
        {ShinyFilter::ANYTHING,     "anything",     "Anything"},
        {ShinyFilter::NOT_SHINY,    "not-shiny",    "Not Shiny"},
        {ShinyFilter::SHINY,        "shiny",        "Shiny"},
        {ShinyFilter::NOTHING,      "nothing",      "Nothing"},
    });
    return database;
}


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
    "Shiny",
    "Nothing",
};
const std::map<std::string, ShinyFilter> ShinyFilter_MAP{
    {ShinyFilter_NAMES[0], ShinyFilter::ANYTHING},
    {ShinyFilter_NAMES[1], ShinyFilter::NOT_SHINY},
    {ShinyFilter_NAMES[2], ShinyFilter::SHINY},
    {ShinyFilter_NAMES[3], ShinyFilter::NOTHING},
};



}
}
}
