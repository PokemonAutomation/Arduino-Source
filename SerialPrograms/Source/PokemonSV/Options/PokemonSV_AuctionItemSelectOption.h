/*  Auction Item Selector, UI component to select multiple items from auctions
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AuctionItemSelectOption_H
#define PokemonAutomation_PokemonSV_AuctionItemSelectOption_H

#include "CommonTools/Options/StringSelectOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



class AuctionItemSelectCell : public StringSelectCell{
public:
    AuctionItemSelectCell(const std::string& default_slug);
};




}
}
}
#endif
