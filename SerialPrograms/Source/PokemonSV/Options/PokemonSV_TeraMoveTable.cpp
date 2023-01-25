/*  Tera Move Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSV_TeraMoveTable.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


const EnumDatabase<TeraMoveType>& tera_move_enum_database(){
    static EnumDatabase<TeraMoveType> database{
        {TeraMoveType::Wait,            "wait",             "Wait for X seconds."},
        {TeraMoveType::Move1,           "move1",            "Move 1"},
        {TeraMoveType::Move2,           "move2",            "Move 2"},
        {TeraMoveType::Move3,           "move3",            "Move 3"},
        {TeraMoveType::Move4,           "move4",            "Move 4"},
        {TeraMoveType::Cheer_AllOut,    "cheer-allout",     "Cheer - All Out"},
        {TeraMoveType::Cheer_HangTough, "cheer-hangtough",  "Cheer - Hang Tough"},
        {TeraMoveType::Cheer_HealUp,    "cheer-healup",     "Cheer - Heal Up"},
    };
    return database;
}
const EnumDatabase<TeraTarget>& tera_target_enum_database(){
    static EnumDatabase<TeraTarget> database{
        {TeraTarget::Opponent,  "opponent", "Opponent"},
        {TeraTarget::Player0,   "player0",  "Player 0 (yourself)"},
        {TeraTarget::Player1,   "player1",  "Player 1 (right of yourself)"},
        {TeraTarget::Player2,   "player2",  "Player 2 (2nd from the right)"},
        {TeraTarget::Player3,   "player3",  "Player 3 (rightmost player)"},
    };
    return database;
}


std::string TeraMoveEntry::to_str() const{
    switch (type){
    case TeraMoveType::Wait:
        return "Wait for " + std::to_string(seconds) + " second(s).";
    case TeraMoveType::Move1:
    case TeraMoveType::Move2:
    case TeraMoveType::Move3:
    case TeraMoveType::Move4:{
        int slot = (int)type - (int)TeraMoveType::Move1;
        std::string str = "Move " + std::to_string(slot + 1) + " on ";
        if (target == TeraTarget::Opponent){
            return str + "opponent.";
        }else{
            slot = (int)target - (int)TeraTarget::Player0;
            return "player " + std::to_string(slot) + ".";
        }
    }
    case TeraMoveType::Cheer_AllOut:
        return "Cheer - All Out";
    case TeraMoveType::Cheer_HangTough:
        return "Cheer - Hang Tough";
    case TeraMoveType::Cheer_HealUp:
        return "Cheer - Heal Up";
    }
    return "(Invalid Move)";
}




TeraMoveTableRow::~TeraMoveTableRow(){
    type.remove_listener(*this);
}
TeraMoveTableRow::TeraMoveTableRow()
    : type(tera_move_enum_database(), LockWhileRunning::UNLOCKED, TeraMoveType::Move1)
    , seconds(LockWhileRunning::UNLOCKED, 5)
    , target(tera_target_enum_database(), LockWhileRunning::UNLOCKED, TeraTarget::Opponent)
    , notes(false, LockWhileRunning::UNLOCKED, "", "(e.g. Screech, Belly Drum)")
{
    PA_ADD_OPTION(type);
    PA_ADD_OPTION(seconds);
    PA_ADD_OPTION(target);
    PA_ADD_OPTION(notes);

    TeraMoveTableRow::value_changed();
    type.add_listener(*this);
}
std::unique_ptr<EditableTableRow> TeraMoveTableRow::clone() const{
    std::unique_ptr<TeraMoveTableRow> ret(new TeraMoveTableRow());
    ret->type.set(type);
    ret->seconds.set(seconds);
    ret->target.set(target);
    ret->notes.set(notes);
    return ret;
}
TeraMoveEntry TeraMoveTableRow::snapshot() const{
    return TeraMoveEntry{type, seconds, target};
}
void TeraMoveTableRow::value_changed(){
    TeraMoveType type = this->type;
//    cout << "Enter: type = " << (int)type << endl;

    seconds.set_visibility(
        type == TeraMoveType::Wait
        ? ConfigOptionState::ENABLED
        : ConfigOptionState::HIDDEN
    );

    bool is_move =
        type == TeraMoveType::Move1 ||
        type == TeraMoveType::Move2 ||
        type == TeraMoveType::Move3 ||
        type == TeraMoveType::Move4;

    target.set_visibility(
        is_move
        ? ConfigOptionState::ENABLED
        : ConfigOptionState::HIDDEN
    );

//    cout << "Exit: type = " << (int)type << endl;

}





TeraMoveTable::TeraMoveTable()
    : EditableTableOption_t<TeraMoveTableRow>(
        "<b>Move Table:</b><br>"
        "Run this sequence of moves. When the end of the table is reached, "
        "the last entry will be repeated until the battle is won or lost. "
        "Changes to this table take effect on the next battle.",
        LockWhileRunning::UNLOCKED,
        make_defaults()
    )
{}

std::vector<TeraMoveEntry> TeraMoveTable::snapshot(){
    return EditableTableOption_t<TeraMoveTableRow>::snapshot<TeraMoveEntry>();
}
std::vector<std::string> TeraMoveTable::make_header() const{
    return {
        "Move",
        "Wait (seconds)",
        "Target",
        "Notes",
    };
}
std::vector<std::unique_ptr<EditableTableRow>> TeraMoveTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(new TeraMoveTableRow());
    return ret;
}








}
}
}
