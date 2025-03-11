/*  Box Sorter Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonHome_BoxSortingTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


const EnumDropdownDatabase<BoxSortingSortType>& BallType_Database(){
    static const EnumDropdownDatabase<BoxSortingSortType> database({
        {BoxSortingSortType::NationalDexNo,     "dex",     "National Dex Number"},
        {BoxSortingSortType::Shiny,    "shiny",    "Shiny"},
        {BoxSortingSortType::Gigantamax,    "gigantamax",    "Gigantamax"},
        {BoxSortingSortType::Ball_Slug,    "ball_slug",    "Ball Type"},
        {BoxSortingSortType::Gender,    "gender",    "Gender (Male, Female, Genderless)"},
    });
    return database;
}



BoxSortingRow::BoxSortingRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , sort_type(BallType_Database(), LockMode::LOCK_WHILE_RUNNING, BoxSortingSortType::NationalDexNo)
    , reverse(LockMode::LOCK_WHILE_RUNNING, false)
{
    PA_ADD_OPTION(sort_type);
    PA_ADD_OPTION(reverse);
}
std::unique_ptr<EditableTableRow> BoxSortingRow::clone() const{
    std::unique_ptr<BoxSortingRow> ret(new BoxSortingRow(parent()));
    ret->sort_type.set_value(sort_type.current_value());
    ret->reverse = reverse.current_value();
    return ret;
}


BoxSortingTable::BoxSortingTable(std::string label)
    : EditableTableOption_t<BoxSortingRow>(
        std::move(label),
        LockMode::LOCK_WHILE_RUNNING,
        make_defaults()
    )
{}


std::vector<BoxSortingSelection> BoxSortingTable::preferences() const{
    std::vector<std::unique_ptr<BoxSortingRow>> table = copy_snapshot();
    std::vector<BoxSortingSelection> selections;
    for (const std::unique_ptr<BoxSortingRow>& row : table){
        BoxSortingSelection selection;
        selection.sort_type = BoxSortingSortType(row->sort_type.current_value());
        selection.reverse = row->reverse.current_value();

        selections.emplace_back(selection);
    }
    return selections;
}




std::vector<std::string> BoxSortingTable::make_header() const{
    return std::vector<std::string>{
        "Criteria", "Reverse",
    };
}

std::vector<std::unique_ptr<EditableTableRow>> BoxSortingTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(std::make_unique<BoxSortingRow>(*this));
    return ret;
}

















}
}
}
