/*  Auction item selector, UI component to select multiple items from auctions
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AuctionItemSelector_H
#define PokemonAutomation_PokemonSV_AuctionItemSelector_H

#include "Common/Cpp/Options/EditableTableOption.h"
#include "PokemonSV_AuctionItemSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class AuctionItemSelectorRow : public EditableTableRow{
public:
    AuctionItemSelectorRow(EditableTableOption& parent_table);
    virtual std::unique_ptr<EditableTableRow> clone() const override;

public:
    AuctionItemSelectCell item;
};



class AuctionItemTable : public EditableTableOption_t<AuctionItemSelectorRow>{
public:
    AuctionItemTable(std::string label);


    // Whether item_slug is among the selected items.
    bool find_item(const std::string& item_slug) const;
    // Return the auction item slugs that the user has selected via the auction item table UI.
    std::vector<std::string> selected_items() const;

    virtual std::vector<std::string> make_header() const override;

    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();
};





}
}
}
#endif
