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
    NationalDexNo,
    Shiny,
    Gigantamax,
    Alpha,
    Ball_Slug,
    Gender,
};

struct SortingRule
{
    SortingRuleType sort_type = SortingRuleType::NationalDexNo;
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



class BoxSortingTable : public EditableTableOption_t<BoxSortingRow>{
public:
    BoxSortingTable(std::string label);

    std::vector<SortingRule> preferences() const;

    virtual std::vector<std::string> make_header() const;

    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();
};





}
}
#endif
