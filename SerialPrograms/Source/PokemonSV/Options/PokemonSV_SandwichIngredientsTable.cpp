/*  Sandwich Ingredients Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "PokemonSV/Resources/PokemonSV_Ingredients.h"
#include "PokemonSV_SandwichIngredientsTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



SandwichIngredientsTableRow::SandwichIngredientsTableRow()
    : item("lettuce")
{
    PA_ADD_OPTION(item);
}
std::unique_ptr<EditableTableRow> SandwichIngredientsTableRow::clone() const {
    std::unique_ptr<SandwichIngredientsTableRow> ret(new SandwichIngredientsTableRow());
    ret->item.set_by_index(item.index());
    return ret;
}

SandwichIngredientsTable::SandwichIngredientsTable(std::string label)
    : EditableTableOption_t<SandwichIngredientsTableRow>(
        std::move(label),
        LockMode::LOCK_WHILE_RUNNING,
        make_defaults()
        )
{}


std::vector<std::string> SandwichIngredientsTable::make_header() const {
    return std::vector<std::string>{
        "Ingredient/Condiment",
    };
}

std::vector<std::unique_ptr<EditableTableRow>> SandwichIngredientsTable::make_defaults() {
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(std::make_unique<SandwichIngredientsTableRow>());
    return ret;
}



}
}
}
