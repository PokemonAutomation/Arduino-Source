/*  Pokemon SV - Multi-Host Table
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
namespace PokemonSV{


class MultiHostSlot : public EditableTableRow{
public:
    MultiHostSlot()
        : game_slot(GameSlot_Database(), LockWhileRunning::LOCKED, 1)
        , user_slot(UserSlot_Database(), LockWhileRunning::LOCKED, 1)
        , post_raid_delay(LockWhileRunning::LOCKED, TICKS_PER_SECOND, "0 * TICKS_PER_SECOND")
    {
        PA_ADD_OPTION(game_slot);
        PA_ADD_OPTION(user_slot);
        PA_ADD_OPTION(post_raid_delay);
    }



    virtual std::unique_ptr<EditableTableRow> clone() const override{
        std::unique_ptr<MultiHostSlot> ret(new MultiHostSlot());
        ret->game_slot.set_value(game_slot.current_value());
        ret->user_slot.set_value(user_slot.current_value());
        ret->post_raid_delay.set(post_raid_delay.current_text());
        return ret;
    }

public:
    IntegerEnumDropdownCell game_slot;
    IntegerEnumDropdownCell user_slot;
    TimeExpressionCell<uint16_t> post_raid_delay;
};



class MultiHostTable : public EditableTableOption{
public:
    MultiHostTable()
        : EditableTableOption("<b>Game List:</b>", LockWhileRunning::LOCKED)
    {}
    std::vector<std::unique_ptr<MultiHostSlot>> copy_snapshot() const{
        return EditableTableOption::copy_snapshot<MultiHostSlot>();
    }
    virtual std::vector<std::string> make_header() const override{
        return std::vector<std::string>{
            "Game",
            "User",
            "Post Raid Delay",
        };
    }
    virtual std::unique_ptr<EditableTableRow> make_row() const override{
        return std::unique_ptr<EditableTableRow>(new MultiHostSlot());
    }
};






}
}
}
#endif
