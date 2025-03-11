/*  Multi-Host Table
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MultiHostTableOption_H
#define PokemonAutomation_PokemonSwSh_MultiHostTableOption_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/EditableTableOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class MultiHostTable;


class MultiHostSlot : public EditableTableRow{
public:
    MultiHostSlot(EditableTableOption& parent_table);

    virtual std::unique_ptr<EditableTableRow> clone() const override;

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
    MillisecondsCell post_raid_delay;
};



class MultiHostTable : public EditableTableOption{
public:
    MultiHostTable(bool p_raid_code_option);
    std::vector<std::unique_ptr<MultiHostSlot>> copy_snapshot() const;
    virtual std::vector<std::string> make_header() const override;
    virtual std::unique_ptr<EditableTableRow> make_row() override;

public:
    const bool raid_code_option;
};






}
}
}
#endif
