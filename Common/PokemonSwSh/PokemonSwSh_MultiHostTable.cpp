/*  Multi-Host Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/NintendoSwitch/NintendoSwitch_SlotDatabase.h"
#include "PokemonSwSh_MultiHostTable.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


MultiHostSlot::MultiHostSlot(EditableTableOption& parent_table)
    : EditableTableRow(parent_table)
    , game_slot(GameSlot_Database(), LockMode::LOCK_WHILE_RUNNING, 1)
    , user_slot(UserSlot_Database(), LockMode::LOCK_WHILE_RUNNING, 1)
    , skips(LockMode::LOCK_WHILE_RUNNING, 3, 0, 7)
    , backup_save(LockMode::LOCK_WHILE_RUNNING, false)
    , always_catchable(LockMode::LOCK_WHILE_RUNNING, true)
    , use_raid_code(LockMode::LOCK_WHILE_RUNNING, true)
    , accept_FRs(LockMode::LOCK_WHILE_RUNNING, true)
    , move_slot(LockMode::LOCK_WHILE_RUNNING, 0, 0, 4)
    , dynamax(LockMode::LOCK_WHILE_RUNNING, true)
    , post_raid_delay(LockMode::LOCK_WHILE_RUNNING, "0 s")
{
    PA_ADD_OPTION(game_slot);
    PA_ADD_OPTION(user_slot);
    PA_ADD_OPTION(skips);
    PA_ADD_OPTION(backup_save);
    PA_ADD_OPTION(always_catchable);
    if (static_cast<MultiHostTable&>(parent_table).raid_code_option){
        PA_ADD_OPTION(use_raid_code);
    }
    PA_ADD_OPTION(accept_FRs);
    PA_ADD_OPTION(move_slot);
    PA_ADD_OPTION(dynamax);
    PA_ADD_OPTION(post_raid_delay);
}

std::unique_ptr<EditableTableRow> MultiHostSlot::clone() const{
    std::unique_ptr<MultiHostSlot> ret(new MultiHostSlot(parent()));
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



MultiHostTable::MultiHostTable(bool p_raid_code_option)
    : EditableTableOption("<b>Game List:</b>", LockMode::LOCK_WHILE_RUNNING)
    , raid_code_option(p_raid_code_option)
{}
std::vector<std::unique_ptr<MultiHostSlot>> MultiHostTable::copy_snapshot() const{
    return EditableTableOption::copy_snapshot<MultiHostSlot>();
}
std::vector<std::string> MultiHostTable::make_header() const{
    if (raid_code_option){
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
std::unique_ptr<EditableTableRow> MultiHostTable::make_row(){
    return std::unique_ptr<EditableTableRow>(new MultiHostSlot(*this));
}




}
}
}
