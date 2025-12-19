/*  Box Sorting Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Pokemon_BoxSortingTable_H
#define PokemonAutomation_Pokemon_BoxSortingTable_H

#include "Common/Cpp/Options/EditableTableOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{
namespace Pokemon{

enum class SortingRuleType
{
    DexNo,
    Shiny,
    Gigantamax,
    Alpha,
    Ball_Slug,
    Gender,
};

struct SortingRule
{
    SortingRuleType sort_type = SortingRuleType::DexNo;
    bool reverse = false;
};

class BoxSortingRow : public EditableTableRow{
public:
    BoxSortingRow(EditableTableOption& parent_table);
    virtual std::unique_ptr<EditableTableRow> clone() const;

public:
    EnumDropdownCell<SortingRuleType> sort_type;
    BooleanCheckBoxCell reverse;
};


// Helper base class to ensure database is initialized before table construction
class BoxSortingTableDatabaseHelper{
protected:
    BoxSortingTableDatabaseHelper(std::shared_ptr<const EnumDropdownDatabase<SortingRuleType>> db = nullptr)
        : m_database(std::move(db))
    {}

    std::shared_ptr<const EnumDropdownDatabase<SortingRuleType>> m_database;
};


class BoxSortingTable : private BoxSortingTableDatabaseHelper, public EditableTableOption_t<BoxSortingRow>{
public:
    BoxSortingTable(std::string label);
    BoxSortingTable(std::string label, const std::vector<SortingRuleType>& allowed_rules);

    std::vector<SortingRule> preferences() const;

    virtual std::vector<std::string> make_header() const;

    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();

    const EnumDropdownDatabase<SortingRuleType>& database() const;
};





}
}
#endif
