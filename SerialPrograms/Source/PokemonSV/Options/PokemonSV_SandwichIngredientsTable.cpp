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
        LockWhileRunning::LOCKED,
        make_defaults()
        )
{}


bool SandwichIngredientsTable::find_item(const std::string& item_slug) const {
    std::vector<std::unique_ptr<SandwichIngredientsTableRow>> table = copy_snapshot();
    for (const std::unique_ptr<SandwichIngredientsTableRow>& row : table) {
        if (row->item.slug() == item_slug) {
            return true;
        }
    }
    return false;
}

std::vector<std::string> SandwichIngredientsTable::selected_items() const {
    std::vector<std::unique_ptr<SandwichIngredientsTableRow>> table = copy_snapshot();
    std::vector<std::string> slugs;
    for (const std::unique_ptr<SandwichIngredientsTableRow>& row : table) {
        slugs.emplace_back(row->item.slug());
    }
    return slugs;
}


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
