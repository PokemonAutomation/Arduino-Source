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
    : enabled(LockWhileRunning::UNLOCKED, true)
    , language(
        Pokemon::PokemonNameReader::instance().languages(),
        LockWhileRunning::UNLOCKED
    )
    , name(false, LockWhileRunning::UNLOCKED, "", "Ash")
    , log10p(LockWhileRunning::UNLOCKED, -2.5, -10, 0)
    , notes(false, LockWhileRunning::UNLOCKED, "", "Joined Kim's raid 10 times.")
{
    PA_ADD_OPTION(enabled);
    PA_ADD_OPTION(language);
    PA_ADD_OPTION(name);
    PA_ADD_OPTION(log10p);
    PA_ADD_OPTION(notes);
}
std::unique_ptr<EditableTableRow> PlayerListRow::clone() const{
    std::unique_ptr<PlayerListRow> ret(new PlayerListRow());
    ret->enabled = enabled.current_value();
    ret->language.set(language);
    ret->name.set(name);
    ret->log10p.set(log10p);
    ret->notes.set(notes);
    return ret;
}
PlayerListRowSnapshot PlayerListRow::snapshot() const{
    PlayerListRowSnapshot entry;
    entry.enabled = enabled;
    entry.language = language;
    entry.name = name;
    entry.log10p = log10p;
    entry.notes = notes;
    return entry;
}




PlayerListTable::PlayerListTable(
    std::string label,
    LockWhileRunning lock_while_running,
    std::string notes_label
)
    : EditableTableOption_t<PlayerListRow>(
        std::move(label),
        lock_while_running,
        make_defaults()
    )
    , m_notes_label(std::move(notes_label))
{}


std::vector<std::string> PlayerListTable::make_header() const{
    return std::vector<std::string>{
        "Enabled", "Language", "Player Name", "Match Threshold (log10p)", m_notes_label
    };
}

std::vector<std::unique_ptr<EditableTableRow>> PlayerListTable::make_defaults(){
    std::vector<std::unique_ptr<EditableTableRow>> ret;
//    ret.emplace_back(std::make_unique<PlayerListRow>());
    return ret;
}




}
}
}
