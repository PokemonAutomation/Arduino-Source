/*  Multi-Host Table
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MultiHostTableOption_H
#define PokemonAutomation_PokemonSwSh_MultiHostTableOption_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption2.h"
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class MultiHostSlot2 : public EditableTableRow2{
public:
    MultiHostSlot2(bool raid_code_option)
        : m_raid_code_option(raid_code_option)
        , game_slot(
            {
                "Game 1",
                "Game 2",
            },
            0
        )
        , user_slot(
            {
                "User 1",
                "User 2",
                "User 3",
                "User 4",
                "User 5",
                "User 6",
                "User 7",
                "User 8",
            },
            0
        )
        , skips(3, 0, 7)
        , backup_save(false)
        , always_catchable(true)
        , use_raid_code(true)
        , accept_FRs(true)
        , move_slot(0, 0, 4)
        , dynamax(true)
        , post_raid_delay(TICKS_PER_SECOND, "0 * TICKS_PER_SECOND")
    {
        PA_ADD_OPTION(game_slot);
        PA_ADD_OPTION(user_slot);
        PA_ADD_OPTION(skips);
        PA_ADD_OPTION(backup_save);
        PA_ADD_OPTION(always_catchable);
        if (raid_code_option){
            PA_ADD_OPTION(use_raid_code);
        }
        PA_ADD_OPTION(accept_FRs);
        PA_ADD_OPTION(move_slot);
        PA_ADD_OPTION(dynamax);
        PA_ADD_OPTION(post_raid_delay);
    }



    virtual std::unique_ptr<EditableTableRow2> clone() const override{
        std::unique_ptr<MultiHostSlot2> ret(new MultiHostSlot2(m_raid_code_option));
        ret->game_slot.set(game_slot);
        ret->user_slot.set(user_slot);
        ret->skips.set(skips);
        ret->backup_save = (bool)backup_save;
        ret->always_catchable = (bool)always_catchable;
        ret->use_raid_code = (bool)use_raid_code;
        ret->accept_FRs = (bool)accept_FRs;
        ret->move_slot.set(move_slot);
        ret->dynamax = (bool)dynamax;
        ret->post_raid_delay.set(post_raid_delay.text());
        return ret;
    }

private:
    const bool m_raid_code_option;
public:
    EnumDropdownCell game_slot;
    EnumDropdownCell user_slot;
    SimpleIntegerCell<uint8_t> skips;
    BooleanCheckBoxCell backup_save;
    BooleanCheckBoxCell always_catchable;
    BooleanCheckBoxCell use_raid_code;
    BooleanCheckBoxCell accept_FRs;
    SimpleIntegerCell<uint8_t> move_slot;
    BooleanCheckBoxCell dynamax;
    TimeExpressionCell<uint16_t> post_raid_delay;
};



class MultiHostTable2 : public EditableTableOptionCore{
public:
    MultiHostTable2(bool raid_code_option)
        : EditableTableOptionCore("<b>Game List:</b>")
        , m_raid_code_option(raid_code_option)
    {}
    std::vector<std::unique_ptr<MultiHostSlot2>> copy_snapshot() const{
        return EditableTableOptionCore::copy_snapshot<MultiHostSlot2>();
    }
    virtual std::vector<std::string> make_header() const override{
        if (m_raid_code_option){
            return std::vector<std::string>{
                "Game",
                "User",
                "Skips",
                "Backup Save",
                "Always Catchable",
                "Use Raid Code",
                "Accept FRs",
                "1st Move",
                "Dynamax",
                "Post Raid Delay",
            };
        }else{
            return std::vector<std::string>{
                "Game",
                "User",
                "Skips",
                "Backup Save",
                "Always Catchable",
                "Accept FRs",
                "1st Move",
                "Dynamax",
                "Post Raid Delay",
            };
        }
    }
    virtual std::unique_ptr<EditableTableRow2> make_row() const override{
        return std::unique_ptr<EditableTableRow2>(new MultiHostSlot2(m_raid_code_option));
    }

private:
    bool m_raid_code_option;
};






}
}
}
#endif
