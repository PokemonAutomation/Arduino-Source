/*  Berry Selector
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Pokemon/Resources/Pokemon_BerryNames.h"
#include "PokemonBDSP_BerryTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{

using namespace Pokemon;



BerrySelectorRow2::BerrySelectorRow2(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , berry("cheri-berry")
{
    PA_ADD_OPTION(berry);
}
std::unique_ptr<EditableTableRow> BerrySelectorRow2::clone() const{
    std::unique_ptr<BerrySelectorRow2> ret(new BerrySelectorRow2(parent()));
    ret->berry.set_by_index(berry.index());
    return ret;
}





BerryTable::BerryTable(std::string label)
    : EditableTableOption_t<BerrySelectorRow2>(
        std::move(label),
        LockMode::LOCK_WHILE_RUNNING,
        make_defaults()
    )
{}



bool BerryTable::find_berry(const std::string& berry_slug) const{
    std::vector<std::unique_ptr<BerrySelectorRow2>> table = copy_snapshot();
    for (const std::unique_ptr<BerrySelectorRow2>& row : table){
        if (row->berry.slug() == berry_slug){
            return true;
        }
    }
    return false;
}

std::vector<std::string> BerryTable::selected_berries() const{
    std::vector<std::unique_ptr<BerrySelectorRow2>> table = copy_snapshot();
    std::vector<std::string> slugs;
    for (const std::unique_ptr<BerrySelectorRow2>& row : table){
        slugs.emplace_back(row->berry.slug());
    }
    return slugs;
}




std::vector<std::string> BerryTable::make_header() const{
    return std::vector<std::string>{
        "Berry",
    };
}

std::vector<std::unique_ptr<EditableTableRow>> BerryTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(std::make_unique<BerrySelectorRow2>(*this));
    return ret;
}

















}
}
}
