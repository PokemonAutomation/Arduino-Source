/*  Player List
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Pokemon/Inference/Pokemon_NameReader.h"
#include "PokemonSV_PlayerList.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{


PlayerListRow::PlayerListRow()
    : language(
        Pokemon::PokemonNameReader::instance().languages(),
        LockWhileRunning::UNLOCKED
    )
    , name(false, LockWhileRunning::UNLOCKED, "", "Ash")
    , log10p(LockWhileRunning::UNLOCKED, -2.0, -10, 0)
{
    PA_ADD_OPTION(language);
    PA_ADD_OPTION(name);
    PA_ADD_OPTION(log10p);
}
std::unique_ptr<EditableTableRow> PlayerListRow::clone() const{
    std::unique_ptr<PlayerListRow> ret(new PlayerListRow());
    ret->language.set(language);
    ret->name.set(name);
    ret->log10p.set(log10p);
    return ret;
}
PlayerListRowSnapshot PlayerListRow::snapshot() const{
    PlayerListRowSnapshot entry;
    entry.language = language;
    entry.name = name;
    entry.log10p = log10p;
    return entry;
}




PlayerListTable::PlayerListTable(std::string label, LockWhileRunning lock_while_running)
    : EditableTableOption_t<PlayerListRow>(
        std::move(label),
        lock_while_running,
        make_defaults()
    )
{}


std::vector<std::string> PlayerListTable::make_header() const{
    return std::vector<std::string>{
        "Language", "Player Name", "Match Threshold (log10p)"
    };
}

std::vector<std::unique_ptr<EditableTableRow>> PlayerListTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
    ret.emplace_back(std::make_unique<PlayerListRow>());
    return ret;
}




}
}
}
