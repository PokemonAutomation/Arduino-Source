/*  Encounter Filter Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV/Resources/PokemonSV_NameDatabase.h"
#include "PokemonSV_EncounterActionsTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;




const EnumDropdownDatabase<EncounterActionsAction>& EncounterFilterAction_database(){
    static EnumDropdownDatabase<EncounterActionsAction> database{
        {EncounterActionsAction::RUN_AWAY,              "run-away",     "Run Away"},
        {EncounterActionsAction::STOP_PROGRAM,          "stop-program", "Stop Program"},
        {EncounterActionsAction::THROW_BALLS,           "throw-balls",  "Throw Balls"},
        {EncounterActionsAction::THROW_BALLS_AND_SAVE,  "throw-balls-and-save",  "Throw Balls. Save if caught."},
    };
    return database;
}
const EnumDropdownDatabase<EncounterActionsShininess>& EncounterFilterShininess_database(){
    static EnumDropdownDatabase<EncounterActionsShininess> database{
        {EncounterActionsShininess::ANYTHING,   "anything",     "Anything"},
        {EncounterActionsShininess::NOT_SHINY,  "not-shiny",    "Not Shiny"},
        {EncounterActionsShininess::SHINY,      "shiny",        "Shiny"},
    };
    return database;
}




EncounterActionsRow::~EncounterActionsRow(){
    action.remove_listener(*this);
}
EncounterActionsRow::EncounterActionsRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , pokemon(
        ALL_POKEMON_NAMES(),
        LockMode::UNLOCK_WHILE_RUNNING,
        "glimmora"
    )
    , shininess(
        EncounterFilterShininess_database(),
        LockMode::UNLOCK_WHILE_RUNNING,
        EncounterActionsShininess::SHINY
    )
    , action(
        EncounterFilterAction_database(),
        LockMode::UNLOCK_WHILE_RUNNING,
        EncounterActionsAction::STOP_PROGRAM
    )
    , pokeball("poke-ball")
    , ball_limit(LockMode::UNLOCK_WHILE_RUNNING, 40, 1, 999)
{
    PA_ADD_OPTION(pokemon);
    PA_ADD_OPTION(shininess);
    PA_ADD_OPTION(action);
    PA_ADD_OPTION(pokeball);
    PA_ADD_OPTION(ball_limit);

    EncounterActionsRow::on_config_value_changed(this);

    action.add_listener(*this);
}
std::unique_ptr<EditableTableRow> EncounterActionsRow::clone() const{
    std::unique_ptr<EncounterActionsRow> ret(new EncounterActionsRow(parent()));
    ret->action.set(action);
    ret->pokeball.set_by_index(pokeball.index());
    ret->pokemon.set_by_index(pokemon.index());
    ret->shininess.set(shininess);
    ret->ball_limit.set(ball_limit);
    return ret;
}
EncounterActionsEntry EncounterActionsRow::snapshot() const{
    return {
        pokemon.slug(),
        shininess,
        action,
        pokeball.slug(),
        ball_limit,
    };
}
void EncounterActionsRow::on_config_value_changed(void* object){
    switch (action){
    case EncounterActionsAction::STOP_PROGRAM:
    case EncounterActionsAction::RUN_AWAY:
        pokeball.set_visibility(ConfigOptionState::HIDDEN);
        ball_limit.set_visibility(ConfigOptionState::HIDDEN);
        break;
    case EncounterActionsAction::THROW_BALLS:
    case EncounterActionsAction::THROW_BALLS_AND_SAVE:
        pokeball.set_visibility(ConfigOptionState::ENABLED);
        ball_limit.set_visibility(ConfigOptionState::ENABLED);
        break;
    }
}



EncounterActionsTable::EncounterActionsTable()
    : EditableTableOption_t<EncounterActionsRow>(
        "<b>Actions Table:</b><br>"
        "By default, the program will run from non-shinies and stop on shinies. "
        "This table lets you override this default behavior for specific " +
        STRING_POKEMON + " and their shininess. "
        "If multiple entries match, the last one will be chosen. Note that running from "
        "shinies will not despawn them since they cannot be killed via Let's Go.",
        LockMode::UNLOCK_WHILE_RUNNING,
        make_defaults()
    )
{}

std::vector<EncounterActionsEntry> EncounterActionsTable::snapshot(){
    return EditableTableOption_t<EncounterActionsRow>::snapshot<EncounterActionsEntry>();
}
std::vector<std::string> EncounterActionsTable::make_header() const{
    return {
        "Species",
        "Shininess",
        "Action",
        Pokemon::STRING_POKEBALL,
        "Ball Limit"
    };
}
std::vector<std::unique_ptr<EditableTableRow>> EncounterActionsTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    return ret;
}






}
}
}
