/*  Player List
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_PlayerList_H
#define PokemonAutomation_PokemonSV_PlayerList_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/StringOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"
#include "CommonFramework/Options/LanguageOCROption.h"

namespace PokemonAutomation{
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
    PlayerListRow();
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
        LockWhileRunning lock_while_running,
        std::string notes_label
    );

    virtual std::vector<std::string> make_header() const;
    static std::vector<std::unique_ptr<EditableTableRow>> make_defaults();

    std::vector<PlayerListRowSnapshot> snapshot() const{
        return EditableTableOption_t<PlayerListRow>::snapshot<PlayerListRowSnapshot>();
    }

private:
    std::string m_notes_label;
};




class RaidPlayerBanList : public PlayerListTable{
public:
    RaidPlayerBanList(LockWhileRunning lock_while_running)
        : PlayerListTable(
            "<b>Ban List:</b><br>Ban these people from the raid. "
            "If anyone's name matches anything in this table, the raid will be reset.<br>"
            "The last column is a tuning parameter that specifies how well the name needs to match. "
            "Optical Character Recognition (OCR) is imperfect. So exact matches are rare and unreliable. "
            "The value is the estimated log10 probability of matching by chance against random characters. "
            "It is always negative. Lower value means the match needs to be more perfect to be a match.<br><br>"
            "If you are getting false positive hits, decrease this value. (make it more negative)<br>"
            "If it is failing to match, increase this value. (make it less negative)",
            LockWhileRunning::UNLOCKED,
            "Ban Reason (shown publicly)"
        )
    {}
};





}
}
}
#endif
