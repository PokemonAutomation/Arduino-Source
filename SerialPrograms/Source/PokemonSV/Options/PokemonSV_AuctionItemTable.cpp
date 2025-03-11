/*  Auction item selector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSV/Resources/PokemonSV_AuctionItemNames.h"
#include "PokemonSV_AuctionItemTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



AuctionItemSelectorRow::AuctionItemSelectorRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , item("level-ball")
{
    PA_ADD_OPTION(item);
}
std::unique_ptr<EditableTableRow> AuctionItemSelectorRow::clone() const{
    std::unique_ptr<AuctionItemSelectorRow> ret(new AuctionItemSelectorRow(parent()));
    ret->item.set_by_index(item.index());
    return ret;
}



AuctionItemTable::AuctionItemTable(std::string label)
    : EditableTableOption_t<AuctionItemSelectorRow>(
        std::move(label),
        LockMode::LOCK_WHILE_RUNNING,
        make_defaults()
    )
{}



bool AuctionItemTable::find_item(const std::string& item_slug) const{
    std::vector<std::unique_ptr<AuctionItemSelectorRow>> table = copy_snapshot();
    for (const std::unique_ptr<AuctionItemSelectorRow>& row : table){
        if (row->item.slug() == item_slug){
            return true;
        }
    }
    return false;
}

std::vector<std::string> AuctionItemTable::selected_items() const{
    std::vector<std::unique_ptr<AuctionItemSelectorRow>> table = copy_snapshot();
    std::vector<std::string> slugs;
    for (const std::unique_ptr<AuctionItemSelectorRow>& row : table){
        slugs.emplace_back(row->item.slug());
    }
    return slugs;
}




std::vector<std::string> AuctionItemTable::make_header() const{
    return std::vector<std::string>{
        "Item",
    };
}

std::vector<std::unique_ptr<EditableTableRow>> AuctionItemTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(std::make_unique<AuctionItemSelectorRow>(*this));
    return ret;
}






}
}
}
