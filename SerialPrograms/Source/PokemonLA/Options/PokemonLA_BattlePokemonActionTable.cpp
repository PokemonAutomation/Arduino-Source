/*  Battle Pokemon Action Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "Common/Compiler.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/Options/EditableTableOption-EnumTableCell.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA_BattlePokemonActionTable.h"

#include <QLabel>
#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;


const std::string MoveStyle_NAMES[] = {
    "No Style",
    "Agile",
    "Strong",
};
const std::map<std::string, MoveStyle> MoveStyle_MAP{
    {MoveStyle_NAMES[0], MoveStyle::NoStyle},
    {MoveStyle_NAMES[1], MoveStyle::Agile},
    {MoveStyle_NAMES[2], MoveStyle::Strong},
};


BattlePokemonActionRow::BattlePokemonActionRow() {}

void BattlePokemonActionRow::load_json(const JsonValue& json){
    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        return;
    }
    {
        const std::string* str;
        for(int i = 0; i < 4; i++){
            str = obj->get_string("Style" + std::to_string(i));
            if (str == nullptr){
                continue;
            }
            const auto iter = MoveStyle_MAP.find(*str);
            if (iter != MoveStyle_MAP.end()){
                style[i] = iter->second;
            }
        }
    }
    obj->read_boolean(switch_pokemon, "Switch");
    obj->read_integer(num_turns_to_switch, "Turns", 0, 65535);
    obj->read_boolean(stop_after_num_moves, "StopAfterNumMoves");
    obj->read_integer(num_moves_to_stop, "NumMovesToStop", 0, 65535);
}
JsonValue BattlePokemonActionRow::to_json() const{
    JsonObject obj;
    for(int i = 0; i < 4; i++){
        obj["Style" + std::to_string(i)] = MoveStyle_NAMES[(size_t)style[i]];
    }
    
    obj["Switch"] = switch_pokemon;
    obj["Turns"] = num_turns_to_switch;
    obj["StopAfterNumMoves"] = stop_after_num_moves;
    obj["NumMovesToStop"] = num_moves_to_stop;
    return obj;
}

std::unique_ptr<EditableTableRow> BattlePokemonActionRow::clone() const{
    return std::unique_ptr<EditableTableRow>(new BattlePokemonActionRow(*this));
}

std::vector<QWidget*> BattlePokemonActionRow::make_widgets(QWidget& parent){
    std::vector<QWidget*> widgets;
    for(int i = 0; i < 4; i++){
        widgets.emplace_back(make_enum_table_cell(parent, MoveStyle_MAP.size(), MoveStyle_NAMES, style[i]));
    }
    widgets.emplace_back(make_boolean_table_cell(parent, this->switch_pokemon));
    widgets.emplace_back(make_integer_table_cell(parent, this->num_turns_to_switch));
    widgets.emplace_back(make_boolean_table_cell(parent, this->stop_after_num_moves));
    widgets.emplace_back(make_integer_table_cell(parent, this->num_moves_to_stop));
    return widgets;
}



std::vector<std::string> BattlePokemonActionTableFactory::make_header() const{
    return std::vector<std::string>{
        "Move 1 Style",
        "Move 2 Style",
        "Move 3 Style",
        "Move 4 Style",
        "Switch " + STRING_POKEMON,
        "Num Turns to Switch",
        "Limit Move Attempts",
        "Max Move Attempts",
    };
}

std::unique_ptr<EditableTableRow> BattlePokemonActionTableFactory::make_row() const{
    return std::unique_ptr<EditableTableRow>(new BattlePokemonActionRow());
}




std::vector<std::unique_ptr<EditableTableRow>> BattlePokemonActionTable::make_defaults() const{
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(std::make_unique<BattlePokemonActionRow>());
    return ret;
}

BattlePokemonActionTable::BattlePokemonActionTable()
    : m_table(
        "<b>" + STRING_POKEMON + " Action Table:</b><br>"
        "Set what move styles to use and whether to switch the " + STRING_POKEMON + " after some turns.<br><br>"
        "Each row is the action for one " + STRING_POKEMON + ". "
        "The table follows the order that " + STRING_POKEMON + " are sent to battle.<br>"
        "You can also set a target number of move attempts. After it is reached the program will finish the battle and stop.<br><br>"
        "Note: if your second last " + STRING_POKEMON + " faints, the game will send your last " + STRING_POKEMON + " automatically for you.<br>"
        "The program cannot detect this switch as there is no switch selection screen. "
        "Therefore the program will treat it as the same " + STRING_POKEMON + ".",
        m_factory, make_defaults()
    )
{}

void BattlePokemonActionTable::load_json(const JsonValue& json){
    m_table.load_json(json);
}

JsonValue BattlePokemonActionTable::to_json() const{
    return m_table.to_json();
}

void BattlePokemonActionTable::restore_defaults(){
    m_table.restore_defaults();
}

ConfigWidget* BattlePokemonActionTable::make_ui(QWidget& parent){
    return m_table.make_ui(parent);
}

MoveStyle BattlePokemonActionTable::get_style(size_t pokemon, size_t move) const{
    if (pokemon >= m_table.size()){
        return MoveStyle::NoStyle;
    }

    const BattlePokemonActionRow& action = static_cast<const BattlePokemonActionRow&>(m_table[pokemon]);
    return action.style[move];
}

bool BattlePokemonActionTable::switch_pokemon(size_t pokemon, size_t num_turns) const{
    if (pokemon >= m_table.size()){
        return false;
    }

    const BattlePokemonActionRow& action = static_cast<const BattlePokemonActionRow&>(m_table[pokemon]);
    return action.switch_pokemon && num_turns >= (size_t)action.num_turns_to_switch;
}

bool BattlePokemonActionTable::stop_battle(size_t pokemon, size_t num_move_attempts) const{
    if (pokemon >= m_table.size()){
        return false;
    }

    const BattlePokemonActionRow& action = static_cast<const BattlePokemonActionRow&>(m_table[pokemon]);
    return action.stop_after_num_moves && num_move_attempts >= (size_t)action.num_moves_to_stop;
}



OneMoveBattlePokemonActionRow::OneMoveBattlePokemonActionRow() {}

void OneMoveBattlePokemonActionRow::load_json(const JsonValue& json){
    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        return;
    }
    const std::string* str = obj->get_string("Style");
    if (str != nullptr){
        const auto iter = MoveStyle_MAP.find(*str);
        if (iter != MoveStyle_MAP.end()){
            style = iter->second;
        }
    }
}

JsonValue OneMoveBattlePokemonActionRow::to_json() const{
    JsonObject obj;
    obj["Style"] = MoveStyle_NAMES[(size_t)style];
    return obj;
}

std::unique_ptr<EditableTableRow> OneMoveBattlePokemonActionRow::clone() const{
    return std::unique_ptr<EditableTableRow>(new OneMoveBattlePokemonActionRow(*this));
}

std::vector<QWidget*> OneMoveBattlePokemonActionRow::make_widgets(QWidget& parent){
    std::vector<QWidget*> widgets;
    widgets.emplace_back(make_enum_table_cell(parent, MoveStyle_MAP.size(), MoveStyle_NAMES, style));
    return widgets;
}



std::vector<std::string> OneMoveBattlePokemonActionTableFactory::make_header() const{
    return std::vector<std::string>{
        "Move Style",
    };
}

std::unique_ptr<EditableTableRow> OneMoveBattlePokemonActionTableFactory::make_row() const{
    return std::unique_ptr<EditableTableRow>(new OneMoveBattlePokemonActionRow());
}




std::vector<std::unique_ptr<EditableTableRow>> OneMoveBattlePokemonActionTable::make_defaults() const{
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(std::make_unique<OneMoveBattlePokemonActionRow>());
    return ret;
}

OneMoveBattlePokemonActionTable::OneMoveBattlePokemonActionTable()
    : m_table(
        "<b>" + STRING_POKEMON + " Action Table:</b><br>"
        "Set what move style to use for each " + STRING_POKEMON + " to grind against a Magikarp. "
        "Each row is the action for one " + STRING_POKEMON + ". "
        "The table follows the order that " + STRING_POKEMON + " are sent to battle.",
        m_factory, make_defaults()
    )
{}

void OneMoveBattlePokemonActionTable::load_json(const JsonValue& json){
    m_table.load_json(json);
}

JsonValue OneMoveBattlePokemonActionTable::to_json() const{
    return m_table.to_json();
}

void OneMoveBattlePokemonActionTable::restore_defaults(){
    m_table.restore_defaults();
}

ConfigWidget* OneMoveBattlePokemonActionTable::make_ui(QWidget& parent){
    return m_table.make_ui(parent);
}

MoveStyle OneMoveBattlePokemonActionTable::get_style(size_t pokemon){
    const OneMoveBattlePokemonActionRow& action = static_cast<const OneMoveBattlePokemonActionRow&>(m_table[pokemon]);
    return action.style;
}


const std::string MoveIndex_NAMES[] = {
    "First move",
    "Second move",
    "Third move",
    "Fourth move",
};

void MoveGrinderActionRow::load_json(const JsonValue& json){
    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        return;
    }
    obj->read_integer(pokemon_index, "PokemonIndex", 0, 5);
    obj->read_integer(move_index, "MoveIndex", 0, 3);
    {
        const std::string* str = obj->get_string("Style");
        if (str != nullptr){
            const auto iter = MoveStyle_MAP.find(*str);
            if (iter != MoveStyle_MAP.end()) {
                style = iter->second;
            }
        }
    }
    obj->read_integer(attempts, "Attempts", 0, 65535);
}

JsonValue MoveGrinderActionRow::to_json() const{
    JsonObject obj;
    obj["PokemonIndex"] = static_cast<uint16_t>(pokemon_index);
    obj["MoveIndex"] = static_cast<uint16_t>(move_index);
    obj["Style"] = MoveStyle_NAMES[(size_t)style];
    obj["Attempts"] = attempts;
    return obj;
}

std::unique_ptr<EditableTableRow> MoveGrinderActionRow::clone() const{
    return std::unique_ptr<EditableTableRow>(new MoveGrinderActionRow(*this));
}

std::vector<QWidget*> MoveGrinderActionRow::make_widgets(QWidget& parent){
    const std::string PokemonIndex_NAMES[] = {
        "First " + STRING_POKEMON,
        "Second " + STRING_POKEMON,
        "Third " + STRING_POKEMON,
        "Fourth " + STRING_POKEMON,
    };

    std::vector<QWidget*> widgets;
    widgets.emplace_back(make_enum_table_cell(parent, 4, PokemonIndex_NAMES, pokemon_index));
    widgets.emplace_back(make_enum_table_cell(parent, 4, MoveIndex_NAMES, move_index));
    widgets.emplace_back(make_enum_table_cell(parent, MoveStyle_MAP.size(), MoveStyle_NAMES, style));
    widgets.emplace_back(make_integer_table_cell(parent, attempts));
    return widgets;
}

std::vector<std::string> MoveGrinderActionTableFactory::make_header() const{
    return std::vector<std::string>{
        "Pokemon index",
        "Move index",
        "Move Style",
        "Move Attempts",
    };
}

std::unique_ptr<EditableTableRow> MoveGrinderActionTableFactory::make_row() const{
    return std::unique_ptr<EditableTableRow>(new MoveGrinderActionRow());
}

MoveGrinderActionTable::MoveGrinderActionTable()
    : m_table("For every move you want to perform, input the style and the number of attemps you want to achieve.", m_factory)
{}

Move MoveGrinderActionTable::get_move(size_t pokemon, size_t move) const{
    // Pokemon index 4 is gonna be Arceus with powerful moves, just use them with normal style and hope you'll win the battle
    if (pokemon == 4){
        return {MoveStyle::NoStyle, std::numeric_limits<decltype(Move::attempts)>::max()};

    }
    for (size_t i = 0; i < m_table.size(); ++i){
        const MoveGrinderActionRow& action = static_cast<const MoveGrinderActionRow&>(m_table[i]);
        if (action.pokemon_index != pokemon){
            continue;
        }
        if (action.move_index != move){
            continue;
        }
        return {action.style , action.attempts};
    }
    return {MoveStyle::NoStyle, 0};
}

void MoveGrinderActionTable::load_json(const JsonValue& json) {
    m_table.load_json(json);
}

JsonValue MoveGrinderActionTable::to_json() const {
    return m_table.to_json();
}

ConfigWidget* MoveGrinderActionTable::make_ui(QWidget& parent) {
    return m_table.make_ui(parent);
}

}
}
}
