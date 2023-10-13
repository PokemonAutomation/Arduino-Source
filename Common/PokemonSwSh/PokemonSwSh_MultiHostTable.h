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
#include "Common/Cpp/Options/EditableTableOption.h"
#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "Common/NintendoSwitch/NintendoSwitch_SlotDatabase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class MultiHostSlot : public EditableTableRow{
public:
    MultiHostSlot(bool raid_code_option)
        : m_raid_code_option(raid_code_option)
        , game_slot(GameSlot_Database(), LockMode::LOCK_WHILE_RUNNING, 1)
        , user_slot(UserSlot_Database(), LockMode::LOCK_WHILE_RUNNING, 1)
        , skips(LockMode::LOCK_WHILE_RUNNING, 3, 0, 7)
        , backup_save(LockMode::LOCK_WHILE_RUNNING, false)
        , always_catchable(LockMode::LOCK_WHILE_RUNNING, true)
        , use_raid_code(LockMode::LOCK_WHILE_RUNNING, true)
        , accept_FRs(LockMode::LOCK_WHILE_RUNNING, true)
        , move_slot(LockMode::LOCK_WHILE_RUNNING, 0, 0, 4)
        , dynamax(LockMode::LOCK_WHILE_RUNNING, true)
        , post_raid_delay(LockMode::LOCK_WHILE_RUNNING, TICKS_PER_SECOND, "0 * TICKS_PER_SECOND")
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



    virtual std::unique_ptr<EditableTableRow> clone() const override{
        std::unique_ptr<MultiHostSlot> ret(new MultiHostSlot(m_raid_code_option));
        ret->game_slot.set_value(game_slot.current_value());
        ret->user_slot.set_value(user_slot.current_value());
        ret->skips.set(skips);
        ret->backup_save = (bool)backup_save;
        ret->always_catchable = (bool)always_catchable;
        ret->use_raid_code = (bool)use_raid_code;
        ret->accept_FRs = (bool)accept_FRs;
        ret->move_slot.set(move_slot);
        ret->dynamax = (bool)dynamax;
        ret->post_raid_delay.set(post_raid_delay.current_text());
        return ret;
    }

private:
    const bool m_raid_code_option;
public:
    IntegerEnumDropdownCell game_slot;
    IntegerEnumDropdownCell user_slot;
    SimpleIntegerCell<uint8_t> skips;
    BooleanCheckBoxCell backup_save;
    BooleanCheckBoxCell always_catchable;
    BooleanCheckBoxCell use_raid_code;
    BooleanCheckBoxCell accept_FRs;
    SimpleIntegerCell<uint8_t> move_slot;
    BooleanCheckBoxCell dynamax;
    TimeExpressionCell<uint16_t> post_raid_delay;
};



class MultiHostTable : public EditableTableOption{
public:
    MultiHostTable(bool raid_code_option)
        : EditableTableOption("<b>Game List:</b>", LockMode::LOCK_WHILE_RUNNING)
        , m_raid_code_option(raid_code_option)
    {}
    std::vector<std::unique_ptr<MultiHostSlot>> copy_snapshot() const{
        return EditableTableOption::copy_snapshot<MultiHostSlot>();
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
    virtual std::unique_ptr<EditableTableRow> make_row() const override{
        return std::unique_ptr<EditableTableRow>(new MultiHostSlot(m_raid_code_option));
    }

private:
    bool m_raid_code_option;
};






}
}
}
#endif
