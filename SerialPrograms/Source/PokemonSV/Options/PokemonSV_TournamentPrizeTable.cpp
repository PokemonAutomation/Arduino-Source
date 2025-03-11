/*  Tournament Prize Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "PokemonSV/Resources/PokemonSV_TournamentPrizeNames.h"
#include "PokemonSV_TournamentPrizeTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

TournamentPrizeSelectorRow::TournamentPrizeSelectorRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , item("beast-ball")
{
    PA_ADD_OPTION(item);
}
std::unique_ptr<EditableTableRow> TournamentPrizeSelectorRow::clone() const{
    std::unique_ptr<TournamentPrizeSelectorRow> ret(new TournamentPrizeSelectorRow(parent()));
    ret->item.set_by_index(item.index());
    return ret;
}

TournamentPrizeTable::TournamentPrizeTable(std::string label)
    : EditableTableOption_t<TournamentPrizeSelectorRow>(
        std::move(label),
        LockMode::LOCK_WHILE_RUNNING,
        make_defaults()
    )
{}

bool TournamentPrizeTable::find_item(const std::string& item_slug) const{
    std::vector<std::unique_ptr<TournamentPrizeSelectorRow>> table = copy_snapshot();
    for (const std::unique_ptr<TournamentPrizeSelectorRow>& row : table){
        if (row->item.slug() == item_slug){
            return true;
        }
    }
    return false;
}

std::vector<std::string> TournamentPrizeTable::selected_items() const{
    std::vector<std::unique_ptr<TournamentPrizeSelectorRow>> table = copy_snapshot();
    std::vector<std::string> slugs;
    for (const std::unique_ptr<TournamentPrizeSelectorRow>& row : table){
        slugs.emplace_back(row->item.slug());
    }
    return slugs;
}


std::vector<std::string> TournamentPrizeTable::make_header() const{
    return std::vector<std::string>{
        "Item",
    };
}

std::vector<std::unique_ptr<EditableTableRow>> TournamentPrizeTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(std::make_unique<TournamentPrizeSelectorRow>(*this));
    return ret;
}






}
}
}
