/*  Battle Move Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSV_BattleMoveTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

const EnumDropdownDatabase<BattleMoveType>& Battle_move_enum_database(){
    static EnumDropdownDatabase<BattleMoveType> database{
        {BattleMoveType::Move1,           "move1",            "Move 1"},
        {BattleMoveType::Move2,           "move2",            "Move 2"},
        {BattleMoveType::Move3,           "move3",            "Move 3"},
        {BattleMoveType::Move4,           "move4",            "Move 4"},
        //{BattleMoveType::tera,           "tera",            "Tera"},
    };
    return database;
}

BattleMoveTableRow::BattleMoveTableRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , type(Battle_move_enum_database(), LockMode::UNLOCK_WHILE_RUNNING, BattleMoveType::Move1)
    , notes(false, LockMode::UNLOCK_WHILE_RUNNING, "", "(e.g. False Swipe, Thunder Wave)")
{
    PA_ADD_OPTION(type);
    PA_ADD_OPTION(notes);
}
std::unique_ptr<EditableTableRow> BattleMoveTableRow::clone() const{
    std::unique_ptr<BattleMoveTableRow> ret(new BattleMoveTableRow(parent()));
    ret->type.set(type);
    ret->notes.set(notes);
    return ret;
}

BattleMoveTable::BattleMoveTable()
    : EditableTableOption_t<BattleMoveTableRow>(
        "<b>Move Table:</b><br>"
        "Run this sequence of moves for your lead Pokemon only. "
        "If your lead faints or the end of the table is reached, the program will switch to throwing the selected ball. ",
        LockMode::LOCK_WHILE_RUNNING,
        make_defaults()
    )
{}

std::vector<std::string> BattleMoveTable::make_header() const{
    return {
        "Move",
        "Notes",
    };
}
std::vector<std::unique_ptr<EditableTableRow>> BattleMoveTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(new BattleMoveTableRow(*this));
    return ret;
}


}
}
}
