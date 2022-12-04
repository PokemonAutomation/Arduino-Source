/*  Player List
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_PlayerList_H
#define PokemonAutomation_PokemonSV_PlayerList_H

#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
#include "CommonFramework/Options/LanguageOCROption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{




struct PlayerListRowSnapshot{
    Language language;
    std::string name;
    double log10p;
};


class PlayerListRow : public EditableTableRow{
public:
    PlayerListRow();
    virtual std::unique_ptr<EditableTableRow> clone() const;

    PlayerListRowSnapshot snapshot() const;

public:
    OCR::LanguageOCRCell language;
    StringCell name;
    FloatingPointCell log10p;
};



class PlayerListTable : public EditableTableOption_t<PlayerListRow>{
public:
    PlayerListTable(std::string label, LockWhileRunning lock_while_running);

    virtual std::vector<std::string> make_header() const;
    static std::vector<std::unique_ptr<EditableTableRow>> make_defaults();

    std::vector<PlayerListRowSnapshot> snapshot() const{
        return EditableTableOption_t<PlayerListRow>::snapshot<PlayerListRowSnapshot>();
    }
};



}
}
}
#endif
