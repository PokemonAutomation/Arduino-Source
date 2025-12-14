/* Hyperspace Reward Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_HyperspaceRewardTable_H
#define PokemonAutomation_PokemonLZA_HyperspaceRewardTable_H

#include "Common/Cpp/Options/EditableTableOption.h"
#include "PokemonLZA_HyperspaceRewardOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



class HyperspaceRewardRow : public EditableTableRow{
public:
    HyperspaceRewardRow(EditableTableOption& parent_table);
    virtual std::unique_ptr<EditableTableRow> clone() const override;

public:
    HyperspaceRewardCell item;
};



class HyperspaceRewardTable : public EditableTableOption_t<HyperspaceRewardRow>{
public:
    HyperspaceRewardTable(std::string label);


    // Whether item_slug is among the selected items.
    bool find_item(const std::string& item_slug) const;
    std::vector<std::string> selected_items() const;

    virtual std::vector<std::string> make_header() const override;

    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();
};





}
}
}
#endif
