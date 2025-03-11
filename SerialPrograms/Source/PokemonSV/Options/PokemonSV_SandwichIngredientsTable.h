/*  Sandwich Ingredients Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_SandwichIngredientsTable_H
#define PokemonAutomation_PokemonSV_SandwichIngredientsTable_H

#include "Common/Cpp/Options/EditableTableOption.h"
#include "PokemonSV_SandwichIngredientsOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class SandwichIngredientsTableRow : public EditableTableRow{
public:
    SandwichIngredientsTableRow(EditableTableOption& parent_table);
    virtual std::unique_ptr<EditableTableRow> clone() const override;

public:
    SandwichIngredientsTableCell item;
};


class SandwichIngredientsTable : public EditableTableOption_t<SandwichIngredientsTableRow>{
public:
    SandwichIngredientsTable(std::string label);

    virtual std::vector<std::string> make_header() const override;

    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();
};



}
}
}
#endif
