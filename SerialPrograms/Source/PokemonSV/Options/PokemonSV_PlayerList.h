/*  Player List
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_PlayerList_H
#define PokemonAutomation_PokemonSV_PlayerList_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
#include "CommonTools/Options/LanguageOCROption.h"

namespace PokemonAutomation{
    class Logger;
namespace NintendoSwitch{
namespace PokemonSV{




struct PlayerListRowSnapshot{
    bool enabled;
    Language language;
    std::string name;
    double log10p;
    std::string notes;
};


class PlayerListRow : public EditableTableRow{
public:
    PlayerListRow(EditableTableOption& parent_table);
    virtual std::unique_ptr<EditableTableRow> clone() const;

    PlayerListRowSnapshot snapshot() const;

public:
    BooleanCheckBoxCell enabled;
    OCR::LanguageOCRCell language;
    StringCell name;
    FloatingPointCell log10p;
    StringCell notes;
};
class PlayerListTable : public EditableTableOption_t<PlayerListRow>{
public:
    PlayerListTable(
        std::string label,
        LockMode lock_while_running,
        std::string notes_label
    );

    virtual std::vector<std::string> make_header() const;
    std::vector<std::unique_ptr<EditableTableRow>> make_defaults();

    std::vector<PlayerListRowSnapshot> snapshot() const;

private:
    std::string m_notes_label;
};




class RaidPlayerBanList : public GroupOption{
public:
    RaidPlayerBanList();

    void refresh_online_table(Logger& logger);

//    std::vector<PlayerListRowSnapshot> banlist_combined() const;
    std::vector<PlayerListRowSnapshot> banlist_local() const;
    std::vector<PlayerListRowSnapshot> banlist_global() const;

public:
    StaticTextOption text;
    PlayerListTable local_table;
    StringOption online_table_url;
    BooleanCheckBoxOption ignore_whitelist;

private:
    PlayerListTable online_table;
};






}
}
}
#endif
