/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Resources/Pokemon_PokeballNames.h"
#include "PokemonBDSP/Resources/PokemonBDSP_NameDatabase.h"
#include "PokemonBDSP_EncounterFilterEnums.h"
#include "PokemonBDSP_EncounterFilterOverride.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;


bool EncounterActionFull::operator==(const EncounterActionFull& x) const{
    switch (action){
    case EncounterAction::StopProgram:
    case EncounterAction::RunAway:
        return action == x.action;
    case EncounterAction::ThrowBalls:
    case EncounterAction::ThrowBallsAndSave:
        return action == x.action && pokeball_slug == x.pokeball_slug;
    }
    throw InternalProgramError(
        nullptr, PA_CURRENT_FUNCTION,
        "EncounterActionFull: Invalid Enum " + std::to_string((int)action)
    );
}
bool EncounterActionFull::operator!=(const EncounterActionFull& x) const{
    return !(*this == x);
}
std::string EncounterActionFull::to_str() const{
    std::string str;
    str += EncounterAction_NAMES[(size_t)action];
    if (action == EncounterAction::ThrowBalls || action == EncounterAction::ThrowBallsAndSave){
        str += " (";
        str += get_pokeball_name(pokeball_slug).display_name();
        str += ")";
    }
    return str;
}





EncounterFilterOverride::~EncounterFilterOverride(){
    action.remove_listener(*this);
}
EncounterFilterOverride::EncounterFilterOverride(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , pokemon(ALL_POKEMON_NAMES(), LockMode::UNLOCK_WHILE_RUNNING, "starly")
    , ball_limit(LockMode::UNLOCK_WHILE_RUNNING, 40, 1, 999)
{
    PA_ADD_OPTION(pokemon);
    PA_ADD_OPTION(shininess);
    PA_ADD_OPTION(action);
    PA_ADD_OPTION(pokeball);
    PA_ADD_OPTION(ball_limit);
    action.add_listener(*this);
}
void EncounterFilterOverride::load_json(const JsonValue& json){
    EditableTableRow::load_json(json);

    //  Parse old format for backwards compatibility.
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }

    const JsonValue* value;
    value = obj->get_value("Action");
    if (value != nullptr){
        action.load_json(*value);
    }
    value = obj->get_value("Ball");
    if (value != nullptr){
        pokeball.load_json(*value);
    }
    value = obj->get_value("Species");
    if (value != nullptr){
        pokemon.load_json(*value);
    }
    value = obj->get_value("ShinyFilter");
    if (value != nullptr){
        shininess.load_json(*value);
    }
}
std::unique_ptr<EditableTableRow> EncounterFilterOverride::clone() const{
    std::unique_ptr<EncounterFilterOverride> ret(new EncounterFilterOverride(parent()));
    ret->action.set(action);
    ret->pokeball.set_by_index(pokeball.index());
    ret->pokemon.set_by_index(pokemon.index());
    ret->shininess.set(shininess);
    ret->ball_limit.set(ball_limit);
    return ret;
}
void EncounterFilterOverride::on_config_value_changed(void* object){
    switch ((EncounterAction)action){
    case EncounterAction::StopProgram:
    case EncounterAction::RunAway:
        pokeball.set_visibility(ConfigOptionState::DISABLED);
        break;
    case EncounterAction::ThrowBalls:
    case EncounterAction::ThrowBallsAndSave:
        pokeball.set_visibility(ConfigOptionState::ENABLED);
        break;
    }
}





EncounterFilterTable::EncounterFilterTable()
    : EditableTableOption_t<EncounterFilterOverride>(
        "<b>Overrides:</b><br>"
        "The game language must be properly set to read " + STRING_POKEMON + " names. "
        "If multiple overrides apply and are conflicting, the program will stop." +
        "<br>Auto-catching only applies in single battles. The program will stop if asked to auto-catch in a double-battle.",
        LockMode::UNLOCK_WHILE_RUNNING
    )
{}
std::vector<std::string> EncounterFilterTable::make_header() const{
    return std::vector<std::string>{
        STRING_POKEMON,
        "Shininess",
        "Action",
        STRING_POKEBALL,
        "Ball Limit"
    };
}

















}
}
}
