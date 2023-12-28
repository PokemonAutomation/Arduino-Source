/*  Single Battle Move Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSV_SinglesMoveTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


const EnumDatabase<SinglesMoveType>& singles_move_enum_database(){
    static EnumDatabase<SinglesMoveType> database{
        {SinglesMoveType::Move1,    "move1",    "Move 1"},
        {SinglesMoveType::Move2,    "move2",    "Move 2"},
        {SinglesMoveType::Move3,    "move3",    "Move 3"},
        {SinglesMoveType::Move4,    "move4",    "Move 4"},
        {SinglesMoveType::Run,      "run",      "Run"},
    };
    return database;
}

std::string SinglesMoveEntry::to_str() const{
    switch (type){
    case SinglesMoveType::Move1:
    case SinglesMoveType::Move2:
    case SinglesMoveType::Move3:
    case SinglesMoveType::Move4:{
        int slot = (int)type - (int)SinglesMoveType::Move1;
        std::string str = "Move " + std::to_string(slot + 1);
        if (terastallize){
            str += " (terastallize)";
        }
        str += ".";
        return str;
    }
    case SinglesMoveType::Run:
        return "Run Away";
    }
    return "(Invalid Move)";
}



SinglesMoveTableRow::~SinglesMoveTableRow(){
    type.remove_listener(*this);
}
SinglesMoveTableRow::SinglesMoveTableRow()
    : type(singles_move_enum_database(), LockMode::UNLOCK_WHILE_RUNNING, SinglesMoveType::Move1)
    , terastallize(LockMode::UNLOCK_WHILE_RUNNING, false)
    , notes(false, LockMode::UNLOCK_WHILE_RUNNING, "", "(e.g. Screech, Belly Drum)")
{
    PA_ADD_OPTION(type);
    PA_ADD_OPTION(terastallize);
    PA_ADD_OPTION(notes);

    SinglesMoveTableRow::value_changed();
    type.add_listener(*this);
}
std::unique_ptr<EditableTableRow> SinglesMoveTableRow::clone() const{
    std::unique_ptr<SinglesMoveTableRow> ret(new SinglesMoveTableRow());
    ret->type.set(type);
    ret->terastallize = (bool)terastallize;
    ret->notes.set(notes);
    return ret;
}
SinglesMoveEntry SinglesMoveTableRow::snapshot() const{
    return SinglesMoveEntry{type, terastallize};
}
void SinglesMoveTableRow::value_changed(){

}





SinglesMoveTable::SinglesMoveTable(std::string label)
    : EditableTableOption_t<SinglesMoveTableRow>(
        std::move(label),
#if 0
        "<b>Move Table:</b><br>"
        "Run this sequence of moves. When the end of the table is reached, "
        "the last entry will be repeated until the battle is won or your " + Pokemon::STRING_POKEMON +" faints."
        "Changes to this table take effect on the next battle.",
#endif
        LockMode::UNLOCK_WHILE_RUNNING,
        make_defaults()
    )
{}

std::vector<SinglesMoveEntry> SinglesMoveTable::snapshot(){
    return EditableTableOption_t<SinglesMoveTableRow>::snapshot<SinglesMoveEntry>();
}
std::vector<std::string> SinglesMoveTable::make_header() const{
    return {
        "Move",
        "Terastallize",
        "Notes",
    };
}
std::vector<std::unique_ptr<EditableTableRow>> SinglesMoveTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(new SinglesMoveTableRow());
    return ret;
}








}
}
}
