/*  Auction Item Names
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_AuctionItemNames_H
#define PokemonAutomation_PokemonSV_AuctionItemNames_H

#include <string>
#include <vector>

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


class AuctionItemNames{
public:
    const std::string& display_name() const{ return m_display_name; }

private:
    friend struct AuctionItemNameDatabase;

    std::string m_display_name;
};


const AuctionItemNames& get_auction_item_name(const std::string& slug);
const std::string& parse_auction_item_name(const std::string& display_name);
const std::string& parse_auction_item_name_nothrow(const std::string& display_name);

const std::vector<std::string>& AUCTION_ITEM_SLUGS();


}
}
}
#endif
