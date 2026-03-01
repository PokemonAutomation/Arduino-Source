/*  Box Sorter Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Pokemon_BoxSortingTable.h"

namespace PokemonAutomation{
namespace Pokemon{


const EnumDropdownDatabase<SortingRuleType>& SortingRuleType_Database(){
    static const EnumDropdownDatabase<SortingRuleType> database({
        {SortingRuleType::DexNo,     "dex",     "Dex Number"},
        {SortingRuleType::Shiny,    "shiny",    "Shiny"},
        {SortingRuleType::Gigantamax,    "gigantamax",    "Gigantamax"},
        {SortingRuleType::Alpha, "alpha", "Alpha"},
        {SortingRuleType::Ball_Slug,    "ball_slug",    "Ball Type"},
        {SortingRuleType::Gender,    "gender",    "Gender (Male, Female, Genderless)"},
    });
    return database;
}



BoxSortingRow::BoxSortingRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , sort_type(
        static_cast<BoxSortingTable&>(parent_table).database(),
        LockMode::LOCK_WHILE_RUNNING,
        SortingRuleType::DexNo
    )
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
    : BoxSortingTableDatabaseHelper(nullptr)
    , EditableTableOption_t<BoxSortingRow>(
        std::move(label),
        LockMode::LOCK_WHILE_RUNNING,
        make_defaults()
    )
{}

BoxSortingTable::BoxSortingTable(std::string label, const std::vector<SortingRuleType>& allowed_rules)
    : BoxSortingTableDatabaseHelper([&allowed_rules](){
        // Build custom database from allowed rules
        const EnumDropdownDatabase<SortingRuleType>& full_db = SortingRuleType_Database();
        auto custom_db = std::make_shared<EnumDropdownDatabase<SortingRuleType>>();

        for (SortingRuleType rule : allowed_rules){
            const EnumEntry* entry = full_db.find(rule);
            if (entry != nullptr){
                custom_db->add(rule, entry->slug, entry->display, entry->enabled);
            }
        }

        return custom_db;
    }())
    , EditableTableOption_t<BoxSortingRow>(
        std::move(label),
        LockMode::LOCK_WHILE_RUNNING,
        make_defaults()
    )
{}


std::vector<SortingRule> BoxSortingTable::preferences() const{
    std::vector<std::unique_ptr<BoxSortingRow>> table = copy_snapshot();
    std::vector<SortingRule> selections;
    for (const std::unique_ptr<BoxSortingRow>& row : table){
        SortingRule selection;
        selection.sort_type = SortingRuleType(row->sort_type.current_value());
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

const EnumDropdownDatabase<SortingRuleType>& BoxSortingTable::database() const{
    if (m_database){
        return *m_database;
    }
    return SortingRuleType_Database();
}









}
}
