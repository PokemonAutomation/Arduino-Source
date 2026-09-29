/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSwSh_EncounterFilterEnums.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



const EnumDropdownDatabase<EncounterAction>& EncounterAction_Database(){
    static const EnumDropdownDatabase<EncounterAction> database({
        {EncounterAction::StopProgram,          "stop",                 "Stop Program"},
        {EncounterAction::RunAway,              "run",                  "Run Away"},
        {EncounterAction::ThrowBalls,           "throw-balls",          "Throw balls."},
        {EncounterAction::ThrowBallsAndSave,    "throw-balls-and-save", "Throw balls. Save if caught."},
    });
    return database;
}



const EnumDropdownDatabase<ShinyFilter>& ShinyFilter_Normal_Database(){
    static const EnumDropdownDatabase<ShinyFilter> database({
        {ShinyFilter::ANYTHING,     "anything",     "Anything"},
        {ShinyFilter::NOT_SHINY,    "not-shiny",    "Not Shiny"},
        {ShinyFilter::ANY_SHINY,    "any-shiny",    "Any Shiny"},
        {ShinyFilter::STAR_ONLY,    "star-shiny",   "Star Shiny"},
        {ShinyFilter::SQUARE_ONLY,  "square-shiny", "Square Shiny"},
//        {ShinyFilter::NOTHING,      "nothing",      "Nothing"},
    });
    return database;
}
const EnumDropdownDatabase<ShinyFilter>& ShinyFilter_StopRareStars_Database(){
    static const EnumDropdownDatabase<ShinyFilter> database({
        {ShinyFilter::NOT_SHINY,    "not-shiny",    "Not Shiny"},
        {ShinyFilter::ANY_SHINY,    "any-shiny",    "Any Shiny"},
        {ShinyFilter::STAR_ONLY,    "star-shiny",   "Star Shiny"},
    });
    return database;
}
const EnumDropdownDatabase<ShinyFilter>& ShinyFilter_TableRareStars_Database(){
    static const EnumDropdownDatabase<ShinyFilter> database({
        {ShinyFilter::NOT_SHINY,    "not-shiny",    "Not Shiny"},
        {ShinyFilter::SQUARE_ONLY,  "square-shiny", "Square Shiny"},
    });
    return database;
}



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







}
}
}
