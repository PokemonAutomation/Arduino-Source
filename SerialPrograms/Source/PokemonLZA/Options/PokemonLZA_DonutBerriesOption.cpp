/*  Donut Berries Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Logging/Logger.h"
#include "PokemonLZA/Resources/PokemonLZA_DonutBerries.h"
#include "PokemonLZA_DonutBerriesOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

StringSelectDatabase make_donut_berries_database(){
    StringSelectDatabase ret;
    for (const auto& slug : DONUT_BERRIES_SLUGS()){
        const DonutBerries& data = get_berry_name(slug);
        const SpriteDatabase::Sprite* sprite = DONUT_BERRIES_DATABASE().get_nothrow(slug);
        if (sprite == nullptr){
            ret.add_entry(StringSelectEntry(slug, data.display_name()));
            global_logger_tagged().log("Missing sprite for: " + slug, COLOR_RED);
        }else{
            ret.add_entry(StringSelectEntry(slug, data.display_name(), sprite->icon));
        }
    }
    return ret;
}
const StringSelectDatabase& DONUT_BERRY_DATABASE(){
    static StringSelectDatabase database = make_donut_berries_database();
    return database;
}


DonutBerriesTableCell::DonutBerriesTableCell(
    const std::string& default_slug
)
    : StringSelectCell(
        DONUT_BERRY_DATABASE(),
        LockMode::LOCK_WHILE_RUNNING,
        default_slug
    )
{}

DonutBerriesTableRow::DonutBerriesTableRow(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , berry("hyper-cheri-berry")
{
    PA_ADD_OPTION(berry);
}
std::unique_ptr<EditableTableRow> DonutBerriesTableRow::clone() const{
    std::unique_ptr<DonutBerriesTableRow> ret(new DonutBerriesTableRow(parent()));
    ret->berry.set_by_index(berry.index());
    return ret;
}

DonutBerriesTable::DonutBerriesTable(std::string label)
    : EditableTableOption_t<DonutBerriesTableRow>(
        std::move(label),
        LockMode::LOCK_WHILE_RUNNING,
        make_defaults()
        )
{}


std::vector<std::string> DonutBerriesTable::make_header() const{
    return std::vector<std::string>{
        "Berry",
    };
}

std::vector<std::unique_ptr<EditableTableRow>> DonutBerriesTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(std::make_unique<DonutBerriesTableRow>(*this));
    return ret;
}



}
}
}
