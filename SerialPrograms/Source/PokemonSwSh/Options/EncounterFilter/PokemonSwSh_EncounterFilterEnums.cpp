/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSwSh_EncounterFilterEnums.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


const std::vector<QString> ShinyFilter_NAMES{
    "Anything",
    "Not Shiny",
    "Any Shiny",
    "Star Shiny",
    "Square Shiny",
    "Nothing",
};
const std::map<QString, ShinyFilter> ShinyFilter_MAP{
    {ShinyFilter_NAMES[0], ShinyFilter::ANYTHING},
    {ShinyFilter_NAMES[1], ShinyFilter::NOT_SHINY},
    {ShinyFilter_NAMES[2], ShinyFilter::ANY_SHINY},
    {ShinyFilter_NAMES[3], ShinyFilter::STAR_ONLY},
    {ShinyFilter_NAMES[4], ShinyFilter::SQUARE_ONLY},
    {ShinyFilter_NAMES[5], ShinyFilter::NOTHING},
};

const std::vector<QString> EncounterAction_NAMES{
    "Stop Program",
    "Run Away",
    "Throw balls.",
    "Throw balls. Save if caught.",
};
const std::map<QString, EncounterAction> EncounterAction_MAP{
    {EncounterAction_NAMES[0], EncounterAction::StopProgram},
    {EncounterAction_NAMES[1], EncounterAction::RunAway},
    {EncounterAction_NAMES[2], EncounterAction::ThrowBalls},
    {EncounterAction_NAMES[3], EncounterAction::ThrowBallsAndSave},
};




}
}
}
