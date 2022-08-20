/*  Encounter Filter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Qt/NoWheelComboBox.h"
#include "Pokemon/Pokemon_Strings.h"
#include "Pokemon/Resources/Pokemon_PokeballNames.h"
#include "Pokemon/Resources/Pokemon_PokemonSlugs.h"
#include "Pokemon/Options/Pokemon_BallSelectWidget.h"
#include "Pokemon/Options/Pokemon_NameSelectWidget.h"
#include "PokemonSwSh/Resources/PokemonSwSh_PokemonSprites.h"
#include "PokemonSwSh_EncounterFilterOverride.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{





EncounterFilterOverride::~EncounterFilterOverride(){
    action.remove_listener(*this);
}
EncounterFilterOverride::EncounterFilterOverride(bool rare_stars)
    : m_rare_stars(rare_stars)
    , shininess(rare_stars)
{
    PA_ADD_OPTION(action);
    PA_ADD_OPTION(pokeball);
    PA_ADD_OPTION(pokemon);
    PA_ADD_OPTION(shininess);
    action.add_listener(*this);
}
void EncounterFilterOverride::load_json(const JsonValue& json){
    EditableTableRow2::load_json(json);

    //  Parse old format for backwards compatibility.
    const JsonObject* obj = json.get_object();
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
std::unique_ptr<EditableTableRow2> EncounterFilterOverride::clone() const{
    std::unique_ptr<EncounterFilterOverride> ret(new EncounterFilterOverride(m_rare_stars));
    ret->action.set(action);
    ret->pokeball.set_by_index(pokeball.index());
    ret->pokemon.set_by_index(pokemon.index());
    ret->shininess.set(shininess);
    return ret;
}
void EncounterFilterOverride::value_changed(){
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



EncounterFilterTable::EncounterFilterTable(bool rare_stars)
    : EditableTableOption2(
        rare_stars
            ?   "<b>Overrides:</b><br>"
                "The game language must be properly set to read " + STRING_POKEMON + " names. "
                "If more than one override applies, the last one will be chosen.<br>"
                "<font color=\"red\">Due to the extreme rarity of star shinies (1 in 6 million), "
                "the filters here will not allow you to run from them. "
                "If you get a star shiny, catch it and cherish it.</font>"
            :
                "<b>Overrides:</b><br>"
                "The game language must be properly set to read " + STRING_POKEMON + " names.<br>"
                "If more than one override applies, the last one will be chosen."
    )
{}
std::vector<std::unique_ptr<EncounterFilterOverride>> EncounterFilterTable::copy_snapshot() const{
    return EditableTableOption2::copy_snapshot<EncounterFilterOverride>();
}
std::vector<std::string> EncounterFilterTable::make_header() const{
    return std::vector<std::string>{
        "Action",
        STRING_POKEBALL,
        STRING_POKEMON,
        "Shininess",
    };
}
std::unique_ptr<EditableTableRow2> EncounterFilterTable::make_row() const{
    return std::unique_ptr<EditableTableRow2>(new EncounterFilterOverride(m_rare_stars));
}











}
}
}
