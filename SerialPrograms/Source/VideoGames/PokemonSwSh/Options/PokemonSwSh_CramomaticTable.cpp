/*  Cram-o-matic Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSwSh_CramomaticTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


const EnumDropdownDatabase<CramomaticBallType>& BallType_Database(){
    static const EnumDropdownDatabase<CramomaticBallType> database({
        {CramomaticBallType::Poke,     "poke",     "Poké Ball"},
        {CramomaticBallType::Great,    "great",    "Great Ball"},
        {CramomaticBallType::Shop1,    "shop1",    "Shop 1 (Ultra Ball, Net Ball, Dusk Ball, Premier Ball)"},
        {CramomaticBallType::Shop2,    "shop2",    "Shop 2 (Repeat Ball, Dive Ball, Quick Ball, Nest Ball, Heal Ball, Timer Ball, Luxury Ball)"},
        {CramomaticBallType::Apricorn, "apricorn", "Apricorn (Level Ball, Lure Ball, Moon Ball, Friend Ball, Love Ball, Fast Ball, Heavy Ball)"},
        {CramomaticBallType::Safari,   "safari",   "Safari Ball"},
        {CramomaticBallType::Sport,    "sport",    "Sport Ball (uses two different Apricorn colors)"},
    });
    return database;
}



CramomaticRow::CramomaticRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , ball_type(BallType_Database(), LockMode::LOCK_WHILE_RUNNING, CramomaticBallType::Apricorn)
    , is_bonus(LockMode::LOCK_WHILE_RUNNING, false)
    , priority(LockMode::LOCK_WHILE_RUNNING, 0)
{
    PA_ADD_OPTION(ball_type);
    PA_ADD_OPTION(is_bonus);
    PA_ADD_OPTION(priority);
}

CramomaticRow::CramomaticRow(EditableTableOption& parent_table, CramomaticBallType p_ball_type, bool p_is_bonus, uint16_t p_priority)
    : CramomaticRow(parent_table)
{
    ball_type.set(p_ball_type);
    is_bonus = p_is_bonus;
    priority.set(p_priority);
}

std::unique_ptr<EditableTableRow> CramomaticRow::clone() const{
    std::unique_ptr<CramomaticRow> ret(new CramomaticRow(parent()));
    ret->ball_type.set_value(ball_type.current_value());
    ret->is_bonus = is_bonus.current_value();
    ret->priority.set(priority.current_value());
    return ret;
}


CramomaticTable::CramomaticTable(std::string label)
    : EditableTableOption_t<CramomaticRow>(
        std::move(label),
        LockMode::LOCK_WHILE_RUNNING,
        make_defaults()
    )
{}


std::vector<CramomaticSelection> CramomaticTable::selected_balls() const{
    std::vector<std::unique_ptr<CramomaticRow>> table = copy_snapshot();
    std::vector<CramomaticSelection> selections;
    for (const std::unique_ptr<CramomaticRow>& row : table){
        CramomaticSelection selection;
        selection.ball_type = CramomaticBallType(row->ball_type.current_value());
        selection.is_bonus = row->is_bonus.current_value();
        selection.priority = row->priority.current_value();

        selections.emplace_back(selection);
    }
    return selections;
}




std::vector<std::string> CramomaticTable::make_header() const{
    return std::vector<std::string>{
        "Ball", "Only Bonus", "Priority",
    };
}

std::vector<std::unique_ptr<EditableTableRow>> CramomaticTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(std::make_unique<CramomaticRow>(*this, CramomaticBallType::Apricorn, true, (uint16_t)1));
    ret.emplace_back(std::make_unique<CramomaticRow>(*this, CramomaticBallType::Apricorn, false, (uint16_t)0));
    return ret;
}

















}
}
}
