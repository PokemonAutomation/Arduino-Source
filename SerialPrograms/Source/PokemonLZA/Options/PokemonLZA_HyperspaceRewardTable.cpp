/*  Hyperspace Reward Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonLZA/Resources/PokemonLZA_HyperspaceRewardNames.h"
#include "PokemonLZA_HyperspaceRewardTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

HyperspaceRewardRow::HyperspaceRewardRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , item("bottle-cap")
{
    PA_ADD_OPTION(item);
}
std::unique_ptr<EditableTableRow> HyperspaceRewardRow::clone() const{
    std::unique_ptr<HyperspaceRewardRow> ret(new HyperspaceRewardRow(parent()));
    ret->item.set_by_index(item.index());
    return ret;
}

HyperspaceRewardTable::HyperspaceRewardTable(std::string label)
    : EditableTableOption_t<HyperspaceRewardRow>(
        std::move(label),
        LockMode::LOCK_WHILE_RUNNING,
        make_defaults()
    )
{}

bool HyperspaceRewardTable::find_item(const std::string& item_slug) const{
    std::vector<std::unique_ptr<HyperspaceRewardRow>> table = copy_snapshot();
    for (const std::unique_ptr<HyperspaceRewardRow>& row : table){
        if (row->item.slug() == item_slug){
            return true;
        }
    }
    return false;
}

std::vector<std::string> HyperspaceRewardTable::selected_items() const{
    std::vector<std::unique_ptr<HyperspaceRewardRow>> table = copy_snapshot();
    std::vector<std::string> slugs;
    for (const std::unique_ptr<HyperspaceRewardRow>& row : table){
        slugs.emplace_back(row->item.slug());
    }
    return slugs;
}


std::vector<std::string> HyperspaceRewardTable::make_header() const{
    return std::vector<std::string>{
        "Item",
    };
}

std::vector<std::unique_ptr<EditableTableRow>> HyperspaceRewardTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(std::make_unique<HyperspaceRewardRow>(*this));
    return ret;
}






}
}
}
