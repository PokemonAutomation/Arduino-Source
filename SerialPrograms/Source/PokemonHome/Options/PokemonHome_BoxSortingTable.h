/*  Cram-o-matic Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonHome_BoxSortingTable_H
#define PokemonAutomation_PokemonHome_BoxSortingTable_H

#include "Common/Cpp/Options/EditableTableOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

enum class BoxSortingSortType
{
    NationalDexNo,
    Shiny,
    Gigantamax,
    Ball_Slug,
};

struct BoxSortingSelection
{
    BoxSortingSortType sort_type;
    bool reverse;
};

class BoxSortingRow : public EditableTableRow{
public:
    BoxSortingRow();
    virtual std::unique_ptr<EditableTableRow> clone() const;

public:
    EnumDropdownCell<BoxSortingSortType> sort_type;
    BooleanCheckBoxCell reverse;
};



class BoxSortingTable : public EditableTableOption_t<BoxSortingRow>{
public:
    BoxSortingTable(std::string label);

    std::vector<BoxSortingSelection> preferences() const;

    virtual std::vector<std::string> make_header() const;

    static std::vector<std::unique_ptr<EditableTableRow>> make_defaults();
};





}
}
}
#endif
